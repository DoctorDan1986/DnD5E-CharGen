#include "dnd/character.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

static void check(bool condition, const std::string& what) {
    if (!condition) throw std::runtime_error("FAILED: " + what);
}

static void file(const fs::path& p, const std::string& bytes) {
    fs::create_directories(p.parent_path());
    std::ofstream out(p);
    out << bytes;
}

int main() {
    try {
        dnd::ContentRegistry registry;
        registry.load(fs::path(DND_SOURCE_DIR) / "Content");
        check(registry.diagnostics().empty(), "example packages validate");
        check(registry.size() == 8, "core and homebrew definitions loaded");
        check(registry.ofKind("race", "5e-2014").size() == 1, "dwarf in registry");
        check(registry.ofKind("subrace", "5e-2014").size() == 3, "homebrew subrace autoloaded");
        check(dnd::abilityModifier(9) == -1, "ability score 9 uses floor");
        check(dnd::abilityModifier(7) == -2, "ability score 7 uses floor");
        check(dnd::abilityModifier(1) == -5, "ability score 1 uses floor");
        check(dnd::abilityModifier(16) == 3, "ability score 16 modifier");

        dnd::CharacterChoices choices;
        choices.baseAbilities = {15, 14, 13, 12, 10, 8};
        choices.raceId = "core2014:dwarf";
        choices.subraceId = "core2014:hill_dwarf";
        auto hill = dnd::resolveCharacter(registry, choices);
        check(hill.abilities[2] == 15, "dwarf +2 CON applied");
        check(hill.abilities[4] == 11, "hill dwarf +1 WIS applied");
        check(hill.modifiers[2] == 2, "CON modifier recalculated");
        check(hill.bonusHitPoints == 1, "level one toughness bonus");
        check(hill.senses.size() == 1 && hill.senses[0] == "Darkvision 60 ft.", "trait reference grants sense");
        choices.level = 10;
        check(dnd::resolveCharacter(registry, choices).bonusHitPoints == 10, "per-level effect scales");
        choices.subraceId = "examplepack:runic_dwarf";
        auto runic = dnd::resolveCharacter(registry, choices);
        check(runic.abilities[3] == 13, "homebrew subrace gives INT +1");
        check(runic.appliedTraits.size() == 2, "inherited and homebrew traits both resolved");
        check(runic.bonusHitPoints == 10, "referenced custom feat automatically grants HP");
        check(runic.appliedFeats.size() == 1, "referenced feat automatically registered");
        choices.subraceId = "core2014:hill_dwarf";
        choices.featIds = {"examplepack:iron_constitution"};
        check(dnd::resolveCharacter(registry, choices).bonusHitPoints == 20, "selected feat stacks with Hill Dwarf toughness");
        choices.featIds.push_back("examplepack:iron_constitution");
        bool threw = false;
        try { (void)dnd::resolveCharacter(registry, choices); }
        catch (const std::exception&) { threw = true; }
        check(threw, "duplicate feats rejected");
        choices.featIds.clear();
        choices.subraceId = "core2014:hill_dwarf";
        choices.raceId = "examplepack:rune_memory";
        threw = false;
        try { (void)dnd::resolveCharacter(registry, choices); }
        catch (const std::exception&) { threw = true; }
        check(threw, "wrong definition kind rejected");

        const auto tmp = fs::temp_directory_path() /
             ("dnd5e-tests-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(tmp);
        try {
            file(tmp / "Custom" / "manifest.json", R"({"id":"custom","name":"Custom Pack","schema_version":"1.0","version":"0.1.0","ruleset":"5e-2014","requires":[]})");
            file(tmp / "Custom" / "traits" / "valid.json", R"({"id":"custom:valid","kind":"trait","name":"Valid","ruleset":"5e-2014","effects":[]})");
            file(tmp / "Custom" / "traits" / "invalid.json", R"({"id":"custom:invalid","kind":"trait","name":"Invalid","ruleset":"5e-2014","effects":[{"op":"arbitrary.execute","value":1}]})");
            file(tmp / "Custom" / "subraces" / "orphan.json", R"({"id":"custom:orphan","kind":"subrace","name":"Orphan","ruleset":"5e-2014","parent":"custom:missing","traits":[]})");
            dnd::ContentRegistry malformed;
            malformed.loadMany({fs::path(DND_SOURCE_DIR) / "Content", tmp});
            check(malformed.find("custom:valid") != nullptr, "valid sibling survives bad JSON definition");
            check(malformed.find("custom:invalid") == nullptr, "unrecognized effect rejected");
            check(malformed.find("custom:orphan") == nullptr, "broken reference rejected");
            check(malformed.diagnostics().size() == 2, "bad content diagnosed");
            file(tmp / "Dependent" / "manifest.json", R"({"id":"dependent","name":"Dependent","schema_version":"1.0","version":"0.1.0","ruleset":"5e-2014","requires":["not_installed"]})");
            file(tmp / "Dependent" / "traits" / "ignored.json", R"({"id":"dependent:ignored","kind":"trait","name":"Ignored","ruleset":"5e-2014"})");
            malformed.loadMany({fs::path(DND_SOURCE_DIR) / "Content", tmp});
            check(malformed.find("dependent:ignored") == nullptr, "missing-dependency pack disabled");
        } catch (...) {
            fs::remove_all(tmp);
            throw;
        }
        fs::remove_all(tmp);
        std::cout << "All core tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

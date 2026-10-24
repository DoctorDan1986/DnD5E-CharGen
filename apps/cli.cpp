#include "dnd/character.hpp"
#include <filesystem>
#include <iostream>

int main(int argc, char** argv) {
    const std::filesystem::path root = argc > 1 ? argv[1] : "Content";
    dnd::ContentRegistry registry;
    registry.load(root);
    for (const auto& diagnostic : registry.diagnostics())
        std::cerr << "Content error [" << diagnostic.file << "]: " << diagnostic.message << '\n';
    std::cout << "Loaded " << registry.size() << " definitions from " << root << "\n";
    auto races = registry.ofKind("race", "5e-2014");
    for (const auto* race : races) std::cout << "Race: " << race->name << " (" << race->id << ")\n";
    if (races.empty()) return 1;
    try {
        dnd::CharacterChoices choices;
        choices.raceId = races.front()->id;
        const auto subraces = registry.ofKind("subrace", "5e-2014");
        for (const auto* sub : subraces)
            if (std::string(sub->data.at("parent").as_string()) == choices.raceId) {
                choices.subraceId = sub->id;
                break;
            }
        auto result = dnd::resolveCharacter(registry, choices);
        std::cout << "Demo result (base scores 15,14,13,12,10,8):\n";
        for (int i = 0; i < 6; ++i)
            std::cout << dnd::abilityNames[i] << " " << result.abilities[i]
                      << " (" << (result.modifiers[i] >= 0 ? "+" : "")
                      << result.modifiers[i] << ")\n";
        std::cout << "Speed " << result.speed << " ft.; trait HP bonus +"
                  << result.bonusHitPoints << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Character resolution error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}

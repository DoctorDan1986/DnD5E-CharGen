#include "dnd/character.hpp"

#include <algorithm>
#include <set>
#include <stdexcept>

namespace dnd {
namespace {

int indexFor(const std::string& key) {
    for (int i = 0; i < 6; ++i) if (key == abilityNames[i]) return i;
    throw std::runtime_error("Invalid ability: " + key);
}

std::string readString(const boost::json::object& obj, const char* key) {
    auto* v = obj.if_contains(key);
    if (!v || !v->is_string()) throw std::runtime_error(std::string("Missing string: ") + key);
    return std::string(v->as_string());
}

const Definition& mustFind(const ContentRegistry& r, const std::string& id,
                           const char* kind, const std::string& edition) {
    auto* def = r.find(id);
    if (!def || def->kind != kind || def->ruleset != edition)
        throw std::runtime_error("Invalid or unavailable " + std::string(kind) + ": " + id);
    return *def;
}

void applyEffects(const Definition& def, ResolvedCharacter& output, int level) {
    auto* effects = def.data.if_contains("effects");
    if (!effects) return;
    for (const auto& value : effects->as_array()) {
        const auto& effect = value.as_object();
        const auto op = readString(effect, "op");
        if (op == "ability.bonus") {
            output.abilities[indexFor(readString(effect, "ability"))] +=
                static_cast<int>(effect.at("value").as_int64());
        } else if (op == "hit_points.bonus_per_level") {
            output.bonusHitPoints += static_cast<int>(effect.at("value").as_int64()) * level;
        } else if (op == "sense.grant") {
            output.senses.push_back(readString(effect, "name") + " " +
                                    std::to_string(effect.at("range").as_int64()) + " ft.");
        }
    }
}

}  // namespace

int abilityModifier(int score) {
    // Mathematical floor, including negative half-values (C++ division truncates).
    return score >= 10 ? (score - 10) / 2 : (score - 11) / 2;
}

ResolvedCharacter resolveCharacter(const ContentRegistry& registry,
                                   const CharacterChoices& choices) {
    if (choices.ruleset != "5e-2014")
        throw std::runtime_error("The 2024 rules engine is not implemented yet");
    if (choices.level < 1 || choices.level > 20)
        throw std::runtime_error("Character level must be 1 through 20");
    for (const int score : choices.baseAbilities)
        if (score < 3 || score > 20) throw std::runtime_error("Base ability must be 3 through 20");

    const auto& race = mustFind(registry, choices.raceId, "race", choices.ruleset);
    const Definition* subrace = nullptr;
    if (!choices.subraceId.empty()) {
        subrace = &mustFind(registry, choices.subraceId, "subrace", choices.ruleset);
        if (readString(subrace->data, "parent") != race.id)
            throw std::runtime_error("Subrace does not belong to selected race");
    }

    ResolvedCharacter out;
    out.abilities = choices.baseAbilities;
    out.speed = static_cast<int>(race.data.at("speed").as_int64());
    out.size = readString(race.data, "size");

    std::set<std::string> visited, active;
    auto walk = [&](auto&& self, const std::string& id, const char* kind) -> void {
        if (active.count(id)) throw std::runtime_error("Cyclic trait/feat reference: " + id);
        if (visited.count(id)) return;
        const auto& feature = mustFind(registry, id, kind, choices.ruleset);
        active.insert(id);
        // Linked features are applied once even when multiple sources grant them.
        for (const char* refKind : {"trait", "feat"}) {
            const char* key = std::string(refKind) == "trait" ? "traits" : "feats";
            if (auto* refs = feature.data.if_contains(key))
                for (const auto& child : refs->as_array())
                    self(self, std::string(child.as_string()), refKind);
        }
        active.erase(id);
        visited.insert(id);
        applyEffects(feature, out, choices.level);
        if (feature.kind == "trait") out.appliedTraits.push_back(id);
        else out.appliedFeats.push_back(id);
    };
    auto process = [&](const Definition& def) {
        applyEffects(def, out, choices.level);
        for (const char* refKind : {"trait", "feat"}) {
            const char* key = std::string(refKind) == "trait" ? "traits" : "feats";
            if (auto* refs = def.data.if_contains(key))
                for (const auto& ref : refs->as_array())
                    walk(walk, std::string(ref.as_string()), refKind);
        }
    };

    process(race);
    if (subrace) process(*subrace);
    std::set<std::string> uniqueFeats;
    for (const auto& featId : choices.featIds) {
        if (!uniqueFeats.insert(featId).second || visited.count(featId))
            throw std::runtime_error("Duplicate feat selection: " + featId);
        walk(walk, featId, "feat");
    }
    for (int i = 0; i < 6; ++i) {
        if (out.abilities[i] < 1 || out.abilities[i] > 30)
            throw std::runtime_error("Calculated ability score out of range");
        out.modifiers[i] = abilityModifier(out.abilities[i]);
    }
    return out;
}

}  // namespace dnd

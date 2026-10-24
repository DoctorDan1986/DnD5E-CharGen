#pragma once

#include "dnd/content_registry.hpp"
#include <array>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace dnd {

inline constexpr std::array<const char*, 6> abilityNames =
    {"STR", "DEX", "CON", "INT", "WIS", "CHA"};

struct CharacterChoices {
    std::string ruleset = "5e-2014";
    std::string raceId;
    std::string subraceId;
    std::string classId;
    int level = 1;
    std::array<int, 6> baseAbilities = {15, 14, 13, 12, 10, 8};
    std::vector<std::string> featIds;
    // Keys are <qualified-source-ID>#<local-choice-ID>.
    // Values are stable option IDs (not translated display text).
    std::map<std::string, std::vector<std::string>> selections;
    std::vector<std::string> equipmentTags;
};

struct ChoiceOption {
    std::string id;
    std::string label;
};
struct ChoiceRequest {
    std::string key;
    std::string label;
    int min = 1;
    int max = 1;
    std::vector<ChoiceOption> options;
    std::vector<std::string> selected;
};

struct ResolvedCharacter {
    std::array<int, 6> abilities{};
    std::array<int, 6> modifiers{};
    std::array<int, 6> savingThrows{};
    std::map<std::string, int> skillBonuses;
    std::map<std::string, std::set<std::string>> proficiencies;
    std::map<std::string, std::set<std::string>> expertises;
    int proficiencyBonus = 2;
    int speed = 0;
    int bonusHitPoints = 0;  // Additional HP, not total max HP.
    std::string size;
    std::vector<std::string> appliedTraits;
    std::vector<std::string> appliedFeats;
    std::vector<std::string> senses;
    std::vector<ChoiceRequest> requests;
    bool complete = true;  // False when mandatory choices remain unresolved.
};

// A missing choice is returned as a ChoiceRequest; invalid/stale choices throw.
ResolvedCharacter resolveCharacter(const ContentRegistry&, const CharacterChoices&);
int abilityModifier(int score);

// Versioned, portable save file containing selections only (no calculated totals).
std::string serializeCharacterChoices(const CharacterChoices&);
CharacterChoices parseCharacterChoices(const std::string& jsonText);

}  // namespace dnd

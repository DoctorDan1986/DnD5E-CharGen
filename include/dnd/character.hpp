#pragma once

#include "dnd/content_registry.hpp"
#include <array>
#include <string>
#include <vector>

namespace dnd {

inline constexpr std::array<const char*, 6> abilityNames =
    {"STR", "DEX", "CON", "INT", "WIS", "CHA"};

struct CharacterChoices {
    std::string ruleset = "5e-2014";
    std::string raceId;
    std::string subraceId;
    int level = 1;
    std::array<int, 6> baseAbilities = {15, 14, 13, 12, 10, 8};
    std::vector<std::string> featIds;
};

struct ResolvedCharacter {
    std::array<int, 6> abilities{};
    std::array<int, 6> modifiers{};
    int speed = 0;
    int bonusHitPoints = 0;  // Extra HP from effects only; NOT maximum HP.
    std::string size;
    std::vector<std::string> appliedTraits;
    std::vector<std::string> appliedFeats;
    std::vector<std::string> senses;
};

// No hidden global state: calculations can be repeated after any player change.
// Throws std::runtime_error if references or choices are invalid.
ResolvedCharacter resolveCharacter(const ContentRegistry&, const CharacterChoices&);
int abilityModifier(int score);

}  // namespace dnd

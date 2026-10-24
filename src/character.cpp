#include "dnd/character.hpp"

#include <algorithm>
#include <boost/json.hpp>
#include <boost/system/error_code.hpp>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <utility>

namespace dnd {
namespace {
namespace json = boost::json;

const std::map<std::string, int> skillAbilities = {
    {"acrobatics",1}, {"animal_handling",4}, {"arcana",3}, {"athletics",0},
    {"deception",5}, {"history",3}, {"insight",4}, {"intimidation",5},
    {"investigation",3}, {"medicine",4}, {"nature",3}, {"perception",4},
    {"performance",5}, {"persuasion",5}, {"religion",3}, {"sleight_of_hand",1},
    {"stealth",1}, {"survival",4}
};

int indexFor(const std::string& key) {
    for (int i = 0; i < 6; ++i) if (key == abilityNames[i]) return i;
    throw std::runtime_error("Invalid ability: " + key);
}
std::string str(const json::object& obj, const char* key) {
    auto* v = obj.if_contains(key);
    if (!v || !v->is_string()) throw std::runtime_error(std::string("Missing string: ") + key);
    return std::string(v->as_string());
}
int number(const json::object& obj, const char* key) {
    const auto* v = obj.if_contains(key);
    if (!v || !v->is_int64() || v->as_int64() < std::numeric_limits<int>::min() ||
        v->as_int64() > std::numeric_limits<int>::max())
        throw std::runtime_error(std::string("Missing or out-of-range integer: ") + key);
    return static_cast<int>(v->as_int64());
}
const Definition& require(const ContentRegistry& r, const std::string& id,
                          const char* kind, const std::string& ruleset) {
    auto* d = r.find(id);
    if (!d || d->kind != kind || d->ruleset != ruleset)
        throw std::runtime_error("Invalid or unavailable " + std::string(kind) + ": " + id);
    return *d;
}
std::vector<std::string> strings(const json::object& obj, const char* key) {
    std::vector<std::string> out;
    if (auto* a = obj.if_contains(key)) {
        for (const auto& v : a->as_array()) out.emplace_back(v.as_string());
    }
    return out;
}
bool qualifies(const json::object& conditions, const CharacterChoices& choices,
               const ResolvedCharacter& out) {
    if (auto* level = conditions.if_contains("level_at_least"))
        if (choices.level < level->as_int64()) return false;
    if (auto* score = conditions.if_contains("score_at_least"))
        if (out.abilities[indexFor(str(conditions, "ability"))] < score->as_int64()) return false;
    if (auto* tag = conditions.if_contains("equipment_tag"))
        if (std::find(choices.equipmentTags.begin(), choices.equipmentTags.end(),
                      std::string(tag->as_string())) == choices.equipmentTags.end()) return false;
    return true;
}
void apply(const json::object& effect, const CharacterChoices& choices,
           ResolvedCharacter& out, bool abilityPhase) {
    const std::string op = str(effect, "op");
    if ((op == "ability.bonus") != abilityPhase) return;
    if (auto* when = effect.if_contains("when"))
        if (!qualifies(when->as_object(), choices, out)) return;
    if (op == "ability.bonus") {
        out.abilities[indexFor(str(effect, "ability"))] += number(effect, "value");
    } else if (op == "hit_points.bonus_per_level") {
        out.bonusHitPoints += number(effect, "value") * choices.level;
    } else if (op == "sense.grant") {
        const std::string sense = str(effect, "name") + " " +
            std::to_string(number(effect, "range")) + " ft.";
        if (std::find(out.senses.begin(), out.senses.end(), sense) == out.senses.end())
            out.senses.push_back(sense);
    } else if (op == "proficiency.grant" || op == "proficiency.expertise") {
        const auto category = str(effect, "category");
        const auto id = str(effect, "id");
        if (op == "proficiency.grant") out.proficiencies[category].insert(id);
        else out.expertises[category].insert(id);
    }
}

void applyEffects(const json::object& obj, const CharacterChoices& choices,
                  ResolvedCharacter& out, bool abilityPhase) {
    if (auto* effects = obj.if_contains("effects"))
        for (const auto& effect : effects->as_array())
            apply(effect.as_object(), choices, out, abilityPhase);
}

// Restrict all dynamic JSON to known grammar operations. A choice only supplies
// effect data; it can never execute code or invoke an arbitrary function.
struct Source {
    const json::object* obj;
    std::string id;
    bool explicitFeat = false;
};

std::vector<Source> gather(const ContentRegistry& registry, const CharacterChoices& choices,
                           const Definition& race, const Definition* subrace,
                           const Definition* characterClass) {
    std::vector<Source> sources;
    std::set<std::string> active, visited;
    auto walk = [&](auto&& self, const Definition& def, bool isExplicit) -> void {
        if (active.count(def.id)) throw std::runtime_error("Cyclic feature reference: " + def.id);
        if (visited.count(def.id)) return;
        active.insert(def.id);
        sources.push_back({&def.data, def.id, isExplicit});
        auto child = [&](const json::object& parent) {
            for (const char* kind : {"trait", "feat"}) {
                for (const auto& id : strings(parent, std::string(kind) == "trait" ? "traits" : "feats"))
                    self(self, require(registry, id, kind, choices.ruleset), isExplicit);
            }
        };
        child(def.data);
        // Level gates may grant referenced features and inline effects.
        if (auto* progression = def.data.if_contains("progression")) {
            for (const auto& stepValue : progression->as_array()) {
                const auto& step = stepValue.as_object();
                if (number(step, "level") <= choices.level) {
                    sources.push_back({&step, def.id + "@" + std::to_string(number(step, "level")), isExplicit});
                    child(step);
                }
            }
        }
        active.erase(def.id);
        visited.insert(def.id);
    };
    walk(walk, race, false);
    if (subrace) walk(walk, *subrace, false);
    if (characterClass) walk(walk, *characterClass, false);
    for (const auto& feat : choices.featIds)
        walk(walk, require(registry, feat, "feat", choices.ruleset), true);
    return sources;
}

json::array toArray(const std::vector<std::string>& items) {
    json::array out;
    for (const auto& id : items) out.emplace_back(id);
    return out;
}
std::vector<std::string> readArray(const json::object& obj, const char* key) {
    std::vector<std::string> out;
    if (auto* value = obj.if_contains(key)) {
        if (!value->is_array()) throw std::runtime_error(std::string("Expected array: ") + key);
        for (const auto& item : value->as_array()) {
            if (!item.is_string()) throw std::runtime_error(std::string("Expected string in ") + key);
            out.emplace_back(item.as_string());
        }
    }
    return out;
}
}  // namespace

int abilityModifier(int score) {
    return score >= 10 ? (score - 10) / 2 : (score - 11) / 2;
}

ResolvedCharacter resolveCharacter(const ContentRegistry& registry,
                                   const CharacterChoices& choices) {
    if (choices.ruleset != "5e-2014")
        throw std::runtime_error("The 2024 rules engine is not implemented yet");
    if (choices.level < 1 || choices.level > 20)
        throw std::runtime_error("Character level must be 1 through 20");
    for (int score : choices.baseAbilities)
        if (score < 3 || score > 20) throw std::runtime_error("Base ability must be 3 through 20");
    const auto& race = require(registry, choices.raceId, "race", choices.ruleset);
    const Definition* subrace = nullptr;
    if (!choices.subraceId.empty()) {
        subrace = &require(registry, choices.subraceId, "subrace", choices.ruleset);
        if (str(subrace->data, "parent") != race.id)
            throw std::runtime_error("Subrace does not belong to selected race");
    }
    const Definition* klass = choices.classId.empty() ? nullptr :
        &require(registry, choices.classId, "class", choices.ruleset);
    ResolvedCharacter out;
    out.abilities = choices.baseAbilities;
    out.proficiencyBonus = 2 + (choices.level - 1) / 4;
    out.speed = number(race.data, "speed");
    out.size = str(race.data, "size");
    const auto sources = gather(registry, choices, race, subrace, klass);
    // Phase 1: apply racial/class bonuses first; explicit feat bonuses cannot
    // qualify that same feat for its own prerequisites.
    for (const auto& source : sources)
        if (!source.explicitFeat) applyEffects(*source.obj, choices, out, true);
    for (int ability : out.abilities)
        if (ability < 1 || ability > 30) throw std::runtime_error("Calculated ability out of range");

    // Phase 2: collect and validate all dynamic choice requests.
    std::set<std::string> seenKeys;
    std::vector<std::pair<const json::object*, bool>> selectedOptions;
    for (const auto& source : sources) {
        if (auto* definitions = source.obj->if_contains("choices")) {
            for (const auto& value : definitions->as_array()) {
                const auto& def = value.as_object();
                ChoiceRequest req;
                req.key = source.id + "#" + str(def, "id");
                req.label = str(def, "label");
                req.min = number(def, "min");
                req.max = number(def, "max");
                if (!seenKeys.insert(req.key).second)
                    throw std::runtime_error("Duplicate choice key: " + req.key);
                for (const auto& option : def.at("options").as_array()) {
                    const auto& item = option.as_object();
                    req.options.push_back({str(item, "id"), str(item, "label")});
                }
                auto found = choices.selections.find(req.key);
                if (found != choices.selections.end()) req.selected = found->second;
                std::set<std::string> unique;
                if (static_cast<int>(req.selected.size()) > req.max)
                    throw std::runtime_error("Too many selections for " + req.key);
                for (const auto& id : req.selected) {
                    if (!unique.insert(id).second) throw std::runtime_error("Duplicate option: " + id);
                    auto& options = def.at("options").as_array();
                    auto it = std::find_if(options.begin(), options.end(), [&](const json::value& x) {
                        return str(x.as_object(), "id") == id;
                    });
                    if (it == options.end()) throw std::runtime_error("Unknown option: " + id);
                    selectedOptions.push_back({&it->as_object(), source.explicitFeat});
                }
                if (static_cast<int>(req.selected.size()) < req.min) out.complete = false;
                out.requests.push_back(std::move(req));
            }
        }
    }
    for (const auto& [key, _] : choices.selections)
        if (!seenKeys.count(key)) throw std::runtime_error("Stale or unavailable choice: " + key);
    for (const auto& option : selectedOptions)
        if (!option.second) applyEffects(*option.first, choices, out, true);
    // Prerequisites see racial/class bonuses and completed non-feat choices,
    // but never the ability bonus granted by the selected feat itself.
    // Player-selected feats must satisfy prerequisites BEFORE their own bonuses.
    // Note: the initial milestone does not track historical feat acquisition.
    std::set<std::string> selectedFeats;
    for (const auto& id : choices.featIds) {
        if (!selectedFeats.insert(id).second)
            throw std::runtime_error("Duplicate feat selection: " + id);
    }
    for (const auto& id : choices.featIds) {
        const auto& feat = require(registry, id, "feat", choices.ruleset);
        if (auto* prereqs = feat.data.if_contains("prerequisites")) {
            for (const auto& v : prereqs->as_array()) {
                const auto& p = v.as_object();
                auto type = str(p, "type");
                bool ok = false;
                if (type == "level") ok = choices.level >= number(p, "min");
                if (type == "ability") ok = out.abilities[indexFor(str(p, "ability"))] >= number(p, "min");
                if (type == "race") ok = choices.raceId == str(p, "id");
                if (type == "subrace") ok = choices.subraceId == str(p, "id");
                if (type == "class") ok = choices.classId == str(p, "id");
                if (type == "feat") ok = selectedFeats.count(str(p, "id")) != 0 && str(p,"id") != id;
                if (!ok) throw std::runtime_error("Unmet prerequisite for " + id + ": " + type);
            }
        }
    }
    for (const auto& source : sources)
        if (source.explicitFeat) applyEffects(*source.obj, choices, out, true);
    for (const auto& option : selectedOptions)
        if (option.second) applyEffects(*option.first, choices, out, true);
    for (int ability : out.abilities)
        if (ability < 1 || ability > 30) throw std::runtime_error("Calculated ability out of range");
    for (int i = 0; i < 6; ++i) out.modifiers[i] = abilityModifier(out.abilities[i]);

    // Phase 3: non-ability mechanics; conditional effects consult finalized scores.
    for (const auto& source : sources) applyEffects(*source.obj, choices, out, false);
    for (const auto& option : selectedOptions) applyEffects(*option.first, choices, out, false);
    for (const auto& source : sources) {
        auto* def = registry.find(source.id);
        if (!def) continue;
        if (def->kind == "trait") out.appliedTraits.push_back(source.id);
        if (def->kind == "feat") out.appliedFeats.push_back(source.id);
    }
    for (const auto& [category, ids] : out.expertises)
        for (const auto& id : ids)
            if (!out.proficiencies[category].count(id))
                throw std::runtime_error("Expertise without proficiency: " + category + ":" + id);
    for (int i = 0; i < 6; ++i)
        out.savingThrows[i] = out.modifiers[i] +
            (out.proficiencies["saving_throw"].count(abilityNames[i]) ? out.proficiencyBonus : 0);
    for (const auto& [skill, idx] : skillAbilities) {
        out.skillBonuses[skill] = out.modifiers[idx] + out.proficiencyBonus *
            (out.expertises["skill"].count(skill) ? 2 : (out.proficiencies["skill"].count(skill) ? 1 : 0));
    }
    return out;
}

std::string serializeCharacterChoices(const CharacterChoices& choices) {
    json::object obj;
    obj["format"] = "dnd5e.character";
    obj["version"] = 1;
    obj["ruleset"] = choices.ruleset;
    obj["race_id"] = choices.raceId;
    obj["subrace_id"] = choices.subraceId;
    obj["class_id"] = choices.classId;
    obj["level"] = choices.level;
    json::array ability;
    for (int score : choices.baseAbilities) ability.emplace_back(score);
    obj["base_abilities"] = std::move(ability);
    obj["feat_ids"] = toArray(choices.featIds);
    obj["equipment_tags"] = toArray(choices.equipmentTags);
    json::object selections;
    for (const auto& [key, ids] : choices.selections) selections[key] = toArray(ids);
    obj["selections"] = std::move(selections);
    return json::serialize(obj);
}

CharacterChoices parseCharacterChoices(const std::string& jsonText) {
    boost::system::error_code error;
    const auto document = json::parse(jsonText, error);
    if (error || !document.is_object()) throw std::runtime_error("Invalid character JSON");
    const auto& obj = document.as_object();
    if (str(obj, "format") != "dnd5e.character" || number(obj,"version") != 1)
        throw std::runtime_error("Unsupported character file format/version");
    CharacterChoices choices;
    choices.ruleset = str(obj, "ruleset");
    choices.raceId = str(obj, "race_id");
    choices.subraceId = str(obj, "subrace_id");
    choices.classId = str(obj, "class_id");
    choices.level = number(obj, "level");
    auto* abilities = obj.if_contains("base_abilities");
    if (!abilities || !abilities->is_array() || abilities->as_array().size() != 6)
        throw std::runtime_error("Expected six base ability scores");
    for (int i = 0; i < 6; ++i) {
        const auto& value = abilities->as_array()[i];
        if (!value.is_int64() || value.as_int64() < 3 || value.as_int64() > 20)
            throw std::runtime_error("Invalid ability value");
        choices.baseAbilities[i] = static_cast<int>(value.as_int64());
    }
    choices.featIds = readArray(obj, "feat_ids");
    choices.equipmentTags = readArray(obj, "equipment_tags");
    auto* selectionValue = obj.if_contains("selections");
    if (!selectionValue || !selectionValue->is_object())
        throw std::runtime_error("Expected selections object");
    for (const auto& [key, val] : selectionValue->as_object()) {
        if (!val.is_array()) throw std::runtime_error("Invalid selection list");
        const std::string choiceKey(key);
        for (const auto& item : val.as_array()) {
            if (!item.is_string()) throw std::runtime_error("Invalid selected option");
            choices.selections[choiceKey].emplace_back(item.as_string());
        }
    }
    return choices;
}
}  // namespace dnd

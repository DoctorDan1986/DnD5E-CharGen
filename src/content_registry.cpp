#include "dnd/content_registry.hpp"
#include "dnd/character.hpp"

#include <algorithm>
#include <boost/system/error_code.hpp>
#include <fstream>
#include <iterator>
#include <set>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace dnd {
namespace {
namespace fs = std::filesystem;
namespace json = boost::json;

std::string readFile(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open file");
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

json::object parseObject(const fs::path& path) {
    const auto bytes = readFile(path);
    boost::system::error_code ec;
    json::value value = json::parse(bytes, ec);
    if (ec) throw std::runtime_error("Invalid JSON: " + ec.message());
    if (!value.is_object()) throw std::runtime_error("Document must be a JSON object");
    return value.as_object();
}

std::string requiredString(const json::object& obj, const char* key) {
    auto* v = obj.if_contains(key);
    if (!v || !v->is_string() || v->as_string().empty())
        throw std::runtime_error(std::string("Required non-empty string: ") + key);
    return std::string(v->as_string());
}

std::string optionalString(const json::object& obj, const char* key) {
    auto* v = obj.if_contains(key);
    if (!v) return {};
    if (!v->is_string()) throw std::runtime_error(std::string("Expected string: ") + key);
    return std::string(v->as_string());
}

std::vector<std::string> stringArray(const json::object& obj, const char* key) {
    std::vector<std::string> result;
    const auto* v = obj.if_contains(key);
    if (!v) return result;
    if (!v->is_array()) throw std::runtime_error(std::string("Expected array: ") + key);
    for (const auto& item : v->as_array()) {
        if (!item.is_string() || item.as_string().empty())
            throw std::runtime_error(std::string("Expected non-empty string in: ") + key);
        result.emplace_back(item.as_string());
    }
    return result;
}

void validateEffects(const json::object& object) {
    const auto* value = object.if_contains("effects");
    if (!value) return;
    if (!value->is_array()) throw std::runtime_error("effects must be an array");
    for (const auto& entry : value->as_array()) {
        if (!entry.is_object()) throw std::runtime_error("Each effect must be an object");
        const auto& effect = entry.as_object();
        const auto op = requiredString(effect, "op");
        if (auto* when = effect.if_contains("when")) {
            if (op == "ability.bonus")
                throw std::runtime_error("Conditional ability bonuses are not supported: circular dependency");
            if (!when->is_object()) throw std::runtime_error("when must be an object");
            const auto& condition = when->as_object();
            if (condition.empty()) throw std::runtime_error("Empty condition");
            for (const auto& [key, value] : condition) {
                auto name = std::string(key);
                if (name == "level_at_least" || name == "score_at_least") {
                    if (!value.is_int64() || value.as_int64() < 1 || value.as_int64() > 30)
                        throw std::runtime_error("Invalid condition threshold");
                } else if (name == "ability") {
                    if (!value.is_string() ||
                        std::find_if(abilityNames.begin(), abilityNames.end(),
                           [&](const char* a) { return value.as_string() == a; }) == abilityNames.end())
                        throw std::runtime_error("Invalid condition ability");
                } else if (name == "equipment_tag") {
                    if (!value.is_string() || value.as_string().empty())
                        throw std::runtime_error("Invalid equipment tag");
                } else throw std::runtime_error("Unknown condition: " + name);
            }
            if ((condition.contains("score_at_least") && !condition.contains("ability")) ||
                (condition.contains("ability") && !condition.contains("score_at_least")))
                throw std::runtime_error("score_at_least needs ability (and vice versa)");
        }
        if (op == "ability.bonus") {
            const auto ability = requiredString(effect, "ability");
            if (std::find_if(abilityNames.begin(), abilityNames.end(),
                    [&](const char* x) { return ability == x; }) == abilityNames.end())
                throw std::runtime_error("Unknown ability: " + ability);
        } else if (op == "hit_points.bonus_per_level") {
            // Validated value below.
        } else if (op == "sense.grant") {
            requiredString(effect, "name");
            auto* range = effect.if_contains("range");
            if (!range || !range->is_int64() || range->as_int64() <= 0 || range->as_int64() > 1000)
                throw std::runtime_error("sense.grant requires positive integer range");
            continue;
        } else if (op == "proficiency.grant" || op == "proficiency.expertise") {
            const auto category = requiredString(effect, "category");
            const auto id = requiredString(effect, "id");
            const std::set<std::string> categories = {
                "skill", "saving_throw", "tool", "weapon", "armor", "language"};
            if (!categories.count(category)) throw std::runtime_error("Unknown proficiency category: " + category);
            if (op == "proficiency.expertise" && category != "skill" && category != "tool")
                throw std::runtime_error("Expertise only supports skills and tools");
            const std::set<std::string> skills = {"acrobatics", "animal_handling", "arcana", "athletics",
              "deception", "history", "insight", "intimidation", "investigation", "medicine",
              "nature", "perception", "performance", "persuasion", "religion", "sleight_of_hand",
              "stealth", "survival"};
            if (category == "skill" && !skills.count(id))
                throw std::runtime_error("Unknown 2014 skill: " + id);
            if (category == "saving_throw" &&
                std::find_if(abilityNames.begin(), abilityNames.end(),
                    [&](const char* a) { return id == a; }) == abilityNames.end())
                throw std::runtime_error("Unknown saving throw: " + id);
            if (category != "skill" && category != "saving_throw" && !validLocalId(id))
                throw std::runtime_error("Proficiency ID must be a local identifier: " + id);
            continue;
        } else throw std::runtime_error("Unrecognized effect operation: " + op);
        auto* magnitude = effect.if_contains("value");
        if (!magnitude || !magnitude->is_int64() ||
            magnitude->as_int64() < -100 || magnitude->as_int64() > 100)
            throw std::runtime_error("Effect requires integer value between -100 and 100");
    }
}

void validateChoices(const json::object& object) {
    auto* value = object.if_contains("choices");
    if (!value) return;
    if (!value->is_array()) throw std::runtime_error("choices must be an array");
    std::set<std::string> keys;
    for (const auto& entry : value->as_array()) {
        if (!entry.is_object()) throw std::runtime_error("choice must be an object");
        const auto& choice = entry.as_object();
        const auto id = requiredString(choice, "id");
        if (!validLocalId(id) || !keys.insert(id).second)
            throw std::runtime_error("Invalid or duplicate choice ID: " + id);
        requiredString(choice, "label");
        auto* min = choice.if_contains("min");
        auto* max = choice.if_contains("max");
        auto* options = choice.if_contains("options");
        if (!min || !max || !min->is_int64() || !max->is_int64() ||
            min->as_int64() < 0 || max->as_int64() < min->as_int64() ||
            max->as_int64() > 32 || !options || !options->is_array() ||
            options->as_array().empty() || options->as_array().size() > 64 ||
            static_cast<std::size_t>(max->as_int64()) > options->as_array().size())
            throw std::runtime_error("Invalid choice cardinality/options: " + id);
        std::set<std::string> optionIds;
        for (const auto& optionValue : options->as_array()) {
            if (!optionValue.is_object()) throw std::runtime_error("option must be an object");
            const auto& option = optionValue.as_object();
            const auto oid = requiredString(option, "id");
            if (!validLocalId(oid) || !optionIds.insert(oid).second)
                throw std::runtime_error("Invalid or duplicate option ID: " + oid);
            requiredString(option, "label");
            validateEffects(option);
        }
    }
}

void validatePrerequisites(const json::object& obj) {
    auto* value = obj.if_contains("prerequisites");
    if (!value) return;
    if (!value->is_array()) throw std::runtime_error("prerequisites must be an array");
    for (const auto& pval : value->as_array()) {
        if (!pval.is_object()) throw std::runtime_error("prerequisite must be an object");
        const auto& p = pval.as_object();
        const auto type = requiredString(p, "type");
        if (type == "level" || type == "ability") {
            auto* min = p.if_contains("min");
            if (!min || !min->is_int64() || min->as_int64() < 1 || min->as_int64() > 30 ||
                (type == "level" && min->as_int64() > 20))
                throw std::runtime_error("Invalid prerequisite minimum");
            if (type == "ability") {
                auto ability = requiredString(p, "ability");
                if (std::find_if(abilityNames.begin(), abilityNames.end(),
                    [&](const char* a) { return ability == a; }) == abilityNames.end())
                    throw std::runtime_error("Invalid prerequisite ability");
            }
        } else if (type == "race" || type == "subrace" || type == "class" || type == "feat") {
            if (!validQualifiedId(requiredString(p, "id")))
                throw std::runtime_error("Invalid prerequisite ID");
        } else throw std::runtime_error("Unknown prerequisite: " + type);
    }
}

void validateProgression(const json::object& object) {
    auto* value = object.if_contains("progression");
    if (!value) return;
    if (!value->is_array()) throw std::runtime_error("progression must be an array");
    std::set<int> levels;
    for (const auto& entry : value->as_array()) {
        if (!entry.is_object()) throw std::runtime_error("progression step must be an object");
        const auto& step = entry.as_object();
        auto* at = step.if_contains("level");
        if (!at || !at->is_int64() || at->as_int64() < 1 || at->as_int64() > 20 ||
            !levels.insert(static_cast<int>(at->as_int64())).second)
            throw std::runtime_error("Invalid or duplicate progression level");
        for (const char* key : {"traits", "feats"})
            for (const auto& id : stringArray(step, key))
                if (!validQualifiedId(id)) throw std::runtime_error("Invalid progression reference: " + id);
        validateEffects(step);
        validateChoices(step);
    }
}

struct Package {
    fs::path path;
    std::string id;
    std::string ruleset;
    std::vector<std::string> requires;
};

}  // namespace

bool validLocalId(const std::string& id) {
    if (id.empty()) return false;
    return std::all_of(id.begin(), id.end(), [](unsigned char ch) {
        return (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
               ch == '_' || ch == '-';
    });
}

bool validQualifiedId(const std::string& id) {
    auto pos = id.find(':');
    return pos != std::string::npos && id.find(':', pos + 1) == std::string::npos &&
           validLocalId(id.substr(0, pos)) && validLocalId(id.substr(pos + 1));
}

void ContentRegistry::load(const fs::path& contentRoot) {
    loadMany({contentRoot});
}

void ContentRegistry::loadMany(const std::vector<fs::path>& contentRoots) {
    definitions_.clear();
    diagnostics_.clear();
    std::vector<fs::path> manifests;
    bool foundRoot = false;
    for (const auto& root : contentRoots) {
        if (!fs::exists(root) || !fs::is_directory(root)) continue;
        foundRoot = true;
        for (const auto& entry : fs::recursive_directory_iterator(root)) {
            if (entry.is_regular_file() && entry.path().filename() == "manifest.json")
                manifests.push_back(fs::weakly_canonical(entry.path()));
        }
    }
    if (!foundRoot) {
        diagnostics_.push_back({"Content", "No content directories found"});
        return;
    }
    std::sort(manifests.begin(), manifests.end());
    manifests.erase(std::unique(manifests.begin(), manifests.end()), manifests.end());

    std::map<std::string, Package> packages;
    std::set<fs::path> packageDirs;
    for (const auto& file : manifests) {
        try {
            auto obj = parseObject(file);
            auto schema = requiredString(obj, "schema_version");
            if (schema != "1.0") throw std::runtime_error("Unsupported schema_version: " + schema);
            auto id = requiredString(obj, "id");
            if (!validLocalId(id)) throw std::runtime_error("Invalid package ID: " + id);
            requiredString(obj, "name");
            requiredString(obj, "version");
            auto ruleset = requiredString(obj, "ruleset");
            if (ruleset != "5e-2014" && ruleset != "5e-2024")
                throw std::runtime_error("Unsupported ruleset: " + ruleset);
            auto requires = stringArray(obj, "requires");
            for (const auto& dep : requires) if (!validLocalId(dep) || dep == id)
                throw std::runtime_error("Invalid package dependency: " + dep);
            if (packages.count(id)) throw std::runtime_error("Duplicate package ID: " + id);
            packages.emplace(id, Package{file.parent_path(), id, ruleset, requires});
            packageDirs.insert(file.parent_path());
        } catch (const std::exception& e) {
            diagnostics_.push_back({file.string(), e.what()});
        }
    }

    // Packages with broken/missing dependencies cannot contribute definitions.
    bool changed;
    do {
        changed = false;
        for (auto it = packages.begin(); it != packages.end();) {
            auto missing = std::find_if(it->second.requires.begin(), it->second.requires.end(),
                [&](const std::string& dependency) {
                    auto dep = packages.find(dependency);
                    return dep == packages.end() || dep->second.ruleset != it->second.ruleset;
                });
            if (missing != it->second.requires.end()) {
                diagnostics_.push_back({it->second.path.string(),
                                        "Missing or incompatible dependency: " + *missing});
                it = packages.erase(it);
                changed = true;
            } else ++it;
        }
    } while (changed);

    // Detect dependency cycles (there are no runtime actions in package manifests).
    std::set<std::string> cyclic;
    std::map<std::string, int> state;
    std::vector<std::string> stack;
    auto visit = [&](auto&& self, const std::string& id) -> void {
        if (state[id] == 2) return;
        if (state[id] == 1) {
            auto start = std::find(stack.begin(), stack.end(), id);
            cyclic.insert(start, stack.end());
            return;
        }
        state[id] = 1;
        stack.push_back(id);
        for (const auto& dep : packages.at(id).requires) self(self, dep);
        stack.pop_back();
        state[id] = 2;
    };
    for (const auto& [id, _] : packages) visit(visit, id);
    for (const auto& id : cyclic) {
        diagnostics_.push_back({packages.at(id).path.string(), "Cyclic package dependency"});
        packages.erase(id);
    }
    do {
        changed = false;
        for (auto it = packages.begin(); it != packages.end();) {
            auto missing = std::find_if(it->second.requires.begin(), it->second.requires.end(),
                [&](const std::string& dep) { return !packages.count(dep); });
            if (missing != it->second.requires.end()) {
                diagnostics_.push_back({it->second.path.string(), "Unavailable dependency: " + *missing});
                it = packages.erase(it);
                changed = true;
            } else ++it;
        }
    } while (changed);

    for (const auto& [packageId, package] : packages) {
        std::vector<fs::path> files;
        for (fs::recursive_directory_iterator it(package.path), end; it != end; ++it) {
            const auto& entry = *it;
            if (entry.is_directory() && entry.path() != package.path &&
                packageDirs.count(entry.path())) {
                it.disable_recursion_pending();
                continue;
            }
            if (entry.is_regular_file() && entry.path().extension() == ".json" &&
                entry.path().filename() != "manifest.json") files.push_back(entry.path());
        }
        std::sort(files.begin(), files.end());
        for (const auto& file : files) {
            try {
                auto obj = parseObject(file);
                const auto id = requiredString(obj, "id");
                const auto kind = requiredString(obj, "kind");
                const auto name = requiredString(obj, "name");
                const auto edition = requiredString(obj, "ruleset");
                if (!validQualifiedId(id) || id.substr(0, id.find(':')) != packageId)
                    throw std::runtime_error("ID must be namespaced to package: " + packageId);
                if (edition != package.ruleset)
                    throw std::runtime_error("Ruleset does not match manifest: " + edition);
                if (kind != "race" && kind != "subrace" && kind != "trait" && kind != "feat" && kind != "class")
                    throw std::runtime_error("Unsupported definition kind: " + kind);
                if (kind == "subrace") requiredString(obj, "parent");
                if (kind == "race") {
                    const auto* speed = obj.if_contains("speed");
                    if (!speed || !speed->is_int64() || speed->as_int64() <= 0 || speed->as_int64() > 200)
                        throw std::runtime_error("Race requires positive integer speed");
                    requiredString(obj, "size");
                }
                for (const auto& ref : stringArray(obj, "traits")) {
                    if (!validQualifiedId(ref)) throw std::runtime_error("Invalid trait ID: " + ref);
                }
                for (const auto& ref : stringArray(obj, "feats")) {
                    if (!validQualifiedId(ref)) throw std::runtime_error("Invalid feat ID: " + ref);
                }
                validateEffects(obj);
                validateChoices(obj);
                validatePrerequisites(obj);
                validateProgression(obj);
                if (definitions_.count(id)) throw std::runtime_error("Duplicate definition ID: " + id);
                definitions_.emplace(id, Definition{id, kind, name, edition, packageId, std::move(obj)});
            } catch (const std::exception& e) {
                diagnostics_.push_back({file.string(), e.what()});
            }
        }
    }

    // Eliminate invalid cross references, and any dependents of removed definitions.
    do {
        changed = false;
        for (auto it = definitions_.begin(); it != definitions_.end();) {
            const auto& d = it->second;
            std::string error;
            try {
                const auto check = [&](const std::string& id, const std::string& expectedKind) {
                    if (!validQualifiedId(id)) throw std::runtime_error("Invalid reference: " + id);
                    const auto target = definitions_.find(id);
                    if (target == definitions_.end() || target->second.kind != expectedKind ||
                        target->second.ruleset != d.ruleset)
                        throw std::runtime_error("Missing or incompatible " + expectedKind + ": " + id);
                    const auto targetPackage = target->second.package;
                    if (targetPackage != d.package &&
                        std::find(packages.at(d.package).requires.begin(),
                                  packages.at(d.package).requires.end(), targetPackage) ==
                                  packages.at(d.package).requires.end())
                        throw std::runtime_error("Undeclared package dependency: " + targetPackage);
                };
                if (d.kind == "subrace") check(requiredString(d.data, "parent"), "race");
                for (const auto& ref : stringArray(d.data, "traits")) check(ref, "trait");
                for (const auto& ref : stringArray(d.data, "feats")) check(ref, "feat");
                if (auto* progression = d.data.if_contains("progression")) {
                    for (const auto& step : progression->as_array()) {
                        for (const auto& ref : stringArray(step.as_object(), "traits")) check(ref, "trait");
                        for (const auto& ref : stringArray(step.as_object(), "feats")) check(ref, "feat");
                    }
                }
                if (auto* prereqs = d.data.if_contains("prerequisites")) {
                    for (const auto& prereq : prereqs->as_array()) {
                        const auto& p = prereq.as_object();
                        const auto type = requiredString(p, "type");
                        if (type == "race" || type == "subrace" || type == "class" || type == "feat")
                            check(requiredString(p, "id"), type);
                    }
                }
            } catch (const std::exception& e) { error = e.what(); }
            if (!error.empty()) {
                diagnostics_.push_back({d.id, error});
                it = definitions_.erase(it);
                changed = true;
            } else ++it;
        }
    } while (changed);
}

const Definition* ContentRegistry::find(const std::string& id) const {
    auto it = definitions_.find(id);
    return it == definitions_.end() ? nullptr : &it->second;
}

std::vector<const Definition*> ContentRegistry::ofKind(const std::string& kind,
                                                        const std::string& ruleset) const {
    std::vector<const Definition*> out;
    for (const auto& [id, d] : definitions_)
        if (d.kind == kind && d.ruleset == ruleset) out.push_back(&d);
    return out;
}

}  // namespace dnd

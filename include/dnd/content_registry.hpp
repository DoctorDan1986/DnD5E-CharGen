#pragma once

#include <boost/json.hpp>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace dnd {

struct Diagnostic {
    std::string file;
    std::string message;
};

struct Definition {
    std::string id;
    std::string kind;
    std::string name;
    std::string ruleset;
    std::string package;
    boost::json::object data;
};

class ContentRegistry {
public:
    // Scan Content/ recursively. Broken files are rejected and diagnosed, not executed.
    void load(const std::filesystem::path& contentRoot);
    void loadMany(const std::vector<std::filesystem::path>& contentRoots);
    const Definition* find(const std::string& id) const;
    std::vector<const Definition*> ofKind(const std::string& kind,
                                           const std::string& ruleset) const;
    const std::vector<Diagnostic>& diagnostics() const { return diagnostics_; }
    std::size_t size() const { return definitions_.size(); }

private:
    std::map<std::string, Definition> definitions_;
    std::vector<Diagnostic> diagnostics_;
};

// Format requirement for module names and qualified definition IDs.
bool validLocalId(const std::string& id);
bool validQualifiedId(const std::string& id);

}  // namespace dnd

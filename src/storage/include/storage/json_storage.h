#pragma once

#include "common/types.h"
#include <nlohmann/json.hpp>
#include <string>

namespace providers_agent {
namespace storage {

class JsonStorage {
public:
    JsonStorage();
    ~JsonStorage();

    // Save JSON to file
    common::Status save(const std::string& filepath, const nlohmann::json& data);

    // Load JSON from file
    common::Status load(const std::string& filepath, nlohmann::json& data);

    // Check if file exists
    bool exists(const std::string& filepath);

    // Delete file
    common::Status remove(const std::string& filepath);

private:
    // TODO: Implement using fstream + nlohmann::json
};

} // namespace storage
} // namespace providers_agent

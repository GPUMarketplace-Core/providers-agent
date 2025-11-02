#include "storage/json_storage.h"

namespace providers_agent {
namespace storage {

// TODO: Implement using std::fstream and nlohmann::json
// - Save hardware.json
// - Load hardware.json
// - Handle file I/O errors

JsonStorage::JsonStorage() {
}

JsonStorage::~JsonStorage() {
}

common::Status JsonStorage::save(const std::string& filepath, const nlohmann::json& data) {
    // TODO: Write JSON to file using std::ofstream
    return common::Status::ERROR_NOT_INITIALIZED;
}

common::Status JsonStorage::load(const std::string& filepath, nlohmann::json& data) {
    // TODO: Read JSON from file using std::ifstream
    return common::Status::ERROR_NOT_INITIALIZED;
}

bool JsonStorage::exists(const std::string& filepath) {
    // TODO: Check if file exists
    return false;
}

common::Status JsonStorage::remove(const std::string& filepath) {
    // TODO: Delete file
    return common::Status::ERROR_NOT_INITIALIZED;
}

} // namespace storage
} // namespace providers_agent

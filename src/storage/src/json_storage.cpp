#include "storage/json_storage.h"
#include <fstream>
#include <filesystem>

namespace providers_agent {
namespace storage {

JsonStorage::JsonStorage() {
}

JsonStorage::~JsonStorage() {
}

common::Status JsonStorage::save(const std::string& filepath, const nlohmann::json& data) {
    try {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return common::Status::ERROR_FILE_IO;
        }
        
        file << data.dump(4); // Pretty print with 4-space indent
        file.close();
        
        if (file.fail()) {
            return common::Status::ERROR_FILE_IO;
        }
        
        return common::Status::OK;
    } catch (const std::exception&) {
        return common::Status::ERROR_FILE_IO;
    }
}

common::Status JsonStorage::load(const std::string& filepath, nlohmann::json& data) {
    try {
        if (!exists(filepath)) {
            return common::Status::ERROR_FILE_IO;
        }
        
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return common::Status::ERROR_FILE_IO;
        }
        
        file >> data;
        file.close();
        
        if (file.fail()) {
            return common::Status::ERROR_FILE_IO;
        }
        
        return common::Status::OK;
    } catch (const std::exception&) {
        return common::Status::ERROR_FILE_IO;
    }
}

bool JsonStorage::exists(const std::string& filepath) {
    try {
        return std::filesystem::exists(filepath) && 
               std::filesystem::is_regular_file(filepath);
    } catch (const std::exception&) {
        return false;
    }
}

common::Status JsonStorage::remove(const std::string& filepath) {
    try {
        if (!exists(filepath)) {
            return common::Status::OK; // Already doesn't exist
        }
        
        if (std::filesystem::remove(filepath)) {
            return common::Status::OK;
        } else {
            return common::Status::ERROR_FILE_IO;
        }
    } catch (const std::exception&) {
        return common::Status::ERROR_FILE_IO;
    }
}

} // namespace storage
} // namespace providers_agent

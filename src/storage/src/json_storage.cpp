#include "storage/json_storage.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace providers_agent {
namespace storage {

JsonStorage::JsonStorage() {
}

JsonStorage::~JsonStorage() {
}

common::Status JsonStorage::save(const std::string& filepath, const nlohmann::json& data) {
    try {
        // Create parent directory if it doesn't exist
        std::filesystem::path file_path(filepath);
        if (file_path.has_parent_path()) {
            std::filesystem::create_directories(file_path.parent_path());
        }

        // Open file for writing
        std::ofstream file(filepath, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filepath << std::endl;
            return common::Status::ERROR_FILE_IO;
        }

        // Write JSON with pretty formatting (4 spaces indentation)
        file << data.dump(4);
        
        if (file.fail()) {
            std::cerr << "Failed to write to file: " << filepath << std::endl;
            return common::Status::ERROR_FILE_IO;
        }

        file.close();
        return common::Status::OK;

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return common::Status::ERROR_FILE_IO;
    } catch (const std::exception& e) {
        std::cerr << "Error saving JSON: " << e.what() << std::endl;
        return common::Status::ERROR_FILE_IO;
    }
}

common::Status JsonStorage::load(const std::string& filepath, nlohmann::json& data) {
    try {
        // Check if file exists
        if (!std::filesystem::exists(filepath)) {
            std::cerr << "File does not exist: " << filepath << std::endl;
            return common::Status::ERROR_FILE_IO;
        }

        // Open file for reading
        std::ifstream file(filepath, std::ios::in);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading: " << filepath << std::endl;
            return common::Status::ERROR_FILE_IO;
        }

        // Parse JSON from file
        try {
            data = nlohmann::json::parse(file);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            file.close();
            return common::Status::ERROR_FILE_IO;
        }

        file.close();
        return common::Status::OK;

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return common::Status::ERROR_FILE_IO;
    } catch (const std::exception& e) {
        std::cerr << "Error loading JSON: " << e.what() << std::endl;
        return common::Status::ERROR_FILE_IO;
    }
}

bool JsonStorage::exists(const std::string& filepath) {
    try {
        return std::filesystem::exists(filepath);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error checking existence: " << e.what() << std::endl;
        return false;
    }
}

common::Status JsonStorage::remove(const std::string& filepath) {
    try {
        if (!std::filesystem::exists(filepath)) {
            // File doesn't exist, consider it successfully removed
            return common::Status::OK;
        }

        if (std::filesystem::remove(filepath)) {
            return common::Status::OK;
        } else {
            std::cerr << "Failed to remove file: " << filepath << std::endl;
            return common::Status::ERROR_FILE_IO;
        }

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return common::Status::ERROR_FILE_IO;
    } catch (const std::exception& e) {
        std::cerr << "Error removing file: " << e.what() << std::endl;
        return common::Status::ERROR_FILE_IO;
    }
}

} // namespace storage
} // namespace providers_agent

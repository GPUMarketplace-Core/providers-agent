#include <gtest/gtest.h>
#include "storage/json_storage.h"
#include <filesystem>
#include <fstream>

using namespace providers_agent::storage;
using namespace providers_agent::common;

class JsonStorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        storage = std::make_unique<JsonStorage>();
        test_dir = "./test_storage_temp";
        
        // Create test directory
        std::filesystem::create_directories(test_dir);
    }

    void TearDown() override {
        // Clean up test directory
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    std::unique_ptr<JsonStorage> storage;
    std::string test_dir;
};

// Test: Basic save and load operation
TEST_F(JsonStorageTest, BasicSaveAndLoad) {
    std::string filepath = test_dir + "/basic_test.json";
    
    // Create test JSON data
    nlohmann::json test_data = {
        {"name", "test"},
        {"value", 42},
        {"enabled", true}
    };

    // Save the data
    Status save_status = storage->save(filepath, test_data);
    EXPECT_EQ(save_status, Status::OK);
    EXPECT_TRUE(storage->exists(filepath));

    // Load the data back
    nlohmann::json loaded_data;
    Status load_status = storage->load(filepath, loaded_data);
    EXPECT_EQ(load_status, Status::OK);
    
    // Verify loaded data matches original
    EXPECT_EQ(loaded_data, test_data);
    EXPECT_EQ(loaded_data["name"], "test");
    EXPECT_EQ(loaded_data["value"], 42);
    EXPECT_EQ(loaded_data["enabled"], true);
}

// Test: Save complex nested JSON
TEST_F(JsonStorageTest, SaveComplexNestedJson) {
    std::string filepath = test_dir + "/complex_test.json";
    
    nlohmann::json complex_data = {
        {"user", {
            {"id", 123},
            {"name", "John Doe"},
            {"email", "john@example.com"}
        }},
        {"settings", {
            {"theme", "dark"},
            {"notifications", true},
            {"preferences", {
                {"language", "en"},
                {"timezone", "UTC"}
            }}
        }},
        {"tags", {"cpp", "json", "testing"}},
        {"metadata", {
            {"created", "2025-11-02"},
            {"version", 1.5}
        }}
    };

    Status save_status = storage->save(filepath, complex_data);
    EXPECT_EQ(save_status, Status::OK);

    nlohmann::json loaded_data;
    Status load_status = storage->load(filepath, loaded_data);
    EXPECT_EQ(load_status, Status::OK);
    
    EXPECT_EQ(loaded_data, complex_data);
    EXPECT_EQ(loaded_data["user"]["name"], "John Doe");
    EXPECT_EQ(loaded_data["settings"]["preferences"]["language"], "en");
    EXPECT_EQ(loaded_data["tags"][0], "cpp");
}

// Test: Save to nested directory that doesn't exist
TEST_F(JsonStorageTest, SaveToNestedDirectory) {
    std::string nested_path = test_dir + "/level1/level2/level3/nested_test.json";
    
    nlohmann::json test_data = {
        {"message", "nested directory test"}
    };

    Status save_status = storage->save(nested_path, test_data);
    EXPECT_EQ(save_status, Status::OK);
    EXPECT_TRUE(storage->exists(nested_path));

    nlohmann::json loaded_data;
    Status load_status = storage->load(nested_path, loaded_data);
    EXPECT_EQ(load_status, Status::OK);
    EXPECT_EQ(loaded_data, test_data);
}

// Test: Load from non-existent file
TEST_F(JsonStorageTest, LoadNonExistentFile) {
    std::string filepath = test_dir + "/nonexistent.json";
    
    nlohmann::json loaded_data;
    Status load_status = storage->load(filepath, loaded_data);
    EXPECT_EQ(load_status, Status::ERROR_FILE_IO);
}

// Test: Load invalid JSON
TEST_F(JsonStorageTest, LoadInvalidJson) {
    std::string filepath = test_dir + "/invalid.json";
    
    // Write invalid JSON manually
    std::ofstream file(filepath);
    file << "{ invalid json content ][}";
    file.close();

    nlohmann::json loaded_data;
    Status load_status = storage->load(filepath, loaded_data);
    EXPECT_EQ(load_status, Status::ERROR_FILE_IO);
}

// Test: File exists check
TEST_F(JsonStorageTest, FileExistsCheck) {
    std::string existing_file = test_dir + "/existing.json";
    std::string nonexistent_file = test_dir + "/nonexistent.json";
    
    // Create a file
    nlohmann::json test_data = {{"test", "data"}};
    storage->save(existing_file, test_data);

    // Check existence
    EXPECT_TRUE(storage->exists(existing_file));
    EXPECT_FALSE(storage->exists(nonexistent_file));
}

// Test: Remove file
TEST_F(JsonStorageTest, RemoveFile) {
    std::string filepath = test_dir + "/to_remove.json";
    
    // Create a file
    nlohmann::json test_data = {{"test", "data"}};
    storage->save(filepath, test_data);
    EXPECT_TRUE(storage->exists(filepath));

    // Remove the file
    Status remove_status = storage->remove(filepath);
    EXPECT_EQ(remove_status, Status::OK);
    EXPECT_FALSE(storage->exists(filepath));
}

// Test: Remove non-existent file (should succeed)
TEST_F(JsonStorageTest, RemoveNonExistentFile) {
    std::string filepath = test_dir + "/does_not_exist.json";
    
    EXPECT_FALSE(storage->exists(filepath));
    Status remove_status = storage->remove(filepath);
    EXPECT_EQ(remove_status, Status::OK);
}

// Test: Save empty JSON object
TEST_F(JsonStorageTest, SaveEmptyJson) {
    std::string filepath = test_dir + "/empty.json";
    
    nlohmann::json empty_data = nlohmann::json::object();
    Status save_status = storage->save(filepath, empty_data);
    EXPECT_EQ(save_status, Status::OK);

    nlohmann::json loaded_data;
    Status load_status = storage->load(filepath, loaded_data);
    EXPECT_EQ(load_status, Status::OK);
    EXPECT_TRUE(loaded_data.is_object());
    EXPECT_TRUE(loaded_data.empty());
}

// Test: Save empty JSON array
TEST_F(JsonStorageTest, SaveEmptyArray) {
    std::string filepath = test_dir + "/empty_array.json";
    
    nlohmann::json empty_array = nlohmann::json::array();
    Status save_status = storage->save(filepath, empty_array);
    EXPECT_EQ(save_status, Status::OK);

    nlohmann::json loaded_data;
    Status load_status = storage->load(filepath, loaded_data);
    EXPECT_EQ(load_status, Status::OK);
    EXPECT_TRUE(loaded_data.is_array());
    EXPECT_TRUE(loaded_data.empty());
}

// Test: Save with various data types
TEST_F(JsonStorageTest, SaveVariousDataTypes) {
    std::string filepath = test_dir + "/data_types.json";
    
    nlohmann::json test_data = {
        {"string", "hello world"},
        {"integer", 42},
        {"negative", -100},
        {"floating", 3.14159},
        {"boolean_true", true},
        {"boolean_false", false},
        {"null_value", nullptr},
        {"array", {1, 2, 3, 4, 5}},
        {"nested_object", {
            {"key", "value"}
        }}
    };

    Status save_status = storage->save(filepath, test_data);
    EXPECT_EQ(save_status, Status::OK);

    nlohmann::json loaded_data;
    Status load_status = storage->load(filepath, loaded_data);
    EXPECT_EQ(load_status, Status::OK);
    
    EXPECT_EQ(loaded_data["string"], "hello world");
    EXPECT_EQ(loaded_data["integer"], 42);
    EXPECT_EQ(loaded_data["negative"], -100);
    EXPECT_DOUBLE_EQ(loaded_data["floating"], 3.14159);
    EXPECT_EQ(loaded_data["boolean_true"], true);
    EXPECT_EQ(loaded_data["boolean_false"], false);
    EXPECT_TRUE(loaded_data["null_value"].is_null());
    EXPECT_EQ(loaded_data["array"].size(), 5);
    EXPECT_EQ(loaded_data["nested_object"]["key"], "value");
}

// Test: Overwrite existing file
TEST_F(JsonStorageTest, OverwriteExistingFile) {
    std::string filepath = test_dir + "/overwrite.json";
    
    // Save initial data
    nlohmann::json initial_data = {{"version", 1}};
    storage->save(filepath, initial_data);

    nlohmann::json loaded_data;
    storage->load(filepath, loaded_data);
    EXPECT_EQ(loaded_data["version"], 1);

    // Overwrite with new data
    nlohmann::json new_data = {{"version", 2}, {"updated", true}};
    Status save_status = storage->save(filepath, new_data);
    EXPECT_EQ(save_status, Status::OK);

    // Verify new data
    storage->load(filepath, loaded_data);
    EXPECT_EQ(loaded_data["version"], 2);
    EXPECT_EQ(loaded_data["updated"], true);
    EXPECT_FALSE(loaded_data.contains("version") && loaded_data.size() == 1);
}

// Test: Save with special characters
TEST_F(JsonStorageTest, SaveWithSpecialCharacters) {
    std::string filepath = test_dir + "/special_chars.json";
    
    nlohmann::json test_data = {
        {"text", "Hello \"World\" with 'quotes'"},
        {"newline", "Line1\nLine2"},
        {"tab", "Column1\tColumn2"},
        {"unicode", "Hello 世界 🌍"},
        {"backslash", "C:\\Users\\Test"}
    };

    Status save_status = storage->save(filepath, test_data);
    EXPECT_EQ(save_status, Status::OK);

    nlohmann::json loaded_data;
    Status load_status = storage->load(filepath, loaded_data);
    EXPECT_EQ(load_status, Status::OK);
    EXPECT_EQ(loaded_data, test_data);
}

// Test: Large JSON data
TEST_F(JsonStorageTest, SaveLargeJson) {
    std::string filepath = test_dir + "/large.json";
    
    nlohmann::json large_data = nlohmann::json::array();
    for (int i = 0; i < 1000; ++i) {
        large_data.push_back({
            {"id", i},
            {"name", "Item " + std::to_string(i)},
            {"value", i * 1.5},
            {"active", i % 2 == 0}
        });
    }

    Status save_status = storage->save(filepath, large_data);
    EXPECT_EQ(save_status, Status::OK);

    nlohmann::json loaded_data;
    Status load_status = storage->load(filepath, loaded_data);
    EXPECT_EQ(load_status, Status::OK);
    EXPECT_EQ(loaded_data.size(), 1000);
    EXPECT_EQ(loaded_data[500]["id"], 500);
    EXPECT_EQ(loaded_data[999]["name"], "Item 999");
}

// Test: Multiple operations in sequence
TEST_F(JsonStorageTest, MultipleOperationsSequence) {
    std::string filepath = test_dir + "/sequence.json";
    
    // Save
    nlohmann::json data1 = {{"step", 1}};
    EXPECT_EQ(storage->save(filepath, data1), Status::OK);
    EXPECT_TRUE(storage->exists(filepath));
    
    // Load
    nlohmann::json loaded;
    EXPECT_EQ(storage->load(filepath, loaded), Status::OK);
    EXPECT_EQ(loaded["step"], 1);
    
    // Update
    nlohmann::json data2 = {{"step", 2}};
    EXPECT_EQ(storage->save(filepath, data2), Status::OK);
    
    // Load again
    EXPECT_EQ(storage->load(filepath, loaded), Status::OK);
    EXPECT_EQ(loaded["step"], 2);
    
    // Remove
    EXPECT_EQ(storage->remove(filepath), Status::OK);
    EXPECT_FALSE(storage->exists(filepath));
    
    // Try to load removed file
    EXPECT_EQ(storage->load(filepath, loaded), Status::ERROR_FILE_IO);
}

// Test: Empty string filepath handling
TEST_F(JsonStorageTest, EmptyFilepath) {
    std::string empty_path = "";
    nlohmann::json test_data = {{"test", "data"}};
    
    // These should fail gracefully
    Status save_status = storage->save(empty_path, test_data);
    EXPECT_EQ(save_status, Status::ERROR_FILE_IO);
    
    nlohmann::json loaded_data;
    Status load_status = storage->load(empty_path, loaded_data);
    EXPECT_EQ(load_status, Status::ERROR_FILE_IO);
}


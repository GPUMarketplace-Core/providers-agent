#include <iostream>
#include <string>
#include "credential_manager/credential_manager.h"

using namespace providers_agent;

void printStatus(const std::string& operation, common::Status status) {
    std::cout << operation << ": ";
    if (status == common::Status::OK) {
        std::cout << "✓ SUCCESS" << std::endl;
    } else {
        std::cout << "✗ FAILED" << std::endl;
    }
}

int main() {
    std::cout << "=== Credential Manager Test ===" << std::endl;
    
#ifdef _WIN32
    std::cout << "Platform: Windows (using DPAPI + Credential Manager)" << std::endl;
#else
    std::cout << "Platform: macOS/Linux (using in-memory test storage)" << std::endl;
    std::cout << "Note: Production macOS would use Keychain Services" << std::endl;
#endif
    
    credential_manager::CredentialManager manager;
    
    // Test 1: Store credentials
    std::cout << "\n[Test 1] Storing credentials..." << std::endl;
    
    auto status = manager.storeCredential("api_token", "sk-test-1234567890abcdef");
    printStatus("Store API token", status);
    
    status = manager.storeCredential("api_key", "my-secret-api-key-xyz");
    printStatus("Store API key", status);
    
    status = manager.storeCredential("oauth_token", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9");
    printStatus("Store OAuth token", status);
    
    // Test 2: Check if credentials exist
    std::cout << "\n[Test 2] Checking credential existence..." << std::endl;
    
    bool exists = manager.hasCredential("api_token");
    std::cout << "api_token exists: " << (exists ? "✓ YES" : "✗ NO") << std::endl;
    
    exists = manager.hasCredential("non_existent_key");
    std::cout << "non_existent_key exists: " << (exists ? "✗ YES (ERROR)" : "✓ NO (correct)") << std::endl;
    
    // Test 3: Retrieve credentials
    std::cout << "\n[Test 3] Retrieving credentials..." << std::endl;
    
    std::string value;
    status = manager.retrieveCredential("api_token", value);
    if (status == common::Status::OK) {
        std::cout << "✓ Retrieved api_token: " << value << std::endl;
    } else {
        std::cout << "✗ Failed to retrieve api_token" << std::endl;
    }
    
    status = manager.retrieveCredential("api_key", value);
    if (status == common::Status::OK) {
        std::cout << "✓ Retrieved api_key: " << value << std::endl;
    } else {
        std::cout << "✗ Failed to retrieve api_key" << std::endl;
    }
    
    status = manager.retrieveCredential("oauth_token", value);
    if (status == common::Status::OK) {
        std::cout << "✓ Retrieved oauth_token: " << value << std::endl;
    } else {
        std::cout << "✗ Failed to retrieve oauth_token" << std::endl;
    }
    
    // Test 4: Try to retrieve non-existent credential
    std::cout << "\n[Test 4] Retrieving non-existent credential..." << std::endl;
    status = manager.retrieveCredential("non_existent_key", value);
    if (status != common::Status::OK) {
        std::cout << "✓ Correctly failed to retrieve non-existent credential" << std::endl;
    } else {
        std::cout << "✗ Should have failed but didn't" << std::endl;
    }
    
    // Test 5: Update existing credential
    std::cout << "\n[Test 5] Updating existing credential..." << std::endl;
    status = manager.storeCredential("api_token", "sk-test-updated-token-999");
    printStatus("Update api_token", status);
    
    status = manager.retrieveCredential("api_token", value);
    if (status == common::Status::OK) {
        std::cout << "✓ Retrieved updated api_token: " << value << std::endl;
    }
    
    // Test 6: Delete credentials
    std::cout << "\n[Test 6] Deleting credentials..." << std::endl;
    
    status = manager.deleteCredential("api_key");
    printStatus("Delete api_key", status);
    
    exists = manager.hasCredential("api_key");
    std::cout << "api_key still exists after delete: " << (exists ? "✗ YES (ERROR)" : "✓ NO (correct)") << std::endl;
    
    // Test 7: Cleanup - delete remaining credentials
    std::cout << "\n[Test 7] Cleanup..." << std::endl;
    manager.deleteCredential("api_token");
    manager.deleteCredential("oauth_token");
    std::cout << "✓ Cleaned up test credentials" << std::endl;
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    
#ifdef _WIN32
    std::cout << "\nNote: You can verify credentials in Windows Credential Manager:" << std::endl;
    std::cout << "Control Panel > User Accounts > Credential Manager > Windows Credentials" << std::endl;
    std::cout << "Look for entries starting with 'ProvidersAgent:'" << std::endl;
#endif
    
    return 0;
}

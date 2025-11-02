#include "credential_manager/credential_manager.h"
#include <memory>
#include <map>
#include <mutex>

#ifdef _WIN32
    #include <windows.h>
    #include <wincred.h>
    #include <dpapi.h>
    #pragma comment(lib, "advapi32.lib")
    #pragma comment(lib, "crypt32.lib")
#else
    // For macOS testing - simple in-memory storage
    // In production macOS, you would use Keychain Services
    #include <vector>
#endif

namespace providers_agent {
namespace credential_manager {

class CredentialManager::Impl {
public:
    std::mutex mutex;
    
#ifndef _WIN32
    // Simple in-memory storage for testing on macOS
    // Production would use Keychain Services API
    std::map<std::string, std::string> test_storage;
#endif

    Impl() {}
    ~Impl() {}

#ifdef _WIN32
    // Windows Implementation using DPAPI
    common::Status storeDPAPI(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mutex);

        // Prepare data to encrypt
        DATA_BLOB dataIn;
        dataIn.pbData = (BYTE*)value.data();
        dataIn.cbData = static_cast<DWORD>(value.size());

        // Optional entropy for additional security
        DATA_BLOB entropy;
        std::string entropyStr = "ProvidersAgent-Salt-2025";
        entropy.pbData = (BYTE*)entropyStr.data();
        entropy.cbData = static_cast<DWORD>(entropyStr.size());

        DATA_BLOB dataOut;
        std::wstring description = L"Providers Agent Credential";

        // Encrypt data
        if (!CryptProtectData(
            &dataIn,
            description.c_str(),
            &entropy,
            NULL,
            NULL,
            CRYPTPROTECT_UI_FORBIDDEN,
            &dataOut)) {
            return common::Status::ERROR_GENERAL;
        }

        // Store encrypted data in Windows Credential Manager
        std::wstring wkey(key.begin(), key.end());
        std::wstring targetName = L"ProvidersAgent:" + wkey;

        CREDENTIALW cred = {0};
        cred.Type = CRED_TYPE_GENERIC;
        cred.TargetName = (LPWSTR)targetName.c_str();
        cred.CredentialBlobSize = dataOut.cbData;
        cred.CredentialBlob = dataOut.pbData;
        cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
        cred.UserName = (LPWSTR)L"ProvidersAgent";
        cred.Comment = (LPWSTR)L"Encrypted API Token";

        BOOL result = CredWriteW(&cred, 0);
        
        // Free encrypted data
        LocalFree(dataOut.pbData);

        return result ? common::Status::OK : common::Status::ERROR_GENERAL;
    }

    common::Status retrieveDPAPI(const std::string& key, std::string& value) {
        std::lock_guard<std::mutex> lock(mutex);

        std::wstring wkey(key.begin(), key.end());
        std::wstring targetName = L"ProvidersAgent:" + wkey;

        PCREDENTIALW pcred = NULL;
        if (!CredReadW(targetName.c_str(), CRED_TYPE_GENERIC, 0, &pcred)) {
            return common::Status::ERROR_GENERAL;
        }

        // Prepare encrypted data
        DATA_BLOB dataIn;
        dataIn.pbData = pcred->CredentialBlob;
        dataIn.cbData = pcred->CredentialBlobSize;

        // Entropy (must match what was used during encryption)
        DATA_BLOB entropy;
        std::string entropyStr = "ProvidersAgent-Salt-2025";
        entropy.pbData = (BYTE*)entropyStr.data();
        entropy.cbData = static_cast<DWORD>(entropyStr.size());

        DATA_BLOB dataOut;
        LPWSTR description = NULL;

        // Decrypt data
        BOOL result = CryptUnprotectData(
            &dataIn,
            &description,
            &entropy,
            NULL,
            NULL,
            CRYPTPROTECT_UI_FORBIDDEN,
            &dataOut);

        if (result) {
            value.assign((char*)dataOut.pbData, dataOut.cbData);
            LocalFree(dataOut.pbData);
        }

        if (description) {
            LocalFree(description);
        }

        CredFree(pcred);

        return result ? common::Status::OK : common::Status::ERROR_GENERAL;
    }

    common::Status deleteDPAPI(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex);

        std::wstring wkey(key.begin(), key.end());
        std::wstring targetName = L"ProvidersAgent:" + wkey;

        BOOL result = CredDeleteW(targetName.c_str(), CRED_TYPE_GENERIC, 0);
        return result ? common::Status::OK : common::Status::ERROR_GENERAL;
    }

    bool hasDPAPI(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex);

        std::wstring wkey(key.begin(), key.end());
        std::wstring targetName = L"ProvidersAgent:" + wkey;

        PCREDENTIALW pcred = NULL;
        BOOL result = CredReadW(targetName.c_str(), CRED_TYPE_GENERIC, 0, &pcred);
        
        if (result && pcred) {
            CredFree(pcred);
            return true;
        }
        
        return false;
    }
#else
    // macOS/Linux test implementation (in-memory)
    // Production macOS would use Security.framework Keychain Services:
    // - SecKeychainAddGenericPassword
    // - SecKeychainFindGenericPassword
    // - SecKeychainItemDelete
    
    common::Status storeTest(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mutex);
        test_storage[key] = value;
        return common::Status::OK;
    }

    common::Status retrieveTest(const std::string& key, std::string& value) {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = test_storage.find(key);
        if (it != test_storage.end()) {
            value = it->second;
            return common::Status::OK;
        }
        return common::Status::ERROR_GENERAL;
    }

    common::Status deleteTest(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = test_storage.find(key);
        if (it != test_storage.end()) {
            test_storage.erase(it);
            return common::Status::OK;
        }
        return common::Status::ERROR_GENERAL;
    }

    bool hasTest(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex);
        return test_storage.find(key) != test_storage.end();
    }
#endif
};

CredentialManager::CredentialManager() : pImpl(new Impl()) {
}

CredentialManager::~CredentialManager() {
    delete pImpl;
}

common::Status CredentialManager::storeCredential(
    const std::string& key,
    const std::string& value
) {
#ifdef _WIN32
    return pImpl->storeDPAPI(key, value);
#else
    return pImpl->storeTest(key, value);
#endif
}

common::Status CredentialManager::retrieveCredential(
    const std::string& key,
    std::string& value
) {
#ifdef _WIN32
    return pImpl->retrieveDPAPI(key, value);
#else
    return pImpl->retrieveTest(key, value);
#endif
}

common::Status CredentialManager::deleteCredential(const std::string& key) {
#ifdef _WIN32
    return pImpl->deleteDPAPI(key);
#else
    return pImpl->deleteTest(key);
#endif
}

bool CredentialManager::hasCredential(const std::string& key) {
#ifdef _WIN32
    return pImpl->hasDPAPI(key);
#else
    return pImpl->hasTest(key);
#endif
}

} // namespace credential_manager
} // namespace providers_agent

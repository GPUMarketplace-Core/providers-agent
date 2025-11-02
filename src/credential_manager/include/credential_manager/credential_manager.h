#pragma once

#include "common/types.h"
#include <string>

namespace providers_agent {
namespace credential_manager {

class CredentialManager {
public:
    CredentialManager();
    ~CredentialManager();

    // Store credential securely
    common::Status storeCredential(
        const std::string& key,
        const std::string& value
    );

    // Retrieve credential
    common::Status retrieveCredential(
        const std::string& key,
        std::string& value
    );

    // Delete credential
    common::Status deleteCredential(const std::string& key);

    // Check if credential exists
    bool hasCredential(const std::string& key);

private:
    // TODO: Implement using Windows Credential Manager / DPAPI
    class Impl;
    Impl* pImpl;
};

} // namespace credential_manager
} // namespace providers_agent

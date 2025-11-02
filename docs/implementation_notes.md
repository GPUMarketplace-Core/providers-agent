# Implementation Notes

## Heartbeat Client

### Overview
The heartbeat client sends periodic HTTP POST requests with JSON payloads containing timestamps to a specified endpoint.

### Implementation Details

#### Windows (Production)
- **Technology**: WinHTTP API
- **Features**:
  - Native Windows HTTP client
  - No external dependencies
  - Supports HTTP and HTTPS
  - Configurable interval (default: 10 seconds)
  - Background thread for periodic execution
  - Callback support for status notifications

#### macOS/Linux (Testing)
- **Technology**: libcurl
- **Purpose**: Development and testing on non-Windows platforms
- **Note**: The Windows version will be used in production

### JSON Payload Format
```json
{
  "timestamp": "2025-11-02T04:08:56.123Z",
  "status": "alive",
  "agent_id": "provider-agent-001"
}
```

### Usage Example
```cpp
#include "heartbeat/heartbeat_client.h"

// Create client with endpoint
heartbeat::HeartbeatClient client("https://api.example.com/heartbeat");

// Set custom interval (optional, default is 10s)
client.setInterval(10);

// Start periodic heartbeat with callback
auto status = client.start([](common::Status status) {
    if (status == common::Status::OK) {
        std::cout << "Heartbeat sent successfully" << std::endl;
    }
});

// Or send single heartbeat
status = client.sendHeartbeat();

// Stop when done
client.stop();
```

### Testing
Use the provided test program:
```bash
./build/bin/test_heartbeat
```

For local testing, you can use:
- **httpbin.org**: `https://httpbin.org/post`
- **Local mock server**: `http://localhost:3000/heartbeat`

---

## Credential Manager

### Overview
Secure storage for API tokens and credentials using OS-level encryption.

### Implementation Details

#### Windows (Production)
- **Technology**: Windows DPAPI (Data Protection API) + Credential Manager
- **Features**:
  - Double-layer security:
    1. DPAPI encrypts the credential data
    2. Credential Manager stores encrypted data
  - Encryption tied to user account
  - Protected from other users on same machine
  - Survives application restarts
  - Native Windows integration

#### Encryption Flow (Windows)
1. **Store**:
   - Original credential → DPAPI encryption with entropy
   - Encrypted blob → Windows Credential Manager
   - Stored as `CRED_TYPE_GENERIC` with prefix `ProvidersAgent:`

2. **Retrieve**:
   - Read from Credential Manager by target name
   - Decrypt using DPAPI with same entropy
   - Return original credential

#### macOS (Testing)
- **Technology**: In-memory map (for development)
- **Production**: Would use Keychain Services API:
  - `SecKeychainAddGenericPassword`
  - `SecKeychainFindGenericPassword`
  - `SecKeychainItemDelete`

### Security Features
- **Entropy**: Additional salt for DPAPI encryption
- **User-specific**: Credentials only accessible by the user who stored them
- **Machine-bound**: Encrypted credentials tied to specific machine
- **No plaintext**: Credentials never stored in plaintext

### Usage Example
```cpp
#include "credential_manager/credential_manager.h"

credential_manager::CredentialManager manager;

// Store credential
auto status = manager.storeCredential("api_token", "sk-1234567890");

// Check if exists
bool exists = manager.hasCredential("api_token");

// Retrieve credential
std::string token;
status = manager.retrieveCredential("api_token", token);

// Delete credential
status = manager.deleteCredential("api_token");
```

### Testing
Use the provided test program:
```bash
./build/bin/test_credentials
```

### Verification (Windows)
You can verify stored credentials in Windows:
1. Open Control Panel
2. Go to User Accounts → Credential Manager
3. Click "Windows Credentials"
4. Look for entries starting with "ProvidersAgent:"

### Security Considerations

#### Best Practices
- ✓ Never log or print credentials
- ✓ Always check return status
- ✓ Delete credentials when no longer needed
- ✓ Use unique keys for different credentials

#### Limitations
- Credentials are user-specific (not shared across users)
- Encrypted credentials cannot be transferred to other machines
- Requires user to be logged in for access
- On macOS test version, credentials lost on restart

---

## Building on macOS

Since you're developing on macOS but targeting Windows, here's how to build and test:

### Prerequisites
```bash
# Install libcurl (for heartbeat testing)
brew install curl

# CMake and build tools
brew install cmake
```

### Build Commands
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Run Tests
```bash
# Test heartbeat client
./bin/test_heartbeat

# Test credential manager
./bin/test_credentials
```

---

## Windows Build Instructions

For your team members on Windows:

### Prerequisites
- Visual Studio 2019/2022 with C++ tools
- CMake 3.20+
- NVIDIA CUDA Toolkit (optional, for GPU module)

### Build Commands
```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Run Tests
```cmd
.\build\bin\Release\test_heartbeat.exe
.\build\bin\Release\test_credentials.exe
```

---

## Integration with Main Application

Both modules are ready to integrate into the main application:

```cpp
#include "heartbeat/heartbeat_client.h"
#include "credential_manager/credential_manager.h"

int main() {
    // Initialize credential manager
    credential_manager::CredentialManager creds;
    
    // Retrieve API token (or prompt user to enter)
    std::string api_token;
    if (creds.hasCredential("api_token")) {
        creds.retrieveCredential("api_token", api_token);
    } else {
        // First time - get from user and store
        std::cout << "Enter API token: ";
        std::cin >> api_token;
        creds.storeCredential("api_token", api_token);
    }
    
    // Start heartbeat client
    heartbeat::HeartbeatClient heartbeat("https://api.example.com/heartbeat");
    heartbeat.start([](common::Status status) {
        // Log heartbeat status
    });
    
    // Run application...
    
    // Cleanup
    heartbeat.stop();
    
    return 0;
}
```

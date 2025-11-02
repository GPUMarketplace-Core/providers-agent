#pragma once

#include "common/types.h"
#include <string>
#include <functional>

namespace providers_agent {
namespace heartbeat {

using HeartbeatCallback = std::function<void(common::Status)>;

class HeartbeatClient {
public:
    HeartbeatClient(const std::string& endpoint_url);
    ~HeartbeatClient();

    // Start heartbeat loop (every 10 seconds)
    common::Status start(HeartbeatCallback callback = nullptr);

    // Stop heartbeat loop
    void stop();

    // Send single heartbeat
    common::Status sendHeartbeat();

    // Set custom interval (default 10s)
    void setInterval(uint32_t seconds);

private:
    // TODO: Implement HTTP POST using WinHTTP or curl
    class Impl;
    Impl* pImpl;
};

} // namespace heartbeat
} // namespace providers_agent

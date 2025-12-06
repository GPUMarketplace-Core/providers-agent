#pragma once

#include "common/types.h"
#include <string>
#include <functional>
#include <vector>
#include <map>

namespace providers_agent {
namespace heartbeat {

using HeartbeatCallback = std::function<void(common::Status)>;

// GPU rate information for heartbeat
struct GPURateInfo {
    uint32_t gpu_index;
    uint64_t hourly_price_cents;  // Price in cents per hour
};

class HeartbeatClient {
public:
    // Constructor: reads HEART_BEAT_API_ENDPOINT from environment
    // Constructs endpoint as: {HEART_BEAT_API_ENDPOINT}/api/nodes/{nodeId}/heartbeat
    HeartbeatClient(const std::string& nodeId, const std::string& oauthToken);
    ~HeartbeatClient();

    // Start heartbeat loop (every 30 seconds by default)
    common::Status start(HeartbeatCallback callback = nullptr);

    // Stop heartbeat loop
    void stop();

    // Send single heartbeat
    common::Status sendHeartbeat();

    // Set custom interval (default 30s)
    void setInterval(uint32_t seconds);

    // Update GPU/rate information for heartbeat payload
    void updateGPURates(const std::vector<GPURateInfo>& gpuRates);

    // Update GPU metrics (from GPU probe)
    void updateGPUMetrics(const std::vector<common::GPUInfo>& gpuMetrics);

    // Update System Info (CPU/Memory)
    void updateSystemInfo(const common::SystemInfo& sysInfo);

private:
    class Impl;
    Impl* pImpl;
};

} // namespace heartbeat
} // namespace providers_agent

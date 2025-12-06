#pragma once

#include "common/types.h"
#include <string>
#include <functional>
#include <vector>

namespace providers_agent {
namespace job_runner {

class JobManager {
public:
    JobManager(const std::string& nodeId, const std::string& oauthToken);
    ~JobManager();

    // Start polling loop
    common::Status start();

    // Stop polling loop
    void stop();

    // Set custom polling interval (default 10s)
    void setInterval(uint32_t seconds);

    // Set callback for log messages
    void setLogCallback(std::function<void(const std::string&)> callback);

private:
    class Impl;
    Impl* pImpl;
};

} // namespace job_runner
} // namespace providers_agent

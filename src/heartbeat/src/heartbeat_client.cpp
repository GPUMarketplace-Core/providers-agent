#include <iostream>
#include "heartbeat/heartbeat_client.h"
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <ctime>
#include <cstdlib>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
    #include <winhttp.h>
    #pragma comment(lib, "winhttp.lib")
    // Avoid conflicts with Windows macros
    #ifdef ERROR_INVALID_PARAMETER
        #undef ERROR_INVALID_PARAMETER
    #endif
#else
    // For macOS/Linux testing, use curl
    #include <curl/curl.h>
    #include <cstring>
#endif

namespace providers_agent {
namespace heartbeat {

class HeartbeatClient::Impl {
public:
    std::string endpoint_url;
    std::string node_id;
    std::string oauth_token;
    uint32_t interval_seconds;
    std::atomic<bool> running;
    std::thread worker_thread;
    HeartbeatCallback callback;
    std::mutex mutex;
    std::vector<GPURateInfo> gpu_rates;
    std::vector<common::GPUInfo> gpu_metrics;
    common::SystemInfo system_info;
    bool has_system_info = false;

    Impl(const std::string& nodeId, const std::string& token) 
        : node_id(nodeId)
        , oauth_token(token)
        , interval_seconds(30)  // Default 30 seconds
        , running(false) {
        
        // Read HEART_BEAT_API_ENDPOINT from environment
        const char* env_endpoint = "http://localhost:3000";
        if (!env_endpoint || strlen(env_endpoint) == 0) {
            // Fallback or error - for now we'll construct a default
            // In production, this should probably fail or use a config file
            endpoint_url = "http://localhost:3000";
        } else {
            // Construct full endpoint URL: {HEART_BEAT_API_ENDPOINT}/api/nodes/{nodeId}/heartbeat
            std::ostringstream url_stream;
            std::string base_url = env_endpoint;
            // Ensure no trailing slash in base URL
            if (!base_url.empty() && base_url.back() == '/') {
                base_url.pop_back();
            }
            url_stream << base_url << "/api/nodes/" << nodeId << "/heartbeat";
            endpoint_url = url_stream.str();
        }

#ifndef _WIN32
        curl_global_init(CURL_GLOBAL_DEFAULT);
#endif
    }

    ~Impl() {
#ifndef _WIN32
        curl_global_cleanup();
#endif
    }

    nlohmann::json createHeartbeatPayload() {
        nlohmann::json payload;
        
        // Status field (default to "online")
        payload["status"] = "online";
        
        // Metrics object with GPU information
        nlohmann::json metrics = nlohmann::json::object();
        
        // Add GPU rates if available
        if (!gpu_rates.empty()) {
            nlohmann::json gpu_rates_json = nlohmann::json::array();
            for (const auto& rate : gpu_rates) {
                gpu_rates_json.push_back({
                    {"gpu_index", rate.gpu_index},
                    {"hourly_price_cents", rate.hourly_price_cents}
                });
            }
            metrics["gpu_rates"] = gpu_rates_json;
        }
        
        // Add GPU metrics if available
        if (!gpu_metrics.empty()) {
            nlohmann::json gpu_metrics_json = nlohmann::json::array();
            for (const auto& gpu : gpu_metrics) {
                nlohmann::json gpu_json = {
                    {"name", gpu.name},
                    {"vram_total_bytes", gpu.vram_total_bytes},
                    {"vram_used_bytes", gpu.vram_used_bytes},
                    {"temperature_celsius", gpu.temperature_celsius},
                    {"utilization_percent", gpu.utilization_percent}
                };
                gpu_metrics_json.push_back(gpu_json);
            }
            metrics["gpus"] = gpu_metrics_json;
        }

        // Add System Info (CPU/Memory/OS) if available
        if (has_system_info) {
            metrics["cpu"] = {
                {"vendor", system_info.cpu.vendor},
                {"model", system_info.cpu.model},
                {"cores", system_info.cpu.cores},
                {"logical_processors", system_info.cpu.logical_processors},
                {"frequency_mhz", system_info.cpu.frequency_mhz}
            };
            
            metrics["memory"] = {
                {"total_bytes", system_info.memory.total_bytes},
                {"available_bytes", system_info.memory.available_bytes},
                {"usage_percent", system_info.memory.usage_percent}
            };

            metrics["os"] = {
                {"name", system_info.os.name},
                {"version", system_info.os.version},
                {"architecture", system_info.os.architecture},
                {"hostname", system_info.os.hostname}
            };
        }
        
        // Only include metrics if it has content
        if (!metrics.empty()) {
            payload["metrics"] = metrics;
        }
        
        return payload;
    }

#ifdef _WIN32
    common::Status sendHeartbeatWinHTTP() {
        // Check if endpoint URL is valid
        if (endpoint_url.empty()) {
            return common::Status::ERROR_INVALID_PARAMETER;
        }
        
        // Parse URL
        std::wstring wurl(endpoint_url.begin(), endpoint_url.end());
        URL_COMPONENTS urlComp = {0};
        urlComp.dwStructSize = sizeof(urlComp);
        
        WCHAR hostName[256] = {0};
        WCHAR urlPath[1024] = {0};
        urlComp.lpszHostName = hostName;
        urlComp.dwHostNameLength = sizeof(hostName) / sizeof(WCHAR);
        urlComp.lpszUrlPath = urlPath;
        urlComp.dwUrlPathLength = sizeof(urlPath) / sizeof(WCHAR);

        if (!WinHttpCrackUrl(wurl.c_str(), 0, 0, &urlComp)) {
            return common::Status::ERROR_NETWORK;
        }

        // Create JSON payload
        auto payload = createHeartbeatPayload();
        std::string json_data = payload.dump();

        // Initialize WinHTTP
        HINTERNET hSession = WinHttpOpen(
            L"ProvidersAgent/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0);

        if (!hSession) {
            std::cerr << "WinHttpOpen failed with error: " << GetLastError() << std::endl;
            return common::Status::ERROR_NETWORK;
        }

        HINTERNET hConnect = WinHttpConnect(
            hSession,
            hostName,
            urlComp.nPort,
            0);

        if (!hConnect) {
            std::cerr << "WinHttpConnect failed with error: " << GetLastError() << std::endl;
            WinHttpCloseHandle(hSession);
            return common::Status::ERROR_NETWORK;
        }

        DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(
            hConnect,
            L"POST",
            urlPath,
            NULL,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            flags);

        if (!hRequest) {
            std::cerr << "WinHttpOpenRequest failed with error: " << GetLastError() << std::endl;
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return common::Status::ERROR_NETWORK;
        }

        // Set headers: Content-Type and Authorization
        std::wstring auth_header = L"Authorization: Bearer " + std::wstring(oauth_token.begin(), oauth_token.end()) + L"\r\n";
        std::wstring content_type = L"Content-Type: application/json\r\n";
        std::wstring headers = auth_header + content_type;
        
        WinHttpAddRequestHeaders(
            hRequest,
            headers.c_str(),
            -1L,
            WINHTTP_ADDREQ_FLAG_ADD);

        // Send request
        BOOL result = WinHttpSendRequest(
            hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            (LPVOID)json_data.c_str(),
            json_data.length(),
            json_data.length(),
            0);

        if (!result) {
            std::cerr << "WinHttpSendRequest failed with error: " << GetLastError() << std::endl;
        }

        common::Status status = common::Status::ERROR_NETWORK;
        if (result && WinHttpReceiveResponse(hRequest, NULL)) {
            DWORD statusCode = 0;
            DWORD statusCodeSize = sizeof(statusCode);
            WinHttpQueryHeaders(
                hRequest,
                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                NULL,
                &statusCode,
                &statusCodeSize,
                NULL);

            if (statusCode >= 200 && statusCode < 300) {
                status = common::Status::OK;
            } else {
                std::cerr << "Heartbeat HTTP Error: " << statusCode << std::endl;
            }
        } else if (result) {
            std::cerr << "WinHttpReceiveResponse failed with error: " << GetLastError() << std::endl;
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        return status;
    }
#else
    // macOS/Linux implementation using libcurl for testing
    common::Status sendHeartbeatCurl() {
        // Check if endpoint URL is valid
        if (endpoint_url.empty()) {
            return common::Status::ERROR_INVALID_PARAMETER;
        }
        
        CURL* curl = curl_easy_init();
        if (!curl) {
            return common::Status::ERROR_NETWORK;
        }

        auto payload = createHeartbeatPayload();
        std::string json_data = payload.dump();

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string auth_header = "Authorization: Bearer " + oauth_token;
        headers = curl_slist_append(headers, auth_header.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, endpoint_url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

        CURLcode res = curl_easy_perform(curl);
        
        common::Status status = common::Status::ERROR_NETWORK;
        if (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code >= 200 && response_code < 300) {
                status = common::Status::OK;
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return status;
    }
#endif

    void heartbeatLoop() {
        while (running.load()) {
            common::Status status;
            
#ifdef _WIN32
            status = sendHeartbeatWinHTTP();
#else
            status = sendHeartbeatCurl();
#endif

            if (callback) {
                callback(status);
            }

            // Wait for interval or until stopped
            auto start = std::chrono::steady_clock::now();
            while (running.load()) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count();
                
                if (elapsed >= (interval_seconds * 1000)) {
                    break;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
};

HeartbeatClient::HeartbeatClient(const std::string& nodeId, const std::string& oauthToken) 
    : pImpl(new Impl(nodeId, oauthToken)) {
}

HeartbeatClient::~HeartbeatClient() {
    stop();
    delete pImpl;
}

common::Status HeartbeatClient::start(HeartbeatCallback callback) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    if (pImpl->running.load()) {
        return common::Status::ERROR_GENERAL; // Already running
    }

    pImpl->callback = callback;
    pImpl->running.store(true);
    pImpl->worker_thread = std::thread(&Impl::heartbeatLoop, pImpl);

    return common::Status::OK;
}

void HeartbeatClient::stop() {
    pImpl->running.store(false);
    if (pImpl->worker_thread.joinable()) {
        pImpl->worker_thread.join();
    }
}

common::Status HeartbeatClient::sendHeartbeat() {
#ifdef _WIN32
    return pImpl->sendHeartbeatWinHTTP();
#else
    return pImpl->sendHeartbeatCurl();
#endif
}

void HeartbeatClient::setInterval(uint32_t seconds) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->interval_seconds = seconds;
}

void HeartbeatClient::updateGPURates(const std::vector<GPURateInfo>& gpuRates) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->gpu_rates = gpuRates;
}

void HeartbeatClient::updateGPUMetrics(const std::vector<common::GPUInfo>& gpuMetrics) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->gpu_metrics = gpuMetrics;
}

void HeartbeatClient::updateSystemInfo(const common::SystemInfo& sysInfo) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->system_info = sysInfo;
    pImpl->has_system_info = true;
}

} // namespace heartbeat
} // namespace providers_agent

#include "heartbeat/heartbeat_client.h"
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <ctime>

#ifdef _WIN32
    #include <windows.h>
    #include <winhttp.h>
    #pragma comment(lib, "winhttp.lib")
#else
    // For macOS/Linux testing, use curl
    #include <curl/curl.h>
#endif

namespace providers_agent {
namespace heartbeat {

class HeartbeatClient::Impl {
public:
    std::string endpoint_url;
    uint32_t interval_seconds;
    std::atomic<bool> running;
    std::thread worker_thread;
    HeartbeatCallback callback;
    std::mutex mutex;

    Impl(const std::string& url) 
        : endpoint_url(url)
        , interval_seconds(10)
        , running(false) {
#ifndef _WIN32
        curl_global_init(CURL_GLOBAL_DEFAULT);
#endif
    }

    ~Impl() {
#ifndef _WIN32
        curl_global_cleanup();
#endif
    }

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        char buffer[64];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", std::gmtime(&time_t_now));
        return std::string(buffer) + "." + std::to_string(ms.count()) + "Z";
    }

    nlohmann::json createHeartbeatPayload() {
        return {
            {"timestamp", getCurrentTimestamp()},
            {"status", "alive"},
            {"agent_id", "provider-agent-001"}
        };
    }

#ifdef _WIN32
    common::Status sendHeartbeatWinHTTP() {
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
            return common::Status::ERROR_NETWORK;
        }

        HINTERNET hConnect = WinHttpConnect(
            hSession,
            hostName,
            urlComp.nPort,
            0);

        if (!hConnect) {
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
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return common::Status::ERROR_NETWORK;
        }

        // Set headers
        std::wstring headers = L"Content-Type: application/json\r\n";
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
            }
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        return status;
    }
#else
    // macOS/Linux implementation using libcurl for testing
    common::Status sendHeartbeatCurl() {
        CURL* curl = curl_easy_init();
        if (!curl) {
            return common::Status::ERROR_NETWORK;
        }

        auto payload = createHeartbeatPayload();
        std::string json_data = payload.dump();

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

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
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - start).count();
                
                if (elapsed >= interval_seconds) {
                    break;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
};

HeartbeatClient::HeartbeatClient(const std::string& endpoint_url) 
    : pImpl(new Impl(endpoint_url)) {
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

} // namespace heartbeat
} // namespace providers_agent

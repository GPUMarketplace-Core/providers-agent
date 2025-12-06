#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include "heartbeat/heartbeat_client.h"

using namespace providers_agent;

int main() {
    std::cout << "=== Heartbeat Client Test ===" << std::endl;
    
    // Check for required environment variable
    const char* api_endpoint = "http://localhost:3000";
    if (!api_endpoint) {
        std::cerr << "Error: HEART_BEAT_API_ENDPOINT environment variable not set" << std::endl;
        std::cerr << "Example: export HEART_BEAT_API_ENDPOINT=https://api.example.com" << std::endl;
        return 1;
    }
    
    // Node ID and OAuth token
    std::string nodeId;
    std::string oauthToken;

    std::cout << "Enter Node ID: ";
    std::cin >> nodeId;
    
    std::cout << "Enter OAuth Token: ";
    std::cin >> oauthToken;
    
    std::cout << "Creating heartbeat client..." << std::endl;
    std::cout << "API Endpoint: " << api_endpoint << std::endl;
    std::cout << "Node ID: " << nodeId << std::endl;
    std::cout << "Full URL: " << api_endpoint << "/api/nodes/" << nodeId << "/heartbeat" << std::endl;
    
    heartbeat::HeartbeatClient client(nodeId, oauthToken);
    
    // Example: Add GPU rate information
    std::vector<heartbeat::GPURateInfo> gpuRates = {
        {0, 5000},  // GPU 0: $50.00/hour (5000 cents)
        {1, 7500}   // GPU 1: $75.00/hour (7500 cents)
    };
    client.updateGPURates(gpuRates);
    
    // Test 1: Single heartbeat
    std::cout << "\n[Test 1] Sending single heartbeat..." << std::endl;
    auto status = client.sendHeartbeat();
    if (status == common::Status::OK) {
        std::cout << "✓ Heartbeat sent successfully!" << std::endl;
    } else {
        std::cout << "✗ Failed to send heartbeat" << std::endl;
    }
    
    // Test 2: Periodic heartbeat with callback (default is 30 seconds)
    std::cout << "\n[Test 2] Starting periodic heartbeat (every 30 seconds)..." << std::endl;
    std::cout << "Note: Using default 30s interval. Use setInterval() to change." << std::endl;
    
    int heartbeat_count = 0;
    // client.setInterval(5); // Uncomment to test with 5 seconds instead
    
    auto callback = [&heartbeat_count](common::Status status) {
        heartbeat_count++;
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::cout << "[" << std::ctime(&time_t);
        std::cout << "] Heartbeat #" << heartbeat_count << " - ";
        
        if (status == common::Status::OK) {
            std::cout << "✓ SUCCESS" << std::endl;
        } else {
            std::cout << "✗ FAILED" << std::endl;
        }
    };
    
    status = client.start(callback);
    if (status != common::Status::OK) {
        std::cerr << "Failed to start heartbeat client" << std::endl;
        return 1;
    }
    
    std::cout << "Heartbeat client running... (will run for 90 seconds to see multiple heartbeats)" << std::endl;
    std::cout << "Press Ctrl+C to stop early" << std::endl;
    
    // Run indefinitely until interrupted
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // Unreachable unless logic changes
    std::cout << "\nStopping heartbeat client..." << std::endl;
    client.stop();
    
    std::cout << "Total heartbeats sent: " << heartbeat_count << std::endl;
    std::cout << "\n=== Test Complete ===" << std::endl;
    
    return 0;
}

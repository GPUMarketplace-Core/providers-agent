#include <iostream>
#include <chrono>
#include <thread>
#include "heartbeat/heartbeat_client.h"

using namespace providers_agent;

int main() {
    std::cout << "=== Heartbeat Client Test ===" << std::endl;
    
    // Use a mock HTTP endpoint (replace with your test server)
    // For testing, you can use: https://httpbin.org/post
    // Or run a local mock server
    std::string endpoint = "https://httpbin.org/post";
    
    std::cout << "Creating heartbeat client..." << std::endl;
    std::cout << "Endpoint: " << endpoint << std::endl;
    
    heartbeat::HeartbeatClient client(endpoint);
    
    // Test 1: Single heartbeat
    std::cout << "\n[Test 1] Sending single heartbeat..." << std::endl;
    auto status = client.sendHeartbeat();
    if (status == common::Status::OK) {
        std::cout << "✓ Heartbeat sent successfully!" << std::endl;
    } else {
        std::cout << "✗ Failed to send heartbeat" << std::endl;
    }
    
    // Test 2: Periodic heartbeat with callback
    std::cout << "\n[Test 2] Starting periodic heartbeat (every 5 seconds)..." << std::endl;
    
    int heartbeat_count = 0;
    client.setInterval(5); // 5 seconds for testing
    
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
    
    std::cout << "Heartbeat client running... (will run for 20 seconds)" << std::endl;
    std::cout << "Press Ctrl+C to stop early" << std::endl;
    
    // Run for 20 seconds (should send ~4 heartbeats)
    std::this_thread::sleep_for(std::chrono::seconds(20));
    
    std::cout << "\nStopping heartbeat client..." << std::endl;
    client.stop();
    
    std::cout << "Total heartbeats sent: " << heartbeat_count << std::endl;
    std::cout << "\n=== Test Complete ===" << std::endl;
    
    return 0;
}

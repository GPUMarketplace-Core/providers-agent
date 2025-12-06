#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <QApplication>
#include "system_probe/system_probe.h"
#include "gpu_probe/gpu_probe.h"
#include "heartbeat/heartbeat_client.h"
#include "credential_manager/credential_manager.h"
#include "storage/json_storage.h"
#include "gui/main_window.h"
#include "job_runner/job_runner.h"
#include "job_runner/job_manager.h"

using namespace providers_agent;

int main(int argc, char* argv[]) {
    // Initialize Qt Application
    QApplication app(argc, argv);
    
    std::cout << "Providers Agent Starting..." << std::endl;

    // Test GPU Probe
    gpu_probe::GPUProbe gpu_probe;
    auto status = gpu_probe.initialize();
    if (status == common::Status::OK) {
        std::cout << "GPU Probe initialized successfully" << std::endl;
        
        // Log GPU info to file
        status = gpu_probe.logToFile("gpu_info.json");
        if (status == common::Status::OK) {
            std::cout << "GPU info logged to gpu_info.json" << std::endl;
        } else {
            std::cout << "Failed to log GPU info" << std::endl;
        }
    } else {
        std::cout << "Failed to initialize GPU Probe" << std::endl;
    }

    // Initialize JobRunner to check for Blender
    job_runner::JobRunner jobRunner;
    // Check for required environment variable
    const char* api_endpoint = "http://localhost:3000";
    if (!api_endpoint) {
        std::cerr << "Error: HEART_BEAT_API_ENDPOINT environment variable not set" << std::endl;
        std::cerr << "Example: export HEART_BEAT_API_ENDPOINT=https://api.example.com" << std::endl;
        return 1;
    }

    // Initialize Credential Manager
    credential_manager::CredentialManager credManager;
    
    // Retrieve or use demo token
    std::string oauthToken;
    bool usingDemoToken = false;
    
    if (credManager.hasCredential("oauth_token")) {
        auto status = credManager.retrieveCredential("oauth_token", oauthToken);
        if (status != common::Status::OK) {
            std::cerr << "Failed to retrieve OAuth token from credential manager" << std::endl;
            return 1;
        }
        std::cout << "✓ Retrieved OAuth token from credential manager" << std::endl;
    } else {
        // Prompt for token
        std::cout << "\n=== Authentication Required ===" << std::endl;
        std::cout << "1. Go to " << api_endpoint << " and login/signup as a Provider." << std::endl;
        std::cout << "2. Copy your authentication token (from the dashboard or profile)." << std::endl;
        std::cout << "3. Paste the token below:" << std::endl;
        std::cout << "> ";
        
        std::cin >> oauthToken;
        
        if (!oauthToken.empty()) {
            std::cout << "✓ Token received!" << std::endl;
            credManager.storeCredential("oauth_token", oauthToken);
        } else {
            std::cerr << "✗ No token provided." << std::endl;
            return 1;
        }
    }

    // Initialize System Probe
    system_probe::SystemProbe systemProbe;
    systemProbe.initialize();

    // Node ID configuration (Auto-generated from Machine ID)
    std::string nodeId = systemProbe.getUniqueIdentifier();
    std::cout << "Using Node ID (Hardware Fixed): " << nodeId << std::endl;
    
    // Initialize Heartbeat Client
    heartbeat::HeartbeatClient heartbeatClient(nodeId, oauthToken);
    // Set heartbeat interval to 2 seconds
    heartbeatClient.setInterval(2);

    // Initialize Job Manager
    job_runner::JobManager jobManager(nodeId, oauthToken);
    jobManager.setInterval(5); // Poll every 5 seconds
    
    // TODO: Initialize GPU Probe when implemented
    // gpu_probe::GPUProbe gpuProbe;
    
    // TODO: Get GPU rates from configuration or user input
    // Example GPU rates (in cents per hour)
    std::vector<heartbeat::GPURateInfo> gpuRates = {
        {0, 5000},  // GPU 0: $50.00/hour
        {1, 7500}   // GPU 1: $75.00/hour
    };
    heartbeatClient.updateGPURates(gpuRates);
    std::cout << "✓ Configured GPU rates" << std::endl;

    // TODO: Update GPU metrics when GPU probe is implemented
    // std::vector<common::GPUInfo> gpuMetrics;
    // gpuProbe.getGPUInfo(gpuMetrics);
    // heartbeatClient.updateGPUMetrics(gpuMetrics);

    // Initialize GUI
    gui::MainWindow mainWindow;
    mainWindow.initialize();
    
    // Connect logger
    jobManager.setLogCallback([&mainWindow](const std::string& msg) {
        mainWindow.addLog(msg);
        std::cout << msg << std::endl;
    });

    // Setup GUI callbacks
    int heartbeatCount = 0;
    bool heartbeatRunning = false;
    
    // Update connection status (initially disconnected since Go Online is OFF)
    mainWindow.updateConnectionStatus(false, "", "v1.2.3", 0);
    
    // Load and display system information
    common::SystemInfo sysInfo;
    if (systemProbe.getSystemInfo(sysInfo) == common::Status::OK) {
        std::ostringstream cpuInfo, memoryInfo;
        cpuInfo << "Vendor: " << sysInfo.cpu.vendor << "\n";
        cpuInfo << "Cores: " << sysInfo.cpu.cores << "\n";
        cpuInfo << "Logical Processors: " << sysInfo.cpu.logical_processors << "\n";
        if (sysInfo.cpu.frequency_mhz > 0) {
            cpuInfo << "Frequency: " << sysInfo.cpu.frequency_mhz << " MHz";
        } else {
            cpuInfo << "Frequency: Unknown";
        }
        
        double totalGB = sysInfo.memory.total_bytes / (1024.0 * 1024.0 * 1024.0);
        double availableGB = sysInfo.memory.available_bytes / (1024.0 * 1024.0 * 1024.0);
        double usedGB = totalGB - availableGB;
        
        memoryInfo << "Total: " << std::fixed << std::setprecision(2) << totalGB << " GB\n";
        memoryInfo << "Available: " << availableGB << " GB\n";
        memoryInfo << "Used: " << usedGB << " GB\n";
        memoryInfo << "Usage: " << sysInfo.memory.usage_percent << "%";
        
        mainWindow.updateSystemInfoDisplay(cpuInfo.str(), memoryInfo.str());
        mainWindow.addLog("System information loaded");

        // Pass initial system info to heartbeat client
        heartbeatClient.updateSystemInfo(sysInfo);
    } else {
        mainWindow.updateSystemInfoDisplay("Failed to load CPU information", "Failed to load memory information");
        mainWindow.addLog("Failed to load system information");
    }
    
    // Go Online toggle callback - controls heartbeat
    mainWindow.onGoOnlineChanged = [&](bool goOnline) {
        if (goOnline) {
            // Start heartbeat
            auto heartbeatCallback = [&](common::Status status) {
                heartbeatCount++;
                std::string message;
                bool success = false;
                
                if (status == common::Status::OK) {
                    success = true;
                    // message = "Heartbeat #" + std::to_string(heartbeatCount) + " - SUCCESS";
                    // std::cout << "✓ " << message << std::endl;
                } else {
                    message = "Heartbeat #" + std::to_string(heartbeatCount) + " - FAILED (Status: " + std::to_string(static_cast<int>(status)) + ")";
                    success = false;
                    std::cerr << "✗ " << message << std::endl;
                }
                
                mainWindow.updateHeartbeatStatus(true, heartbeatCount, message, success);
                mainWindow.updateConnectionStatus(true, "", "v1.2.3", 0); // Reset sync time on successful heartbeat
            };
            
            auto status = heartbeatClient.start(heartbeatCallback);
            if (status == common::Status::OK) {
                heartbeatRunning = true;
                heartbeatCount = 0;
                mainWindow.updateHeartbeatStatus(true, 0, "Heartbeat started", true);
                mainWindow.updateConnectionStatus(true, "", "v1.2.3", 0);
                mainWindow.addLog("Go Online: ENABLED - Heartbeat service started");
                std::cout << "✓ Go Online: Heartbeat service started" << std::endl;

                // Start Job Manager
                jobManager.start();
                mainWindow.addLog("Job Manager started");
            } else {
                mainWindow.updateHeartbeatStatus(false, 0, "Failed to start heartbeat", false);
                mainWindow.addLog("Go Online: FAILED to start heartbeat service");
                mainWindow.setGoOnline(false);  // Revert toggle
                std::cerr << "✗ Failed to start heartbeat service" << std::endl;
            }
        } else {
            // Stop heartbeat
            heartbeatClient.stop();
            heartbeatRunning = false;
            heartbeatCount = 0;
            mainWindow.updateHeartbeatStatus(false, 0, "Heartbeat stopped", true);
            mainWindow.updateConnectionStatus(false, "", "v1.2.3", 0);
            mainWindow.addLog("Go Online: DISABLED - Heartbeat service stopped");
            std::cout << "✓ Go Online: Heartbeat service stopped" << std::endl;

            // Stop Job Manager
            jobManager.stop();
            mainWindow.addLog("Job Manager stopped");
        }
    };
    
    // Show GUI
    mainWindow.show();
    
    std::cout << "✓ Providers Agent GUI Ready" << std::endl;
    std::cout << "ℹ️  Toggle 'Go Online' to start sending heartbeats" << std::endl;
    std::cout << "📊 View dashboard at: http://localhost:3000" << std::endl;

    // Run Qt event loop
    return app.exec();
}

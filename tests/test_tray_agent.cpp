#include <gtest/gtest.h>
#include "tray_agent/tray_agent.h"
#include <thread>
#include <chrono>

using namespace providers_agent::tray_agent;
using namespace providers_agent::common;

class TrayAgentTest : public ::testing::Test {
protected:
    void SetUp() override {
        agent = std::make_unique<TrayAgent>();
    }

    void TearDown() override {
        agent.reset();
    }

    std::unique_ptr<TrayAgent> agent;
};

// Test: Constructor and Destructor
TEST_F(TrayAgentTest, ConstructorDestructor) {
    // Should not crash
    TrayAgent temp_agent;
    SUCCEED();
}

// Test: Initialize
TEST_F(TrayAgentTest, Initialize) {
    Status status = agent->initialize();
    
#ifdef _WIN32
    // On Windows, initialization should succeed
    EXPECT_EQ(status, Status::OK);
#else
    // On non-Windows platforms, should return error
    EXPECT_EQ(status, Status::ERROR_NOT_INITIALIZED);
#endif
}

// Test: Double initialization
TEST_F(TrayAgentTest, DoubleInitialization) {
#ifdef _WIN32
    Status first_init = agent->initialize();
    EXPECT_EQ(first_init, Status::OK);
    
    // Second initialization should also succeed (idempotent)
    Status second_init = agent->initialize();
    EXPECT_EQ(second_init, Status::OK);
#else
    GTEST_SKIP() << "Test only runs on Windows";
#endif
}

// Test: Set status to Online
TEST_F(TrayAgentTest, SetStatusOnline) {
#ifdef _WIN32
    agent->initialize();
    
    // Should not crash
    agent->setStatus(AgentStatus::ONLINE);
    SUCCEED();
#else
    // Should not crash even on non-Windows
    agent->setStatus(AgentStatus::ONLINE);
    SUCCEED();
#endif
}

// Test: Set status to Offline
TEST_F(TrayAgentTest, SetStatusOffline) {
#ifdef _WIN32
    agent->initialize();
    
    agent->setStatus(AgentStatus::OFFLINE);
    SUCCEED();
#else
    agent->setStatus(AgentStatus::OFFLINE);
    SUCCEED();
#endif
}

// Test: Set status to Busy
TEST_F(TrayAgentTest, SetStatusBusy) {
#ifdef _WIN32
    agent->initialize();
    
    agent->setStatus(AgentStatus::BUSY);
    SUCCEED();
#else
    agent->setStatus(AgentStatus::BUSY);
    SUCCEED();
#endif
}

// Test: Set status without initialization
TEST_F(TrayAgentTest, SetStatusWithoutInitialization) {
    // Should not crash even when called before initialize
    agent->setStatus(AgentStatus::ONLINE);
    agent->setStatus(AgentStatus::OFFLINE);
    agent->setStatus(AgentStatus::BUSY);
    SUCCEED();
}

// Test: Status transitions
TEST_F(TrayAgentTest, StatusTransitions) {
#ifdef _WIN32
    agent->initialize();
    
    // Transition through all states
    agent->setStatus(AgentStatus::OFFLINE);
    agent->setStatus(AgentStatus::ONLINE);
    agent->setStatus(AgentStatus::BUSY);
    agent->setStatus(AgentStatus::OFFLINE);
    
    SUCCEED();
#else
    GTEST_SKIP() << "Test only runs on Windows";
#endif
}

// Test: Show notification with basic message
TEST_F(TrayAgentTest, ShowNotificationBasic) {
#ifdef _WIN32
    agent->initialize();
    
    // Should not crash
    agent->showNotification("Test Title", "Test Message");
    SUCCEED();
#else
    // Should not crash even on non-Windows
    agent->showNotification("Test Title", "Test Message");
    SUCCEED();
#endif
}

// Test: Show notification without initialization
TEST_F(TrayAgentTest, ShowNotificationWithoutInitialization) {
    // Should not crash even when called before initialize
    agent->showNotification("Test", "Message");
    SUCCEED();
}

// Test: Show notification with empty strings
TEST_F(TrayAgentTest, ShowNotificationEmpty) {
#ifdef _WIN32
    agent->initialize();
    
    agent->showNotification("", "");
    SUCCEED();
#else
    agent->showNotification("", "");
    SUCCEED();
#endif
}

// Test: Show notification with long strings
TEST_F(TrayAgentTest, ShowNotificationLongStrings) {
#ifdef _WIN32
    agent->initialize();
    
    std::string long_title(100, 'A');
    std::string long_message(500, 'B');
    
    agent->showNotification(long_title, long_message);
    SUCCEED();
#else
    std::string long_title(100, 'A');
    std::string long_message(500, 'B');
    agent->showNotification(long_title, long_message);
    SUCCEED();
#endif
}

// Test: Show notification with special characters
TEST_F(TrayAgentTest, ShowNotificationSpecialCharacters) {
#ifdef _WIN32
    agent->initialize();
    
    agent->showNotification("Test™", "Message with émojis 🚀 and ñ special chars!");
    SUCCEED();
#else
    agent->showNotification("Test™", "Message with émojis 🚀 and ñ special chars!");
    SUCCEED();
#endif
}

// Test: Multiple notifications in sequence
TEST_F(TrayAgentTest, MultipleNotifications) {
#ifdef _WIN32
    agent->initialize();
    
    agent->showNotification("Notification 1", "First message");
    agent->showNotification("Notification 2", "Second message");
    agent->showNotification("Notification 3", "Third message");
    
    SUCCEED();
#else
    agent->showNotification("Notification 1", "First message");
    agent->showNotification("Notification 2", "Second message");
    agent->showNotification("Notification 3", "Third message");
    SUCCEED();
#endif
}

// Test: Exec without initialization
TEST_F(TrayAgentTest, ExecWithoutInitialization) {
    int result = agent->exec();
    
    // Should return error code
    EXPECT_EQ(result, -1);
}

// Test: Exec on non-Windows platform
TEST_F(TrayAgentTest, ExecNonWindows) {
#ifndef _WIN32
    agent->initialize();
    int result = agent->exec();
    
    // Should return error code on non-Windows
    EXPECT_EQ(result, -1);
#else
    GTEST_SKIP() << "Test only runs on non-Windows platforms";
#endif
}

// Test: Complete initialization and cleanup cycle
TEST_F(TrayAgentTest, InitializationCleanupCycle) {
#ifdef _WIN32
    // Initialize
    Status init_status = agent->initialize();
    EXPECT_EQ(init_status, Status::OK);
    
    // Set status
    agent->setStatus(AgentStatus::ONLINE);
    
    // Show notification
    agent->showNotification("Test", "Cleanup test");
    
    // Destroy (cleanup happens in destructor)
    agent.reset();
    
    // Recreate
    agent = std::make_unique<TrayAgent>();
    init_status = agent->initialize();
    EXPECT_EQ(init_status, Status::OK);
#else
    GTEST_SKIP() << "Test only runs on Windows";
#endif
}

// Test: Rapid status changes
TEST_F(TrayAgentTest, RapidStatusChanges) {
#ifdef _WIN32
    agent->initialize();
    
    // Rapidly change status multiple times
    for (int i = 0; i < 100; ++i) {
        agent->setStatus(AgentStatus::ONLINE);
        agent->setStatus(AgentStatus::OFFLINE);
        agent->setStatus(AgentStatus::BUSY);
    }
    
    SUCCEED();
#else
    GTEST_SKIP() << "Test only runs on Windows";
#endif
}

// Test: Notification with newlines
TEST_F(TrayAgentTest, NotificationWithNewlines) {
#ifdef _WIN32
    agent->initialize();
    
    agent->showNotification("Title\nWith\nNewlines", "Message\nAlso\nWith\nNewlines");
    SUCCEED();
#else
    agent->showNotification("Title\nWith\nNewlines", "Message\nAlso\nWith\nNewlines");
    SUCCEED();
#endif
}

// Test: Notification with tabs
TEST_F(TrayAgentTest, NotificationWithTabs) {
#ifdef _WIN32
    agent->initialize();
    
    agent->showNotification("Title\tWith\tTabs", "Message\tWith\tTabs");
    SUCCEED();
#else
    agent->showNotification("Title\tWith\tTabs", "Message\tWith\tTabs");
    SUCCEED();
#endif
}

// Test: Mixed operations
TEST_F(TrayAgentTest, MixedOperations) {
#ifdef _WIN32
    agent->initialize();
    
    // Mix different operations
    agent->setStatus(AgentStatus::ONLINE);
    agent->showNotification("Status", "Agent is now online");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    agent->setStatus(AgentStatus::BUSY);
    agent->showNotification("Status", "Agent is now busy");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    agent->setStatus(AgentStatus::OFFLINE);
    agent->showNotification("Status", "Agent is now offline");
    
    SUCCEED();
#else
    GTEST_SKIP() << "Test only runs on Windows";
#endif
}

// Test: Multiple agent instances
TEST_F(TrayAgentTest, MultipleInstances) {
#ifdef _WIN32
    auto agent2 = std::make_unique<TrayAgent>();
    auto agent3 = std::make_unique<TrayAgent>();
    
    Status init1 = agent->initialize();
    Status init2 = agent2->initialize();
    Status init3 = agent3->initialize();
    
    EXPECT_EQ(init1, Status::OK);
    EXPECT_EQ(init2, Status::OK);
    EXPECT_EQ(init3, Status::OK);
    
    agent->setStatus(AgentStatus::ONLINE);
    agent2->setStatus(AgentStatus::OFFLINE);
    agent3->setStatus(AgentStatus::BUSY);
    
    agent3.reset();
    agent2.reset();
    
    SUCCEED();
#else
    GTEST_SKIP() << "Test only runs on Windows";
#endif
}

// Test: Status setting after cleanup and reinit
TEST_F(TrayAgentTest, StatusAfterReinit) {
#ifdef _WIN32
    agent->initialize();
    agent->setStatus(AgentStatus::ONLINE);
    
    // Cleanup
    agent.reset();
    
    // Reinitialize
    agent = std::make_unique<TrayAgent>();
    Status status = agent->initialize();
    EXPECT_EQ(status, Status::OK);
    
    // Should be able to set status again
    agent->setStatus(AgentStatus::BUSY);
    SUCCEED();
#else
    GTEST_SKIP() << "Test only runs on Windows";
#endif
}

// Test: Notification stress test
TEST_F(TrayAgentTest, NotificationStressTest) {
#ifdef _WIN32
    agent->initialize();
    
    // Send many notifications
    for (int i = 0; i < 50; ++i) {
        agent->showNotification(
            "Notification #" + std::to_string(i),
            "Message content " + std::to_string(i)
        );
    }
    
    SUCCEED();
#else
    GTEST_SKIP() << "Test only runs on Windows";
#endif
}

// Test: Status enum values
TEST_F(TrayAgentTest, StatusEnumValues) {
    // Ensure enum values are valid
    AgentStatus online = AgentStatus::ONLINE;
    AgentStatus offline = AgentStatus::OFFLINE;
    AgentStatus busy = AgentStatus::BUSY;
    
    EXPECT_NE(online, offline);
    EXPECT_NE(online, busy);
    EXPECT_NE(offline, busy);
}

// Test: Notification with Unicode characters
TEST_F(TrayAgentTest, NotificationUnicode) {
#ifdef _WIN32
    agent->initialize();
    
    agent->showNotification("你好世界", "Привет мир 🌍");
    agent->showNotification("مرحبا", "こんにちは");
    agent->showNotification("🚀🎉🎊", "Emoji test 😊👍");
    
    SUCCEED();
#else
    agent->showNotification("你好世界", "Привет мир 🌍");
    SUCCEED();
#endif
}

// Test: All status types with notifications
TEST_F(TrayAgentTest, AllStatusTypesWithNotifications) {
#ifdef _WIN32
    agent->initialize();
    
    agent->setStatus(AgentStatus::ONLINE);
    agent->showNotification("Status Change", "Now ONLINE");
    
    agent->setStatus(AgentStatus::OFFLINE);
    agent->showNotification("Status Change", "Now OFFLINE");
    
    agent->setStatus(AgentStatus::BUSY);
    agent->showNotification("Status Change", "Now BUSY");
    
    SUCCEED();
#else
    GTEST_SKIP() << "Test only runs on Windows";
#endif
}


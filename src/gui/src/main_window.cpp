#include "gui/main_window.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QTimer>
#include <QGroupBox>
#include <QFormLayout>
#include <QStatusBar>
#include <QMainWindow>
#include <QWidget>
#include <QProgressBar>
#include <QCheckBox>
#include <QScrollArea>
#include <QFrame>
#include <QGridLayout>
#include <QTextCursor>
#include <QFont>
#include <Qt>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <vector>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

namespace providers_agent {
namespace gui {

class MainWindow::Impl {
public:
    QMainWindow* window;
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    
    // Top status bar
    QLabel* connectionStatus;
    QLabel* machineName;
    QLabel* agentVersion;
    QLabel* lastSync;
    
    // Left side - System Information
    QGroupBox* gpusGroup;  // Reused for System Information panel
    QTextEdit* systemInfoText;
    
    // Right side - Job Control and Logs
    QGroupBox* jobControlGroup;
    QCheckBox* acceptingJobsCheckbox;  // Used for "Go Online" toggle
    
    QGroupBox* logsGroup;
    QTextEdit* logsText;
    
    QTimer* updateTimer;
    
    int heartbeatCount;
    bool heartbeatRunning;
    int lastSyncMinutes;
    
    Impl() : heartbeatCount(0), heartbeatRunning(false), lastSyncMinutes(2) {
        window = new QMainWindow();
        centralWidget = new QWidget();
        mainLayout = new QVBoxLayout(centralWidget);
        
        // Top Status Bar
        QFrame* statusBar = new QFrame();
        statusBar->setFrameShape(QFrame::Box);
        statusBar->setStyleSheet("background-color: #f0f0f0; padding: 10px;");
        QHBoxLayout* statusLayout = new QHBoxLayout(statusBar);
        
        connectionStatus = new QLabel("Connected");
        // connectionStatus->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #008000; }");
        machineName = new QLabel("Machine: " + QString::fromStdString(getMachineName()));
        agentVersion = new QLabel("Agent v1.2.3");
        lastSync = new QLabel("Last sync: 2m ago");
        
        statusLayout->addWidget(connectionStatus);
        statusLayout->addWidget(machineName);
        statusLayout->addWidget(agentVersion);
        statusLayout->addStretch();
        statusLayout->addWidget(lastSync);
        
        // Main content area - split left/right
        QHBoxLayout* contentLayout = new QHBoxLayout();
        
    // Left side - System Information
    gpusGroup = new QGroupBox("System Information");
    QVBoxLayout* systemInfoLayout = new QVBoxLayout();
    systemInfoText = new QTextEdit();
    systemInfoText->setReadOnly(true);
    systemInfoText->setFont(QFont("Courier", 10));
    systemInfoText->setStyleSheet("background-color: #f8f9fa; border: 1px solid #ddd; border-radius: 6px; padding: 10px;");
    systemInfoLayout->addWidget(systemInfoText);
    gpusGroup->setLayout(systemInfoLayout);
        
    // Right side
    QVBoxLayout* rightLayout = new QVBoxLayout();
    
    // Job Control
    jobControlGroup = new QGroupBox("Job Control");
    QVBoxLayout* jobControlLayout = new QVBoxLayout();
    
    QHBoxLayout* goOnlineLayout = new QHBoxLayout();
    goOnlineLayout->addWidget(new QLabel("Go Online:"));
    acceptingJobsCheckbox = new QCheckBox();
    acceptingJobsCheckbox->setChecked(false);  // Default OFF
    acceptingJobsCheckbox->setStyleSheet("QCheckBox::indicator { width: 20px; height: 20px; }");
    goOnlineLayout->addWidget(acceptingJobsCheckbox);
    goOnlineLayout->addStretch();
    jobControlLayout->addLayout(goOnlineLayout);
    jobControlGroup->setLayout(jobControlLayout);
        
        // Logs
        logsGroup = new QGroupBox("Logs");
        QVBoxLayout* logsLayout = new QVBoxLayout();
        logsText = new QTextEdit();
        logsText->setReadOnly(true);
        logsText->setFont(QFont("Courier", 9));
        logsLayout->addWidget(logsText);
        logsGroup->setLayout(logsLayout);
        
        rightLayout->addWidget(jobControlGroup);
        rightLayout->addWidget(logsGroup);
        
        contentLayout->addWidget(gpusGroup, 2);  // GPUs take 2/3 of space
        contentLayout->addLayout(rightLayout, 1);  // Right side takes 1/3
        
        mainLayout->addWidget(statusBar);
        mainLayout->addLayout(contentLayout);
        
        window->setCentralWidget(centralWidget);
        window->setWindowTitle("OpenGPU Agent Dashboard");
        window->resize(1000, 700);
        
        updateTimer = new QTimer();
        
        // Initialize system info display
        systemInfoText->setPlainText("Loading system information...");
    }
    
    ~Impl() {
        delete updateTimer;
        delete window;
    }
    
    std::string getMachineName() {
#ifdef _WIN32
        char computerName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = sizeof(computerName);
        if (GetComputerNameA(computerName, &size)) {
            return std::string(computerName) + "-Win";
        }
#else
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            return std::string(hostname) + "-Mac";
        }
#endif
        return "Unknown-Machine";
    }
    
    
    void addLog(const std::string& logEntry) {
        logsText->append(QString::fromStdString(logEntry));
        // Auto-scroll to bottom
        QTextCursor cursor = logsText->textCursor();
        cursor.movePosition(QTextCursor::End);
        logsText->setTextCursor(cursor);
    }
};

MainWindow::MainWindow() : pImpl(new Impl()) {
    // Connect signals
    QObject::connect(pImpl->acceptingJobsCheckbox, &QCheckBox::toggled,
                     [this](bool checked) { 
                         if (onGoOnlineChanged) onGoOnlineChanged(checked);
                     });
    
    // Update last sync timer
    pImpl->updateTimer = new QTimer();
    QObject::connect(pImpl->updateTimer, &QTimer::timeout, [this]() {
        pImpl->lastSyncMinutes++;
        pImpl->lastSync->setText(QString("Last sync: %1m ago").arg(pImpl->lastSyncMinutes));
    });
    pImpl->updateTimer->start(60000); // Update every minute
}

MainWindow::~MainWindow() {
    delete pImpl;
}

common::Status MainWindow::initialize() {
    return common::Status::OK;
}

void MainWindow::show() {
    pImpl->window->show();
}

void MainWindow::updateSystemInfo(const std::string& json_text) {
    QMetaObject::invokeMethod(pImpl->window, [this, json_text]() {
        // System info can be logged if needed
        pImpl->addLog("System info updated");
    }, Qt::QueuedConnection);
}

void MainWindow::updateHeartbeatStatus(bool running, int count, const std::string& lastMessage, bool lastSuccess) {
    QMetaObject::invokeMethod(pImpl->window, [this, running, count, lastMessage, lastSuccess]() {
        pImpl->heartbeatRunning = running;
        pImpl->heartbeatCount = count;
        
        if (running) {
            pImpl->connectionStatus->setText("Connected");
            // pImpl->connectionStatus->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #008000; }");
        } else {
            pImpl->connectionStatus->setText("Disconnected");
            // pImpl->connectionStatus->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #ff0000; }");
        }
        
        if (!lastMessage.empty()) {
            pImpl->addLog(lastMessage);
        }
    }, Qt::QueuedConnection);
}

void MainWindow::updateConnectionStatus(bool connected, const std::string& machineName, const std::string& agentVersion, int lastSyncMinutes) {
    QMetaObject::invokeMethod(pImpl->window, [this, connected, machineName, agentVersion, lastSyncMinutes]() {
        if (connected) {
            pImpl->connectionStatus->setText("Connected");
            // pImpl->connectionStatus->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #008000; }");
        } else {
            pImpl->connectionStatus->setText("Disconnected");
            // pImpl->connectionStatus->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #ff0000; }");
        }
        
        if (!machineName.empty()) {
            pImpl->machineName->setText(QString("Machine: %1").arg(QString::fromStdString(machineName)));
        }
        
        if (!agentVersion.empty()) {
            pImpl->agentVersion->setText(QString("Agent %1").arg(QString::fromStdString(agentVersion)));
        }
        
        pImpl->lastSyncMinutes = lastSyncMinutes;
        //pImpl->lastSync->setText(QString("Last sync: %1m ago").arg(lastSyncMinutes));
    }, Qt::QueuedConnection);
}

void MainWindow::updateGPUs(const std::vector<GPUInfo>& gpus) {
    QMetaObject::invokeMethod(pImpl->window, [this, gpus]() {
        // GPU display functionality - can be implemented later if needed
        // For now, we're showing system info instead
    }, Qt::QueuedConnection);
}

void MainWindow::addLog(const std::string& logEntry) {
    QMetaObject::invokeMethod(pImpl->window, [this, logEntry]() {
        pImpl->addLog(logEntry);
    }, Qt::QueuedConnection);
}

void MainWindow::setGoOnline(bool online) {
    QMetaObject::invokeMethod(pImpl->window, [this, online]() {
        pImpl->acceptingJobsCheckbox->setChecked(online);
    }, Qt::QueuedConnection);
}

void MainWindow::updateSystemInfoDisplay(const std::string& cpuInfo, const std::string& memoryInfo) {
    QMetaObject::invokeMethod(pImpl->window, [this, cpuInfo, memoryInfo]() {
        QString info = QString("CPU Information:\n%1\n\nMemory Information:\n%2")
            .arg(QString::fromStdString(cpuInfo))
            .arg(QString::fromStdString(memoryInfo));
        pImpl->systemInfoText->setPlainText(info);
    }, Qt::QueuedConnection);
}

int MainWindow::exec() {
    // Note: QApplication::exec() should be called in main(), not here
    // This method is kept for interface compatibility but does nothing
    return 0;
}

} // namespace gui
} // namespace providers_agent

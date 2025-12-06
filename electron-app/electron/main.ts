import { app, BrowserWindow, ipcMain, Tray, Menu, nativeImage } from 'electron';
import path from 'path';
import { spawn, ChildProcess, execSync } from 'child_process';
import Store from 'electron-store';
import * as os from 'os';
import * as fs from 'fs';

const store = new Store();
let mainWindow: BrowserWindow | null = null;
let tray: Tray | null = null;

// C++ Backend processes
let heartbeatProcess: ChildProcess | null = null;
let jobManagerProcess: ChildProcess | null = null;

const isDev = process.env.NODE_ENV === 'development';

interface SystemInfo {
  cpu: {
    vendor: string;
    model: string;
    cores: number;
    logical_processors: number;
    frequency_mhz: number;
  };
  memory: {
    total_bytes: number;
    available_bytes: number;
    usage_percent: number;
  };
  gpus: Array<{
    name: string;
    vram_total_bytes: number;
    vram_used_bytes: number;
    temperature_celsius: number;
    utilization_percent: number;
  }>;
  os: {
    name: string;
    version: string;
    architecture: string;
    hostname: string;
  };
  nodeId?: string;
}

interface AppState {
  isOnline: boolean;
  systemInfo: SystemInfo | null;
  apiEndpoint: string;
  oauthToken: string | null;
  heartbeatInterval: number;
  jobPollingInterval: number;
  currentJob: any | null;
  logs: string[];
}

const appState: AppState = {
  isOnline: false,
  systemInfo: null,
  apiEndpoint: process.env.HEART_BEAT_API_ENDPOINT || 'http://localhost:3000',
  oauthToken: store.get('oauthToken') as string || null,
  heartbeatInterval: 2,
  jobPollingInterval: 5,
  currentJob: null,
  logs: [],
};

// Get the path to the C++ backend build directory
function getBackendPath(): string {
  if (isDev) {
    // In development, the backend is in the parent directory
    return path.join(__dirname, '..', '..', '..', 'build', 'bin');
  } else {
    // In production, it's in the resources folder
    return path.join(process.resourcesPath, 'backend', 'bin');
  }
}

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1400,
    height: 900,
    minWidth: 1200,
    minHeight: 700,
    backgroundColor: '#f9fafb',
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      preload: path.join(__dirname, 'preload.js'),
    },
    titleBarStyle: 'hiddenInset',
    trafficLightPosition: { x: 15, y: 15 },
  });

  if (isDev) {
    // Try multiple ports in case Vite picked a different one
    const tryPorts = [5173, 5174, 5175, 5176, 5177];
    let loaded = false;
    
    const tryLoadUrl = async (portIndex: number) => {
      if (portIndex >= tryPorts.length || loaded) return;
      
      const port = tryPorts[portIndex];
      const url = `http://localhost:${port}`;
      
      try {
        await mainWindow?.loadURL(url);
        loaded = true;
        addLog(`Connected to Vite dev server on port ${port}`);
      } catch (error) {
        // Try next port
        tryLoadUrl(portIndex + 1);
      }
    };
    
    tryLoadUrl(0);
    mainWindow.webContents.openDevTools();
  } else {
    mainWindow.loadFile(path.join(__dirname, '../renderer/index.html'));
  }

  mainWindow.on('closed', () => {
    mainWindow = null;
  });

  mainWindow.on('close', (event) => {
    if (process.platform === 'darwin') {
      event.preventDefault();
      mainWindow?.hide();
    }
  });
}

function createTray() {
  // Create a simple colored icon for the tray (placeholder)
  const icon = nativeImage.createFromDataURL(
    'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAABHNCSVQICAgIfAhkiAAAAAlwSFlzAAAA7AAAAOwBeShxvQAAABl0RVh0U29mdHdhcmUAd3d3Lmlua3NjYXBlLm9yZ5vuPBoAAAIVSURBVFiF7ZdNaBNBFMd/s5vdJrtJ2qSpVUSsIoIgePDkQfDgRRBB8OLRix/gQT0oHrx48uRBUPDgwYMHQQTBg4iCBy8ePHhQUBHFKmqrtW2SJtnNx+zOeDCbNLubJG1TwYP/aWfmvXn/921m3oLBYDAYDAaDwfAfIv7WQJZlnU6n09Vq9RiAZVmPgCfAN8uyCoVC4eOuBMjzvJ1KpW4Cx4FjwArgGXAXuA+8A04AZ4HLwGXLsl4WCoXV/w0gy/IOYAm4DVwErgGfgEvAY+BGt9vtViqV18B94BZwFTgFnO1nY0cBsixvB54DJ4EngNLv95dlWSdVVV0GXgErgNLtdj8opQpKqR+SJH0BKv1s7CggyzIBPAGjUqmUTafTJ4HXnU7nbbvdfqCUetVqtVYkSfqmlPra6XR+Dts4tAc8z9vT7/cfAq8lSToFzAOzQFQUxU+tVuuELMuzwJwkSbO9Xm+23+9PA3PDNm4KSCaTCeAKcAN4DpwGckAamABywHGgAEwBSeBBPB6f6Pf7M8AIEAXiwJH/EXAeuA18kCRpkUgkgjAMj+q6ntB1XdQ07ZCqqgfCMDykaVrS87x4GAZngiC44HneQhAEF33fnx0YIEkSADqOE7VtO2YYhmXbdsw0zZht23aj0bBt245pmvVqtWrbdr1SqdiGYdi1Ws22LKve7/cjQRAE/Q5CoxRFAUBd1/8A6Lru/B0MAAAAAASUVORK5CYII='
  );

  tray = new Tray(icon);

  const contextMenu = Menu.buildFromTemplate([
    {
      label: 'Show App',
      click: () => {
        mainWindow?.show();
      },
    },
    {
      label: appState.isOnline ? 'Go Offline' : 'Go Online',
      id: 'toggle-online',
      click: () => {
        toggleOnlineStatus();
      },
    },
    { type: 'separator' },
    {
      label: 'Quit',
      click: () => {
        app.quit();
      },
    },
  ]);

  tray.setContextMenu(contextMenu);
  tray.setToolTip('GPU Marketplace Provider');

  tray.on('click', () => {
    mainWindow?.show();
  });
}

function updateTrayMenu() {
  if (!tray) return;

  const contextMenu = Menu.buildFromTemplate([
    {
      label: 'Show App',
      click: () => {
        mainWindow?.show();
      },
    },
    {
      label: appState.isOnline ? 'Go Offline' : 'Go Online',
      click: () => {
        toggleOnlineStatus();
      },
    },
    { type: 'separator' },
    {
      label: `Status: ${appState.isOnline ? 'Online' : 'Offline'}`,
      enabled: false,
    },
    { type: 'separator' },
    {
      label: 'Quit',
      click: () => {
        app.quit();
      },
    },
  ]);

  tray.setContextMenu(contextMenu);
}

function addLog(message: string) {
  const timestamp = new Date().toISOString();
  const logEntry = `[${timestamp}] ${message}`;
  appState.logs.push(logEntry);

  // Keep only last 1000 logs
  if (appState.logs.length > 1000) {
    appState.logs.shift();
  }

  // Send to renderer
  mainWindow?.webContents.send('log-update', logEntry);
}

function toggleOnlineStatus() {
  if (appState.isOnline) {
    goOffline();
  } else {
    goOnline();
  }
}

function goOnline() {
  if (!appState.oauthToken) {
    mainWindow?.webContents.send('auth-required');
    addLog('Authentication required. Please set OAuth token in settings.');
    return;
  }

  appState.isOnline = true;
  addLog('Going online...');

  // Start heartbeat
  startHeartbeat();

  // Start job polling
  startJobPolling();

  mainWindow?.webContents.send('status-changed', { isOnline: true });
  updateTrayMenu();

  addLog('Now online and accepting jobs.');
}

function goOffline() {
  appState.isOnline = false;
  addLog('Going offline...');

  // Stop heartbeat
  stopHeartbeat();

  // Stop job polling
  stopJobPolling();

  mainWindow?.webContents.send('status-changed', { isOnline: false });
  updateTrayMenu();

  addLog('Now offline.');
}

// Simulated heartbeat (replace with actual C++ backend call)
let heartbeatTimer: NodeJS.Timeout | null = null;

function startHeartbeat() {
  if (heartbeatTimer) return;

  addLog(`Starting heartbeat with ${appState.heartbeatInterval}s interval`);

  heartbeatTimer = setInterval(() => {
    sendHeartbeat();
  }, appState.heartbeatInterval * 1000);

  // Send first heartbeat immediately
  sendHeartbeat();
}

function stopHeartbeat() {
  if (heartbeatTimer) {
    clearInterval(heartbeatTimer);
    heartbeatTimer = null;
    addLog('Heartbeat stopped');
  }
}

async function sendHeartbeat() {
  try {
    // This would call your C++ backend or send directly
    const payload = {
      status: 'online',
      metrics: appState.systemInfo,
    };

    // Simulate API call
    addLog('Heartbeat sent successfully');
    mainWindow?.webContents.send('heartbeat-sent', { success: true });
  } catch (error) {
    addLog(`Heartbeat failed: ${error}`);
    mainWindow?.webContents.send('heartbeat-sent', { success: false, error });
  }
}

// Simulated job polling
let jobPollingTimer: NodeJS.Timeout | null = null;

function startJobPolling() {
  if (jobPollingTimer) return;

  addLog(`Starting job polling with ${appState.jobPollingInterval}s interval`);

  jobPollingTimer = setInterval(() => {
    pollForJobs();
  }, appState.jobPollingInterval * 1000);

  // Poll immediately
  pollForJobs();
}

function stopJobPolling() {
  if (jobPollingTimer) {
    clearInterval(jobPollingTimer);
    jobPollingTimer = null;
    addLog('Job polling stopped');
  }
}

async function pollForJobs() {
  try {
    // This would call your C++ backend job manager
    addLog('Polling for jobs...');

    // Simulate finding no jobs (replace with actual implementation)
    mainWindow?.webContents.send('jobs-polled', { jobs: [] });
  } catch (error) {
    addLog(`Job polling failed: ${error}`);
  }
}

// Get real system info using Node.js os module and C++ backend
async function getSystemInfo(): Promise<SystemInfo> {
  const cpus = os.cpus();
  const totalMem = os.totalmem();
  const freeMem = os.freemem();
  const usedMem = totalMem - freeMem;
  const memUsagePercent = Math.round((usedMem / totalMem) * 100);

  // Get CPU info
  const cpuModel = cpus[0]?.model || 'Unknown CPU';
  const cpuVendor = cpuModel.includes('Intel') ? 'Intel' : 
                    cpuModel.includes('AMD') ? 'AMD' : 
                    cpuModel.includes('Apple') ? 'Apple' : 'Unknown';
  const cpuFrequency = cpus[0]?.speed || 0;

  // Get OS info
  const platform = os.platform();
  const osName = platform === 'win32' ? 'Windows' :
                 platform === 'darwin' ? 'macOS' :
                 platform === 'linux' ? 'Linux' : platform;
  const osVersion = os.release();
  const arch = os.arch();
  const hostname = os.hostname();

  // Get GPU info from C++ backend's gpu_info.json
  let gpus: SystemInfo['gpus'] = [];
  try {
    const backendPath = getBackendPath();
    const gpuInfoPath = path.join(backendPath, 'gpu_info.json');
    
    if (fs.existsSync(gpuInfoPath)) {
      const gpuData = JSON.parse(fs.readFileSync(gpuInfoPath, 'utf-8'));
      if (gpuData.gpus && Array.isArray(gpuData.gpus)) {
        gpus = gpuData.gpus.map((gpu: any) => ({
          name: gpu.name || 'Unknown GPU',
          vram_total_bytes: gpu.vram_total_bytes || 0,
          vram_used_bytes: gpu.vram_used_bytes || 0,
          temperature_celsius: gpu.temperature_celsius || 0,
          utilization_percent: gpu.utilization_percent || 0,
        }));
      }
      addLog(`Loaded GPU info from ${gpuInfoPath}`);
    } else {
      addLog(`GPU info file not found at ${gpuInfoPath}, using fallback detection`);
      // Fallback: Try to detect GPU using system commands on Windows
      if (platform === 'win32') {
        try {
          const wmicOutput = execSync('wmic path win32_VideoController get name,AdapterRAM /format:csv', { encoding: 'utf-8' });
          const lines = wmicOutput.trim().split('\n').filter(line => line.trim() && !line.includes('Node,'));
          
          for (const line of lines) {
            const parts = line.split(',');
            if (parts.length >= 3) {
              const adapterRam = parseInt(parts[1]) || 0;
              const gpuName = parts[2]?.trim() || 'Unknown GPU';
              if (gpuName && gpuName !== 'Name') {
                gpus.push({
                  name: gpuName,
                  vram_total_bytes: adapterRam,
                  vram_used_bytes: 0,
                  temperature_celsius: 0,
                  utilization_percent: 0,
                });
              }
            }
          }
          addLog(`Detected ${gpus.length} GPU(s) using WMIC`);
        } catch (e) {
          addLog(`Failed to detect GPU using WMIC: ${e}`);
        }
      }
    }
  } catch (error) {
    addLog(`Error loading GPU info: ${error}`);
  }

  // If no GPUs found, add a placeholder
  if (gpus.length === 0) {
    gpus.push({
      name: 'No GPU detected',
      vram_total_bytes: 0,
      vram_used_bytes: 0,
      temperature_celsius: 0,
      utilization_percent: 0,
    });
  }

  const systemInfo: SystemInfo = {
    cpu: {
      vendor: cpuVendor,
      model: cpuModel,
      cores: cpus.length > 0 ? Math.floor(cpus.length / 2) : 1, // Physical cores (approximate)
      logical_processors: cpus.length,
      frequency_mhz: cpuFrequency,
    },
    memory: {
      total_bytes: totalMem,
      available_bytes: freeMem,
      usage_percent: memUsagePercent,
    },
    gpus,
    os: {
      name: osName,
      version: osVersion,
      architecture: arch,
      hostname: hostname,
    },
  };

  return systemInfo;
}

// IPC Handlers
ipcMain.handle('get-system-info', async () => {
  // Always refresh system info
  appState.systemInfo = await getSystemInfo();
  return appState.systemInfo;
});

ipcMain.handle('get-app-state', async () => {
  return {
    isOnline: appState.isOnline,
    apiEndpoint: appState.apiEndpoint,
    oauthToken: appState.oauthToken ? '***' : null,
    heartbeatInterval: appState.heartbeatInterval,
    jobPollingInterval: appState.jobPollingInterval,
  };
});

ipcMain.handle('toggle-online', async () => {
  toggleOnlineStatus();
  return { isOnline: appState.isOnline };
});

ipcMain.handle('set-oauth-token', async (_event, token: string) => {
  appState.oauthToken = token;
  store.set('oauthToken', token);
  addLog('OAuth token updated');
  return { success: true };
});

ipcMain.handle('set-api-endpoint', async (_event, endpoint: string) => {
  appState.apiEndpoint = endpoint;
  addLog(`API endpoint updated: ${endpoint}`);
  return { success: true };
});

ipcMain.handle('set-intervals', async (_event, { heartbeat, jobPolling }: { heartbeat: number, jobPolling: number }) => {
  appState.heartbeatInterval = heartbeat;
  appState.jobPollingInterval = jobPolling;

  // Restart timers if online
  if (appState.isOnline) {
    stopHeartbeat();
    stopJobPolling();
    startHeartbeat();
    startJobPolling();
  }

  addLog(`Intervals updated: Heartbeat=${heartbeat}s, Job Polling=${jobPolling}s`);
  return { success: true };
});

ipcMain.handle('get-logs', async () => {
  return appState.logs;
});

ipcMain.handle('clear-logs', async () => {
  appState.logs = [];
  addLog('Logs cleared');
  return { success: true };
});

// App lifecycle
app.whenReady().then(async () => {
  // Load system info
  appState.systemInfo = await getSystemInfo();
  addLog('GPU Marketplace Provider Agent started');
  addLog(`System info loaded: ${appState.systemInfo.cpu.model}, ${(appState.systemInfo.memory.total_bytes / 1024 / 1024 / 1024).toFixed(1)}GB RAM`);
  if (appState.systemInfo.gpus.length > 0) {
    addLog(`GPU detected: ${appState.systemInfo.gpus.map(g => g.name).join(', ')}`);
  }

  createWindow();
  createTray();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    } else {
      mainWindow?.show();
    }
  });
});

app.on('window-all-closed', () => {
  // Keep app running in background on macOS
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('before-quit', () => {
  // Cleanup
  stopHeartbeat();
  stopJobPolling();

  if (heartbeatProcess !== null) {
    (heartbeatProcess as ChildProcess).kill();
  }
  if (jobManagerProcess !== null) {
    (jobManagerProcess as ChildProcess).kill();
  }
});

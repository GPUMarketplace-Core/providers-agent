import { app, BrowserWindow, ipcMain, Tray, Menu, nativeImage } from 'electron';
import path from 'path';
import { spawn, ChildProcess, execSync } from 'child_process';
import Store from 'electron-store';
import * as os from 'os';
import * as fs from 'fs';
import * as crypto from 'crypto';
import axios from 'axios';
import FormData from 'form-data';

const store = new Store();
let mainWindow: BrowserWindow | null = null;
let tray: Tray | null = null;

// Blender process
let blenderProcess: ChildProcess | null = null;

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

interface Job {
  id: string;
  status: 'queued' | 'running' | 'completed' | 'failed';
  config_ref: string;
  created_at: string;
  completed_at?: string;
  failure_reason?: string;
}

interface AppState {
  isOnline: boolean;
  systemInfo: SystemInfo | null;
  apiEndpoint: string;
  oauthToken: string | null;
  heartbeatInterval: number;
  jobPollingInterval: number;
  currentJob: Job | null;
  logs: string[];
  nodeId: string;
  heartbeatCount: number;
  jobsCompleted: number;
  jobsFailed: number;
  gpuRates: Array<{ gpu_index: number; hourly_price_cents: number }>;
}

// Generate a unique node ID based on machine info (UUID v4 format)
function generateNodeId(): string {
  const stored = store.get('nodeId') as string | undefined;
  
  // Check if stored ID is already a valid UUID format
  if (stored && /^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/i.test(stored)) {
    return stored;
  }
  
  // Generate a deterministic UUID from machine-specific info
  const machineId = `${os.hostname()}-${os.platform()}-${os.arch()}-${os.cpus()[0]?.model || 'cpu'}`;
  const hash = crypto.createHash('sha256').update(machineId).digest('hex');
  
  // Format as UUID v4 (8-4-4-4-12)
  const nodeId = [
    hash.substring(0, 8),
    hash.substring(8, 12),
    '4' + hash.substring(13, 16), // Version 4
    ((parseInt(hash.substring(16, 17), 16) & 0x3) | 0x8).toString(16) + hash.substring(17, 20), // Variant
    hash.substring(20, 32),
  ].join('-');
  
  store.set('nodeId', nodeId);
  return nodeId;
}

// Load stored token
const storedToken = store.get('oauthToken') as string | undefined;
console.log('[STARTUP] Loading stored token:', storedToken ? `${storedToken.substring(0, 20)}...` : 'none');

const appState: AppState = {
  isOnline: false,
  systemInfo: null,
  apiEndpoint: process.env.HEART_BEAT_API_ENDPOINT || (store.get('apiEndpoint') as string) || 'http://localhost:3000',
  oauthToken: storedToken || null,
  heartbeatInterval: (store.get('heartbeatInterval') as number) || 2,
  jobPollingInterval: (store.get('jobPollingInterval') as number) || 5,
  currentJob: null,
  logs: [],
  nodeId: generateNodeId(),
  heartbeatCount: 0,
  jobsCompleted: 0,
  jobsFailed: 0,
  gpuRates: (store.get('gpuRates') as any[]) || [{ gpu_index: 0, hourly_price_cents: 5000 }],
};

// Get the path to temp jobs directory
function getTempJobsDir(): string {
  const backendPath = getBackendPath();
  const tempDir = path.join(backendPath, 'temp_jobs');
  if (!fs.existsSync(tempDir)) {
    fs.mkdirSync(tempDir, { recursive: true });
  }
  return tempDir;
}

// Get the path to the C++ backend build directory
function getBackendPath(): string {
  if (isDev) {
    return path.join(__dirname, '..', '..', '..', 'build', 'bin');
  } else {
    return path.join(process.resourcesPath, 'backend', 'bin');
  }
}

// Get Blender executable path
function getBlenderPath(): string {
  const backendPath = getBackendPath();
  const blenderPath = path.join(backendPath, 'blender', 'blender-5.0.0-windows-x64', 'blender.exe');
  return blenderPath;
}

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1400,
    height: 900,
    minWidth: 1200,
    minHeight: 700,
    backgroundColor: '#0f0f23',
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      preload: path.join(__dirname, 'preload.js'),
    },
    titleBarStyle: 'hiddenInset',
    trafficLightPosition: { x: 15, y: 15 },
    frame: process.platform === 'darwin' ? false : true,
  });

  if (isDev) {
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
  const icon = nativeImage.createFromDataURL(
    'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAABHNCSVQICAgIfAhkiAAAAAlwSFlzAAAA7AAAAOwBeShxvQAAABl0RVh0U29mdHdhcmUAd3d3Lmlua3NjYXBlLm9yZ5vuPBoAAAIVSURBVFiF7ZdNaBNBFMd/s5vdJrtJ2qSpVUSsIoIgePDkQfDgRRBB8OLRix/gQT0oHrx48uRBUPDgwYMHQQTBg4iCBy8ePHhQUBHFKmqrtW2SJtnNx+zOeDCbNLubJG1TwYP/aWfmvXn/921m3oLBYDAYDAaDwfAfIv7WQJZlnU6n09Vq9RiAZVmPgCfAN8uyCoVC4eOuBMjzvJ1KpW4Cx4FjwArgGXAXuA+8A04AZ4HLwGXLsl4WCoXV/w0gy/IOYAm4DVwErgGfgEvAY+BGt9vtViqV18B94BZwFTgFnO1nY0cBsixvB54DJ4AngNLv95dlWSdVVV0GXgErgNLtdj8opQpKqR+SJH0BKv1s7CggyzIBPAGjUqmUTafTJ4HXnU7nbbvdfqCUetVqtVYkSfqmlPra6XR+Dts4tAc8z9vT7/cfAq8lSToFzAOzQFQUxU+tVuuELMuzwJwkSbO9Xm+23+9PA3PDNm4KSCaTCeAKcAN4DpwGckAamABywHGgAEwBSeBBPB6f6Pf7M8AIEAXiwJH/EXAeuA18kCRpkUgkgjAMj+q6ntB1XdQ07ZCqqgfCMDykaVrS87x4GAZngiC44HneQhAEF33fnx0YIEkSADqOE7VtO2YYhmXbdsw0zZht23aj0bBt245pmvVqtWrbdr1SqdiGYdi1Ws22LKve7/cjQRAE/Q5CoxRFAUBd1/8A6Lru/B0MAAAAAASUVORK5CYII='
  );

  tray = new Tray(icon);
  updateTrayMenu();
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
      click: () => mainWindow?.show(),
    },
    {
      label: appState.isOnline ? '🟢 Go Offline' : '⚪ Go Online',
      click: () => toggleOnlineStatus(),
    },
    { type: 'separator' },
    {
      label: `Status: ${appState.isOnline ? 'Online' : 'Offline'}`,
      enabled: false,
    },
    {
      label: `Node: ${appState.nodeId.substring(0, 12)}...`,
      enabled: false,
    },
    { type: 'separator' },
    {
      label: 'Quit',
      click: () => app.quit(),
    },
  ]);

  tray.setContextMenu(contextMenu);
}

function addLog(message: string) {
  const timestamp = new Date().toISOString();
  const logEntry = `[${timestamp}] ${message}`;
  appState.logs.push(logEntry);

  if (appState.logs.length > 1000) {
    appState.logs.shift();
  }

  mainWindow?.webContents.send('log-update', logEntry);
  console.log(logEntry);
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
    addLog('❌ Authentication required. Please set OAuth token in settings.');
    return;
  }

  appState.isOnline = true;
  appState.heartbeatCount = 0;
  addLog('🟢 Going online...');

  startHeartbeat();
  startJobPolling();

  mainWindow?.webContents.send('status-changed', { isOnline: true });
  updateTrayMenu();

  addLog('✅ Now online and accepting jobs.');
}

function goOffline() {
  appState.isOnline = false;
  addLog('⚪ Going offline...');

  stopHeartbeat();
  stopJobPolling();

  mainWindow?.webContents.send('status-changed', { isOnline: false });
  updateTrayMenu();

  addLog('✅ Now offline.');
}

// ============ HEARTBEAT ============
let heartbeatTimer: NodeJS.Timeout | null = null;

function startHeartbeat() {
  if (heartbeatTimer) return;

  addLog(`💓 Starting heartbeat with ${appState.heartbeatInterval}s interval`);

  heartbeatTimer = setInterval(() => {
    sendHeartbeat();
  }, appState.heartbeatInterval * 1000);

  sendHeartbeat();
}

function stopHeartbeat() {
  if (heartbeatTimer) {
    clearInterval(heartbeatTimer);
    heartbeatTimer = null;
    addLog('💓 Heartbeat stopped');
  }
}

async function sendHeartbeat() {
  if (!appState.oauthToken) {
    addLog('⚠️ Heartbeat skipped: No OAuth token');
    return;
  }
  
  if (!appState.systemInfo) {
    addLog('⚠️ Heartbeat skipped: System info not loaded');
    return;
  }

  try {
    const url = `${appState.apiEndpoint}/api/nodes/${appState.nodeId}/heartbeat`;
    
    const payload = {
      status: 'online',
      metrics: {
        cpu: appState.systemInfo.cpu,
        memory: appState.systemInfo.memory,
        gpus: appState.systemInfo.gpus,
        os: appState.systemInfo.os,
        gpu_rates: appState.gpuRates,
      },
    };

    // Log first heartbeat details for debugging
    if (appState.heartbeatCount === 0) {
      addLog(`📡 Heartbeat URL: ${url}`);
      addLog(`🔑 Token preview: ${appState.oauthToken.substring(0, 30)}...`);
    }

    const response = await axios.post(url, payload, {
      headers: {
        'Authorization': `Bearer ${appState.oauthToken}`,
        'Content-Type': 'application/json',
      },
      timeout: 10000,
    });

    appState.heartbeatCount++;
    
    if (response.data.success) {
      mainWindow?.webContents.send('heartbeat-sent', { 
        success: true, 
        count: appState.heartbeatCount 
      });
    }
  } catch (error: any) {
    const statusCode = error.response?.status || 'N/A';
    const errorMsg = error.response?.data?.error || error.message;
    const details = error.response?.data?.details || '';
    const stack = error.response?.data?.stack || '';
    
    addLog(`❌ Heartbeat failed [${statusCode}]: ${errorMsg}`);
    if (details) addLog(`   Details: ${details}`);
    if (stack) addLog(`   Stack: ${stack.split('\n')[0]}`);
    
    // Log the full response for debugging
    if (error.response?.data) {
      console.log('Server response:', JSON.stringify(error.response.data, null, 2));
    }
    
    mainWindow?.webContents.send('heartbeat-sent', { success: false, error: errorMsg });
  }
}

// ============ JOB POLLING ============
let jobPollingTimer: NodeJS.Timeout | null = null;
let isProcessingJob = false;

function startJobPolling() {
  if (jobPollingTimer) return;

  addLog(`📋 Starting job polling with ${appState.jobPollingInterval}s interval`);

  jobPollingTimer = setInterval(() => {
    pollForJobs();
  }, appState.jobPollingInterval * 1000);

  pollForJobs();
}

function stopJobPolling() {
  if (jobPollingTimer) {
    clearInterval(jobPollingTimer);
    jobPollingTimer = null;
    addLog('📋 Job polling stopped');
  }
}

async function pollForJobs() {
  if (isProcessingJob || !appState.oauthToken) {
    return;
  }

  try {
    const url = `${appState.apiEndpoint}/api/providers/me/jobs?limit=10`;
    
    const response = await axios.get(url, {
      headers: {
        'Authorization': `Bearer ${appState.oauthToken}`,
      },
      timeout: 10000,
    });

    if (response.data.success && response.data.jobs) {
      const jobs: Job[] = response.data.jobs;
      mainWindow?.webContents.send('jobs-polled', { jobs });

      // Find a queued job to process
      const queuedJob = jobs.find(j => j.status === 'queued');
      if (queuedJob) {
        processJob(queuedJob);
      }
    }
  } catch (error: any) {
    const errorMsg = error.response?.data?.error || error.message;
    addLog(`❌ Job polling failed: ${errorMsg}`);
  }
}

async function processJob(job: Job) {
  if (isProcessingJob) return;
  
  isProcessingJob = true;
  appState.currentJob = job;
  
  addLog(`🚀 Starting Job: ${job.id}`);
  mainWindow?.webContents.send('job-started', job);

  try {
    // 1. Update job status to running
    await updateJobStatus(job.id, 'running');
    addLog(`📝 Job ${job.id} status set to running`);

    // 2. Download the input file
    const tempDir = getTempJobsDir();
    const inputFile = path.join(tempDir, `${job.id}_input.blend`);
    const downloadUrl = `${appState.apiEndpoint}${job.config_ref}`;
    
    addLog(`📥 Downloading input file...`);
    await downloadFile(downloadUrl, inputFile);
    addLog(`✅ Input file downloaded`);

    // 3. Create GPU setup script
    const gpuScriptPath = path.join(tempDir, `${job.id}_gpu_setup.py`);
    createGpuSetupScript(gpuScriptPath);

    // 4. Run Blender
    const outputPrefix = path.join(tempDir, `${job.id}_out_`);
    addLog(`🎬 Launching Blender...`);
    
    const exitCode = await runBlender(inputFile, gpuScriptPath, outputPrefix);
    
    // 5. Clean up input and script files
    fs.unlinkSync(inputFile);
    if (fs.existsSync(gpuScriptPath)) fs.unlinkSync(gpuScriptPath);

    if (exitCode !== 0) {
      throw new Error(`Blender exited with code ${exitCode}`);
    }

    // 6. Find and upload output file
    const outputFile = `${outputPrefix}0001.png`;
    if (!fs.existsSync(outputFile)) {
      throw new Error('Output file not generated');
    }

    addLog(`📤 Uploading result...`);
    await uploadResult(job.id, outputFile);
    
    // Clean up output
    fs.unlinkSync(outputFile);

    addLog(`✅ Job ${job.id} COMPLETED`);
    appState.jobsCompleted++;
    mainWindow?.webContents.send('job-completed', { jobId: job.id, success: true });

  } catch (error: any) {
    addLog(`❌ Job ${job.id} FAILED: ${error.message}`);
    appState.jobsFailed++;
    
    try {
      await updateJobStatus(job.id, 'failed', error.message);
    } catch (e) {
      addLog(`❌ Failed to update job status: ${e}`);
    }
    
    mainWindow?.webContents.send('job-completed', { jobId: job.id, success: false, error: error.message });
  } finally {
    isProcessingJob = false;
    appState.currentJob = null;
    
    // Poll again shortly
    setTimeout(() => pollForJobs(), 2000);
  }
}

async function updateJobStatus(jobId: string, status: string, failureReason?: string) {
  const url = `${appState.apiEndpoint}/api/jobs/${jobId}/status`;
  
  const payload: any = { status };
  if (failureReason) payload.failure_reason = failureReason;

  await axios.post(url, payload, {
    headers: {
      'Authorization': `Bearer ${appState.oauthToken}`,
      'Content-Type': 'application/json',
    },
    timeout: 10000,
  });
}

async function downloadFile(url: string, savePath: string) {
  const response = await axios.get(url, {
    responseType: 'arraybuffer',
    timeout: 60000,
  });

  fs.writeFileSync(savePath, Buffer.from(response.data));
}

async function uploadResult(jobId: string, filePath: string) {
  const url = `${appState.apiEndpoint}/api/jobs/${jobId}/complete`;
  
  const formData = new FormData();
  formData.append('file', fs.createReadStream(filePath), 'result.png');

  await axios.post(url, formData, {
    headers: {
      'Authorization': `Bearer ${appState.oauthToken}`,
      ...formData.getHeaders(),
    },
    timeout: 120000,
  });
}

function createGpuSetupScript(scriptPath: string) {
  const script = `import bpy

def enable_gpu():
    print('[GPU-SETUP] Attempting to enable GPU rendering...')
    try:
        prop = bpy.context.preferences.addons['cycles'].preferences
        prop.refresh_devices()
        
        # Priority: OPTIX > CUDA > HIP > METAL > ONEAPI
        device_types = ['OPTIX', 'CUDA', 'HIP', 'METAL', 'ONEAPI']
        found = False
        
        for device_type in device_types:
            devices = prop.get_devices_for_type(device_type)
            if devices:
                print(f'[GPU-SETUP] Found {device_type} devices:')
                for d in devices:
                    print(f' - {d.name}')
                    d.use = True
                prop.compute_device_type = device_type
                found = True
                break
        
        if found:
            bpy.context.scene.cycles.device = 'GPU'
            print('[GPU-SETUP] GPU rendering enabled.')
        else:
            print('[GPU-SETUP] No compatible GPU devices found. Falling back to CPU.')
            bpy.context.scene.cycles.device = 'CPU'
    except Exception as e:
        print(f'[GPU-SETUP] Error enabling GPU: {e}')

try:
    enable_gpu()
except Exception as e:
    print(f'[GPU-SETUP] Script failed: {e}')
`;

  fs.writeFileSync(scriptPath, script);
}

function runBlender(inputFile: string, gpuScriptPath: string, outputPrefix: string): Promise<number> {
  return new Promise((resolve) => {
    const blenderPath = getBlenderPath();
    
    if (!fs.existsSync(blenderPath)) {
      addLog(`❌ Blender not found at: ${blenderPath}`);
      resolve(1);
      return;
    }

    const args = ['-b', inputFile];
    
    if (fs.existsSync(gpuScriptPath)) {
      args.push('-P', gpuScriptPath);
    }
    
    args.push('-o', outputPrefix, '-f', '1');

    addLog(`🎬 Running: blender ${args.join(' ')}`);
    
    blenderProcess = spawn(blenderPath, args, {
      cwd: getBackendPath(),
    });

    blenderProcess.stdout?.on('data', (data) => {
      const output = data.toString().trim();
      if (output) {
        output.split('\n').forEach((line: string) => {
          addLog(`[Blender] ${line}`);
        });
      }
    });

    blenderProcess.stderr?.on('data', (data) => {
      const output = data.toString().trim();
      if (output) {
        output.split('\n').forEach((line: string) => {
          addLog(`[Blender ERR] ${line}`);
        });
      }
    });

    blenderProcess.on('close', (code) => {
      blenderProcess = null;
      resolve(code || 0);
    });

    blenderProcess.on('error', (err) => {
      addLog(`❌ Blender process error: ${err.message}`);
      blenderProcess = null;
      resolve(1);
    });
  });
}

// Get real system info
async function getSystemInfo(): Promise<SystemInfo> {
  const cpus = os.cpus();
  const totalMem = os.totalmem();
  const freeMem = os.freemem();
  const usedMem = totalMem - freeMem;
  const memUsagePercent = Math.round((usedMem / totalMem) * 100);

  const cpuModel = cpus[0]?.model || 'Unknown CPU';
  const cpuVendor = cpuModel.includes('Intel') ? 'Intel' : 
                    cpuModel.includes('AMD') ? 'AMD' : 
                    cpuModel.includes('Apple') ? 'Apple' : 'Unknown';
  const cpuFrequency = cpus[0]?.speed || 0;

  const platform = os.platform();
  const osName = platform === 'win32' ? 'Windows' :
                 platform === 'darwin' ? 'macOS' :
                 platform === 'linux' ? 'Linux' : platform;
  const osVersion = os.release();
  const arch = os.arch();
  const hostname = os.hostname();

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
      addLog(`✅ Loaded GPU info from ${gpuInfoPath}`);
    } else {
      addLog(`⚠️ GPU info file not found, using system detection`);
      
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
          addLog(`✅ Detected ${gpus.length} GPU(s) using WMIC`);
        } catch (e) {
          addLog(`⚠️ Failed to detect GPU using WMIC: ${e}`);
        }
      }
    }
  } catch (error) {
    addLog(`⚠️ Error loading GPU info: ${error}`);
  }

  if (gpus.length === 0) {
    gpus.push({
      name: 'No GPU detected',
      vram_total_bytes: 0,
      vram_used_bytes: 0,
      temperature_celsius: 0,
      utilization_percent: 0,
    });
  }

  return {
    cpu: {
      vendor: cpuVendor,
      model: cpuModel,
      cores: cpus.length > 0 ? Math.floor(cpus.length / 2) : 1,
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
    nodeId: appState.nodeId,
  };
}

// ============ IPC Handlers ============
ipcMain.handle('get-system-info', async () => {
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
    nodeId: appState.nodeId,
    heartbeatCount: appState.heartbeatCount,
    jobsCompleted: appState.jobsCompleted,
    jobsFailed: appState.jobsFailed,
    currentJob: appState.currentJob,
    gpuRates: appState.gpuRates,
  };
});

ipcMain.handle('toggle-online', async () => {
  toggleOnlineStatus();
  return { isOnline: appState.isOnline };
});

ipcMain.handle('set-oauth-token', async (_event, token: string) => {
  if (!token || token.trim() === '') {
    addLog('❌ Cannot save empty OAuth token');
    return { success: false };
  }
  
  appState.oauthToken = token.trim();
  store.set('oauthToken', token.trim());
  
  // Verify it was saved
  const savedToken = store.get('oauthToken') as string;
  const verified = savedToken === token.trim();
  
  addLog(`✅ OAuth token saved (${token.length} chars, verified: ${verified})`);
  addLog(`🔑 Token preview: ${token.substring(0, 30)}...`);
  
  return { success: verified };
});

ipcMain.handle('set-api-endpoint', async (_event, endpoint: string) => {
  appState.apiEndpoint = endpoint;
  store.set('apiEndpoint', endpoint);
  addLog(`✅ API endpoint updated: ${endpoint}`);
  return { success: true };
});

ipcMain.handle('set-intervals', async (_event, { heartbeat, jobPolling }: { heartbeat: number, jobPolling: number }) => {
  appState.heartbeatInterval = heartbeat;
  appState.jobPollingInterval = jobPolling;
  store.set('heartbeatInterval', heartbeat);
  store.set('jobPollingInterval', jobPolling);

  if (appState.isOnline) {
    stopHeartbeat();
    stopJobPolling();
    startHeartbeat();
    startJobPolling();
  }

  addLog(`✅ Intervals updated: Heartbeat=${heartbeat}s, Job Polling=${jobPolling}s`);
  return { success: true };
});

ipcMain.handle('set-gpu-rates', async (_event, rates: Array<{ gpu_index: number; hourly_price_cents: number }>) => {
  appState.gpuRates = rates;
  store.set('gpuRates', rates);
  addLog(`✅ GPU rates updated`);
  return { success: true };
});

ipcMain.handle('get-logs', async () => {
  return appState.logs;
});

ipcMain.handle('clear-logs', async () => {
  appState.logs = [];
  addLog('🗑️ Logs cleared');
  return { success: true };
});

ipcMain.handle('refresh-system-info', async () => {
  appState.systemInfo = await getSystemInfo();
  addLog('🔄 System info refreshed');
  return appState.systemInfo;
});

// ============ App lifecycle ============
app.whenReady().then(async () => {
  appState.systemInfo = await getSystemInfo();
  addLog('🚀 GPU Marketplace Provider Agent started');
  addLog(`📍 Node ID: ${appState.nodeId}`);
  addLog(`🌐 API Endpoint: ${appState.apiEndpoint}`);
  addLog(`💻 System: ${appState.systemInfo.cpu.model}, ${(appState.systemInfo.memory.total_bytes / 1024 / 1024 / 1024).toFixed(1)}GB RAM`);
  
  if (appState.systemInfo.gpus.length > 0 && appState.systemInfo.gpus[0].name !== 'No GPU detected') {
    addLog(`🎮 GPU: ${appState.systemInfo.gpus.map(g => g.name).join(', ')}`);
  }
  
  // Log token status
  if (appState.oauthToken) {
    addLog(`🔑 OAuth token loaded (${appState.oauthToken.length} chars): ${appState.oauthToken.substring(0, 30)}...`);
  } else {
    addLog('⚠️ No OAuth token found. Please configure in Settings.');
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
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('before-quit', () => {
  stopHeartbeat();
  stopJobPolling();

  if (blenderProcess) {
    blenderProcess.kill();
  }
});

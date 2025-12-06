import { contextBridge, ipcRenderer } from 'electron';

export interface SystemInfo {
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

export interface AppState {
  isOnline: boolean;
  apiEndpoint: string;
  oauthToken: string | null;
  heartbeatInterval: number;
  jobPollingInterval: number;
}

// Expose protected methods that allow the renderer process to use
// ipcRenderer without exposing the entire object
contextBridge.exposeInMainWorld('electronAPI', {
  // System info
  getSystemInfo: (): Promise<SystemInfo> => ipcRenderer.invoke('get-system-info'),

  // App state
  getAppState: (): Promise<AppState> => ipcRenderer.invoke('get-app-state'),
  toggleOnline: (): Promise<{ isOnline: boolean }> => ipcRenderer.invoke('toggle-online'),

  // Settings
  setOAuthToken: (token: string): Promise<{ success: boolean }> =>
    ipcRenderer.invoke('set-oauth-token', token),
  setApiEndpoint: (endpoint: string): Promise<{ success: boolean }> =>
    ipcRenderer.invoke('set-api-endpoint', endpoint),
  setIntervals: (intervals: { heartbeat: number; jobPolling: number }): Promise<{ success: boolean }> =>
    ipcRenderer.invoke('set-intervals', intervals),

  // Logs
  getLogs: (): Promise<string[]> => ipcRenderer.invoke('get-logs'),
  clearLogs: (): Promise<{ success: boolean }> => ipcRenderer.invoke('clear-logs'),

  // Event listeners
  onStatusChanged: (callback: (data: { isOnline: boolean }) => void) => {
    ipcRenderer.on('status-changed', (_event, data) => callback(data));
  },
  onLogUpdate: (callback: (log: string) => void) => {
    ipcRenderer.on('log-update', (_event, log) => callback(log));
  },
  onAuthRequired: (callback: () => void) => {
    ipcRenderer.on('auth-required', () => callback());
  },
  onHeartbeatSent: (callback: (data: { success: boolean; error?: any }) => void) => {
    ipcRenderer.on('heartbeat-sent', (_event, data) => callback(data));
  },
  onJobsPolled: (callback: (data: { jobs: any[] }) => void) => {
    ipcRenderer.on('jobs-polled', (_event, data) => callback(data));
  },

  // Remove listeners
  removeAllListeners: (channel: string) => {
    ipcRenderer.removeAllListeners(channel);
  },
});

// Type declaration for TypeScript
declare global {
  interface Window {
    electronAPI: {
      getSystemInfo: () => Promise<SystemInfo>;
      getAppState: () => Promise<AppState>;
      toggleOnline: () => Promise<{ isOnline: boolean }>;
      setOAuthToken: (token: string) => Promise<{ success: boolean }>;
      setApiEndpoint: (endpoint: string) => Promise<{ success: boolean }>;
      setIntervals: (intervals: { heartbeat: number; jobPolling: number }) => Promise<{ success: boolean }>;
      getLogs: () => Promise<string[]>;
      clearLogs: () => Promise<{ success: boolean }>;
      onStatusChanged: (callback: (data: { isOnline: boolean }) => void) => void;
      onLogUpdate: (callback: (log: string) => void) => void;
      onAuthRequired: (callback: () => void) => void;
      onHeartbeatSent: (callback: (data: { success: boolean; error?: any }) => void) => void;
      onJobsPolled: (callback: (data: { jobs: any[] }) => void) => void;
      removeAllListeners: (channel: string) => void;
    };
  }
}

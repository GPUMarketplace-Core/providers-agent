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

export {};

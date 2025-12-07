import { useState, useEffect } from 'react';
import { SystemInfo } from '../../../electron/preload';

interface DashboardProps {
  isOnline: boolean;
  stats: {
    heartbeatCount: number;
    jobsCompleted: number;
    jobsFailed: number;
    nodeId: string;
  };
}

const Dashboard = ({ isOnline, stats }: DashboardProps) => {
  const [systemInfo, setSystemInfo] = useState<SystemInfo | null>(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    loadSystemInfo();
  }, []);

  const loadSystemInfo = async () => {
    try {
      const info = await window.electronAPI.getSystemInfo();
      setSystemInfo(info);
    } catch (error) {
      console.error('Failed to load system info:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleRefresh = async () => {
    setLoading(true);
    try {
      const info = await window.electronAPI.refreshSystemInfo();
      setSystemInfo(info);
    } catch (error) {
      console.error('Failed to refresh system info:', error);
    } finally {
      setLoading(false);
    }
  };

  if (loading) {
    return (
      <div className="flex items-center justify-center h-full">
        <div className="text-center">
          <div className="w-16 h-16 border-4 border-violet-500 border-t-transparent rounded-full animate-spin mx-auto"></div>
          <p className="mt-4 text-slate-400">Loading system information...</p>
        </div>
      </div>
    );
  }

  if (!systemInfo) {
    return (
      <div className="flex items-center justify-center h-full">
        <div className="text-center">
          <p className="text-red-400 mb-4">Failed to load system information</p>
          <button
            onClick={handleRefresh}
            className="px-4 py-2 bg-violet-600 text-white rounded-lg hover:bg-violet-700 transition-colors"
          >
            Retry
          </button>
        </div>
      </div>
    );
  }

  const totalMemoryGB = (systemInfo.memory.total_bytes / 1024 / 1024 / 1024).toFixed(1);
  const availableMemoryGB = (systemInfo.memory.available_bytes / 1024 / 1024 / 1024).toFixed(1);

  return (
    <div className="space-y-6 animate-fade-in">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h2 className="text-2xl font-bold text-white">Dashboard</h2>
          <p className="text-slate-400 text-sm mt-1">Monitor your provider status and system resources</p>
        </div>
        <button
          onClick={handleRefresh}
          className="px-4 py-2 bg-slate-800 hover:bg-slate-700 text-slate-300 rounded-lg transition-colors flex items-center gap-2 border border-white/10"
        >
          <span>🔄</span>
          Refresh
        </button>
      </div>

      {/* Status Banner */}
      <div className={`p-6 rounded-2xl border ${
        isOnline 
          ? 'bg-gradient-to-r from-emerald-500/10 to-green-500/10 border-emerald-500/20' 
          : 'bg-gradient-to-r from-slate-800/50 to-slate-700/50 border-white/5'
      }`}>
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-4">
            <div className={`w-4 h-4 rounded-full ${isOnline ? 'bg-emerald-400 animate-pulse' : 'bg-slate-500'}`} />
            <div>
              <h3 className="text-lg font-semibold text-white">
                {isOnline ? 'Provider Online' : 'Provider Offline'}
              </h3>
              <p className="text-sm text-slate-400">
                {isOnline ? 'Actively accepting and processing render jobs' : 'Toggle "Go Online" to start accepting jobs'}
              </p>
            </div>
          </div>
          {isOnline && (
            <div className="flex gap-6">
              <Stat label="Heartbeats" value={stats.heartbeatCount.toString()} color="cyan" />
              <Stat label="Completed" value={stats.jobsCompleted.toString()} color="emerald" />
              <Stat label="Failed" value={stats.jobsFailed.toString()} color="red" />
            </div>
          )}
        </div>
      </div>

      {/* Stats Grid */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
        <StatCard
          title="CPU Cores"
          value={systemInfo.cpu.logical_processors.toString()}
          subtitle={`${systemInfo.cpu.cores} physical cores`}
          icon="🔥"
          gradient="from-orange-500 to-red-500"
        />
        <StatCard
          title="Memory"
          value={`${totalMemoryGB} GB`}
          subtitle={`${availableMemoryGB} GB available`}
          icon="💾"
          gradient="from-violet-500 to-purple-500"
        />
        <StatCard
          title="GPUs"
          value={systemInfo.gpus.length.toString()}
          subtitle={systemInfo.gpus[0]?.name?.substring(0, 24) || 'No GPU'}
          icon="🎮"
          gradient="from-cyan-500 to-blue-500"
        />
        <StatCard
          title="CPU Freq"
          value={`${(systemInfo.cpu.frequency_mhz / 1000).toFixed(1)} GHz`}
          subtitle={systemInfo.cpu.vendor}
          icon="⚡"
          gradient="from-yellow-500 to-orange-500"
        />
      </div>

      {/* System Information Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* CPU Info */}
        <InfoCard title="CPU Information" icon="🖥️">
          <InfoRow label="Vendor" value={systemInfo.cpu.vendor} />
          <InfoRow label="Model" value={systemInfo.cpu.model} />
          <InfoRow label="Physical Cores" value={systemInfo.cpu.cores.toString()} />
          <InfoRow label="Logical Processors" value={systemInfo.cpu.logical_processors.toString()} />
          <InfoRow label="Base Frequency" value={`${systemInfo.cpu.frequency_mhz} MHz`} />
        </InfoCard>

        {/* Memory Info */}
        <InfoCard title="Memory" icon="💾">
          <InfoRow label="Total Memory" value={`${totalMemoryGB} GB`} />
          <InfoRow label="Available" value={`${availableMemoryGB} GB`} />
          <InfoRow label="Usage" value={`${systemInfo.memory.usage_percent}%`} />
          <div className="mt-4">
            <div className="w-full bg-slate-700 rounded-full h-2.5 overflow-hidden">
              <div
                className="bg-gradient-to-r from-violet-500 to-fuchsia-500 h-full rounded-full transition-all duration-500"
                style={{ width: `${systemInfo.memory.usage_percent}%` }}
              />
            </div>
            <div className="flex justify-between text-xs text-slate-500 mt-1">
              <span>0 GB</span>
              <span>{totalMemoryGB} GB</span>
            </div>
          </div>
        </InfoCard>

        {/* GPU Info */}
        <InfoCard title="GPU Information" icon="🎮">
          {systemInfo.gpus.length > 0 && systemInfo.gpus[0].name !== 'No GPU detected' ? (
            <div className="space-y-3">
              {systemInfo.gpus.map((gpu, index) => (
                <div key={index} className="p-4 bg-slate-800/50 rounded-xl border border-white/5">
                  <div className="flex items-start justify-between">
                    <div>
                      <p className="font-semibold text-white">{gpu.name}</p>
                      {gpu.vram_total_bytes > 0 && (
                        <p className="text-sm text-slate-400 mt-1">
                          VRAM: {(gpu.vram_total_bytes / 1024 / 1024 / 1024).toFixed(1)} GB
                        </p>
                      )}
                    </div>
                    <span className="px-2 py-1 bg-emerald-500/20 text-emerald-400 text-xs rounded-lg font-medium">
                      GPU {index}
                    </span>
                  </div>
                </div>
              ))}
            </div>
          ) : (
            <div className="text-center py-8">
              <div className="text-4xl mb-3">🎮</div>
              <p className="text-slate-400">No discrete GPU detected</p>
              <p className="text-slate-500 text-sm mt-1">Rendering will use CPU</p>
            </div>
          )}
        </InfoCard>

        {/* OS Info */}
        <InfoCard title="Operating System" icon="💻">
          <InfoRow label="OS" value={systemInfo.os.name} />
          <InfoRow label="Version" value={systemInfo.os.version} />
          <InfoRow label="Architecture" value={systemInfo.os.architecture} />
          <InfoRow label="Hostname" value={systemInfo.os.hostname} />
          <div className="mt-4 pt-4 border-t border-white/5">
            <div className="text-xs text-slate-500 mb-1">Node ID</div>
            <div className="font-mono text-sm text-cyan-400 break-all">{stats.nodeId}</div>
          </div>
        </InfoCard>
      </div>
    </div>
  );
};

const Stat = ({ label, value, color }: { label: string; value: string; color: string }) => {
  const colorClasses: Record<string, string> = {
    cyan: 'text-cyan-400',
    emerald: 'text-emerald-400',
    red: 'text-red-400',
  };
  
  return (
    <div className="text-center">
      <div className={`text-2xl font-bold font-mono ${colorClasses[color]}`}>{value}</div>
      <div className="text-xs text-slate-500">{label}</div>
    </div>
  );
};

const StatCard = ({ 
  title, 
  value, 
  subtitle, 
  icon, 
  gradient 
}: { 
  title: string; 
  value: string; 
  subtitle: string; 
  icon: string; 
  gradient: string;
}) => (
  <div className="p-5 rounded-2xl bg-slate-800/50 border border-white/5 backdrop-blur-sm hover:border-white/10 transition-colors">
    <div className="flex items-start justify-between mb-3">
      <div className={`w-10 h-10 rounded-xl bg-gradient-to-br ${gradient} flex items-center justify-center text-lg shadow-lg`}>
        {icon}
      </div>
    </div>
    <div className="text-2xl font-bold text-white mb-1">{value}</div>
    <div className="text-sm text-slate-400">{title}</div>
    <div className="text-xs text-slate-500 mt-1 truncate">{subtitle}</div>
  </div>
);

const InfoCard = ({ 
  title, 
  icon, 
  children 
}: { 
  title: string; 
  icon: string; 
  children: React.ReactNode;
}) => (
  <div className="p-6 rounded-2xl bg-slate-800/50 border border-white/5 backdrop-blur-sm">
    <div className="flex items-center gap-3 mb-5">
      <span className="text-xl">{icon}</span>
      <h3 className="text-lg font-semibold text-white">{title}</h3>
    </div>
    <div className="space-y-3">{children}</div>
  </div>
);

const InfoRow = ({ label, value }: { label: string; value: string }) => (
  <div className="flex justify-between items-center">
    <span className="text-sm text-slate-400">{label}</span>
    <span className="text-sm font-medium text-white">{value}</span>
  </div>
);

export default Dashboard;

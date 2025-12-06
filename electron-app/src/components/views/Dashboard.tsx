import { useState, useEffect } from 'react';
import { SystemInfo } from '../../../electron/preload';
import StatCard from '../shared/StatCard';
import SystemCard from '../shared/SystemCard';

const Dashboard = () => {
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

  if (loading) {
    return (
      <div className="flex items-center justify-center h-full">
        <div className="text-center">
          <div className="w-16 h-16 border-4 border-violet-600 border-t-transparent rounded-full animate-spin mx-auto"></div>
          <p className="mt-4 text-gray-600">Loading system information...</p>
        </div>
      </div>
    );
  }

  if (!systemInfo) {
    return (
      <div className="flex items-center justify-center h-full">
        <p className="text-red-600">Failed to load system information</p>
      </div>
    );
  }

  const totalMemoryGB = (systemInfo.memory.total_bytes / 1024 / 1024 / 1024).toFixed(2);
  const availableMemoryGB = (systemInfo.memory.available_bytes / 1024 / 1024 / 1024).toFixed(2);

  return (
    <div className="space-y-6 animate-fade-in-up">
      {/* Header */}
      <div className="glass rounded-3xl p-8 shadow-xl">
        <h2 className="text-3xl font-bold gradient-text mb-2">Dashboard</h2>
        <p className="text-gray-600">Monitor your system and provider status</p>
      </div>

      {/* Stats Grid */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6">
        <StatCard
          title="CPU Cores"
          value={systemInfo.cpu.cores.toString()}
          subtitle={`${systemInfo.cpu.logical_processors} logical processors`}
          gradient="from-violet-500 to-purple-600"
          icon="🔥"
        />
        <StatCard
          title="Memory"
          value={`${totalMemoryGB} GB`}
          subtitle={`${availableMemoryGB} GB available`}
          gradient="from-fuchsia-500 to-pink-600"
          icon="💾"
        />
        <StatCard
          title="GPUs"
          value={systemInfo.gpus.length.toString()}
          subtitle={systemInfo.gpus[0]?.name || 'No GPU detected'}
          gradient="from-blue-500 to-cyan-600"
          icon="🎮"
        />
        <StatCard
          title="CPU Frequency"
          value={`${systemInfo.cpu.frequency_mhz} MHz`}
          subtitle={systemInfo.cpu.vendor}
          gradient="from-green-500 to-emerald-600"
          icon="⚡"
        />
      </div>

      {/* System Information Cards */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* CPU Info */}
        <SystemCard title="CPU Information" icon="🖥️">
          <div className="space-y-3">
            <InfoRow label="Vendor" value={systemInfo.cpu.vendor} />
            <InfoRow label="Model" value={systemInfo.cpu.model} />
            <InfoRow label="Cores" value={systemInfo.cpu.cores.toString()} />
            <InfoRow
              label="Logical Processors"
              value={systemInfo.cpu.logical_processors.toString()}
            />
            <InfoRow
              label="Frequency"
              value={`${systemInfo.cpu.frequency_mhz} MHz`}
            />
          </div>
        </SystemCard>

        {/* Memory Info */}
        <SystemCard title="Memory Information" icon="💾">
          <div className="space-y-3">
            <InfoRow label="Total Memory" value={`${totalMemoryGB} GB`} />
            <InfoRow label="Available Memory" value={`${availableMemoryGB} GB`} />
            <InfoRow
              label="Usage"
              value={`${systemInfo.memory.usage_percent}%`}
            />
            <div className="pt-2">
              <div className="w-full bg-gray-200 rounded-full h-3 overflow-hidden">
                <div
                  className="bg-gradient-to-r from-violet-600 to-fuchsia-600 h-full rounded-full transition-all duration-500"
                  style={{ width: `${systemInfo.memory.usage_percent}%` }}
                />
              </div>
            </div>
          </div>
        </SystemCard>

        {/* GPU Info */}
        <SystemCard title="GPU Information" icon="🎮">
          {systemInfo.gpus.length > 0 ? (
            <div className="space-y-4">
              {systemInfo.gpus.map((gpu, index) => (
                <div key={index} className="p-4 bg-gray-50 rounded-xl">
                  <p className="font-semibold text-gray-900">{gpu.name}</p>
                  {gpu.vram_total_bytes > 0 && (
                    <p className="text-sm text-gray-600 mt-1">
                      VRAM: {(gpu.vram_total_bytes / 1024 / 1024 / 1024).toFixed(2)} GB
                    </p>
                  )}
                </div>
              ))}
            </div>
          ) : (
            <p className="text-gray-500 text-center py-4">No GPUs detected</p>
          )}
        </SystemCard>

        {/* OS Info */}
        <SystemCard title="Operating System" icon="💻">
          <div className="space-y-3">
            <InfoRow label="OS Name" value={systemInfo.os.name} />
            <InfoRow label="Version" value={systemInfo.os.version} />
            <InfoRow label="Architecture" value={systemInfo.os.architecture} />
            <InfoRow label="Hostname" value={systemInfo.os.hostname} />
          </div>
        </SystemCard>
      </div>
    </div>
  );
};

const InfoRow = ({ label, value }: { label: string; value: string }) => (
  <div className="flex justify-between items-center">
    <span className="text-sm text-gray-600">{label}</span>
    <span className="text-sm font-semibold text-gray-900">{value}</span>
  </div>
);

export default Dashboard;

import { useState, useEffect } from 'react';

const Settings = () => {
  const [oauthToken, setOauthToken] = useState('');
  const [apiEndpoint, setApiEndpoint] = useState('http://localhost:3000');
  const [heartbeatInterval, setHeartbeatInterval] = useState(2);
  const [jobPollingInterval, setJobPollingInterval] = useState(5);
  const [gpuRate, setGpuRate] = useState(50); // dollars per hour
  const [showToken, setShowToken] = useState(false);
  const [saved, setSaved] = useState(false);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    loadSettings();
  }, []);

  const loadSettings = async () => {
    try {
      const state = await window.electronAPI.getAppState();
      setApiEndpoint(state.apiEndpoint);
      setHeartbeatInterval(state.heartbeatInterval);
      setJobPollingInterval(state.jobPollingInterval);
      if (state.gpuRates && state.gpuRates.length > 0) {
        setGpuRate(state.gpuRates[0].hourly_price_cents / 100);
      }
    } catch (error) {
      console.error('Failed to load settings:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleSaveSettings = async () => {
    try {
      if (oauthToken) {
        await window.electronAPI.setOAuthToken(oauthToken);
      }
      
      await window.electronAPI.setApiEndpoint(apiEndpoint);
      await window.electronAPI.setIntervals({
        heartbeat: heartbeatInterval,
        jobPolling: jobPollingInterval,
      });
      await window.electronAPI.setGpuRates([
        { gpu_index: 0, hourly_price_cents: Math.round(gpuRate * 100) }
      ]);

      setSaved(true);
      setTimeout(() => setSaved(false), 3000);
    } catch (error) {
      console.error('Failed to save settings:', error);
    }
  };

  if (loading) {
    return (
      <div className="flex items-center justify-center h-full">
        <div className="w-8 h-8 border-2 border-violet-500 border-t-transparent rounded-full animate-spin" />
      </div>
    );
  }

  return (
    <div className="space-y-6 animate-fade-in max-w-3xl">
      {/* Header */}
      <div>
        <h2 className="text-2xl font-bold text-white">Settings</h2>
        <p className="text-slate-400 text-sm mt-1">Configure your provider agent</p>
      </div>

      {/* Authentication */}
      <div className="p-6 rounded-2xl bg-slate-800/50 border border-white/5 backdrop-blur-sm">
        <div className="flex items-center gap-3 mb-6">
          <div className="w-10 h-10 rounded-xl bg-gradient-to-br from-violet-500 to-purple-500 flex items-center justify-center">
            🔐
          </div>
          <div>
            <h3 className="text-lg font-semibold text-white">Authentication</h3>
            <p className="text-sm text-slate-400">Connect to GPU Marketplace</p>
          </div>
        </div>

        <div className="space-y-5">
          {/* OAuth Token */}
          <div>
            <label className="block text-sm font-medium text-slate-300 mb-2">
              Provider Token
            </label>
            <div className="relative">
              <input
                type={showToken ? 'text' : 'password'}
                value={oauthToken}
                onChange={(e) => setOauthToken(e.target.value)}
                placeholder="Paste your OAuth token here"
                className="w-full px-4 py-3 text-sm bg-slate-900/50 border border-white/10 rounded-xl focus:ring-2 focus:ring-violet-500 focus:border-transparent outline-none transition-all text-white placeholder-slate-500 pr-20"
              />
              <button
                onClick={() => setShowToken(!showToken)}
                className="absolute right-3 top-1/2 -translate-y-1/2 px-3 py-1 text-xs text-violet-400 hover:text-violet-300 hover:bg-violet-500/10 rounded-lg transition-colors"
              >
                {showToken ? 'Hide' : 'Show'}
              </button>
            </div>
            <p className="mt-2 text-xs text-slate-500">
              Get your token from the GPU Marketplace provider dashboard after signing in with Google
            </p>
          </div>

          {/* API Endpoint */}
          <div>
            <label className="block text-sm font-medium text-slate-300 mb-2">
              API Endpoint
            </label>
            <input
              type="text"
              value={apiEndpoint}
              onChange={(e) => setApiEndpoint(e.target.value)}
              placeholder="https://api.gpumarketplace.com"
              className="w-full px-4 py-3 text-sm bg-slate-900/50 border border-white/10 rounded-xl focus:ring-2 focus:ring-violet-500 focus:border-transparent outline-none transition-all text-white placeholder-slate-500"
            />
            <p className="mt-2 text-xs text-slate-500">
              The URL of the GPU Marketplace API server
            </p>
          </div>
        </div>
      </div>

      {/* Pricing */}
      <div className="p-6 rounded-2xl bg-slate-800/50 border border-white/5 backdrop-blur-sm">
        <div className="flex items-center gap-3 mb-6">
          <div className="w-10 h-10 rounded-xl bg-gradient-to-br from-emerald-500 to-green-500 flex items-center justify-center">
            💰
          </div>
          <div>
            <h3 className="text-lg font-semibold text-white">Pricing</h3>
            <p className="text-sm text-slate-400">Set your GPU hourly rate</p>
          </div>
        </div>

        <div>
          <label className="block text-sm font-medium text-slate-300 mb-2">
            Hourly Rate (USD)
          </label>
          <div className="relative">
            <span className="absolute left-4 top-1/2 -translate-y-1/2 text-slate-400">$</span>
            <input
              type="number"
              min="0"
              step="0.01"
              value={gpuRate}
              onChange={(e) => setGpuRate(parseFloat(e.target.value) || 0)}
              className="w-full pl-8 pr-16 py-3 text-sm bg-slate-900/50 border border-white/10 rounded-xl focus:ring-2 focus:ring-violet-500 focus:border-transparent outline-none transition-all text-white"
            />
            <span className="absolute right-4 top-1/2 -translate-y-1/2 text-slate-500 text-sm">/hour</span>
          </div>
          <p className="mt-2 text-xs text-slate-500">
            This is the amount you'll earn per hour of GPU usage
          </p>
        </div>
      </div>

      {/* Intervals */}
      <div className="p-6 rounded-2xl bg-slate-800/50 border border-white/5 backdrop-blur-sm">
        <div className="flex items-center gap-3 mb-6">
          <div className="w-10 h-10 rounded-xl bg-gradient-to-br from-cyan-500 to-blue-500 flex items-center justify-center">
            ⏱️
          </div>
          <div>
            <h3 className="text-lg font-semibold text-white">Timing</h3>
            <p className="text-sm text-slate-400">Configure polling intervals</p>
          </div>
        </div>

        <div className="grid grid-cols-2 gap-4">
          <div>
            <label className="block text-sm font-medium text-slate-300 mb-2">
              Heartbeat Interval
            </label>
            <div className="relative">
              <input
                type="number"
                min="1"
                max="60"
                value={heartbeatInterval}
                onChange={(e) => setHeartbeatInterval(parseInt(e.target.value) || 2)}
                className="w-full px-4 py-3 text-sm bg-slate-900/50 border border-white/10 rounded-xl focus:ring-2 focus:ring-violet-500 focus:border-transparent outline-none transition-all text-white pr-16"
              />
              <span className="absolute right-4 top-1/2 -translate-y-1/2 text-slate-500 text-sm">sec</span>
            </div>
          </div>
          
          <div>
            <label className="block text-sm font-medium text-slate-300 mb-2">
              Job Polling Interval
            </label>
            <div className="relative">
              <input
                type="number"
                min="1"
                max="60"
                value={jobPollingInterval}
                onChange={(e) => setJobPollingInterval(parseInt(e.target.value) || 5)}
                className="w-full px-4 py-3 text-sm bg-slate-900/50 border border-white/10 rounded-xl focus:ring-2 focus:ring-violet-500 focus:border-transparent outline-none transition-all text-white pr-16"
              />
              <span className="absolute right-4 top-1/2 -translate-y-1/2 text-slate-500 text-sm">sec</span>
            </div>
          </div>
        </div>
      </div>

      {/* Save Button */}
      <div className="flex items-center gap-4">
        <button
          onClick={handleSaveSettings}
          className={`flex-1 py-3.5 px-4 rounded-xl font-semibold transition-all duration-300 transform hover:scale-[1.01] active:scale-95 ${
            saved
              ? 'bg-emerald-500 text-white'
              : 'bg-gradient-to-r from-violet-600 to-fuchsia-600 text-white shadow-lg shadow-violet-500/25 hover:shadow-violet-500/40'
          }`}
        >
          {saved ? '✓ Settings Saved!' : 'Save Settings'}
        </button>
      </div>

      {saved && (
        <div className="p-4 rounded-xl bg-emerald-500/10 border border-emerald-500/20 text-center">
          <p className="text-sm text-emerald-400 font-medium">
            Your settings have been saved successfully!
          </p>
        </div>
      )}
    </div>
  );
};

export default Settings;

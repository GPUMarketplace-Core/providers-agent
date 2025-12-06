import { useState, useEffect } from 'react';

const Settings = () => {
  const [apiEndpoint, setApiEndpoint] = useState('');
  const [oauthToken, setOauthToken] = useState('');
  const [heartbeatInterval, setHeartbeatInterval] = useState(2);
  const [jobPollingInterval, setJobPollingInterval] = useState(5);
  const [showToken, setShowToken] = useState(false);
  const [saved, setSaved] = useState(false);

  useEffect(() => {
    loadSettings();
  }, []);

  const loadSettings = async () => {
    const state = await window.electronAPI.getAppState();
    setApiEndpoint(state.apiEndpoint);
    setHeartbeatInterval(state.heartbeatInterval);
    setJobPollingInterval(state.jobPollingInterval);
  };

  const handleSaveSettings = async () => {
    try {
      if (oauthToken) {
        await window.electronAPI.setOAuthToken(oauthToken);
      }

      if (apiEndpoint) {
        await window.electronAPI.setApiEndpoint(apiEndpoint);
      }

      await window.electronAPI.setIntervals({
        heartbeat: heartbeatInterval,
        jobPolling: jobPollingInterval,
      });

      setSaved(true);
      setTimeout(() => setSaved(false), 3000);
    } catch (error) {
      console.error('Failed to save settings:', error);
    }
  };

  return (
    <div className="space-y-6 animate-fade-in-up max-w-4xl">
      {/* Header */}
      <div className="glass rounded-3xl p-8 shadow-xl">
        <h2 className="text-3xl font-bold gradient-text mb-2">Settings</h2>
        <p className="text-gray-600">Configure your provider agent</p>
      </div>

      {/* API Configuration */}
      <div className="glass rounded-3xl p-8 shadow-xl">
        <h3 className="text-xl font-bold text-gray-900 mb-6 flex items-center gap-2">
          <span>🔌</span>
          API Configuration
        </h3>

        <div className="space-y-6">
          {/* API Endpoint */}
          <div>
            <label className="block text-sm font-medium text-gray-700 mb-2">
              API Endpoint
            </label>
            <input
              type="text"
              value={apiEndpoint}
              onChange={(e) => setApiEndpoint(e.target.value)}
              placeholder="http://localhost:3000"
              className="w-full px-4 py-3 text-sm border border-gray-300 rounded-xl focus:ring-2 focus:ring-violet-500 focus:border-transparent outline-none transition-all bg-white"
            />
            <p className="mt-2 text-xs text-gray-500">
              The base URL of your GPU Marketplace API server
            </p>
          </div>

          {/* OAuth Token */}
          <div>
            <label className="block text-sm font-medium text-gray-700 mb-2">
              OAuth Token
            </label>
            <div className="relative">
              <input
                type={showToken ? 'text' : 'password'}
                value={oauthToken}
                onChange={(e) => setOauthToken(e.target.value)}
                placeholder="Enter your provider token"
                className="w-full px-4 py-3 text-sm border border-gray-300 rounded-xl focus:ring-2 focus:ring-violet-500 focus:border-transparent outline-none transition-all bg-white pr-24"
              />
              <button
                onClick={() => setShowToken(!showToken)}
                className="absolute right-2 top-1/2 -translate-y-1/2 px-3 py-1 text-xs text-violet-600 hover:bg-violet-50 rounded-lg transition-colors"
              >
                {showToken ? 'Hide' : 'Show'}
              </button>
            </div>
            <p className="mt-2 text-xs text-gray-500">
              Get your token from the provider dashboard at {apiEndpoint}
            </p>
          </div>
        </div>
      </div>

      {/* Intervals Configuration */}
      <div className="glass rounded-3xl p-8 shadow-xl">
        <h3 className="text-xl font-bold text-gray-900 mb-6 flex items-center gap-2">
          <span>⏱️</span>
          Intervals Configuration
        </h3>

        <div className="space-y-6">
          {/* Heartbeat Interval */}
          <div>
            <label className="block text-sm font-medium text-gray-700 mb-2">
              Heartbeat Interval: {heartbeatInterval} seconds
            </label>
            <input
              type="range"
              min="1"
              max="60"
              value={heartbeatInterval}
              onChange={(e) => setHeartbeatInterval(Number(e.target.value))}
              className="w-full h-2 bg-gray-200 rounded-lg appearance-none cursor-pointer accent-violet-600"
            />
            <div className="flex justify-between text-xs text-gray-500 mt-1">
              <span>1s (Frequent)</span>
              <span>60s (Less frequent)</span>
            </div>
            <p className="mt-2 text-xs text-gray-500">
              How often to send status updates to the server
            </p>
          </div>

          {/* Job Polling Interval */}
          <div>
            <label className="block text-sm font-medium text-gray-700 mb-2">
              Job Polling Interval: {jobPollingInterval} seconds
            </label>
            <input
              type="range"
              min="1"
              max="60"
              value={jobPollingInterval}
              onChange={(e) => setJobPollingInterval(Number(e.target.value))}
              className="w-full h-2 bg-gray-200 rounded-lg appearance-none cursor-pointer accent-fuchsia-600"
            />
            <div className="flex justify-between text-xs text-gray-500 mt-1">
              <span>1s (Frequent)</span>
              <span>60s (Less frequent)</span>
            </div>
            <p className="mt-2 text-xs text-gray-500">
              How often to check for new rendering jobs
            </p>
          </div>
        </div>
      </div>

      {/* Save Button */}
      <div className="glass rounded-3xl p-8 shadow-xl">
        <button
          onClick={handleSaveSettings}
          className="w-full py-3 px-4 bg-gradient-to-r from-violet-600 via-fuchsia-600 to-pink-600 hover:from-violet-700 hover:via-fuchsia-700 hover:to-pink-700 text-white rounded-xl font-semibold shadow-lg hover:shadow-xl transform hover:scale-[1.02] active:scale-95 transition-all duration-300"
        >
          {saved ? '✓ Settings Saved!' : 'Save Settings'}
        </button>

        {saved && (
          <p className="mt-4 text-center text-sm text-green-600 font-medium animate-fade-in">
            Your settings have been saved successfully!
          </p>
        )}
      </div>
    </div>
  );
};

export default Settings;

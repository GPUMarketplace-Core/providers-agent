import { useState } from 'react';

const Settings = () => {
  const [oauthToken, setOauthToken] = useState('');
  const [showToken, setShowToken] = useState(false);
  const [saved, setSaved] = useState(false);

  const handleSaveSettings = async () => {
    try {
      if (oauthToken) {
        await window.electronAPI.setOAuthToken(oauthToken);
      }

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

      {/* Authentication */}
      <div className="glass rounded-3xl p-8 shadow-xl">
        <h3 className="text-xl font-bold text-gray-900 mb-6 flex items-center gap-2">
          <span>🔐</span>
          Authentication
        </h3>

        <div className="space-y-6">
          {/* OAuth Token */}
          <div>
            <label className="block text-sm font-medium text-gray-700 mb-2">
              Provider Token
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
              Get your token from the GPU Marketplace provider dashboard
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

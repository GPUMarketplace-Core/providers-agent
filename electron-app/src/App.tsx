import { useState, useEffect } from 'react';
import Sidebar from './components/Sidebar';
import Dashboard from './components/views/Dashboard';
import Jobs from './components/views/Jobs';
import Settings from './components/views/Settings';
import Logs from './components/views/Logs';

export type View = 'dashboard' | 'jobs' | 'settings' | 'logs';

interface AppStats {
  heartbeatCount: number;
  jobsCompleted: number;
  jobsFailed: number;
  nodeId: string;
}

function App() {
  const [currentView, setCurrentView] = useState<View>('dashboard');
  const [isOnline, setIsOnline] = useState(false);
  const [stats, setStats] = useState<AppStats>({
    heartbeatCount: 0,
    jobsCompleted: 0,
    jobsFailed: 0,
    nodeId: '',
  });

  useEffect(() => {
    loadAppState();

    // Listen for status changes
    window.electronAPI.onStatusChanged((data) => {
      setIsOnline(data.isOnline);
    });

    // Listen for heartbeat updates
    window.electronAPI.onHeartbeatSent((data) => {
      if (data.success && data.count) {
        setStats(prev => ({ ...prev, heartbeatCount: data.count! }));
      }
    });

    // Listen for auth required
    window.electronAPI.onAuthRequired(() => {
      setCurrentView('settings');
    });

    // Refresh stats periodically
    const interval = setInterval(loadAppState, 5000);

    return () => {
      clearInterval(interval);
      window.electronAPI.removeAllListeners('status-changed');
      window.electronAPI.removeAllListeners('heartbeat-sent');
      window.electronAPI.removeAllListeners('auth-required');
    };
  }, []);

  const loadAppState = async () => {
    try {
      const state = await window.electronAPI.getAppState();
      setIsOnline(state.isOnline);
      setStats({
        heartbeatCount: state.heartbeatCount,
        jobsCompleted: state.jobsCompleted,
        jobsFailed: state.jobsFailed,
        nodeId: state.nodeId,
      });
    } catch (e) {
      console.error('Failed to load app state:', e);
    }
  };

  const handleToggleOnline = async () => {
    const result = await window.electronAPI.toggleOnline();
    setIsOnline(result.isOnline);
  };

  return (
    <div className="flex h-screen w-screen overflow-hidden bg-slate-950">
      {/* Animated gradient background */}
      <div className="fixed inset-0 overflow-hidden pointer-events-none z-0">
        <div className="absolute inset-0 bg-gradient-to-br from-slate-950 via-slate-900 to-slate-950" />
        <div className="absolute top-0 -left-40 w-96 h-96 bg-violet-600/20 rounded-full filter blur-[128px] animate-blob" />
        <div className="absolute top-1/4 -right-20 w-80 h-80 bg-cyan-500/15 rounded-full filter blur-[100px] animate-blob animation-delay-2000" />
        <div className="absolute bottom-0 left-1/3 w-72 h-72 bg-fuchsia-500/10 rounded-full filter blur-[80px] animate-blob animation-delay-4000" />
      </div>

      {/* Grid pattern overlay */}
      <div 
        className="fixed inset-0 pointer-events-none z-0 opacity-[0.02]"
        style={{
          backgroundImage: `url("data:image/svg+xml,%3csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 32 32' width='32' height='32' fill='none' stroke='rgb(255 255 255)'%3e%3cpath d='M0 .5H31.5V32'/%3e%3c/svg%3e")`,
        }}
      />

      {/* Main content */}
      <div className="flex w-full h-full relative z-10">
        <Sidebar
          currentView={currentView}
          onViewChange={setCurrentView}
          isOnline={isOnline}
          onToggleOnline={handleToggleOnline}
          stats={stats}
        />

        <main className="flex-1 flex flex-col overflow-hidden">
          {/* macOS traffic lights spacer */}
          <div className="macos-traffic-lights-spacer flex-shrink-0 h-8" />

          {/* Content area */}
          <div className="flex-1 overflow-auto px-8 pb-8">
            {currentView === 'dashboard' && <Dashboard isOnline={isOnline} stats={stats} />}
            {currentView === 'jobs' && <Jobs />}
            {currentView === 'settings' && <Settings />}
            {currentView === 'logs' && <Logs />}
          </div>
        </main>
      </div>
    </div>
  );
}

export default App;

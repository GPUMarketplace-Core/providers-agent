import { useState, useEffect } from 'react';
import Sidebar from './components/Sidebar';
import Dashboard from './components/views/Dashboard';
import Jobs from './components/views/Jobs';
import Settings from './components/views/Settings';
import Logs from './components/views/Logs';

export type View = 'dashboard' | 'jobs' | 'settings' | 'logs';

function App() {
  const [currentView, setCurrentView] = useState<View>('dashboard');
  const [isOnline, setIsOnline] = useState(false);

  useEffect(() => {
    // Load initial state
    loadAppState();

    // Listen for status changes
    window.electronAPI.onStatusChanged((data) => {
      setIsOnline(data.isOnline);
    });

    return () => {
      window.electronAPI.removeAllListeners('status-changed');
    };
  }, []);

  const loadAppState = async () => {
    const state = await window.electronAPI.getAppState();
    setIsOnline(state.isOnline);
  };

  const handleToggleOnline = async () => {
    const result = await window.electronAPI.toggleOnline();
    setIsOnline(result.isOnline);
  };

  return (
    <div className="flex h-screen w-screen overflow-hidden">
      {/* Animated background blobs */}
      <div className="fixed inset-0 overflow-hidden pointer-events-none z-0">
        <div className="absolute top-0 -left-4 w-72 h-72 bg-violet-300 rounded-full mix-blend-multiply filter blur-xl opacity-70 animate-blob"></div>
        <div className="absolute top-0 -right-4 w-72 h-72 bg-fuchsia-300 rounded-full mix-blend-multiply filter blur-xl opacity-70 animate-blob animation-delay-2000"></div>
        <div className="absolute -bottom-8 left-20 w-72 h-72 bg-pink-300 rounded-full mix-blend-multiply filter blur-xl opacity-70 animate-blob animation-delay-4000"></div>
      </div>

      {/* Main content */}
      <div className="flex w-full h-full relative z-10">
        <Sidebar
          currentView={currentView}
          onViewChange={setCurrentView}
          isOnline={isOnline}
          onToggleOnline={handleToggleOnline}
        />

        <main className="flex-1 flex flex-col overflow-hidden">
          {/* macOS traffic lights spacer */}
          <div className="macos-traffic-lights-spacer flex-shrink-0" />

          {/* Content area */}
          <div className="flex-1 overflow-auto px-8 pb-8">
            {currentView === 'dashboard' && <Dashboard />}
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

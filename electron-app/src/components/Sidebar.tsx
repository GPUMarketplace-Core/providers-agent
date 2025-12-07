import { View } from '../App';

interface SidebarProps {
  currentView: View;
  onViewChange: (view: View) => void;
  isOnline: boolean;
  onToggleOnline: () => void;
  stats: {
    heartbeatCount: number;
    jobsCompleted: number;
    jobsFailed: number;
    nodeId: string;
  };
}

const Sidebar = ({ currentView, onViewChange, isOnline, onToggleOnline, stats }: SidebarProps) => {
  const navItems: { id: View; label: string; icon: string }[] = [
    { id: 'dashboard', label: 'Dashboard', icon: '📊' },
    { id: 'jobs', label: 'Jobs', icon: '🎬' },
    { id: 'logs', label: 'Logs', icon: '📝' },
    { id: 'settings', label: 'Settings', icon: '⚙️' },
  ];

  return (
    <aside className="w-72 h-full bg-slate-900/50 backdrop-blur-xl border-r border-white/5 flex flex-col">
      {/* Logo/Brand */}
      <div className="px-6 py-6 border-b border-white/5">
        <div className="flex items-center gap-3">
          <div className="w-10 h-10 rounded-xl bg-gradient-to-br from-violet-500 to-fuchsia-500 flex items-center justify-center text-xl shadow-lg shadow-violet-500/20">
            🎮
          </div>
          <div>
            <h1 className="text-lg font-bold text-white">GPU Marketplace</h1>
            <p className="text-xs text-slate-400">Provider Agent</p>
          </div>
        </div>
      </div>

      {/* Online/Offline Toggle */}
      <div className="px-5 py-5 border-b border-white/5">
        <button
          onClick={onToggleOnline}
          className={`w-full py-3.5 px-4 rounded-xl font-semibold transition-all duration-300 transform hover:scale-[1.02] active:scale-95 flex items-center justify-center gap-2 ${
            isOnline
              ? 'bg-gradient-to-r from-emerald-500 to-green-500 text-white shadow-lg shadow-emerald-500/25'
              : 'bg-gradient-to-r from-violet-600 to-fuchsia-600 text-white shadow-lg shadow-violet-500/25 hover:shadow-violet-500/40'
          }`}
        >
          <span className={`w-2.5 h-2.5 rounded-full ${isOnline ? 'bg-white animate-pulse' : 'bg-white/50'}`} />
          {isOnline ? 'Online' : 'Go Online'}
        </button>

        {/* Status indicator */}
        <div className="mt-4 p-3 rounded-xl bg-slate-800/50 border border-white/5">
          <div className="flex items-center justify-between text-xs">
            <span className="text-slate-400">Status</span>
            <span className={`font-medium ${isOnline ? 'text-emerald-400' : 'text-slate-500'}`}>
              {isOnline ? 'Accepting Jobs' : 'Offline'}
            </span>
          </div>
          {isOnline && (
            <div className="flex items-center justify-between text-xs mt-2">
              <span className="text-slate-400">Heartbeats</span>
              <span className="font-mono text-cyan-400">{stats.heartbeatCount}</span>
            </div>
          )}
        </div>
      </div>

      {/* Navigation */}
      <nav className="flex-1 px-4 py-5 space-y-1.5 overflow-y-auto">
        {navItems.map((item) => (
          <button
            key={item.id}
            onClick={() => onViewChange(item.id)}
            className={`w-full flex items-center gap-3 px-4 py-3 rounded-xl transition-all duration-200 ${
              currentView === item.id
                ? 'bg-gradient-to-r from-violet-600/90 to-fuchsia-600/90 text-white shadow-lg shadow-violet-500/20'
                : 'text-slate-400 hover:text-white hover:bg-white/5'
            }`}
          >
            <span className="text-lg">{item.icon}</span>
            <span className="font-medium">{item.label}</span>
            {item.id === 'jobs' && stats.jobsCompleted > 0 && (
              <span className="ml-auto px-2 py-0.5 text-xs rounded-full bg-emerald-500/20 text-emerald-400 font-mono">
                {stats.jobsCompleted}
              </span>
            )}
          </button>
        ))}
      </nav>

      {/* Node Info Footer */}
      <div className="px-5 py-4 border-t border-white/5">
        <div className="p-3 rounded-xl bg-slate-800/30 border border-white/5">
          <div className="text-xs text-slate-500 mb-1">Node ID</div>
          <div className="text-xs font-mono text-cyan-400 truncate">
            {stats.nodeId || 'Loading...'}
          </div>
        </div>
        <p className="text-xs text-slate-600 text-center mt-3">v1.0.0</p>
      </div>
    </aside>
  );
};

export default Sidebar;

import { View } from '../App';

interface SidebarProps {
  currentView: View;
  onViewChange: (view: View) => void;
  isOnline: boolean;
  onToggleOnline: () => void;
}

const Sidebar = ({ currentView, onViewChange, isOnline, onToggleOnline }: SidebarProps) => {
  const navItems: { id: View; label: string; icon: string }[] = [
    { id: 'dashboard', label: 'Dashboard', icon: '📊' },
    { id: 'jobs', label: 'Jobs', icon: '💼' },
    { id: 'logs', label: 'Logs', icon: '📝' },
    { id: 'settings', label: 'Settings', icon: '⚙️' },
  ];

  return (
    <aside className="w-64 h-full glass border-r border-gray-200/50 flex flex-col animate-slide-in-right">
      {/* Logo/Brand */}
      <div className="px-6 py-8 border-b border-gray-200/50">
        <h1 className="text-2xl font-bold gradient-text">GPU Marketplace</h1>
        <p className="text-sm text-gray-600 mt-1">Provider Agent</p>
      </div>

      {/* Online/Offline Toggle */}
      <div className="px-6 py-4 border-b border-gray-200/50">
        <button
          onClick={onToggleOnline}
          className={`w-full py-3 px-4 rounded-xl font-semibold transition-all duration-300 transform hover:scale-[1.02] active:scale-95 shadow-lg ${
            isOnline
              ? 'bg-gradient-to-r from-green-500 to-emerald-600 hover:from-green-600 hover:to-emerald-700 text-white'
              : 'bg-gradient-to-r from-violet-600 via-fuchsia-600 to-pink-600 hover:from-violet-700 hover:via-fuchsia-700 hover:to-pink-700 text-white'
          }`}
        >
          {isOnline ? '✓ Online' : 'Go Online'}
        </button>

        {/* Status indicator */}
        <div className="mt-3 flex items-center justify-center gap-2">
          <div
            className={`w-2 h-2 rounded-full ${
              isOnline ? 'bg-green-500 animate-pulse' : 'bg-gray-400'
            }`}
          />
          <span className="text-xs text-gray-600">
            {isOnline ? 'Accepting Jobs' : 'Offline'}
          </span>
        </div>
      </div>

      {/* Navigation */}
      <nav className="flex-1 px-4 py-6 space-y-2 overflow-y-auto">
        {navItems.map((item, index) => (
          <button
            key={item.id}
            onClick={() => onViewChange(item.id)}
            className={`w-full flex items-center gap-3 px-4 py-3 rounded-xl transition-all duration-300 transform hover:scale-[1.02] animate-fade-in-up ${
              currentView === item.id
                ? 'bg-gradient-to-r from-violet-600 via-fuchsia-600 to-pink-600 text-white shadow-lg'
                : 'text-gray-700 hover:bg-gray-100'
            }`}
            style={{ animationDelay: `${index * 100}ms` }}
          >
            <span className="text-xl">{item.icon}</span>
            <span className="font-medium">{item.label}</span>
          </button>
        ))}
      </nav>

      {/* Footer */}
      <div className="px-6 py-4 border-t border-gray-200/50">
        <p className="text-xs text-gray-500 text-center">
          v1.0.0
        </p>
      </div>
    </aside>
  );
};

export default Sidebar;

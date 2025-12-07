import { useState, useEffect, useRef } from 'react';

const Logs = () => {
  const [logs, setLogs] = useState<string[]>([]);
  const [filter, setFilter] = useState('');
  const [autoScroll, setAutoScroll] = useState(true);
  const [levelFilter, setLevelFilter] = useState<'all' | 'error' | 'success' | 'info'>('all');
  const logsEndRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    loadLogs();

    // Listen for new logs
    window.electronAPI.onLogUpdate((log) => {
      setLogs((prev) => [...prev, log]);
    });

    return () => {
      window.electronAPI.removeAllListeners('log-update');
    };
  }, []);

  useEffect(() => {
    if (autoScroll && logsEndRef.current) {
      logsEndRef.current.scrollIntoView({ behavior: 'smooth' });
    }
  }, [logs, autoScroll]);

  const loadLogs = async () => {
    const existingLogs = await window.electronAPI.getLogs();
    setLogs(existingLogs);
  };

  const handleClearLogs = async () => {
    await window.electronAPI.clearLogs();
    setLogs([]);
  };

  const getLogLevel = (log: string): 'error' | 'success' | 'info' => {
    if (log.includes('❌') || log.includes('ERROR') || log.includes('failed') || log.includes('Failed')) return 'error';
    if (log.includes('✅') || log.includes('✓') || log.includes('success') || log.includes('COMPLETED')) return 'success';
    return 'info';
  };

  const filteredLogs = logs.filter((log) => {
    const matchesText = log.toLowerCase().includes(filter.toLowerCase());
    const matchesLevel = levelFilter === 'all' || getLogLevel(log) === levelFilter;
    return matchesText && matchesLevel;
  });

  const getLogStyle = (log: string) => {
    const level = getLogLevel(log);
    switch (level) {
      case 'error':
        return 'text-red-400 bg-red-500/5';
      case 'success':
        return 'text-emerald-400 bg-emerald-500/5';
      default:
        return 'text-slate-300';
    }
  };

  const extractTimestamp = (log: string) => {
    const match = log.match(/\[(.*?)\]/);
    if (match) {
      try {
        const date = new Date(match[1]);
        return date.toLocaleTimeString();
      } catch {
        return '';
      }
    }
    return '';
  };

  const extractMessage = (log: string) => {
    return log.replace(/\[.*?\]\s*/, '');
  };

  return (
    <div className="space-y-6 animate-fade-in h-full flex flex-col">
      {/* Header */}
      <div className="flex items-start justify-between flex-shrink-0">
        <div>
          <h2 className="text-2xl font-bold text-white">Logs</h2>
          <p className="text-slate-400 text-sm mt-1">Real-time system and job activity</p>
        </div>
        <div className="flex gap-2">
          <button
            onClick={() => setAutoScroll(!autoScroll)}
            className={`px-4 py-2 rounded-xl text-sm font-medium transition-all duration-200 border ${
              autoScroll
                ? 'bg-violet-500/20 text-violet-400 border-violet-500/30'
                : 'bg-slate-800 text-slate-400 border-white/10 hover:border-white/20'
            }`}
          >
            {autoScroll ? '📌 Auto-scroll ON' : '📌 Auto-scroll OFF'}
          </button>
          <button
            onClick={handleClearLogs}
            className="px-4 py-2 bg-red-500/10 text-red-400 hover:bg-red-500/20 rounded-xl text-sm font-medium transition-all duration-200 border border-red-500/20"
          >
            🗑️ Clear
          </button>
        </div>
      </div>

      {/* Filters */}
      <div className="flex gap-3 flex-shrink-0">
        <div className="flex-1 relative">
          <span className="absolute left-4 top-1/2 -translate-y-1/2 text-slate-500">🔍</span>
          <input
            type="text"
            value={filter}
            onChange={(e) => setFilter(e.target.value)}
            placeholder="Filter logs..."
            className="w-full pl-10 pr-4 py-2.5 text-sm bg-slate-800/50 border border-white/10 rounded-xl focus:ring-2 focus:ring-violet-500 focus:border-transparent outline-none transition-all text-white placeholder-slate-500"
          />
        </div>
        <div className="flex bg-slate-800/50 rounded-xl border border-white/10 p-1">
          {(['all', 'info', 'success', 'error'] as const).map((level) => (
            <button
              key={level}
              onClick={() => setLevelFilter(level)}
              className={`px-3 py-1.5 rounded-lg text-xs font-medium transition-colors ${
                levelFilter === level
                  ? level === 'error'
                    ? 'bg-red-500/20 text-red-400'
                    : level === 'success'
                    ? 'bg-emerald-500/20 text-emerald-400'
                    : 'bg-violet-500/20 text-violet-400'
                  : 'text-slate-400 hover:text-white'
              }`}
            >
              {level.charAt(0).toUpperCase() + level.slice(1)}
            </button>
          ))}
        </div>
      </div>

      {/* Logs Display */}
      <div className="flex-1 min-h-0 rounded-2xl bg-slate-900/80 border border-white/5 backdrop-blur-sm overflow-hidden">
        <div className="h-full overflow-y-auto p-4 font-mono text-sm">
          {filteredLogs.length === 0 ? (
            <div className="flex flex-col items-center justify-center h-full text-center">
              <div className="text-5xl mb-4 opacity-50">📝</div>
              <p className="text-slate-500">
                {filter || levelFilter !== 'all' ? 'No logs match your filter' : 'No logs yet'}
              </p>
              <p className="text-slate-600 text-xs mt-2">
                Logs will appear here as events occur
              </p>
            </div>
          ) : (
            <div className="space-y-0.5">
              {filteredLogs.map((log, index) => (
                <div
                  key={index}
                  className={`px-3 py-1.5 rounded-lg transition-colors hover:bg-white/5 ${getLogStyle(log)}`}
                >
                  <span className="text-slate-600 mr-3 text-xs">{extractTimestamp(log)}</span>
                  <span>{extractMessage(log)}</span>
                </div>
              ))}
              <div ref={logsEndRef} />
            </div>
          )}
        </div>
      </div>

      {/* Stats Bar */}
      <div className="flex items-center justify-between text-xs text-slate-500 px-1 flex-shrink-0">
        <div className="flex items-center gap-4">
          <span>{filteredLogs.length} log{filteredLogs.length !== 1 ? 's' : ''}</span>
          {(filter || levelFilter !== 'all') && (
            <span className="text-slate-600">(filtered from {logs.length})</span>
          )}
        </div>
        <div className="flex items-center gap-4">
          <span className="flex items-center gap-1.5">
            <span className="w-2 h-2 rounded-full bg-emerald-500" />
            {logs.filter(l => getLogLevel(l) === 'success').length} success
          </span>
          <span className="flex items-center gap-1.5">
            <span className="w-2 h-2 rounded-full bg-red-500" />
            {logs.filter(l => getLogLevel(l) === 'error').length} errors
          </span>
        </div>
      </div>
    </div>
  );
};

export default Logs;

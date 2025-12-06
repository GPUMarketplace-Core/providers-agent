import { useState, useEffect, useRef } from 'react';

const Logs = () => {
  const [logs, setLogs] = useState<string[]>([]);
  const [filter, setFilter] = useState('');
  const [autoScroll, setAutoScroll] = useState(true);
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

  const filteredLogs = logs.filter((log) =>
    log.toLowerCase().includes(filter.toLowerCase())
  );

  const getLogColor = (log: string) => {
    if (log.includes('ERROR') || log.includes('failed')) return 'text-red-600';
    if (log.includes('WARNING')) return 'text-yellow-600';
    if (log.includes('success')) return 'text-green-600';
    return 'text-gray-700';
  };

  return (
    <div className="space-y-6 animate-fade-in-up">
      {/* Header */}
      <div className="glass rounded-3xl p-8 shadow-xl">
        <div className="flex items-start justify-between">
          <div>
            <h2 className="text-3xl font-bold gradient-text mb-2">Logs</h2>
            <p className="text-gray-600">Real-time system and job logs</p>
          </div>
          <div className="flex gap-3">
            <button
              onClick={() => setAutoScroll(!autoScroll)}
              className={`px-4 py-2 rounded-xl font-medium transition-all duration-300 ${
                autoScroll
                  ? 'bg-violet-100 text-violet-700'
                  : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
              }`}
            >
              {autoScroll ? '📌 Auto-scroll ON' : '📌 Auto-scroll OFF'}
            </button>
            <button
              onClick={handleClearLogs}
              className="px-4 py-2 bg-red-100 text-red-700 hover:bg-red-200 rounded-xl font-medium transition-all duration-300"
            >
              🗑️ Clear Logs
            </button>
          </div>
        </div>

        {/* Filter */}
        <div className="mt-6">
          <input
            type="text"
            value={filter}
            onChange={(e) => setFilter(e.target.value)}
            placeholder="🔍 Filter logs..."
            className="w-full px-4 py-3 text-sm border border-gray-300 rounded-xl focus:ring-2 focus:ring-violet-500 focus:border-transparent outline-none transition-all bg-white"
          />
        </div>
      </div>

      {/* Logs Display */}
      <div className="glass rounded-3xl p-6 shadow-xl">
        <div className="bg-gray-900 rounded-2xl p-6 h-[600px] overflow-y-auto font-mono text-sm">
          {filteredLogs.length === 0 ? (
            <div className="text-center py-12">
              <p className="text-gray-500">
                {filter ? 'No logs match your filter' : 'No logs yet'}
              </p>
            </div>
          ) : (
            <div className="space-y-1">
              {filteredLogs.map((log, index) => (
                <div
                  key={index}
                  className={`${getLogColor(log)} hover:bg-white/5 px-2 py-1 rounded transition-colors text-gray-300`}
                >
                  {log}
                </div>
              ))}
              <div ref={logsEndRef} />
            </div>
          )}
        </div>

        {/* Stats */}
        <div className="mt-4 flex items-center justify-between text-sm text-gray-600">
          <span>
            {filteredLogs.length} log{filteredLogs.length !== 1 ? 's' : ''}
            {filter && ` (filtered from ${logs.length})`}
          </span>
          <span className="text-xs text-gray-500">
            Logs are limited to the last 1000 entries
          </span>
        </div>
      </div>
    </div>
  );
};

export default Logs;

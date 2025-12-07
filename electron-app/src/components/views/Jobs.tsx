import { useState, useEffect } from 'react';
import { Job } from '../../../electron/preload';

const Jobs = () => {
  const [jobs, setJobs] = useState<Job[]>([]);
  const [currentJob, setCurrentJob] = useState<Job | null>(null);

  useEffect(() => {
    // Listen for job updates
    window.electronAPI.onJobsPolled((data) => {
      if (data.jobs) {
        setJobs(data.jobs);
      }
    });

    window.electronAPI.onJobStarted((job) => {
      setCurrentJob(job);
    });

    window.electronAPI.onJobCompleted((data) => {
      if (data.success) {
        setCurrentJob(null);
      }
      // Refresh jobs list
      window.electronAPI.getAppState().then(state => {
        if (state.currentJob === null) {
          setCurrentJob(null);
        }
      });
    });

    return () => {
      window.electronAPI.removeAllListeners('jobs-polled');
      window.electronAPI.removeAllListeners('job-started');
      window.electronAPI.removeAllListeners('job-completed');
    };
  }, []);

  const getStatusBadge = (status: Job['status']) => {
    const styles: Record<string, string> = {
      queued: 'bg-amber-500/20 text-amber-400 border-amber-500/30',
      running: 'bg-blue-500/20 text-blue-400 border-blue-500/30',
      completed: 'bg-emerald-500/20 text-emerald-400 border-emerald-500/30',
      failed: 'bg-red-500/20 text-red-400 border-red-500/30',
    };
    
    const icons: Record<string, string> = {
      queued: '⏳',
      running: '🔄',
      completed: '✅',
      failed: '❌',
    };

    return (
      <span className={`inline-flex items-center gap-1.5 px-3 py-1 rounded-lg text-xs font-medium border ${styles[status]}`}>
        <span>{icons[status]}</span>
        {status.charAt(0).toUpperCase() + status.slice(1)}
      </span>
    );
  };

  const formatDate = (dateStr: string) => {
    try {
      return new Date(dateStr).toLocaleString();
    } catch {
      return dateStr;
    }
  };

  return (
    <div className="space-y-6 animate-fade-in">
      {/* Header */}
      <div>
        <h2 className="text-2xl font-bold text-white">Jobs</h2>
        <p className="text-slate-400 text-sm mt-1">Monitor and manage rendering jobs</p>
      </div>

      {/* Current Job */}
      {currentJob && (
        <div className="p-6 rounded-2xl bg-gradient-to-r from-blue-500/10 to-cyan-500/10 border border-blue-500/20 backdrop-blur-sm">
          <div className="flex items-start justify-between mb-4">
            <div className="flex items-center gap-3">
              <div className="w-12 h-12 rounded-xl bg-blue-500/20 flex items-center justify-center">
                <span className="text-2xl animate-spin-slow">🎬</span>
              </div>
              <div>
                <h3 className="text-lg font-semibold text-white">Current Job</h3>
                <p className="text-sm text-slate-400 font-mono">{currentJob.id}</p>
              </div>
            </div>
            {getStatusBadge(currentJob.status)}
          </div>

          <div className="mt-4">
            <div className="w-full bg-slate-700 rounded-full h-2 overflow-hidden">
              <div className="bg-gradient-to-r from-blue-500 to-cyan-500 h-full rounded-full animate-pulse w-full" />
            </div>
            <p className="text-sm text-slate-400 mt-2 text-center flex items-center justify-center gap-2">
              <span className="inline-block w-2 h-2 bg-blue-400 rounded-full animate-pulse" />
              Rendering in progress...
            </p>
          </div>
        </div>
      )}

      {/* Job Queue */}
      <div className="p-6 rounded-2xl bg-slate-800/50 border border-white/5 backdrop-blur-sm">
        <div className="flex items-center justify-between mb-6">
          <h3 className="text-lg font-semibold text-white flex items-center gap-2">
            <span>📋</span>
            Job Queue
          </h3>
          <span className="text-sm text-slate-400">
            {jobs.filter(j => j.status === 'queued').length} pending
          </span>
        </div>

        {jobs.length === 0 ? (
          <div className="text-center py-16">
            <div className="text-6xl mb-4">📭</div>
            <p className="text-slate-300 text-lg font-medium">No jobs yet</p>
            <p className="text-slate-500 text-sm mt-2 max-w-md mx-auto">
              Jobs will appear here when you go online and consumers submit render requests to your node
            </p>
          </div>
        ) : (
          <div className="space-y-3">
            {jobs.map((job) => (
              <div
                key={job.id}
                className={`p-5 rounded-xl border transition-all duration-200 hover:border-white/10 ${
                  job.status === 'running' 
                    ? 'bg-blue-500/5 border-blue-500/20' 
                    : 'bg-slate-800/50 border-white/5'
                }`}
              >
                <div className="flex items-start justify-between">
                  <div className="flex-1">
                    <div className="flex items-center gap-3">
                      <p className="font-mono text-sm text-white">{job.id}</p>
                      {getStatusBadge(job.status)}
                    </div>
                    <div className="mt-2 flex items-center gap-4 text-xs text-slate-500">
                      <span>Created: {formatDate(job.created_at)}</span>
                      {job.completed_at && (
                        <span>Completed: {formatDate(job.completed_at)}</span>
                      )}
                    </div>
                  </div>
                </div>

                {job.failure_reason && (
                  <div className="mt-3 p-3 bg-red-500/10 border border-red-500/20 rounded-lg">
                    <p className="text-sm text-red-400">
                      <span className="font-medium">Error:</span> {job.failure_reason}
                    </p>
                  </div>
                )}
              </div>
            ))}
          </div>
        )}
      </div>

      {/* Job History Stats */}
      <div className="grid grid-cols-3 gap-4">
        <div className="p-5 rounded-2xl bg-slate-800/50 border border-white/5 text-center">
          <div className="text-3xl font-bold text-white font-mono">
            {jobs.filter(j => j.status === 'completed').length}
          </div>
          <div className="text-sm text-emerald-400 mt-1">Completed</div>
        </div>
        <div className="p-5 rounded-2xl bg-slate-800/50 border border-white/5 text-center">
          <div className="text-3xl font-bold text-white font-mono">
            {jobs.filter(j => j.status === 'queued').length}
          </div>
          <div className="text-sm text-amber-400 mt-1">Queued</div>
        </div>
        <div className="p-5 rounded-2xl bg-slate-800/50 border border-white/5 text-center">
          <div className="text-3xl font-bold text-white font-mono">
            {jobs.filter(j => j.status === 'failed').length}
          </div>
          <div className="text-sm text-red-400 mt-1">Failed</div>
        </div>
      </div>
    </div>
  );
};

export default Jobs;

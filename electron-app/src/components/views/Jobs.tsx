import { useState, useEffect } from 'react';

interface Job {
  id: string;
  status: 'queued' | 'running' | 'completed' | 'failed';
  createdAt: string;
  completedAt?: string;
  error?: string;
}

const Jobs = () => {
  const [jobs, setJobs] = useState<Job[]>([]);
  const [currentJob, setCurrentJob] = useState<Job | null>(null);

  useEffect(() => {
    // Listen for job updates
    window.electronAPI.onJobsPolled((data) => {
      if (data.jobs.length > 0) {
        setJobs(data.jobs);
      }
    });

    return () => {
      window.electronAPI.removeAllListeners('jobs-polled');
    };
  }, []);

  const getStatusColor = (status: Job['status']) => {
    switch (status) {
      case 'queued':
        return 'bg-yellow-100 text-yellow-800';
      case 'running':
        return 'bg-blue-100 text-blue-800';
      case 'completed':
        return 'bg-green-100 text-green-800';
      case 'failed':
        return 'bg-red-100 text-red-800';
      default:
        return 'bg-gray-100 text-gray-800';
    }
  };

  return (
    <div className="space-y-6 animate-fade-in-up">
      {/* Header */}
      <div className="glass rounded-3xl p-8 shadow-xl">
        <h2 className="text-3xl font-bold gradient-text mb-2">Jobs</h2>
        <p className="text-gray-600">Monitor and manage rendering jobs</p>
      </div>

      {/* Current Job */}
      {currentJob && (
        <div className="glass rounded-3xl p-8 shadow-xl border-2 border-violet-200">
          <div className="flex items-start justify-between mb-4">
            <div>
              <h3 className="text-xl font-bold text-gray-900">Current Job</h3>
              <p className="text-sm text-gray-600 mt-1">Job ID: {currentJob.id}</p>
            </div>
            <span
              className={`px-3 py-1 rounded-full text-xs font-medium ${getStatusColor(
                currentJob.status
              )}`}
            >
              {currentJob.status.toUpperCase()}
            </span>
          </div>

          {currentJob.status === 'running' && (
            <div className="mt-4">
              <div className="w-full bg-gray-200 rounded-full h-2 overflow-hidden">
                <div className="bg-gradient-to-r from-violet-600 via-fuchsia-600 to-pink-600 h-full rounded-full animate-pulse" />
              </div>
              <p className="text-sm text-gray-600 mt-2 text-center">
                Rendering in progress...
              </p>
            </div>
          )}
        </div>
      )}

      {/* Job History */}
      <div className="glass rounded-3xl p-8 shadow-xl">
        <h3 className="text-xl font-bold text-gray-900 mb-6">Job History</h3>

        {jobs.length === 0 ? (
          <div className="text-center py-12">
            <div className="text-6xl mb-4">📭</div>
            <p className="text-gray-600 text-lg">No jobs yet</p>
            <p className="text-gray-500 text-sm mt-2">
              Jobs will appear here when you go online and start accepting work
            </p>
          </div>
        ) : (
          <div className="space-y-4">
            {jobs.map((job) => (
              <div
                key={job.id}
                className="p-6 bg-white/50 rounded-2xl border border-gray-200/50 hover:shadow-lg transition-all duration-300"
              >
                <div className="flex items-start justify-between">
                  <div>
                    <p className="font-semibold text-gray-900">Job {job.id}</p>
                    <p className="text-sm text-gray-600 mt-1">
                      Created: {new Date(job.createdAt).toLocaleString()}
                    </p>
                    {job.completedAt && (
                      <p className="text-sm text-gray-600">
                        Completed: {new Date(job.completedAt).toLocaleString()}
                      </p>
                    )}
                  </div>
                  <span
                    className={`px-3 py-1 rounded-full text-xs font-medium ${getStatusColor(
                      job.status
                    )}`}
                  >
                    {job.status.toUpperCase()}
                  </span>
                </div>

                {job.error && (
                  <div className="mt-4 p-3 bg-red-50 border border-red-200 rounded-lg">
                    <p className="text-sm text-red-800">
                      <strong>Error:</strong> {job.error}
                    </p>
                  </div>
                )}
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
};

export default Jobs;

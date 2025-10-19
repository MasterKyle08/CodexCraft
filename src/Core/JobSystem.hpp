#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace core
{
class JobSystem
{
  public:
    using Job = std::function<void()>;

    explicit JobSystem(std::size_t workerCount = std::thread::hardware_concurrency());
    ~JobSystem();

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;

    void enqueue(Job job);
    std::size_t pending_jobs() const;

  private:
    void worker_loop();

    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<Job> m_jobs;
    std::vector<std::thread> m_workers;
    std::atomic<bool> m_running{true};
};

} // namespace core

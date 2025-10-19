#include "JobSystem.hpp"

#include <algorithm>

namespace core
{
JobSystem::JobSystem(std::size_t workerCount)
{
    if (workerCount == 0)
    {
        workerCount = 1;
    }
    const std::size_t workers = workerCount > 0 ? workerCount - 1 : 0;
    for (std::size_t i = 0; i < workers; ++i)
    {
        m_workers.emplace_back([this] { worker_loop(); });
    }
}

JobSystem::~JobSystem()
{
    m_running = false;
    m_cv.notify_all();
    for (auto& worker : m_workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

void JobSystem::enqueue(Job job)
{
    {
        std::lock_guard lock(m_mutex);
        m_jobs.push(std::move(job));
    }
    m_cv.notify_one();
}

std::size_t JobSystem::pending_jobs() const
{
    std::lock_guard lock(m_mutex);
    return m_jobs.size();
}

void JobSystem::worker_loop()
{
    while (m_running)
    {
        Job job;
        {
            std::unique_lock lock(m_mutex);
            m_cv.wait(lock, [this] { return !m_running || !m_jobs.empty(); });
            if (!m_running && m_jobs.empty())
            {
                return;
            }
            job = std::move(m_jobs.front());
            m_jobs.pop();
        }

        if (job)
        {
            job();
        }
    }
}

} // namespace core

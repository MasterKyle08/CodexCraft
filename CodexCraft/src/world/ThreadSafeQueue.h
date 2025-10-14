#pragma once

#include <cstddef>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

namespace CodexCraft::World {

// Simple multi-producer multi-consumer queue used for chunk work scheduling.
template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;

    void Push(T value) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(value));
        }
        m_condition.notify_one();
    }

    [[nodiscard]] bool TryPop(T& outValue) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }

        outValue = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    T WaitPop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this]() { return !m_queue.empty(); });

        T value = std::move(m_queue.front());
        m_queue.pop();
        return value;
    }

    [[nodiscard]] bool Empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    [[nodiscard]] std::size_t Size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    std::queue<T> m_queue;
};

} // namespace CodexCraft::World


#pragma once

#include <chrono>
#include <cstdio>
#include <mutex>
#include <string_view>

namespace util
{
class Logger
{
  public:
    static Logger& instance()
    {
        static Logger g_logger;
        return g_logger;
    }

    template <typename... Args>
    void info(std::string_view fmt, Args&&... args)
    {
        log("[INFO] ", fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(std::string_view fmt, Args&&... args)
    {
        log("[WARN] ", fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(std::string_view fmt, Args&&... args)
    {
        log("[ERR ] ", fmt, std::forward<Args>(args)...);
    }

  private:
    template <typename... Args>
    void log(std::string_view prefix, std::string_view fmt, Args&&... args)
    {
        std::scoped_lock lock(m_mutex);
        std::fputs(prefix.data(), stdout);
        std::fprintf(stdout, fmt.data(), std::forward<Args>(args)...);
        std::fputc('\n', stdout);
        std::fflush(stdout);
    }

    std::mutex m_mutex;
};

inline Logger& log()
{
    return Logger::instance();
}

} // namespace util

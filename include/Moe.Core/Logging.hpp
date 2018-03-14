/**
 * @file
 * @date 2017/5/29
 */
#pragma once
#include "StringUtils.hpp"
#include "Time.hpp"

#include <thread>
#include <chrono>
#include <fstream>

namespace moe
{
    /**
     * @brief 日志支持
     *
     * 提供全局的日志功能。
     */
    namespace Logging
    {
        using LoggingTimePoint = Time::Timestamp;

        /**
         * @brief 日志等级
         */
        enum class LogLevel
        {
            Debug,
            Trace,
            Info,
            Warn,
            Error,
            Fatal
        };

        /**
         * @brief 日志记录器基类
         *
         * 日志记录器保证线程安全。
         */
        class Logger :
            public NonCopyable
        {
        public:
            Logger() = default;
            ~Logger() = default;

        public:
            /**
             * @brief 记录日志事件
             * @param level 日志级别
             * @param message 日志信息
             * @param file 文件名
             * @param line 行号
             * @param function 函数名
             * @param thread 线程id
             * @param time 日志时间
             *
             * Logging::Log保证对该方法的调用线程安全。
             */
            virtual void Log(LogLevel level, const std::string& message, const char* file, unsigned line,
                const char* function, std::thread::id thread, LoggingTimePoint time)noexcept = 0;
        };

        /**
         * @brief 控制台标准输入输出流日志器
         */
        class ConsoleLogger :
            public Logger
        {
        public:
            void Log(LogLevel level, const std::string& message, const char* file, unsigned line, const char* function,
                std::thread::id thread, LoggingTimePoint time)noexcept;
        };

        /**
         * @brief 文件输出流日志器
         */
        class FileLogger :
            public Logger
        {
        public:
            FileLogger(const char* path);

        public:
            void Log(LogLevel level, const std::string& message, const char* file, unsigned line, const char* function,
                std::thread::id thread, LoggingTimePoint time)noexcept;

        private:
            std::fstream m_stFile;
        };

        namespace details
        {
            void DispatchLogMessage(LogLevel level, const std::string& message, const char* file, unsigned line,
                const char* function, std::thread::id thread, LoggingTimePoint time)noexcept;
        }

        /**
         * @brief 将日志级别转到字符串
         */
        const char* LogLevelToString(LogLevel level)noexcept;

        /**
         * @brief 添加一个记录器
         * @param logger 记录器
         */
        void AddLogger(std::shared_ptr<Logger> logger);

        /**
         * @brief 移除一个已存在的记录器
         * @param logger 记录器
         */
        void RemoveLogger(Logger* logger)noexcept;

        /**
         * @brief 移除所有记录器
         */
        void ClearLogger()noexcept;

        /**
         * @brief 获取过滤器的最低等级
         * @return 返回不被过滤的最低等级
         */
        LogLevel GetFilterMinLevel()noexcept;

        /**
         * @brief 设置过滤器的最低等级
         * @param level 低于该级别的日志将被过滤
         */
        void SetFilterMinLevel(LogLevel level)noexcept;

        /**
         * @brief 记录日志
         * @param level 日志级别
         * @param file 文件名
         * @param line 行号
         * @param function 函数名
         * @param format 日志格式
         */
        template <typename... Args>
        inline void Log(LogLevel level, const char* file, unsigned line, const char* function, const char* format,
            const Args&... args)noexcept
        {
            try
            {
                auto message(StringUtils::Format(format, args...));
                details::DispatchLogMessage(level, message, file, line, function, std::this_thread::get_id(),
                    Time::GetLocalTime());
            }
            catch (...)
            {
                // 忽略该条日志
            }
        }
    }
}

#define MOE_LOG(level, format, ...) \
    do { \
        if (level >= moe::Logging::GetFilterMinLevel()) \
            moe::Logging::Log(level, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
    } while (false)

#define MOE_DEBUG(format, ...) \
    MOE_LOG(moe::Logging::LogLevel::Debug, format, ##__VA_ARGS__)

#define MOE_TRACE(format, ...) \
    MOE_LOG(moe::Logging::LogLevel::Trace, format, ##__VA_ARGS__)

#define MOE_INFO(format, ...) \
    MOE_LOG(moe::Logging::LogLevel::Info, format, ##__VA_ARGS__)

#define MOE_WARN(format, ...) \
    MOE_LOG(moe::Logging::LogLevel::Warn, format, ##__VA_ARGS__)

#define MOE_ERROR(format, ...) \
    MOE_LOG(moe::Logging::LogLevel::Error, format, ##__VA_ARGS__)

#define MOE_FATAL(format, ...) \
    MOE_LOG(moe::Logging::LogLevel::Fatal, format, ##__VA_ARGS__)

#define MOE_EXCEPTION(ex) \
    do { \
        const auto& except = (ex); \
        if (moe::Logging::LogLevel::Error >= moe::Logging::GetFilterMinLevel()) \
            moe::Logging::Log(moe::Logging::LogLevel::Error, except.GetSourceFile(), except.GetLineNumber(), \
            except.GetFunctionName(), "(Logged at {0}:{1}) {2}", __FUNCTION__, __LINE__, except.GetDescription()); \
    } while (false)

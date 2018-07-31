/**
 * @file
 * @date 2017/5/29
 */
#pragma once
#include <thread>
#include <chrono>
#include <fstream>
#include <atomic>
#include <vector>
#include <mutex>
#include <algorithm>
#include <memory>

#include "Time.hpp"
#include "PathUtils.hpp"
#include "StringUtils.hpp"

namespace moe
{
    /**
     * @brief 日志系统
     *
     * 提供全局的统一日志功能，保证日志输出的灵活性和线程安全。
     */
    class Logging :
        public NonCopyable
    {
    public:
        static const char* kFormatErrorMsg;
        static const size_t kFormatErrorMsgLength;
        static const char* kAllocErrorMsg;
        static const size_t kAllocErrorMsgLength;

        /**
         * @brief 日志等级
         */
        enum class Level
        {
            Debug,
            Trace,
            Info,
            Warn,
            Error,
            Fatal,
        };

        /**
         * @brief 日志上下文
         *
         * 记录基本的日志上下文信息，供格式化输出串使用。
         */
        struct Context
        {
            Time::Timestamp Time = 0;
            const char* File = nullptr;
            uint32_t Line = 0;
            const char* Function = nullptr;
            uint64_t ThreadId = 0;

            static uint64_t GetThreadIdCached()noexcept;

            Context() = default;
            Context(const char* file, uint32_t line, const char* func)noexcept
                : Time(Time::Now()), File(file), Line(line), Function(func), ThreadId(GetThreadIdCached()) {}
            Context(Time::Timestamp time, const char* file, uint32_t line, const char* func, uint64_t tid)noexcept
                : Time(time), File(file), Line(line), Function(func), ThreadId(tid) {}
        };

        /**
         * @brief 格式化器基类
         *
         * 需要注意：我们允许多个Sink共享一个Formatter，因此在多线程环境下，Format函数要保证没有数据竞争。
         */
        class FormatterBase
        {
        public:
            /**
             * @brief 格式化指定日志文本
             * @note 要求线程安全
             * @param dest 输出缓冲区
             * @param level 日志级别
             * @param context 日志上下文
             * @param msg 日志信息
             */
            virtual void Format(std::string& dest, Level level, const Context& context, const char* msg)const = 0;

            /**
             * @brief 创建副本
             */
            virtual std::shared_ptr<FormatterBase> Clone()const = 0;
        };

        using FormatterPtr = std::shared_ptr<FormatterBase>;

        /**
         * @brief 一般文本格式化器
         *
         * 使用下述字符来描述各个部分：
         *  - {date} 日期，如1993-12-23，自动补齐
         *  - {short_date} 短日期，如93-12-23，自动补齐
         *  - {time} 时间，如10:24:30.123，自动补齐
         *  - {level} 日志级别，如INFO ERROR等
         *  - {thread} 线程
         *  - {path} 完整文件路径
         *  - {file} 文件名
         *  - {func} 函数名
         *  - {line} 行号
         *  - {msg} 日志数据
         */
        class PlainFormatter :
            public FormatterBase
        {
        public:
            /**
             * @brief 获取格式描述
             */
            const std::string& GetFormat()const noexcept { return m_stFormat; }

            /**
             * @brief 设置格式描述
             */
            void SetFormat(const std::string& format) { m_stFormat = format; }
            void SetFormat(std::string&& format) { m_stFormat = std::move(format); }

        protected:
            void Format(std::string& dest, Level level, const Context& context, const char* msg)const override;
            std::shared_ptr<FormatterBase> Clone()const override;

        private:
            std::string m_stFormat = "[{short_date} {time}][{level,-5}][0x{thread:H}][{file}:{line},{func}] {msg}";
        };

        /**
         * @brief ANSI标准颜色格式化输出
         *
         * 格式参照PlainFormatter。
         */
        class AnsiColorFormatter :
            public FormatterBase
        {
        public:
            enum class Colors
            {
                Default = 0,
                Black,
                Red,
                Green,
                Yellow,
                Blue,
                Magenta,
                Cyan,
                White,
                BrightBlack,
                BrightRed,
                BrightGreen,
                BrightYellow,
                BrightBlue,
                BrightMagenta,
                BrightCyan,
                BrightWhite,
            };

        public:
            AnsiColorFormatter();

        public:
            /**
             * @brief 获取格式描述
             */
            const std::string& GetFormat()const noexcept { return m_stFormat; }

            /**
             * @brief 设置格式描述
             */
            void SetFormat(const std::string& format) { m_stFormat = format; }
            void SetFormat(std::string&& format) { m_stFormat = std::move(format); }

            /**
             * @brief 获取颜色
             * @param level 日志级别
             */
            std::pair<Colors, Colors> GetColor(Level level)const noexcept
            {
                assert(static_cast<unsigned>(level) < CountOf(m_stColors));
                return m_stColors[static_cast<unsigned>(level)];
            }

            /**
             * @brief 设置颜色
             * @param level 日志级别
             * @param fg 前景色
             * @param bg 背景色
             */
            void SetColor(Level level, Colors fg, Colors bg)
            {
                assert(static_cast<unsigned>(level) < CountOf(m_stColors));
                m_stColors[static_cast<unsigned>(level)] = std::pair<Colors, Colors>(fg, bg);
            }

        protected:
            void Format(std::string& dest, Level level, const Context& context, const char* msg)const override;
            std::shared_ptr<FormatterBase> Clone()const override;

        private:
            std::string m_stFormat = "[{short_date} {time}][{level,-5}][0x{thread:H}][{file}:{line},{func}] {msg}";
            std::pair<Colors, Colors> m_stColors[static_cast<unsigned>(Level::Fatal) + 1];
        };

        /**
         * @brief 日志落地对象基类
         */
        class SinkBase
        {
        public:
            SinkBase() = default;
            SinkBase(const SinkBase& rhs);

        public:
            /**
             * @brief 是否总是刷新流
             */
            bool IsAlwaysFlush()const noexcept { return m_bAlwaysFlush; }

            /**
             * @brief 设置总是刷新流
             */
            void SetAlwaysFlush(bool v)noexcept { m_bAlwaysFlush = v; }

            /**
             * @brief 获取日志最小输出级别（闭区间）
             */
            Level GetMinLevel()const noexcept { return m_iMinLevel; }

            /**
             * @brief 设置日志最小输出级别（闭区间）
             */
            void SetMinLevel(Level level)noexcept { m_iMinLevel = level; }

            /**
             * @brief 获取日志最大输出级别（闭区间）
             */
            Level GetMaxLevel()const noexcept { return m_iMaxLevel; }

            /**
             * @brief 设置日志最大输出级别（闭区间）
             */
            void SetMaxLevel(Level level)noexcept { m_iMaxLevel = level; }

            /**
             * @brief 判断该日志级别是否应当被记录
             */
            bool ShouldLog(Level level)const noexcept { return GetMinLevel() <= level && level <= GetMaxLevel(); }

            /**
             * @brief 获取格式化器
             */
            FormatterPtr GetFormatter()const noexcept { return m_pFormatter; }

            /**
             * @brief 设置格式化器
             */
            void SetFormatter(FormatterPtr p)noexcept { m_pFormatter = p; }

            /**
             * @brief 记录日志
             * @note 要求线程安全
             * @param level 日志级别
             * @param context 日志上下文
             * @param msg 日志消息
             */
            void Log(Level level, const Context& context, const char* msg)const noexcept;

            /**
             * @brief 创建副本
             */
            virtual std::shared_ptr<SinkBase> Clone()const = 0;

        protected:
            virtual void Sink(Level level, const Context& context, const char* msg, const char* formatted,
                size_t length)const noexcept = 0;
            virtual void Flush()const noexcept;

        private:
            bool m_bAlwaysFlush = false;
            Level m_iMinLevel = Level::Debug;
            Level m_iMaxLevel = Level::Fatal;
            FormatterPtr m_pFormatter;
        };

        using SinkPtr = std::shared_ptr<SinkBase>;
        using SinkContainerType = std::vector<SinkPtr>;
        using SinkContainerPtr = std::shared_ptr<std::vector<SinkPtr>>;

        /**
         * @brief 控制台输出支持
         *
         * 支持跨平台带颜色输出到终端。
         */
        class TerminalSink :
            public SinkBase
        {
        public:
            /**
             * @brief 输出类型
             */
            enum class OutputType
            {
                StdOut,  // 标准输出
                StdErr,  // 标准错误输出
            };

            static std::mutex& GetStdOutMutex()noexcept;
            static std::mutex& GetStdErrMutex()noexcept;

        public:
            TerminalSink(OutputType type=OutputType::StdOut);
            TerminalSink(const TerminalSink& rhs);

        public:
            std::shared_ptr<SinkBase> Clone()const override;

        protected:
            void Sink(Level level, const Context& context, const char* msg, const char* formatted,
                size_t length)const noexcept override;
            void Flush()const noexcept override;

        private:
            OutputType m_iType = OutputType::StdOut;
#if defined(MOE_WINDOWS)
            void* m_pFile = nullptr;
#else
            FILE* m_pFile = nullptr;
#endif
            bool m_bDoColor = false;
        };

        /**
         * @brief 获取全局唯一实例
         */
        static Logging& GetInstance()noexcept;

    public:
        /**
         * @brief 获取全局日志最小输出级别（闭区间）
         * @note 线程安全
         */
        Level GetMinLevel()const noexcept { return m_iMinLevel.load(std::memory_order_relaxed); }

        /**
         * @brief 设置全局日志最小输出级别（闭区间）
         * @note 线程安全
         */
        void SetMinLevel(Level level)noexcept { m_iMinLevel.store(level, std::memory_order_relaxed); }

        /**
         * @brief 获取全局日志最大输出级别（闭区间）
         * @note 线程安全
         */
        Level GetMaxLevel()const noexcept { return m_iMaxLevel.load(std::memory_order_relaxed); }

        /**
         * @brief 设置全局日志最大输出级别（闭区间）
         * @note 线程安全
         */
        void SetMaxLevel(Level level)noexcept { m_iMaxLevel.store(level, std::memory_order_relaxed); }

        /**
         * @brief 判断该日志级别是否应当被记录
         * @note 线程安全
         */
        bool ShouldLog(Level level)const noexcept { return GetMinLevel() <= level && level <= GetMaxLevel(); }

        /**
         * @brief 删除所有落地对象
         * @warning 非线程安全，只能在某一线程操作
         */
        void ClearSinks()noexcept { m_stSinks.clear(); }

        /**
         * @brief 是否存在落地对象
         * @warning 非线程安全，只能在某一线程操作
         * @param p 落地对象
         */
        bool ContainsSink(SinkPtr p)const noexcept
        {
            return std::find(m_stSinks.begin(), m_stSinks.end(), p) != m_stSinks.end();
        }

        /**
         * @brief 追加落地对象
         * @warning 非线程安全，只能在某一线程操作
         * @param p 落地对象
         */
        void AppendSink(SinkPtr p)
        {
            if (p && !ContainsSink(p))
                m_stSinks.push_back(p);
        }

        /**
         * @brief 删除落地对象
         * @warning 非线程安全，只能在某一线程操作
         * @param p 落地对象
         */
        bool RemoveSink(SinkPtr p)
        {
            auto it = std::find(m_stSinks.begin(), m_stSinks.end(), p);
            if (it != m_stSinks.end())
            {
                m_stSinks.erase(it);
                return true;
            }
            return false;
        }

        /**
         * @brief 提交并使对Sinks的修改生效
         * @warning 非线程安全，只能在某一线程操作
         *
         * 方法会使当前对落地对象的修改生效。
         */
        void Commit();

        /**
         * @brief 记录日志
         * @tparam Args 格式化参数
         * @note 线程安全
         * @param level 日志级别
         * @param context 日志上下文
         * @param format 格式化文本
         * @param args 格式化参数
         */
        template <typename... Args>
        void Log(Level level, const Context& context, const char* format, const Args&... args)noexcept
        {
            if (!ShouldLog(level))
                return;

            std::string& formatted = GetFormatStringThreadCache();
            formatted.clear();

            try
            {
                StringUtils::Format(formatted, format, args...);
                Sink(level, context, formatted.c_str());
            }
            catch (const std::bad_alloc&)
            {
                Sink(Level::Fatal, context, kAllocErrorMsg);
            }
            catch (...)
            {
                Sink(Level::Fatal, context, kFormatErrorMsg);
            }
        }

        /**
         * @brief 记录日志
         * @tparam Args 格式化参数
         * @note 线程安全
         * @param level 日志级别
         * @param context 日志上下文
         * @param format 格式化文本
         * @param args 格式化参数
         */
        template <typename TChar = char, typename... Args>
        void Log(Level level, const Context& context, const std::string& format, const Args&... args)noexcept
        {
            Log(level, context, format.c_str(), args...);
        }

    private:
        std::string& GetFormatStringThreadCache()const noexcept;
        void Sink(Level level, const Context& context, const char* msg)const noexcept;

    private:
#ifdef NDEBUG
        std::atomic<Level> m_iMinLevel { Level::Info };
#else
        std::atomic<Level> m_iMinLevel { Level::Debug };
#endif
        std::atomic<Level> m_iMaxLevel { Level::Fatal };

        SinkContainerType m_stSinks;  // 当前正在修改的Sinks（非线程安全，无保护）

        mutable std::mutex m_stLock;
        SinkContainerPtr m_stSinksInUse;  // 当前正在使用的Sinks
    };
}

#define MOE_LOG(level, format, ...) \
    do { \
        moe::Logging::GetInstance().Log(level, moe::Logging::Context(__FILE__, __LINE__, __FUNCTION__), format, \
            ##__VA_ARGS__); \
    } while (false)

#define MOE_LOG_DEBUG(format, ...) \
    MOE_LOG(moe::Logging::Level::Debug, format, ##__VA_ARGS__)

#define MOE_LOG_TRACE(format, ...) \
    MOE_LOG(moe::Logging::Level::Trace, format, ##__VA_ARGS__)

#define MOE_LOG_INFO(format, ...) \
    MOE_LOG(moe::Logging::Level::Info, format, ##__VA_ARGS__)

#define MOE_LOG_WARN(format, ...) \
    MOE_LOG(moe::Logging::Level::Warn, format, ##__VA_ARGS__)

#define MOE_LOG_ERROR(format, ...) \
    MOE_LOG(moe::Logging::Level::Error, format, ##__VA_ARGS__)

#define MOE_LOG_FATAL(format, ...) \
    MOE_LOG(moe::Logging::Level::Fatal, format, ##__VA_ARGS__)

#define MOE_LOG_EXCEPTION(ex) \
    do { \
        const auto& except = (ex); \
        moe::Logging::GetInstance().Log(moe::Logging::Level::Error, moe::Logging::Context(__FILE__, __LINE__, \
            __FUNCTION__), "(Exception occurred at {0}:{1},{2}) {3}", \
            moe::PathUtils::GetFileName(except.GetSourceFile()), except.GetLineNumber(), except.GetFunctionName(), \
            except.GetDescription()); \
    } while (false)

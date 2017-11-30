/**
 * @file
 * @date 2017/5/29
 */
#include <Moe.Core/Logging.hpp>
#include <Moe.Core/Exception.hpp>

#include <mutex>
#include <vector>
#include <iostream>
#include <iomanip>

#ifndef MOE_WINDOWS
#include <unistd.h>
#else
#include <Windows.h>
#endif

using namespace std;
using namespace moe;
using namespace Logging;

//////////////////////////////////////////////////////////////////////////////// ConsoleLogger

namespace
{
#ifndef MOE_WINDOWS
    const char* GetLogLevelColor(LogLevel level)
    {
        switch (level)
        {
            case LogLevel::Debug:
                return "\033[37m";  // 白色
            case LogLevel::Trace:
                return "\033[36m";  // 青色
            case LogLevel::Info:
                return "\033[32m";  // 绿色
            case LogLevel::Warn:
                return "\033[33m";  // 黄色
            case LogLevel::Error:
                return "\033[31m";  // 红色
            case LogLevel::Fatal:
                return "\033[41m";  // 红色高亮
        }

        return "";
    }
#else
    class WindowsConsoleHelper :
        public NonCopyable
    {
    public:
        WindowsConsoleHelper()
        {
            ::CONSOLE_SCREEN_BUFFER_INFO consoleBufferInfo;
            ::memset(&consoleBufferInfo, 0, sizeof(consoleBufferInfo));

            m_hOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            m_hErrorHandle = GetStdHandle(STD_ERROR_HANDLE);

            ::GetConsoleScreenBufferInfo(m_hOutputHandle, &consoleBufferInfo);
            m_wOutputOldAttrs = consoleBufferInfo.wAttributes;

            ::GetConsoleScreenBufferInfo(m_hErrorHandle, &consoleBufferInfo);
            m_wErrorOldAttrs = consoleBufferInfo.wAttributes;
        }

    public:
        void RestoreStdOutput()
        {
            ::SetConsoleTextAttribute(m_hOutputHandle, m_wOutputOldAttrs);
        }

        void SetColorStdOutput(::WORD attrs)
        {
            ::SetConsoleTextAttribute(m_hOutputHandle, attrs);
        }

        void RestoreStdError()
        {
            ::SetConsoleTextAttribute(m_hErrorHandle, m_wErrorOldAttrs);
        }

        void SetColorStdError(::WORD attrs)
        {
            ::SetConsoleTextAttribute(m_hErrorHandle, attrs);
        }

    private:
        ::HANDLE m_hOutputHandle = INVALID_HANDLE_VALUE;
        ::HANDLE m_hErrorHandle = INVALID_HANDLE_VALUE;
        ::WORD m_wOutputOldAttrs = 0;
        ::WORD m_wErrorOldAttrs = 0;
    };

    ::WORD GetLogLevelColor(LogLevel level)
    {
        switch (level)
        {
            case LogLevel::Debug:
                return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;  // 白色
            case LogLevel::Trace:
                return FOREGROUND_BLUE | FOREGROUND_INTENSITY;  // 青色
            case LogLevel::Info:
                return FOREGROUND_GREEN | FOREGROUND_INTENSITY;  // 绿色
            case LogLevel::Warn:
                return FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;  // 黄色
            case LogLevel::Error:
                return FOREGROUND_RED | FOREGROUND_INTENSITY;  // 红色
            case LogLevel::Fatal:
                // 红色高亮
                return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_RED | FOREGROUND_INTENSITY;
        }

        return 0;
    }
#endif
}

void ConsoleLogger::Log(LogLevel level, const std::string& message, const char* file, unsigned line,
    const char* function, std::thread::id thread, LoggingTimePoint time)noexcept
{
    auto dt = Time::ToDateTime(time);
    auto& out = level < LogLevel::Warn ? cout : cerr;

#ifndef MOE_WINDOWS
    static const auto istty = ::isatty(STDOUT_FILENO);
    if (istty)
        out << GetLogLevelColor(level);
#else
    static WindowsConsoleHelper s_stHelper;
    if (level < LogLevel::Warn)
        s_stHelper.SetColorStdOutput(GetLogLevelColor(level));
    else
        s_stHelper.SetColorStdError(GetLogLevelColor(level));
#endif

    out << "[" << LogLevelToString(level) << "]";
    out << "[" << Time::ToString(dt) << "]";
    out << "[thread-" << thread << "]";
    out << " " << function << ": " << message << " @ " << file << ":" << line;

#ifndef MOE_WINDOWS
    if (istty)
        out << "\033[0m";
    out << endl;
    out.flush();
#else
    out << endl;
    out.flush();
    if (level < LogLevel::Warn)
        s_stHelper.RestoreStdOutput();
    else
        s_stHelper.RestoreStdError();
#endif
}

//////////////////////////////////////////////////////////////////////////////// FileLogger

FileLogger::FileLogger(const char* path)
    : m_stFile(path, ios::out)
{
    if (!m_stFile)
        MOE_THROW(IOException, "Failed to open file \"{0}\".", path);
}

void FileLogger::Log(LogLevel level, const std::string& message, const char* file, unsigned line, const char* function,
    std::thread::id thread, LoggingTimePoint time)noexcept
{
    auto dt = Time::ToDateTime(time);

    m_stFile << "[" << LogLevelToString(level) << "]";
    m_stFile << "[" << Time::ToString(dt) << "]";
    m_stFile << "[thread-" << thread << "]";
    m_stFile << " " << function << ": " << message << " @ " << file << ":" << line;
    m_stFile << endl;

    m_stFile.flush();
}

//////////////////////////////////////////////////////////////////////////////// Threading

static mutex s_stLoggerLock;
static vector<shared_ptr<Logger>> s_stLoggerList = { make_shared<ConsoleLogger>() };

#ifdef NDEBUG
static LogLevel s_iFilterMinLevel = LogLevel::Info;
#else
static LogLevel s_iFilterMinLevel = LogLevel::Debug;
#endif

void Logging::details::DispatchLogMessage(LogLevel level, const std::string& message, const char* file, unsigned line,
    const char* function, thread::id thread, LoggingTimePoint time)noexcept
{
    unique_lock<mutex> lockGuard(s_stLoggerLock);

    if (level < s_iFilterMinLevel)
        return;

    for (auto it = s_stLoggerList.begin(); it != s_stLoggerList.end(); ++it)
        (*it)->Log(level, message, file, line, function, thread, time);
}

const char* Logging::LogLevelToString(LogLevel level)noexcept
{
    switch (level)
    {
        case LogLevel::Debug:
            return "Debug";
        case LogLevel::Trace:
            return "Trace";
        case LogLevel::Info:
            return "Info";
        case LogLevel::Warn:
            return "Warn";
        case LogLevel::Error:
            return "Error";
        case LogLevel::Fatal:
            return "Fatal";
    }

    return "Unknown";
}

void Logging::AddLogger(std::shared_ptr<Logger> logger)
{
    unique_lock<mutex> lockGuard(s_stLoggerLock);
    s_stLoggerList.emplace_back(logger);
}

void Logging::RemoveLogger(Logger* logger)noexcept
{
    unique_lock<mutex> lockGuard(s_stLoggerLock);
    for (auto it = s_stLoggerList.begin(); it != s_stLoggerList.end(); ++it)
    {
        if (it->get() == logger)
        {
            s_stLoggerList.erase(it);
            return;
        }
    }
}

void Logging::ClearLogger()noexcept
{
    unique_lock<mutex> lockGuard(s_stLoggerLock);
    s_stLoggerList.clear();
}

LogLevel Logging::GetFilterMinLevel()noexcept
{
    unique_lock<mutex> lockGuard(s_stLoggerLock);
    return s_iFilterMinLevel;
}

void Logging::SetFilterMinLevel(LogLevel level)noexcept
{
    unique_lock<mutex> lockGuard(s_stLoggerLock);
    s_iFilterMinLevel = level;
}

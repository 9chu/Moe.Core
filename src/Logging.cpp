/**
 * @file
 * @date 2017/5/29
 */
#include <Moe.Core/Logging.hpp>
#include <Moe.Core/Exception.hpp>
#include <Moe.Core/Encoding.hpp>

#include <mutex>
#include <vector>
#include <atomic>
#include <iostream>
#include <iomanip>
#include <sstream>

#ifdef MOE_WINDOWS
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std;
using namespace moe;

namespace
{
    const char* GetLogLevelString(Logging::Level level)noexcept
    {
        switch (level)
        {
            case Logging::Level::Debug:
                return "DEBUG";
            case Logging::Level::Trace:
                return "TRACE";
            case Logging::Level::Info:
                return "INFO";
            case Logging::Level::Warn:
                return "WARN";
            case Logging::Level::Error:
                return "ERROR";
            case Logging::Level::Fatal:
                return "FATAL";
            default:
                return "";
        }
    }

    struct DateTimeLazyCalc
    {
        bool ShortVersion;
        Time::Timestamp Time;
        mutable char Buffer[16];

        DateTimeLazyCalc(bool shortVersion, Time::Timestamp time)noexcept
            : ShortVersion(shortVersion), Time(time) {}

        const char* ToString()const noexcept
        {
            auto dt = Time::ToDateTime(Time);
            if (ShortVersion)
            {
                snprintf(Buffer, sizeof(Buffer) - 1, "%02d-%02d-%02d", dt.Year % 100, dt.Month, dt.Day);
                Buffer[sizeof(Buffer) - 1] = '\0';
            }
            else
            {
                snprintf(Buffer, sizeof(Buffer) - 1, "%04d-%02d-%02d", dt.Year, dt.Month, dt.Day);
                Buffer[sizeof(Buffer) - 1] = '\0';
            }
            return Buffer;
        }
    };

    struct TimeLazyCalc
    {
        Time::Timestamp Time = 0;
        mutable char Buffer[16];

        TimeLazyCalc(Time::Timestamp time)noexcept
            : Time(time) {}

        const char* ToString()const noexcept
        {
            auto dt = Time::ToDateTime(Time);
            snprintf(Buffer, sizeof(Buffer) - 1, "%02d:%02d:%02d.%03d", dt.Hour, dt.Minutes, dt.Seconds,
                dt.MilliSeconds);
            Buffer[sizeof(Buffer) - 1] = '\0';
            return Buffer;
        }
    };

    struct FilenameLazyCalc
    {
        const char* Path = nullptr;

        FilenameLazyCalc(const char* path)noexcept
            : Path(path) {}

        ArrayView<char> ToString()const noexcept
        {
            return PathUtils::GetFileName(Path);
        }
    };
}

const char* Logging::kFormatErrorMsg = "(Format message failed while logging message)";
const size_t Logging::kFormatErrorMsgLength = strlen(kFormatErrorMsg);
const char* Logging::kAllocErrorMsg = "(Alloc memory failed while logging message)";
const size_t Logging::kAllocErrorMsgLength = strlen(kAllocErrorMsg);

//////////////////////////////////////////////////////////////////////////////// Context

uint64_t Logging::Context::GetThreadIdCached()noexcept
{
    thread_local const uint64_t s_ullTid = Pal::GetCurrentThreadId();
    return s_ullTid;
}

//////////////////////////////////////////////////////////////////////////////// PlainFormatter

void Logging::PlainFormatter::Format(std::string& dest, Level level, const Context& context, const char* msg)const
{
    DateTimeLazyCalc date(false, context.Time);
    DateTimeLazyCalc shortDate(true, context.Time);
    TimeLazyCalc time(context.Time);
    FilenameLazyCalc filename(context.File);

    StringUtils::VariableFormat(dest, m_stFormat.c_str(), make_pair("date", reference_wrapper<DateTimeLazyCalc>(date)),
        make_pair("short_date", reference_wrapper<DateTimeLazyCalc>(shortDate)),
        make_pair("time", reference_wrapper<TimeLazyCalc>(time)), make_pair("level", GetLogLevelString(level)),
        make_pair("path", context.File), make_pair("file", reference_wrapper<FilenameLazyCalc>(filename)),
        make_pair("func", context.Function), make_pair("line", context.Line), make_pair("thread", context.ThreadId),
        make_pair("msg", msg));
}

std::shared_ptr<Logging::FormatterBase> Logging::PlainFormatter::Clone()const
{
    return static_pointer_cast<Logging::FormatterBase>(make_shared<Logging::PlainFormatter>(*this));
}

//////////////////////////////////////////////////////////////////////////////// SinkBase

Logging::SinkBase::SinkBase(const SinkBase& rhs)
    : m_bAlwaysFlush(rhs.m_bAlwaysFlush), m_iMinLevel(rhs.m_iMinLevel), m_iMaxLevel(rhs.m_iMaxLevel),
    m_pFormatter(rhs.m_pFormatter ? rhs.m_pFormatter->Clone() : nullptr)
{
}

void Logging::SinkBase::Log(Level level, const Context& context, const char* msg)const noexcept
{
    if (!ShouldLog(level))
        return;

    if (!m_pFormatter)
        Sink(level, context, msg, msg, strlen(msg));
    else
    {
        thread_local string formatted;

        try
        {
            m_pFormatter->Format(formatted, level, context, msg);
        }
        catch (...)
        {
            Sink(level, context, msg, kFormatErrorMsg, kFormatErrorMsgLength);
            if (IsAlwaysFlush())
                Flush();
            return;
        }

        Sink(level, context, msg, formatted.c_str(), formatted.length());
    }

    if (IsAlwaysFlush())
        Flush();
}

void Logging::SinkBase::Flush()const noexcept
{
}

//////////////////////////////////////////////////////////////////////////////// ConsoleSink

namespace
{
#if defined(MOE_WINDOWS)
    HANDLE GetStdOut()
    {
        return ::GetStdHandle(STD_OUTPUT_HANDLE);
    }

    HANDLE GetStdErr()
    {
        return ::GetStdHandle(STD_ERROR_HANDLE);
    }
#else
    FILE* GetStdOut()
    {
        return stdout;
    }

    FILE* GetStdErr()
    {
        return stderr;
    }
#endif

#if defined(MOE_WINDOWS)
    class WindowsTerminalDefaultStateSaver
    {
    public:
        WindowsTerminalDefaultStateSaver()noexcept
        {
            CONSOLE_SCREEN_BUFFER_INFO stdOutInfo;
            CONSOLE_SCREEN_BUFFER_INFO sttdErrInfo;

            memset(&stdOutInfo, 0, sizeof(stdOutInfo));
            memset(&sttdErrInfo, 0, sizeof(sttdErrInfo));

            ::GetConsoleScreenBufferInfo(GetStdOut(), &stdOutInfo);
            m_uStdOutOldAttr = stdOutInfo.wAttributes;

            ::GetConsoleScreenBufferInfo(GetStdErr(), &sttdErrInfo);
            m_uStdErrOldAttr = stdOutInfo.wAttributes;
        }

    public:
        WORD GetStdOutOldAttr()const noexcept { return m_uStdOutOldAttr; }
        WORD GetStdErrOldAttr()const noexcept { return m_uStdErrOldAttr; }

    private:
        WORD m_uStdOutOldAttr = 0u;
        WORD m_uStdErrOldAttr = 0u;
    };

    // See: https://en.wikipedia.org/wiki/ANSI_escape_code
    class WindowsTerminalEscapeSequenceSimulator
    {
    public:
        WindowsTerminalEscapeSequenceSimulator(HANDLE handle, WORD defaultStatus)noexcept
            : m_hHandle(handle), m_iDefaultStatus(defaultStatus) {}

    public:
        void Print(const char* input, size_t length)
        {
            m_iState = 0;
            m_uLastRegionStart = 0;
            m_uParameterRegionStart = 0;

            for (size_t i = 0; i <= length; ++i)
            {
                auto ch = static_cast<char>(i >= length ? '\0' : input[i]);

                switch (m_iState)
                {
                    case 0:
                        if (ch == 0)
                        {
                            if (m_uLastRegionStart < i)
                            {
                                PrintText(&input[m_uLastRegionStart], i - m_uLastRegionStart);
                                m_uLastRegionStart = i;
                            }
                        }
                        else if (ch == 27)  // ESC
                            m_iState = 1;
                        break;
                    case 1:
                        if (ch == '[')
                        {
                            m_iState = 2;
                            m_uParameterRegionStart = i + 1;
                        }
                        else
                        {
                            m_iState = 0;
                            PrintText(&input[m_uLastRegionStart], i - m_uLastRegionStart);
                            m_uLastRegionStart = i;
                        }
                        break;
                    case 2:
                        if (ch == 'm')
                        {
                            m_iState = 0;
                            PrintText(&input[m_uLastRegionStart], i - m_uLastRegionStart -
                                (i - m_uParameterRegionStart + 2));
                            m_uLastRegionStart = i + 1;

                            ParseParameters(&input[m_uParameterRegionStart], i - m_uParameterRegionStart);
                        }
                        else if (!StringUtils::IsDigit(ch) && ch != ';')
                        {
                            m_iState = 0;
                            PrintText(&input[m_uLastRegionStart], i - m_uLastRegionStart);
                            m_uLastRegionStart = i;
                        }
                        break;
                    default:
                        assert(false);
                        break;
                }
            }
        }

    private:
        void PrintText(const char* start, size_t length)
        {
            thread_local wstring s_stBuffer;
            Encoding::Convert<Encoding::Utf8, Encoding::Utf16, char, wchar_t>(s_stBuffer,
                ArrayView<char>(start, length),
                Encoding::DefaultUnicodeFallbackHandler);

            ::WriteConsoleW(m_hHandle, s_stBuffer.c_str(), static_cast<DWORD>(s_stBuffer.length()), nullptr, nullptr);
        }

        void ParseParameters(const char* start, size_t length)
        {
            CONSOLE_SCREEN_BUFFER_INFO info;
            memset(&info, 0, sizeof(info));
            ::GetConsoleScreenBufferInfo(m_hHandle, &info);

            auto it = StringUtils::SplitByCharsFirst(ArrayView<char>(start, length), ArrayView<char>(";", 1));
            while (it != StringUtils::SplitByCharsLast())
            {
                auto split = *it;
                if (split.GetSize() == 0)
                    InterpretCommand(info.wAttributes, 0);
                else
                {
                    size_t processed = 0;
                    auto result = Convert::ParseInt(split.GetBuffer(), split.GetSize(), processed);
                    InterpretCommand(info.wAttributes, static_cast<int>(result));
                }

                ++it;
            }

            ::SetConsoleTextAttribute(m_hHandle, info.wAttributes);
        }

        void InterpretCommand(WORD& current, int cmd)noexcept
        {
            // 30~37
            static const WORD kFgColors[] = {
                0,  // Black
                FOREGROUND_RED,  // Red
                FOREGROUND_GREEN,  // Green
                FOREGROUND_RED | FOREGROUND_GREEN,  // Yellow
                FOREGROUND_BLUE,  // Blue
                FOREGROUND_RED | FOREGROUND_BLUE,  // Magenta
                FOREGROUND_GREEN | FOREGROUND_BLUE,  // Cyan
                FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,  // White
            };

            // 40~47
            static const WORD kBgColors[] = {
                0,  // Black
                BACKGROUND_RED,  // Red
                BACKGROUND_GREEN,  // Green
                BACKGROUND_RED | BACKGROUND_GREEN,  // Yellow
                BACKGROUND_BLUE,  // Blue
                BACKGROUND_RED | BACKGROUND_BLUE,  // Magenta
                BACKGROUND_GREEN | BACKGROUND_BLUE,  // Cyan
                BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,  // White
            };


            if (cmd == 0)
                current = m_iDefaultStatus;
            else if (cmd >= 30 && cmd <= 37)
                current = static_cast<WORD>((current & ~0xFu) | kFgColors[cmd - 30]);
            else if (cmd >= 90 && cmd <= 97)
                current = static_cast<WORD>((current & ~0xFu) | kFgColors[cmd - 90] | FOREGROUND_INTENSITY);
            else if (cmd >= 40 && cmd <= 47)
                current = static_cast<WORD>((current & ~0xF0u) | kBgColors[cmd - 40]);
            else if (cmd >= 100 && cmd <= 107)
                current = static_cast<WORD>((current & ~0xF0u) | kBgColors[cmd - 100] | BACKGROUND_INTENSITY);
        }

    private:
        HANDLE m_hHandle = nullptr;
        WORD m_iDefaultStatus = 0;
        int m_iState = 0;
        size_t m_uLastRegionStart = 0;
        size_t m_uParameterRegionStart = 0;
    };
#endif
}

std::mutex& Logging::TerminalSink::GetStdOutMutex()noexcept
{
    static std::mutex s_stLock;
    return s_stLock;
}

std::mutex& Logging::TerminalSink::GetStdErrMutex()noexcept
{
    static std::mutex s_stLock;
    return s_stLock;
}

Logging::TerminalSink::TerminalSink(OutputType type)
    : m_iType(type), m_pFile(type == OutputType::StdErr ? GetStdErr() : GetStdOut())
{
#if defined(MOE_WINDOWS)
    m_bDoColor = Pal::IsColorTerminal();
#else
    m_bDoColor = Pal::IsInTerminal(m_pFile) && Pal::IsColorTerminal();
#endif
}

Logging::TerminalSink::TerminalSink(const TerminalSink& rhs)
    : SinkBase(rhs), m_iType(rhs.m_iType), m_pFile(rhs.m_pFile), m_bDoColor(rhs.m_bDoColor)
{
}

std::shared_ptr<Logging::SinkBase> Logging::TerminalSink::Clone()const
{
    return static_pointer_cast<Logging::SinkBase>(make_shared<Logging::TerminalSink>(*this));
}

void Logging::TerminalSink::Sink(Level level, const Context& context, const char* msg, const char* formatted,
    size_t length)const noexcept
{
    MOE_UNUSED(level);
    MOE_UNUSED(context);
    MOE_UNUSED(msg);

    try
    {
        unique_lock<mutex> guard(m_iType == OutputType::StdErr ? GetStdErrMutex() : GetStdOutMutex());
#if defined(MOE_WINDOWS)
        static const WindowsTerminalDefaultStateSaver s_stSaver;

        WindowsTerminalEscapeSequenceSimulator simulator(m_pFile, m_iType == OutputType::StdErr ?
            s_stSaver.GetStdErrOldAttr() : s_stSaver.GetStdOutOldAttr());
        simulator.Print(formatted, length);
        simulator.Print("\r\n", 2);
#else
        ::fwrite(formatted, sizeof(char), length, m_pFile);  // 忽略所有错误
        ::fwrite("\n", sizeof(char), 1, m_pFile);
#endif
    }
    catch (...)
    {
    }
}

void Logging::TerminalSink::Flush()const noexcept
{
#if !defined(MOE_WINDOWS)
    try
    {
        unique_lock<mutex> guard(m_iType == OutputType::StdErr ? GetStdErrMutex() : GetStdOutMutex());
        ::fflush(m_pFile);
    }
    catch (...)
    {
    }
#endif
}

//////////////////////////////////////////////////////////////////////////////// Logging

Logging& Logging::GetInstance()noexcept
{
    static Logging s_stLogging;
    return s_stLogging;
}

void Logging::Commit()
{
    // 构造Sinks的拷贝
    SinkContainerPtr p = make_shared<SinkContainerType>();
    p->reserve(m_stSinks.size());

    for (auto& i : m_stSinks)
        p->emplace_back(i->Clone());

    // 锁定m_stSinksInUse并赋予新值
    {
        unique_lock<mutex> lock(m_stLock);
        m_stSinksInUse = p;
    }
}

std::string& Logging::GetFormatStringThreadCache()const noexcept
{
    thread_local string s_stBuffer;
    return s_stBuffer;
}

void Logging::Sink(Level level, const Context& context, const char* msg)const noexcept
{
    // 获取指针
    SinkContainerPtr p;

    try
    {
        unique_lock<mutex> lock(m_stLock);
        p = m_stSinksInUse;
    }
    catch (...)
    {
        // 直接吃掉异常
        assert(false);
        return;
    }

    // 分发日志
    if (p)
    {
        for (auto it = p->begin(); it != p->end(); ++it)
            (*it)->Log(level, context, msg);
    }
}

#if 0

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

//////////////////////////////////////////////////////////////////////////////// Logging

static mutex s_stLoggerLock;
static vector<shared_ptr<Logger>> s_stLoggerList = { make_shared<ConsoleLogger>() };

#ifdef NDEBUG
static std::atomic<unsigned> s_iFilterMinLevel(static_cast<unsigned>(LogLevel::Info));
#else
static std::atomic<unsigned> s_iFilterMinLevel(static_cast<unsigned>(LogLevel::Debug));
#endif

void Logging::details::DispatchLogMessage(LogLevel level, const std::string& message, const char* file, unsigned line,
    const char* function, thread::id thread, LoggingTimePoint time)noexcept
{
    unique_lock<mutex> lockGuard(s_stLoggerLock);

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
    auto ret = s_iFilterMinLevel.load(memory_order_consume);
    return static_cast<LogLevel>(ret);
}

void Logging::SetFilterMinLevel(LogLevel level)noexcept
{
    s_iFilterMinLevel.store(static_cast<unsigned>(level), memory_order_release);
}
#endif

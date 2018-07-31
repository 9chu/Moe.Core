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

//////////////////////////////////////////////////////////////////////////////// AnsiColorFormatter

Logging::AnsiColorFormatter::AnsiColorFormatter()
{
    SetColor(Level::Debug, Colors::Default, Colors::Default);
    SetColor(Level::Trace, Colors::BrightCyan, Colors::Default);
    SetColor(Level::Info, Colors::BrightGreen, Colors::Default);
    SetColor(Level::Warn, Colors::BrightYellow, Colors::Default);
    SetColor(Level::Error, Colors::BrightRed, Colors::Default);
    SetColor(Level::Fatal, Colors::BrightWhite, Colors::Red);
}

void Logging::AnsiColorFormatter::Format(std::string& dest, Level level, const Context& context, const char* msg)const
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

    // Color it
    auto pair = GetColor(level);
    int fg = 0, bg = 0;
    switch (pair.first)
    {
        case Colors::Black:
            fg = 30;
            break;
        case Colors::Red:
            fg = 31;
            break;
        case Colors::Green:
            fg = 32;
            break;
        case Colors::Yellow:
            fg = 33;
            break;
        case Colors::Blue:
            fg = 34;
            break;
        case Colors::Magenta:
            fg = 35;
            break;
        case Colors::Cyan:
            fg = 36;
            break;
        case Colors::White:
            fg = 37;
            break;
        case Colors::BrightBlack:
            fg = 90;
            break;
        case Colors::BrightRed:
            fg = 91;
            break;
        case Colors::BrightGreen:
            fg = 92;
            break;
        case Colors::BrightYellow:
            fg = 93;
            break;
        case Colors::BrightBlue:
            fg = 94;
            break;
        case Colors::BrightMagenta:
            fg = 95;
            break;
        case Colors::BrightCyan:
            fg = 96;
            break;
        case Colors::BrightWhite:
            fg = 97;
            break;
        default:
            break;
    }

    switch (pair.second)
    {
        case Colors::Black:
            bg = 40;
            break;
        case Colors::Red:
            bg = 41;
            break;
        case Colors::Green:
            bg = 42;
            break;
        case Colors::Yellow:
            bg = 43;
            break;
        case Colors::Blue:
            bg = 44;
            break;
        case Colors::Magenta:
            bg = 45;
            break;
        case Colors::Cyan:
            bg = 46;
            break;
        case Colors::White:
            bg = 47;
            break;
        case Colors::BrightBlack:
            bg = 100;
            break;
        case Colors::BrightRed:
            bg = 101;
            break;
        case Colors::BrightGreen:
            bg = 102;
            break;
        case Colors::BrightYellow:
            bg = 103;
            break;
        case Colors::BrightBlue:
            bg = 104;
            break;
        case Colors::BrightMagenta:
            bg = 105;
            break;
        case Colors::BrightCyan:
            bg = 106;
            break;
        case Colors::BrightWhite:
            bg = 107;
            break;
        default:
            break;
    }

    if (fg != 0 || bg != 0)
    {
        char indicator[16] = { 0 };
        char buf[16] = { 0 };

        indicator[0] = '\033';
        indicator[1] = '[';

        if (fg != 0)
        {
            Convert::ToDecimalString(fg, buf);
            strcat(indicator, buf);
        }
        if (bg != 0)
        {
            if (fg != 0)
                strcat(indicator, ";");

            Convert::ToDecimalString(bg, buf);
            strcat(indicator, buf);
        }

        strcat(indicator, "m");
        dest.insert(0, indicator);
        dest.append("\033[m");
    }
}

std::shared_ptr<Logging::FormatterBase> Logging::AnsiColorFormatter::Clone()const
{
    return static_pointer_cast<Logging::FormatterBase>(make_shared<Logging::AnsiColorFormatter>(*this));
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

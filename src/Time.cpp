/**
 * @file
 * @date 2017/7/8
 * @see https://github.com/lhmouse/poseidon/blob/master/src/time.cpp
 * @see https://github.com/lhmouse/MCF/blob/master/MCFCRT/src/env/clocks.c
 */
#include <Moe.Core/Time.hpp>
#include <Moe.Core/Utils.hpp>
#include <Moe.Core/Logging.hpp>

#ifdef MOE_WINDOWS
#include <Windows.h>
#endif

using namespace std;
using namespace moe;

namespace
{
    class TimeZoneProvider
    {
    public:
        TimeZoneProvider()noexcept
        {
#ifdef MOE_WINDOWS
            // 下述代码其实没有必要，可以统一使用::timezone
            TIME_ZONE_INFORMATION tzInfo;
            ::memset(&tzInfo, 0, sizeof(tzInfo));

            auto ret = ::GetTimeZoneInformation(&tzInfo);
            assert(ret != TIME_ZONE_ID_INVALID);
            MOE_UNUSED(ret);

            m_llTimeZoneBias = static_cast<Time::TimestampOffset>(tzInfo.Bias * 60000ll);
#else
            ::tzset();

            m_llTimeZoneBias = static_cast<Time::TimestampOffset>(::timezone * 1000ll);
#endif
        }

    public:
        Time::TimestampOffset GetTimeZoneBias()const noexcept { return m_llTimeZoneBias; }

    private:
        Time::TimestampOffset m_llTimeZoneBias = 0;
    };

#ifdef MOE_WINDOWS
    class PerformanceCounterProvider
    {
    public:
        PerformanceCounterProvider()
        {
            LARGE_INTEGER freq;
            ::memset(&freq, 0, sizeof(freq));

            auto ret = ::QueryPerformanceFrequency(&freq);
            assert(ret);
            MOE_UNUSED(ret);

            m_dPerformanceFreq = 1000. / static_cast<double>(freq.QuadPart);
        }

    public:
        double GetFrequency()const noexcept { return m_dPerformanceFreq; }

    private:
        double m_dPerformanceFreq = 0.;
    };
#endif
}

Time::Timestamp Time::GetUtcTime()noexcept
{
#ifdef MOE_WINDOWS
    union
    {
        FILETIME ft;
        LARGE_INTEGER li;
    } utc;

    ::GetSystemTimeAsFileTime(&utc.ft);

    // 0x019DB1DED53E8000 = duration since 1601-01-01 until 1970-01-01 in nanoseconds.
    return (uint64_t)(((double)utc.li.QuadPart - 0x019DB1DED53E8000ll) / 10000.0);
#else
    ::timespec ts;
    ::memset(&ts, 0, sizeof(ts));

    int ret = ::clock_gettime(CLOCK_REALTIME, &ts);
    assert(ret == 0);
    MOE_UNUSED(ret);

    return (uint64_t)ts.tv_sec * 1000 + (uint32_t)ts.tv_nsec / 1000000;
#endif
}

Time::TimestampOffset Time::GetTimeZoneOffset()noexcept
{
    static TimeZoneProvider s_stProvider;
    return s_stProvider.GetTimeZoneBias();
}

Time::Tick Time::GetTickCount()noexcept
{
#ifdef MOE_WINDOWS
#if _WIN32_WINNT >= 0x0600
    return ::GetTickCount64();
#else
    return ::GetTickCount();
#endif
#else
    ::timespec ts;
    ::memset(&ts, 0, sizeof(ts));

    int ret = ::clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(ret == 0);
    MOE_UNUSED(ret);

    return (uint64_t)ts.tv_sec * 1000 + (uint32_t)ts.tv_nsec / 1000000;
#endif
}

Time::HiResTick Time::GetHiResTickCount()noexcept
{
#ifdef MOE_WINDOWS
    static PerformanceCounterProvider s_stProvider;

    LARGE_INTEGER counter;
    ::memset(&counter, 0, sizeof(counter));

    auto ret = ::QueryPerformanceCounter(&counter);
    assert(ret);
    MOE_UNUSED(ret);

    return (double)counter.QuadPart * s_stProvider.GetFrequency();
#else
    ::timespec ts;
    ::memset(&ts, 0, sizeof(ts));

    int ret = ::clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(ret == 0);
    MOE_UNUSED(ret);

    return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec / 1e6;
#endif
}

Time::DateTime Time::ToDateTime(Timestamp ts)noexcept
{
    const ::time_t seconds = static_cast<::time_t>(ts / 1000);
    const auto milliseconds = ts % 1000;

    ::tm desc;
    ::memset(&desc, 0, sizeof(desc));
#ifdef MOE_WINDOWS
    ::gmtime_s(&desc, &seconds);
#else
    ::gmtime_r(&seconds, &desc);
#endif

    DateTime dt;
    dt.Year = static_cast<unsigned>(1900 + desc.tm_year);
    dt.Month = static_cast<unsigned>(1 + desc.tm_mon);
    dt.Day = static_cast<unsigned>(desc.tm_mday);
    dt.Hour = static_cast<unsigned>(desc.tm_hour);
    dt.Minutes = static_cast<unsigned>(desc.tm_min);
    dt.Seconds = static_cast<unsigned>(desc.tm_sec);
    dt.MilliSeconds = static_cast<unsigned>(milliseconds);
    return dt;
}

Time::Timestamp Time::ToTimestamp(const DateTime& dt)noexcept
{
    ::tm desc;
    ::memset(&desc, 0, sizeof(desc));

    desc.tm_year = static_cast<int>(dt.Year - 1900);
    desc.tm_mon  = static_cast<int>(dt.Month - 1);
    desc.tm_mday = static_cast<int>(dt.Day);
    desc.tm_hour = static_cast<int>(dt.Hour);
    desc.tm_min  = static_cast<int>(dt.Minutes);
    desc.tm_sec  = static_cast<int>(dt.Seconds);
#ifdef MOE_WINDOWS
    return static_cast<uint64_t>(::_mkgmtime(&desc)) * 1000 + dt.MilliSeconds;
#else
    return static_cast<uint64_t>(::timegm(&desc)) * 1000 + dt.MilliSeconds;
#endif
}

std::string Time::ToString(Timestamp ts)
{
    DateTime dt = { 0, 1, 1, 0, 0, 0, 0 };
    if (ts == 0)
        dt.Year = 0;
    else if (ts == static_cast<Timestamp>(-1))
        dt.Year = 9999;
    else
        dt = ToDateTime(ts);
    return ToString(dt);
}

std::string Time::ToString(const DateTime& dt)
{
    char buffer[32] = { 0 };
    ::snprintf(buffer, sizeof(buffer) - 1, "%04u-%02u-%02u %02u:%02u:%02u.%03u", dt.Year, dt.Month, dt.Day, dt.Hour,
        dt.Minutes, dt.Seconds, dt.MilliSeconds);
    return string(buffer);
}

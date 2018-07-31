/**
 * @file
 * @date 2017/7/8
 * @see https://github.com/lhmouse/poseidon/blob/master/src/time.cpp
 * @see https://github.com/lhmouse/MCF/blob/master/MCFCRT/src/env/clocks.c
 */
#include <Moe.Core/Time.hpp>
#include <Moe.Core/Utils.hpp>
#include <Moe.Core/Logging.hpp>

using namespace std;
using namespace moe;
using namespace Time;

TimestampOffset Time::GetTimeZoneOffset()noexcept
{
    static const auto s_llBias = Pal::GetTimeZoneOffset();
    return s_llBias;
}

DateTime Time::ToDateTime(Timestamp ts)noexcept
{
    const auto seconds = static_cast<::time_t>(ts / 1000);
    const auto milliseconds = ts % 1000;

    ::tm desc;
    Pal::TimestampToDateTime(desc, seconds);

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

Timestamp Time::ToTimestamp(const DateTime& dt)noexcept
{
    ::tm desc;
    memset(&desc, 0, sizeof(desc));

    desc.tm_year = static_cast<int>(dt.Year - 1900);
    desc.tm_mon  = static_cast<int>(dt.Month - 1);
    desc.tm_mday = static_cast<int>(dt.Day);
    desc.tm_hour = static_cast<int>(dt.Hour);
    desc.tm_min  = static_cast<int>(dt.Minutes);
    desc.tm_sec  = static_cast<int>(dt.Seconds);
    return static_cast<uint64_t>(Pal::DateTimeToTimestamp(desc)) * 1000 + dt.MilliSeconds;
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

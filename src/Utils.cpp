/**
 * @file
 * @date 2017/5/29
 */
#include <Moe.Core/Utils.hpp>

#include <cstring>

using namespace std;
using namespace moe;

::tm Time::ToLocalDateTime(::time_t time)noexcept
{
    ::tm ret;
    ::memset(&ret, 0, sizeof(ret));

#ifdef MOE_WINDOWS
    ::localtime_s(&ret, &time);
#else
    ::localtime_r(&time, &ret);
#endif

    return ret;
}

::time_t Time::CurrentTime()noexcept
{
    return time(NULL);
}

unsigned Time::GetSystemTick()noexcept
{
    return static_cast<unsigned>(clock() * 1000ull / CLOCKS_PER_SEC);
}

/**
 * @file
 * @date 2017/5/29
 */
#include <Moe.Core/Utils.hpp>

using namespace std;
using namespace moe;

::tm Time::ToLocalDateTime(::time_t time)
{
    ::tm ret = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

#ifdef MOE_WINDOWS
    ::localtime_s(&ret, &time);
#else
    ::localtime_r(&time, &ret);
#endif

    return ret;
}

::time_t Time::CurrentTime()
{
    return time(NULL);
}

unsigned Time::GetSystemTick()
{
    return static_cast<unsigned>(clock() * 1000ull / CLOCKS_PER_SEC);
}

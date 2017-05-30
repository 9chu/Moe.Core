/**
 * @file
 * @date 2017/5/29
 */
#include <Moe.Core/Threading.hpp>

#include <Windows.h>

using namespace std;
using namespace moe;
using namespace Threading;

//////////////////////////////////////////////////////////////////////////////// Sleeper

void Sleeper::Wait()
{
    if (m_uSpinCount < kMaxActiveSpin)
    {
        ++m_uSpinCount;
        AsmVolatilePause();
    }
    else
    {
#ifdef MOE_WINDOWS
        ::Sleep(0);
#else
        /*
         * Always sleep 0.5ms, assuming this will make the kernel put
         * us down for whatever its minimum timer resolution is (in
         * linux this varies by kernel version from 1ms to 10ms).
         */
        struct timespec ts = { 0, 500000 };
        ::nanosleep(&ts, nullptr);
#endif
    }
}

//////////////////////////////////////////////////////////////////////////////// Event

Event::Event(bool set, bool autoReset)
    : m_bEventSet(set), m_bAutoReset(autoReset) {}

void Event::Wait()
{
    unique_lock<mutex> lockGuard(m_stLock);

    while (!m_bEventSet)
        m_stCondVar.wait(lockGuard);

    if (m_bAutoReset)
        m_bEventSet = false;
}

bool Event::WaitFor(WaitingDuration duration)
{
    unique_lock<mutex> lockGuard(m_stLock);

    auto status = m_stCondVar.wait_for(lockGuard, duration, [=]() {
        return m_bEventSet;
    });

    if (!status)
        return false;

    if (m_bAutoReset)
        m_bEventSet = false;

    return true;
}

void Event::Set()
{
    unique_lock<mutex> lockGuard(m_stLock);
    m_bEventSet = true;
    m_stCondVar.notify_all();
}

void Event::Reset()
{
    unique_lock<mutex> lockGuard(m_stLock);
    m_bEventSet = false;
}

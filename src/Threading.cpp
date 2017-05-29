/**
 * @file
 * @date 2017/5/29
 */
#include <Moe.Core/Threading.hpp>

using namespace std;
using namespace moe;
using namespace Threading;

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

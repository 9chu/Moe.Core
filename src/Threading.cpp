/**
 * @file
 * @date 2017/5/29
 */
#include <Moe.Core/Threading.hpp>
#include <Moe.Core/Exception.hpp>
#include <Moe.Core/Encoding.hpp>

#ifdef MOE_WINDOWS
#include <Windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif

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

//////////////////////////////////////////////////////////////////////////////// SharedMemory

namespace
{
    bool ValidateSharedMemoryName(const std::string& name)noexcept
    {
        for (auto c : name)
        {
            if (c == '\\' || c == '/')
                return false;
        }
        return true;
    }

    std::string MakePlatformSpecificShmName(const std::string& name)
    {
#ifdef MOE_WINDOWS
        static const string kPerfix("Local\\");
#else
        static const string kPerfix("/");
#endif
        return kPerfix + name;
    }
}

SharedMemory::SharedMemory()noexcept
{
}

SharedMemory::SharedMemory(const char* name, size_t sz, AttachMode mode)
    : m_stName(name), m_uSize(sz + sizeof(Header))
{
    if (!ValidateSharedMemoryName(m_stName))
        MOE_THROW(BadArgumentException, "Invalid shared memory name \"{0}\"", name);
    
#ifdef MOE_WINDOWS
    static_assert(sizeof(wchar_t) == sizeof(char16_t), "Bad platform");
    m_stPlatformName = Encoding::Convert<Encoding::UTF16, Encoding::UTF8>(MakePlatformSpecificShmName(m_stName));

    switch (mode)
    {
        case AttachMode::AttachOnly:
            m_hHandle = ::OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE,
                reinterpret_cast<const wchar_t*>(m_stPlatformName.c_str()));
            if (!m_hHandle)
            {
                auto err = GetLastError();
                MOE_THROW(ApiException, "Open shared memory \"{0}\" failed, err={1}", m_stName, err);
            }
            m_bCreateMode = false;
            break;
        default:
            m_hHandle = ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, m_uSize,
                reinterpret_cast<const wchar_t*>(m_stPlatformName.c_str()));
            if (!m_hHandle)
            {
                auto err = GetLastError();
                MOE_THROW(ApiException, "Create shared memory \"{0}\" failed, err={1}", m_stName, err);
            }
            if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                if (mode == AttachMode::CreateOnly)
                {
                    ::CloseHandle(m_hHandle);
                    MOE_THROW(ApiException, "Shared memory \"{0}\" already exsists", m_stName);
                }
                m_bCreateMode = false;
            }
            else
                m_bCreateMode = true;
            break;
    }

    assert(m_hHandle);
    m_pMappingData = static_cast<Header*>(::MapViewOfFile(m_hHandle, FILE_MAP_ALL_ACCESS, 0, 0, m_uSize));
    if (!m_pMappingData)
    {
        auto err = GetLastError();

        ::CloseHandle(m_hHandle);
        MOE_THROW(ApiException, "Map shared memory \"{0}\" failed, err={1}", m_stName, err);
    }
#else
    m_stPlatformName = MakePlatformSpecificShmName(m_stName);

    switch (mode)
    {
    case AttachMode::AttachOnly:
        m_iFd = ::shm_open(m_stPlatformName.c_str(), O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
        if (m_iFd == -1)
        {
            auto err = errno;
            MOE_THROW(ApiException, "Open shared memory \"{0}\" failed, err={1}", m_stName, err);
        }
        m_bCreateMode = false;
        break;
    case AttachMode::CreateOnly:
        m_iFd = ::shm_open(m_stPlatformName.c_str(), O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
        if (m_iFd == -1)
        {
            auto err = errno;
            if (err == EEXIST)
                MOE_THROW(ApiException, "Shared memory \"{0}\" already exsists", m_stName);
            MOE_THROW(ApiException, "Create shared memory \"{0}\" failed, err={1}", m_stName, err);
        }
        if (::ftruncate(m_iFd, m_uSize) == -1)
        {
            auto err = errno;
            ::close(m_iFd);
            ::shm_unlink(m_stPlatformName.c_str());
            MOE_THROW(ApiException, "Create shared memory \"{0}\" failed, err={1}", m_stName, err);
        }
        m_bCreateMode = true;
        break;
    default:
        m_bCreateMode = false;
        m_iFd = ::shm_open(m_stPlatformName.c_str(), O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
        if (m_iFd == -1)  // 打开失败的情况下会尝试创建
        {
            m_iFd = ::shm_open(m_stPlatformName.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
            if (m_iFd == -1)
            {
                auto err = errno;
                MOE_THROW(ApiException, "Create shared memory \"{0}\" failed, err={1}", m_stName, err);
            }
            if (::ftruncate(m_iFd, m_uSize) == -1)
            {
                auto err = errno;
                ::close(m_iFd);
                ::shm_unlink(m_stPlatformName.c_str());
                MOE_THROW(ApiException, "Create shared memory \"{0}\" failed, err={1}", m_stName, err);
            }
            m_bCreateMode = true;
        }
        break;
    }

    assert(m_iFd != -1);
    m_pMappingData = static_cast<Header*>(::mmap(0, m_uSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_iFd, 0));
    if (m_pMappingData == MAP_FAILED)
    {
        auto err = errno;
        if (m_bCreateMode)
        {
            ::close(m_iFd);
            ::shm_unlink(m_stPlatformName.c_str());
        }
        MOE_THROW(ApiException, "Map shared memory \"{0}\" failed, err={1}", m_stName, err);
    }
#endif

    assert(m_pMappingData);

    if (m_bCreateMode)
    {
        m_pMappingData->Magic[0] = 'M';
        m_pMappingData->Magic[1] = 'O';
        m_pMappingData->Magic[2] = 'E';
        m_pMappingData->Magic[3] = '\0';
        ::memset(m_pMappingData->Padding, 0, sizeof(Header::Padding));
        m_pMappingData->Size = m_uSize;
    }
    else
    {
        // 校验共享内存数据
        if (m_pMappingData->Magic[0] != 'M' ||
            m_pMappingData->Magic[1] != 'O' ||
            m_pMappingData->Magic[2] != 'E' ||
            m_pMappingData->Magic[3] != '\0')
        {
            MOE_THROW(BadFormatException, "Header check failed");
        }
        if (m_pMappingData->Size != m_uSize)
            MOE_THROW(BadFormatException, "Shared memory size mismatched");
    }
}

SharedMemory::SharedMemory(SharedMemory&& org)noexcept
    : m_stName(std::move(org.m_stName)), m_stPlatformName(std::move(org.m_stPlatformName)), m_uSize(org.m_uSize),
    m_pMappingData(org.m_pMappingData), m_bCreateMode(org.m_bCreateMode), m_bAutoFree(org.m_bAutoFree)
{
#ifdef MOE_WINDOWS
    m_hHandle = org.m_hHandle;
    org.m_hHandle = nullptr;
#else
    m_iFd = org.m_iFd;
    org.m_iFd = -1;
#endif

    org.m_uSize = 0;
    org.m_pMappingData = nullptr;
    org.m_bCreateMode = false;
    org.m_bAutoFree = false;
}

SharedMemory::~SharedMemory()
{
#ifdef MOE_WINDOWS
    Free();
#else
    if (m_bAutoFree)
        Free();
#endif
}

SharedMemory::operator bool()const noexcept
{
#ifdef MOE_WINDOWS
    return m_hHandle != nullptr && m_pMappingData;
#else
    return m_iFd != -1 && m_pMappingData;
#endif
}

SharedMemory& SharedMemory::operator=(SharedMemory&& rhs)noexcept
{
    m_stName = std::move(rhs.m_stName);
    m_stPlatformName = std::move(rhs.m_stPlatformName);

#ifdef MOE_WINDOWS
    m_hHandle = rhs.m_hHandle;
    rhs.m_hHandle = nullptr;
#else
    m_iFd = rhs.m_iFd;
    rhs.m_iFd = -1;
#endif

    m_uSize = rhs.m_uSize;
    rhs.m_uSize = 0u;
    
    m_pMappingData = rhs.m_pMappingData;
    rhs.m_pMappingData = nullptr;

    m_bCreateMode = rhs.m_bCreateMode;
    rhs.m_bCreateMode = false;
    
    m_bAutoFree = rhs.m_bAutoFree;
    rhs.m_bAutoFree = false;

    return *this;
}

void SharedMemory::Free()noexcept
{
#ifdef MOE_WINDOWS
    if (m_pMappingData)
    {
        ::UnmapViewOfFile(m_pMappingData);
        m_pMappingData = nullptr;
    }
    if (m_hHandle)
    {
        ::CloseHandle(m_hHandle);
        m_hHandle = nullptr;
    }
#else
    if (m_pMappingData)
    {
        ::munmap(m_pMappingData, m_uSize);
        m_pMappingData = nullptr;
    }
    if (m_iFd != -1)
    {
        ::close(m_iFd);
        ::shm_unlink(m_stPlatformName.c_str());
        m_iFd = -1;
    }
#endif

    m_stName.clear();
    m_stPlatformName.clear();
    m_uSize = 0;
    m_bCreateMode = false;
    m_bAutoFree = false;
}

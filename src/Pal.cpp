/**
 * @file
 * @author chu
 * @date 2018/7/29
 */
#include <Moe.Core/Pal.hpp>
#include <Moe.Core/Utils.hpp>
#include <Moe.Core/Exception.hpp>
#include <Moe.Core/Encoding.hpp>

#include <cmath>
#include <chrono>
#include <thread>
#include <algorithm>

#if defined(MOE_WINDOWS)
#include <Windows.h>
#include <io.h>
#include <process.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(MOE_MINGW)
#include <share.h>
#endif
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef MOE_LINUX
#include <sys/syscall.h>
#elif MOE_FREEBSD
#include <sys/thr.h>
#endif
#endif

#ifdef _MSC_VER
#include <intrin.h>
#endif

using namespace std;
using namespace moe;
using namespace Pal;

////////////////////////////////////////////////////////////////////////////////

namespace
{
#ifdef MOE_WINDOWS
    class PerformanceCounterProvider
    {
    public:
        PerformanceCounterProvider()
        {
            LARGE_INTEGER freq;
            memset(&freq, 0, sizeof(freq));

            auto ret = ::QueryPerformanceFrequency(&freq);
            if (ret == FALSE)
            {
                assert(false);
                return;
            }
            m_dPerformanceFreq = 1000 / static_cast<double>(freq.QuadPart);
        }

    public:
        double GetFrequency()const noexcept { return m_dPerformanceFreq; }

    private:
        double m_dPerformanceFreq = 0;
    };
#endif

#if !defined(MOE_WINDOWS)
    uint64_t GetRealTimeClockFallback()noexcept
    {
        static const auto s_stEpoch(chrono::system_clock::from_time_t(0));
        auto now = chrono::system_clock::now() - s_stEpoch;
        return static_cast<uint64_t>(chrono::duration_cast<chrono::milliseconds>(now).count());
    }
#endif

#if !defined(MOE_POSIX)
    std::pair<uint64_t, uint64_t> GetMonotonicClockFallback()noexcept
    {
        auto now = chrono::high_resolution_clock::now().time_since_epoch();
        auto ms = chrono::duration_cast<chrono::milliseconds>(now);
        auto ns = chrono::duration_cast<chrono::nanoseconds>(now - ms);
        return make_pair<uint64_t, uint32_t>(static_cast<uint64_t>(ms.count()), static_cast<uint32_t>(ns.count()));
    }
#endif
}

uint64_t Pal::GetRealTimeClock()noexcept
{
#if defined(MOE_WINDOWS)
    union
    {
        FILETIME ft;
        LARGE_INTEGER li;
    } utc;

    ::GetSystemTimeAsFileTime(&utc.ft);

    // 0x019DB1DED53E8000 = duration since 1601-01-01 until 1970-01-01 in nanoseconds.
    return (static_cast<uint64_t>(utc.li.QuadPart) - 0x019DB1DED53E8000ull) / 10000;
#elif defined(MOE_POSIX)
    ::timespec ts;
    memset(&ts, 0, sizeof(ts));

    if (::clock_gettime(CLOCK_REALTIME, &ts) < 0)
    {
        assert(false);
        return GetRealTimeClockFallback();
    }
    return static_cast<uint64_t>(ts.tv_sec) * 1000 + static_cast<uint32_t>(ts.tv_nsec) / 1000000;
#else
    return GetRealTimeClockFallback();
#endif
}

std::pair<uint64_t, uint64_t> Pal::GetMonotonicClock()noexcept
{
#ifdef MOE_WINDOWS
    static PerformanceCounterProvider s_stProvider;

    auto freq = s_stProvider.GetFrequency();
    if (freq == 0.)
        return GetMonotonicClockFallback();

    LARGE_INTEGER counter;
    memset(&counter, 0, sizeof(counter));

    auto ret = ::QueryPerformanceCounter(&counter);
    if (ret == FALSE)
    {
        assert(false);
        return make_pair<uint64_t, uint32_t>(0, 0);  // 不fallback到chrono，以防止意外
    }

    double frac, integer = 0.;
    frac = modf(static_cast<double>(counter.QuadPart) * freq, &integer);
    return make_pair<uint64_t, uint32_t>(static_cast<uint64_t>(integer), static_cast<uint32_t>(frac * 1000000.));
#elif defined(MOE_POSIX)
    ::timespec ts;
    memset(&ts, 0, sizeof(ts));

    if (::clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
    {
        assert(false);
        return make_pair<uint64_t, uint32_t>(0, 0);  // 不fallback到chrono，以防止意外
    }

    return make_pair<uint64_t, uint32_t>(
        static_cast<uint64_t>(ts.tv_sec) * 1000 + static_cast<uint32_t>(ts.tv_nsec) / 1000000,
        static_cast<uint32_t>(ts.tv_nsec) % 1000000);
#else
    return GetMonotonicClockFallback();
#endif
}

int64_t Pal::GetTimeZoneOffset()noexcept
{
#ifdef MOE_WINDOWS
    TIME_ZONE_INFORMATION tz;
    memset(&tz, 0, sizeof(tz));

    auto ret = ::GetTimeZoneInformation(&tz);
    assert(ret != TIME_ZONE_ID_INVALID);
    MOE_UNUSED(ret);

    return -static_cast<int64_t>(tz.Bias * 60000ll);
#else
    ::tzset();
    return -static_cast<int64_t>(::timezone * 1000ll);
#endif
}

void Pal::TimestampToDateTime(::tm& out, ::time_t clock)noexcept
{
    memset(&out, 0, sizeof(out));
#ifdef MOE_WINDOWS
    ::gmtime_s(&out, &clock);
#else
    ::gmtime_r(&clock, &out);
#endif
}

time_t Pal::DateTimeToTimestamp(const ::tm& date)noexcept
{
    ::tm copy = date;
#ifdef MOE_WINDOWS
    return ::_mkgmtime(&copy);
#else
    return ::timegm(&copy);
#endif
}

////////////////////////////////////////////////////////////////////////////////

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
    m_stPlatformName = Encoding::Convert<Encoding::Utf8, Encoding::Utf16>(MakePlatformSpecificShmName(m_stName),
        Encoding::DefaultUnicodeFallbackHandler);

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
            m_hHandle = ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
                static_cast<DWORD>(m_uSize / numeric_limits<DWORD>::max()),
                static_cast<DWORD>(m_uSize % numeric_limits<DWORD>::max()),
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
        {
            MOE_THROW(BadFormatException, "Shared memory size mismatched, {0} expected, but get {1}", m_uSize,
                m_pMappingData->Size);
        }
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

////////////////////////////////////////////////////////////////////////////////

void Pal::Pause()noexcept
{
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
    ::_mm_pause();
#elif defined(__i386__) || (defined(__x86_64__) || defined(_M_X64))
    asm volatile("pause");
#elif defined(__aarch64__) || defined(__arm__)
    asm volatile("yield");
#elif defined(__powerpc64__)
    asm volatile("or 27,27,27");
#else
    this_thread::yield();
#endif
}

void Pal::FastSleep()noexcept
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

uint64_t Pal::GetCurrentThreadId()noexcept
{
#if defined(MOE_WINDOWS)
    return static_cast<uint64_t>(::GetCurrentThreadId());
#elif defined(MOE_LINUX)
#if defined(MOE_ANDROID) && defined(__ANDROID_API__) && (__ANDROID_API__ < 21)
#define SYS_gettid __NR_gettid
#endif
    return static_cast<uint64_t>(::syscall(SYS_gettid));
#elif defined(MOE_FREEBSD)
    long tid = 0;
    ::thr_self(&tid);
    return static_cast<uint64_t>(tid);
#elif defined(MOE_APPLE)
    uint64_t tid = 0;
    ::pthread_threadid_np(nullptr, &tid);
    return static_cast<uint64_t>(tid);
#else
    return static_cast<uint64_t>(hash<thread::id>()(this_thread::get_id()));
#endif
}

uint64_t Pal::GetCurrentProcessId()noexcept
{
#if defined(MOE_WINDOWS)
    return static_cast<int>(::GetCurrentProcessId());
#else
    return static_cast<int>(::getpid());
#endif
}

////////////////////////////////////////////////////////////////////////////////

// see: https://github.com/agauniyal/rang/
bool Pal::IsColorTerminal()noexcept
{
#if defined(MOE_WINDOWS)
    return true;
#else
    static constexpr const char* s_stTerms[] = {
        "ansi", "color", "console", "cygwin", "gnome", "konsole", "kterm", "linux", "msys", "putty", "rxvt", "screen",
        "vt100", "xterm"
    };

    const char* env = getenv("TERM");
    if (env == nullptr)
        return false;

    for (size_t i = 0; i < CountOf(s_stTerms); ++i)
    {
        if (std::strstr(env, s_stTerms[i]) != nullptr)
            return true;
    }
    return false;
#endif
}

// see: https://github.com/agauniyal/rang/
bool Pal::IsInTerminal(FILE* file)noexcept
{
#if defined(MOE_WINDOWS)
    return ::_isatty(::_fileno(file)) != 0;
#else
    return ::isatty(::fileno(file)) != 0;
#endif
}

////////////////////////////////////////////////////////////////////////////////

FILE* Pal::OpenFile(const char* path, const char* mode)
{
    FILE* ret = nullptr;
#if defined(MOE_WINDOWS)
    wstring wpath, wmode;
    Encoding::Convert<Encoding::Utf8, Encoding::Utf16, char, wchar_t>(wpath, ArrayView<char>(path, strlen(path)),
        Encoding::DefaultUnicodeFallbackHandler);
    Encoding::Convert<Encoding::Utf8, Encoding::Utf16, char, wchar_t>(wmode, ArrayView<char>(mode, strlen(mode)),
        Encoding::DefaultUnicodeFallbackHandler);

    ret = ::_wfsopen(wpath.c_str(), wmode.c_str(), strstr(mode, "w") != nullptr ? _SH_DENYWR : 0);
#else
    ret = ::fopen(path, mode);
#endif
    if (!ret)
        MOE_THROW(ApiException, "Open file \"{0}\" failed, errno={1}", path, errno);
    return ret;
}

void Pal::RemoveFile(const char* path)
{
#if defined(MOE_WINDOWS)
    wstring wpath;
    Encoding::Convert<Encoding::Utf8, Encoding::Utf16, char, wchar_t>(wpath, ArrayView<char>(path, strlen(path)),
        Encoding::DefaultUnicodeFallbackHandler);

    auto ret = ::_wremove(wpath.c_str());
#else
    auto ret = ::remove(path);
#endif
    if (ret != 0)
        MOE_THROW(ApiException, "Remove file \"{0}\" failed, errno={1}", path, errno);
}

void Pal::RenameFile(const char* dest, const char* src)
{
#if defined(MOE_WINDOWS)
    wstring wdest, wsrc;
    Encoding::Convert<Encoding::Utf8, Encoding::Utf16, char, wchar_t>(wdest, ArrayView<char>(dest, strlen(dest)),
        Encoding::DefaultUnicodeFallbackHandler);
    Encoding::Convert<Encoding::Utf8, Encoding::Utf16, char, wchar_t>(wsrc, ArrayView<char>(src, strlen(src)),
        Encoding::DefaultUnicodeFallbackHandler);

    auto ret = ::_wrename(wsrc.c_str(), wdest.c_str());
#else
    auto ret = ::rename(src, dest);
#endif
    if (ret != 0)
        MOE_THROW(ApiException, "Rename file \"{0}\" -> \"{1}\" failed, errno={2}", src, dest, errno);
}

bool Pal::IsFileExists(const char* path)
{
#if defined(MOE_WINDOWS)
    wstring wpath;
    Encoding::Convert<Encoding::Utf8, Encoding::Utf16, char, wchar_t>(wpath, ArrayView<char>(path, strlen(path)),
        Encoding::DefaultUnicodeFallbackHandler);

    auto attr = ::GetFileAttributesW(wpath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        auto err = ::GetLastError();
        if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
            return false;
        MOE_THROW(ApiException, "Stat on file \"{0}\" failed, lastError={1}", path, err);
    }
    return (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
    struct stat buffer;
    if (::stat(path, &buffer) != 0)
    {
        auto err = errno;
        if (err == ENOENT)
            return false;
        MOE_THROW(ApiException, "Stat on file \"{0}\" failed, errno={1}", path, err);
    }
    return S_ISREG(buffer.st_mode);
#endif
}

bool Pal::IsDirectoryExists(const char* path)
{
#if defined(MOE_WINDOWS)
    wstring wpath;
    Encoding::Convert<Encoding::Utf8, Encoding::Utf16, char, wchar_t>(wpath, ArrayView<char>(path, strlen(path)),
        Encoding::DefaultUnicodeFallbackHandler);

    auto attr = ::GetFileAttributesW(wpath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        auto err = ::GetLastError();
        if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
            return false;
        MOE_THROW(ApiException, "Stat on file \"{0}\" failed, lastError={1}", path, err);
    }
    return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    struct stat buffer;
    if (::stat(path, &buffer) != 0)
    {
        auto err = errno;
        if (err == ENOENT)
            return false;
        MOE_THROW(ApiException, "Stat on file \"{0}\" failed, errno={1}", path, err);
    }
    return S_ISDIR(buffer.st_mode);
#endif
}

uint64_t Pal::GetFileSize(FILE* f)
{
    if (f == nullptr)
        MOE_THROW(BadArgumentException, "Must specific file");

#if defined(MOE_WINDOWS)
    auto fd = ::_fileno(f);
    auto ret = ::_filelengthi64(fd);
    if (ret == -1ll)
        MOE_THROW(ApiException, "Stat on file failed, errno={0}", errno);
    return static_cast<uint64_t>(ret);
#else
    auto fd = fileno(f);
    struct stat64 st;
    if (::fstat64(fd, &st) != 0)
        MOE_THROW(ApiException, "Stat on file failed, errno={0}", errno);
    return static_cast<uint64_t>(st.st_size);
#endif
}

uint64_t Pal::GetFileSize(const char* path)
{
#if defined(MOE_WINDOWS)
    wstring wpath;
    Encoding::Convert<Encoding::Utf8, Encoding::Utf16, char, wchar_t>(wpath, ArrayView<char>(path, strlen(path)),
        Encoding::DefaultUnicodeFallbackHandler);

    struct _stat64 st;
    if (::_wstat64(wpath.c_str(), &st) != 0)
        MOE_THROW(ApiException, "Stat on file \"{0}\" failed, errno={1}", path, errno);
    return static_cast<uint64_t>(st.st_size);
#else
    struct stat64 st;
    if (::stat64(path, &st) != 0)
        MOE_THROW(ApiException, "Stat on file \"{0}\" failed, errno={1}", path, errno);
    return static_cast<uint64_t>(st.st_size);
#endif
}

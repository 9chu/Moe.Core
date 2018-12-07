/**
 * @file
 * @author chu
 * @date 2018/7/28
 */
#pragma once
#include <ctime>
#include <cstring>
#include <utility>

#include "Utils.hpp"

/**
 * @brief 平台宏
 */
#if defined(__linux__)
    #define MOE_LINUX
    #define MOE_POSIX
    #if defined(__ANDROID__)
        #define MOE_ANDROID
        #define MOE_PLATFORM "linux_android"
    #else
        #define MOE_PLATFORM "linux"
    #endif
#elif defined(_WIN32)
    #define MOE_WINDOWS
    #if defined(_WIN64)
        #if defined(__MINGW64__)
            #define MOE_MINGW
            #define MOE_PLATFORM "win64_mingw"
        #else
            #define MOE_PLATFORM "win64"
        #endif
    #elif defined(_WIN32)
        #if defined(__MINGW32__)
            #define MOE_MINGW
            #define MOE_PLATFORM "win_mingw"
        #else
            #define MOE_PLATFORM "win"
        #endif
    #else
        #error "Unknown Windows platform"
    #endif
#elif defined(__APPLE__)
    #define MOE_POSIX
    #define MOE_APPLE
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
        #define MOE_IOS
    #elif TARGET_OS_IPHONE
        #define MOE_IOS
    #elif TARGET_OS_MAC
        #define MOE_MACOS
    #else
        #error "Unknown Apple platform"
    #endif
#elif defined(__FreeBSD__)
    #define MOE_BSD
    #define MOE_POSIX
    #define MOE_PLATFORM "bsd_freebsd"
#elif defined(__NetBSD__)
    #define MOE_BSD
    #define MOE_POSIX
    #define MOE_PLATFORM "bsd_netbsd"
#elif defined(__OpenBSD__)
    #define MOE_BSD
    #define MOE_POSIX
    #define MOE_PLATFORM "bsd_openbsd"
#elif defined(__EMSCRIPTEN__)
    #define MOE_EMSCRIPTEN
    #define MOE_POSIX
    #define MOE_PLATFORM "emscripten"
#else
    #error "Unknown platform"
#endif

namespace moe
{
    /**
     * @brief 平台无关抽象层
     */
    namespace Pal
    {
        //////////////////////////////////////// <editor-fold desc="时间日期函数">

        /**
         * @brief 获取实时时钟
         * @return 实时钟值，精确到毫秒，从1970/1/1开始计算。若发生异常，返回0。
         */
        uint64_t GetRealTimeClock()noexcept;

        /**
         * @brief 获取高精度单调时间时钟
         * @return 返回值为<毫秒时间,纳秒时间>。即时钟时间=毫秒时间+纳秒时间。若发生异常，返回0。
         */
        std::pair<uint64_t, uint64_t> GetMonotonicClock()noexcept;

        /**
         * @brief 获取当前时区偏移量（相对于UTC时间的）
         * @return 时区偏移量，返回单位为毫秒。若发生异常，返回0。
         *
         * 该方法将会获取时区偏移值。
         * 注意到，Time::GetTimeZoneOffset最终会调用这一方法，不同之处在于该方法不会缓存取值。
         *
         * 举例：
         *   北京时间 UTC+8，返回+28800000
         */
        int64_t GetTimeZoneOffset()noexcept;

        /**
         * @brief gmtime的跨平台线程安全版本
         * @param[out] out 日历时间
         * @param clock 时钟时间
         */
        void TimestampToDateTime(::tm& out, ::time_t clock)noexcept;

        /**
         * @brief timegm的跨平台线程安全版本
         * @param tm 日历时间
         * @return UNIX时间戳
         */
        time_t DateTimeToTimestamp(const ::tm& date)noexcept;

        //////////////////////////////////////// </editor-fold>
        //////////////////////////////////////// <editor-fold desc="IPC支持">

        /**
         * @brief 共享内存
         */
        class SharedMemory :
            public NonCopyable
        {
            struct Header
            {
                char Magic[4];
                char Padding[4];
                uint64_t Size;
                char Data[1];
            };

        public:
#ifdef MOE_WINDOWS
            using PlatformSpecificNameType = std::u16string;
#else
            using PlatformSpecificNameType = std::string;
#endif

            /**
             * @brief 共享内存挂载模式
             */
            enum class AttachMode
            {
                AttachOnly,
                CreateOnly,
                CreateOrAttach,
            };

        public:
            SharedMemory()noexcept;

            /**
             * @brief 构造共享内存
             * @param name 共享内存名称
             * @param sz 大小
             * @param mode 创建模式
             *
             * 方法会创建特定格式的共享内存，这意味着会校验共享内存的大小，当大小不匹配时会抛出异常。
             *
             * 注意到：
             *  - Windows平台下当所有进程不再引用某个共享内存时，该共享内存会被自动销毁。
             */
            SharedMemory(const char* name, size_t sz, AttachMode mode=AttachMode::CreateOrAttach);

            SharedMemory(SharedMemory&& org)noexcept;
            ~SharedMemory();

            operator bool()const noexcept;
            SharedMemory& operator=(SharedMemory&& rhs)noexcept;

        public:
            /**
             * @brief 获取名称
             */
            const std::string& GetName()const noexcept { return m_stName; }

            /**
             * @brief 获取平台特定名称
             */
            const PlatformSpecificNameType& GetPlatformSpecificName()const noexcept { return m_stPlatformName; }

            /**
             * @brief 获取大小
             */
            size_t GetSize()const noexcept { return m_uSize - sizeof(Header); }

            /**
             * @breif 获取映射的内存地址
             */
            const void* GetPointer()const noexcept
            {
                if (!m_pMappingData)
                    return nullptr;
                return m_pMappingData->Data;
            }
            void* GetPointer()noexcept
            {
                if (!m_pMappingData)
                    return nullptr;
                return m_pMappingData->Data;
            }

            /**
             * @brief 是否是首次创建
             */
            bool IsCreateMode()const noexcept { return m_bCreateMode; }

            /**
             * @brief 获取或设置是否自动释放
             *
             * 若自动释放则在析构时自动销毁共享内存。
             * Windows下无意义。
             * 默认不自动释放。
             */
            bool IsAutoFree()const noexcept { return m_bAutoFree; }
            void SetAutoFree(bool free)noexcept { m_bAutoFree = free; }

            /**
             * @brief 释放内存
             */
            void Free()noexcept;

        private:
            std::string m_stName;
            PlatformSpecificNameType m_stPlatformName;

#ifdef MOE_WINDOWS
            void* m_hHandle = nullptr;
#else
            int m_iFd = -1;
#endif

            size_t m_uSize = 0;
            Header* m_pMappingData = nullptr;
            bool m_bCreateMode = false;
            bool m_bAutoFree = false;
        };

        //////////////////////////////////////// </editor-fold>
        //////////////////////////////////////// <editor-fold desc="多线程相关">

        /**
         * @brief 短暂暂停
         */
        void Pause()noexcept;

        /**
         * @brief 交出控制权
         *
         * 相比Pause()具备更长的暂停时间。
         */
        void FastSleep()noexcept;

        /**
         * @brief 获取当前线程的线程ID
         */
        uint64_t GetCurrentThreadId()noexcept;

        /**
         * @brief 获取当前进程的进程ID
         */
        uint64_t GetCurrentProcessId()noexcept;

        //////////////////////////////////////// </editor-fold>
        //////////////////////////////////////// <editor-fold desc="终端">

        /**
         * @brief 检查终端是否支持着色
         */
        bool IsColorTerminal()noexcept;

        /**
         * @brief 检查文件描述符是否在终端中
         * @param file 文件描述符
         * @return 是否为TTY
         */
        bool IsInTerminal(FILE* file)noexcept;

        //////////////////////////////////////// </editor-fold>
        //////////////////////////////////////// <editor-fold desc="I/O">

        /**
         * @brief 打开文件
         * @exception ApiException 当发生平台相关错误时抛出
         * @param path 文件路径
         * @param mode 打开模式
         * @return 文件描述符
         *
         * 在Windows下，将以UTF8来编码文件路径。
         */
        FILE* OpenFile(const char* path, const char* mode="r");

        /**
         * @brief 删除文件
         * @exception ApiException 当发生平台相关错误时抛出
         * @param path 文件路径
         *
         * 在Windows下，将以UTF8来编码文件路径。
         */
        void RemoveFile(const char* path);

        /**
         * @brief 重命名文件（移动）
         * @exception ApiException 当发生平台相关错误时抛出
         * @param dest 目的路径
         * @param src 源路径
         *
         * 在Windows下，将以UTF8来编码文件路径。
         */
        void RenameFile(const char* dest, const char* src);

        /**
         * @brief 检查文件是否存在
         * @exception ApiException 当发生平台相关错误时抛出
         * @param path 路径
         *
         * 在Windows下，将以UTF8来编码文件路径。
         */
        bool IsFileExists(const char* path);

        /**
         * @brief 判断目录是否存在
         * @exception ApiException 当发生平台相关错误时抛出
         * @param path 路径
         *
         * 在Windows下，将以UTF8来编码文件路径。
         */
        bool IsDirectoryExists(const char* path);

        /**
         * @brief 获取文件大小
         * @exception ApiException 当发生平台相关错误时抛出
         * @param f 文件描述符
         * @return 文件大小（字节）
         */
        uint64_t GetFileSize(FILE* f);
        uint64_t GetFileSize(const char* path);

        //////////////////////////////////////// </editor-fold>
    }
}

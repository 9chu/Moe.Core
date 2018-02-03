/**
 * @file
 * @date 2017/5/29
 */
#pragma once
#include <mutex>
#include <thread>
#include <atomic>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <condition_variable>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include "Utils.hpp"

namespace moe
{
    /**
     * @brief 多线程相关支持
     *
     * 跨平台的多线程相关支持方法。
     */
    namespace Threading
    {
        using WaitingDuration = std::chrono::milliseconds;

        /**
         * @brief 瞬时休眠器
         *
         * 用于进行短时间的快速睡眠操作以交出CPU控制权。
         */
        class Sleeper
        {
            static const uint32_t kMaxActiveSpin = 4000;

        public:
            static inline void AsmVolatilePause()
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
                #error "Unknown platform"
#endif
            }

        public:
            Sleeper() = default;

        public:
            void Wait();

        private:
            uint32_t m_uSpinCount = 0;
        };

        /**
         * @brief 自旋锁
         */
        class SpinLock :
            public NonCopyable
        {
            enum class State
            {
                Free,
                Locked,
            };

        public:
            SpinLock()
            {
                m_stLock.store(static_cast<uint8_t>(State::Free));
            }

            bool TryLock()
            {
                uint8_t compare = static_cast<uint8_t>(State::Free);
                uint8_t val = static_cast<uint8_t>(State::Locked);

                return std::atomic_compare_exchange_strong_explicit(&m_stLock, &compare, val, std::memory_order_acquire,
                    std::memory_order_relaxed);
            }

            void Lock()
            {
                Sleeper sleeper;
                do
                {
                    while (m_stLock.load() != static_cast<uint8_t>(State::Free))
                        sleeper.Wait();
                } while (!TryLock());
                assert(m_stLock.load() == static_cast<uint8_t>(State::Locked));
            }

            void Unlock()
            {
                assert(m_stLock.load() == static_cast<uint8_t>(State::Locked));
                m_stLock.store(static_cast<uint8_t>(State::Free), std::memory_order_release);
            }

        private:
            std::atomic<uint8_t> m_stLock;
        };

        /**
         * @brief 事件
         *
         * 基于锁和条件变量实现的事件。
         */
        class Event :
            public NonCopyable
        {
        public:
            /**
             * @brief Event构造函数
             * @param set 初始化时是否设置了事件
             * @param autoReset 是否自动重置事件
             *
             * 对于autoReset行为，当有一个等待的线程被激活，它将自动把已设置的事件重置。
             * 这意味着同一时刻一个Set只能触发一个等待线程。
             */
            Event(bool set = false, bool autoReset = false);

            Event(Event&&)noexcept = delete;
            ~Event() = default;

            Event& operator=(Event &&)noexcept = delete;

        public:
            /**
             * @brief 等待事件触发
             */
            void Wait();

            /**
             * @brief 等待事件触发或者超时
             * @param duration 等待时间
             * @return 事件是否完成
             */
            bool WaitFor(WaitingDuration duration);

            /**
             * @brief 触发事件
             */
            void Set();

            /**
             * @brief 重置事件
             */
            void Reset();

        private:
            std::mutex m_stLock;
            std::condition_variable m_stCondVar;

            bool m_bEventSet;
            bool m_bAutoReset;
        };

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
             * 默认自动释放。
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
            bool m_bAutoFree = true;
        };
    }
}

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

#include "Pal.hpp"
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
            Sleeper() = default;

        public:
            void Wait()
            {
                if (m_uSpinCount < kMaxActiveSpin)
                {
                    ++m_uSpinCount;
                    Pal::Pause();
                }
                else
                    Pal::FastSleep();
            }

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
    }
}

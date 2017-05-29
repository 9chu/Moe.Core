/**
 * @file
 * @date 2017/5/29
 */
#pragma once
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

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

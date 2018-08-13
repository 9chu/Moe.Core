/**
 * @file
 * @date 2017/6/16
 */
#pragma once
#include <cassert>
#include <cstdlib>
#include <cstddef>
#include <stdexcept>
#include <memory>
#include <functional>

#include "Utils.hpp"
#include "Exception.hpp"

namespace moe
{
    namespace details
    {
        template <typename T>
        struct Finalizer
        {
            void operator()(T* p)noexcept
            {
                p->~T();
            }
        };

        template <>
        struct Finalizer<void>
        {
            void operator()(void* p)noexcept
            {
                MOE_UNUSED(p);
            }
        };
    }

    /**
     * @brief 基于定长对象的缓存分配器
     *
     * 简单的FreeList，单线程下使用，适用于小对象的频繁分配释放。
     * 根据Benchmark的结果，无论如何，在Windows/Linux下，使用ObjectPool性能总是会好于直接的malloc/free。
     * 然而OSX下反而不如直接malloc/free。原因不明（cache miss?）。
     */
    class ObjectPool :
        public NonCopyable
    {
    public:
        static const unsigned kSmallSizeThreshold = 4096;  // 4K
        static const unsigned kSmallSizeBlockSize = 32;
        static const unsigned kSmallSizeBlocks = kSmallSizeThreshold / kSmallSizeBlockSize;
        static const unsigned kLargeSizeThreshold = 128 * 1024;  // 128M
        static const unsigned kLargeSizeBlockSize = 256;
        static const unsigned kLargeSizeBlocks = (kLargeSizeThreshold - kSmallSizeThreshold) / kLargeSizeBlockSize;
        static const unsigned kTotalBlocks = 1 + kSmallSizeBlocks + kLargeSizeBlocks;

        template <typename T>
        struct Deleter
        {
            void operator()(T* p)const noexcept
            {
                details::Finalizer<T>()(p);
                Free(p);
            }
        };

        struct AllocContext
        {
            const char* Filename = nullptr;
            unsigned Line = 0u;

            AllocContext() = default;
            AllocContext(const char* file, unsigned line)noexcept
                : Filename(file), Line(line) {}
        };

        /**
         * @brief 通过指针获取对应的对象池
         */
        static ObjectPool* GetPoolFromPointer(void* p)noexcept;

        /**
         * @brief 释放对象
         */
        static void Free(void* p)noexcept;

    private:
        enum class NodeStatus : uint32_t
        {
            Free,
            Used,
        };

        struct Bucket;

        struct Node
        {
            struct {
                NodeStatus Status = NodeStatus::Free;  // 节点状态
                Bucket* Parent = nullptr;  // 父对象，nullptr表示从系统中分配的，直接对应malloc/free。
#ifndef NDEBUG
                Node* Prev = nullptr;  // 关联链中的上一个缓冲区
                Node* Next = nullptr;  // 关联链中的下一个缓冲区
                AllocContext Context;  // 分配上下文
#else
                Node* Next = nullptr;
#endif
            } Header;

            // 注意：OSX系统是16字节对齐的，这里必须做处理
            alignas(16) uint8_t Data[1];  // 缓冲区

#ifndef NDEBUG
            void Attach(Node* node)noexcept;  // 将节点插入到node后
            void Detach()noexcept;  // 将自己从链表脱离
#else
            void Attach(Node* node)noexcept;
            void Detach(Node* prev)noexcept;
#endif
        };

        struct Bucket
        {
            ObjectPool* Pool = nullptr;  // 父对象
            size_t NodeSize = 0;  // 单个节点的大小
            size_t AllocatedCount = 0;  // 总共分配的大小
            size_t FreeCount = 0;  // 空闲的大小
#ifndef NDEBUG
            Node UseList;  // 正在使用的内存块
#endif
            Node FreeList;  // 空闲的内存块
        };

        /**
         * @brief 内存泄漏回调指针
         * @pre 方法不能抛出异常
         *
         * 调试版本参数：（内存地址，分配块大小，分配时的上下文信息（若有））
         * 非调试版本参数：（分配块大小，内存泄漏个数）
         */
#ifndef NDEBUG
        using MemLeakReportCallback = std::function<void(void*, size_t, const AllocContext&)>;
#else
        using MemLeakReportCallback = std::function<void(size_t, size_t)>;
#endif

    public:
        ObjectPool();
        ~ObjectPool();

    public:
        /**
         * @brief 获取总共分配的数量（字节）
         */
        size_t GetAllocatedSize()const noexcept;

        /**
         * @brief 获取空闲的数量（字节）
         */
        size_t GetFreeSize()const noexcept;

        /**
         * @brief 获取使用中的数量（字节）
         */
        size_t GetUsedSize()const noexcept { return GetAllocatedSize() - GetFreeSize(); }

        /**
         * @brief 获取内存泄漏报告对象
         */
        const MemLeakReportCallback& GetLeakReporter()const noexcept { return m_pLeakReporter; }

        /**
         * @brief 设置内存泄漏报告对象
         * @param cb 回调
         */
        void SetLeakReporter(const MemLeakReportCallback& cb)noexcept { m_pLeakReporter = cb; }
        void SetLeakReporter(MemLeakReportCallback&& cb)noexcept { m_pLeakReporter = std::move(cb); }

        /**
         * @brief 从每个池回收1/factor个空闲对象
         * @param factor 系数，不小于1
         * @param maxFree 最大回收数量，0表示不限制
         * @return 回收掉的内存量（字节）
         *
         * 当需要回收所有空闲对象时，设置 factor = 1。
         * 当 maxFree 不为0时，当回收超过该数量的内存后退出。
         */
        size_t CollectGarbage(unsigned factor=1, size_t maxFree=0)noexcept;

        /**
         * @brief 分配内存
         * @param sz 大小
         * @param context 上下文，用于调试。仅调试版本有效。
         * @return 指针
         */
#ifndef NDEBUG
        std::unique_ptr<void, Deleter<void>> Alloc(size_t sz, const AllocContext& context=EmptyRefOf<AllocContext>())
#else
        std::unique_ptr<void, Deleter<void>> Alloc(size_t sz)
#endif
        {
            std::unique_ptr<void, Deleter<void>> ret;
#ifndef NDEBUG
            ret.reset(InternalAlloc(sz, context));
#else
            ret.reset(InternalAlloc(sz));
#endif
            return ret;
        }

#ifndef NDEBUG
        std::unique_ptr<void, Deleter<void>>& Realloc(std::unique_ptr<void, Deleter<void>>& p, size_t sz,
            const AllocContext& context=EmptyRefOf<AllocContext>())
#else
        std::unique_ptr<void, Deleter<void>>& Realloc(std::unique_ptr<void, Deleter<void>>& p, size_t sz)
#endif
        {
#ifndef NDEBUG
            auto ret = InternalRealloc(p.get(), sz, context);
#else
            auto ret = InternalRealloc(p.get(), sz);
#endif
            p.release();
            p.reset(ret);
            return p;
        }

    private:
#ifndef NDEBUG
        void* InternalAlloc(size_t sz, const AllocContext& context);
        void* InternalRealloc(void* p, size_t sz, const AllocContext& context);
#else
        void* InternalAlloc(size_t sz);
        void* InternalRealloc(void* p, size_t sz);
#endif
        void InternalFree(Node* p)noexcept;

    private:
        Bucket m_stBuckets[kTotalBlocks];
        MemLeakReportCallback m_pLeakReporter;
    };

    template <typename T>
    using UniquePooledObject = std::unique_ptr<T, ObjectPool::Deleter<T>>;
}

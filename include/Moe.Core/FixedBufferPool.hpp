/**
 * @file
 * @date 2017/6/16
 */
#pragma once
#include <cassert>
#include <cstdlib>
#include <cstddef>
#include <stdexcept>

#include "Utils.hpp"
#include "Logging.hpp"
#include "Exception.hpp"

namespace moe
{
    /**
     * @brief 定长缓冲区池
     *
     * 用于分配并缓存若干大小的缓冲区。
     * 定长缓冲区池为：
     *  - 0 <= sz <= 512
     *  - 512 < sz <= 4096
     *  - 4096 < sz <= 32768
     *  - 32768 < sz <= 262144
     *  - 262144 < sz <= 2097152
     *  - 2097152 < sz <= 16777216
     *
     *  大于16MB将拒绝分配。
     */
    class FixedBufferPool :
        public NonCopyable
    {
        /**
         * @brief 定长缓冲区
         */
        struct FixedBuffer
        {
            FixedBuffer* Prev = nullptr;  // 关联链中的上一个缓冲区
            FixedBuffer* Next = nullptr;  // 关联链中的下一个缓冲区
            bool Free = false;  // 是否空闲
            size_t Size = 0;  // 缓冲区的大小
            uint8_t Data[1];  // 缓冲区
        };

        /**
         * @brief 定长缓冲区管理器
         * @tparam Size 缓冲区大小
         * @pre Size >= 16
         */
        template <size_t Size>
        class FixedBufferManager :
            public NonCopyable
        {
            static_assert(Size >= 16, "Size must be big enough");

        public:
            ~FixedBufferManager()
            {
                if (m_uCount != m_uFreeCount)
                {
                    /// FIXME: 针对BufferManager的Singleton对象，如果走到这个分支需要依赖Logger的析构顺序
                    /// FIXME: 通过在FixedBufferManager的构造中依赖Logger可以约束析构顺序
                    /// FIXME: 但是需要修改Logger成Logger::GetInstance()的方式，暂时不予处理

                    MOE_FATAL("Memory leak detected, block size: {0}, free: {1}, allocated: {2}", Size, m_uFreeCount,
                        m_uCount);

                    // 扫描被使用的节点
                    FixedBuffer* node = m_stBuffers.Next;
                    while (node)
                    {
                        if (!node->Free)
                        {
                            MOE_FATAL("Leaked block memory: {0}, data: {1,2[0]:H} {2,2[0]:H} {3,2[0]:H} {4,2[0]:H} "
                                "{5,2[0]:H} {6,2[0]:H} {7,2[0]:H} {8,2[0]:H} {9,2[0]:H} {10,2[0]:H} {11,2[0]:H} "
                                "{12,2[0]:H} {13,2[0]:H} {14,2[0]:H} {15,2[0]:H} {16,2[0]:H}", node->Data,
                                node->Data[0], node->Data[1], node->Data[2], node->Data[3], node->Data[4],
                                node->Data[5], node->Data[6], node->Data[7], node->Data[8], node->Data[9],
                                node->Data[10], node->Data[11], node->Data[12], node->Data[13], node->Data[14],
                                node->Data[15]);
                        }

                        node = node->Next;
                    }

                    // 防止进一步错误，直接终止程序
                    std::terminate();
                }

                CollectGarbage();
            }

        public:
            /**
             * @brief 获取单个分配单元的大小
             */
            constexpr size_t GetBlockSize()const noexcept { return Size; }

            /**
             * @brief 获取缓冲区个数
             */
            size_t GetTotalBufferCount()const noexcept { return m_uCount; }

            /**
             * @brief 获取空闲缓冲区个数
             */
            size_t GetFreeBufferCount()const noexcept { return m_uFreeCount; }

            /**
             * @brief 分配一个缓冲区
             */
            FixedBuffer* Alloc()
            {
                FixedBuffer* freeNode = nullptr;

                if (m_uFreeCount > 0)
                {
                    freeNode = m_stFreeBuffers.Next;
                    assert(freeNode);

                    m_stFreeBuffers.Next = freeNode->Next;
                    if (m_stFreeBuffers.Next)
                        m_stFreeBuffers.Next->Prev = &m_stFreeBuffers;
                    --m_uFreeCount;
                }
                else
                {
                    freeNode = static_cast<FixedBuffer*>(::malloc(sizeof(FixedBuffer) + Size));
                    if (!freeNode)
                        throw std::bad_alloc();

                    freeNode->Size = Size;
                    ++m_uCount;
                }

                freeNode->Prev = &m_stBuffers;
                freeNode->Next = m_stBuffers.Next;
                freeNode->Free = false;

                if (m_stBuffers.Next)
                    m_stBuffers.Next->Prev = freeNode;
                m_stBuffers.Next = freeNode;

                return freeNode;
            }

            /**
             * @brief 释放一个缓冲区
             * @param buffer 缓冲区对象
             */
            void Free(FixedBuffer* buffer)noexcept
            {
                assert(buffer);
                assert(buffer->Prev);
                assert(!buffer->Free);
                assert(buffer->Size == Size);

                buffer->Prev->Next = buffer->Next;
                if (buffer->Next)
                    buffer->Next->Prev = buffer->Prev;

                buffer->Prev = &m_stFreeBuffers;
                buffer->Next = m_stFreeBuffers.Next;

                if (m_stFreeBuffers.Next)
                    m_stFreeBuffers.Next->Prev = buffer;
                m_stFreeBuffers.Next = buffer;

                buffer->Free = true;

                ++m_uFreeCount;
                assert(m_uFreeCount <= m_uCount);
            }

            /**
             * @brief 释放所有空闲的缓冲区
             */
            void CollectGarbage()noexcept
            {
                FixedBuffer* node = m_stFreeBuffers.Next;
                while (node)
                {
                    FixedBuffer* next = node->Next;

                    ::free(node);
                    --m_uCount;
                    --m_uFreeCount;

                    node = next;
                }
                m_stFreeBuffers.Next = nullptr;
                assert(m_uFreeCount == 0);
            }

        private:
            size_t m_uCount = 0;  // 节点总数
            size_t m_uFreeCount = 0;  // 空闲的数量

            FixedBuffer m_stBuffers;  // 正在使用的节点
            FixedBuffer m_stFreeBuffers;  // 空闲节点
        };

    public:
        static const size_t kMaxAllocSize = 16777216u;

    public:
        FixedBufferPool() = default;

    public:
        /**
         * @brief 获取总分配的内存大小
         */
        size_t GetTotalBufferSize()noexcept
        {
            size_t total = 0;
            total += m_stBuffer512.GetBlockSize() * m_stBuffer512.GetTotalBufferCount();
            total += m_stBuffer4096.GetBlockSize() * m_stBuffer4096.GetTotalBufferCount();
            total += m_stBuffer32768.GetBlockSize() * m_stBuffer32768.GetTotalBufferCount();
            total += m_stBuffer262144.GetBlockSize() * m_stBuffer262144.GetTotalBufferCount();
            total += m_stBuffer2097152.GetBlockSize() * m_stBuffer2097152.GetTotalBufferCount();
            total += m_stBuffer16777216.GetBlockSize() * m_stBuffer16777216.GetTotalBufferCount();
            return total;
        }

        /**
         * @brief 获取空闲的内存大小
         */
        size_t GetTotalFreeSize()noexcept
        {
            size_t total = 0;
            total += m_stBuffer512.GetBlockSize() * m_stBuffer512.GetFreeBufferCount();
            total += m_stBuffer4096.GetBlockSize() * m_stBuffer4096.GetFreeBufferCount();
            total += m_stBuffer32768.GetBlockSize() * m_stBuffer32768.GetFreeBufferCount();
            total += m_stBuffer262144.GetBlockSize() * m_stBuffer262144.GetFreeBufferCount();
            total += m_stBuffer2097152.GetBlockSize() * m_stBuffer2097152.GetFreeBufferCount();
            total += m_stBuffer16777216.GetBlockSize() * m_stBuffer16777216.GetFreeBufferCount();
            return total;
        }

        /**
         * @brief 获取使用中的内存大小
         */
        size_t GetTotalUsedSize()noexcept
        {
            return GetTotalBufferSize() - GetTotalFreeSize();
        }

        /**
         * @brief 分配缓冲区
         * @param sz 缓冲区大小
         * @return 分配的缓冲区
         */
        void* Alloc(size_t sz)
        {
            FixedBuffer* buffer = nullptr;

            if (sz <= 512)
                buffer = m_stBuffer512.Alloc();
            else if (sz <= 4096)
                buffer = m_stBuffer4096.Alloc();
            else if (sz <= 32768)
                buffer = m_stBuffer32768.Alloc();
            else if (sz <= 262144)
                buffer = m_stBuffer262144.Alloc();
            else if (sz <= 2097152)
                buffer = m_stBuffer2097152.Alloc();
            else if (sz <= 16777216)
                buffer = m_stBuffer16777216.Alloc();
            else
                MOE_THROW(BadArgumentException, "Required buffer size {0} is too big", sz);

            return static_cast<void*>(buffer->Data);
        }

        /**
         * @brief 释放缓冲区
         * @param buffer 缓冲区指针
         */
        void Free(void* p)
        {
            FixedBuffer* buffer =
                reinterpret_cast<FixedBuffer*>(static_cast<uint8_t*>(p) - offsetof(FixedBuffer, Data));
            assert(!buffer->Free);

            size_t sz = buffer->Size;
            if (sz == 512)
                m_stBuffer512.Free(buffer);
            else if (sz == 4096)
                m_stBuffer4096.Free(buffer);
            else if (sz == 32768)
                m_stBuffer32768.Free(buffer);
            else if (sz == 262144)
                m_stBuffer262144.Free(buffer);
            else if (sz == 2097152)
                m_stBuffer2097152.Free(buffer);
            else if (sz == 16777216)
                m_stBuffer16777216.Free(buffer);
            else
                MOE_THROW(BadArgumentException, "Invalid buffer size {0}", sz);
        }

        /**
         * @brief 回收并释放所有空闲内存
         */
        void CollectGarbage()noexcept
        {
            m_stBuffer512.CollectGarbage();
            m_stBuffer4096.CollectGarbage();
            m_stBuffer32768.CollectGarbage();
            m_stBuffer262144.CollectGarbage();
            m_stBuffer2097152.CollectGarbage();
            m_stBuffer16777216.CollectGarbage();
        }

    private:
        FixedBufferManager<512> m_stBuffer512;  // 512
        FixedBufferManager<4096> m_stBuffer4096;  // 4K
        FixedBufferManager<32768> m_stBuffer32768;  // 32K
        FixedBufferManager<262144> m_stBuffer262144;  // 256K
        FixedBufferManager<2097152> m_stBuffer2097152;  // 2M
        FixedBufferManager<16777216> m_stBuffer16777216;  // 16M
    };
}

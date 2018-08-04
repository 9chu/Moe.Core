/**
 * @file
 * @author chu
 * @date 2018/8/2
 */
#include <Moe.Core/ObjectPool.hpp>

using namespace std;
using namespace moe;

//////////////////////////////////////////////////////////////////////////////// Node

#ifndef NDEBUG

void ObjectPool::Node::Attach(Node* node)noexcept
{
    assert(node);
    Header.Prev = node;
    Header.Next = node->Header.Next;
    node->Header.Next = this;
    if (Header.Next)
        Header.Next->Header.Prev = this;
}

void ObjectPool::Node::Detach()noexcept
{
    if (Header.Prev)
    {
        assert(Header.Prev->Header.Next == this);
        Header.Prev->Header.Next = Header.Next;
    }
    if (Header.Next)
    {
        assert(Header.Next->Header.Prev == this);
        Header.Next->Header.Prev = Header.Prev;
    }
    Header.Prev = Header.Next = nullptr;
}

#else

void ObjectPool::Node::Attach(Node* node)noexcept
{
    assert(node);
    Header.Next = node->Header.Next;
    node->Header.Next = this;
}

void ObjectPool::Node::Detach(Node* prev)noexcept
{
    assert(prev);
    assert(prev->Header.Next == this);
    prev->Header.Next = Header.Next;
    Header.Next = nullptr;
}

#endif

//////////////////////////////////////////////////////////////////////////////// ObjectPool

namespace
{
    size_t SizeToIndex(size_t size)noexcept
    {
        if (size <= ObjectPool::kSmallSizeThreshold)
            return (size + (ObjectPool::kSmallSizeBlockSize - 1)) / ObjectPool::kSmallSizeBlockSize;
        else if (size <= ObjectPool::kLargeSizeThreshold)
        {
            size -= ObjectPool::kSmallSizeThreshold;
            return (size + (ObjectPool::kLargeSizeBlockSize - 1)) / ObjectPool::kLargeSizeBlockSize +
                ObjectPool::kSmallSizeBlocks;
        }
        assert(false);
        return 0;
    }
}

void ObjectPool::Free(void* p)noexcept
{
    if (!p)
        return;

    Node* n = reinterpret_cast<Node*>(static_cast<uint8_t*>(p) - offsetof(Node, Data));
    assert(n->Header.Status == NodeStatus::Used);
    assert(n->Header.Parent);

    n->Header.Parent->Pool->InternalFree(n);
}

ObjectPool::ObjectPool()
{
    m_stBuckets[0].NodeSize = 0;
    for (unsigned i = 1; i <= kSmallSizeBlocks; ++i)
        m_stBuckets[i].NodeSize = i * kSmallSizeBlockSize;
    for (unsigned i = kSmallSizeBlocks + 1; i < kTotalBlocks; ++i)
        m_stBuckets[i].NodeSize = (i - kSmallSizeBlocks) * kLargeSizeBlockSize + kSmallSizeThreshold;
}

ObjectPool::~ObjectPool()
{
    CollectGarbage();  // 收集所有节点

    // 检查内存泄漏
    bool leak = false;
    for (unsigned i = 0; i < CountOf(m_stBuckets); ++i)
    {
        auto& bucket = m_stBuckets[i];

#ifndef NDEBUG
        auto p = bucket.UseList.Header.Next;
        if (p)
        {
            while (p && m_pLeakReporter)
            {
                m_pLeakReporter(p->Data, bucket.NodeSize, p->Header.Context);
                p = p->Header.Next;
            }
            leak = true;
        }
#else
        if (bucket.AllocatedCount)
        {
            if (m_pLeakReporter)
                m_pLeakReporter(bucket.NodeSize, bucket.AllocatedCount - bucket.FreeCount);
            leak = true;
        }
#endif
    }
    assert(!leak);
    MOE_UNUSED(leak);
}

size_t ObjectPool::GetAllocatedSize()const noexcept
{
    size_t ret = 0;
    for (unsigned i = 0; i < CountOf(m_stBuckets); ++i)
        ret += m_stBuckets[i].AllocatedCount * m_stBuckets[i].NodeSize;
    return ret;
}

size_t ObjectPool::GetFreeSize()const noexcept
{
    size_t ret = 0;
    for (unsigned i = 0; i < CountOf(m_stBuckets); ++i)
        ret += m_stBuckets[i].FreeCount * m_stBuckets[i].NodeSize;
    return ret;
}

size_t ObjectPool::CollectGarbage(unsigned factor, size_t maxFree)noexcept
{
    factor = max<unsigned>(factor, 1);

    size_t ret = 0;
    unsigned i = CountOf(m_stBuckets);
    while (i-- > 0)
    {
        auto& bucket = m_stBuckets[i];
        if (bucket.FreeCount == 0)
        {
            assert(bucket.FreeList.Header.Next == nullptr);
            continue;
        }

        size_t collects = bucket.FreeCount / factor;
        for (unsigned j = 0; j < collects; ++j)
        {
            auto obj = bucket.FreeList.Header.Next;
            assert(obj);
#ifndef NDEBUG
            obj->Detach();
#else
            obj->Detach(&bucket.FreeList);
#endif
            ::free(obj);

            --bucket.FreeCount;
            --bucket.AllocatedCount;
            ret += bucket.NodeSize;
            if (maxFree && ret >= maxFree)
                return ret;
        }
    }

    return ret;
}

#ifndef NDEBUG
void* ObjectPool::InternalAlloc(size_t sz, const AllocContext& context)
#else
void* ObjectPool::InternalAlloc(size_t sz)
#endif
{
    Node* ret = nullptr;

    sz = max<size_t>(sz, 1);
    if (sz > kLargeSizeThreshold)  // 直接从系统分配，并挂在大小为0的节点上
    {
        ret = reinterpret_cast<Node*>(::malloc(sizeof(Node::Header) + sz));
        if (!ret)
            throw bad_alloc();
        ret->Header.Status = NodeStatus::Used;
        ret->Header.Parent = &m_stBuckets[0];
#ifndef NDEBUG
        ret->Header.Context = context;
        ret->Attach(&m_stBuckets->UseList);
#else
        ret->Header.Next = nullptr;
#endif
        ++m_stBuckets[0].AllocatedCount;
        return static_cast<void*>(ret->Data);
    }

    // 获取对应的Bucket
    auto index = SizeToIndex(sz);
    assert(m_stBuckets[index].NodeSize >= sz);
    Bucket& bucket = m_stBuckets[index];
    if (bucket.FreeList.Header.Next)  // 如果有空闲节点，就分配
    {
        assert(bucket.FreeCount > 0);
        ret = bucket.FreeList.Header.Next;
#ifndef NDEBUG
        ret->Detach();
#else
        ret->Detach(&bucket.FreeList);
#endif
        --bucket.FreeCount;
    }
    else
    {
        ret = reinterpret_cast<Node*>(::malloc(sizeof(Node::Header) + bucket.NodeSize));
        if (!ret)
            throw bad_alloc();
        ret->Header.Parent = &bucket;
        ++bucket.AllocatedCount;
    }

    assert(ret);
    ret->Header.Status = NodeStatus::Used;
#ifndef NDEBUG
    ret->Header.Context = context;
    ret->Attach(&bucket.UseList);
#else
    ret->Header.Next = nullptr;
#endif
    return static_cast<void*>(ret->Data);
}

void ObjectPool::InternalFree(Node* p)noexcept
{
    assert(p);

    p->Header.Status = NodeStatus::Free;
#ifndef NDEBUG
    p->Detach();
#endif

    Bucket& bucket = *p->Header.Parent;
    if (bucket.NodeSize == 0)  // 超大对象，直接释放
    {
        ::free(p);
        --bucket.AllocatedCount;
        return;
    }

    // 回收到FreeList
    p->Attach(&bucket.FreeList);
    ++bucket.FreeCount;
}

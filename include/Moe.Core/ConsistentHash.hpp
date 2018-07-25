/**
 * @file
 * @author chu
 * @date 2018/7/21
 */
#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include "Hasher.hpp"
#include "Exception.hpp"

namespace moe
{
    namespace details
    {
        template <typename T>
        struct ConsistentHashKeyWrapper;

        template <>
        struct ConsistentHashKeyWrapper<std::string>
        {
            BytesView operator()(const std::string& input)const noexcept
            {
                return StringToBytesView(input);
            }
        };

        template <typename Key, typename HashMethod, typename HashType,
            typename HashResult = typename HashMethod::ResultType>
        struct ConsistentHashMethodWrapper;

        template <typename Key, typename HashMethod, typename HashType>
        struct ConsistentHashMethodWrapper<Key, HashMethod, HashType, HashType>
        {
            HashType operator()(const Key& key)const noexcept
            {
                HashMethod h;
                h.Update(ConsistentHashKeyWrapper<Key>()(key));
                return h.Final();
            }

            HashType operator()(const Key& key, uint32_t index)const noexcept
            {
                HashMethod h;
                h.Update(ConsistentHashKeyWrapper<Key>()(key));
                h.Update(BytesView(reinterpret_cast<const uint8_t*>(&index), sizeof(index)));
                return h.Final();
            }
        };

        template <typename Key, typename HashMethod>
        struct ConsistentHashMethodWrapper<Key, HashMethod, uint32_t, uint64_t>
        {
            uint32_t operator()(const Key& key)const noexcept
            {
                HashMethod h;
                h.Update(ConsistentHashKeyWrapper<Key>()(key));
                auto ret = h.Final();
                return static_cast<uint32_t>((ret >> 32u) ^ (ret & 0xFFFFFFFFu));
            }

            uint32_t operator()(const Key& key, uint32_t index)const noexcept
            {
                HashMethod h;
                h.Update(ConsistentHashKeyWrapper<Key>()(key));
                h.Update(BytesView(reinterpret_cast<const uint8_t*>(&index), sizeof(index)));
                auto ret = h.Final();
                return static_cast<uint32_t>((ret >> 32u) ^ (ret & 0xFFFFFFFFu));
            }
        };
    }

    /**
     * @brief 一致性HASH
     * @tparam TValue 值类型
     * @tparam TKey 键类型
     * @tparam HashMethod HASH方法
     * @tparam VNodesPerNode 单个节点的虚拟节点个数
     *
     * 一致性HASH用于解决分布式系统中的热点问题，一定程度上可以用作负载均衡。
     * 算法保证了在新增节点和移除节点的时候只有少量的对应关系发生颠簸。
     */
    template <typename TValue, typename TKey = std::string, typename HashMethod = Hasher::Murmur3<0>,
        size_t VNodesPerNode = 16>
    class ConsistentHash
    {
    public:
        struct VirtualNode;

        using ValueType = TValue;
        using KeyType = TKey;
        using HashMethodType = HashMethod;
        using HashType = uint32_t;

        struct Node
        {
            KeyType Key {};
            ValueType Value {};
            VirtualNode* Header = nullptr;
        };

        struct VirtualNode
        {
            Node* Parent = nullptr;
            VirtualNode* Next = nullptr;
            uint32_t VirtualIndex = 0;
            HashType Hash = 0;
        };

        using NodePtr = std::unique_ptr<Node>;
        using VirtualNodePtr = std::unique_ptr<VirtualNode>;
        using NodesContainerType = std::unordered_map<KeyType, NodePtr>;
        using VirtualNodesContainerType = std::vector<VirtualNodePtr>;

        struct VirtualNodeComparer
        {
            template <typename T, typename P>
            bool operator()(const T& a, const P& b)const noexcept
            {
                return a->Hash < b->Hash;
            }

            template <typename T>
            bool operator()(HashType a, const T& b)const noexcept
            {
                return a < b->Hash;
            }

            template <typename T>
            bool operator()(const T& a, HashType b)const noexcept
            {
                return a->Hash < b;
            }
        };

        struct VirtualNodeDescriptor
        {
            const KeyType& Key;
            const ValueType& Value;
            uint32_t Index;
            HashType Hash;
        };

        class Iterator :
            public std::iterator<std::input_iterator_tag, VirtualNodeDescriptor>
        {
        public:
            Iterator()noexcept = default;
            Iterator(const Iterator&)noexcept = default;

            Iterator(typename VirtualNodesContainerType::const_iterator it)noexcept
                : m_stIterator(it)
            {
            }

            VirtualNodeDescriptor operator*()const noexcept
            {
                return VirtualNodeDescriptor {
                    m_stIterator->get()->Parent->Key,
                    m_stIterator->get()->Parent->Value,
                    m_stIterator->get()->VirtualIndex,
                    m_stIterator->get()->Hash
                };
            }

            Iterator& operator++()noexcept
            {
                ++m_stIterator;
                return *this;
            }

            Iterator operator++(int)noexcept
            {
                Iterator tmp(m_stIterator);
                ++m_stIterator;
                return tmp;
            }

            bool operator==(const Iterator& rhs)const noexcept
            {
                return m_stIterator == rhs.m_stIterator;
            }

            bool operator!=(const Iterator& rhs)const noexcept
            {
                return !(*this == rhs);
            }

        private:
            typename VirtualNodesContainerType::const_iterator m_stIterator;
        };

    public:
        const ValueType& operator[](const KeyType& key)const noexcept
        {
            assert(!m_stVirtualNodes.empty());

            details::ConsistentHashMethodWrapper<KeyType, HashMethod, HashType> hasher;
            auto hash = hasher.operator()(key);

            auto it = std::lower_bound(m_stVirtualNodes.begin(), m_stVirtualNodes.end(), hash, VirtualNodeComparer());
            if (it == m_stVirtualNodes.end())
                it = m_stVirtualNodes.begin();

            return (*it)->Parent->Value;
        }

        ValueType& operator[](const KeyType& key)noexcept
        {
            assert(!m_stVirtualNodes.empty());

            details::ConsistentHashMethodWrapper<KeyType, HashMethod, HashType> hasher;
            auto hash = hasher.operator()(key);

            auto it = std::lower_bound(m_stVirtualNodes.begin(), m_stVirtualNodes.end(), hash, VirtualNodeComparer());
            if (it == m_stVirtualNodes.end())
                it = m_stVirtualNodes.begin();

            return (*it)->Parent->Value;
        }

    public:
        /**
         * @brief 获取节点的数量
         */
        size_t GetNodeCount()const noexcept { return m_stNodes.size(); }

        /**
         * @brief 获取虚拟节点的数量
         */
        size_t GetVirtualNodeCount()const noexcept { return m_stVirtualNodes.size(); }

        /**
         * @brief 是否为空
         */
        bool IsEmpty()const noexcept { return m_stVirtualNodes.empty(); }

        /**
         * @brief 增加节点
         * @param key 节点键值
         * @param weight 权重
         *
         * 向一致性HASH容器中增加节点。
         * 当权重为1时将插入 VNodesPerNode 个节点。
         * 当权重为2时将插入 VNodesPerNode*2 个节点。
         * 以此类推。调用方需要自行计算权重和 weight 的对应关系。
         * 注意到新增节点会引发一定范围的颠簸。
         */
        void AddNode(const KeyType& key, const ValueType& value, uint32_t weight=1)
        {
            auto ret = m_stNodes.emplace(key, NodePtr {});
            if (!ret.second)
                MOE_THROW(ObjectExistsException, "Insertion node already exists");

            NodePtr& p = ret.first->second;
            p.reset(new Node());
            p->Key = key;
            p->Value = value;

            auto count = VNodesPerNode * weight;
            details::ConsistentHashMethodWrapper<KeyType, HashMethod, HashType> hasher;
            for (uint32_t i = 0; i < count; ++i)
            {
                VirtualNodePtr vp;
                vp.reset(new VirtualNode());
                vp->Parent = p.get();
                vp->VirtualIndex = i;
                vp->Hash = hasher(p->Key, i);

                if (p->Header)
                    vp->Next = p->Header;
                p->Header = vp.get();

                m_stVirtualNodes.emplace_back(std::move(vp));
            }

            // 排序所有节点
            std::sort(m_stVirtualNodes.begin(), m_stVirtualNodes.end(), VirtualNodeComparer());
        }

        void AddNode(KeyType&& key, ValueType&& value, uint32_t weight=1)
        {
            auto ret = m_stNodes.emplace(std::move(key), NodePtr {});
            if (!ret.second)
                MOE_THROW(ObjectExistsException, "Insertion node already exists");

            NodePtr& p = ret.first->second;
            p.reset(new Node());
            p->Key = ret.first->first;
            p->Value = std::move(value);

            auto count = VNodesPerNode * weight;
            details::ConsistentHashMethodWrapper<KeyType, HashMethod, HashType> hasher;
            for (uint32_t i = 0; i < count; ++i)
            {
                VirtualNodePtr vp;
                vp.reset(new VirtualNode());
                vp->Parent = p.get();
                vp->VirtualIndex = i;
                vp->Hash = hasher(p->Key, i);

                if (p->Header)
                    vp->Next = p->Header;
                p->Header = vp.get();

                m_stVirtualNodes.emplace_back(std::move(vp));
            }

            // 排序所有节点
            std::sort(m_stVirtualNodes.begin(), m_stVirtualNodes.end(), VirtualNodeComparer());
        }

        /**
         * @brief 移除一个节点
         * @param key 键值
         * @return 是否成功移除
         *
         * 当节点不存在时返回false。否则返回true。
         * 注意到删除节点会引发一定范围的颠簸。
         */
        bool RemoveNode(const KeyType& key)noexcept
        {
            auto it = m_stNodes.find(key);
            if (it == m_stNodes.end())
                return false;

            // 沿着链表删掉所有虚节点
            auto p = it->second->Header;
            while (p)
            {
                auto next = p->Next;
                auto range = std::equal_range(m_stVirtualNodes.begin(), m_stVirtualNodes.end(), p,
                    VirtualNodeComparer());

#ifndef NDEBUG
                bool found = false;
#endif
                for (auto jt = range.first; jt != range.second; ++jt)
                {
                    if ((*jt).get() == p)
                    {
#ifndef NDEBUG
                        found = true;
#endif
                        m_stVirtualNodes.erase(jt);
                        break;
                    }
                }
                assert(found);

                p = next;
            }

            m_stNodes.erase(it);
            return true;
        }

        /**
         * @brief 删除所有节点
         */
        void Clear()noexcept
        {
            m_stNodes.clear();
            m_stVirtualNodes.clear();
        }

        /**
         * @brief 获取首个虚拟节点
         */
        Iterator First()const noexcept
        {
            return Iterator(m_stVirtualNodes.begin());
        }

        /**
         * @brief 获取最后一个虚拟节点的后继
         */
        Iterator Last()const noexcept
        {
            return Iterator(m_stVirtualNodes.end());
        }

    private:
        NodesContainerType m_stNodes;
        VirtualNodesContainerType m_stVirtualNodes;
    };
}

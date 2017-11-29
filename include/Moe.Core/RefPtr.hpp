/**
 * @file
 * @date 2017/5/29
 * @see https://github.com/lhmouse/RefPtr
 */
#pragma once
#include <cassert>
#include <mutex>
#include <atomic>
#include <exception>
#include <memory>

#include "Utils.hpp"

namespace moe
{
    template <typename T, class D = std::default_delete<T>>
    class RefBase;

    template <typename T>
    class RefPtr;

    template <typename T>
    class RefWeakPtr;

    namespace details
    {
        class RefCountBase
        {
        protected:
            constexpr RefCountBase()noexcept
                : m_iRef(1)
            {
            }

            constexpr RefCountBase(const RefCountBase&)noexcept
                : m_iRef(1)
            {
            }

            RefCountBase &operator=(const RefCountBase&)noexcept
            {
                return *this;
            }

            ~RefCountBase()
            {
                if (m_iRef.load(std::memory_order_relaxed) > 1)
                {
                    assert(false);
                    std::terminate();
                }
            }

        public:
            bool IsUnique()const volatile noexcept
            {
                return m_iRef.load(std::memory_order_relaxed) == 1;
            }

            long GetRefCount()const volatile noexcept
            {
                return m_iRef.load(std::memory_order_relaxed);
            }

            bool TryAddRef()const volatile noexcept
            {
                assert(m_iRef.load(std::memory_order_relaxed) >= 0);

                auto old = m_iRef.load(std::memory_order_relaxed);
                do
                {
                    if (old == 0)
                        return false;
                } while(!m_iRef.compare_exchange_strong(old, old + 1, std::memory_order_relaxed));

                return true;
            }

            void AddRef()const volatile noexcept
            {
                assert(m_iRef.load(std::memory_order_relaxed) > 0);

                m_iRef.fetch_add(1, std::memory_order_relaxed);
            }

            bool DropRef()const volatile noexcept
            {
                assert(m_iRef.load(std::memory_order_relaxed) > 0);

                return m_iRef.fetch_sub(1, std::memory_order_relaxed) == 1;
            }

        private:
            mutable std::atomic<long> m_iRef;
        };

        template <typename R, typename S, typename = void>
        struct StaticCastOrDynamicCastHelper
        {
            constexpr R operator()(S& s)const
            {
                return dynamic_cast<R>(std::forward<S>(s));
            }
        };

        template <typename R, typename S>
        struct StaticCastOrDynamicCastHelper<R, S, decltype(static_cast<void>(static_cast<R>(std::declval<S>())))>
        {
            constexpr R operator()(S& s)const
            {
                return static_cast<R>(std::forward<S>(s));
            }
        };

        template<typename R, typename S>
        constexpr R StaticCastOrDynamicCast(S&& s)
        {
            return StaticCastOrDynamicCastHelper<R, S>()(s);
        }

        template <typename T>
        class WeakView :
            public RefCountBase,
            public NonCopyable
        {
        public:
            explicit constexpr WeakView(T* parent)noexcept
                : m_pParent(parent) {}

        public:
            bool IsExpired()const noexcept
            {
                const std::lock_guard<std::mutex> guard(m_stMutex);
                const auto p = m_pParent;
                if (!p)
                    return true;
                return p->RefCountBase::GetRefCount() == 0;
            }

            template <typename U>
            RefPtr<U> Lock()const noexcept
            {
                const std::lock_guard<std::mutex> guard(m_stMutex);
                const auto u = StaticCastOrDynamicCast<U*>(m_pParent);

                if (!u)
                    return nullptr;
                if (!u->RefCountBase::TryAddRef())
                    return nullptr;

                return RefPtr<U>(u);
            }

            void Unlink()noexcept
            {
                const std::lock_guard<std::mutex> guard(m_stMutex);
                m_pParent = nullptr;
            }

        private:
            mutable std::mutex m_stMutex;
            T* m_pParent;
        };
    }

    /**
     * @brief 引用计数基类
     *
     * 该类实现侵入式指针的基类，所有需要支持引用计数的对象都应当以该类为基类。
     *
     * 需要注意的是，该类没有使用虚析构，如果存在：
     *   class A : public RefBase;
     *   class B : public A;
     * 则 A::~A() 必须为 virtual 函数，否则将产生内存泄漏。
     */
    template <typename T, typename D>
    class RefBase :
        public details::RefCountBase, private D
    {
        template <typename>
        friend class RefPtr;

        template <typename>
        friend class RefWeakPtr;

    public:
        using Pointer = T*;
        using ElementType = T;
        using DeleterType = D;

    private:
        using WeakView = details::WeakView<typename std::remove_cv<T>::type>;

    protected:
        template <typename CvU, typename CvT>
        static RefPtr<CvU> ForkStrong(CvT* t)noexcept
        {
            const auto u = details::StaticCastOrDynamicCast<CvU*>(t);

            if (!u)
                return nullptr;

            u->RefCountBase::AddRef();
            return RefPtr<CvU>(u);
        }

        template <typename CvU, typename CvT>
        static RefWeakPtr<CvU> ForkWeak(CvT* t)
        {
            const auto u = details::StaticCastOrDynamicCast<CvU*>(t);

            if (!u)
                return nullptr;

            return RefWeakPtr<CvU>(u);
        }

    public:
        constexpr RefBase()noexcept
            : m_stView(nullptr) {}

        constexpr RefBase(const RefBase&)noexcept
            : m_stView(nullptr) {}

        RefBase& operator=(const RefBase&)noexcept
        {
            return *this;
        }

        ~RefBase()
        {
            const auto v = m_stView.load(std::memory_order_consume);

            if (v)
            {
                if (v->details::RefCountBase::DropRef())
                    delete v;
                else
                    v->Unlink();
            }
        }

    public:
        /**
         * @brief 获取删除器
         * @return 删除器对象
         */
        const DeleterType& GetDeleter()const noexcept { return *this; }
        DeleterType& GetDeleter()noexcept { return *this; }

        /**
         * @brief 检查对象是否被单个其他对象引用
         * @return 是否唯一
         */
        bool IsRefUnique()const volatile noexcept { return details::RefCountBase::IsUnique(); }

        /**
         * @brief 获取引用计数器个数
         * @return 引用计数
         */
        long GetRefCount()const volatile noexcept { return details::RefCountBase::GetRefCount(); }

        /**
         * @brief 获取弱引用计数个数
         * @return 弱引用计数
         */
        long GetWeakRefCount()const volatile noexcept
        {
            const auto v = GetWeakView();
            if (!v)
                return 0;
            return v->details::RefCountBase::GetRefCount() - 1;
        }

        /**
         * @brief 预分配弱引用观察者
         */
        void ReserveWeakRef()const volatile { RequireWeakView(); }

        template <typename U = T>
        RefPtr<const volatile U> RefFromThis()const volatile noexcept { return ForkStrong<const volatile U>(this); }

        template <typename U = T>
        RefPtr<const U> RefFromThis()const noexcept { return ForkStrong<const U>(this); }

        template <typename U = T>
        RefPtr<volatile U> RefFromThis()volatile noexcept { return ForkStrong<volatile U>(this); }

        template <typename U = T>
        RefPtr<U> RefFromThis()noexcept { return ForkStrong<U>(this); }

        template <typename U = T>
        RefWeakPtr<const volatile U> WeakRefFromThis()const volatile { return ForkWeak<const volatile U>(this); }

        template <typename U = T>
        RefWeakPtr<const U> WeakRefFromThis()const { return ForkWeak<const U>(this); }

        template <typename U = T>
        RefWeakPtr<volatile U> WeakRefFromThis()volatile { return ForkWeak<volatile U>(this); }

        template <typename U = T>
        RefWeakPtr<U> WeakRefFromThis() { return ForkWeak<U>(this); }

    private:
        WeakView* GetWeakView()const volatile noexcept
        {
            auto v = m_stView.load(std::memory_order_consume);
            return v;
        }

        WeakView* RequireWeakView()const volatile
        {
            auto v = m_stView.load(std::memory_order_consume);
            if (!v)
            {
                const auto t = details::StaticCastOrDynamicCast<const volatile T*>(this);

                if (!t)
                    throw std::bad_cast();

                const auto newV = new WeakView(const_cast<T*>(t));
                if (m_stView.compare_exchange_strong(v, newV, std::memory_order_release, std::memory_order_consume))
                    v = newV;
                else
                    delete newV;
            }
            return v;
        }

#if _MSC_VER <= 1900
        virtual void GenerateVtableForMSVC_() {}
#endif

    private:
        mutable std::atomic<WeakView*> m_stView;
    };

    namespace details
    {
        template <typename T, class D>
        const volatile RefBase<T, D>* LocateRefBase(const volatile RefBase<T, D>* p)noexcept
        {
            return p;
        }

        template <typename T, class D>
        const RefBase<T, D>* LocateRefBase(const RefBase<T, D>* p)noexcept
        {
            return p;
        }

        template <typename T, class D>
        volatile RefBase<T, D>* LocateRefBase(volatile RefBase<T, D>* p)noexcept
        {
            return p;
        }

        template <typename T, class D>
        RefBase<T, D>* LocateRefBase(RefBase<T, D>* p)noexcept
        {
            return p;
        }
    }

    /**
     * @brief 引用计数指针
     *
     * - 应当使用 MakeRef 来构造引用计数指针
     * - 如果直接从原生指针构造，需要注意原生指针的引用计数（针对原生指针 RefPtr 不会自动 AddRef）
     */
    template <typename T>
    class RefPtr
    {
        template <typename>
        friend class RefPtr;

        template <typename>
        friend class RefWeakPtr;

    public:
        using Pointer = T*;
        using ElementType = T;
        using DeleterType = typename std::remove_pointer<
            decltype(details::LocateRefBase(std::declval<T*>()))>::type::DeleterType;

    public:
        constexpr RefPtr(std::nullptr_t = nullptr)noexcept
            : m_pObject(nullptr) {}

        explicit constexpr RefPtr(T* t)noexcept
            : m_pObject(t) {}

        template <typename U, typename E, typename std::enable_if<
            std::is_convertible<typename std::unique_ptr<U, E>::pointer, Pointer>::value &&
            std::is_convertible<typename std::unique_ptr<U, E>::deleter_type, DeleterType>::value, int>::type = 0>
        RefPtr(std::unique_ptr<U, E>&& r)noexcept
            : m_pObject(r.release()) {}

        template <typename U, typename std::enable_if<
                std::is_convertible<typename RefPtr<U>::Pointer, Pointer>::value &&
                std::is_convertible<typename RefPtr<U>::DeleterType, DeleterType>::value, int>::type = 0>
        RefPtr(const RefPtr<U>& r)noexcept
            : m_pObject(r.Fork()) {}

        template <typename U, typename std::enable_if<
                std::is_convertible<typename RefPtr<U>::Pointer, Pointer>::value &&
                std::is_convertible<typename RefPtr<U>::DeleterType, DeleterType>::value, int>::type = 0>
        RefPtr(RefPtr<U>&& r)noexcept
            : m_pObject(r.Release()) {}

        RefPtr(const RefPtr& r)noexcept
            : m_pObject(r.Fork()) {}

        RefPtr(RefPtr&& r)noexcept
            : m_pObject(r.Release()) {}

        RefPtr& operator=(const RefPtr& r)noexcept
        {
            RefPtr(r).Swap(*this);
            return *this;
        }

        RefPtr& operator=(RefPtr&& r)noexcept
        {
            RefPtr(std::move(r)).Swap(*this);
            return *this;
        }

        ~RefPtr()
        {
            const auto t = m_pObject;
            if (t)
            {
                if (t->details::RefCountBase::DropRef())
                {
                    auto d = std::move(details::LocateRefBase(t)->GetDeleter());
                    std::move(d)(t);
                }
            }
        }

        explicit constexpr operator bool()const noexcept { return GetPointer() != nullptr; }

        T& operator*()const
        {
            assert(GetPointer());
            return *GetPointer();
        }

        T* operator->()const
        {
            assert(GetPointer());
            return GetPointer();
        }

        constexpr bool operator==(std::nullptr_t)const noexcept { return m_pObject == nullptr; }

        constexpr bool operator!=(std::nullptr_t)const noexcept { return m_pObject != nullptr; }

        template <typename U>
        constexpr bool operator==(const RefPtr<U>& r)const noexcept { return m_pObject == r.m_pObject; }

        template <typename U>
        constexpr bool operator==(U* u)const noexcept { return m_pObject == u; }

        template <typename U>
        friend constexpr bool operator==(U* u, const RefPtr<T>& r)noexcept { return u == r.m_pObject; }

        template <typename U>
        constexpr bool operator!=(const RefPtr<U>& r)const noexcept { return m_pObject != r.m_pObject; }

        template <typename U>
        constexpr bool operator!=(U* u)const noexcept { return m_pObject != u; }

        template <typename U>
        friend constexpr bool operator!=(U* u, const RefPtr<T>& r)noexcept { return u != r.m_pObject; }

        template <typename U>
        constexpr bool operator<(const RefPtr<U>& r)const noexcept { return m_pObject < r.m_pObject; }

        template <typename U>
        constexpr bool operator<(U* u)const noexcept { return m_pObject < u; }

        template <typename U>
        friend constexpr bool operator<(U* u, const RefPtr<T>& r)noexcept { return u < r.m_pObject; }

        template <typename U>
        constexpr bool operator>(const RefPtr<U>& r)const noexcept { return m_pObject > r.m_pObject; }

        template <typename U>
        constexpr bool operator>(U* u)const noexcept { return m_pObject > u; }

        template <typename U>
        friend constexpr bool operator>(U* u, const RefPtr<T>& r)noexcept { return u > r.m_pObject; }

        template <typename U>
        constexpr bool operator<=(const RefPtr<U>& r)const noexcept { return m_pObject <= r.m_pObject; }

        template <typename U>
        constexpr bool operator<=(U* u)const noexcept { return m_pObject <= u; }

        template <typename U>
        friend constexpr bool operator<=(U* u, const RefPtr<T>& r)noexcept { return u <= r.m_pObject; }

        template <typename U>
        constexpr bool operator>=(const RefPtr<U>& r)const noexcept { return m_pObject >= r.m_pObject; }

        template <typename U>
        constexpr bool operator>=(U* u)const noexcept { return m_pObject >= u; }

        template <typename U>
        friend constexpr bool operator>=(U* u, const RefPtr<T>& r)noexcept { return u >= r.m_pObject; }

    public:
        /**
         * @brief 获得原生指针
         * @return 指向T类型的指针
         */
        constexpr T* GetPointer()const noexcept
        {
            return m_pObject;
        }

        /**
         * @brief 释放对对象的引用
         * @return 指向T类型的指针
         *
         * 方法释放 RefPtr 对对象的所有权，但是不会减少对象的引用计数。
         */
        T* Release()noexcept
        {
            const auto t = m_pObject;
            m_pObject = nullptr;
            return t;
        }

        /**
         * @brief 是否为唯一所有者
         */
        bool IsRefUnique()const noexcept
        {
            const auto t = m_pObject;
            if (!t)
                return false;
            return t->details::RefCountBase::IsUnique();
        }

        /**
         * @brief 返回引用计数数量
         * @return 引用计数
         */
        long GetRefCount()const noexcept
        {
            const auto t = m_pObject;
            if (!t)
                return 0;
            return t->details::RefCountBase::GetRefCount();
        }

        /**
         * @brief 返回弱引用计数数量
         * @return 弱引用计数
         */
        long GetWeakRefCount()const noexcept
        {
            const auto t = m_pObject;
            if (!t)
                return 0;

            const auto v = details::LocateRefBase(t)->GetWeakView();
            if (!v)
                return 0;

            return v->details::RefCountBase::GetRefCount() - 1;
        }

        /**
         * @brief 重置
         */
        void Reset(std::nullptr_t = nullptr)noexcept
        {
            RefPtr().Swap(*this);
        }

        void Reset(T* t)noexcept
        {
            RefPtr(t).Swap(*this);
        }

        /**
         * @brief 交换
         * @param r 被交换RefPtr
         */
        void Swap(RefPtr& r)noexcept
        {
            const auto t = m_pObject;
            m_pObject = r.m_pObject;
            r.m_pObject = t;
        }

        friend void Swap(RefPtr<T>& l, RefPtr<T>& r)noexcept { l.Swap(r); }

        template <typename U>
        RefPtr<U> CastTo()const noexcept
        {
            RefPtr clone = *this;
            const auto u = static_cast<U*>(clone.GetPointer());
            clone.Release();
            return RefPtr<U>(u);
        }

        template <typename U>
        RefPtr<U> DynamicCastTo()const noexcept
        {
            RefPtr clone = *this;
            const auto u = dynamic_cast<U*>(clone.GetPointer());
            if (u)
                clone.Release();
            return RefPtr<U>(u);
        }

    private:
        T* Fork()const noexcept
        {
            const auto t = m_pObject;
            if (t)
                t->details::RefCountBase::AddRef();
            return t;
        }

    private:
        T* m_pObject;
    };

    /**
     * @brief 弱引用引用计数指针
     * @tparam T 类型
     */
    template <typename T>
    class RefWeakPtr
    {
        template <typename>
        friend class RefPtr;

        template <typename>
        friend class RefWeakPtr;

    public:
        using Pointer = typename RefPtr<T>::Pointer;
        using ElementType = typename RefPtr<T>::ElementType;
        using DeleterType = typename RefPtr<T>::DeleterType;

    private:
        using WeakView = typename std::remove_pointer<
            decltype(details::LocateRefBase(std::declval<Pointer>()))>::type::WeakView;

    private:
        static WeakView* CreateViewFromElement(const volatile T* t)
        {
            if (!t)
                return nullptr;
            const auto v = details::LocateRefBase(t)->RequireWeakView();
            v->details::RefCountBase::AddRef();
            return v;
        }

    private:
        WeakView* m_pObject;

    public:
        constexpr RefWeakPtr(std::nullptr_t = nullptr)noexcept
            : m_pObject(nullptr) {}

        explicit RefWeakPtr(T* t)
            : m_pObject(CreateViewFromElement(t)) {}

        template <typename U,
            typename std::enable_if<
                std::is_convertible<typename RefPtr<U>::Pointer, Pointer>::value &&
                std::is_convertible<typename RefPtr<U>::DeleterType, DeleterType>::value, int>::type = 0>
        RefWeakPtr(const RefPtr<U>& r)
            : m_pObject(CreateViewFromElement(r.GetPointer())) {}

        template <typename U,
            typename std::enable_if<
                std::is_convertible<typename RefWeakPtr<U>::Pointer, Pointer>::value &&
                std::is_convertible<typename RefWeakPtr<U>::DeleterType, DeleterType>::value, int>::type = 0>
        RefWeakPtr(const RefWeakPtr<U>& r)noexcept
            : m_pObject(r.Fork()) {}

        template <typename U,
            typename std::enable_if<
                std::is_convertible<typename RefWeakPtr<U>::Pointer, Pointer>::value &&
                std::is_convertible<typename RefWeakPtr<U>::DeleterType, DeleterType>::value, int>::type = 0>
        RefWeakPtr(RefWeakPtr<U>&& r)noexcept
            : m_pObject(r.Release()) {}

        RefWeakPtr(const RefWeakPtr& r)noexcept
            : m_pObject(r.Fork()) {}

        RefWeakPtr(RefWeakPtr&& r)noexcept
            : m_pObject(r.Release()) {}

        RefWeakPtr& operator=(const RefWeakPtr& r)noexcept
        {
            RefWeakPtr(r).Swap(*this);
            return *this;
        }

        RefWeakPtr& operator=(RefWeakPtr&& r)noexcept
        {
            RefWeakPtr(std::move(r)).Swap(*this);
            return *this;
        }

        ~RefWeakPtr()
        {
            const auto v = m_pObject;
            if (v)
            {
                if (v->details::RefCountBase::DropRef())
                    delete v;
            }
        }

        template <typename U>
        constexpr bool operator==(const RefWeakPtr<U>& r)const noexcept { return m_pObject == r.m_pObject; }

        template <typename U>
        constexpr bool operator!=(const RefWeakPtr<U>& r)const noexcept { return m_pObject != r.m_pObject; }

        template <typename U>
        constexpr bool operator<(const RefWeakPtr<U>& r)const noexcept { return m_pObject < r.m_pObject; }

        template <typename U>
        constexpr bool operator>(const RefWeakPtr<U>& r)const noexcept { return m_pObject > r.m_pObject; }

        template <typename U>
        constexpr bool operator<=(const RefWeakPtr<U>& r)const noexcept { return m_pObject <= r.m_pObject; }

        template <typename U>
        constexpr bool operator>=(const RefWeakPtr<U>& r)const noexcept { return m_pObject >= r.m_pObject; }

    private:
        WeakView* Fork()const noexcept
        {
            const auto v = m_pObject;
            if (!v)
                return nullptr;
            v->details::RefCountBase::AddRef();
            return v;
        }

        WeakView* Release()noexcept
        {
            const auto v = m_pObject;
            m_pObject = nullptr;
            return v;
        }

    public:
        /**
         * @brief 对象是否已经被释放
         * @return 是否被释放
         */
        bool IsExpired()const noexcept
        {
            const auto v = m_pObject;
            if (!v)
                return true;
            return v->IsExpired();
        }

        /**
         * @brief 获取弱引用的计数个数
         * @return 弱引用计数个数
         *
         * 如果被观察对象已经被释放，这个值将没有意义。
         */
        long GetWeakRefCount()const noexcept
        {
            const auto v = m_pObject;
            if (!v)
                return 0;
            return v->details::RefCountBase::GetRefCount() - 1;
        }

        template <typename U = T>
        RefPtr<U> Lock()const noexcept
        {
            const auto v = m_pObject;
            if (!v)
                return nullptr;
            return v->template Lock<U>();
        }

        void Reset(std::nullptr_t = nullptr)noexcept
        {
            RefWeakPtr().Swap(*this);
        }

        void Reset(T* t)
        {
            RefWeakPtr(t).Swap(*this);
        }

        void Swap(RefWeakPtr& r)noexcept
        {
            const auto v = m_pObject;
            m_pObject = r.m_pObject;
            r.m_pObject = v;
        }

        friend void Swap(RefWeakPtr<T>& l, RefWeakPtr<T>& r)noexcept { l.Swap(r); }
    };

    /**
     * @brief 构造引用计数指针
     * @tparam T 类型
     * @tparam Args 参数类型列表
     * @param args 参数列表
     * @return 被构造的对象
     */
    template <typename T, typename... Args>
    RefPtr<T> MakeRef(Args&&... args)
    {
        static_assert(!std::is_array<T>::value, "RefPtr does not accept arrays");
        static_assert(!std::is_reference<T>::value, "RefPtr does not accept references");

        return RefPtr<T>(new T(std::forward<Args>(args)...));
    }
}

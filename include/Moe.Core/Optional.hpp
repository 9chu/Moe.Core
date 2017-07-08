/**
 * @file
 * @date 2017/7/6
 * @see https://github.com/akrzemi1/Optional
 */
#pragma once
#include <cassert>
#include <memory>
#include <utility>
#include <type_traits>
#include <initializer_list>

namespace moe
{
    namespace details
    {
        template <class T>
        inline constexpr T&& ConstexprForward(typename std::remove_reference<T>::type& t)noexcept
        {
            return static_cast<T&&>(t);
        }

        template <class T>
        inline constexpr T&& ConstexprForward(typename std::remove_reference<T>::type&& t)noexcept
        {
            static_assert(!std::is_lvalue_reference<T>::value, "Unexpected type");
            return static_cast<T&&>(t);
        }

        template <class T>
        inline constexpr typename std::remove_reference<T>::type&& ConstexprMove(T&& t)noexcept
        {
            return static_cast<typename std::remove_reference<T>::type&&>(t);
        }

        template <typename T>
        struct HasOverloadedAddressof
        {
            template <class X>
            constexpr static bool HasOverload(...) { return false; }

            template <class X, size_t S = sizeof(std::declval<X&>().operator&())>
            constexpr static bool HasOverload(bool) { return true; }

            constexpr static bool Value = HasOverload<T>(true);
        };

        template <typename T, typename std::enable_if<!HasOverloadedAddressof<T>::Value, bool>::type = false>
        constexpr T* StaticAddressof(T& ref)
        {
            return &ref;
        }

        template <typename T, typename std::enable_if<HasOverloadedAddressof<T>::Value, bool>::type = false>
        T* StaticAddressof(T& ref)
        {
            return std::addressof(ref);
        }

        struct OptionalTrivialInitTag {};

        struct OptionalInPlaceInitTag {};

        template <class T>
        union OptionalStorage
        {
            unsigned char Dummy;
            T Value;

            // 用以区别 Args = void 的情况
            constexpr OptionalStorage(OptionalTrivialInitTag)noexcept
                : Dummy() {}

            template <class... Args>
            constexpr OptionalStorage(Args&&... args)
                : Value(ConstexprForward<Args>(args)...) {}

            ~OptionalStorage() {}
        };

        template <class T>
        union ConstexprOptionalStorage
        {
            unsigned char Dummy;
            T Value;

            constexpr ConstexprOptionalStorage(OptionalTrivialInitTag)noexcept
                : Dummy() {}

            template <class... Args>
            constexpr ConstexprOptionalStorage(Args&&... args)
                : Value(ConstexprForward<Args>(args)...) {}

            ~ConstexprOptionalStorage() = default;
        };

        template <class T>
        struct OptionalBase
        {
            bool Inited;
            OptionalStorage<T> Storage;

            constexpr OptionalBase()noexcept
                : Inited(false), Storage(OptionalTrivialInitTag()) {}

            explicit constexpr OptionalBase(const T& v)
                : Inited(true), Storage(v) {}

            explicit constexpr OptionalBase(T&& v)
                : Inited(true), Storage(ConstexprMove(v)) {}

            template <class... Args>
            explicit OptionalBase(OptionalInPlaceInitTag, Args&&... args)
                : Inited(true), Storage(ConstexprForward<Args>(args)...) {}

            template <class U, class... Args,
                typename std::enable_if<std::is_constructible<T, std::initializer_list<U>>::value, bool>::type = false>
            explicit OptionalBase(OptionalInPlaceInitTag, std::initializer_list<U> l, Args&&... args)
                : Inited(true), Storage(l, std::forward<Args>(args)...) {}

            ~OptionalBase()
            {
                if (Inited)
                    Storage.Value.T::~T();
            }
        };

        template <class T>
        struct ConstexprOptionalBase
        {
            bool Inited;
            ConstexprOptionalStorage<T> Storage;

            constexpr ConstexprOptionalBase()noexcept
                : Inited(false), Storage(OptionalTrivialInitTag()) {}

            explicit constexpr ConstexprOptionalBase(const T& v)
                : Inited(true), Storage(v) {}

            explicit constexpr ConstexprOptionalBase(T&& v)
                : Inited(true), Storage(ConstexprMove(v)) {}

            template <class... Args>
            explicit constexpr ConstexprOptionalBase(OptionalInPlaceInitTag, Args&&... args)
                : Inited(true), Storage(ConstexprForward<Args>(args)...) {}

            template <class U, class... Args,
                typename std::enable_if<std::is_constructible<T, std::initializer_list<U>>::value, bool>::type = false>
            explicit constexpr ConstexprOptionalBase(OptionalInPlaceInitTag, std::initializer_list<U> l, Args&&... args)
                : Inited(true), Storage(l, std::forward<Args>(args)...) {}

            ~ConstexprOptionalBase() = default;
        };

        // 决定析构是否是平凡的
        template <class T>
        using OptionalBase = typename std::conditional<
            std::is_trivially_destructible<T>::value,
            ConstexprOptionalBase<typename std::remove_const<T>::type>,
            OptionalBase<typename std::remove_const<T>::type>>::type;
    }

    /**
     * @brief 原地构造标记
     */
    constexpr details::OptionalInPlaceInitTag InPlaceInit {};

    template <class T>
    class Optional;

    template <class T>
    class Optional<T&>;  // 左值引用特化

    template <class T>
    class Optional :
        private details::OptionalBase<T>
    {
        static_assert(!std::is_same<typename std::decay<T>::type, details::OptionalInPlaceInitTag>::value, "bad T");

    public:
        using ValueType = T;

    public:
        constexpr Optional()noexcept
            : details::OptionalBase<T>() {}

        Optional(const Optional& rhs)
            : details::OptionalBase<T>()
        {
            if (rhs.Initialized())
            {
                ::new (static_cast<void*>(GetPointer())) T(*rhs);
                details::OptionalBase<T>::Inited = true;
            }
        }

        Optional(Optional&& rhs)noexcept(std::is_nothrow_move_constructible<T>::value)
            : details::OptionalBase<T>()
        {
            if (rhs.Initialized())
            {
                ::new (static_cast<void*>(GetPointer())) T(std::move(*rhs));
                details::OptionalBase<T>::Inited = true;
            }
        }

        constexpr Optional(const T& v)
            : details::OptionalBase<T>(v) {}

        constexpr Optional(T&& v)
            : details::OptionalBase<T>(details::ConstexprMove(v)) {}

        template <class... Args,
            typename std::enable_if<!std::is_constructible<T, Args&&...>::value, bool>::type = false>
        explicit constexpr Optional(details::OptionalInPlaceInitTag, Args&&... args)
            : OptionalBase<T>(details::OptionalInPlaceInitTag(), details::ConstexprForward<Args>(args)...) {}

        template <class U, class... Args,
            typename std::enable_if<std::is_constructible<T, std::initializer_list<U>>::value, bool>::type = false>
        explicit constexpr Optional(details::OptionalInPlaceInitTag, std::initializer_list<U> l, Args&&... args)
            : OptionalBase<T>(details::OptionalInPlaceInitTag(), l, details::ConstexprForward<Args>(args)...) {}

        ~Optional() = default;

        Optional& operator=(const Optional& rhs)
        {
            if (Initialized() && !rhs.Initialized())
                Clear();
            else if (!Initialized() && rhs.Initialized())
                Initialize(*rhs);
            else if (Initialized() && rhs.Initialized())
                Value() = *rhs;
            return *this;
        }

        Optional& operator=(Optional&& rhs)noexcept(std::is_nothrow_move_assignable<T>::value &&
            std::is_nothrow_move_constructible<T>::value)
        {
            if (Initialized() && !rhs.Initialized())
                Clear();
            else if (!Initialized() && rhs.Initialized())
                Initialize(std::move(*rhs));
            else if (Initialized() && rhs.Initialized())
                Value() = std::move(*rhs);
            return *this;
        }

        template <class U>
        typename std::enable_if<std::is_same<typename std::decay<U>::type, T>::value, Optional&>::type
        operator=(U&& v)
        {
            if (Initialized())
                Value() = std::forward<U>(v);
            else
                Initialize(std::forward<U>(v));
            return *this;
        }

        constexpr T const* operator ->()const
        {
            assert(Initialized());
            return GetPointer();
        }

        constexpr T* operator ->()
        {
            assert(Initialized());
            return GetPointer();
        }

        constexpr T const& operator *()const&
        {
            assert(Initialized());
            return Value();
        }

        constexpr T& operator *()&
        {
            assert(Initialized());
            return Value();
        }

        constexpr T&& operator *()&&
        {
            assert(Initialized());
            return details::ConstexprMove(Value());
        }

        explicit constexpr operator bool()const noexcept { return Initialized(); }

    public:
        template <class... Args>
        void Emplace(Args&&... args)
        {
            Clear();
            Initialize(std::forward<Args>(args)...);
        }

        template <class U, class... Args>
        void Emplace(std::initializer_list<U> l, Args&&... args)
        {
            Clear();
            Initialize<U, Args...>(l, std::forward<Args>(args)...);
        }

        void Clear()noexcept
        {
            if (Initialized())
                GetPointer()->T::~T();
            details::OptionalBase<T>::Inited = false;
        }

        void Swap(Optional<T>& rhs)noexcept(std::is_nothrow_move_constructible<T>::value &&
            noexcept(std::swap(std::declval<T&>(), std::declval<T&>())))
        {
            if (Initialized() && !rhs.Initialized())
            {
                rhs.Initialize(std::move(**this));
                Clear();
            }
            else if (!Initialized() && rhs.Initialized())
            {
                Initialize(std::move(*rhs));
                rhs.Clear();
            }
            else if (Initialized() && rhs.Initialized())
            {
                std::swap(**this, *rhs);
            }
        }

        constexpr bool HasValue()const noexcept { return Initialized(); }

    private:
        constexpr bool Initialized()const noexcept { return details::OptionalBase<T>::Inited; }

        typename std::remove_const<T>::type* GetPointer()
        {
            return std::addressof(details::OptionalBase<T>::Storage.Value);
        }

        constexpr const T* GetPointer()const
        {
            return details::StaticAddressof(details::OptionalBase<T>::Storage.Value);
        }

        constexpr const T& Value()const& { return details::OptionalBase<T>::Storage.Value; }

        T& Value()& { return details::OptionalBase<T>::Storage.Value; }
        T&& Value()&& { return std::move(details::OptionalBase<T>::Storage.Value); }

        template <class... Args>
        void Initialize(Args&&... args)noexcept(noexcept(T(std::forward<Args>(args)...)))
        {
            assert(!details::OptionalBase<T>::Inited);
            ::new (static_cast<void*>(GetPointer())) T(std::forward<Args>(args)...);
            details::OptionalBase<T>::Inited = true;
        }

        template <class U, class... Args>
        void Initialize(std::initializer_list<U> l, Args&&... args)noexcept(noexcept(T(l, std::forward<Args>(args)...)))
        {
            assert(!details::OptionalBase<T>::Inited);
            ::new (static_cast<void*>(GetPointer())) T(l, std::forward<Args>(args)...);
            details::OptionalBase<T>::Inited = true;
        }
    };

    template <class T>
    class Optional<T&>
    {
        static_assert(!std::is_same<T, details::OptionalInPlaceInitTag>::value, "bad T" );

    public:
        constexpr Optional()noexcept
            : m_pRef(nullptr) {}

        constexpr Optional(T& v)noexcept
            : m_pRef(details::StaticAddressof(v)) {}

        Optional(T&&) = delete;

        constexpr Optional(const Optional& rhs)noexcept
            : m_pRef(rhs.m_pRef) {}

        explicit constexpr Optional(details::OptionalInPlaceInitTag, T& v)noexcept
            : m_pRef(details::StaticAddressof(v)) {}

        explicit Optional(details::OptionalInPlaceInitTag, T&&) = delete;

        ~Optional() = default;

        template <typename U>
        typename std::enable_if<std::is_same<typename std::decay<U>::type, Optional<T&>>::value,
            Optional&>::type
        operator=(U&& rhs)noexcept
        {
            m_pRef = rhs.m_pRef;
            return *this;
        }

        template <typename U>
        typename std::enable_if<!std::is_same<typename std::decay<U>::type, Optional<T&>>::value,
            Optional&>::type
        operator=(U&& rhs)noexcept = delete;

        constexpr T* operator->()const
        {
            assert(m_pRef);
            return m_pRef;
        }

        constexpr T& operator*()const
        {
            assert(m_pRef);
            return *m_pRef;
        }

        explicit constexpr operator bool()const noexcept
        {
            return m_pRef != nullptr;
        }

    public:
        void Emplace(T& v)noexcept { m_pRef = details::StaticAddressof(v); }

        void Emplace(T&&) = delete;

        void Clear()noexcept { m_pRef = nullptr; }

        void Swap(Optional<T&>& rhs)noexcept { std::swap(m_pRef, rhs.m_pRef); }

        constexpr bool HasValue()const noexcept { return m_pRef != nullptr; }

    private:
        T* m_pRef;
    };

    template <class T>
    class Optional<T&&>
    {
        static_assert(sizeof(T) == 0, "Optional rvalue references disallowed");
    };

    template <class T>
    constexpr bool operator==(const Optional<T>& x, const Optional<T>& y)
    {
        return bool(x) != bool(y) ? false : !bool(x) ? true : *x == *y;
    }

    template <class T>
    constexpr bool operator!=(const Optional<T>& x, const Optional<T>& y)
    {
        return !(x == y);
    }

    template <class T>
    constexpr bool operator<(const Optional<T>& x, const Optional<T>& y)
    {
        return (!y) ? false : (!x) ? true : *x < *y;
    }

    template <class T>
    constexpr bool operator>(const Optional<T>& x, const Optional<T>& y)
    {
        return (y < x);
    }

    template <class T>
    constexpr bool operator<=(const Optional<T>& x, const Optional<T>& y)
    {
        return !(y < x);
    }

    template <class T>
    constexpr bool operator>=(const Optional<T>& x, const Optional<T>& y)
    {
        return !(x < y);
    }

    template <class T>
    constexpr bool operator==(const Optional<T>& x, const T& v)
    {
        return bool(x) ? *x == v : false;
    }

    template <class T>
    constexpr bool operator==(const T& v, const Optional<T>& x)
    {
        return bool(x) ? v == *x : false;
    }

    template <class T>
    constexpr bool operator!=(const Optional<T>& x, const T& v)
    {
        return bool(x) ? *x != v : true;
    }

    template <class T>
    constexpr bool operator!=(const T& v, const Optional<T>& x)
    {
        return bool(x) ? v != *x : true;
    }

    template <class T>
    constexpr bool operator<(const Optional<T>& x, const T& v)
    {
        return bool(x) ? *x < v : true;
    }

    template <class T>
    constexpr bool operator>(const T& v, const Optional<T>& x)
    {
        return bool(x) ? v > *x : true;
    }

    template <class T>
    constexpr bool operator>(const Optional<T>& x, const T& v)
    {
        return bool(x) ? *x > v : false;
    }

    template <class T>
    constexpr bool operator<(const T& v, const Optional<T>& x)
    {
        return bool(x) ? v < *x : false;
    }

    template <class T>
    constexpr bool operator>=(const Optional<T>& x, const T& v)
    {
        return bool(x) ? *x >= v : false;
    }

    template <class T>
    constexpr bool operator<=(const T& v, const Optional<T>& x)
    {
        return bool(x) ? v <= *x : false;
    }

    template <class T>
    constexpr bool operator<=(const Optional<T>& x, const T& v)
    {
        return bool(x) ? *x <= v : true;
    }

    template <class T>
    constexpr bool operator>=(const T& v, const Optional<T>& x)
    {
        return bool(x) ? v >= *x : true;
    }

    template <class T>
    constexpr bool operator==(const Optional<T&>& x, const T& v)
    {
        return bool(x) ? *x == v : false;
    }

    template <class T>
    constexpr bool operator==(const T& v, const Optional<T&>& x)
    {
        return bool(x) ? v == *x : false;
    }

    template <class T>
    constexpr bool operator!=(const Optional<T&>& x, const T& v)
    {
        return bool(x) ? *x != v : true;
    }

    template <class T>
    constexpr bool operator!=(const T& v, const Optional<T&>& x)
    {
        return bool(x) ? v != *x : true;
    }

    template <class T>
    constexpr bool operator<(const Optional<T&>& x, const T& v)
    {
        return bool(x) ? *x < v : true;
    }

    template <class T>
    constexpr bool operator>(const T& v, const Optional<T&>& x)
    {
        return bool(x) ? v > *x : true;
    }

    template <class T>
    constexpr bool operator>(const Optional<T&>& x, const T& v)
    {
        return bool(x) ? *x > v : false;
    }

    template <class T>
    constexpr bool operator<(const T& v, const Optional<T&>& x)
    {
        return bool(x) ? v < *x : false;
    }

    template <class T>
    constexpr bool operator>=(const Optional<T&>& x, const T& v)
    {
        return bool(x) ? *x >= v : false;
    }

    template <class T>
    constexpr bool operator<=(const T& v, const Optional<T&>& x)
    {
        return bool(x) ? v <= *x : false;
    }

    template <class T>
    constexpr bool operator<=(const Optional<T&>& x, const T& v)
    {
        return bool(x) ? *x <= v : true;
    }

    template <class T>
    constexpr bool operator>=(const T& v, const Optional<T&>& x)
    {
        return bool(x) ? v >= *x : true;
    }

    template <class T>
    constexpr bool operator==(const Optional<const T&>& x, const T& v)
    {
        return bool(x) ? *x == v : false;
    }

    template <class T>
    constexpr bool operator==(const T& v, const Optional<const T&>& x)
    {
        return bool(x) ? v == *x : false;
    }

    template <class T>
    constexpr bool operator!=(const Optional<const T&>& x, const T& v)
    {
        return bool(x) ? *x != v : true;
    }

    template <class T>
    constexpr bool operator!=(const T& v, const Optional<const T&>& x)
    {
        return bool(x) ? v != *x : true;
    }

    template <class T>
    constexpr bool operator<(const Optional<const T&>& x, const T& v)
    {
        return bool(x) ? *x < v : true;
    }

    template <class T>
    constexpr bool operator>(const T& v, const Optional<const T&>& x)
    {
        return bool(x) ? v > *x : true;
    }

    template <class T>
    constexpr bool operator>(const Optional<const T&>& x, const T& v)
    {
        return bool(x) ? *x > v : false;
    }

    template <class T>
    constexpr bool operator<(const T& v, const Optional<const T&>& x)
    {
        return bool(x) ? v < *x : false;
    }

    template <class T>
    constexpr bool operator>=(const Optional<const T&>& x, const T& v)
    {
        return bool(x) ? *x >= v : false;
    }

    template <class T>
    constexpr bool operator<=(const T& v, const Optional<const T&>& x)
    {
        return bool(x) ? v <= *x : false;
    }

    template <class T>
    constexpr bool operator<=(const Optional<const T&>& x, const T& v)
    {
        return bool(x) ? *x <= v : true;
    }

    template <class T>
    constexpr bool operator>=(const T& v, const Optional<const T&>& x)
    {
        return bool(x) ? v >= *x : true;
    }

    template <class T>
    constexpr Optional<typename std::decay<T>::type> MakeOptional(T&& v)
    {
        return Optional<typename std::decay<T>::type>(details::ConstexprForward<T>(v));
    }

    template <class X>
    constexpr Optional<X&> MakeOptional(std::reference_wrapper<X> v)
    {
        return Optional<X&>(v.get());
    }
}

/**
 * @file
 * @date 2017/7/6
 * @see https://github.com/akrzemi1/Optional
 */
#pragma once
#include <memory>
#include <utility>
#include <type_traits>
#include <initializer_list>

#if 0
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

    template <class T>
    class Optional;

    template <class T>
    class Optional<T&>;  // 左值引用特化

    constexpr details::OptionalInPlaceInitTag InPlaceInit{};

    template <class T>
    class Optional :
        private details::OptionalBase<T>
    {
        static_assert(!std::is_same<typename std::decay<T>::type, details::OptionalInPlaceInitTag>::value, "bad T");

    public:
        using ValueType = T;

    public:
        constexpr Optional()noexcept
            : OptionalBase<T>() {}

        Optional(const Optional& rhs)
            : OptionalBase<T>()
        {
            if (rhs.Initialized())
            {
                ::new (static_cast<void*>(GetPointer())) T(*rhs);
                OptionalBase<T>::Inited = true;
            }
        }

        Optional(Optional&& rhs)noexcept(std::is_nothrow_move_constructible<T>::value)
            : OptionalBase<T>()
        {
            if (rhs.Initialized())
            {
                ::new (static_cast<void*>(GetPointer())) T(std::move(*rhs));
                OptionalBase<T>::Inited = true;
            }
        }

        constexpr Optional(const T& v)
            : OptionalBase<T>(v) {}

        constexpr Optional(T&& v)
            : OptionalBase<T>(details::ConstexprMove(v)) {}

        template <class... Args,
            typename std::enable_if<!std::is_constructible<T, Args&&...>::value, bool>::type = false>
        explicit constexpr Optional(details::OptionalInPlaceInitTag, Args&&... args)
            : OptionalBase<T>(InPlaceInit, details::ConstexprForward<Args>(args)...) {}

        template <class U, class... Args,
            typename std::enable_if<std::is_constructible<T, std::initializer_list<U>>::value, bool>::type = false>
        explicit constexpr Optional(details::OptionalInPlaceInitTag, std::initializer_list<U> l, Args&&... args)
            : OptionalBase<T>(InPlaceInit, l, details::ConstexprForward<Args>(args)...) {}

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

    public:
        template <class... Args>
        void emplace(Args&&... args)
        {
            clear();
            initialize(std::forward<Args>(args)...);
        }

        template <class U, class... Args>
        void emplace(initializer_list<U> il, Args&&... args)
        {
            clear();
            initialize<U, Args...>(il, std::forward<Args>(args)...);
        }

    private:
        constexpr bool Initialized()const noexcept { return OptionalBase<T>::Inited; }

        typename std::remove_const<T>::type* GetPointer() { return std::addressof(OptionalBase<T>::Storage.Value); }

        constexpr const T* GetPointer()const { return details::StaticAddressof(OptionalBase<T>::Storage.Value); }

        constexpr const T& Value()const& { return OptionalBase<T>::Storage.Value; }

        T& Value() & { return OptionalBase<T>::storage_.value_; }
  T&& contained_val() && { return std::move(OptionalBase<T>::storage_.value_); }

        void clear() noexcept {
            if (initialized()) dataptr()->T::~T();
            OptionalBase<T>::init_ = false;
        }

        template <class... Args>
        void initialize(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...)))
        {
            assert(!OptionalBase<T>::init_);
            ::new (static_cast<void*>(dataptr())) T(std::forward<Args>(args)...);
            OptionalBase<T>::init_ = true;
        }

        template <class U, class... Args>
        void initialize(std::initializer_list<U> il, Args&&... args) noexcept(noexcept(T(il, std::forward<Args>(args)...)))
        {
            assert(!OptionalBase<T>::init_);
            ::new (static_cast<void*>(dataptr())) T(il, std::forward<Args>(args)...);
            OptionalBase<T>::init_ = true;
        }

    public:


        // 20.5.4.4, Swap
        void swap(optional<T>& rhs) noexcept(is_nothrow_move_constructible<T>::value && noexcept(swap(declval<T&>(), declval<T&>())))
        {
            if      (initialized() == true  && rhs.initialized() == false) { rhs.initialize(std::move(**this)); clear(); }
            else if (initialized() == false && rhs.initialized() == true)  { initialize(std::move(*rhs)); rhs.clear(); }
            else if (initialized() == true  && rhs.initialized() == true)  { using std::swap; swap(**this, *rhs); }
        }

        // 20.5.4.5, Observers

        explicit constexpr operator bool() const noexcept { return initialized(); }
        constexpr bool has_value() const noexcept { return initialized(); }

        constexpr T const* operator ->() const {
            return TR2_OPTIONAL_ASSERTED_EXPRESSION(initialized(), dataptr());
        }

# if OPTIONAL_HAS_MOVE_ACCESSORS == 1

        OPTIONAL_MUTABLE_CONSTEXPR T* operator ->() {
    assert (initialized());
    return dataptr();
  }

  constexpr T const& operator *() const& {
    return TR2_OPTIONAL_ASSERTED_EXPRESSION(initialized(), contained_val());
  }

  OPTIONAL_MUTABLE_CONSTEXPR T& operator *() & {
    assert (initialized());
    return contained_val();
  }

  OPTIONAL_MUTABLE_CONSTEXPR T&& operator *() && {
    assert (initialized());
    return constexpr_move(contained_val());
  }

  constexpr T const& value() const& {
    return initialized() ? contained_val() : (throw bad_optional_access("bad optional access"), contained_val());
  }

  OPTIONAL_MUTABLE_CONSTEXPR T& value() & {
    return initialized() ? contained_val() : (throw bad_optional_access("bad optional access"), contained_val());
  }

  OPTIONAL_MUTABLE_CONSTEXPR T&& value() && {
    if (!initialized()) throw bad_optional_access("bad optional access");
	return std::move(contained_val());
  }

# else

        T* operator ->() {
            assert (initialized());
            return dataptr();
        }

        constexpr T const& operator *() const {
            return TR2_OPTIONAL_ASSERTED_EXPRESSION(initialized(), contained_val());
        }

        T& operator *() {
            assert (initialized());
            return contained_val();
        }

        constexpr T const& value() const {
            return initialized() ? contained_val() : (throw bad_optional_access("bad optional access"), contained_val());
        }

        T& value() {
            return initialized() ? contained_val() : (throw bad_optional_access("bad optional access"), contained_val());
        }

# endif

# if OPTIONAL_HAS_THIS_RVALUE_REFS == 1

        template <class V>
  constexpr T value_or(V&& v) const&
  {
    return *this ? **this : detail_::convert<T>(constexpr_forward<V>(v));
  }

#   if OPTIONAL_HAS_MOVE_ACCESSORS == 1

  template <class V>
  OPTIONAL_MUTABLE_CONSTEXPR T value_or(V&& v) &&
  {
    return *this ? constexpr_move(const_cast<optional<T>&>(*this).contained_val()) : detail_::convert<T>(constexpr_forward<V>(v));
  }

#   else

  template <class V>
  T value_or(V&& v) &&
  {
    return *this ? constexpr_move(const_cast<optional<T>&>(*this).contained_val()) : detail_::convert<T>(constexpr_forward<V>(v));
  }

#   endif

# else

        template <class V>
        constexpr T value_or(V&& v) const
        {
            return *this ? **this : detail_::convert<T>(constexpr_forward<V>(v));
        }

# endif

        // 20.6.3.6, modifiers
        void reset() noexcept { clear(); }
    };

}
#endif

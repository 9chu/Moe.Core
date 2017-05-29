/**
 * @file
 * @date 2017/5/21
 */
#pragma once
#include <string>
#include <typeinfo>
#include <stdexcept>

namespace moe
{
    class Any
    {
        union Storage
        {
            // 保证小对象优化能够用在float4和string上
            static const size_t StorageSize = (sizeof(void*) > 16u ? sizeof(void*) : 16u) > sizeof(std::string) ?
                (sizeof(void*) > 16u ? sizeof(void*) : 16u) : sizeof(std::string);
            static const size_t AlignSize = (alignof(void*) > 8u ? alignof(void*) : 8u) > alignof(std::string) ?
                (alignof(void*) > 8u ? alignof(void*) : 8u) : alignof(std::string);

            void* Pointer;
            std::aligned_storage<StorageSize, AlignSize>::type Buffer;

            Storage() = default;

            // 考虑到Buffer可能存储非Pod对象，禁止默认拷贝构造
            Storage(const Storage&) = delete;
            Storage& operator=(const Storage&) = delete;
        };

        template <typename T, typename Safe = std::is_nothrow_move_constructible<T>,
            bool Fits = (sizeof(T) <= sizeof(Storage)) && (alignof(T) <= alignof(Storage))>
        using StorageSelector = std::integral_constant<bool, Safe::value && Fits>;

        template <typename T>
        struct LocalStorageManager;

        template <typename T>
        struct HeapStorageManager;

        template <typename T>
        using StorageManager = typename std::conditional<StorageSelector<T>::value,
            LocalStorageManager<T>, HeapStorageManager<T>>::type;

        template <typename T, typename Decayed = typename std::decay<T>::type>
        using Decay = typename std::enable_if<!std::is_same<Decayed, Any>::value, Decayed>::type;

        enum class ManagerOperator
        {
            Access,
            GetTypeInfo,
            Clone,
            Destroy,
            Move,
        };

        union ManagerArg
        {
            void* Object;
            const std::type_info* TypeInfo;
            Any* Other;
        };

        template <typename T>
        struct LocalStorageManager
        {
            static void Manager(ManagerOperator op, const Any* any, ManagerArg* arg)
            {
                auto ptr = reinterpret_cast<const T*>(&any->m_stStorage.Buffer);

                switch (op)
                {
                    case ManagerOperator::Access:
                        arg->Object = const_cast<T*>(ptr);
                        break;
                    case ManagerOperator::GetTypeInfo:
                        arg->TypeInfo = &typeid(T);
                        break;
                    case ManagerOperator::Clone:
                        ::new(&arg->Other->m_stStorage.Buffer) T(*ptr);
                        arg->Other->m_pManager = any->m_pManager;
                        break;
                    case ManagerOperator::Destroy:
                        ptr->~T();
                        break;
                    case ManagerOperator::Move:
                        ::new(&arg->Other->m_stStorage.Buffer) T(*ptr);
                        ptr->~T();
                        arg->Other->m_pManager = any->m_pManager;
                        const_cast<Any*>(any)->m_pManager = nullptr;
                        break;
                }
            }

            template <typename U>
            static void Create(Storage& storage, U&& value)
            {
                void* address = &storage.Buffer;
                ::new(address) T(std::forward<U>(value));
            }
        };

        template <typename T>
        struct HeapStorageManager
        {
            static void Manager(ManagerOperator op, const Any* any, ManagerArg* arg)
            {
                auto ptr = reinterpret_cast<const T*>(any->m_stStorage.Pointer);

                switch (op)
                {
                    case ManagerOperator::Access:
                        arg->Object = const_cast<T*>(ptr);
                        break;
                    case ManagerOperator::GetTypeInfo:
                        arg->TypeInfo = &typeid(T);
                        break;
                    case ManagerOperator::Clone:
                        arg->Other->m_stStorage.Pointer = new T(*ptr);
                        arg->Other->m_pManager = any->m_pManager;
                        break;
                    case ManagerOperator::Destroy:
                        delete ptr;
                        break;
                    case ManagerOperator::Move:
                        arg->Other->m_stStorage.Pointer = any->m_stStorage.Pointer;
                        arg->Other->m_pManager = any->m_pManager;
                        const_cast<Any*>(any)->m_pManager = nullptr;
                        break;
                }
            }

            template <typename U>
            static void Create(Storage& storage, U&& value)
            {
                storage.Pointer = new T(std::forward<U>(value));
            }
        };

        template <typename T>
        static constexpr bool IsValidCast()
        {
            return std::is_reference<T>::value || std::is_copy_constructible<T>::value;
        }

        template <typename T>
        void* InternalCastTo()const
        {
            if (m_pManager != &StorageManager<typename std::decay<T>::type>::Manager)
                return nullptr;
            ManagerArg arg;
            m_pManager(ManagerOperator::Access, this, &arg);
            return arg.Object;
        }

    public:
        Any()noexcept {}

        Any(const Any& rhs)
        {
            if (!rhs.IsEmpty())
            {
                ManagerArg arg;
                arg.Other = this;
                rhs.m_pManager(ManagerOperator::Clone, &rhs, &arg);
            }
        }

        Any(Any&& rhs)noexcept
        {
            if (!rhs.IsEmpty())
            {
                ManagerArg arg;
                arg.Other = this;
                rhs.m_pManager(ManagerOperator::Move, &rhs, &arg);
            }
        }

        template <typename TRaw, typename T = Decay<TRaw>,
            typename Manager = StorageManager<T>,
            typename std::enable_if<std::is_constructible<T, TRaw&&>::value, bool>::type = true>
        Any(TRaw&& value)
            : m_pManager(&Manager::Manager)
        {
            static_assert(std::is_copy_constructible<T>::value, "The contained object must be CopyConstructible");
            Manager::Create(m_stStorage, std::forward<TRaw>(value));
        }

        template <typename TRaw, typename T = Decay<TRaw>,
            typename Manager = StorageManager<T>,
            typename std::enable_if<!std::is_constructible<T, TRaw&&>::value, bool>::type = false>
        Any(TRaw&& value)
            : m_pManager(&Manager::Manager)
        {
            static_assert(std::is_copy_constructible<T>::value, "The contained object must be CopyConstructible");
            Manager::Create(m_stStorage, value);
        }

        ~Any()
        {
            Clear();
        }

        Any& operator=(const Any& rhs)
        {
            if (rhs.IsEmpty())
                Clear();
            else if (this != &rhs)
            {
                if (!IsEmpty())
                    m_pManager(ManagerOperator::Destroy, this, nullptr);
                ManagerArg arg;
                arg.Other = this;
                rhs.m_pManager(ManagerOperator::Clone, &rhs, &arg);
            }

            return *this;
        }

        Any& operator=(Any&& rhs)noexcept
        {
            if (rhs.IsEmpty())
                Clear();
            else if (this != &rhs)
            {
                if (!IsEmpty())
                    m_pManager(ManagerOperator::Destroy, this, nullptr);
                ManagerArg arg;
                arg.Other = this;
                rhs.m_pManager(ManagerOperator::Move, &rhs, &arg);
            }

            return *this;
        }

        template <typename TRaw>
        typename std::enable_if<!std::is_same<Any, typename std::decay<TRaw>::type>::value, Any&>::type
        operator=(TRaw&& rhs)
        {
            *this = Any(std::forward<TRaw>(rhs));
            return *this;
        }

    public:
        void Clear()noexcept
        {
            if (!IsEmpty())
            {
                m_pManager(ManagerOperator::Destroy, this, nullptr);
                m_pManager = nullptr;
            }
        }

        void Swap(Any& rhs)noexcept
        {
            if (IsEmpty() && rhs.IsEmpty())
                return;

            if (!IsEmpty() && !rhs.IsEmpty())
            {
                if (this == &rhs)
                    return;

                Any tmp;
                ManagerArg arg;
                arg.Other = &tmp;
                rhs.m_pManager(ManagerOperator::Move, &rhs, &arg);
                arg.Other = &rhs;
                m_pManager(ManagerOperator::Move, this, &arg);
                arg.Other = this;
                tmp.m_pManager(ManagerOperator::Move, &tmp, &arg);
            }
            else
            {
                Any* empty = IsEmpty() ? this : &rhs;
                Any* full = IsEmpty() ? &rhs : this;
                ManagerArg arg;
                arg.Other = empty;
                full->m_pManager(ManagerOperator::Move, full, &arg);
            }
        }

        bool IsEmpty()const noexcept
        {
            return m_pManager == nullptr;
        }

        const std::type_info& GetTypeInfo()const noexcept
        {
            if (IsEmpty())
                return typeid(void);
            ManagerArg arg;
            m_pManager(ManagerOperator::GetTypeInfo, this, &arg);
            return *arg.TypeInfo;
        }

        template <typename TRaw>
        inline TRaw CastTo()const
        {
            static_assert(IsValidCast<TRaw>(), "Template argument must be a reference or CopyConstructible type");
            auto p = static_cast<const TRaw*>(
                InternalCastTo<typename std::add_const<typename std::remove_reference<TRaw>::type>::type>());
            if (p)
                return *p;
            throw std::runtime_error("Bad cast");
        }

        template <typename TRaw>
        inline TRaw CastTo()
        {
            static_assert(IsValidCast<TRaw>(), "Template argument must be a reference or CopyConstructible type");
            auto p = static_cast<const TRaw*>(InternalCastTo<typename std::remove_reference<TRaw>::type>());
            if (p)
                return *p;
            throw std::runtime_error("Bad cast");
        }

        template <typename TRaw>
        inline TRaw SafeCastTo()const noexcept
        {
            static_assert(IsValidCast<TRaw>(), "Template argument must be a reference or CopyConstructible type");
            static_assert(!std::is_reference<TRaw>(), "SafeCast cannot apply on reference type");

            auto p = static_cast<const TRaw*>(
                InternalCastTo<typename std::add_const<TRaw>::type>());
            if (p)
                return *p;
            return TRaw();
        }

        template <typename TRaw>
        inline TRaw SafeCastTo()noexcept
        {
            static_assert(IsValidCast<TRaw>(), "Template argument must be a reference or CopyConstructible type");
            static_assert(!std::is_reference<TRaw>(), "SafeCast cannot apply on reference type");

            auto p = static_cast<const TRaw*>(InternalCastTo<TRaw>());
            if (p)
                return *p;
            return TRaw();
        }

    private:
        void (*m_pManager)(ManagerOperator, const Any*, ManagerArg*) = nullptr;
        Storage m_stStorage;
    };
}

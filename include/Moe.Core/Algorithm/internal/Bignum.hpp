/**
 * @file
 * @date 2017/5/4
 */
#pragma once
#include "../../Utility/Misc.hpp"

namespace moe
{
    namespace internal
    {
        class Bignum :
            public NonCopyable
        {
        public:
            static const size_t kMaxSignificantBits = 3584;

            static int Compare(const Bignum& a, const Bignum& b);
            static bool Equal(const Bignum& a, const Bignum& b) { return Compare(a, b) == 0; }
            static bool LessEqual(const Bignum& a, const Bignum& b) { return Compare(a, b) <= 0; }
            static bool Less(const Bignum& a, const Bignum& b) { return Compare(a, b) < 0; }

            static int PlusCompare(const Bignum& a, const Bignum& b, const Bignum& c);

            static bool PlusEqual(const Bignum& a, const Bignum& b, const Bignum& c)
            {
                return PlusCompare(a, b, c) == 0;
            }

            static bool PlusLessEqual(const Bignum& a, const Bignum& b, const Bignum& c)
            {
                return PlusCompare(a, b, c) <= 0;
            }

            static bool PlusLess(const Bignum& a, const Bignum& b, const Bignum& c) { return PlusCompare(a, b, c) < 0; }

        private:
            using Chunk = uint32_t;
            using DoubleChunk = uint64_t;

            static const int kChunkSize = sizeof(Chunk) * 8;
            static const int kDoubleChunkSize = sizeof(DoubleChunk) * 8;
            static const size_t kBigitSize = 28;
            static const Chunk kBigitMask = (1 << kBigitSize) - 1;
            static const size_t kBigitCapacity = kMaxSignificantBits / kBigitSize;

            template<typename T>
            static size_t BitSize(T value)
            {
                MOE_UNUSED(value);
                return 8 * sizeof(value);
            }

            template <typename T>
            static size_t SizeInHexChars(T number)
            {
                assert(number > 0);
                T val = number;
                size_t result = 0;

                while (val != 0)
                {
                    val >>= 4;
                    result++;
                }

                return result;
            }

            template <typename T>
            static T HexCharOfValue(int value)
            {
                assert(0 <= value && value <= 16);
                if (value < 10)
                    return static_cast<T>(value + '0');
                return static_cast<T>(value - 10 + 'A');
            }

            template <typename T>
            static uint64_t ReadUInt64(const ArrayView<T>& buffer, size_t from, size_t digitsToRead)
            {
                uint64_t result = 0;
                for (size_t i = from; i < from + digitsToRead; ++i)
                {
                    int digit = buffer[i] - '0';
                    assert(0 <= digit && digit <= 9);
                    result = result * 10 + digit;
                }
                return result;
            }

            template <typename T>
            static int HexCharValue(T c)
            {
                if ('0' <= c && c <= '9')
                    return c - '0';
                if ('a' <= c && c <= 'f')
                    return 10 + c - 'a';
                assert('A' <= c && c <= 'F');
                return 10 + c - 'A';
            }

        public:
            Bignum();

        public:
            void AssignUInt16(uint16_t value);
            void AssignUInt64(uint64_t value);
            void AssignBignum(const Bignum& other);
            void AssignPowerUInt16(uint16_t base, int powerExponent);

            template <typename T = char>
            void AssignDecimalString(const ArrayView<T>& value)
            {
                const int kMaxUint64DecimalDigits = 19;

                Zero();
                size_t length = value.Size();
                unsigned int pos = 0;
                while (length >= kMaxUint64DecimalDigits)
                {
                    uint64_t digits = ReadUInt64(value, pos, kMaxUint64DecimalDigits);
                    pos += kMaxUint64DecimalDigits;
                    length -= kMaxUint64DecimalDigits;
                    MultiplyByPowerOfTen(kMaxUint64DecimalDigits);
                    AddUInt64(digits);
                }

                uint64_t digits = ReadUInt64(value, pos, length);
                MultiplyByPowerOfTen(length);
                AddUInt64(digits);
                Clamp();
            }

            template <typename T = char>
            void AssignHexString(const ArrayView<T>& value)
            {
                Zero();
                size_t length = value.Size();

                size_t neededBigits = length * 4 / kBigitSize + 1;
                EnsureCapacity(neededBigits);
                int stringIndex = static_cast<int>(length) - 1;
                for (size_t i = 0; i < neededBigits - 1; ++i)
                {
                    Chunk currentBigit = 0;
                    for (int j = 0; j < static_cast<int>(kBigitSize) / 4; j++)
                        currentBigit += HexCharValue(value[stringIndex--]) << (j * 4);
                    m_stBigits[i] = currentBigit;
                }
                m_uUsedDigits = neededBigits - 1;

                Chunk mostSignificantBigit = 0;
                for (int j = 0; j <= stringIndex; ++j)
                {
                    mostSignificantBigit <<= 4;
                    mostSignificantBigit += HexCharValue(value[j]);
                }

                if (mostSignificantBigit != 0)
                {
                    m_stBigits[m_uUsedDigits] = mostSignificantBigit;
                    m_uUsedDigits++;
                }

                Clamp();
            }

            void AddUInt64(uint64_t operand);
            void AddBignum(const Bignum& other);
            void SubtractBignum(const Bignum& other);

            void Square();
            void ShiftLeft(int shiftAmount);
            void MultiplyByUInt32(uint32_t factor);
            void MultiplyByUInt64(uint64_t factor);
            void MultiplyByPowerOfTen(int exponent);
            void Times10() { return MultiplyByUInt32(10); }
            uint16_t DivideModuloIntBignum(const Bignum& other);

            template <typename T = char>
            bool ToHexString(T* buffer, size_t bufferSize)const
            {
                assert(IsClamped());
                assert(kBigitSize % 4 == 0);
                const int kHexCharsPerBigit = kBigitSize / 4;

                if (m_uUsedDigits == 0)
                {
                    if (bufferSize < 2)
                        return false;
                    buffer[0] = '0';
                    buffer[1] = '\0';
                    return true;
                }

                size_t neededChars = (BigitLength() - 1) * kHexCharsPerBigit +
                    SizeInHexChars(m_stBigits[m_uUsedDigits - 1]) + 1;
                if (neededChars > bufferSize)
                    return false;

                size_t stringIndex = neededChars - 1;
                buffer[stringIndex--] = '\0';
                for (int i = 0; i < m_iExponent; ++i)
                {
                    for (int j = 0; j < kHexCharsPerBigit; ++j)
                    {
                        buffer[stringIndex--] = '0';
                    }
                }
                for (size_t i = 0; i < m_uUsedDigits - 1; ++i)
                {
                    Chunk currentBigit = m_stBigits[i];
                    for (int j = 0; j < kHexCharsPerBigit; ++j)
                    {
                        buffer[stringIndex--] = HexCharOfValue<T>(currentBigit & 0xF);
                        currentBigit >>= 4;
                    }
                }

                Chunk mostSignificantBigit = m_stBigits[m_uUsedDigits - 1];
                while (mostSignificantBigit != 0)
                {
                    buffer[stringIndex--] = HexCharOfValue<T>(mostSignificantBigit & 0xF);
                    mostSignificantBigit >>= 4;
                }

                return true;
            }

        private:
            void EnsureCapacity(size_t size)
            {
                if (size > kBigitCapacity)
                    MOE_UNREACHABLE();
            }
            void Align(const Bignum& other);
            void Clamp();
            bool IsClamped()const;
            void Zero();
            void BigitsShiftLeft(int shiftAmount);
            int BigitLength()const { return static_cast<int>(m_uUsedDigits) + m_iExponent; }
            Chunk BigitAt(int index)const;
            void SubtractTimes(const Bignum& other, int factor);

        private:
            Chunk m_aBigitsBuffer[kBigitCapacity];
            MutableArrayView<Chunk> m_stBigits;
            size_t m_uUsedDigits;
            int m_iExponent;
        };
    }
}

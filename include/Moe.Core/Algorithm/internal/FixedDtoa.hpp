/**
 * @file
 * @date 2017/5/1
 * @see https://github.com/google/double-conversion
 */
#pragma once
#include "DiyFp.hpp"

namespace moe
{
    namespace internal
    {
        class UInt128
        {
        private:
            static const uint64_t kMask32 = 0xFFFFFFFF;

        public:
            UInt128()
                : m_ulHighBits(0), m_ulLowBits(0) {}
            UInt128(uint64_t high, uint64_t low)
                : m_ulHighBits(high), m_ulLowBits(low) {}

            void Multiply(uint32_t multiplicand)
            {
                uint64_t accumulator;

                accumulator = (m_ulLowBits & kMask32) * multiplicand;
                uint32_t part = static_cast<uint32_t>(accumulator & kMask32);
                accumulator >>= 32;
                accumulator = accumulator + (m_ulLowBits >> 32) * multiplicand;
                m_ulLowBits = (accumulator << 32) + part;
                accumulator >>= 32;
                accumulator = accumulator + (m_ulHighBits & kMask32) * multiplicand;
                part = static_cast<uint32_t>(accumulator & kMask32);
                accumulator >>= 32;
                accumulator = accumulator + (m_ulHighBits >> 32) * multiplicand;
                m_ulHighBits = (accumulator << 32) + part;

                assert((accumulator >> 32) == 0);
            }

            void Shift(int shiftAmount)
            {
                assert(-64 <= shiftAmount && shiftAmount <= 64);

                if (shiftAmount == 0)
                    return;

                if (shiftAmount == -64)
                {
                    m_ulHighBits = m_ulLowBits;
                    m_ulLowBits = 0;
                }
                else if (shiftAmount == 64)
                {
                    m_ulLowBits = m_ulHighBits;
                    m_ulHighBits = 0;
                }
                else if (shiftAmount <= 0)
                {
                    m_ulHighBits <<= -shiftAmount;
                    m_ulHighBits += m_ulLowBits >> (64 + shiftAmount);
                    m_ulLowBits <<= -shiftAmount;
                }
                else
                {
                    m_ulLowBits >>= shiftAmount;
                    m_ulLowBits += m_ulHighBits << (64 - shiftAmount);
                    m_ulHighBits >>= shiftAmount;
                }
            }

            int DivModPowerOf2(int power)
            {
                if (power >= 64)
                {
                    int result = static_cast<int>(m_ulHighBits >> (power - 64));
                    m_ulHighBits -= static_cast<uint64_t>(result) << (power - 64);
                    return result;
                }

                uint64_t partLow = m_ulLowBits >> power;
                uint64_t partHigh = m_ulHighBits << (64 - power);
                int result = static_cast<int>(partLow + partHigh);
                m_ulHighBits = 0;
                m_ulLowBits -= partLow << power;
                return result;
            }

            bool IsZero()const { return m_ulHighBits == 0 && m_ulLowBits == 0; }

            int BitAt(size_t position)const
            {
                if (position >= 64)
                    return static_cast<int>(m_ulHighBits >> (position - 64)) & 1;
                return static_cast<int>(m_ulLowBits >> position) & 1;
            }

        private:
            uint64_t m_ulHighBits;
            uint64_t m_ulLowBits;
        };

        class FixedDtoa
        {
        public:
            static const int kDoubleSignificandSize = 53;

            template <typename T>
            static void FillDigits32FixedLength(uint32_t number, size_t requestedLength, ArrayView<T>& buffer,
                size_t& length)
            {
                // for (int i = requestedLength - 1; i >= 0; --i)
                if (requestedLength > 0)
                {
                    for (size_t i = requestedLength; i-- > 0;)
                    {
                        buffer[length + i] = '0' + number % 10;
                        number /= 10;
                    }
                }

                length += requestedLength;
            }

            template <typename T>
            static void FillDigits32(uint32_t number, ArrayView<T>& buffer, size_t& length)
            {
                size_t numberLength = 0;
                while (number != 0)
                {
                    int digit = number % 10;
                    number /= 10;
                    buffer[length + numberLength] = static_cast<T>('0' + digit);
                    numberLength++;
                }

                size_t i = length;
                size_t j = length + numberLength - 1;
                while (i < j)
                {
                    T tmp = buffer[i];
                    buffer[i] = buffer[j];
                    buffer[j] = tmp;
                    i++;
                    j--;
                }

                length += numberLength;
            }

            template <typename T>
            static void FillDigits64FixedLength(uint64_t number, ArrayView<T>& buffer, size_t& length)
            {
                const uint32_t kTen7 = 10000000;

                uint32_t part2 = static_cast<uint32_t>(number % kTen7);
                number /= kTen7;
                uint32_t part1 = static_cast<uint32_t>(number % kTen7);
                uint32_t part0 = static_cast<uint32_t>(number / kTen7);

                FillDigits32FixedLength(part0, 3, buffer, length);
                FillDigits32FixedLength(part1, 7, buffer, length);
                FillDigits32FixedLength(part2, 7, buffer, length);
            }

            template <typename T>
            static void FillDigits64(uint64_t number, ArrayView<T>& buffer, size_t& length)
            {
                const uint32_t kTen7 = 10000000;

                uint32_t part2 = static_cast<uint32_t>(number % kTen7);
                number /= kTen7;
                uint32_t part1 = static_cast<uint32_t>(number % kTen7);
                uint32_t part0 = static_cast<uint32_t>(number / kTen7);

                if (part0 != 0)
                {
                    FillDigits32(part0, buffer, length);
                    FillDigits32FixedLength(part1, 7, buffer, length);
                    FillDigits32FixedLength(part2, 7, buffer, length);
                }
                else if (part1 != 0)
                {
                    FillDigits32(part1, buffer, length);
                    FillDigits32FixedLength(part2, 7, buffer, length);
                }
                else
                    FillDigits32(part2, buffer, length);
            }

            template <typename T>
            static void RoundUp(ArrayView<T>& buffer, size_t& length, int& decimalPoint)
            {
                if (length == 0)
                {
                    buffer[0] = '1';
                    decimalPoint = 1;
                    length = 1;
                    return;
                }

                buffer[length - 1]++;
                for (size_t i = length - 1; i > 0; --i)
                {
                    if (buffer[i] != '0' + 10)
                        return;
                    buffer[i] = '0';
                    buffer[i - 1]++;
                }

                if (buffer[0] == '0' + 10)
                {
                    buffer[0] = '1';
                    ++decimalPoint;
                }
            }

            template <typename T>
            static void FillFractionals(uint64_t fractionals, int exponent, size_t fractionalCount,
                ArrayView<T>& buffer, size_t& length, int& decimalPoint)
            {
                assert(-128 <= exponent && exponent <= 0);

                if (-exponent <= 64)
                {
                    assert(fractionals >> 56 == 0);
                    int point = -exponent;
                    for (size_t i = 0; i < fractionalCount; ++i)
                    {
                        if (fractionals == 0)
                            break;

                        fractionals *= 5;
                        point--;
                        int digit = static_cast<int>(fractionals >> point);
                        assert(digit <= 9);
                        buffer[length] = static_cast<char>('0' + digit);
                        ++length;
                        fractionals -= static_cast<uint64_t>(digit) << point;
                    }

                    assert(fractionals == 0 || point - 1 >= 0);
                    if ((fractionals != 0) && ((fractionals >> (point - 1)) & 1) == 1)
                        RoundUp(buffer, length, decimalPoint);
                }
                else
                {
                    assert(64 < -exponent && -exponent <= 128);
                    UInt128 fractionals128 = UInt128(fractionals, 0);
                    fractionals128.Shift(-exponent - 64);
                    size_t point = 128;
                    for (size_t i = 0; i < fractionalCount; ++i)
                    {
                        if (fractionals128.IsZero())
                            break;

                        fractionals128.Multiply(5);
                        point--;
                        int digit = fractionals128.DivModPowerOf2(point);
                        assert(digit <= 9);
                        buffer[length] = static_cast<char>('0' + digit);
                        ++length;
                    }
                    if (fractionals128.BitAt(point - 1) == 1)
                        RoundUp(buffer, length, decimalPoint);
                }
            }

            template <typename T>
            static void TrimZeros(ArrayView<T>& buffer, size_t& length, int& decimalPoint)
            {
                while (length > 0 && buffer[length - 1] == '0')
                    --length;

                size_t firstNonZero = 0;
                while (firstNonZero < length && buffer[firstNonZero] == '0')
                    firstNonZero++;

                if (firstNonZero != 0)
                {
                    for (size_t i = firstNonZero; i < length; ++i)
                        buffer[i - firstNonZero] = buffer[i];

                    length -= firstNonZero;
                    decimalPoint -= firstNonZero;
                }
            }

            template <typename T>
            static bool Dtoa(double v, size_t fractionalCount, ArrayView<T>& buffer, size_t& length, int& decimalPoint)
            {
                const uint32_t kMaxUInt32 = 0xFFFFFFFF;
                uint64_t significand = Double(v).Significand();
                int exponent = Double(v).Exponent();

                if (exponent > 20)
                    return false;
                if (fractionalCount > 20)
                    return false;

                length = 0;
                if (exponent + kDoubleSignificandSize > 64)
                {
                    const uint64_t kFive17 = 0xB1A2BC2EC5ull;  // 5^17
                    uint64_t divisor = kFive17;
                    int divisor_power = 17;
                    uint64_t dividend = significand;
                    uint32_t quotient;
                    uint64_t remainder;

                    if (exponent > divisor_power)
                    {
                        dividend <<= exponent - divisor_power;
                        quotient = static_cast<uint32_t>(dividend / divisor);
                        remainder = (dividend % divisor) << divisor_power;
                    }
                    else
                    {
                        divisor <<= divisor_power - exponent;
                        quotient = static_cast<uint32_t>(dividend / divisor);
                        remainder = (dividend % divisor) << exponent;
                    }

                    FillDigits32(quotient, buffer, length);
                    FillDigits64FixedLength(remainder, buffer, length);
                    decimalPoint = length;
                }
                else if (exponent >= 0)
                {
                    significand <<= exponent;
                    FillDigits64(significand, buffer, length);
                    decimalPoint = length;
                }
                else if (exponent > -kDoubleSignificandSize)
                {
                    uint64_t integrals = significand >> -exponent;
                    uint64_t fractionals = significand - (integrals << -exponent);
                    if (integrals > kMaxUInt32)
                        FillDigits64(integrals, buffer, length);
                    else
                        FillDigits32(static_cast<uint32_t>(integrals), buffer, length);

                    decimalPoint = length;
                    FillFractionals(fractionals, exponent, fractionalCount, buffer, length, decimalPoint);
                }
                else if (exponent < -128)
                {
                    assert(fractionalCount <= 20);
                    buffer[0] = '\0';
                    length = 0;
                    decimalPoint = -fractionalCount;
                }
                else
                {
                    decimalPoint = 0;
                    FillFractionals(significand, exponent, fractionalCount, buffer, length, decimalPoint);
                }

                TrimZeros(buffer, length, decimalPoint);
                buffer[length] = '\0';
                if (length == 0)
                    decimalPoint = -fractionalCount;
                return true;
            }
        };
    }
}

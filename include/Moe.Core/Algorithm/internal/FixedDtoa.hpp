/**
 * @file
 * @date 2017/5/1
 * @see https://github.com/google/double-conversion
 */
#pragma once
#include "DiyFp.hpp"

#if 0
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

            int BitAt(int position)const
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
            static void FillDigits32FixedLength(uint32_t number, size_t requestedLength, Buffer<T>& buffer,
                size_t* length)
            {
                // for (int i = requestedLength - 1; i >= 0; --i)
                for (size_t i = requestedLength; i-- > 0;)
                {
                    buffer[(*length) + i] = '0' + number % 10;
                    number /= 10;
                }
                *length += requestedLength;
            }

            template <typename T>
            static void FillDigits32(uint32_t number, Buffer<T>& buffer, size_t* length)
            {
                size_t numberLength = 0;
                while (number != 0)
                {
                    int digit = number % 10;
                    number /= 10;
                    buffer[(*length) + numberLength] = static_cast<T>('0' + digit);
                    numberLength++;
                }

                size_t i = *length;
                size_t j = *length + numberLength - 1;
                while (i < j)
                {
                    T tmp = buffer[i];
                    buffer[i] = buffer[j];
                    buffer[j] = tmp;
                    i++;
                    j--;
                }

                *length += numberLength;
            }

            template <typename T>
            static void FillDigits64FixedLength(uint64_t number, Buffer<T>& buffer, size_t* length)
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
            static void FillDigits64(uint64_t number, Buffer<T>& buffer, size_t* length)
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
            static void RoundUp(Buffer<T>& buffer, size_t* length, size_t* decimalPoint)
            {
                if (*length == 0)
                {
                    buffer[0] = '1';
                    *decimalPoint = 1;
                    *length = 1;
                    return;
                }

                buffer[(*length) - 1]++;
                for (int i = (*length) - 1; i > 0; --i)
                {
                    if (buffer[i] != '0' + 10)
                        return;
                    buffer[i] = '0';
                    buffer[i - 1]++;
                }

                if (buffer[0] == '0' + 10)
                {
                    buffer[0] = '1';
                    (*decimalPoint)++;
                }
            }

            template <typename T>
            static void FillFractionals(uint64_t fractionals, int exponent, int fractionalCount, Buffer<T>& buffer,
                size_t* length, size_t* decimalPoint)
            {
                assert(-128 <= exponent && exponent <= 0);

                if (-exponent <= 64)
                {
                    assert(fractionals >> 56 == 0);
                    int point = -exponent;
                    for (int i = 0; i < fractionalCount; ++i)
                    {
                        if (fractionals == 0)
                            break;

                        fractionals *= 5;
                        point--;
                        int digit = static_cast<int>(fractionals >> point);
                        assert(digit <= 9);
                        buffer[*length] = static_cast<char>('0' + digit);
                        (*length)++;
                        fractionals -= static_cast<uint64_t>(digit) << point;
                    }

                    assert(fractionals == 0 || point - 1 >= 0);
                    if ((fractionals != 0) && ((fractionals >> (point - 1)) & 1) == 1)
                        RoundUp(buffer, length, decimalPoint);
                } else {  // We need 128 bits.
                    ASSERT(64 < -exponent && -exponent <= 128);
                    UInt128 fractionals128 = UInt128(fractionals, 0);
                    fractionals128.Shift(-exponent - 64);
                    int point = 128;
                    for (int i = 0; i < fractional_count; ++i) {
                        if (fractionals128.IsZero()) break;
                        // As before: instead of multiplying by 10 we multiply by 5 and adjust the
                        // point location.
                        // This multiplication will not overflow for the same reasons as before.
                        fractionals128.Multiply(5);
                        point--;
                        int digit = fractionals128.DivModPowerOf2(point);
                        ASSERT(digit <= 9);
                        buffer[*length] = static_cast<char>('0' + digit);
                        (*length)++;
                    }
                    if (fractionals128.BitAt(point - 1) == 1) {
                        RoundUp(buffer, length, decimal_point);
                    }
                }
            }
        };









// The given fractionals number represents a fixed-point number with binary
// point at bit (-exponent).
// Preconditions:
//   -128 <= exponent <= 0.
//   0 <= fractionals * 2^exponent < 1
//   The buffer holds the result.
// The function will round its result. During the rounding-process digits not
// generated by this function might be updated, and the decimal-point variable
// might be updated. If this function generates the digits 99 and the buffer
// already contained "199" (thus yielding a buffer of "19999") then a
// rounding-up will change the contents of the buffer to "20000".



// Removes leading and trailing zeros.
// If leading zeros are removed then the decimal point position is adjusted.
        static void TrimZeros(Vector<char> buffer, int* length, int* decimal_point) {
            while (*length > 0 && buffer[(*length) - 1] == '0') {
                (*length)--;
            }
            int first_non_zero = 0;
            while (first_non_zero < *length && buffer[first_non_zero] == '0') {
                first_non_zero++;
            }
            if (first_non_zero != 0) {
                for (int i = first_non_zero; i < *length; ++i) {
                    buffer[i - first_non_zero] = buffer[i];
                }
                *length -= first_non_zero;
                *decimal_point -= first_non_zero;
            }
        }


        bool FastFixedDtoa(double v,
            int fractional_count,
            Vector<char> buffer,
            int* length,
            int* decimal_point) {
            const uint32_t kMaxUInt32 = 0xFFFFFFFF;
            uint64_t significand = Double(v).Significand();
            int exponent = Double(v).Exponent();
            // v = significand * 2^exponent (with significand a 53bit integer).
            // If the exponent is larger than 20 (i.e. we may have a 73bit number) then we
            // don't know how to compute the representation. 2^73 ~= 9.5*10^21.
            // If necessary this limit could probably be increased, but we don't need
            // more.
            if (exponent > 20) return false;
            if (fractional_count > 20) return false;
            *length = 0;
            // At most kDoubleSignificandSize bits of the significand are non-zero.
            // Given a 64 bit integer we have 11 0s followed by 53 potentially non-zero
            // bits:  0..11*..0xxx..53*..xx
            if (exponent + kDoubleSignificandSize > 64) {
                // The exponent must be > 11.
                //
                // We know that v = significand * 2^exponent.
                // And the exponent > 11.
                // We simplify the task by dividing v by 10^17.
                // The quotient delivers the first digits, and the remainder fits into a 64
                // bit number.
                // Dividing by 10^17 is equivalent to dividing by 5^17*2^17.
                const uint64_t kFive17 = UINT64_2PART_C(0xB1, A2BC2EC5);  // 5^17
                uint64_t divisor = kFive17;
                int divisor_power = 17;
                uint64_t dividend = significand;
                uint32_t quotient;
                uint64_t remainder;
                // Let v = f * 2^e with f == significand and e == exponent.
                // Then need q (quotient) and r (remainder) as follows:
                //   v            = q * 10^17       + r
                //   f * 2^e      = q * 10^17       + r
                //   f * 2^e      = q * 5^17 * 2^17 + r
                // If e > 17 then
                //   f * 2^(e-17) = q * 5^17        + r/2^17
                // else
                //   f  = q * 5^17 * 2^(17-e) + r/2^e
                if (exponent > divisor_power) {
                    // We only allow exponents of up to 20 and therefore (17 - e) <= 3
                    dividend <<= exponent - divisor_power;
                    quotient = static_cast<uint32_t>(dividend / divisor);
                    remainder = (dividend % divisor) << divisor_power;
                } else {
                    divisor <<= divisor_power - exponent;
                    quotient = static_cast<uint32_t>(dividend / divisor);
                    remainder = (dividend % divisor) << exponent;
                }
                FillDigits32(quotient, buffer, length);
                FillDigits64FixedLength(remainder, buffer, length);
                *decimal_point = *length;
            } else if (exponent >= 0) {
                // 0 <= exponent <= 11
                significand <<= exponent;
                FillDigits64(significand, buffer, length);
                *decimal_point = *length;
            } else if (exponent > -kDoubleSignificandSize) {
                // We have to cut the number.
                uint64_t integrals = significand >> -exponent;
                uint64_t fractionals = significand - (integrals << -exponent);
                if (integrals > kMaxUInt32) {
                    FillDigits64(integrals, buffer, length);
                } else {
                    FillDigits32(static_cast<uint32_t>(integrals), buffer, length);
                }
                *decimal_point = *length;
                FillFractionals(fractionals, exponent, fractional_count,
                    buffer, length, decimal_point);
            } else if (exponent < -128) {
                // This configuration (with at most 20 digits) means that all digits must be
                // 0.
                ASSERT(fractional_count <= 20);
                buffer[0] = '\0';
                *length = 0;
                *decimal_point = -fractional_count;
            } else {
                *decimal_point = 0;
                FillFractionals(significand, exponent, fractional_count,
                    buffer, length, decimal_point);
            }
            TrimZeros(buffer, length, decimal_point);
            buffer[*length] = '\0';
            if ((*length) == 0) {
                // The string is empty and the decimal_point thus has no importance. Mimick
                // Gay's dtoa and and set it to -fractional_count.
                *decimal_point = -fractional_count;
            }
            return true;
        }

    }
}
#endif

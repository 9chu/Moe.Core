/**
 * @file
 * @date 2017/5/9
 * @see https://github.com/google/double-conversion
 */
#pragma once
#include "Dtoa.hpp"

#include <limits>

namespace moe
{
    namespace internal
    {
        template <typename T = char>
        class StringToDoubleConverter :
            public NonCopyable
        {
        public:
            // Enumeration for allowing octals and ignoring junk when converting
            // strings to numbers.
            enum class AtodFlags
            {
                Default = 0,
                AllowHex = 1,
                AllowOctals = 2,
                AllowTrailingJunk = 4,
                AllowLeadingSpaces = 8,
                AllowTrailingSpaces = 16,
                AllowSpacesAfterSign = 32
            };

            // Returns a converter following the EcmaScript specification.
            static const StringToDoubleConverter& EcmaScriptConverter()
            {
                static const T kInfinityString[] = { 'I', 'n', 'f', 'i', 'n', 'i', 't', 'y', '\0' };
                static const T kNanString[] = { 'N', 'a', 'N', '\0' };
                static StringToDoubleConverter s_stConverter(
                    static_cast<AtodFlags>(AtodFlags::AllowTrailingJunk | AtodFlags::AllowLeadingSpaces |
                        AtodFlags::AllowTrailingSpaces),
                    Double::Nan(),
                    Double::Nan(),
                    kInfinityString,
                    kNanString);
                return s_stConverter;
            }

        private:
            // Double operations detection based on target architecture.
            // Linux uses a 80bit wide floating point stack on x86. This induces double
            // rounding, which in turn leads to wrong results.
            // An easy way to test if the floating-point operations are correct is to
            // evaluate: 89255.0/1e22. If the floating-point stack is 64 bits wide then
            // the result is equal to 89255e-22.
            // The best way to test this, is to create a division-function and to compare
            // the output of the division with the expected result. (Inlining must be
            // disabled.)
            // On Linux,x86 89255e-22 != Div_double(89255.0/1e22)
#if defined(_M_X64) || defined(__x86_64__) || \
    defined(__ARMEL__) || defined(__avr32__) || \
    defined(__hppa__) || defined(__ia64__) || \
    defined(__mips__) || \
    defined(__powerpc__) || defined(__ppc__) || defined(__ppc64__) || \
    defined(_POWER) || defined(_ARCH_PPC) || defined(_ARCH_PPC64) || \
    defined(__sparc__) || defined(__sparc) || defined(__s390__) || \
    defined(__SH4__) || defined(__alpha__) || \
    defined(_MIPS_ARCH_MIPS32R2) || \
    defined(__AARCH64EL__) || defined(__aarch64__) || \
    defined(__riscv)
            constexpr static const bool kPlatformCorrectDoubleOperations = true;
#elif defined(__mc68000__)
            constexpr static const bool kPlatformCorrectDoubleOperations = false;
#elif defined(_M_IX86) || defined(__i386__) || defined(__i386)
#if defined(_WIN32)
// Windows uses a 64bit wide floating point stack.
            constexpr static const bool kPlatformCorrectDoubleOperations = true;
#else
            static const bool kPlatformCorrectDoubleOperations = false;
#endif  // _WIN32
#else
#error Target architecture was not detected as supported by Double-Conversion.
#endif

            // 2^53 = 9007199254740992.
            // Any integer with at most 15 decimal digits will hence fit into a double
            // (which has a 53bit significand) without loss of precision.
            static const int kMaxExactDoubleIntegerDecimalDigits = 15;

            // 2^64 = 18446744073709551616 > 10^19
            static const int kMaxUInt64DecimalDigits = 19;

            // Max double: 1.7976931348623157 x 10^308
            // Min non-zero double: 4.9406564584124654 x 10^-324
            // Any x >= 10^309 is interpreted as +infinity.
            // Any x <= 10^-324 is interpreted as 0.
            // Note that 2.5e-324 (despite being smaller than the min double) will be read
            // as non-zero (equal to the min non-zero double).
            static const int kMaxDecimalPower = 309;
            static const int kMinDecimalPower = -324;

            static const size_t kMaxSignificantDecimalDigits = 780;

            static ArrayView<T> TrimLeadingZeros(ArrayView<T> buffer)
            {
                for (size_t i = 0; i < buffer.Size(); ++i)
                {
                    if (buffer[i] != '0')
                        return buffer.Slice(i, buffer.Size());
                }

                return ArrayView<T>(buffer.GetBuffer(), 0);
            }

            static ArrayView<T> TrimTrailingZeros(ArrayView<T> buffer)
            {
                for (size_t i = buffer.Size(); i-- > 0;)
                {
                    if (buffer[i] != '0')
                        return buffer.Slice(0, i + 1);
                }

                return ArrayView<T>(buffer.GetBuffer(), 0);
            }

            static void CutToMaxSignificantDigits(ArrayView<T> buffer, int exponent, T& significantBuffer,
                int& significantExponent)
            {
                for (size_t i = 0; i < kMaxSignificantDecimalDigits - 1; ++i)
                    significantBuffer[i] = buffer[i];

                // The input buffer has been trimmed. Therefore the last digit must be
                // different from '0'.
                assert(buffer[buffer.Size() - 1] != '0');

                // Set the last digit to be non-zero. This is sufficient to guarantee
                // correct rounding.
                significantBuffer[kMaxSignificantDecimalDigits - 1] = '1';
                significantExponent = exponent + (buffer.Size() - kMaxSignificantDecimalDigits);
            }

            static void TrimAndCut(ArrayView<T> buffer, int exponent, T* bufferCopySpace, int spaceSize,
                ArrayView<T>& trimmed, int& updatedExponent)
            {
                ArrayView<T> leftTrimmed = TrimLeadingZeros(buffer);
                ArrayView<T> rightTrimmed = TrimTrailingZeros(leftTrimmed);
                exponent += leftTrimmed.Size() - rightTrimmed.Size();
                if (rightTrimmed.Size() > kMaxSignificantDecimalDigits)
                {
                    MOE_UNUSED(spaceSize);
                    assert(spaceSize >= kMaxSignificantDecimalDigits);
                    CutToMaxSignificantDigits(rightTrimmed, exponent, bufferCopySpace, updatedExponent);
                    trimmed = ArrayView<T>(bufferCopySpace, kMaxSignificantDecimalDigits);
                }
                else
                {
                    trimmed = rightTrimmed;
                    updatedExponent = exponent;
                }
            }

            // Reads digits from the buffer and converts them to a uint64.
            // Reads in as many digits as fit into a uint64.
            // When the string starts with "1844674407370955161" no further digit is read.
            // Since 2^64 = 18446744073709551616 it would still be possible read another
            // digit if it was less or equal than 6, but this would complicate the code.
            static uint64_t ReadUInt64(ArrayView<T> buffer, size_t& numberOfReadDigits)
            {
                uint64_t result = 0;
                size_t i = 0;
                while (i < buffer.Size() && result <= (std::numeric_limits<uint64_t>::max() / 10 - 1))
                {
                    int digit = buffer[i++] - '0';
                    assert(0 <= digit && digit <= 9);
                    result = 10 * result + digit;
                }
                numberOfReadDigits = i;
                return result;
            }

            // Reads a DiyFp from the buffer.
            // The returned DiyFp is not necessarily normalized.
            // If remaining_decimals is zero then the returned DiyFp is accurate.
            // Otherwise it has been rounded and has error of at most 1/2 ulp.
            static void ReadDiyFp(ArrayView<T> buffer, DiyFp& result, size_t& remainingDecimals)
            {
                size_t readDigits;
                uint64_t significand = ReadUInt64(buffer, readDigits);
                if (buffer.Size() == readDigits)
                {
                    result = DiyFp(significand, 0);
                    remainingDecimals = 0;
                }
                else
                {
                    // Round the significand.
                    if (buffer[readDigits] >= '5')
                        significand++;

                    // Compute the binary exponent.
                    int exponent = 0;
                    result = DiyFp(significand, exponent);
                    remainingDecimals = buffer.Size() - readDigits;
                }
            }

            static bool DoubleStrtod(ArrayView<T> trimmed, int exponent, double& result)
            {
                static const double kExactPowersOfTen[] =
                {
                    1.0,  // 10^0
                    10.0,
                    100.0,
                    1000.0,
                    10000.0,
                    100000.0,
                    1000000.0,
                    10000000.0,
                    100000000.0,
                    1000000000.0,
                    10000000000.0,  // 10^10
                    100000000000.0,
                    1000000000000.0,
                    10000000000000.0,
                    100000000000000.0,
                    1000000000000000.0,
                    10000000000000000.0,
                    100000000000000000.0,
                    1000000000000000000.0,
                    10000000000000000000.0,
                    100000000000000000000.0,  // 10^20
                    1000000000000000000000.0,
                    // 10^22 = 0x21e19e0c9bab2400000 = 0x878678326eac9 * 2^22
                    10000000000000000000000.0
                };
                static const int kExactPowersOfTenSize = static_cast<int>(CountOf(kExactPowersOfTen));

                if (!kPlatformCorrectDoubleOperations)
                    return false;

                if (trimmed.Size() <= kMaxExactDoubleIntegerDecimalDigits)
                {
                    size_t readDigits;

                    // The trimmed input fits into a double.
                    // If the 10^exponent (resp. 10^-exponent) fits into a double too then we
                    // can compute the result-double simply by multiplying (resp. dividing) the
                    // two numbers.
                    // This is possible because IEEE guarantees that floating-point operations
                    // return the best possible approximation.
                    if (exponent < 0 && -exponent < kExactPowersOfTenSize)
                    {
                        // 10^-exponent fits into a double.
                        result = static_cast<double>(ReadUInt64(trimmed, readDigits));
                        assert(readDigits == trimmed.Size());
                        result /= kExactPowersOfTen[-exponent];
                        return true;
                    }

                    if (0 <= exponent && exponent < kExactPowersOfTenSize)
                    {
                        // 10^exponent fits into a double.
                        result = static_cast<double>(ReadUInt64(trimmed, readDigits));
                        assert(readDigits == trimmed.Size());
                        result *= kExactPowersOfTen[exponent];
                        return true;
                    }

                    int remainingDigits = kMaxExactDoubleIntegerDecimalDigits - trimmed.Size();
                    if ((0 <= exponent) && (exponent - remainingDigits < kExactPowersOfTenSize))
                    {
                        // The trimmed string was short and we can multiply it with
                        // 10^remaining_digits. As a result the remaining exponent now fits
                        // into a double too.
                        result = static_cast<double>(ReadUInt64(trimmed, readDigits));
                        assert(readDigits == trimmed.Size());
                        result *= kExactPowersOfTen[remainingDigits];
                        result *= kExactPowersOfTen[exponent - remainingDigits];
                        return true;
                    }
                }

                return false;
            }

            // Returns 10^exponent as an exact DiyFp.
            // The given exponent must be in the range [1; kDecimalExponentDistance[.
            static DiyFp AdjustmentPowerOfTen(int exponent)
            {
                assert(0 < exponent);
                assert(exponent < PowersOfTenCache::kDecimalExponentDistance);

                // Simply hardcode the remaining powers for the given decimal exponent
                // distance.
                assert(PowersOfTenCache::kDecimalExponentDistance == 8);

                switch (exponent)
                {
                    case 1:
                        return DiyFp(0xA000000000000000ull, -60);
                    case 2:
                        return DiyFp(0xC800000000000000ull, -57);
                    case 3:
                        return DiyFp(0xfA00000000000000ull, -54);
                    case 4:
                        return DiyFp(0x9C40000000000000ull, -50);
                    case 5:
                        return DiyFp(0xC350000000000000ull, -47);
                    case 6:
                        return DiyFp(0xF424000000000000ull, -44);
                    case 7:
                        return DiyFp(0x9896800000000000ull, -40);
                    default:
                        MOE_UNREACHABLE();
                }
            }

            // If the function returns true then the result is the correct double.
            // Otherwise it is either the correct double or the double that is just below
            // the correct double.
            static bool DiyFpStrtod(ArrayView<T> buffer, int exponent, double& result)
            {
                DiyFp input;
                size_t remainingDecimals;
                ReadDiyFp(buffer, input, remainingDecimals);

                // Since we may have dropped some digits the input is not accurate.
                // If remaining_decimals is different than 0 than the error is at most
                // .5 ulp (unit in the last place).
                // We don't want to deal with fractions and therefore keep a common
                // denominator.
                const uint64_t kDenominatorLog = 3;
                const uint64_t kDenominator = 1 << kDenominatorLog;

                // Move the remaining decimals into the exponent.
                exponent += static_cast<int>(remainingDecimals);
                uint64_t error = (remainingDecimals == 0u ? 0u : kDenominator / 2u);

                int oldE = input.Exponent();
                input.Normalize();
                error <<= oldE - input.Exponent();

                assert(exponent <= PowersOfTenCache::kMaxDecimalExponent);
                if (exponent < PowersOfTenCache::kMinDecimalExponent)
                {
                    result = 0.0;
                    return true;
                }

                DiyFp cachedPower;
                int cachedDecimalExponent;
                PowersOfTenCache::GetCachedPowerForDecimalExponent(exponent, cachedPower, cachedDecimalExponent);

                if (cachedDecimalExponent != exponent)
                {
                    int adjustmentExponent = exponent - cachedDecimalExponent;
                    DiyFp adjustmentPower = AdjustmentPowerOfTen(adjustmentExponent);
                    input.Multiply(adjustmentPower);

                    if (kMaxUInt64DecimalDigits - buffer.Size() >= adjustmentExponent)
                    {
                        // The product of input with the adjustment power fits into a 64 bit
                        // integer.
                        assert(DiyFp::kSignificandSize == 64);
                    }
                    else
                    {
                        // The adjustment power is exact. There is hence only an error of 0.5.
                        error += kDenominator / 2u;
                    }
                }

                input.Multiply(cachedPower);

                // The error introduced by a multiplication of a*b equals
                //   error_a + error_b + error_a*error_b/2^64 + 0.5
                // Substituting a with 'input' and b with 'cached_power' we have
                //   error_b = 0.5  (all cached powers have an error of less than 0.5 ulp),
                //   error_ab = 0 or 1 / kDenominator > error_a*error_b/ 2^64
                uint64_t errorB = kDenominator / 2;
                uint64_t errorAb = (error == 0 ? 0 : 1);  // We round up to 1.
                uint64_t fixedError = kDenominator / 2;
                error += errorB + errorAb + fixedError;

                oldE = input.Exponent();
                input.Normalize();
                error <<= oldE - input.Exponent();

                // See if the double's significand changes if we add/subtract the error.
                int orderOfMagnitude = DiyFp::kSignificandSize + input.Exponent();
                int effectiveSignificandSize = Double::SignificandSizeForOrderOfMagnitude(orderOfMagnitude);
                int precisionDigitsCount = DiyFp::kSignificandSize - effectiveSignificandSize;
                if (precisionDigitsCount + kDenominatorLog >= DiyFp::kSignificandSize)
                {
                    // This can only happen for very small denormals. In this case the
                    // half-way multiplied by the denominator exceeds the range of an uint64.
                    // Simply shift everything to the right.
                    int shiftAmount = (precisionDigitsCount + static_cast<int>(kDenominatorLog)) -
                        DiyFp::kSignificandSize + 1;
                    input.SetSignificand(input.Significand() >> shiftAmount);
                    input.SetExponent(input.Exponent() + shiftAmount);

                    // We add 1 for the lost precision of error, and kDenominator for
                    // the lost precision of input.f().
                    error = (error >> shiftAmount) + 1 + kDenominator;
                    precisionDigitsCount -= shiftAmount;
                }

                // We use uint64_ts now. This only works if the DiyFp uses uint64_ts too.
                assert(DiyFp::kSignificandSize == 64);
                assert(precisionDigitsCount < 64);

                uint64_t one64 = 1;
                uint64_t precisionBitsMask = (one64 << precisionDigitsCount) - 1;
                uint64_t precisionBits = input.Significand() & precisionBitsMask;
                uint64_t halfWay = one64 << (precisionDigitsCount - 1);
                precisionBits *= kDenominator;
                halfWay *= kDenominator;
                DiyFp roundedInput(input.Significand() >> precisionDigitsCount, input.Exponent() +
                    precisionDigitsCount);
                if (precisionBits >= halfWay + error)
                    roundedInput.SetSignificand(roundedInput.Significand() + 1);

                // If the last_bits are too close to the half-way case than we are too
                // inaccurate and round down. In this case we return false so that we can
                // fall back to a more precise algorithm.

                result = Double(roundedInput).ToDouble();
                if (halfWay - error < precisionBits && precisionBits < halfWay + error)
                {
                    // Too imprecise. The caller will have to fall back to a slower version.
                    // However the returned number is guaranteed to be either the correct
                    // double, or the next-lower double.
                    return false;
                }
                else
                    return true;
            }

            // Returns true if the guess is the correct double.
            // Returns false, when guess is either correct or the next-lower double.
            static bool ComputeGuess(ArrayView<T> trimmed, int exponent, double& guess)
            {
                if (trimmed.Size() == 0)
                {
                    guess = 0.0;
                    return true;
                }

                if (exponent + trimmed.Size() - 1 >= kMaxDecimalPower)
                {
                    guess = Double::Infinity();
                    return true;
                }

                if (exponent + trimmed.Size() <= kMinDecimalPower)
                {
                    guess = 0.0;
                    return true;
                }

                if (DoubleStrtod(trimmed, exponent, guess) || DiyFpStrtod(trimmed, exponent, guess))
                    return true;

                return guess == Double::Infinity();
            }

            // Returns
            //   - -1 if buffer*10^exponent < diy_fp.
            //   -  0 if buffer*10^exponent == diy_fp.
            //   - +1 if buffer*10^exponent > diy_fp.
            // Preconditions:
            //   buffer.length() + exponent <= kMaxDecimalPower + 1
            //   buffer.length() + exponent > kMinDecimalPower
            //   buffer.length() <= kMaxDecimalSignificantDigits
            static int CompareBufferWithDiyFp(ArrayView<T> buffer, int exponent, DiyFp diyFp)
            {
                assert(buffer.Size() + exponent <= kMaxDecimalPower + 1);
                assert(buffer.Size() + exponent > kMinDecimalPower);
                assert(buffer.Size() <= kMaxSignificantDecimalDigits);

                // Make sure that the Bignum will be able to hold all our numbers.
                // Our Bignum implementation has a separate field for exponents. Shifts will
                // consume at most one bigit (< 64 bits).
                // ln(10) == 3.3219...
                assert(((kMaxDecimalPower + 1) * 333 / 100) < Bignum::kMaxSignificantBits);
                Bignum bufferBignum;
                Bignum diyFpBignum;
                bufferBignum.AssignDecimalString(buffer);
                diyFpBignum.AssignUInt64(diyFp.Significand());
                if (exponent >= 0)
                    bufferBignum.MultiplyByPowerOfTen(exponent);
                else
                    diyFpBignum.MultiplyByPowerOfTen(-exponent);

                if (diyFp.Exponent() > 0)
                    diyFpBignum.ShiftLeft(diyFp.Exponent());
                else
                    bufferBignum.ShiftLeft(-diyFp.Exponent());

                return Bignum::Compare(bufferBignum, diyFpBignum);
            }

            // The buffer must only contain digits in the range [0-9]. It must not
            // contain a dot or a sign. It must not start with '0', and must not be empty.
            double Strtod(ArrayView<T> buffer, int exponent)
            {
                T copyBuffer[kMaxSignificantDecimalDigits];
                ArrayView<T> trimmed;
                int updatedExponent;
                TrimAndCut(buffer, exponent, copyBuffer, kMaxSignificantDecimalDigits, trimmed, updatedExponent);
                exponent = updatedExponent;

                double guess;
                bool isCorrect = ComputeGuess(trimmed, exponent, guess);
                if (isCorrect)
                    return guess;

                DiyFp upperBoundary = Double(guess).UpperBoundary();
                int comparison = CompareBufferWithDiyFp(trimmed, exponent, upperBoundary);
                if (comparison < 0)
                    return guess;
                else if (comparison > 0)
                    return Double(guess).NextDouble();
                else if ((Double(guess).Significand() & 1) == 0)
                    return guess;  // Round towards even.
                else
                    return Double(guess).NextDouble();
            }

            // The buffer must only contain digits in the range [0-9]. It must not
            // contain a dot or a sign. It must not start with '0', and must not be empty.
            float Strtof(ArrayView<T> buffer, int exponent)
            {
                T copyBuffer[kMaxSignificantDecimalDigits];
                ArrayView<T> trimmed;
                int updatedExponent;
                TrimAndCut(buffer, exponent, copyBuffer, kMaxSignificantDecimalDigits, trimmed, updatedExponent);
                exponent = updatedExponent;

                double doubleGuess;
                bool isCorrect = ComputeGuess(trimmed, exponent, doubleGuess);

                float floatGuess = static_cast<float>(doubleGuess);
                if (floatGuess == doubleGuess)
                    return floatGuess;  // This shortcut triggers for integer values.

                // We must catch double-rounding. Say the double has been rounded up, and is
                // now a boundary of a float, and rounds up again. This is why we have to
                // look at previous too.
                // Example (in decimal numbers):
                //    input: 12349
                //    high-precision (4 digits): 1235
                //    low-precision (3 digits):
                //       when read from input: 123
                //       when rounded from high precision: 124.
                // To do this we simply look at the neigbors of the correct result and see
                // if they would round to the same float. If the guess is not correct we have
                // to look at four values (since two different doubles could be the correct
                // double).

                double doubleNext = Double(doubleGuess).NextDouble();
                double doublePrevious = Double(doubleGuess).PreviousDouble();

                float f1 = static_cast<float>(doublePrevious);
                float f2 = floatGuess;
                float f3 = static_cast<float>(doubleNext);
                float f4;
                if (isCorrect)
                    f4 = f3;
                else
                {
                    double doubleNext2 = Double(doubleNext).NextDouble();
                    f4 = static_cast<float>(doubleNext2);
                }
                MOE_UNUSED(f2);
                assert(f1 <= f2 && f2 <= f3 && f3 <= f4);

                // If the guess doesn't lie near a single-precision boundary we can simply
                // return its float-value.
                if (f1 == f4)
                    return floatGuess;

                assert((f1 != f2 && f2 == f3 && f3 == f4) || (f1 == f2 && f2 != f3 && f3 == f4) ||
                    (f1 == f2 && f2 == f3 && f3 != f4));

                // guess and next are the two possible canditates (in the same way that
                // double_guess was the lower candidate for a double-precision guess).
                float guess = f1;
                float next = f4;
                DiyFp upperBoundary;
                if (guess == 0.0f)
                {
                    float minFloat = 1e-45f;
                    upperBoundary = Double(static_cast<double>(minFloat) / 2).ToDiyFp();
                }
                else
                    upperBoundary = Single(guess).UpperBoundary();

                int comparison = CompareBufferWithDiyFp(trimmed, exponent, upperBoundary);
                if (comparison < 0)
                    return guess;
                else if (comparison > 0)
                    return next;
                else if ((Single(guess).Significand() & 1) == 0)
                    return guess;  // Round towards even.
                else
                    return next;
            }

            static const size_t kMaxSignificantDigits = 772;

            static bool IsDigit(int x, int radix)
            {
                return (x >= '0' && x <= '9' && x < '0' + radix) ||
                    (radix > 10 && x >= 'a' && x < 'a' + radix - 10) ||
                    (radix > 10 && x >= 'A' && x < 'A' + radix - 10);
            }

            static bool IsCharacterDigitForRadix(int c, int radix, T aCharacter)
            {
                return radix > 10 && c >= aCharacter && c < aCharacter + radix - 10;
            }

            static bool IsDecimalDigitForRadix(int c, int radix)
            {
                return '0' <= c && c <= '9' && (c - '0') < radix;
            }

            static bool IsWhitespace(int x)
            {
                static const T kWhitespaceTable7[] = { 32, 13, 10, 9, 11, 12 };
                static const T kWhitespaceTable16[] =
                {
                    160, 8232, 8233, 5760, 6158, 8192, 8193, 8194, 8195, 8196, 8197, 8198, 8199, 8200, 8201, 8202, 8239,
                    8287, 12288, 65279
                };
                
                if (x < 128)
                {
                    for (size_t i = 0; i < CountOf(kWhitespaceTable7); ++i)
                    {
                        if (kWhitespaceTable7[i] == x)
                            return true;
                    }
                }
                else
                {
                    for (size_t i = 0; i < CountOf(kWhitespaceTable16); ++i)
                    {
                        if (kWhitespaceTable16[i] == x)
                            return true;
                    }
                }
                
                return false;
            }

            static double SignedZero(bool sign)
            {
                return sign ? -0.0 : 0.0;
            }

            template <class Iterator>
            static inline bool AdvanceToNonSpace(Iterator& current, Iterator end)
            {
                while (current != end)
                {
                    if (!IsWhitespace(*current))
                        return true;
                    ++current;
                }

                return false;
            }

            template <class Iterator>
            static bool ConsumeSubString(Iterator& current, Iterator end, const T* subString)
            {
                assert(*current == subString[0]);
                for (subString++; *subString != '\0'; subString++)
                {
                    ++current;
                    if (current == end || *current != *subString)
                        return false;
                }
                ++current;
                return true;
            }

            template <int RadixLog2, class Iterator>
            static double RadixStringToIeee(Iterator& current, Iterator end, bool sign, bool allowTrailingJunk,
                double junkStringValue, bool readAsDouble, bool& resultIsJunk)
            {
                assert(current != end);

                const int kDoubleSize = Double::kSignificandSize;
                const int kSingleSize = Single::kSignificandSize;
                const int kSignificandSize = readAsDouble ? kDoubleSize : kSingleSize;

                resultIsJunk = true;

                // Skip leading 0s.
                while (*current == '0')
                {
                    ++current;
                    if (current == end)
                    {
                        resultIsJunk = false;
                        return SignedZero(sign);
                    }
                }

                int64_t number = 0;
                int exponent = 0;
                const int radix = (1 << RadixLog2);

                do
                {
                    int digit;
                    if (IsDecimalDigitForRadix(*current, radix))
                        digit = static_cast<T>(*current) - '0';
                    else if (IsCharacterDigitForRadix(*current, radix, 'a'))
                        digit = static_cast<T>(*current) - 'a' + 10;
                    else if (IsCharacterDigitForRadix(*current, radix, 'A'))
                        digit = static_cast<T>(*current) - 'A' + 10;
                    else
                    {
                        if (allowTrailingJunk || !AdvanceToNonSpace(current, end))
                            break;
                        else
                            return junkStringValue;
                    }

                    number = number * radix + digit;
                    int overflow = static_cast<int>(number >> kSignificandSize);
                    if (overflow != 0)
                    {
                        // Overflow occurred. Need to determine which direction to round the
                        // result.
                        int overflowBitsCount = 1;
                        while (overflow > 1)
                        {
                            ++overflowBitsCount;
                            overflow >>= 1;
                        }

                        int droppedBitsMask = ((1 << overflowBitsCount) - 1);
                        int droppedBits = static_cast<int>(number) & droppedBitsMask;
                        number >>= overflowBitsCount;
                        exponent = overflowBitsCount;

                        bool zeroTail = true;
                        for (;;)
                        {
                            ++current;
                            if (current == end || !IsDigit(*current, radix))
                                break;
                            zeroTail = zeroTail && *current == '0';
                            exponent += RadixLog2;
                        }

                        if (!allowTrailingJunk && AdvanceToNonSpace(current, end))
                            return junkStringValue;

                        int middleValue = (1 << (overflowBitsCount - 1));
                        if (droppedBits > middleValue)
                            number++;  // Rounding up.
                        else if (droppedBits == middleValue)
                        {
                            // Rounding to even to consistency with decimals: half-way case rounds
                            // up if significant part is odd and down otherwise.
                            if ((number & 1) != 0 || !zeroTail)
                                number++;  // Rounding up.
                        }

                        // Rounding up may cause overflow.
                        if ((number & ((int64_t)1 << kSignificandSize)) != 0)
                        {
                            exponent++;
                            number >>= 1;
                        }

                        break;
                    }

                    ++current;
                }
                while (current != end);

                assert(number < ((int64_t)1 << kSignificandSize));
                assert(static_cast<int64_t>(static_cast<double>(number)) == number);

                resultIsJunk = false;

                if (exponent == 0)
                {
                    if (sign)
                    {
                        if (number == 0)
                            return -0.0;
                        number = -number;
                    }

                    return static_cast<double>(number);
                }

                assert(number != 0);
                return Double(DiyFp(static_cast<uint64_t>(number), exponent)).ToDouble();
            }

        public:
            // Flags should be a bit-or combination of the possible Flags-enum.
            //  - NO_FLAGS: no special flags.
            //  - ALLOW_HEX: recognizes the prefix "0x". Hex numbers may only be integers.
            //      Ex: StringToDouble("0x1234") -> 4660.0
            //          In StringToDouble("0x1234.56") the characters ".56" are trailing
            //          junk. The result of the call is hence dependent on
            //          the ALLOW_TRAILING_JUNK flag and/or the junk value.
            //      With this flag "0x" is a junk-string. Even with ALLOW_TRAILING_JUNK,
            //      the string will not be parsed as "0" followed by junk.
            //
            //  - ALLOW_OCTALS: recognizes the prefix "0" for octals:
            //      If a sequence of octal digits starts with '0', then the number is
            //      read as octal integer. Octal numbers may only be integers.
            //      Ex: StringToDouble("01234") -> 668.0
            //          StringToDouble("012349") -> 12349.0  // Not a sequence of octal
            //                                               // digits.
            //          In StringToDouble("01234.56") the characters ".56" are trailing
            //          junk. The result of the call is hence dependent on
            //          the ALLOW_TRAILING_JUNK flag and/or the junk value.
            //          In StringToDouble("01234e56") the characters "e56" are trailing
            //          junk, too.
            //  - ALLOW_TRAILING_JUNK: ignore trailing characters that are not part of
            //      a double literal.
            //  - ALLOW_LEADING_SPACES: skip over leading whitespace, including spaces,
            //                          new-lines, and tabs.
            //  - ALLOW_TRAILING_SPACES: ignore trailing whitespace.
            //  - ALLOW_SPACES_AFTER_SIGN: ignore whitespace after the sign.
            //       Ex: StringToDouble("-   123.2") -> -123.2.
            //           StringToDouble("+   123.2") -> 123.2
            //
            // empty_string_value is returned when an empty string is given as input.
            // If ALLOW_LEADING_SPACES or ALLOW_TRAILING_SPACES are set, then a string
            // containing only spaces is converted to the 'empty_string_value', too.
            //
            // junk_string_value is returned when
            //  a) ALLOW_TRAILING_JUNK is not set, and a junk character (a character not
            //     part of a double-literal) is found.
            //  b) ALLOW_TRAILING_JUNK is set, but the string does not start with a
            //     double literal.
            //
            // infinity_symbol and nan_symbol are strings that are used to detect
            // inputs that represent infinity and NaN. They can be null, in which case
            // they are ignored.
            // The conversion routine first reads any possible signs. Then it compares the
            // following character of the input-string with the first character of
            // the infinity, and nan-symbol. If either matches, the function assumes, that
            // a match has been found, and expects the following input characters to match
            // the remaining characters of the special-value symbol.
            // This means that the following restrictions apply to special-value symbols:
            //  - they must not start with signs ('+', or '-'),
            //  - they must not have the same first character.
            //  - they must not start with digits.
            //
            // Examples:
            //  flags = ALLOW_HEX | ALLOW_TRAILING_JUNK,
            //  empty_string_value = 0.0,
            //  junk_string_value = NaN,
            //  infinity_symbol = "infinity",
            //  nan_symbol = "nan":
            //    StringToDouble("0x1234") -> 4660.0.
            //    StringToDouble("0x1234K") -> 4660.0.
            //    StringToDouble("") -> 0.0  // empty_string_value.
            //    StringToDouble(" ") -> NaN  // junk_string_value.
            //    StringToDouble(" 1") -> NaN  // junk_string_value.
            //    StringToDouble("0x") -> NaN  // junk_string_value.
            //    StringToDouble("-123.45") -> -123.45.
            //    StringToDouble("--123.45") -> NaN  // junk_string_value.
            //    StringToDouble("123e45") -> 123e45.
            //    StringToDouble("123E45") -> 123e45.
            //    StringToDouble("123e+45") -> 123e45.
            //    StringToDouble("123E-45") -> 123e-45.
            //    StringToDouble("123e") -> 123.0  // trailing junk ignored.
            //    StringToDouble("123e-") -> 123.0  // trailing junk ignored.
            //    StringToDouble("+NaN") -> NaN  // NaN string literal.
            //    StringToDouble("-infinity") -> -inf.  // infinity literal.
            //    StringToDouble("Infinity") -> NaN  // junk_string_value.
            //
            //  flags = ALLOW_OCTAL | ALLOW_LEADING_SPACES,
            //  empty_string_value = 0.0,
            //  junk_string_value = NaN,
            //  infinity_symbol = NULL,
            //  nan_symbol = NULL:
            //    StringToDouble("0x1234") -> NaN  // junk_string_value.
            //    StringToDouble("01234") -> 668.0.
            //    StringToDouble("") -> 0.0  // empty_string_value.
            //    StringToDouble(" ") -> 0.0  // empty_string_value.
            //    StringToDouble(" 1") -> 1.0
            //    StringToDouble("0x") -> NaN  // junk_string_value.
            //    StringToDouble("0123e45") -> NaN  // junk_string_value.
            //    StringToDouble("01239E45") -> 1239e45.
            //    StringToDouble("-infinity") -> NaN  // junk_string_value.
            //    StringToDouble("NaN") -> NaN  // junk_string_value.
            StringToDoubleConverter(AtodFlags flags, double emptyStringValue, double junkStringValue,
                const char* infinitySymbol, const char* nanSymbol)
                : m_iFlags(flags), m_dEmptyStringValue(emptyStringValue), m_dJunkStringValue(junkStringValue),
                m_pszInfinitySymbol(infinitySymbol), m_pszNanSymbol(nanSymbol) {}

            // Performs the conversion.
            // The output parameter 'processed_characters_count' is set to the number
            // of characters that have been processed to read the number.
            // Spaces than are processed with ALLOW_{LEADING|TRAILING}_SPACES are included
            // in the 'processed_characters_count'. Trailing junk is never included.
            double StringToDouble(const T* buffer, size_t length, size_t& processedCharactersCount)const
            {
                return StringToIeee(buffer, length, true, processedCharactersCount);
            }

            // Same as StringToDouble but reads a float.
            // Note that this is not equivalent to static_cast<float>(StringToDouble(...))
            // due to potential double-rounding.
            float StringToFloat(const T* buffer, size_t length, size_t& processedCharactersCount)const
            {
                return static_cast<float>(StringToIeee(buffer, length, false, processedCharactersCount));
            }

        private:
            template <class Iterator>
            double StringToIeee(Iterator input, size_t length, bool readAsDouble, size_t& processedCharactersCount)const
            {
                Iterator current = input;
                Iterator end = input + length;

                processedCharactersCount = 0;

                const bool allowTrailingJunk = (m_iFlags & AtodFlags::AllowTrailingJunk) != 0;
                const bool allowLeadingSpaces = (m_iFlags & AtodFlags::AllowLeadingSpaces) != 0;
                const bool allowTrailingSpaces = (m_iFlags & AtodFlags::AllowTrailingSpaces) != 0;
                const bool allowSpacesAfterSign = (m_iFlags & AtodFlags::AllowSpacesAfterSign) != 0;

                // To make sure that iterator dereferencing is valid the following
                // convention is used:
                // 1. Each '++current' statement is followed by check for equality to 'end'.
                // 2. If AdvanceToNonSpace returned false then current == end.
                // 3. If 'current' becomes equal to 'end' the function returns or goes to
                // 'parsingDone'.
                // 4. 'current' is not dereferenced after the 'parsingDone' label.
                // 5. Code before 'parsingDone' may rely on 'current != end'.
                if (current == end)
                    return m_dEmptyStringValue;

                if (allowLeadingSpaces || allowTrailingSpaces)
                {
                    if (!AdvanceToNonSpace(current, end))
                    {
                        processedCharactersCount = static_cast<size_t>(current - input);
                        return m_dEmptyStringValue;
                    }

                    if (!allowLeadingSpaces && (input != current))
                    {
                        // No leading spaces allowed, but AdvanceToNonSpace moved forward.
                        return m_dJunkStringValue;
                    }
                }

                // The longest form of simplified number is: "-<significant digits>.1eXXX\0".
                const size_t kBufferSize = kMaxSignificantDigits + 10;
                T buffer[kBufferSize];  // NOLINT: size is known at compile time.
                size_t bufferPos = 0;

                // Exponent will be adjusted if insignificant digits of the integer part
                // or insignificant leading zeros of the fractional part are dropped.
                int exponent = 0;
                int significantDigits = 0;
                int insignificantDigits = 0;
                bool nonzeroDigitDropped = false;

                bool sign = false;

                if (*current == '+' || *current == '-')
                {
                    sign = (*current == '-');
                    ++current;
                    Iterator nextNonSpace = current;

                    // Skip following spaces (if allowed).
                    if (!AdvanceToNonSpace(nextNonSpace, end))
                        return m_dJunkStringValue;
                    if (!allowSpacesAfterSign && (current != nextNonSpace))
                        return m_dJunkStringValue;
                    current = nextNonSpace;
                }

                if (m_pszInfinitySymbol != nullptr)
                {
                    if (*current == m_pszInfinitySymbol[0])
                    {
                        if (!ConsumeSubString(current, end, m_pszInfinitySymbol))
                            return m_dJunkStringValue;

                        if (!(allowTrailingSpaces || allowTrailingJunk) && (current != end))
                            return m_dJunkStringValue;

                        if (!allowTrailingJunk && AdvanceToNonSpace(current, end))
                            return m_dJunkStringValue;

                        assert(bufferPos == 0);
                        processedCharactersCount = static_cast<size_t>(current - input);
                        return sign ? -Double::Infinity() : Double::Infinity();
                    }
                }

                if (m_pszNanSymbol != nullptr)
                {
                    if (*current == m_pszNanSymbol[0])
                    {
                        if (!ConsumeSubString(current, end, m_pszNanSymbol))
                            return m_dJunkStringValue;

                        if (!(allowTrailingSpaces || allowTrailingJunk) && (current != end))
                            return m_dJunkStringValue;

                        if (!allowTrailingJunk && AdvanceToNonSpace(current, end))
                            return m_dJunkStringValue;

                        assert(bufferPos == 0);
                        processedCharactersCount = static_cast<size_t>(current - input);
                        return sign ? -Double::Nan() : Double::Nan();
                    }
                }

                bool leadingZero = false;
                if (*current == '0')
                {
                    ++current;
                    if (current == end)
                    {
                        processedCharactersCount = static_cast<size_t>(current - input);
                        return SignedZero(sign);
                    }

                    leadingZero = true;

                    // It could be hexadecimal value.
                    if ((m_iFlags & AtodFlags::AllowHex) && (*current == 'x' || *current == 'X'))
                    {
                        ++current;
                        if (current == end || !IsDigit(*current, 16))
                            return m_dJunkStringValue;  // "0x".

                        bool resultIsJunk;
                        double result = RadixStringToIeee<4>(current, end, sign, allowTrailingJunk, m_dJunkStringValue,
                            readAsDouble, resultIsJunk);
                        if (!resultIsJunk)
                        {
                            if (allowTrailingSpaces)
                                AdvanceToNonSpace(current, end);
                            processedCharactersCount = static_cast<size_t>(current - input);
                        }

                        return result;
                    }

                    // Ignore leading zeros in the integer part.
                    while (*current == '0')
                    {
                        ++current;
                        if (current == end)
                        {
                            processedCharactersCount = static_cast<size_t>(current - input);
                            return SignedZero(sign);
                        }
                    }
                }

                bool octal = leadingZero && (m_iFlags & AtodFlags::AllowOctals) != 0;

                // Copy significant digits of the integer part (if any) to the buffer.
                while (*current >= '0' && *current <= '9')
                {
                    if (significantDigits < kMaxSignificantDigits)
                    {
                        assert(bufferPos < kBufferSize);
                        buffer[bufferPos++] = static_cast<T>(*current);
                        significantDigits++;
                        // Will later check if it's an octal in the buffer.
                    }
                    else
                    {
                        insignificantDigits++;  // Move the digit into the exponential part.
                        nonzeroDigitDropped = nonzeroDigitDropped || *current != '0';
                    }

                    octal = octal && *current < '8';
                    ++current;
                    if (current == end)
                        goto parsingDone;
                }

                if (significantDigits == 0)
                    octal = false;

                if (*current == '.')
                {
                    if (octal && !allowTrailingJunk)
                        return m_dJunkStringValue;
                    if (octal)
                        goto parsingDone;

                    ++current;
                    if (current == end)
                    {
                        if (significantDigits == 0 && !leadingZero)
                            return m_dJunkStringValue;
                        else
                            goto parsingDone;
                    }

                    if (significantDigits == 0)
                    {
                        // octal = false;
                        // Integer part consists of 0 or is absent. Significant digits start after
                        // leading zeros (if any).
                        while (*current == '0')
                        {
                            ++current;
                            if (current == end)
                            {
                                processedCharactersCount = static_cast<size_t>(current - input);
                                return SignedZero(sign);
                            }

                            exponent--;  // Move this 0 into the exponent.
                        }
                    }

                    // There is a fractional part.
                    // We don't emit a '.', but adjust the exponent instead.
                    while (*current >= '0' && *current <= '9')
                    {
                        if (significantDigits < kMaxSignificantDigits)
                        {
                            assert(bufferPos < kBufferSize);
                            buffer[bufferPos++] = static_cast<T>(*current);
                            ++significantDigits;
                            --exponent;
                        }
                        else
                        {
                            // Ignore insignificant digits in the fractional part.
                            nonzeroDigitDropped = nonzeroDigitDropped || *current != '0';
                        }

                        ++current;
                        if (current == end)
                            goto parsingDone;
                    }
                }

                if (!leadingZero && exponent == 0 && significantDigits == 0)
                {
                    // If leading_zeros is true then the string contains zeros.
                    // If exponent < 0 then string was [+-]\.0*...
                    // If significant_digits != 0 the string is not equal to 0.
                    // Otherwise there are no digits in the string.
                    return m_dJunkStringValue;
                }

                // Parse exponential part.
                if (*current == 'e' || *current == 'E')
                {
                    if (octal && !allowTrailingJunk)
                        return m_dJunkStringValue;
                    if (octal)
                        goto parsingDone;
                    ++current;
                    if (current == end)
                    {
                        if (allowTrailingJunk)
                            goto parsingDone;
                        else
                            return m_dJunkStringValue;
                    }
                    
                    T exponenSign = '+';
                    if (*current == '+' || *current == '-')
                    {
                        exponenSign = static_cast<T>(*current);
                        ++current;
                        if (current == end)
                        {
                            if (allowTrailingJunk)
                                goto parsingDone;
                            else
                                return m_dJunkStringValue;
                        }
                    }

                    if (current == end || *current < '0' || *current > '9')
                    {
                        if (allowTrailingJunk)
                            goto parsingDone;
                        else
                            return m_dJunkStringValue;
                    }

                    const int maxExponent = std::numeric_limits<int>::max() / 2;
                    assert(-maxExponent / 2 <= exponent && exponent <= maxExponent / 2);
                    int num = 0;
                    do
                    {
                        // Check overflow.
                        int digit = *current - '0';
                        if (num >= maxExponent / 10 && !(num == maxExponent / 10 && digit <= maxExponent % 10))
                            num = maxExponent;
                        else
                            num = num * 10 + digit;
                        ++current;
                    }
                    while (current != end && *current >= '0' && *current <= '9');

                    exponent += (exponenSign == '-' ? -num : num);
                }

                if (!(allowTrailingSpaces || allowTrailingJunk) && (current != end))
                    return m_dJunkStringValue;
                
                if (!allowTrailingJunk && AdvanceToNonSpace(current, end))
                    return m_dJunkStringValue;
                
                if (allowTrailingSpaces)
                    AdvanceToNonSpace(current, end);

            parsingDone:
                exponent += insignificantDigits;

                if (octal)
                {
                    double result;
                    bool resultIsJunk;
                    T* start = buffer;
                    result = RadixStringToIeee<3>(start, buffer + bufferPos, sign, allowTrailingJunk,
                        m_dJunkStringValue, readAsDouble, resultIsJunk);
                    assert(!resultIsJunk);
                    processedCharactersCount = static_cast<size_t>(current - input);
                    return result;
                }

                if (nonzeroDigitDropped)
                {
                    buffer[bufferPos++] = '1';
                    --exponent;
                }

                assert(bufferPos < kBufferSize);
                buffer[bufferPos] = '\0';

                double converted;
                if (readAsDouble)
                    converted = Strtod(ArrayView<T>(buffer, bufferPos), exponent);
                else
                    converted = Strtof(ArrayView<T>(buffer, bufferPos), exponent);

                processedCharactersCount = static_cast<size_t>(current - input);
                return sign ? -converted : converted;
            }

        private:
            const AtodFlags m_iFlags;
            const double m_dEmptyStringValue;
            const double m_dJunkStringValue;
            const T* m_pszInfinitySymbol;
            const T* m_pszNanSymbol;
        };
    }
}

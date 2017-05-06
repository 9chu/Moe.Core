/**
 * @file
 * @date 2017/5/6
 * @see https://github.com/google/double-conversion
 */
#include <Moe.Core/Algorithm/Dtoa.hpp>

#include <cmath>
#include <algorithm>

using namespace std;
using namespace moe;
using namespace internal;

void DiyFp::Multiply(const DiyFp& other)
{
    // Simply "emulates" a 128 bit multiplication.
    // However: the resulting number only contains 64 bits. The least
    // significant 64 bits are only used for rounding the most significant 64
    // bits.
    const uint64_t kM32 = 0xFFFFFFFFU;
    uint64_t a = m_ulSignificand >> 32;
    uint64_t b = m_ulSignificand & kM32;
    uint64_t c = other.m_ulSignificand >> 32;
    uint64_t d = other.m_ulSignificand & kM32;
    uint64_t ac = a * c;
    uint64_t bc = b * c;
    uint64_t ad = a * d;
    uint64_t bd = b * d;
    uint64_t tmp = (bd >> 32) + (ad & kM32) + (bc & kM32);
    // By adding 1U << 31 to tmp we round the final result.
    // Halfway cases will be round up.
    tmp += 1U << 31;
    uint64_t result_f = ac + (ad >> 32) + (bc >> 32) + (tmp >> 32);
    m_iExponent += other.m_iExponent + 64;
    m_ulSignificand = result_f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CachedPower
{
    uint64_t significand;
    int16_t binaryExponent;
    int16_t decimalExponent;
};

static const CachedPower kCachedPowers[] =
{
    { 0xfa8fd5a0081c0288ull, -1220, -348 },
    { 0xbaaee17fa23ebf76ull, -1193, -340 },
    { 0x8b16fb203055ac76ull, -1166, -332 },
    { 0xcf42894a5dce35eaull, -1140, -324 },
    { 0x9a6bb0aa55653b2dull, -1113, -316 },
    { 0xe61acf033d1a45dfull, -1087, -308 },
    { 0xab70fe17c79ac6caull, -1060, -300 },
    { 0xff77b1fcbebcdc4full, -1034, -292 },
    { 0xbe5691ef416bd60cull, -1007, -284 },
    { 0x8dd01fad907ffc3cull, -980, -276 },
    { 0xd3515c2831559a83ull, -954, -268 },
    { 0x9d71ac8fada6c9b5ull, -927, -260 },
    { 0xea9c227723ee8bcbull, -901, -252 },
    { 0xaecc49914078536dull, -874, -244 },
    { 0x823c12795db6ce57ull, -847, -236 },
    { 0xc21094364dfb5637ull, -821, -228 },
    { 0x9096ea6f3848984full, -794, -220 },
    { 0xd77485cb25823ac7ull, -768, -212 },
    { 0xa086cfcd97bf97f4ull, -741, -204 },
    { 0xef340a98172aace5ull, -715, -196 },
    { 0xb23867fb2a35b28eull, -688, -188 },
    { 0x84c8d4dfd2c63f3bull, -661, -180 },
    { 0xc5dd44271ad3cdbaull, -635, -172 },
    { 0x936b9fcebb25c996ull, -608, -164 },
    { 0xdbac6c247d62a584ull, -582, -156 },
    { 0xa3ab66580d5fdaf6ull, -555, -148 },
    { 0xf3e2f893dec3f126ull, -529, -140 },
    { 0xb5b5ada8aaff80b8ull, -502, -132 },
    { 0x87625f056c7c4a8bull, -475, -124 },
    { 0xc9bcff6034c13053ull, -449, -116 },
    { 0x964e858c91ba2655ull, -422, -108 },
    { 0xdff9772470297ebdull, -396, -100 },
    { 0xa6dfbd9fb8e5b88full, -369, -92 },
    { 0xf8a95fcf88747d94ull, -343, -84 },
    { 0xb94470938fa89bcfull, -316, -76 },
    { 0x8a08f0f8bf0f156bull, -289, -68 },
    { 0xcdb02555653131b6ull, -263, -60 },
    { 0x993fe2c6d07b7facull, -236, -52 },
    { 0xe45c10c42a2b3b06ull, -210, -44 },
    { 0xaa242499697392d3ull, -183, -36 },
    { 0xfd87b5f28300ca0eull, -157, -28 },
    { 0xbce5086492111aebull, -130, -20 },
    { 0x8cbccc096f5088ccull, -103, -12 },
    { 0xd1b71758e219652cull, -77, -4 },
    { 0x9c40000000000000ull, -50, 4 },
    { 0xe8d4a51000000000ull, -24, 12 },
    { 0xad78ebc5ac620000ull, 3, 20 },
    { 0x813f3978f8940984ull, 30, 28 },
    { 0xc097ce7bc90715b3ull, 56, 36 },
    { 0x8f7e32ce7bea5c70ull, 83, 44 },
    { 0xd5d238a4abe98068ull, 109, 52 },
    { 0x9f4f2726179a2245ull, 136, 60 },
    { 0xed63a231d4c4fb27ull, 162, 68 },
    { 0xb0de65388cc8ada8ull, 189, 76 },
    { 0x83c7088e1aab65dbull, 216, 84 },
    { 0xc45d1df942711d9aull, 242, 92 },
    { 0x924d692ca61be758ull, 269, 100 },
    { 0xda01ee641a708deaull, 295, 108 },
    { 0xa26da3999aef774aull, 322, 116 },
    { 0xf209787bb47d6b85ull, 348, 124 },
    { 0xb454e4a179dd1877ull, 375, 132 },
    { 0x865b86925b9bc5c2ull, 402, 140 },
    { 0xc83553c5c8965d3dull, 428, 148 },
    { 0x952ab45cfa97a0b3ull, 455, 156 },
    { 0xde469fbd99a05fe3ull, 481, 164 },
    { 0xa59bc234db398c25ull, 508, 172 },
    { 0xf6c69a72a3989f5cull, 534, 180 },
    { 0xb7dcbf5354e9beceull, 561, 188 },
    { 0x88fcf317f22241e2ull, 588, 196 },
    { 0xcc20ce9bd35c78a5ull, 614, 204 },
    { 0x98165af37b2153dfull, 641, 212 },
    { 0xe2a0b5dc971f303aull, 667, 220 },
    { 0xa8d9d1535ce3b396ull, 694, 228 },
    { 0xfb9b7cd9a4a7443cull, 720, 236 },
    { 0xbb764c4ca7a44410ull, 747, 244 },
    { 0x8bab8eefb6409c1aull, 774, 252 },
    { 0xd01fef10a657842cull, 800, 260 },
    { 0x9b10a4e5e9913129ull, 827, 268 },
    { 0xe7109bfba19c0c9dull, 853, 276 },
    { 0xac2820d9623bf429ull, 880, 284 },
    { 0x80444b5e7aa7cf85ull, 907, 292 },
    { 0xbf21e44003acdd2dull, 933, 300 },
    { 0x8e679c2f5e44ff8full, 960, 308 },
    { 0xd433179d9c8cb841ull, 986, 316 },
    { 0x9e19db92b4e31ba9ull, 1013, 324 },
    { 0xeb96bf6ebadf77d9ull, 1039, 332 },
    { 0xaf87023b9bf0ee6bull, 1066, 340 },
};

static const int kCachedPowersOffset = 348;  // -1 * the first decimal_exponent.
static const double kD_1_LOG2_10 = 0.30102999566398114;  //  1 / lg(10)

// Difference between the decimal exponents in the table above.
const int PowersOfTenCache::kDecimalExponentDistance = 8;
const int PowersOfTenCache::kMinDecimalExponent = -348;
const int PowersOfTenCache::kMaxDecimalExponent = 340;

void PowersOfTenCache::GetCachedPowerForBinaryExponentRange(int minExponent, int maxExponent, DiyFp* power,
    int* decimalExponent)
{
    int kQ = DiyFp::kSignificandSize;
    double k = std::ceil((minExponent + kQ - 1) * kD_1_LOG2_10);
    int foo = kCachedPowersOffset;
    int index = (foo + static_cast<int>(k) - 1) / kDecimalExponentDistance + 1;
    assert(0 <= index && index < static_cast<int>(CountOf(kCachedPowers)));
    CachedPower cachedPower = kCachedPowers[index];
    assert(minExponent <= cachedPower.binaryExponent);
    MOE_UNUSED(maxExponent);
    assert(cachedPower.binaryExponent <= maxExponent);
    *decimalExponent = cachedPower.decimalExponent;
    *power = DiyFp(cachedPower.significand, cachedPower.binaryExponent);
}

void PowersOfTenCache::GetCachedPowerForDecimalExponent(int requestedExponent, DiyFp* power, int* foundExponent)
{
    assert(kMinDecimalExponent <= requestedExponent);
    assert(requestedExponent < kMaxDecimalExponent + kDecimalExponentDistance);
    int index = (requestedExponent + kCachedPowersOffset) / kDecimalExponentDistance;
    CachedPower cachedPower = kCachedPowers[index];
    *power = DiyFp(cachedPower.significand, cachedPower.binaryExponent);
    *foundExponent = cachedPower.decimalExponent;
    assert(*foundExponent <= requestedExponent);
    assert(requestedExponent < *foundExponent + kDecimalExponentDistance);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Bignum::Compare(const Bignum& a, const Bignum& b)
{
    assert(a.IsClamped());
    assert(b.IsClamped());

    int bigitLengthA = a.BigitLength();
    int bigitLengthB = b.BigitLength();
    if (bigitLengthA < bigitLengthB)
        return -1;
    if (bigitLengthA > bigitLengthB)
        return +1;

    for (int i = bigitLengthA - 1; i >= std::min(a.m_iExponent, b.m_iExponent); --i)
    {
        Chunk bigitA = a.BigitAt(i);
        Chunk bigitB = b.BigitAt(i);
        if (bigitA < bigitB)
            return -1;
        if (bigitA > bigitB)
            return +1;
    }

    return 0;
}

int Bignum::PlusCompare(const Bignum& a, const Bignum& b, const Bignum& c)
{
    assert(a.IsClamped());
    assert(b.IsClamped());
    assert(c.IsClamped());

    if (a.BigitLength() < b.BigitLength())
        return PlusCompare(b, a, c);
    if (a.BigitLength() + 1 < c.BigitLength())
        return -1;
    if (a.BigitLength() > c.BigitLength())
        return +1;
    if (a.m_iExponent >= b.BigitLength() && a.BigitLength() < c.BigitLength())
        return -1;

    Chunk borrow = 0;
    int minExponent = std::min(std::min(a.m_iExponent, b.m_iExponent), c.m_iExponent);
    for (int i = c.BigitLength() - 1; i >= minExponent; --i)
    {
        Chunk chunkA = a.BigitAt(i);
        Chunk chunkB = b.BigitAt(i);
        Chunk chunkC = c.BigitAt(i);
        Chunk sum = chunkA + chunkB;
        if (sum > chunkC + borrow)
            return +1;
        else
        {
            borrow = chunkC + borrow - sum;
            if (borrow > 1)
                return -1;
            borrow <<= kBigitSize;
        }
    }

    if (borrow == 0)
        return 0;
    return -1;
}

Bignum::Bignum()
    : m_stBigits(m_aBigitsBuffer, kBigitCapacity), m_uUsedDigits(0), m_iExponent(0)
{
    for (size_t i = 0; i < kBigitCapacity; ++i)
        m_stBigits[i] = 0;
}

void Bignum::AssignUInt16(uint16_t value)
{
    assert(kBigitSize >= BitSize(value));
    Zero();
    if (value == 0)
        return;

    EnsureCapacity(1);
    m_stBigits[0] = value;
    m_uUsedDigits = 1;
}

void Bignum::AssignUInt64(uint64_t value)
{
    const int kUInt64Size = 64;

    Zero();
    if (value == 0)
        return;

    size_t neededBigits = kUInt64Size / kBigitSize + 1;
    EnsureCapacity(neededBigits);
    for (size_t i = 0; i < neededBigits; ++i)
    {
        m_stBigits[i] = static_cast<Chunk>(value & kBigitMask);
        value = value >> kBigitSize;
    }

    m_uUsedDigits = neededBigits;
    Clamp();
}

void Bignum::AssignBignum(const Bignum& other)
{
    m_iExponent = other.m_iExponent;
    for (size_t i = 0; i < other.m_uUsedDigits; ++i)
        m_stBigits[i] = other.m_stBigits[i];

    for (size_t i = other.m_uUsedDigits; i < m_uUsedDigits; ++i)
        m_stBigits[i] = 0;

    m_uUsedDigits = other.m_uUsedDigits;
}

void Bignum::AssignPowerUInt16(uint16_t base, int powerExponent)
{
    assert(base != 0);
    assert(powerExponent >= 0);
    if (powerExponent == 0)
    {
        AssignUInt16(1);
        return;
    }
    
    Zero();
    int shifts = 0;
    while ((base & 1) == 0)
    {
        base >>= 1;
        shifts++;
    }
    
    int bitSize = 0;
    int tmpBase = base;
    while (tmpBase != 0)
    {
        tmpBase >>= 1;
        bitSize++;
    }
    
    int finalSize = bitSize * powerExponent;
    EnsureCapacity(finalSize / kBigitSize + 2);

    int mask = 1;
    while (powerExponent >= mask)
        mask <<= 1;

    mask >>= 2;
    uint64_t thisValue = base;

    bool delayedMultipliciation = false;
    const uint64_t max32bits = 0xFFFFFFFF;
    while (mask != 0 && thisValue <= max32bits)
    {
        thisValue = thisValue * thisValue;
        if ((powerExponent & mask) != 0)
        {
            uint64_t baseBitsMask = ~((static_cast<uint64_t>(1) << (64 - bitSize)) - 1);
            bool highBitsZero = (thisValue & baseBitsMask) == 0;
            if (highBitsZero)
                thisValue *= base;
            else
                delayedMultipliciation = true;
        }
        mask >>= 1;
    }
    AssignUInt64(thisValue);
    if (delayedMultipliciation)
        MultiplyByUInt32(base);

    while (mask != 0)
    {
        Square();
        if ((powerExponent & mask) != 0)
            MultiplyByUInt32(base);
        mask >>= 1;
    }

    ShiftLeft(shifts * powerExponent);
}

void Bignum::AddUInt64(uint64_t operand)
{
    if (operand == 0)
        return;
    Bignum other;
    other.AssignUInt64(operand);
    AddBignum(other);
}

void Bignum::AddBignum(const Bignum& other)
{
    assert(IsClamped());
    assert(other.IsClamped());

    Align(other);

    EnsureCapacity(static_cast<size_t>(1 + std::max(BigitLength(), other.BigitLength()) - m_iExponent));
    Chunk carry = 0;
    int bigitPos = other.m_iExponent - m_iExponent;
    assert(bigitPos >= 0);
    for (size_t i = 0; i < other.m_uUsedDigits; ++i)
    {
        Chunk sum = m_stBigits[bigitPos] + other.m_stBigits[i] + carry;
        m_stBigits[bigitPos] = sum & kBigitMask;
        carry = sum >> kBigitSize;
        bigitPos++;
    }

    while (carry != 0)
    {
        Chunk sum = m_stBigits[bigitPos] + carry;
        m_stBigits[bigitPos] = sum & kBigitMask;
        carry = sum >> kBigitSize;
        bigitPos++;
    }
    
    m_uUsedDigits = std::max(static_cast<size_t>(bigitPos), m_uUsedDigits);
    assert(IsClamped());
}

void Bignum::SubtractBignum(const Bignum& other)
{
    assert(IsClamped());
    assert(other.IsClamped());
    assert(LessEqual(other, *this));

    Align(other);

    int offset = other.m_iExponent - m_iExponent;
    Chunk borrow = 0;
    size_t i;
    for (i = 0; i < other.m_uUsedDigits; ++i)
    {
        assert((borrow == 0) || (borrow == 1));
        Chunk difference = m_stBigits[i + offset] - other.m_stBigits[i] - borrow;
        m_stBigits[i + offset] = difference & kBigitMask;
        borrow = difference >> (kChunkSize - 1);
    }
    while (borrow != 0)
    {
        Chunk difference = m_stBigits[i + offset] - borrow;
        m_stBigits[i + offset] = difference & kBigitMask;
        borrow = difference >> (kChunkSize - 1);
        ++i;
    }
    Clamp();
}

void Bignum::Square()
{
    assert(IsClamped());
    size_t productLength = 2 * m_uUsedDigits;
    EnsureCapacity(productLength);

    if ((1 << (2 * (kChunkSize - kBigitSize))) <= m_uUsedDigits)
        MOE_UNREACHABLE();
    
    DoubleChunk accumulator = 0;
    int copyOffset = m_uUsedDigits;
    for (size_t i = 0; i < m_uUsedDigits; ++i)
        m_stBigits[copyOffset + i] = m_stBigits[i];
    
    for (size_t i = 0; i < m_uUsedDigits; ++i)
    {
        int bigitIndex1 = static_cast<int>(i);
        size_t bigitIndex2 = 0;
        
        while (bigitIndex1 >= 0)
        {
            Chunk chunk1 = m_stBigits[copyOffset + bigitIndex1];
            Chunk chunk2 = m_stBigits[copyOffset + bigitIndex2];
            accumulator += static_cast<DoubleChunk>(chunk1) * chunk2;
            bigitIndex1--;
            bigitIndex2++;
        }

        m_stBigits[i] = static_cast<Chunk>(accumulator) & kBigitMask;
        accumulator >>= kBigitSize;
    }
    
    for (size_t i = m_uUsedDigits; i < productLength; ++i)
    {
        size_t bigitIndex1 = m_uUsedDigits - 1;
        size_t bigitIndex2 = i - bigitIndex1;
        
        while (bigitIndex2 < m_uUsedDigits)
        {
            Chunk chunk1 = m_stBigits[copyOffset + bigitIndex1];
            Chunk chunk2 = m_stBigits[copyOffset + bigitIndex2];
            accumulator += static_cast<DoubleChunk>(chunk1) * chunk2;
            bigitIndex1--;
            bigitIndex2++;
        }
        
        m_stBigits[i] = static_cast<Chunk>(accumulator) & kBigitMask;
        accumulator >>= kBigitSize;
    }
    
    assert(accumulator == 0);

    m_uUsedDigits = productLength;
    m_iExponent *= 2;
    Clamp();
}

void Bignum::ShiftLeft(int shiftAmount)
{
    if (m_uUsedDigits == 0)
        return;
    m_iExponent += shiftAmount / kBigitSize;
    int local_shift = shiftAmount % kBigitSize;
    EnsureCapacity(m_uUsedDigits + 1);
    BigitsShiftLeft(local_shift);
}

void Bignum::MultiplyByUInt32(uint32_t factor)
{
    if (factor == 1)
        return;
    if (factor == 0)
    {
        Zero();
        return;
    }
    if (m_uUsedDigits == 0)
        return;

    assert(kDoubleChunkSize >= kBigitSize + 32 + 1);
    DoubleChunk carry = 0;
    for (size_t i = 0; i < m_uUsedDigits; ++i)
    {
        DoubleChunk product = static_cast<DoubleChunk>(factor) * m_stBigits[i] + carry;
        m_stBigits[i] = static_cast<Chunk>(product & kBigitMask);
        carry = (product >> kBigitSize);
    }

    while (carry != 0)
    {
        EnsureCapacity(m_uUsedDigits + 1);
        m_stBigits[m_uUsedDigits] = static_cast<Chunk>(carry & kBigitMask);
        m_uUsedDigits++;
        carry >>= kBigitSize;
    }
}

void Bignum::MultiplyByUInt64(uint64_t factor)
{
    if (factor == 1)
        return;
    if (factor == 0)
    {
        Zero();
        return;
    }

    assert(kBigitSize < 32);
    uint64_t carry = 0;
    uint64_t low = factor & 0xFFFFFFFF;
    uint64_t high = factor >> 32;
    for (size_t i = 0; i < m_uUsedDigits; ++i)
    {
        uint64_t productLow = low * m_stBigits[i];
        uint64_t productHigh = high * m_stBigits[i];
        uint64_t tmp = (carry & kBigitMask) + productLow;
        m_stBigits[i] = static_cast<Chunk>(tmp & kBigitMask);
        carry = (carry >> kBigitSize) + (tmp >> kBigitSize) + (productHigh << (32 - kBigitSize));
    }

    while (carry != 0)
    {
        EnsureCapacity(m_uUsedDigits + 1);
        m_stBigits[m_uUsedDigits] = static_cast<Chunk>(carry & kBigitMask);
        m_uUsedDigits++;
        carry >>= kBigitSize;
    }
}

void Bignum::MultiplyByPowerOfTen(int exponent)
{
    const uint64_t kFive27 = 0x6765C793FA10079Dull;
    const uint16_t kFive1 = 5;
    const uint16_t kFive2 = kFive1 * 5;
    const uint16_t kFive3 = kFive2 * 5;
    const uint16_t kFive4 = kFive3 * 5;
    const uint16_t kFive5 = kFive4 * 5;
    const uint16_t kFive6 = kFive5 * 5;
    const uint32_t kFive7 = static_cast<uint32_t>(kFive6 * 5);
    const uint32_t kFive8 = kFive7 * 5;
    const uint32_t kFive9 = kFive8 * 5;
    const uint32_t kFive10 = kFive9 * 5;
    const uint32_t kFive11 = kFive10 * 5;
    const uint32_t kFive12 = kFive11 * 5;
    const uint32_t kFive13 = kFive12 * 5;
    const uint32_t kFive1To12[] =
    {
        kFive1, kFive2, kFive3, kFive4, kFive5, kFive6, kFive7, kFive8, kFive9, kFive10, kFive11, kFive12
    };

    assert(exponent >= 0);
    if (exponent == 0)
        return;
    if (m_uUsedDigits == 0)
        return;

    int remainingExponent = exponent;
    while (remainingExponent >= 27)
    {
        MultiplyByUInt64(kFive27);
        remainingExponent -= 27;
    }

    while (remainingExponent >= 13)
    {
        MultiplyByUInt32(kFive13);
        remainingExponent -= 13;
    }

    if (remainingExponent > 0)
        MultiplyByUInt32(kFive1To12[remainingExponent - 1]);

    ShiftLeft(exponent);
}

uint16_t Bignum::DivideModuloIntBignum(const Bignum& other)
{
    assert(IsClamped());
    assert(other.IsClamped());
    assert(other.m_uUsedDigits > 0);

    if (BigitLength() < other.BigitLength())
        return 0;

    Align(other);

    uint16_t result = 0;

    while (BigitLength() > other.BigitLength())
    {
        assert(other.m_stBigits[other.m_uUsedDigits - 1] >= ((1 << kBigitSize) / 16));
        assert(m_stBigits[m_uUsedDigits - 1] < 0x10000);

        result += static_cast<uint16_t>(m_stBigits[m_uUsedDigits - 1]);
        SubtractTimes(other, m_stBigits[m_uUsedDigits - 1]);
    }

    assert(BigitLength() == other.BigitLength());

    Chunk thisBigit = m_stBigits[m_uUsedDigits - 1];
    Chunk otherBigit = other.m_stBigits[other.m_uUsedDigits - 1];

    if (other.m_uUsedDigits == 1)
    {
        int quotient = thisBigit / otherBigit;
        m_stBigits[m_uUsedDigits - 1] = thisBigit - otherBigit * quotient;
        assert(quotient < 0x10000);
        result += static_cast<uint16_t>(quotient);
        Clamp();
        return result;
    }

    int divisionEstimate = thisBigit / (otherBigit + 1);
    assert(divisionEstimate < 0x10000);
    result += static_cast<uint16_t>(divisionEstimate);
    SubtractTimes(other, divisionEstimate);

    if (otherBigit * (divisionEstimate + 1) > thisBigit)
        return result;

    while (LessEqual(other, *this))
    {
        SubtractBignum(other);
        result++;
    }
    return result;
}

void Bignum::Align(const Bignum& other)
{
    if (m_iExponent > other.m_iExponent)
    {
        int zeroDigits = m_iExponent - other.m_iExponent;
        EnsureCapacity(m_uUsedDigits + zeroDigits);

        // for (int i = m_uUsedDigits - 1; i >= 0; --i)
        for (size_t i = m_uUsedDigits; i-- > 0;)
            m_stBigits[i + zeroDigits] = m_stBigits[i];

        for (int i = 0; i < zeroDigits; ++i)
            m_stBigits[i] = 0;

        assert(static_cast<int>(m_uUsedDigits) + zeroDigits >= 0);
        m_uUsedDigits += zeroDigits;
        m_iExponent -= zeroDigits;
        assert(m_iExponent >= 0);
    }
}

void Bignum::Clamp()
{
    while (m_uUsedDigits > 0 && m_stBigits[m_uUsedDigits - 1] == 0)
        --m_uUsedDigits;

    if (m_uUsedDigits == 0)
        m_iExponent = 0;
}

bool Bignum::IsClamped()const
{
    return m_uUsedDigits == 0 || m_stBigits[m_uUsedDigits - 1] != 0;
}

void Bignum::Zero()
{
    for (size_t i = 0; i < m_uUsedDigits; ++i)
        m_stBigits[i] = 0;
    m_uUsedDigits = 0;
    m_iExponent = 0;
}

void Bignum::BigitsShiftLeft(int shiftAmount)
{
    assert(shiftAmount < static_cast<int>(kBigitSize));
    assert(shiftAmount >= 0);

    Chunk carry = 0;
    for (size_t i = 0; i < m_uUsedDigits; ++i)
    {
        Chunk newCarry = m_stBigits[i] >> (kBigitSize - shiftAmount);
        m_stBigits[i] = ((m_stBigits[i] << shiftAmount) + carry) & kBigitMask;
        carry = newCarry;
    }

    if (carry != 0)
    {
        m_stBigits[m_uUsedDigits] = carry;
        m_uUsedDigits++;
    }
}

Bignum::Chunk Bignum::BigitAt(int index)const
{
    if (index >= BigitLength())
        return 0;
    if (index < m_iExponent)
        return 0;
    return m_stBigits[index - m_iExponent];
}

void Bignum::SubtractTimes(const Bignum& other, int factor)
{
    assert(m_iExponent <= other.m_iExponent);
    if (factor < 3)
    {
        for (int i = 0; i < factor; ++i)
            SubtractBignum(other);
        return;
    }

    Chunk borrow = 0;
    int exponentDiff = other.m_iExponent - m_iExponent;
    for (size_t i = 0; i < other.m_uUsedDigits; ++i)
    {
        DoubleChunk product = static_cast<DoubleChunk>(factor) * other.m_stBigits[i];
        DoubleChunk remove = borrow + product;
        Chunk difference = static_cast<Chunk>(m_stBigits[i + exponentDiff] - (remove & kBigitMask));
        m_stBigits[i + exponentDiff] = difference & kBigitMask;
        borrow = static_cast<Chunk>((difference >> (kChunkSize - 1)) + (remove >> kBigitSize));
    }

    for (size_t i = other.m_uUsedDigits + exponentDiff; i < m_uUsedDigits; ++i)
    {
        if (borrow == 0)
            return;
        Chunk difference = m_stBigits[i] - borrow;
        m_stBigits[i] = difference & kBigitMask;
        borrow = difference >> (kChunkSize - 1);
    }

    Clamp();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void InitialScaledStartValuesPositiveExponent(uint64_t significand, int exponent, int estimatedPower,
    bool needBoundaryDeltas, Bignum& numerator, Bignum& denominator, Bignum& deltaMinus, Bignum& deltaPlus)
{
    assert(estimatedPower >= 0);

    numerator.AssignUInt64(significand);
    numerator.ShiftLeft(exponent);
    denominator.AssignPowerUInt16(10, estimatedPower);

    if (needBoundaryDeltas)
    {
        denominator.ShiftLeft(1);
        numerator.ShiftLeft(1);

        deltaPlus.AssignUInt16(1);
        deltaPlus.ShiftLeft(exponent);

        deltaMinus.AssignUInt16(1);
        deltaMinus.ShiftLeft(exponent);
    }
}

static void InitialScaledStartValuesNegativeExponentPositivePower(uint64_t significand, int exponent,
    int estimatedPower, bool needBoundaryDeltas, Bignum& numerator, Bignum& denominator, Bignum& deltaMinus,
    Bignum& deltaPlus)
{
    numerator.AssignUInt64(significand);
    denominator.AssignPowerUInt16(10, estimatedPower);
    denominator.ShiftLeft(-exponent);

    if (needBoundaryDeltas)
    {
        denominator.ShiftLeft(1);
        numerator.ShiftLeft(1);

        deltaPlus.AssignUInt16(1);

        deltaMinus.AssignUInt16(1);
    }
}

static void InitialScaledStartValuesNegativeExponentNegativePower(uint64_t significand, int exponent,
    int estimatedPower, bool needBoundaryDeltas, Bignum& numerator, Bignum& denominator, Bignum& deltaMinus,
    Bignum& deltaPlus)
{
    Bignum& powerTen = numerator;
    powerTen.AssignPowerUInt16(10, -estimatedPower);

    if (needBoundaryDeltas)
    {
        deltaPlus.AssignBignum(powerTen);
        deltaMinus.AssignBignum(powerTen);
    }

    assert(&numerator == &powerTen);
    numerator.MultiplyByUInt64(significand);

    denominator.AssignUInt16(1);
    denominator.ShiftLeft(-exponent);

    if (needBoundaryDeltas)
    {
        numerator.ShiftLeft(1);
        denominator.ShiftLeft(1);
    }
}

int BignumDtoa::NormalizedExponent(uint64_t significand, int exponent)
{
    assert(significand != 0);
    while ((significand & Double::kHiddenBit) == 0)
    {
        significand = significand << 1;
        exponent = exponent - 1;
    }

    return exponent;
}

int BignumDtoa::EstimatePower(int exponent)
{
    const double k1Log10 = 0.30102999566398114;  // 1/lg(10)

    const int kSignificandSize = Double::kSignificandSize;
    double estimate = std::ceil((exponent + kSignificandSize - 1) * k1Log10 - 1e-10);
    return static_cast<int>(estimate);
}

void BignumDtoa::InitialScaledStartValues(uint64_t significand, int exponent, bool lowerBoundaryIsCloser,
    int estimatedPower, bool needBoundaryDeltas, Bignum& numerator, Bignum& denominator, Bignum& deltaMinus,
    Bignum& deltaPlus)
{
    if (exponent >= 0)
    {
        InitialScaledStartValuesPositiveExponent(significand, exponent, estimatedPower, needBoundaryDeltas, numerator,
            denominator, deltaMinus, deltaPlus);
    }
    else if (estimatedPower >= 0)
    {
        InitialScaledStartValuesNegativeExponentPositivePower(significand, exponent, estimatedPower, needBoundaryDeltas,
            numerator, denominator, deltaMinus, deltaPlus);
    }
    else
    {
        InitialScaledStartValuesNegativeExponentNegativePower(significand, exponent, estimatedPower, needBoundaryDeltas,
            numerator, denominator, deltaMinus, deltaPlus);
    }

    if (needBoundaryDeltas && lowerBoundaryIsCloser)
    {
        denominator.ShiftLeft(1);
        numerator.ShiftLeft(1);
        deltaPlus.ShiftLeft(1);
    }
}

void BignumDtoa::FixupMultiply10(int estimatedPower, bool isEven, int& decimalPoint, Bignum& numerator,
    Bignum& denominator, Bignum& deltaMinus, Bignum& deltaPlus)
{
    bool inRange;
    if (isEven)
        inRange = Bignum::PlusCompare(numerator, deltaPlus, denominator) >= 0;
    else
        inRange = Bignum::PlusCompare(numerator, deltaPlus, denominator) > 0;

    if (inRange)
        decimalPoint = estimatedPower + 1;
    else
    {
        decimalPoint = estimatedPower;
        numerator.Times10();
        if (Bignum::Equal(deltaMinus, deltaPlus))
        {
            deltaMinus.Times10();
            deltaPlus.AssignBignum(deltaMinus);
        }
        else
        {
            deltaMinus.Times10();
            deltaPlus.Times10();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



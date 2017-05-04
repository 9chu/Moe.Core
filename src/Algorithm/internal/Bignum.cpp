/**
 * @file
 * @date 2017/5/4
 */
#include <Moe.Core/Algorithm/internal/Bignum.hpp>

#include <algorithm>

using namespace moe;
using namespace internal;

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

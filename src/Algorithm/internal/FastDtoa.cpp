/**
 * @file
 * @date 2017/5/1
 */
#include <Moe.Core/Algorithm/internal/FastDtoa.hpp>

using namespace std;
using namespace moe;
using namespace internal;

static void FastDtoa::BiggestPowerTen(uint32_t number, int numberBits, uint32_t* power, int* exponentPlusOne)
{
    static unsigned int const kSmallPowersOfTen[] =
        { 0, 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

    assert(number < (1u << (numberBits + 1)));
    int exponentPlusOneGuess = ((numberBits + 1) * 1233 >> 12);
    exponentPlusOneGuess++;
    if (number < kSmallPowersOfTen[exponentPlusOneGuess])
        --exponentPlusOneGuess;
    *power = kSmallPowersOfTen[exponentPlusOneGuess];
    *exponentPlusOne = exponentPlusOneGuess;
}

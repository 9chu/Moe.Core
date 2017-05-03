/**
 * @file
 * @date 2017/5/2
 */
#pragma once
#include <Moe.Core/Utility/Misc.hpp>

namespace Testing
{
    struct PrecomputedPrecision
    {
        double v;
        int numberDigits;
        const char* representation;
        int decimalPoint;
    };

    moe::ArrayView<const PrecomputedPrecision> PrecomputedPrecisionRepresentations();
}

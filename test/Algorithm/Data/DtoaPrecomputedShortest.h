/**
 * @file
 * @date 2017/5/2
 */
#pragma once
#include <Moe.Core/Utility/Misc.hpp>

namespace moe
{
    namespace Testing
    {
        struct PrecomputedShortest
        {
            double v;
            const char* representation;
            int decimalPoint;
        };

        ArrayView<const PrecomputedShortest> PrecomputedShortestRepresentations();
    }
}

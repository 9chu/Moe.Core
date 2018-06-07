/**
 * @file
 * @date 2017/5/2
 */
#pragma once
#include <Moe.Core/Utils.hpp>
#include <Moe.Core/ArrayView.hpp>

namespace Testing
{
    struct PrecomputedShortestSingle
    {
        float v;
        const char* representation;
        int decimalPoint;
    };

    const moe::ArrayView<PrecomputedShortestSingle> PrecomputedShortestSingleRepresentations();
}

/**
 *
 * @brief 数学辅助函数库
 * @date: 2017-05-05 
 *
 */

#pragma once
#include <cmath>
#include <algorithm>
#include <type_traits>
#include <limits>

namespace moe 
{
    template<typename T = double>
    struct MathConstants
    {
        constexpr static const T Pi = static_cast<T>(3.14159265358979323846);
        constexpr static const T PiOver2 = Pi / static_cast<T>(2.);
        constexpr static const T PiOver4 = Pi / static_cast<T>(4.);
        
    }
}
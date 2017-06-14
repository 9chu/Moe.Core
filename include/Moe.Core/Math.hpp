/**
 * @file
 * @date 2017/6/14
 */
#pragma once
#include <cstdint>

namespace moe
{
    namespace Math
    {
        //////////////////////////////////////// <editor-fold desc="位运算技巧">

        /**
         * @brief 计算不小于v的最近的二次幂值
         * @param v 输入
         * @return 不小于v的二次幂值
         *
         * 例如：
         *   输入0 返回1
         *   输入7 返回8
         *   输入16 返回16
         */
        uint32_t NextPowerOf2(uint32_t v)
        {
            v--;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            v |= v >> 8;
            v |= v >> 16;
            v++;
            v += (v == 0);
            return v;
        }

        //////////////////////////////////////// </editor-fold>
    }
}

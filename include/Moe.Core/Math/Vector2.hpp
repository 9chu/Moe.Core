/**
 * @file
 * @brief 二维向量
 *
 * 本文件定义了二维向量模板和定义在二维向量上的运算
 * 对于二维向量, 所有的运算符重载均以分量为单位进行计算
 * 针对点乘需要调用Dot方法
 */

#pragma once
#include "MathHelper.hpp"


namespace moe
{
    template <typename T = float>
    class Vector2
    {
            static_assert(
                std::is_arithmetic<T>::value,
                "T must be a numerical type"
            );

        public:
            T x, y;
        private:
            Vector2(): x(T(0)), y(T(0)) {}
            Vector2(T v): x(v), y(v) {}
            Vector2(T vx, T vy): x(vx), y(vy) {}

            Vector2(const Vector2& rhs) = default;
            Vector2(Vector2&& rhs) = default;
    };
};

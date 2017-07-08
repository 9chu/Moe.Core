/**
 * @file
 * @date 2017/7/8
 */
#pragma once
#include <ctime>
#include <cstdint>

#include <string>

namespace moe
{
    /**
     * @brief 时间辅助
     */
    namespace Time
    {
        /**
         * @brief 时间戳类型
         *
         * 跟UNIX时间戳一样从1970/1/1开始计算，但是精确到毫秒。
         */
        using Timestamp = uint64_t;
        using TimestampOffset = int64_t;
        using UnixTimestamp = ::time_t;
        using Tick = uint64_t;  // 滴答数，精确到毫秒
        using HiResTick = double;  // 滴答数，整形部分为毫秒，小数部分依据平台精度而定

        /**
         * @brief 日期
         *
         * 不考虑夏令时，GMT时间。
         */
        struct DateTime
        {
            unsigned Year;  // [1900, )
            unsigned Month;  // [1, 12]
            unsigned Day;  // [1, 31]
            unsigned Hour;  // [0, 23]
            unsigned Minutes;  // [0, 59]
            unsigned Seconds;  // [0, 59]
            unsigned MilliSeconds;  // [0, 999]
        };

        /**
         * @brief 从Utc时间戳转到Unix时间戳
         * @param ts Utc时间戳
         */
        constexpr Timestamp ToUnixTimestamp(Timestamp ts)noexcept { return static_cast<uint32_t>(ts / 1000); }

        /**
         * @brief 获取Utc时间戳
         *
         * 从1970/1/1日起经过的毫秒数。
         */
        Timestamp GetUtcTime()noexcept;

        /**
         * @brief 获取当前时区下相对Utc时间的偏移量
         *
         * 注意该方法只会取一次TimeZone，这意味着当系统TimeZone变化时需要重启程序才会生效。
         */
        TimestampOffset GetTimeZoneOffset()noexcept;

        /**
         * @brief UTC时间到本地时间
         * @param utc 被转换的UTC时间
         */
        inline Timestamp ToLocalTime(Timestamp utc)noexcept { return utc - GetTimeZoneOffset(); }

        /**
         * @brief 本地时间到UTC时间
         * @param local 被转换的本地时间
         */
        inline Timestamp ToUtcTime(Timestamp local)noexcept { return local + GetTimeZoneOffset(); }

        /**
         * @brief 转换UTC时间到本地时间
         */
        inline Timestamp GetLocalTime()noexcept { return ToLocalTime(GetUtcTime()); }

        /**
         * @brief 获取系统滴答数
         */
        Tick GetTickCount()noexcept;

        /**
         * @brief 获取高分辨率滴答数
         */
        HiResTick GetHiResTickCount()noexcept;

        /**
         * @brief 转换时间戳到时间日期
         * @param ts 时间戳
         */
        DateTime ToDateTime(Timestamp ts)noexcept;

        /**
         * @brief 转换时间日期到时间戳
         * @param dt 时间日期
         */
        Timestamp ToTimestamp(const DateTime& dt)noexcept;

        /**
         * @brief 转换到字符串
         *
         * 固定格式：yyyy-mm-dd HH:MM:SS.MS
         */
        std::string ToString(Timestamp ts);
        std::string ToString(const DateTime& dt);
    };
}

/**
 * @file
 * @date 2017/10/12
 *
 * MBP: Moe Binary Data-exchange Protocol
 */
#pragma once

namespace moe
{
    enum class MbpWireTypes
    {
        Null,
        Fixed8 = 1,  // bool/char/byte
        Fixed32 = 2,  // float
        Fixed64 = 3,  // double
        Varint = 4,  // int
        Buffer = 5,  // string/bytes
        List = 6,  // vector<T>/array<T>
        Map = 7,  // unordered_map<K,V>/map<K,V>
        Struct = 8,  // struct
    };

    struct MbpStruct
    {
    };

    class MbpReader
    {

    };

    class MbpWriter
    {

    };
}

/**
 * @file
 * @author chu
 * @date 2017/12/26
 */
#include <gtest/gtest.h>

#include <Moe.Core/Mdr.hpp>

using namespace std;
using namespace moe;

template <typename T>
size_t PutAndFetch(const T& in, T& out)
{
    uint8_t buf[128] = { 0 };

    BytesViewStream stream { MutableBytesView(buf, 128) };
    Mdr::Writer writer(&stream);
    writer.Write(in, 0);

    BytesViewStream readStream { BytesView(buf, stream.GetPosition()) };
    Mdr::Reader reader(&readStream);
    reader.Read(out, 0);
    return stream.GetPosition();
}

template <typename T>
void AutoTestIntType()
{
    T n1 = 0; T n1o;
    EXPECT_EQ(1u, PutAndFetch(n1, n1o));
    EXPECT_EQ(n1, n1o);
    T n2 = 1; T n2o;
    EXPECT_EQ(1u, PutAndFetch(n2, n2o));
    EXPECT_EQ(n2, n2o);
    if (sizeof(T) == 1)
    {
        if (is_signed<T>::value)
        {
            T n3 = numeric_limits<T>::min(), n3o;
            EXPECT_EQ(2u, PutAndFetch(n3, n3o));
            EXPECT_EQ(n3, n3o);
        }
        T n4 = numeric_limits<T>::max(), n4o;
        EXPECT_EQ(2u, PutAndFetch(n4, n4o));
        EXPECT_EQ(n4, n4o);
    }
    else
    {
        if (is_signed<T>::value)
        {
            T n3 = -64; T n3o;
            EXPECT_EQ(2u, PutAndFetch(n3, n3o));
            EXPECT_EQ(n3, n3o);
            T n4 = 65; T n4o;
            EXPECT_EQ(3u, PutAndFetch(n4, n4o));
            EXPECT_EQ(n4, n4o);
        }
        else
        {
            T n3 = 127; T n3o;
            EXPECT_EQ(2u, PutAndFetch(n3, n3o));
            EXPECT_EQ(n3, n3o);
            T n4 = 255; T n4o;
            EXPECT_EQ(3u, PutAndFetch(n4, n4o));
            EXPECT_EQ(n4, n4o);
        }
        if (sizeof(T) == 2)
        {
            if (is_signed<T>::value)
            {
                T n5 = numeric_limits<T>::min(), n5o;
                EXPECT_EQ(4u, PutAndFetch(n5, n5o));
                EXPECT_EQ(n5, n5o);
            }
            T n6 = numeric_limits<T>::max(), n6o;
            EXPECT_EQ(4u, PutAndFetch(n6, n6o));
            EXPECT_EQ(n6, n6o);
        }
        if (sizeof(T) == 4)
        {
            if (is_signed<T>::value)
            {
                T n5 = numeric_limits<T>::min(), n5o;
                EXPECT_EQ(5u, PutAndFetch(n5, n5o));
                EXPECT_EQ(n5, n5o);
            }
            T n6 = numeric_limits<T>::max(), n6o;
            EXPECT_EQ(5u, PutAndFetch(n6, n6o));
            EXPECT_EQ(n6, n6o);
        }
        if (sizeof(T) == 8)
        {
            if (is_signed<T>::value)
            {
                T n5 = numeric_limits<T>::min(); T n5o;
                EXPECT_EQ(9u, PutAndFetch(n5, n5o));
                EXPECT_EQ(n5, n5o);
            }
            T n6 = numeric_limits<T>::max(); T n6o;
            EXPECT_EQ(9u, PutAndFetch(n6, n6o));
            EXPECT_EQ(n6, n6o);
        }
    }
}

TEST(Mdr, Integer)
{
    // bool
    {
        bool n1 = false, n1o;
        EXPECT_EQ(1u, PutAndFetch(n1, n1o));
        EXPECT_EQ(n1, n1o);

        bool n2 = false, n2o;
        EXPECT_EQ(1u, PutAndFetch(n2, n2o));
        EXPECT_EQ(n2, n2o);
    }

    // char
    AutoTestIntType<char>();

    // signed char
    AutoTestIntType<signed char>();

    // unsigned char
    AutoTestIntType<unsigned char>();

    // short
    AutoTestIntType<short>();

    // unsigned short
    AutoTestIntType<unsigned short>();

    // int
    AutoTestIntType<int>();

    // unsigned int
    AutoTestIntType<unsigned int>();

    // long
    AutoTestIntType<long>();

    // unsigned long
    AutoTestIntType<unsigned long>();

    // long long
    AutoTestIntType<long long>();

    // unsigned long long
    AutoTestIntType<unsigned long long>();

    // special
    {
        uint32_t n1 = 268435455, n1o;
        EXPECT_EQ(5u, PutAndFetch(n1, n1o));
        EXPECT_EQ(n1, n1o);

        uint32_t n2 = 268435456, n2o;
        EXPECT_EQ(5u, PutAndFetch(n2, n2o));
        EXPECT_EQ(n2, n2o);

        uint64_t n3 = 72057594037927935ull, n3o;
        EXPECT_EQ(9u, PutAndFetch(n3, n3o));
        EXPECT_EQ(n3, n3o);

        uint64_t n4 = 72057594037927936ull, n4o;
        EXPECT_EQ(9u, PutAndFetch(n4, n4o));
        EXPECT_EQ(n4, n4o);
    }
}

TEST(Mdr, Floating)
{
    // float
    {
        float n1 = 0.f, n1o;
        EXPECT_EQ(5u, PutAndFetch(n1, n1o));
        EXPECT_EQ(n1, n1o);

        float n2 = 1.f, n2o;
        EXPECT_EQ(5u, PutAndFetch(n2, n2o));
        EXPECT_EQ(n2, n2o);
    }

    // double
    {
        double n1 = 0., n1o;
        EXPECT_EQ(9u, PutAndFetch(n1, n1o));
        EXPECT_EQ(n1, n1o);

        double n2 = 1., n2o;
        EXPECT_EQ(9u, PutAndFetch(n2, n2o));
        EXPECT_EQ(n2, n2o);
    }
}

TEST(Mdr, String)
{
    // string
    {
        string n1 = "", n1o;
        EXPECT_EQ(2u, PutAndFetch(n1, n1o));
        EXPECT_EQ(n1, n1o);

        string n2 = "abc", n2o;
        EXPECT_EQ(5u, PutAndFetch(n2, n2o));
        EXPECT_EQ(n2, n2o);
    }
}

TEST(Mdr, Structure)
{
    struct Test1
    {
        int a;
        Optional<int> b;
        Optional<int> c;
        int d;

        bool operator==(const Test1& rhs)const noexcept
        {
            return a == rhs.a && b == rhs.b && c == rhs.c && d == rhs.d;
        }

        void ReadFrom(Mdr::Reader* reader)
        {
            assert(reader);
            reader->Read(a, 0);
            reader->Read(b, 1);
            reader->Read(c, 2);
            reader->Read(d, 3);
        }

        void WriteTo(Mdr::Writer* writer)const
        {
            assert(writer);
            writer->Write(a, 0);
            writer->Write(b, 1);
            writer->Write(c, 2);
            writer->Write(d, 3);
        }
    };

    Test1 s1, s1o;
    s1.a = 0xAABBCCDD;
    s1.b = 0;
    s1.d = 10;
    PutAndFetch(s1, s1o);
    EXPECT_EQ(s1, s1o);
}

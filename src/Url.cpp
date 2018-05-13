/**
 * @file
 * @author chu
 * @date 2018/3/9
 */
#include <Moe.Core/Url.hpp>
#include <Moe.Core/Exception.hpp>
#include <Moe.Core/StringUtils.hpp>

#include <cassert>

using namespace std;
using namespace moe;

namespace
{
    bool ParseIpv4Number(uint32_t& result, const char* start, const char* end)
    {
        uint64_t temp = 0;
        unsigned radix = 10;
        if (end - start >= 2)
        {
            if (start[0] == '0' && (start[1] == 'x' || start[1] == 'X'))  // 16进制
            {
                start += 2;
                radix = 16;
            }
            else if (start[0] == '0')  // 8进制
            {
                start += 1;
                radix = 8;
            }
        }

        temp = result = 0;
        if (start >= end)
            return true;

        while (start < end)
        {
            char ch = *(start++);

            if (ch >= 'a' && ch <= 'f')
            {
                if (radix != 16)
                    return false;
                temp = temp * 16 + (ch - 'a') + 10;
            }
            else if (ch >= 'A' && ch <= 'F')
            {
                if (radix != 16)
                    return false;
                temp = temp * 16 + (ch - 'A') + 10;
            }
            else if (ch >= '0' && ch <= '9')
            {
                if ((ch == '8' || ch == '9') && radix == 8)
                    return false;
                temp = temp * radix + (ch - '0');
            }
            else
                return false;

            if (temp > numeric_limits<uint32_t>::max())
                return false;
        }

        result = static_cast<uint32_t>(temp);
        return true;
    }
}

//////////////////////////////////////////////////////////////////////////////// Url::Host

Url::Host::Host()
{
}

Url::Host::Host(const Host& rhs)
    : m_iType(rhs.m_iType)
{
    switch (m_iType)
    {
        case ValueTypes::None:
            break;
        case ValueTypes::Domain:
            new(&m_stValue.Domain) string(rhs.m_stValue.Domain);
            break;
        case ValueTypes::Ipv4:
            m_stValue.Ipv4 = rhs.m_stValue.Ipv4;
            break;
        case ValueTypes::Ipv6:
            new(&m_stValue.Ipv6) Ipv6AddressType(rhs.m_stValue.Ipv6);
            break;
        case ValueTypes::Opaque:
            new(&m_stValue.Opaque) string(rhs.m_stValue.Opaque);
            break;
        default:
            assert(false);
            break;
    }
}

Url::Host::Host(Host&& rhs)
    : m_iType(rhs.m_iType)
{
    switch (m_iType)
    {
        case ValueTypes::None:
            break;
        case ValueTypes::Domain:
            new(&m_stValue.Domain) string(std::move(rhs.m_stValue.Domain));
            break;
        case ValueTypes::Ipv4:
            m_stValue.Ipv4 = rhs.m_stValue.Ipv4;
            break;
        case ValueTypes::Ipv6:
            new(&m_stValue.Ipv6) Ipv6AddressType(rhs.m_stValue.Ipv6);
            break;
        case ValueTypes::Opaque:
            new(&m_stValue.Opaque) string(std::move(rhs.m_stValue.Opaque));
            break;
        default:
            assert(false);
            break;
    }

    rhs.Reset();
}

Url::Host::~Host()
{
    Reset();
}

Url::Host& Url::Host::operator=(const Host& rhs)
{
    Reset();

    switch (m_iType)
    {
        case ValueTypes::None:
            break;
        case ValueTypes::Domain:
            new(&m_stValue.Domain) string(rhs.m_stValue.Domain);
            break;
        case ValueTypes::Ipv4:
            m_stValue.Ipv4 = rhs.m_stValue.Ipv4;
            break;
        case ValueTypes::Ipv6:
            new(&m_stValue.Ipv6) Ipv6AddressType(rhs.m_stValue.Ipv6);
            break;
        case ValueTypes::Opaque:
            new(&m_stValue.Opaque) string(rhs.m_stValue.Opaque);
            break;
        default:
            assert(false);
            break;
    }

    m_iType = rhs.m_iType;
    return *this;
}

Url::Host& Url::Host::operator=(Host&& rhs)noexcept
{
    Reset();

    switch (m_iType)
    {
        case ValueTypes::None:
            break;
        case ValueTypes::Domain:
            new(&m_stValue.Domain) string(std::move(rhs.m_stValue.Domain));
            break;
        case ValueTypes::Ipv4:
            m_stValue.Ipv4 = rhs.m_stValue.Ipv4;
            break;
        case ValueTypes::Ipv6:
            new(&m_stValue.Ipv6) Ipv6AddressType(rhs.m_stValue.Ipv6);
            break;
        case ValueTypes::Opaque:
            new(&m_stValue.Opaque) string(std::move(rhs.m_stValue.Opaque));
            break;
        default:
            assert(false);
            break;
    }

    m_iType = rhs.m_iType;
    rhs.Reset();
    return *this;
}

bool Url::Host::operator==(const Host& rhs)const noexcept
{
    if (m_iType != rhs.m_iType)
        return false;

    switch (m_iType)
    {
        case ValueTypes::None:
            return true;
        case ValueTypes::Domain:
            return m_stValue.Domain == rhs.m_stValue.Domain;
        case ValueTypes::Ipv4:
            return m_stValue.Ipv4 == rhs.m_stValue.Ipv4;
        case ValueTypes::Ipv6:
            return m_stValue.Ipv6 == rhs.m_stValue.Ipv6;
        case ValueTypes::Opaque:
            return m_stValue.Opaque == rhs.m_stValue.Opaque;
        default:
            return false;
    }
}

bool Url::Host::operator!=(const Host& rhs)const noexcept
{
    return !operator==(rhs);
}

Url::Host::operator bool()const noexcept
{
    return m_iType != ValueTypes::None;
}

Url::Host::ValueTypes Url::Host::GetType()const noexcept
{
    return m_iType;
}

const std::string& Url::Host::GetDomain()const noexcept
{
    static const string kEmpty;
    return m_iType == ValueTypes::Domain ? m_stValue.Domain : kEmpty;
}

void Url::Host::SetDomain(const std::string& value)
{
    Reset();

    new(&m_stValue.Domain) string(value);
    m_iType = ValueTypes::Domain;
}

void Url::Host::SetDomain(std::string&& value)noexcept
{
    Reset();

    new(&m_stValue.Domain) string(std::move(value));
    m_iType = ValueTypes::Domain;
}

uint32_t Url::Host::GetIpv4()const noexcept
{
    return m_iType == ValueTypes::Ipv4 ? m_stValue.Ipv4 : 0u;
}

void Url::Host::SetIpv4(uint32_t value)noexcept
{
    Reset();

    m_iType = ValueTypes::Ipv4;
    m_stValue.Ipv4 = value;
}

const Url::Host::Ipv6AddressType& Url::Host::GetIpv6()const noexcept
{
    static const Ipv6AddressType kEmpty = {};
    return m_iType == ValueTypes::Ipv6 ? m_stValue.Ipv6 : kEmpty;
}

void Url::Host::SetIpv6(const Ipv6AddressType& value)noexcept
{
    Reset();

    m_iType = ValueTypes::Ipv6;
    m_stValue.Ipv6 = value;
}

const std::string& Url::Host::GetOpaque()const noexcept
{
    static const string kEmpty;
    return m_iType == ValueTypes::Opaque ? m_stValue.Opaque : kEmpty;
}

void Url::Host::SetOpaque(const std::string& value)
{
    Reset();

    new(&m_stValue.Opaque) string(value);
    m_iType = ValueTypes::Opaque;
}

void Url::Host::SetOpaque(std::string&& value)noexcept
{
    Reset();

    new(&m_stValue.Opaque) string(std::move(value));
    m_iType = ValueTypes::Opaque;
}

void Url::Host::Reset()
{
    switch (m_iType)
    {
        case ValueTypes::Domain:
            m_stValue.Domain.~string();
            break;
        case ValueTypes::Ipv4:
            break;
        case ValueTypes::Ipv6:
            m_stValue.Ipv6.~Ipv6AddressType();
            break;
        case ValueTypes::Opaque:
            m_stValue.Opaque.~string();
            break;
        default:
            break;
    }

    m_iType = ValueTypes::None;
}

bool Url::Host::ParseIpv4Part(const char* start, const char* end)
{
    auto mark = start;
    unsigned parts = 0;
    unsigned tooBigParts = 0;
    uint32_t numbers[4] = { 0, 0, 0, 0 };

    while (start <= end)
    {
        char ch = start < end ? *start : '\0';
        if (ch == '.' || ch == '\0')
        {
            if (++parts > 4)
                return false;  // IPV4地址至多只有4个部分
            if (start == mark)
                return false;  // 无效的空的点分部分

            // 解析[mark, start)部分的数字
            uint32_t n = 0;
            if (!ParseIpv4Number(n, mark, start))
                return false;
            if (n > 255)
                ++tooBigParts;

            numbers[parts - 1] = n;
            mark = start + 1;
            if (ch == '.' && start + 1 >= end)
                break;  // 允许输入的最后一个字符是'.'
        }
        ++start;
    }
    assert(parts >= 0);

    if (tooBigParts > 1 || (tooBigParts == 1 && numbers[parts - 1] <= 255) ||
        numbers[parts - 1] >= std::pow(256, static_cast<double>(5 - parts)))
    {
        // 规范要求每个点分部分不能超过255，但是最后一个元素例外
        // 此外，整个IPV4的解析结果也要保证不能溢出
        return false;
    }

    // 计算IPV4值
    uint32_t val = numbers[parts - 1];
    for (unsigned n = 0; n < parts - 1; ++n)
    {
        double b = 3 - n;
        val += numbers[n] * std::pow(256, b);
    }
    SetIpv4(val);
    return true;
}

bool Url::Host::ParseIpv6Part(const char* start, const char* end)
{
    char ch = start < end ? *start : '\0';
    unsigned current = 0;  // 指示当前解析的部分
    uint32_t compress = 0xFFFFFFFF;  // 指示压缩可扩充的位置

    Ipv6AddressType address;
    address.fill(0);

    // 压缩的case '::xxxx'
    if (ch == ':')
    {
        if (end - start < 2 || start[1] != ':')
            return false;  // 无效的':
        start += 2;
        ch = start < end ? *start : '\0';
        ++current;
        compress = current;
    }

    while (ch != '\0')
    {
        if (current >= address.max_size())
            return false;  // 无效的地址（至多8个部分）

        // 压缩的case 'fe80::xxxxx'
        if (ch == ':')
        {
            if (compress != 0xFFFFFFFF)
                return;  // 不可能同时存在两个压缩部分
            ++start;
            ch = start < end ? *start : '\0';
            address[current++] = 0;
            compress = current;
            continue;
        }

        uint32_t value = 0;
        unsigned len = 0;
        while (len < 4 && StringUtils::IsHexDigit(ch))
        {
            if (ch >= '0' && ch <= '9')
                value = value * 16 + (ch - '0');
            else if (ch >= 'a' && ch <= 'f')
                value = value * 16 + (ch - 'a' + 10);
            else
            {
                assert(ch >= 'A' && ch <= 'F');
                value = value * 16 + (ch - 'A' + 10);
            }

            ++start;
            ch = start < end ? *start : '\0';
            ++len;
        }

        switch (ch)
        {
            case '.':  // 内嵌IPV4地址
                if (len == 0)
                    return false;
                start -= len;  // 回退
                ch = start < end ? *start : '\0';
                if (current > address.max_size() - 2)  // IPV4地址占两个Piece
                    return false;

                // 解析IPV4部分（这一地址只允许使用十进制点分结构）
                {
                    unsigned numbers = 0;
                    while (ch != '\0')
                    {
                        value = 0xFFFFFFFF;
                        if (numbers > 0)
                        {
                            if (ch == '.' && numbers < 4)
                            {
                                ++start;
                                ch = start < end ? *start : '\0';
                            }
                            else
                                return false;
                        }
                        if (!StringUtils::IsDigit(ch))
                            return false;
                        while (StringUtils::IsDigit(ch))
                        {
                            unsigned number = ch - '0';
                            if (value == 0xFFFFFFFF)
                                value = number;
                            else if (value == 0)
                                return false;
                            else
                                value = value * 10 + number;
                            if (value > 255)
                                return false;
                            ++start;
                            ch = start < end ? *start : '\0';
                        }
                        address[current] = address[current] * 0x100 + value;
                        ++numbers;
                        if (numbers == 2 || numbers == 4)
                            ++current;
                    }
                    if (numbers != 4)
                        return false;
                }
                continue;
            case ':':
                ++start;
                ch = start < end ? *start : '\0';
                if (ch == '\0')
                    return false;
                break;
            case '\0':
                break;
            default:
                return false;
        }
        address[current] = value;
        ++current;
    }

    if (compress != 0xFFFFFFFF)
    {
        auto swaps = current - compress;
        current = address.max_size() - 1;
        while (current != 0 && swaps > 0)
        {
            auto swap = compress + swaps - 1;
            std::swap(address[current], address[swap]);
            --current;
            --swaps;
        }
    }
    else if (compress == 0xFFFFFFFF && current != address.max_size())
        return false;

    SetIpv6(address);
    return true;
}

void Url::Host::ParseOpaqueHost()
{

}

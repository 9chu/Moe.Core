/**
 * @file
 * @date 2017/5/28
 */
#include <Moe.Core/Exception.hpp>

using namespace std;
using namespace moe;

const std::string& Exception::ToString()const
{
    assert(m_pStorage);
    if (m_pStorage->FullDescCache.empty())
    {
        StringUtils::Format(m_pStorage->FullDescCache, "[{0}:{1}] {2}: {3}", GetSourceFile(), GetLineNumber(),
            GetFunctionName(), GetDescription());
    }

    return m_pStorage->FullDescCache;
}

const char* Exception::what()const noexcept
{
    return ToString().c_str();
}

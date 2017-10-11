/**
 * @file
 * @date 2017/9/20
 */
#include <Moe.Core/TextReader.hpp>

using namespace std;
using namespace moe;

TextReader::TextReader()
{
}

TextReader::TextReader(ArrayView<char> input, const char* sourceName)
    : m_stBuffer(input), m_stSourceName(sourceName)
{
}

TextReader::TextReader(const std::string& input, const char* sourceName)
    : m_stBuffer(input.c_str(), input.size()), m_stSourceName(sourceName)
{
}

TextReader::TextReader(const TextReader& rhs)
    : m_stBuffer(rhs.m_stBuffer), m_stSourceName(rhs.m_stSourceName), m_uPosition(rhs.m_uPosition),
    m_uLine(rhs.m_uLine), m_uColumn(rhs.m_uColumn)
{
}

void TextReader::Back()
{
    if (m_uPosition == 0)
        MOE_THROW(OutOfRangeException, "Already at the first character");

    char ch = m_stBuffer[--m_uPosition];

    if ((ch == '\r' && (m_uPosition + 1 >= m_stBuffer.GetSize() || m_stBuffer[m_uPosition + 1] != '\n')) || ch == '\n')
    {
        --m_uLine;
        m_uColumn = 1;

        // 回溯寻找字符个数
        if (m_uPosition != 0)
        {
            const char* c = &m_stBuffer[m_uPosition - 1];
            do
            {
                if ((*c == '\r' && *(c + 1) != '\n') || *c == '\n')
                    break;
                --c;
                ++m_uColumn;
            } while (c >= m_stBuffer.GetBuffer());
        }
    }
    else
        --m_uColumn;
}

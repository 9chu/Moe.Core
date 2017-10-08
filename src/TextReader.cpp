/**
 * @file
 * @date 2017/9/20
 */
#include <Moe.Core/TextReader.hpp>

using namespace std;
using namespace moe;

//////////////////////////////////////////////////////////////////////////////// TextReaderFromView

TextReaderFromView::TextReaderFromView(const ArrayView<char>& view, const char* sourceName)
    : m_stView(view), m_stSourceName(sourceName)
{
}

const char* TextReaderFromView::GetSourceName()const noexcept
{
    return m_stSourceName.c_str();
}

size_t TextReaderFromView::GetLength()const noexcept
{
    return m_stView.GetSize();
}

size_t TextReaderFromView::GetPosition()const noexcept
{
    return m_uPosition;
}

uint32_t TextReaderFromView::GetLine()const noexcept
{
    return m_uLine;
}

uint32_t TextReaderFromView::GetColumn()const noexcept
{
    return m_uColumn;
}

bool TextReaderFromView::IsEof()const noexcept
{
    return m_uPosition >= m_stView.GetSize();
}

char TextReaderFromView::Read()
{
    if (IsEof())
        return '\0';

    char ch = m_stView[m_uPosition];
    ++m_uPosition;
    ++m_uColumn;

    if ((ch == '\r' && Peek() != '\n') || ch == '\n')
    {
        ++m_uLine;
        m_uColumn = 1;
    }

    return ch;
}

char TextReaderFromView::Peek()
{
    if (IsEof())
        return '\0';
    return m_stView[m_uPosition];
}

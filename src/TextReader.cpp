/**
 * @file
 * @date 2017/9/20
 */
#include <Moe.Core/TextReader.hpp>

using namespace std;
using namespace moe;

//////////////////////////////////////////////////////////////////////////////// TextReaderFromStream

TextReaderFromView::TextReaderFromView(const ArrayView<char>& view)
    : m_stView(view)
{
}

size_t TextReaderFromView::GetLength()noexcept override
{
    return m_stView.GetSize();
}

size_t TextReaderFromView::GetPosition()noexcept override
{
    return m_uPosition;
}

bool TextReaderFromView::IsEof()noexcept override
{
    return m_uPosition >= m_stView.GetSize();
}

int TextReaderFromView::Read()override
{
    if (IsEof())
        return -1;
    return m_stView[m_uPosition++];
}

int TextReaderFromView::Peek()override
{
    if (IsEof())
        return -1;
    return m_stView[m_uPosition];
}

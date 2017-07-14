/**
 * @file
 * @date 2017/7/14
 */
#include <Moe.Core/Stream.hpp>

using namespace std;
using namespace moe;

//////////////////////////////////////////////////////////////////////////////// Stream

Stream::~Stream()
{
}

//////////////////////////////////////////////////////////////////////////////// BytesViewStream

BytesViewStream::BytesViewStream(const BytesView& view)
    : m_stView(view)
{
}

BytesViewStream::BytesViewStream(const MutableBytesView& view)
    : m_stView(view), m_stMutableView(InPlaceInit, view)
{
}

bool BytesViewStream::IsReadable()const noexcept
{
    return true;
}

bool BytesViewStream::IsWriteable()const noexcept
{
    return static_cast<bool>(m_stMutableView);
}

bool BytesViewStream::IsSeekable()const noexcept
{
    return true;
}

size_t BytesViewStream::GetLength()const
{
    return m_stView.GetSize();
}

size_t BytesViewStream::GetPosition()const
{
    return m_uPosition;
}

void BytesViewStream::Flush()
{
}

int BytesViewStream::ReadByte()
{
    if (m_uPosition >= m_stView.GetSize())
        return -1;
    return m_stView[m_uPosition++];
}

size_t BytesViewStream::Read(MutableBytesView& out, size_t count)
{
    assert(m_stView.GetSize() >= m_uPosition);
    assert(out.GetSize() >= count);
    count = std::min(count, out.GetSize());
    count = std::min(count, m_stView.GetSize() - m_uPosition);
    ::memcpy(out.GetBuffer(), m_stView.GetBuffer(), count);
    return count;
}

size_t BytesViewStream::Seek(int64_t offset, StreamSeekOrigin origin)
{
    switch (origin)
    {
        case StreamSeekOrigin::Begin:
            m_uPosition = static_cast<size_t>(std::max<int64_t>(0, offset));
            m_uPosition = std::min(m_uPosition, m_stView.GetSize());
            break;
        case StreamSeekOrigin::Current:
            if (offset < 0)
            {
                size_t positive = (size_t)-offset;
                if (positive >= m_uPosition)
                    m_uPosition = 0;
                else
                    m_uPosition -= positive;
            }
            else
            {
                m_uPosition += (size_t)offset;
                m_uPosition = std::min(m_uPosition, m_stView.GetSize());
            }
            break;
        case StreamSeekOrigin::End:
            if (offset >= 0)
                m_uPosition = m_stView.GetSize();
            else
            {
                size_t positive = (size_t)-offset;
                if (positive >= m_stView.GetSize())
                    m_uPosition = 0;
                else
                    m_uPosition -= positive;
            }
            break;
    }

    return m_uPosition;
}

void BytesViewStream::SetLength(size_t length)
{
    MOE_UNUSED(length);
    MOE_THROW(OperationNotSupport, "BytesView cannot reset size");
}

void BytesViewStream::WriteByte(uint8_t b)
{
    if (!m_stMutableView)
        MOE_THROW(OperationNotSupport, "BytesView is not mutable");
    if (m_uPosition >= m_stMutableView->GetSize())
        MOE_THROW(OutOfRangeException, "Write out of range");

    (*m_stMutableView)[m_uPosition++] = b;
}

void BytesViewStream::Write(const BytesView& view, size_t count)
{
    assert(view.GetSize() >= count);
    count = std::min(count, view.GetSize());
    if (!m_stMutableView)
        MOE_THROW(OperationNotSupport, "BytesView is not mutable");
    if (m_uPosition + count > m_stMutableView->GetSize())
        MOE_THROW(OutOfRangeException, "Write out of range");

    ::memcpy(m_stMutableView->GetBuffer() + m_uPosition, view.GetBuffer(), count);
    m_uPosition += count;
}

#include "odpch.h"
#include "Odyssey/Core/Buffer.h"

#include "Odyssey/Core/Assert.h"

#include <cstring>

namespace Odyssey {

	Buffer::Buffer(uint64_t size)
	{
		Allocate(size);
	}

	Buffer::Buffer(Buffer&& other) noexcept
		: m_Data(std::move(other.m_Data)), m_Size(other.m_Size)
	{
		other.m_Size = 0;
	}

	Buffer& Buffer::operator=(Buffer&& other) noexcept
	{
		if (this != &other)
		{
			m_Data = std::move(other.m_Data);
			m_Size = other.m_Size;
			other.m_Size = 0;
		}
		return *this;
	}

	Buffer Buffer::Copy(const void* data, uint64_t size)
	{
		OD_CORE_VERIFY(data != nullptr || size == 0, "Buffer::Copy called with null data and non-zero size {}", size);

		Buffer buffer(size);
		if (size > 0)
			std::memcpy(buffer.m_Data.get(), data, static_cast<size_t>(size));
		return buffer;
	}

	Buffer Buffer::Clone() const
	{
		return Copy(m_Data.get(), m_Size);
	}

	void Buffer::Allocate(uint64_t size)
	{
		Release();
		if (size == 0)
			return;

		m_Data.reset(new uint8_t[static_cast<size_t>(size)]);
		m_Size = size;
	}

	void Buffer::Release()
	{
		m_Data.reset();
		m_Size = 0;
	}

	void Buffer::ZeroInitialize()
	{
		if (m_Size > 0)
			std::memset(m_Data.get(), 0, static_cast<size_t>(m_Size));
	}

	void Buffer::Write(const void* data, uint64_t size, uint64_t offset)
	{
		if (size == 0)
			return;

		OD_CORE_VERIFY(data != nullptr, "Buffer::Write called with null data");
		VerifyRange(size, offset);
		std::memcpy(m_Data.get() + offset, data, static_cast<size_t>(size));
	}

	void Buffer::VerifyRange(uint64_t size, uint64_t offset) const
	{
		// Written to avoid overflow in offset + size.
		const bool inRange = size <= m_Size && offset <= m_Size - size;
		OD_CORE_VERIFY(inRange, "Buffer access out of range (offset {}, size {}, buffer size {})", offset, size, m_Size);
	}

}

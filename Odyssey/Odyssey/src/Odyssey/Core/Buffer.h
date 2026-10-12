#pragma once

#include "Odyssey/Core/Base.h"

#include <cstddef>
#include <cstring>
#include <memory>
#include <span>
#include <type_traits>

namespace Odyssey {

	// Owning, move-only block of bytes. Storage is aligned to at least alignof(std::max_align_t).
	// Use std::span<const uint8_t> to pass non-owning views of the data.
	class Buffer
	{
	public:
		Buffer() = default;
		// Allocates uninitialized storage.
		explicit Buffer(uint64_t size);
		~Buffer() = default;

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
		Buffer(Buffer&& other) noexcept;
		Buffer& operator=(Buffer&& other) noexcept;

		// Creates a buffer holding a copy of the given bytes. data may be null only if size is 0.
		static Buffer Copy(const void* data, uint64_t size);
		Buffer Clone() const;

		// Replaces the contents with uninitialized storage of the given size. Size 0 releases the buffer.
		void Allocate(uint64_t size);
		void Release();
		void ZeroInitialize();

		// Copies size bytes into the buffer at offset. Verifies the range lies within the buffer.
		void Write(const void* data, uint64_t size, uint64_t offset = 0);

		// Returns a copy of the value stored at offset. Safe for unaligned offsets.
		template<typename T>
		T Read(uint64_t offset = 0) const
		{
			static_assert(std::is_trivially_copyable_v<T> && std::is_default_constructible_v<T>, "Buffer::Read requires a trivially copyable, default constructible type");
			VerifyRange(sizeof(T), offset);

			T value;
			std::memcpy(&value, m_Data.get() + offset, sizeof(T));
			return value;
		}

		template<typename T>
		T* As() { return reinterpret_cast<T*>(m_Data.get()); }

		template<typename T>
		const T* As() const { return reinterpret_cast<const T*>(m_Data.get()); }

		uint8_t* GetData() { return m_Data.get(); }
		const uint8_t* GetData() const { return m_Data.get(); }
		uint64_t GetSize() const { return m_Size; }
		bool IsEmpty() const { return m_Size == 0; }

		std::span<uint8_t> GetSpan() { return { m_Data.get(), static_cast<size_t>(m_Size) }; }
		std::span<const uint8_t> GetSpan() const { return { m_Data.get(), static_cast<size_t>(m_Size) }; }

		explicit operator bool() const { return m_Size != 0; }
	private:
		void VerifyRange(uint64_t size, uint64_t offset) const;
	private:
		std::unique_ptr<uint8_t[]> m_Data;
		uint64_t m_Size = 0;
	};

}

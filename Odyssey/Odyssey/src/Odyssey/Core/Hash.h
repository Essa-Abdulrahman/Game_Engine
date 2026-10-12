#pragma once

#include <cstdint>
#include <string_view>

namespace Odyssey {

	class Hash
	{
	public:
		// 64-bit FNV-1a. Stable across platforms and builds, so results may be persisted.
		// Not suitable for security purposes.
		static constexpr uint64_t FNV1a64(std::string_view text)
		{
			constexpr uint64_t offsetBasis = 0xcbf29ce484222325ull;
			constexpr uint64_t prime = 0x100000001b3ull;

			uint64_t hash = offsetBasis;
			for (char character : text)
			{
				hash ^= static_cast<uint8_t>(character);
				hash *= prime;
			}
			return hash;
		}
	};

}

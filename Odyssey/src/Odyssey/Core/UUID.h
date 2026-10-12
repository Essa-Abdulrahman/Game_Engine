#pragma once

#include "Odyssey/Core/Base.h"

#include <compare>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

namespace Odyssey {

	// 64-bit random identifier for entities, assets and other persistent objects.
	// The value 0 is reserved as the null identifier and is never generated.
	class UUID
	{
	public:
		// Generates a new random, non-null identifier. Thread-safe.
		UUID();
		constexpr explicit UUID(uint64_t value)
			: m_Value(value)
		{
		}

		static constexpr UUID Null() { return UUID(0); }

		constexpr uint64_t GetValue() const { return m_Value; }
		constexpr bool IsNull() const { return m_Value == 0; }
		constexpr explicit operator uint64_t() const { return m_Value; }

		// Fixed-width, 16 lowercase hexadecimal digits. Serialized files store identifiers as strings
		// because many JSON consumers cannot represent 64-bit integers exactly.
		std::string ToString() const;
		// Accepts exactly 16 hexadecimal digits (either case). Returns nullopt for anything else.
		static std::optional<UUID> FromString(std::string_view text);

		constexpr auto operator<=>(const UUID& other) const = default;
	private:
		uint64_t m_Value;
	};

}

namespace std {

	template<>
	struct hash<Odyssey::UUID>
	{
		size_t operator()(const Odyssey::UUID& uuid) const noexcept
		{
			return std::hash<uint64_t>()(uuid.GetValue());
		}
	};

}

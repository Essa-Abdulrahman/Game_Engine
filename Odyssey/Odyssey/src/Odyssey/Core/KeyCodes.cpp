#include "odpch.h"
#include "Odyssey/Core/KeyCodes.h"

#include "Odyssey/Core/StringUtils.h"

#include <algorithm>
#include <array>

namespace Odyssey {

	namespace {

		template<typename T>
		struct CodeName
		{
			T Code;
			const char* Name;
		};

#define OD_INTERNAL_KEY_ENTRY(name, value) CodeName<KeyCode> { KeyCode::name, #name },
#define OD_INTERNAL_MOUSE_ENTRY(name, value) CodeName<MouseCode> { MouseCode::name, #name },

		constexpr std::array s_KeyCodeNames = { OD_KEY_CODE_LIST(OD_INTERNAL_KEY_ENTRY) };
		constexpr std::array s_MouseCodeNames = { OD_MOUSE_CODE_LIST(OD_INTERNAL_MOUSE_ENTRY) };

#undef OD_INTERNAL_KEY_ENTRY
#undef OD_INTERNAL_MOUSE_ENTRY

#define OD_INTERNAL_KEY_VALUE(name, value) KeyCode::name,
#define OD_INTERNAL_MOUSE_VALUE(name, value) MouseCode::name,

		constexpr std::array s_KeyCodes = { OD_KEY_CODE_LIST(OD_INTERNAL_KEY_VALUE) };
		constexpr std::array s_MouseCodes = { OD_MOUSE_CODE_LIST(OD_INTERNAL_MOUSE_VALUE) };

#undef OD_INTERNAL_KEY_VALUE
#undef OD_INTERNAL_MOUSE_VALUE

		template<typename T, size_t N>
		constexpr bool IsStrictlyAscending(const std::array<T, N>& codes)
		{
			for (size_t i = 1; i < N; i++)
			{
				if (!(static_cast<uint16_t>(codes[i - 1]) < static_cast<uint16_t>(codes[i])))
					return false;
			}
			return true;
		}

		// Ascending order guarantees unique values and lets lookups by value use binary search.
		static_assert(IsStrictlyAscending(s_KeyCodes), "OD_KEY_CODE_LIST must be sorted by value without duplicates");
		static_assert(IsStrictlyAscending(s_MouseCodes), "OD_MOUSE_CODE_LIST must be sorted by value without duplicates");

		template<typename T, size_t N>
		const char* FindName(const std::array<CodeName<T>, N>& table, T code)
		{
			const auto it = std::lower_bound(table.begin(), table.end(), code, [](const CodeName<T>& entry, T value)
			{
				return static_cast<uint16_t>(entry.Code) < static_cast<uint16_t>(value);
			});

			if (it != table.end() && it->Code == code)
				return it->Name;
			return "Unknown";
		}

		template<typename T, size_t N>
		std::optional<T> FindCode(const std::array<CodeName<T>, N>& table, std::string_view name)
		{
			for (const CodeName<T>& entry : table)
			{
				if (StringUtils::EqualsIgnoreCase(name, entry.Name))
					return entry.Code;
			}
			return std::nullopt;
		}

	}

	const char* KeyCodeToString(KeyCode keyCode)
	{
		return FindName(s_KeyCodeNames, keyCode);
	}

	std::optional<KeyCode> KeyCodeFromString(std::string_view name)
	{
		return FindCode(s_KeyCodeNames, name);
	}

	std::span<const KeyCode> GetAllKeyCodes()
	{
		return s_KeyCodes;
	}

	const char* MouseCodeToString(MouseCode mouseCode)
	{
		return FindName(s_MouseCodeNames, mouseCode);
	}

	std::optional<MouseCode> MouseCodeFromString(std::string_view name)
	{
		return FindCode(s_MouseCodeNames, name);
	}

	std::span<const MouseCode> GetAllMouseCodes()
	{
		return s_MouseCodes;
	}

}

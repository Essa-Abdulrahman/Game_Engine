#pragma once

#include <string>
#include <string_view>

namespace Odyssey {

	// Locale-independent helpers. Case conversions only affect ASCII letters, so they are safe on UTF-8 text.
	class StringUtils
	{
	public:
		static bool EqualsIgnoreCase(std::string_view a, std::string_view b);
		static std::string ToLower(std::string_view text);
		static std::string ToUpper(std::string_view text);
		// Removes leading and trailing ASCII whitespace.
		static std::string_view Trim(std::string_view text);
	};

}

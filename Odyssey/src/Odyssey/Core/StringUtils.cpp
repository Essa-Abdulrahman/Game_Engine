#include "odpch.h"
#include "Odyssey/Core/StringUtils.h"

namespace Odyssey {

	namespace {

		constexpr char ToLowerASCII(char character)
		{
			return (character >= 'A' && character <= 'Z') ? static_cast<char>(character - 'A' + 'a') : character;
		}

		constexpr char ToUpperASCII(char character)
		{
			return (character >= 'a' && character <= 'z') ? static_cast<char>(character - 'a' + 'A') : character;
		}

		constexpr bool IsWhitespaceASCII(char character)
		{
			return character == ' ' || character == '\t' || character == '\n' || character == '\r' || character == '\f' || character == '\v';
		}

	}

	bool StringUtils::EqualsIgnoreCase(std::string_view a, std::string_view b)
	{
		if (a.size() != b.size())
			return false;

		for (size_t i = 0; i < a.size(); i++)
		{
			if (ToLowerASCII(a[i]) != ToLowerASCII(b[i]))
				return false;
		}
		return true;
	}

	std::string StringUtils::ToLower(std::string_view text)
	{
		std::string result(text);
		for (char& character : result)
			character = ToLowerASCII(character);
		return result;
	}

	std::string StringUtils::ToUpper(std::string_view text)
	{
		std::string result(text);
		for (char& character : result)
			character = ToUpperASCII(character);
		return result;
	}

	std::string_view StringUtils::Trim(std::string_view text)
	{
		size_t begin = 0;
		while (begin < text.size() && IsWhitespaceASCII(text[begin]))
			begin++;

		size_t end = text.size();
		while (end > begin && IsWhitespaceASCII(text[end - 1]))
			end--;

		return text.substr(begin, end - begin);
	}

}

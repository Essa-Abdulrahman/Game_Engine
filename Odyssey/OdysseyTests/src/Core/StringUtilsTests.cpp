#include <doctest/doctest.h>

#include "Odyssey/Core/StringUtils.h"

using namespace Odyssey;

TEST_CASE("StringUtils: EqualsIgnoreCase compares ASCII case-insensitively")
{
	CHECK(StringUtils::EqualsIgnoreCase("Space", "SPACE"));
	CHECK(StringUtils::EqualsIgnoreCase("", ""));
	CHECK_FALSE(StringUtils::EqualsIgnoreCase("Space", "Spac"));
	CHECK_FALSE(StringUtils::EqualsIgnoreCase("a", "b"));
	// Non-ASCII bytes are compared exactly.
	CHECK_FALSE(StringUtils::EqualsIgnoreCase("\xc3\xa9", "\xc3\x89"));
}

TEST_CASE("StringUtils: ToLower and ToUpper only change ASCII letters")
{
	CHECK(StringUtils::ToLower("Hello World 42!") == "hello world 42!");
	CHECK(StringUtils::ToUpper("Hello World 42!") == "HELLO WORLD 42!");
	CHECK(StringUtils::ToLower("\xc3\x89") == "\xc3\x89");
}

TEST_CASE("StringUtils: Trim removes surrounding whitespace only")
{
	CHECK(StringUtils::Trim("  value \t\r\n") == "value");
	CHECK(StringUtils::Trim("inner  space") == "inner  space");
	CHECK(StringUtils::Trim("   ").empty());
	CHECK(StringUtils::Trim("").empty());
}

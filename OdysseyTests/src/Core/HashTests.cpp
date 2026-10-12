#include <doctest/doctest.h>

#include "Odyssey/Core/Hash.h"

using namespace Odyssey;

// Reference values from the FNV specification test suite.
static_assert(Hash::FNV1a64("") == 0xcbf29ce484222325ull);
static_assert(Hash::FNV1a64("a") == 0xaf63dc4c8601ec8cull);
static_assert(Hash::FNV1a64("foobar") == 0x85944171f73967e8ull);

TEST_CASE("Hash: FNV-1a matches reference values at runtime")
{
	const std::string text = "foobar";
	CHECK(Hash::FNV1a64(text) == 0x85944171f73967e8ull);
	CHECK(Hash::FNV1a64("Odyssey") == 0xec1d483bec9e17e1ull);
}

TEST_CASE("Hash: FNV-1a treats bytes as unsigned and distinguishes similar strings")
{
	CHECK(Hash::FNV1a64("\xff") != Hash::FNV1a64("\x7f"));
	CHECK(Hash::FNV1a64("ab") != Hash::FNV1a64("ba"));
	CHECK(Hash::FNV1a64(std::string_view("a\0b", 3)) != Hash::FNV1a64("a"));
}

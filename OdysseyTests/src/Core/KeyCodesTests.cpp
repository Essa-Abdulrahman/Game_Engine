#include <doctest/doctest.h>

#include "Odyssey/Core/KeyCodes.h"

#include <set>
#include <string>

using namespace Odyssey;

// The platform layer casts GLFW values directly, so these must never change.
static_assert(static_cast<uint16_t>(KeyCode::Space) == 32);
static_assert(static_cast<uint16_t>(KeyCode::D0) == 48);
static_assert(static_cast<uint16_t>(KeyCode::A) == 65);
static_assert(static_cast<uint16_t>(KeyCode::Z) == 90);
static_assert(static_cast<uint16_t>(KeyCode::Escape) == 256);
static_assert(static_cast<uint16_t>(KeyCode::F25) == 314);
static_assert(static_cast<uint16_t>(KeyCode::KPEqual) == 336);
static_assert(static_cast<uint16_t>(KeyCode::Menu) == 348);
static_assert(static_cast<uint16_t>(MouseCode::Left) == 0);
static_assert(static_cast<uint16_t>(MouseCode::Right) == 1);
static_assert(static_cast<uint16_t>(MouseCode::Middle) == 2);
static_assert(static_cast<uint16_t>(MouseCode::Button7) == 7);

TEST_CASE("KeyCodes: every key code round-trips through its name")
{
	REQUIRE(GetAllKeyCodes().size() == 120);

	std::set<std::string> names;
	for (KeyCode keyCode : GetAllKeyCodes())
	{
		const std::string name = KeyCodeToString(keyCode);
		CHECK(name != "Unknown");
		names.insert(name);

		const std::optional<KeyCode> parsed = KeyCodeFromString(name);
		REQUIRE(parsed.has_value());
		CHECK(*parsed == keyCode);
	}
	CHECK(names.size() == GetAllKeyCodes().size());
}

TEST_CASE("KeyCodes: names match enumerators and parse case-insensitively")
{
	CHECK(std::string(KeyCodeToString(KeyCode::Space)) == "Space");
	CHECK(std::string(KeyCodeToString(KeyCode::D1)) == "D1");
	CHECK(std::string(KeyCodeToString(KeyCode::X)) == "X");
	CHECK(std::string(KeyCodeToString(KeyCode::KPEnter)) == "KPEnter");

	CHECK(KeyCodeFromString("space") == KeyCode::Space);
	CHECK(KeyCodeFromString("LEFTSHIFT") == KeyCode::LeftShift);
	CHECK(KeyCodeFromString("x") == KeyCode::X);
}

TEST_CASE("KeyCodes: unknown names and values")
{
	CHECK_FALSE(KeyCodeFromString("").has_value());
	CHECK_FALSE(KeyCodeFromString("NotAKey").has_value());
	CHECK(std::string(KeyCodeToString(static_cast<KeyCode>(1))) == "Unknown");
	CHECK(std::string(KeyCodeToString(static_cast<KeyCode>(1000))) == "Unknown");
}

TEST_CASE("KeyCodes: every mouse code round-trips through its name")
{
	REQUIRE(GetAllMouseCodes().size() == 8);
	for (MouseCode mouseCode : GetAllMouseCodes())
	{
		const std::optional<MouseCode> parsed = MouseCodeFromString(MouseCodeToString(mouseCode));
		REQUIRE(parsed.has_value());
		CHECK(*parsed == mouseCode);
	}

	CHECK(std::string(MouseCodeToString(MouseCode::Middle)) == "Middle");
	CHECK(MouseCodeFromString("left") == MouseCode::Left);
	CHECK_FALSE(MouseCodeFromString("Button8").has_value());
	CHECK(std::string(MouseCodeToString(static_cast<MouseCode>(42))) == "Unknown");
}

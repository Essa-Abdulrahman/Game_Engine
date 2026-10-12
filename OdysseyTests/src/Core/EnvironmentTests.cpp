#include <doctest/doctest.h>

#include "Odyssey/Core/Environment.h"
#include "Odyssey/Core/UUID.h"

using namespace Odyssey;

TEST_CASE("Environment: reads variables that are set")
{
	// PATH is defined on every supported platform (lookups are case-insensitive on Windows).
	const std::optional<std::string> path = Environment::GetVariable("PATH");
	REQUIRE(path.has_value());
	CHECK_FALSE(path->empty());
}

TEST_CASE("Environment: returns nullopt for variables that are not set")
{
	const std::string name = "ODYSSEY_TEST_UNSET_" + UUID().ToString();
	CHECK_FALSE(Environment::GetVariable(name).has_value());
}

#include <doctest/doctest.h>

#include "Odyssey/Core/CommandLine.h"

#include <vector>

using namespace Odyssey;

namespace {

	CommandLine Parse(const std::vector<const char*>& arguments)
	{
		return CommandLine(static_cast<int>(arguments.size()), arguments.data());
	}

}

TEST_CASE("CommandLine: empty input")
{
	const CommandLine defaultConstructed;
	CHECK(defaultConstructed.GetProgramName().empty());
	CHECK(defaultConstructed.GetPositionalArguments().empty());

	const CommandLine fromNull(0, nullptr);
	CHECK(fromNull.GetProgramName().empty());
	CHECK_FALSE(fromNull.HasOption("anything"));
}

TEST_CASE("CommandLine: program name, flags and values")
{
	const CommandLine commandLine = Parse({ "OdysseyEditor", "--headless", "--project=Games/Tetris", "--log-level=warn" });

	CHECK(commandLine.GetProgramName() == "OdysseyEditor");
	CHECK(commandLine.HasOption("headless"));
	CHECK_FALSE(commandLine.GetOptionValue("headless").has_value());
	CHECK(commandLine.GetOptionValue("project") == std::optional<std::string_view>("Games/Tetris"));
	CHECK(commandLine.GetOptionValue("log-level") == std::optional<std::string_view>("warn"));
	CHECK_FALSE(commandLine.HasOption("missing"));
	CHECK(commandLine.GetPositionalArguments().empty());
}

TEST_CASE("CommandLine: values may be empty or contain '='")
{
	const CommandLine commandLine = Parse({ "app", "--name=", "--expression=a=b" });

	CHECK(commandLine.GetOptionValue("name") == std::optional<std::string_view>(""));
	CHECK(commandLine.GetOptionValue("expression") == std::optional<std::string_view>("a=b"));
}

TEST_CASE("CommandLine: the last occurrence of an option wins")
{
	const CommandLine commandLine = Parse({ "app", "--level=1", "--level=2", "--level" });

	CHECK(commandLine.HasOption("level"));
	CHECK_FALSE(commandLine.GetOptionValue("level").has_value());

	const CommandLine values = Parse({ "app", "--level=1", "--level=2" });
	CHECK(values.GetOptionValue("level") == std::optional<std::string_view>("2"));
}

TEST_CASE("CommandLine: positional arguments and the -- terminator")
{
	const CommandLine commandLine = Parse({ "app", "Scene.json", "-v", "-", "--flag", "--", "--not-an-option", "last" });

	CHECK(commandLine.HasOption("flag"));
	CHECK_FALSE(commandLine.HasOption("not-an-option"));

	const std::vector<std::string>& positional = commandLine.GetPositionalArguments();
	REQUIRE(positional.size() == 5);
	CHECK(positional[0] == "Scene.json");
	CHECK(positional[1] == "-v");
	CHECK(positional[2] == "-");
	CHECK(positional[3] == "--not-an-option");
	CHECK(positional[4] == "last");
}

TEST_CASE("CommandLine: options without a name are kept as positional arguments")
{
	const CommandLine commandLine = Parse({ "app", "--=value" });
	REQUIRE(commandLine.GetPositionalArguments().size() == 1);
	CHECK(commandLine.GetPositionalArguments()[0] == "--=value");
}

TEST_CASE("CommandLine: option names are case-sensitive")
{
	const CommandLine commandLine = Parse({ "app", "--Headless" });
	CHECK(commandLine.HasOption("Headless"));
	CHECK_FALSE(commandLine.HasOption("headless"));
}

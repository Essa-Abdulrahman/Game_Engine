#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Odyssey {

	// Parses command line arguments with deliberately simple, unambiguous rules:
	//   --name          option without a value (a flag)
	//   --name=value    option with a value (the value may be empty)
	//   --              every following argument is positional
	//   anything else   positional argument (including "-x" and "-")
	// Option names are case-sensitive. When an option is repeated, the last occurrence wins.
	class CommandLine
	{
	public:
		CommandLine() = default;
		CommandLine(int argc, const char* const* argv);

		// argv[0], or empty if argc was 0.
		const std::string& GetProgramName() const { return m_ProgramName; }

		bool HasOption(std::string_view name) const;
		// Returns the value of --name=value. Returns nullopt if the option is absent or was given as a flag.
		std::optional<std::string_view> GetOptionValue(std::string_view name) const;

		const std::vector<std::string>& GetPositionalArguments() const { return m_PositionalArguments; }
	private:
		struct Option
		{
			std::string Name;
			std::optional<std::string> Value;
		};

		const Option* FindOption(std::string_view name) const;
	private:
		std::string m_ProgramName;
		std::vector<Option> m_Options;
		std::vector<std::string> m_PositionalArguments;
	};

}

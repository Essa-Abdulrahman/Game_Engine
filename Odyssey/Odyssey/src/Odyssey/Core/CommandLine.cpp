#include "odpch.h"
#include "Odyssey/Core/CommandLine.h"

namespace Odyssey {

	CommandLine::CommandLine(int argc, const char* const* argv)
	{
		if (argc <= 0 || argv == nullptr)
			return;

		m_ProgramName = argv[0] != nullptr ? argv[0] : "";

		bool onlyPositional = false;
		for (int i = 1; i < argc; i++)
		{
			if (argv[i] == nullptr)
				continue;

			const std::string_view argument = argv[i];
			if (onlyPositional || !argument.starts_with("--"))
			{
				m_PositionalArguments.emplace_back(argument);
				continue;
			}

			if (argument == "--")
			{
				onlyPositional = true;
				continue;
			}

			const std::string_view body = argument.substr(2);
			const size_t separator = body.find('=');
			const std::string_view name = body.substr(0, separator);
			if (name.empty())
			{
				// "--=value" has no name; keep it rather than silently dropping user input.
				m_PositionalArguments.emplace_back(argument);
				continue;
			}

			Option option;
			option.Name = std::string(name);
			if (separator != std::string_view::npos)
				option.Value = std::string(body.substr(separator + 1));
			m_Options.push_back(std::move(option));
		}
	}

	bool CommandLine::HasOption(std::string_view name) const
	{
		return FindOption(name) != nullptr;
	}

	std::optional<std::string_view> CommandLine::GetOptionValue(std::string_view name) const
	{
		const Option* option = FindOption(name);
		if (option == nullptr || !option->Value)
			return std::nullopt;

		return std::string_view(*option->Value);
	}

	const CommandLine::Option* CommandLine::FindOption(std::string_view name) const
	{
		for (auto it = m_Options.rbegin(); it != m_Options.rend(); ++it)
		{
			if (it->Name == name)
				return &*it;
		}
		return nullptr;
	}

}

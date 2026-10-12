#pragma once

#include <optional>
#include <string>

namespace Odyssey {

	class Environment
	{
	public:
		// Returns the value of an environment variable, or nullopt if it is not set. Thread-safe.
		static std::optional<std::string> GetVariable(const std::string& name);
	};

}

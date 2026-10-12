#include "odpch.h"
#include "Odyssey/Core/Environment.h"

#include <cstdlib>

namespace Odyssey {

	std::optional<std::string> Environment::GetVariable(const std::string& name)
	{
#if defined(OD_PLATFORM_WINDOWS)
		char* value = nullptr;
		size_t length = 0;
		if (_dupenv_s(&value, &length, name.c_str()) != 0 || value == nullptr)
			return std::nullopt;

		std::string result(value);
		std::free(value);
		return result;
#else
		// getenv is thread-safe as long as nothing modifies the environment concurrently; the engine never does.
		const char* value = std::getenv(name.c_str());
		if (value == nullptr)
			return std::nullopt;

		return std::string(value);
#endif
	}

}

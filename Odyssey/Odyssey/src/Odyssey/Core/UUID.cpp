#include "odpch.h"
#include "Odyssey/Core/UUID.h"

#include <charconv>
#include <format>
#include <limits>
#include <random>

namespace Odyssey {

	namespace {

		std::mt19937_64& GetThreadRandomEngine()
		{
			thread_local std::mt19937_64 s_Engine = []()
			{
				std::random_device device;
				std::seed_seq seed { device(), device(), device(), device(), device(), device(), device(), device() };
				return std::mt19937_64(seed);
			}();
			return s_Engine;
		}

	}

	UUID::UUID()
	{
		std::uniform_int_distribution<uint64_t> distribution(1, std::numeric_limits<uint64_t>::max());
		m_Value = distribution(GetThreadRandomEngine());
	}

	std::string UUID::ToString() const
	{
		return std::format("{:016x}", m_Value);
	}

	std::optional<UUID> UUID::FromString(std::string_view text)
	{
		if (text.size() != 16)
			return std::nullopt;

		uint64_t value = 0;
		const char* begin = text.data();
		const char* end = text.data() + text.size();
		const std::from_chars_result result = std::from_chars(begin, end, value, 16);
		if (result.ec != std::errc() || result.ptr != end)
			return std::nullopt;

		return UUID(value);
	}

}

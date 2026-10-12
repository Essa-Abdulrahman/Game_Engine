#include <doctest/doctest.h>

#include "Odyssey/Core/Timer.h"
#include "Odyssey/Core/Timestep.h"

#include <thread>

using namespace Odyssey;

TEST_CASE("Timestep: unit conversions")
{
	constexpr Timestep timestep = 0.016f;
	static_assert(timestep.GetSeconds() == 0.016f);

	CHECK(timestep.GetMilliseconds() == doctest::Approx(16.0));
	CHECK(static_cast<float>(timestep) == doctest::Approx(0.016));
	CHECK(Timestep().GetSeconds() == 0.0f);
}

TEST_CASE("Timer: measures at least the time slept and resets")
{
	Timer timer;
	std::this_thread::sleep_for(std::chrono::milliseconds(20));

	// sleep_for guarantees a minimum duration; only lower bounds are reliable on loaded CI machines.
	const float elapsedMillis = timer.ElapsedMillis();
	CHECK(elapsedMillis >= 19.0f);
	CHECK(timer.Elapsed() >= 0.019f);

	timer.Reset();
	CHECK(timer.ElapsedMillis() < elapsedMillis);
}

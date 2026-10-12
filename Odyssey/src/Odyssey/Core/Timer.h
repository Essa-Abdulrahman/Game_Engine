#pragma once

#include <chrono>

namespace Odyssey {

	// Measures elapsed wall time using a monotonic clock.
	class Timer
	{
	public:
		Timer() { Reset(); }

		void Reset() { m_Start = std::chrono::steady_clock::now(); }

		// Seconds since construction or the last Reset().
		float Elapsed() const
		{
			return std::chrono::duration<float>(std::chrono::steady_clock::now() - m_Start).count();
		}

		float ElapsedMillis() const
		{
			return std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - m_Start).count();
		}
	private:
		std::chrono::steady_clock::time_point m_Start;
	};

}

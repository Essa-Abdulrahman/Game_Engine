#pragma once

#include "Odyssey/Core/Base.h"

#include <cstdlib>
#include <format>
#include <string>

namespace Odyssey {

	struct AssertionFailure
	{
		const char* Expression = "";
		const char* File = "";
		int Line = 0;
		std::string Message;
		// True for OD_CORE_* (engine) assertions, false for OD_* (client) assertions.
		bool IsCore = true;
	};

	// Called when an assertion fails. Returns true if the caller should break into the debugger.
	// A handler may also throw (the unit tests do this to observe assertion failures).
	using AssertHandler = bool (*)(const AssertionFailure& failure);

	class Assert
	{
	public:
		// Thread-safe. Returns the previously installed handler.
		static AssertHandler SetHandler(AssertHandler handler);
		static AssertHandler GetHandler();

		// Logs the failure as critical and flushes all logs.
		static bool DefaultHandler(const AssertionFailure& failure);

		static bool ReportFailure(bool isCore, const char* expression, const char* file, int line, std::string message = {});

		template<typename... Args>
		static std::string FormatAssertMessage(std::format_string<Args...> format, Args&&... args)
		{
			return std::format(format, std::forward<Args>(args)...);
		}
	};

}

// The condition text is stringized at the outermost macro so it appears in the report exactly as written.
// Conditions containing top-level commas (eg. template argument lists) must be wrapped in parentheses.
#define OD_INTERNAL_REPORT_FAILURE(isCore, conditionText, ...) \
	::Odyssey::Assert::ReportFailure(isCore, conditionText, __FILE__, __LINE__ __VA_OPT__(, ::Odyssey::Assert::FormatAssertMessage(__VA_ARGS__)))

// Asserts (Debug and Release only): document and check assumptions. Execution may continue from the debugger.
#if defined(OD_ENABLE_ASSERTS)
	#define OD_INTERNAL_ASSERT_IMPL(isCore, condition, conditionText, ...) \
		do \
		{ \
			if (!(condition)) \
			{ \
				if (OD_INTERNAL_REPORT_FAILURE(isCore, conditionText __VA_OPT__(, __VA_ARGS__))) \
					OD_DEBUGBREAK(); \
			} \
		} while (false)
#else
	// The condition is not evaluated, but still referenced so variables used only in asserts do not trigger warnings.
	#define OD_INTERNAL_ASSERT_IMPL(isCore, condition, conditionText, ...) ((void)sizeof(!(condition)))
#endif

// Verifies (all configurations): invariants whose violation makes it unsafe to continue.
// The condition is always evaluated and a failure terminates the process after the handler returns.
#define OD_INTERNAL_VERIFY_IMPL(isCore, condition, conditionText, ...) \
	do \
	{ \
		if (!(condition)) \
		{ \
			if (OD_INTERNAL_REPORT_FAILURE(isCore, conditionText __VA_OPT__(, __VA_ARGS__))) \
				OD_DEBUGBREAK(); \
			::std::abort(); \
		} \
	} while (false)

#define OD_CORE_ASSERT(condition, ...) OD_INTERNAL_ASSERT_IMPL(true, condition, #condition __VA_OPT__(, __VA_ARGS__))
#define OD_ASSERT(condition, ...)      OD_INTERNAL_ASSERT_IMPL(false, condition, #condition __VA_OPT__(, __VA_ARGS__))
#define OD_CORE_VERIFY(condition, ...) OD_INTERNAL_VERIFY_IMPL(true, condition, #condition __VA_OPT__(, __VA_ARGS__))
#define OD_VERIFY(condition, ...)      OD_INTERNAL_VERIFY_IMPL(false, condition, #condition __VA_OPT__(, __VA_ARGS__))

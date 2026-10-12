#include "odpch.h"
#include "Odyssey/Core/Assert.h"

#include "Odyssey/Core/Log.h"

#include <atomic>

namespace Odyssey {

	namespace {

		std::atomic<AssertHandler>& GetHandlerStorage()
		{
			static std::atomic<AssertHandler> s_Handler = &Assert::DefaultHandler;
			return s_Handler;
		}

	}

	AssertHandler Assert::SetHandler(AssertHandler handler)
	{
		return GetHandlerStorage().exchange(handler != nullptr ? handler : &Assert::DefaultHandler);
	}

	AssertHandler Assert::GetHandler()
	{
		return GetHandlerStorage().load();
	}

	bool Assert::DefaultHandler(const AssertionFailure& failure)
	{
		Logger& logger = failure.IsCore ? Log::GetCoreLogger() : Log::GetClientLogger();
		if (failure.Message.empty())
			logger.Critical("Assertion failed: {} ({}:{})", failure.Expression, failure.File, failure.Line);
		else
			logger.Critical("Assertion failed: {} ({}:{}): {}", failure.Expression, failure.File, failure.Line, failure.Message);

		Log::Flush();
		return true;
	}

	bool Assert::ReportFailure(bool isCore, const char* expression, const char* file, int line, std::string message)
	{
		AssertionFailure failure;
		failure.Expression = expression;
		failure.File = file;
		failure.Line = line;
		failure.Message = std::move(message);
		failure.IsCore = isCore;

		return GetHandler()(failure);
	}

}

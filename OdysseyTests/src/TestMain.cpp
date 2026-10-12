#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include "Odyssey/Core/Environment.h"
#include "Odyssey/Core/Log.h"

int main(int argc, char** argv)
{
	// Tests deliberately trigger errors, so console output is opt-in (set ODYSSEY_TEST_LOG=1).
	// The in-memory sink is always enabled because tests inspect it.
	Odyssey::LogSpecification logSpecification;
	logSpecification.EnableConsole = Odyssey::Environment::GetVariable("ODYSSEY_TEST_LOG").has_value();
	logSpecification.ConsoleTarget = Odyssey::ConsoleStream::StdErr;
	logSpecification.MemoryCapacity = 1024;
	Odyssey::Log::Init(logSpecification);

	doctest::Context context;
	context.applyCommandLine(argc, argv);
	const int result = context.run();

	Odyssey::Log::Shutdown();
	return result;
}

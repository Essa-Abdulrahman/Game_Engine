#include <doctest/doctest.h>

#include "TestUtilities.h"

#include "Odyssey/Core/Assert.h"

#include <string>

using namespace Odyssey;

namespace {

	bool s_CustomHandlerCalled = false;

	bool RecordingHandler(const AssertionFailure&)
	{
		s_CustomHandlerCalled = true;
		return false;
	}

}

TEST_CASE("Assert: verify reports expression, location and formatted message")
{
	const int expected = 3;
	const int actual = 4;
	const auto failure = Testing::CaptureAssertionFailure([&]()
	{
		OD_CORE_VERIFY(actual == expected, "expected {} but got {}", expected, actual);
	});

	REQUIRE(failure.has_value());
	CHECK(std::string(failure->Expression) == "actual == expected");
	CHECK(std::string(failure->Message) == "expected 3 but got 4");
	CHECK(std::string(failure->File).find("AssertTests.cpp") != std::string::npos);
	CHECK(failure->Line > 0);
	CHECK(failure->IsCore);
}

TEST_CASE("Assert: verify without a message reports an empty message")
{
	const auto failure = Testing::CaptureAssertionFailure([]()
	{
		OD_VERIFY(1 + 1 == 3);
	});

	REQUIRE(failure.has_value());
	CHECK(failure->Message.empty());
	CHECK(std::string(failure->Expression) == "1 + 1 == 3");
	CHECK_FALSE(failure->IsCore);
}

TEST_CASE("Assert: passing conditions do not report and are evaluated exactly once")
{
	int evaluations = 0;
	const auto failure = Testing::CaptureAssertionFailure([&]()
	{
		OD_CORE_VERIFY(++evaluations == 1);
		OD_VERIFY(evaluations == 1, "unused message {}", evaluations);
	});

	CHECK_FALSE(failure.has_value());
	CHECK(evaluations == 1);
}

#if defined(OD_ENABLE_ASSERTS)

TEST_CASE("Assert: asserts report failures when enabled")
{
	const auto failure = Testing::CaptureAssertionFailure([]()
	{
		const int index = 7;
		OD_CORE_ASSERT(index < 5, "index {} out of range", index);
	});

	REQUIRE(failure.has_value());
	CHECK(failure->Message == "index 7 out of range");
	CHECK(failure->IsCore);
}

TEST_CASE("Assert: client asserts are flagged as non-core")
{
	const auto failure = Testing::CaptureAssertionFailure([]() { OD_ASSERT(false); });

	REQUIRE(failure.has_value());
	CHECK_FALSE(failure->IsCore);
}

#else

TEST_CASE("Assert: asserts do not evaluate their condition when disabled")
{
	int evaluations = 0;
	OD_CORE_ASSERT(++evaluations == 100);
	CHECK(evaluations == 0);
}

#endif

TEST_CASE("Assert: SetHandler returns the previous handler and null restores the default")
{
	const AssertHandler original = Assert::GetHandler();
	CHECK(original == &Assert::DefaultHandler);

	const AssertHandler previous = Assert::SetHandler(&RecordingHandler);
	CHECK(previous == original);
	CHECK(Assert::GetHandler() == &RecordingHandler);

	s_CustomHandlerCalled = false;
	CHECK_FALSE(Assert::ReportFailure(true, "expression", "file.cpp", 1, "message"));
	CHECK(s_CustomHandlerCalled);

	Assert::SetHandler(nullptr);
	CHECK(Assert::GetHandler() == &Assert::DefaultHandler);
	Assert::SetHandler(original);
}

TEST_CASE("Assert: capture restores the previous handler afterwards")
{
	const AssertHandler before = Assert::GetHandler();
	const auto failure = Testing::CaptureAssertionFailure([]() { OD_CORE_VERIFY(false); });
	CHECK(failure.has_value());
	CHECK(Assert::GetHandler() == before);
}

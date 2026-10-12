#include <doctest/doctest.h>

#include "TestUtilities.h"

#include "Odyssey/Core/Buffer.h"

#include <array>
#include <limits>
#include <string>
#include <type_traits>

using namespace Odyssey;

static_assert(!std::is_copy_constructible_v<Buffer>);
static_assert(!std::is_copy_assignable_v<Buffer>);
static_assert(std::is_nothrow_move_constructible_v<Buffer>);
static_assert(std::is_nothrow_move_assignable_v<Buffer>);

TEST_CASE("Buffer: default constructed buffer is empty")
{
	Buffer buffer;
	CHECK(buffer.IsEmpty());
	CHECK(buffer.GetSize() == 0);
	CHECK(buffer.GetData() == nullptr);
	CHECK_FALSE(static_cast<bool>(buffer));
	CHECK(buffer.GetSpan().empty());
}

TEST_CASE("Buffer: allocation, zeroing and release")
{
	Buffer buffer(64);
	REQUIRE(buffer.GetSize() == 64);
	REQUIRE(buffer.GetData() != nullptr);
	CHECK(static_cast<bool>(buffer));

	buffer.ZeroInitialize();
	bool allZero = true;
	for (uint8_t byte : buffer.GetSpan())
		allZero = allZero && byte == 0;
	CHECK(allZero);

	buffer.Allocate(0);
	CHECK(buffer.IsEmpty());
	CHECK(buffer.GetData() == nullptr);

	buffer.Allocate(8);
	CHECK(buffer.GetSize() == 8);
	buffer.Release();
	CHECK(buffer.IsEmpty());
}

TEST_CASE("Buffer: Copy and Clone produce independent deep copies")
{
	const std::string text = "odyssey";
	Buffer original = Buffer::Copy(text.data(), text.size());
	REQUIRE(original.GetSize() == text.size());

	Buffer clone = original.Clone();
	REQUIRE(clone.GetSize() == original.GetSize());
	CHECK(clone.GetData() != original.GetData());
	CHECK(std::string(clone.As<char>(), static_cast<size_t>(clone.GetSize())) == text);

	clone.GetData()[0] = 'O';
	CHECK(original.GetData()[0] == 'o');

	Buffer empty = Buffer::Copy(nullptr, 0);
	CHECK(empty.IsEmpty());
}

TEST_CASE("Buffer: Copy verifies that null data has zero size")
{
	const auto failure = Testing::CaptureAssertionFailure([]() { Buffer::Copy(nullptr, 4); });
	CHECK(failure.has_value());
}

TEST_CASE("Buffer: move transfers ownership and empties the source")
{
	Buffer source(16);
	const uint8_t* data = source.GetData();

	Buffer moved(std::move(source));
	CHECK(moved.GetData() == data);
	CHECK(moved.GetSize() == 16);
	CHECK(source.IsEmpty());
	CHECK(source.GetData() == nullptr);

	Buffer assigned;
	assigned = std::move(moved);
	CHECK(assigned.GetData() == data);
	CHECK(moved.IsEmpty());
}

TEST_CASE("Buffer: Write and Read values at offsets, including unaligned ones")
{
	Buffer buffer(32);
	buffer.ZeroInitialize();

	const uint32_t value = 0xA1B2C3D4u;
	buffer.Write(&value, sizeof(value), 1);
	CHECK(buffer.Read<uint32_t>(1) == value);

	const double number = 6.25;
	buffer.Write(&number, sizeof(number), 24);
	CHECK(buffer.Read<double>(24) == 6.25);

	CHECK(buffer.Read<uint8_t>(0) == 0);
}

TEST_CASE("Buffer: out-of-range access is verified")
{
	Buffer buffer(8);
	const std::array<uint8_t, 4> bytes = { 1, 2, 3, 4 };

	CHECK_FALSE(Testing::CaptureAssertionFailure([&]() { buffer.Write(bytes.data(), 4, 4); }).has_value());
	CHECK(Testing::CaptureAssertionFailure([&]() { buffer.Write(bytes.data(), 4, 5); }).has_value());
	CHECK(Testing::CaptureAssertionFailure([&]() { buffer.Write(bytes.data(), 4, std::numeric_limits<uint64_t>::max()); }).has_value());
	CHECK(Testing::CaptureAssertionFailure([&]() { (void)buffer.Read<uint64_t>(1); }).has_value());
	CHECK(Testing::CaptureAssertionFailure([&]() { buffer.Write(nullptr, 1, 0); }).has_value());

	// Zero-size writes are a no-op, even with null data.
	CHECK_FALSE(Testing::CaptureAssertionFailure([&]() { buffer.Write(nullptr, 0, 100); }).has_value());
}

#include <doctest/doctest.h>

#include "Odyssey/Core/UUID.h"

#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>

using namespace Odyssey;

TEST_CASE("UUID: generated identifiers are never null and do not repeat")
{
	std::unordered_set<UUID> identifiers;
	for (int i = 0; i < 10000; i++)
	{
		const UUID uuid;
		CHECK_FALSE(uuid.IsNull());
		identifiers.insert(uuid);
	}
	CHECK(identifiers.size() == 10000);
}

TEST_CASE("UUID: explicit values and the null identifier")
{
	constexpr UUID uuid(0x1234u);
	static_assert(uuid.GetValue() == 0x1234u);
	CHECK(static_cast<uint64_t>(uuid) == 0x1234u);
	CHECK_FALSE(uuid.IsNull());

	constexpr UUID null = UUID::Null();
	static_assert(null.IsNull());
	CHECK(null == UUID(0));
}

TEST_CASE("UUID: comparison operators order by value")
{
	const UUID small(1);
	const UUID large(2);
	CHECK(small < large);
	CHECK(large > small);
	CHECK(small != large);
	CHECK(small == UUID(1));
}

TEST_CASE("UUID: ToString produces 16 lowercase hex digits")
{
	CHECK(UUID(0).ToString() == "0000000000000000");
	CHECK(UUID(0xabcdefu).ToString() == "0000000000abcdef");
	CHECK(UUID(0xffffffffffffffffull).ToString() == "ffffffffffffffff");
}

TEST_CASE("UUID: FromString round-trips and accepts either case")
{
	for (int i = 0; i < 100; i++)
	{
		const UUID uuid;
		const std::optional<UUID> parsed = UUID::FromString(uuid.ToString());
		REQUIRE(parsed.has_value());
		CHECK(*parsed == uuid);
	}

	const std::optional<UUID> upper = UUID::FromString("00000000DEADBEEF");
	REQUIRE(upper.has_value());
	CHECK(upper->GetValue() == 0xdeadbeefull);

	const std::optional<UUID> null = UUID::FromString("0000000000000000");
	REQUIRE(null.has_value());
	CHECK(null->IsNull());
}

TEST_CASE("UUID: FromString rejects malformed input")
{
	CHECK_FALSE(UUID::FromString("").has_value());
	CHECK_FALSE(UUID::FromString("123").has_value());
	CHECK_FALSE(UUID::FromString("00000000000000000").has_value());
	CHECK_FALSE(UUID::FromString("000000000000000g").has_value());
	CHECK_FALSE(UUID::FromString("-000000000000001").has_value());
	CHECK_FALSE(UUID::FromString("0x00000000000001").has_value());
	CHECK_FALSE(UUID::FromString(" 000000000000001").has_value());
}

TEST_CASE("UUID: generation is thread-safe and unique across threads")
{
	constexpr int threadCount = 8;
	constexpr int perThread = 2000;

	std::mutex mutex;
	std::unordered_set<UUID> identifiers;
	std::vector<std::thread> threads;
	for (int thread = 0; thread < threadCount; thread++)
	{
		threads.emplace_back([&]()
		{
			std::vector<UUID> local;
			local.reserve(perThread);
			for (int i = 0; i < perThread; i++)
				local.emplace_back();

			std::scoped_lock lock(mutex);
			identifiers.insert(local.begin(), local.end());
		});
	}
	for (std::thread& thread : threads)
		thread.join();

	CHECK(identifiers.size() == static_cast<size_t>(threadCount * perThread));
}

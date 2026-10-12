#include <doctest/doctest.h>

#include "Odyssey/Core/Base.h"

#include <string>
#include <type_traits>

namespace {

	class Counter
	{
	public:
		explicit Counter(int start)
			: m_Value(start)
		{
		}

		int Add(int amount)
		{
			m_Value += amount;
			return m_Value;
		}

		auto GetBoundAdd() { return OD_BIND_EVENT_FN(Add); }
	private:
		int m_Value;
	};

}

TEST_CASE("Base: exactly one platform macro is defined")
{
	int platformCount = 0;
#if defined(OD_PLATFORM_WINDOWS)
	platformCount++;
#endif
#if defined(OD_PLATFORM_MACOS)
	platformCount++;
#endif
#if defined(OD_PLATFORM_LINUX)
	platformCount++;
#endif
	CHECK(platformCount == 1);
}

TEST_CASE("Base: OD_BIT produces single-bit unsigned masks")
{
	CHECK(OD_BIT(0) == 1u);
	CHECK(OD_BIT(1) == 2u);
	CHECK(OD_BIT(4) == 16u);
	CHECK(OD_BIT(31) == 0x80000000u);
	static_assert(std::is_unsigned_v<decltype(OD_BIT(3))>);
}

TEST_CASE("Base: CreateScope and CreateRef construct with forwarded arguments")
{
	Odyssey::Scope<std::string> scoped = Odyssey::CreateScope<std::string>(3, 'x');
	REQUIRE(scoped != nullptr);
	CHECK(*scoped == "xxx");

	Odyssey::Ref<std::string> shared = Odyssey::CreateRef<std::string>("odyssey");
	Odyssey::Ref<std::string> copy = shared;
	CHECK(*copy == "odyssey");
	CHECK(shared.use_count() == 2);
}

TEST_CASE("Base: OD_BIND_EVENT_FN binds a member function to the current object")
{
	Counter counter(10);
	auto add = counter.GetBoundAdd();
	CHECK(add(5) == 15);
	CHECK(add(-3) == 12);
}

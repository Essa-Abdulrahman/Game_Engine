#pragma once

#include <cstdint>
#include <memory>
#include <utility>

// ---------------------------------------------------------------------------------------------
// Platform detection
// ---------------------------------------------------------------------------------------------
#if defined(_WIN32)
	#if !defined(_WIN64)
		#error "Odyssey requires a 64-bit Windows target."
	#endif
	#define OD_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
	#include <TargetConditionals.h>
	#if TARGET_OS_OSX
		#define OD_PLATFORM_MACOS 1
	#else
		#error "Odyssey supports macOS only among Apple platforms."
	#endif
#elif defined(__linux__)
	#define OD_PLATFORM_LINUX 1
#else
	#error "Unsupported platform. Odyssey supports Windows, macOS and Linux."
#endif

static_assert(sizeof(void*) == 8, "Odyssey requires a 64-bit target.");

// ---------------------------------------------------------------------------------------------
// Build configuration (defined by CMake, exactly one of them must be set)
// ---------------------------------------------------------------------------------------------
#if (defined(OD_DEBUG) + defined(OD_RELEASE) + defined(OD_DIST)) != 1
	#error "Exactly one of OD_DEBUG, OD_RELEASE or OD_DIST must be defined (normally done by CMake)."
#endif

#if defined(OD_DEBUG) || defined(OD_RELEASE)
	#define OD_ENABLE_ASSERTS 1
#endif

#if defined(OD_DIST)
	#define OD_DEBUGBREAK() ((void)0)
#elif defined(OD_PLATFORM_WINDOWS)
	#define OD_DEBUGBREAK() __debugbreak()
#elif defined(__clang__)
	#define OD_DEBUGBREAK() __builtin_debugtrap()
#else
	#include <signal.h>
	#define OD_DEBUGBREAK() raise(SIGTRAP)
#endif

// ---------------------------------------------------------------------------------------------
// Utility macros
// ---------------------------------------------------------------------------------------------
#define OD_EXPAND_MACRO(x) x
#define OD_STRINGIFY_MACRO(x) #x

#define OD_BIT(x) (1u << (x))

// Binds a member function of the current object so it can be passed as a callback (eg. to an EventDispatcher).
#define OD_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace Odyssey {

	// Unique ownership. Prefer this over Ref<T> unless ownership is genuinely shared.
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	// Shared ownership.
	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}

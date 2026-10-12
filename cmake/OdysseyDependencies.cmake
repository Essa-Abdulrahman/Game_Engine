# Third-party dependencies. Policy (see AGENTS.md, "Dependencies"):
#   - Every dependency is pinned to an exact release tag (or commit) and listed in docs/Architecture.md.
#   - Dependencies are only added together with the engine code that uses them.
#   - Third-party code is consumed as SYSTEM includes and never gets Odyssey's warning flags.

include(FetchContent)

# doctest (unit tests). Header-only: we deliberately skip its CMakeLists.txt (SOURCE_SUBDIR points to a
# directory without one) so upstream build-script changes cannot affect us, and define our own target.
if (ODYSSEY_BUILD_TESTS)
	FetchContent_Declare(doctest
		GIT_REPOSITORY https://github.com/doctest/doctest.git
		GIT_TAG        v2.4.11
		GIT_SHALLOW    TRUE
		SOURCE_SUBDIR  odyssey-header-only)
	FetchContent_MakeAvailable(doctest)

	add_library(OdysseyDoctest INTERFACE)
	target_include_directories(OdysseyDoctest SYSTEM INTERFACE "${doctest_SOURCE_DIR}")
	add_library(doctest::doctest ALIAS OdysseyDoctest)
endif()

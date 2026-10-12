cmake_minimum_required(VERSION 3.25)

# Must be set before project() to take effect. std::format with floating point requires macOS 13.3.
if (APPLE AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
	set(CMAKE_OSX_DEPLOYMENT_TARGET "13.3" CACHE STRING "Minimum macOS version")
endif()

project(Odyssey
	VERSION 0.1.0
	DESCRIPTION "Odyssey 3D game engine"
	LANGUAGES C CXX)

if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not supported. Use a preset, eg. 'cmake --preset debug'.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ODYSSEY_BUILD_TESTS "Build the Odyssey unit tests" ON)
option(ODYSSEY_WARNINGS_AS_ERRORS "Treat compiler warnings as errors in Odyssey targets" ON)
option(ODYSSEY_ENABLE_SANITIZERS "Build Odyssey targets with AddressSanitizer and UndefinedBehaviorSanitizer (GCC/Clang)" OFF)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(OdysseyConfigurations)
include(OdysseyCompilerOptions)
include(OdysseyDependencies)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

add_subdirectory(Odyssey)

if (ODYSSEY_BUILD_TESTS)
	enable_testing()
	add_subdirectory(OdysseyTests)
endif()

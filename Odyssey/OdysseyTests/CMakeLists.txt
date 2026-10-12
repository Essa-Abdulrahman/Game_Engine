set(ODYSSEY_TEST_SOURCES
	src/TestMain.cpp
	src/TestUtilities.cpp
	src/TestUtilities.h

	src/Core/AssertTests.cpp
	src/Core/BaseTests.cpp
	src/Core/BufferTests.cpp
	src/Core/CommandLineTests.cpp
	src/Core/EnvironmentTests.cpp
	src/Core/FileSystemTests.cpp
	src/Core/HashTests.cpp
	src/Core/KeyCodesTests.cpp
	src/Core/LayerStackTests.cpp
	src/Core/LogTests.cpp
	src/Core/StringUtilsTests.cpp
	src/Core/TimeTests.cpp
	src/Core/UUIDTests.cpp

	src/Events/EventTests.cpp)

add_executable(OdysseyTests ${ODYSSEY_TEST_SOURCES})
odyssey_configure_target(OdysseyTests)

target_include_directories(OdysseyTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(OdysseyTests PRIVATE Odyssey doctest::doctest)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ODYSSEY_TEST_SOURCES})

# A single CTest entry runs the whole suite; doctest prints per-case results.
# Run a subset directly with: OdysseyTests --test-case="UUID*"
add_test(NAME OdysseyTests COMMAND OdysseyTests)
set_tests_properties(OdysseyTests PROPERTIES TIMEOUT 300)

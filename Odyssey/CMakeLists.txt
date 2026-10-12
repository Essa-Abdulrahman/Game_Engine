# Odyssey engine static library. Source files are listed explicitly; add new files here (see AGENTS.md).

set(ODYSSEY_SOURCES
	src/Odyssey.h
	src/odpch.h

	src/Odyssey/Core/Assert.cpp
	src/Odyssey/Core/Assert.h
	src/Odyssey/Core/Base.h
	src/Odyssey/Core/Buffer.cpp
	src/Odyssey/Core/Buffer.h
	src/Odyssey/Core/CommandLine.cpp
	src/Odyssey/Core/CommandLine.h
	src/Odyssey/Core/Environment.cpp
	src/Odyssey/Core/Environment.h
	src/Odyssey/Core/FileSystem.cpp
	src/Odyssey/Core/FileSystem.h
	src/Odyssey/Core/Hash.h
	src/Odyssey/Core/KeyCodes.cpp
	src/Odyssey/Core/KeyCodes.h
	src/Odyssey/Core/Layer.cpp
	src/Odyssey/Core/Layer.h
	src/Odyssey/Core/LayerStack.cpp
	src/Odyssey/Core/LayerStack.h
	src/Odyssey/Core/Log.cpp
	src/Odyssey/Core/Log.h
	src/Odyssey/Core/StringUtils.cpp
	src/Odyssey/Core/StringUtils.h
	src/Odyssey/Core/Timer.h
	src/Odyssey/Core/Timestep.h
	src/Odyssey/Core/UUID.cpp
	src/Odyssey/Core/UUID.h

	src/Odyssey/Events/ApplicationEvent.h
	src/Odyssey/Events/Event.h
	src/Odyssey/Events/KeyEvent.h
	src/Odyssey/Events/MouseEvent.h)

add_library(Odyssey STATIC ${ODYSSEY_SOURCES})
odyssey_configure_target(Odyssey)

target_include_directories(Odyssey PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_precompile_headers(Odyssey PRIVATE src/odpch.h)

find_package(Threads REQUIRED)
target_link_libraries(Odyssey PUBLIC Threads::Threads)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ODYSSEY_SOURCES})

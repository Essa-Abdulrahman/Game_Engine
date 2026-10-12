# Odyssey uses three build configurations:
#   Debug   - no optimization, asserts enabled             (OD_DEBUG)
#   Release - optimized with debug info, asserts enabled   (OD_RELEASE)
#   Dist    - optimized, asserts disabled; shipped builds  (OD_DIST)

set(ODYSSEY_CONFIGURATIONS Debug Release Dist)

get_property(ODYSSEY_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (ODYSSEY_IS_MULTI_CONFIG)
	set(CMAKE_CONFIGURATION_TYPES "${ODYSSEY_CONFIGURATIONS}" CACHE STRING "Build configurations" FORCE)
else()
	if (NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build configuration" FORCE)
	endif()
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${ODYSSEY_CONFIGURATIONS})
	if (NOT CMAKE_BUILD_TYPE IN_LIST ODYSSEY_CONFIGURATIONS)
		message(FATAL_ERROR "Unsupported CMAKE_BUILD_TYPE '${CMAKE_BUILD_TYPE}'. Use one of: ${ODYSSEY_CONFIGURATIONS}.")
	endif()
endif()

# Dist uses the toolchain's Release flags. The global Release flags are left untouched (debug information for
# first-party targets is added per target in odyssey_configure_target), so repeated configures are stable.
foreach (language C CXX)
	set(CMAKE_${language}_FLAGS_DIST "${CMAKE_${language}_FLAGS_RELEASE}" CACHE STRING "Flags for the Dist configuration")
endforeach()
foreach (kind EXE SHARED MODULE STATIC)
	set(CMAKE_${kind}_LINKER_FLAGS_DIST "${CMAKE_${kind}_LINKER_FLAGS_RELEASE}" CACHE STRING "Linker flags for the Dist configuration")
endforeach()

# MSVC: program database debug information for Debug and Release (policy CMP0141, CMake 3.25+).
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,Release>:ProgramDatabase>")

# Imported targets (eg. from find_package) usually only provide Release variants.
set(CMAKE_MAP_IMPORTED_CONFIG_DIST Release)

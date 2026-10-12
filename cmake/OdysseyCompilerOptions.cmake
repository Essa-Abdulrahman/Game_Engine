# Applies Odyssey's compiler settings to a first-party target. Never call this on third-party targets.
function(odyssey_configure_target target)
	target_compile_features(${target} PUBLIC cxx_std_20)

	target_compile_definitions(${target} PUBLIC
		$<$<CONFIG:Debug>:OD_DEBUG>
		$<$<CONFIG:Release>:OD_RELEASE>
		$<$<CONFIG:Dist>:OD_DIST>)

	if (MSVC)
		target_compile_options(${target} PRIVATE
			/W4
			/permissive-
			/Zc:preprocessor   # standard preprocessor, required for __VA_OPT__
			/Zc:__cplusplus
			/utf-8
			/MP)
		target_compile_definitions(${target} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
		target_link_options(${target} PRIVATE "$<$<CONFIG:Release>:/DEBUG;/OPT:REF;/OPT:ICF>")
		if (ODYSSEY_WARNINGS_AS_ERRORS)
			target_compile_options(${target} PRIVATE /WX)
		endif()
	else()
		target_compile_options(${target} PRIVATE
			-Wall
			-Wextra
			-Wpedantic
			-Wshadow
			-Wconversion
			-Wsign-conversion
			-Wnon-virtual-dtor
			-Woverloaded-virtual
			-Wcast-align
			-Wformat=2
			-Wimplicit-fallthrough
			$<$<CONFIG:Release>:-g>)
		if (ODYSSEY_WARNINGS_AS_ERRORS)
			target_compile_options(${target} PRIVATE -Werror)
		endif()

		if (ODYSSEY_ENABLE_SANITIZERS)
			target_compile_options(${target} PUBLIC -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
			target_link_options(${target} PUBLIC -fsanitize=address,undefined)
		endif()
	endif()
endfunction()

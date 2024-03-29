cmake_minimum_required (VERSION 3.0)
project("LavaCake-Helpers")

set_property(GLOBAL PROPERTY HELPERS_PATH ${CMAKE_CURRENT_LIST_DIR})

macro(clearShader)
	set(INPUT_SHADER)
	set(OUTPUT_SHADER)
	set(SPV_VERSION)
endmacro()

macro(addShader input output)
	set (variadic_args ${ARGN})
	list(LENGTH variadic_args variadic_count)
	set(INPUT_SHADER ${INPUT_SHADER} ${input})
	set(OUTPUT_SHADER ${OUTPUT_SHADER} ${output})
	
	if (${variadic_count} GREATER 0)
		list(GET variadic_args 0 version)
	else()
		set(version spv1.3)
	endif()

	set(SPV_VERSION ${SPV_VERSION} ${version})
	message(STATUS "Added new rule to convert ${input} to ${output} using ${version}")
endmacro()

set(CMAKE_VERBOSE_MAKEFILE on)

macro(AutoSPIRV TARGET_NAME)

	list(LENGTH INPUT_SHADER len1)
	
	if(${len1} GREATER_EQUAL 1)
		math(EXPR length "${len1} - 1")
		set(SHADERS_DEP)
		foreach(val RANGE ${length})
			list(GET INPUT_SHADER ${val} val1)
			list(GET OUTPUT_SHADER ${val} val2)
			list(GET SPV_VERSION ${val} val3)
			get_property(path GLOBAL PROPERTY HELPERS_PATH)

			set(OUTPUT_FILE ${val2})

			
			set(COMMANDS glslc ${val1} -g -o  ${val2} --target-spv=${val3})
			
			
			add_custom_command(
				OUTPUT ${OUTPUT_FILE}
				COMMAND ${COMMANDS}
				DEPENDS ${val1}
				COMMENT "AutoSPIRV ${COMMANDS}"
				VERBATIM
			)

			set(SHADERS_DEP ${SHADERS_DEP} ${OUTPUT_FILE})
		endforeach()
		add_custom_target(
			${TARGET_NAME}_SHADERS
			DEPENDS ${SHADERS_DEP}
		)
		set_source_files_properties(${SHADERS_DEP} PROPERTIES GENERATED TRUE)
		add_dependencies(${TARGET_NAME} ${TARGET_NAME}_SHADERS)
		set_property( TARGET ${TARGET_NAME}_SHADERS PROPERTY FOLDER "AutoSPIRV" )
	endif()
endmacro()
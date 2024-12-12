cmake_minimum_required (VERSION 3.5)
project("LavaCake-Helpers")

set_property(GLOBAL PROPERTY HELPERS_PATH ${CMAKE_CURRENT_LIST_DIR})

macro(clearShader)
	set(INPUT_SHADER)
	set(OUTPUT_SHADER)
	set(SPV_VERSION)
	set(INPUT_SHADER_SLANG)
	set(OUTPUT_SHADER_SLANG)
	set(STAGE_NAME_SLANG)
	set(STAGE_NUMBER_SLANG)
endmacro()

macro(addSlangShader input output)
	set (variadic_args ${ARGN})
	set(INPUT_SHADER_SLANG ${INPUT_SHADER_SLANG} ${input})
	set(OUTPUT_SHADER_SLANG ${OUTPUT_SHADER_SLANG} ${output})
	set(STAGE_NAME_SLANG ${STAGE_NAME_SLANG} ${variadic_args})
	list(LENGTH variadic_args lenStageName)
	set(STAGE_NUMBER_SLANG ${STAGE_NUMBER_SLANG} ${lenStageName})
	message(${lenStageName})
	message(STATUS "Added new rule to convert ${input} to ${output}")
endmacro()

set(CMAKE_VERBOSE_MAKEFILE on)

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


	#GLSL SHADERS
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

	#SLANG SHADERS
	list(LENGTH INPUT_SHADER_SLANG len1)
	list(LENGTH STAGE_NAME_SLANG len2)

	message("${STAGE_NUMBER_SLANG}")
	#message(${len2})
	set(count 0)
	if(${len1} GREATER_EQUAL 1)
		math(EXPR length "${len1} - 1")
		set(SHADERS_DEP)
		foreach(val RANGE ${length})
			list(GET INPUT_SHADER_SLANG ${val} val1)
			list(GET OUTPUT_SHADER_SLANG ${val} val2)

			list(GET STAGE_NUMBER_SLANG ${val} lenStageName )
			
			math(EXPR lenStageName "${lenStageName} /2 -1 ")

			set(stage_arg)


			foreach(index RANGE ${lenStageName})
				math(EXPR index1 "${count} + ${index} *2 ")
				math(EXPR index2 "${count} + ${index} *2 +1")
				
				list(GET STAGE_NAME_SLANG ${index1} stage)
				list(GET STAGE_NAME_SLANG ${index2} stage_name)


				set(stage_arg ${stage_arg} -entry ${stage_name} -stage ${stage})


			endforeach()

			math(EXPR count "${count} + (${lenStageName}+1) * 2")
			get_property(path GLOBAL PROPERTY HELPERS_PATH)

			set(OUTPUT_FILE ${val2})

			
			set(COMMANDS slangc ${val1} -o ${val2} -target spirv "-fvk-use-entrypoint-name" ${stage_arg})
			
			
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

#====================================================================================================================#
# Function: compile_shaders
# Description: 
#	If the shader is missing or has been modified (re)compile it, otherwise skip it
# Usage: 
#	compile_shaders(src_dir dest_dir)
function(compile_shaders SRC_DIR DEST_DIR)
	if(NOT SRC_DIR)
		message(SEND_ERROR "compile_shaders: SHADER DIRECTORY not specified")
		return()
	endif()

	if(NOT DEST_DIR)
		message(SEND_ERROR "compile_shaders: OUTPUT DIRECTORY not specified")
		return()
	endif()

	if (NOT Vulkan_glslc_FOUND)
		message(SEND_ERROR "compile_shaders: Vulkan glslc not found")
		return()
	endif()

	message(STATUS "compile_shaders: Using Vulkan glslc at ${Vulkan_GLSLC_EXECUTABLE}")
	
	file(GLOB_RECURSE shaders CONFIGURE_DEPENDS 
		${SRC_DIR}/*.vert 
		${SRC_DIR}/*.frag 
		${SRC_DIR}/*.geom 
		${SRC_DIR}/*.tesc 
		${SRC_DIR}/*.tese 
		${SRC_DIR}/*.comp
	)

	list(LENGTH shaders shaders_count)
	message(STATUS "compile_shaders: Compiling ${shaders_count} shaders from ${SRC_DIR} to ${DEST_DIR}")

	set(shader_log)
	foreach(shader IN LISTS shaders)
		get_filename_component(filename ${shader} NAME)
		get_filename_component(filename_we ${shader} NAME_WE)
		set(output_file ${DEST_DIR}/${filename_we}.spv)

		if(NOT EXISTS ${output_file} OR ${shader} IS_NEWER_THAN ${output_file})
			execute_process(
				COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${shader} -o ${output_file}
				OUTPUT_VARIABLE output
				RESULT_VARIABLE result
			)
			if(result)
				list(APPEND shader_log "compile_shaders: ❌ ${filename} -> ${filename_we}.spv (${result})")
				message(WARNING ${output})
			else()
				list(APPEND shader_log "compile_shaders: ✅ ${filename} -> ${filename_we}.spv (sucsess)")
			endif()
		else()
			list(APPEND shader_log "compile_shaders: ♻️ ${filename} -> ${filename_we}.spv (up to date)")
		endif()
	endforeach()

	foreach(_line IN LISTS _log)
		message(STATUS ${_line})
	endforeach()

	unset(shader_log)
endfunction()
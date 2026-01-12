cmake_minimum_required(VERSION 3.31.10)

project(Star-Dust LANGUAGES C)

set(BUILD_SHARED_LIBS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

set(EXTERNAL_SOURCE_DIR $ENV{External})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

if(MSVC)
	string(APPEND CMAKE_RUNTIME_OUTPUT_DIRECTORY "/bin")
	string(APPEND CMAKE_LIBRARY_OUTPUT_DIRECTORY "/lib")
	string(APPEND CMAKE_ARCHIVE_OUTPUT_DIRECTORY "/lib")
else()
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(CGLM_USE_TESTS OFF CACHE BOOL "Enable tests" FORCE)

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

set(SRC_FILES
	src/vulkan_api/utils/tools.c
	src/vulkan_api/context/context.c
	src/vulkan_api/presentation/main_view.c
	src/vulkan_api/pipeline/stages/shader/shader.c
	src/vulkan_api/pipeline/stages/uniform/descriptor_set_layout.c
	src/vulkan_api/pipeline/stages/vertex/vertex_input_state.c
	src/vulkan_api/pipeline/descriptors/descriptor_pool.c
	src/vulkan_api/pipeline/graphics_pipeline.c
	src/vulkan_api/command_pool/command_buffer_pool.c
	src/vulkan_api/sync/sync_manager.c
	src/vulkan_api/texture/texture2D.c
	src/vulkan_api/buffers/buffer_holder.c
	src/vulkan_api/render/render.c
	src/app/camera/camera.c
	src/app/application.c
	src/main.c
)

set(HDR_FILES
	src/vulkan_api/utils/tools.h
	src/vulkan_api/context/context.h
	src/vulkan_api/presentation/main_view.h
	src/vulkan_api/pipeline/stages/shader/shader.h
	src/vulkan_api/pipeline/stages/uniform/descriptor_set_layout.h
	src/vulkan_api/pipeline/stages/vertex/vertex_input_state.h
	src/vulkan_api/pipeline/descriptors/descriptor_pool.h
	src/vulkan_api/pipeline/graphics_pipeline.h
	src/vulkan_api/command_pool/command_buffer_pool.h
	src/vulkan_api/sync/sync_manager.h
	src/vulkan_api/texture/texture2D.h
	src/vulkan_api/buffers/buffer_holder.h
	src/vulkan_api/render/render.h
	src/app/camera/camera.h
	src/app/application.h
)

set(SHADER_FILES
	src/shaders/vertex_shader.vert
	src/shaders/fragment_shader.frag
)

source_group("shaders" FILES ${SHADER_FILES})

set(ALL_SOURCE_CODE_FILES
	${SRC_FILES} 
	${HDR_FILES}
)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)
include(${CMAKE_SOURCE_DIR}/src/cmake/compile_shaders.cmake)
compile_shaders(${CMAKE_SOURCE_DIR}/src/shaders ${CMAKE_BINARY_DIR}/shaders)

add_executable(${PROJECT_NAME} ${ALL_SOURCE_CODE_FILES})
target_sources(${PROJECT_NAME} PRIVATE ${SHADER_FILES})

add_subdirectory(${EXTERNAL_SOURCE_DIR}/glfw glfw)
add_subdirectory(${EXTERNAL_SOURCE_DIR}/cglm cglm)

target_include_directories(${PROJECT_NAME} PRIVATE
	${Vulkan_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/src
	${EXTERNAL_SOURCE_DIR}/stb
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	${Vulkan_LIBRARIES}
	glfw
	cglm
	$<$<BOOL:${UNIX}>:xcb>
	$<$<BOOL:${UNIX}>:m>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
	CGLM_USE_ANONYMOUS_STRUCT
	$<$<BOOL:${WIN32}>:VK_USE_PLATFORM_WIN32_KHR>
	$<$<BOOL:${WIN32}>:GLFW_EXPOSE_NATIVE_WIN32>
	$<$<BOOL:${UNIX}>:VK_USE_PLATFORM_XCB_KHR>
	$<$<BOOL:${UNIX}>:GLFW_EXPOSE_NATIVE_X11>
)

target_compile_features(${PROJECT_NAME} PRIVATE c_std_17)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res     $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/res/shaders
	VERBATIM
)

if(MSVC)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_SOURCE_CODE_FILES})
	set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})	
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
endif()
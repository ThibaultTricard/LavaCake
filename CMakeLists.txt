cmake_minimum_required (VERSION 3.0)
project( "LavaCake" )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set( CMAKE_SUPPRESS_REGENERATION true )

option(BUILD_LAVACAKE_EXAMPLES "Build LavaCake examples" true)

###############################################################
# Environment setup                                           #
###############################################################

if( CMAKE_BUILD_TYPE STREQUAL "" )
	set( CMAKE_BUILD_TYPE "debug" )
endif()

if( NOT DEFINED VK_USE_PLATFORM )
	if( WIN32 )
		set( VK_USE_PLATFORM "WIN32" )
	elseif( UNIX )
		set( VK_USE_PLATFORM "XCB" )
	endif()
endif()

if( VK_USE_PLATFORM STREQUAL "VK_USE_PLATFORM_WIN32_KHR" )
	set( CMAKE_C_FLAGS_RELWITHDEBINFO    "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MT" )
	set( CMAKE_C_FLAGS_RELEASE           "${CMAKE_C_FLAGS_RELEASE} /MT" )
	set( CMAKE_C_FLAGS_DEBUG             "${CMAKE_C_FLAGS_DEBUG} /MTd" )
	set( CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT" )
	set( CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_RELEASE} /MT" )
	set( CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} /MTd" )
	set( PLATFORM_LIBRARY                "" )
elseif( VK_USE_PLATFORM STREQUAL "XCB" )
	add_definitions( -std=c++11 )
	set( PLATFORM_LIBRARY dl xcb )
elseif( VK_USE_PLATFORM STREQUAL "XLIB" )
	add_definitions( -std=c++11 )
	set( PLATFORM_LIBRARY dl X11 )
endif()

if( CMAKE_BUILD_TYPE STREQUAL "debug" )
	add_definitions(-D_DEBUG)
endif()

#add_definitions( -DVK_NO_PROTOTYPES )
add_definitions( -DVK_USE_PLATFORM_${VK_USE_PLATFORM}_KHR )

###############################################################
# External Library                                            #
###############################################################
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(./External/lib/glfw/)
set(IMGUI_SOURCE 
./External/lib/imgui/imgui.h
./External/lib/imgui/imgui.cpp
./External/lib/imgui/imgui_draw.cpp
./External/lib/imgui/imgui_demo.cpp
./External/lib/imgui/imgui_widgets.cpp
)

###############################################################
# LavaCake Library                                            #
###############################################################

# External files
find_package(vulkan)

set(EXTERNAL_HEADER_FILES
./External/stb_image.h
./External/tiny_obj_loader.h
./External/vk_platform.h
./External/vulkan.h
./External/vulkan_core.h
./External/vulkan_win32.h
./External/lib/glfw/include/GLFW/glfw3.h
./External/lib/glfw/include/GLFW/glfw3native.h
)


set(LAVACAKE_INCLUDE_DIR
"${CMAKE_CURRENT_LIST_DIR}/External" "${CMAKE_CURRENT_LIST_DIR}/Library/Common Files" "${CMAKE_CURRENT_LIST_DIR}/Library/Source Files" "${CMAKE_CURRENT_LIST_DIR}/External/lib/glfw/include/GLFW"
)

source_group( "External" FILES ${EXTERNAL_HEADER_FILES}, ${VULKAN_HEADER_FILES} ${IMGUI_SOURCE} )

set(LIBRARY_COMMON_DIR "Library/Common Files")

set(LIBRARY_COMMON_HEADER_FILES
${LIBRARY_COMMON_DIR}/AllHeaders.h
${LIBRARY_COMMON_DIR}/Common.h
${LIBRARY_COMMON_DIR}/Tools.h
${LIBRARY_COMMON_DIR}/VulkanDestroyer.h
${LIBRARY_COMMON_DIR}/VulkanFunctions.h
${LIBRARY_COMMON_DIR}/ListOfVulkanFunctions.inl
)

set(LIBRARY_COMMON_SOURCE_FILES
${LIBRARY_COMMON_DIR}/Common.cpp
${LIBRARY_COMMON_DIR}/Tools.cpp
${LIBRARY_COMMON_DIR}/VulkanFunctions.cpp
)

source_group( "Library\\Common\\Header" FILES ${LIBRARY_COMMON_HEADER_FILES} )
source_group( "Library\\Common\\Source" FILES ${LIBRARY_COMMON_SOURCE_FILES} )


set(LIBRARY_HELPER_DIR "Library/Source Files/Helpers")
set(LIBRARY_HELPER_HEADER 
${LIBRARY_HELPER_DIR}/helpers.h
)

set(LIBRARY_HELPER_SOURCE 
${LIBRARY_HELPER_DIR}/helpers.cpp
)

source_group( "Library\\Helpers\\Header" FILES ${LIBRARY_HELPER_HEADER} )
source_group( "Library\\Helpers\\Source" FILES ${LIBRARY_HELPER_SOURCE} )

set(LIBRARY_CORE_DIR "Library/Source Files/Core")
set(LIBRARY_CORE_HEADER 
${LIBRARY_CORE_DIR}/VulkanBuffer.h
${LIBRARY_CORE_DIR}/VulkanCommand.h
${LIBRARY_CORE_DIR}/VulkanDescriptor.h
${LIBRARY_CORE_DIR}/VulkanDevice.h
${LIBRARY_CORE_DIR}/VulkanFence.h
${LIBRARY_CORE_DIR}/VulkanImage.h
${LIBRARY_CORE_DIR}/VulkanInstance.h
${LIBRARY_CORE_DIR}/VulkanLoader.h
${LIBRARY_CORE_DIR}/VulkanMemory.h
${LIBRARY_CORE_DIR}/VulkanPresentation.h
${LIBRARY_CORE_DIR}/VulkanQueue.h
${LIBRARY_CORE_DIR}/VulkanSemaphore.h
${LIBRARY_CORE_DIR}/VulkanSwapChain.h
${LIBRARY_CORE_DIR}/VulkanViewport.h
)

set(LIBRARY_CORE_SOURCE 
${LIBRARY_CORE_DIR}/VulkanBuffer.cpp
${LIBRARY_CORE_DIR}/VulkanCommand.cpp
${LIBRARY_CORE_DIR}/VulkanDescriptor.cpp
${LIBRARY_CORE_DIR}/VulkanDevice.cpp
${LIBRARY_CORE_DIR}/VulkanFence.cpp
${LIBRARY_CORE_DIR}/VulkanImage.cpp
${LIBRARY_CORE_DIR}/VulkanInstance.cpp
${LIBRARY_CORE_DIR}/VulkanLoader.cpp
${LIBRARY_CORE_DIR}/VulkanMemory.cpp
${LIBRARY_CORE_DIR}/VulkanPresentation.cpp
${LIBRARY_CORE_DIR}/VulkanQueue.cpp
${LIBRARY_CORE_DIR}/VulkanSemaphore.cpp
${LIBRARY_CORE_DIR}/VulkanSwapChain.cpp
${LIBRARY_CORE_DIR}/VulkanViewport.cpp
)


source_group( "Library\\Core\\Header" FILES ${LIBRARY_CORE_HEADER} )
source_group( "Library\\Core\\Source" FILES ${LIBRARY_CORE_SOURCE} )

set(LIBRARY_FRAMEWORK_DIR "Library/Source Files/Framework")

set(LIBRARY_FRAMEWORK_HEADER 
${LIBRARY_FRAMEWORK_DIR}/CommandBuffer.h
${LIBRARY_FRAMEWORK_DIR}/ComputePipeline.h
${LIBRARY_FRAMEWORK_DIR}/Constant.h
${LIBRARY_FRAMEWORK_DIR}/Device.h
${LIBRARY_FRAMEWORK_DIR}/ErrorCheck.h
${LIBRARY_FRAMEWORK_DIR}/Framework.h
${LIBRARY_FRAMEWORK_DIR}/GraphicPipeline.h
${LIBRARY_FRAMEWORK_DIR}/ImGUIWrapper.h
${LIBRARY_FRAMEWORK_DIR}/Pipeline.h
${LIBRARY_FRAMEWORK_DIR}/Queue.h
${LIBRARY_FRAMEWORK_DIR}/RenderPass.h
${LIBRARY_FRAMEWORK_DIR}/ShaderModule.h
${LIBRARY_FRAMEWORK_DIR}/SwapChain.h
${LIBRARY_FRAMEWORK_DIR}/Texture.h
${LIBRARY_FRAMEWORK_DIR}/UniformBuffer.h
${LIBRARY_FRAMEWORK_DIR}/VertexBuffer.h
${LIBRARY_FRAMEWORK_DIR}/Window.h
)

set(LIBRARY_FRAMEWORK_SOURCE
${LIBRARY_FRAMEWORK_DIR}/CommandBuffer.cpp
${LIBRARY_FRAMEWORK_DIR}/ComputePipeline.cpp
${LIBRARY_FRAMEWORK_DIR}/Constant.cpp
${LIBRARY_FRAMEWORK_DIR}/Device.cpp
${LIBRARY_FRAMEWORK_DIR}/ErrorCheck.cpp
${LIBRARY_FRAMEWORK_DIR}/GraphicPipeline.cpp
${LIBRARY_FRAMEWORK_DIR}/ImGUIWrapper.cpp
${LIBRARY_FRAMEWORK_DIR}/Pipeline.cpp
${LIBRARY_FRAMEWORK_DIR}/RenderPass.cpp
${LIBRARY_FRAMEWORK_DIR}/SwapChain.cpp
${LIBRARY_FRAMEWORK_DIR}/Texture.cpp
${LIBRARY_FRAMEWORK_DIR}/UniformBuffer.cpp
${LIBRARY_FRAMEWORK_DIR}/VertexBuffer.cpp
${LIBRARY_FRAMEWORK_DIR}/Window.cpp
)

source_group( "Library\\Framework\\Header" FILES ${LIBRARY_FRAMEWORK_HEADER} )
source_group( "Library\\Framework\\Source" FILES ${LIBRARY_FRAMEWORK_SOURCE} )



# Recipes Library generation
add_library( LavaCake ${EXTERNAL_HEADER_FILES} ${LIBRARY_COMMON_HEADER_FILES} ${LIBRARY_COMMON_SOURCE_FILES} ${LIBRARY_HELPER_HEADER} ${LIBRARY_HELPER_SOURCE}  ${LIBRARY_CORE_HEADER} ${LIBRARY_CORE_SOURCE} ${LIBRARY_FRAMEWORK_HEADER} ${LIBRARY_FRAMEWORK_SOURCE} ${IMGUI_SOURCE})
target_link_libraries( LavaCake ${PLATFORM_LIBRARY} glfw )
target_include_directories( LavaCake PUBLIC ${LAVACAKE_INCLUDE_DIR} )




###############################################################
# Sample projects                                             #
###############################################################

if (BUILD_LAVACAKE_EXAMPLES)
# Sample projects generation
macro( list_samples )
		file( GLOB SAMPLES RELATIVE "${CMAKE_CURRENT_LIST_DIR}/Samples/Source Files" "${CMAKE_CURRENT_LIST_DIR}/Samples/Source Files/*" )

		FOREACH( SAMPLE ${SAMPLES} )
			file( GLOB SAMPLE_SOURCE_FILES "Samples/Source Files/${SAMPLE}/*.cpp" )
			source_group( "" FILES ${SAMPLE_SOURCE_FILES} )

			add_executable( ${SAMPLE} ${SAMPLE_SOURCE_FILES} )
			target_link_libraries( ${SAMPLE} LavaCake )
			#target_include_directories( ${SAMPLE} PUBLIC ${LAVACAKE_INCLUDE_DIR} )
			set_property( TARGET ${SAMPLE} PROPERTY FOLDER "Samples" )
		ENDFOREACH()
endmacro()

# Sample projects generation
list_samples()
file( COPY "${CMAKE_CURRENT_LIST_DIR}/Samples/Data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}" )
endif()



cmake_minimum_required (VERSION 3.0)
project("LavaCake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set( CMAKE_SUPPRESS_REGENERATION true )



###############################################################
# Environment setup                                           #
###############################################################

find_package(Vulkan REQUIRED)

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

if ( APPLE )
	add_definitions( -DVK_USE_PLATFORM_MACOS_MVK )
	add_definitions( -DVK_USE_PLATFORM_METAL_EXT )

else()

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

	#add_definitions( -DVK_NO_PROTOTYPES )
	add_definitions( -DVK_USE_PLATFORM_${VK_USE_PLATFORM}_KHR )
endif()

if( CMAKE_BUILD_TYPE STREQUAL "debug" )
	add_definitions(-D_DEBUG)
endif()


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
# Helpers                                                     #
###############################################################

add_subdirectory(Helpers)

###############################################################
# LavaCake Library                                            #
###############################################################

# External files
find_package(Vulkan)

set(EXTERNAL_HEADER_FILES
./External/stb_image.h
./External/tiny_obj_loader.h
./External/lib/glfw/include/GLFW/glfw3.h
./External/lib/glfw/include/GLFW/glfw3native.h 
)



set(VULKAN_HEADER_FILES
${Vulkan_INCLUDE_DIRS}/vulkan/vk_platform.h
${Vulkan_INCLUDE_DIRS}/vulkan/vulkan.h
${Vulkan_INCLUDE_DIRS}/vulkan/vulkan_core.h
)

if(WIN32)
	set(VULKAN_HEADER_FILES
		${VULKAN_HEADER_FILES}
		${Vulkan_INCLUDE_DIRS}/vulkan/vulkan_win32.h
	)
endif()

if(APPLE)
	set(VULKAN_HEADER_FILES
		${VULKAN_HEADER_FILES}
		${Vulkan_INCLUDE_DIRS}/vulkan/vulkan_metal.h
		${Vulkan_INCLUDE_DIRS}/vulkan/mvk_datatypes.h
		${Vulkan_INCLUDE_DIRS}/vulkan/mvk_vulkan.h
		${Vulkan_INCLUDE_DIRS}/vulkan/vk_mvk_moltenvk.h
	)
endif()


message(${Vulkan_INCLUDE_DIRS})

set(LAVACAKE_INCLUDE_DIR
"${CMAKE_CURRENT_LIST_DIR}/External" "${CMAKE_CURRENT_LIST_DIR}/Library/Common Files" "${CMAKE_CURRENT_LIST_DIR}/Library/Source Files" "${CMAKE_CURRENT_LIST_DIR}/External/lib/glfw/include/GLFW"
)

source_group( "External" FILES ${EXTERNAL_HEADER_FILES} ${VULKAN_HEADER_FILES} ${IMGUI_SOURCE} )

set(LIBRARY_COMMON_DIR "Library/Common Files")

set(LIBRARY_COMMON_HEADER_FILES
${LIBRARY_COMMON_DIR}/AllHeaders.h
${LIBRARY_COMMON_DIR}/Common.h
${LIBRARY_COMMON_DIR}/Tools.h
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
${LIBRARY_HELPER_DIR}/Field.h
${LIBRARY_HELPER_DIR}/ABBox.h
)

set(LIBRARY_HELPER_SOURCE 
${LIBRARY_HELPER_DIR}/helpers.cpp
)

source_group( "Library\\Helpers\\Header" FILES ${LIBRARY_HELPER_HEADER} )
source_group( "Library\\Helpers\\Source" FILES ${LIBRARY_HELPER_SOURCE} )

set(LIBRARY_CORE_DIR "Library/Source Files/Core")
set(LIBRARY_CORE_HEADER 
${LIBRARY_CORE_DIR}/VulkanLoader.h
)

set(LIBRARY_CORE_SOURCE 
${LIBRARY_CORE_DIR}/VulkanLoader.cpp
)


source_group( "Library\\Core\\Header" FILES ${LIBRARY_CORE_HEADER} )
source_group( "Library\\Core\\Source" FILES ${LIBRARY_CORE_SOURCE} )

set(LIBRARY_FRAMEWORK_DIR "Library/Source Files/Framework")

set(LIBRARY_FRAMEWORK_HEADER 
${LIBRARY_FRAMEWORK_DIR}/Buffer.h
${LIBRARY_FRAMEWORK_DIR}/CommandBuffer.h
${LIBRARY_FRAMEWORK_DIR}/ComputePipeline.h
${LIBRARY_FRAMEWORK_DIR}/Constant.h
${LIBRARY_FRAMEWORK_DIR}/Device.h
${LIBRARY_FRAMEWORK_DIR}/DescriptorSet.h
${LIBRARY_FRAMEWORK_DIR}/ErrorCheck.h
${LIBRARY_FRAMEWORK_DIR}/Framework.h
${LIBRARY_FRAMEWORK_DIR}/GraphicPipeline.h
${LIBRARY_FRAMEWORK_DIR}/Image.h
${LIBRARY_FRAMEWORK_DIR}/ImGuiWrapper.h
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
${LIBRARY_FRAMEWORK_DIR}/Buffer.cpp
${LIBRARY_FRAMEWORK_DIR}/CommandBuffer.cpp
${LIBRARY_FRAMEWORK_DIR}/ComputePipeline.cpp
${LIBRARY_FRAMEWORK_DIR}/Constant.cpp
${LIBRARY_FRAMEWORK_DIR}/Device.cpp
${LIBRARY_FRAMEWORK_DIR}/DescriptorSet.h
${LIBRARY_FRAMEWORK_DIR}/ErrorCheck.cpp
${LIBRARY_FRAMEWORK_DIR}/GraphicPipeline.cpp
${LIBRARY_FRAMEWORK_DIR}/Image.cpp
${LIBRARY_FRAMEWORK_DIR}/ImGuiWrapper.cpp
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


set(LIBRARY_RAYTRACING_DIR "Library/Source Files/RayTracing")


set(LIBRARY_RAYTRACING_HEADER 
${LIBRARY_RAYTRACING_DIR}/BottomLevelAS.h
${LIBRARY_RAYTRACING_DIR}/RayTracingPipeline.h
${LIBRARY_RAYTRACING_DIR}/ShaderBindingTable.h
${LIBRARY_RAYTRACING_DIR}/RayTracingShaderModule.h
${LIBRARY_RAYTRACING_DIR}/TopLevelAS.h
)

set(LIBRARY_RAYTRACING_SOURCE
${LIBRARY_RAYTRACING_DIR}/BottomLevelAS.cpp
${LIBRARY_RAYTRACING_DIR}/RayTracingPipeline.cpp
${LIBRARY_RAYTRACING_DIR}/ShaderBindingTable.cpp
${LIBRARY_RAYTRACING_DIR}/RayTracingShaderModule.cpp
${LIBRARY_RAYTRACING_DIR}/TopLevelAS.cpp
)

source_group( "Library\\RayTracing\\Header" FILES ${LIBRARY_RAYTRACING_HEADER} )
source_group( "Library\\RayTracing\\Source" FILES ${LIBRARY_RAYTRACING_SOURCE} )


set(LIBRARY_GEOMETRY_DIR "Library/Source Files/Geometry")

set(LIBRARY_GEOMETRY_HEADER 
${LIBRARY_GEOMETRY_DIR}/format.h
${LIBRARY_GEOMETRY_DIR}/mesh.h
${LIBRARY_GEOMETRY_DIR}/meshLoader.h
${LIBRARY_GEOMETRY_DIR}/meshExporter.h
${LIBRARY_GEOMETRY_DIR}/computationalMesh.h
)

set(LIBRARY_GEOMETRY_SOURCE
)

source_group( "Library\\Geometry\\Header" FILES ${LIBRARY_GEOMETRY_HEADER} )
source_group( "Library\\Geometry\\Source" FILES ${LIBRARY_GEOMETRY_SOURCE} )


set(LIBRARY_MATH_DIR "Library/Source Files/Math")

set(LIBRARY_MATH_HEADER
  ${LIBRARY_MATH_DIR}/basics.h
  )
  
set(LIBRARY_MATH_SOURCE
  ${LIBRARY_MATH_DIR}/basics.cpp
  )

source_group( "Library\\Math\\Header" FILES ${LIBRARY_MATH_HEADER} )
source_group( "Library\\Math\\Source" FILES ${LIBRARY_MATH_SOURCE} )



# Recipes Library generation
add_library( LavaCake STATIC 
${EXTERNAL_HEADER_FILES} 
${LIBRARY_COMMON_HEADER_FILES} ${LIBRARY_COMMON_SOURCE_FILES} 
${LIBRARY_HELPER_HEADER} ${LIBRARY_HELPER_SOURCE}  
${LIBRARY_CORE_HEADER} ${LIBRARY_CORE_SOURCE}
${LIBRARY_MATH_HEADER} ${LIBRARY_MATH_SOURCE}
${LIBRARY_FRAMEWORK_HEADER} ${LIBRARY_FRAMEWORK_SOURCE} 
${LIBRARY_RAYTRACING_HEADER} ${LIBRARY_RAYTRACING_SOURCE} 
${LIBRARY_GEOMETRY_HEADER} ${LIBRARY_GEOMETRY_SOURCE} 
${IMGUI_SOURCE})
target_link_libraries( LavaCake ${PLATFORM_LIBRARY} ${Vulkan_LIBRARY} glfw )
target_include_directories( LavaCake PUBLIC ${LAVACAKE_INCLUDE_DIR} ${Vulkan_INCLUDE_DIRS})




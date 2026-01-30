cmake_minimum_required(VERSION 3.13)
project(LavaCake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(LAVACAKE_BUILD_EXAMPLES "Build LavaCake examples (requires GLFW)" ON)


if(APPLE)
    # Force Clang if GCC is detected
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(WARNING "GCC detected. Switching to Clang for macOS compatibility.")
        set(CMAKE_C_COMPILER "/usr/bin/clang")
        set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
    endif()
    
    # Use libc++ (required for macOS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++")

    # Suppress nullability warnings from external libraries
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nullability-completeness")
endif()

find_package(Vulkan REQUIRED)



include(FetchContent)

FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG master
)

FetchContent_MakeAvailable(VulkanMemoryAllocator)

# === SHADERC DETECTION ===

#TODO Try to find shaderc locally
set(SHADERC_FOUND false)


# If shaderc not found, fetch it from source
if(NOT SHADERC_FOUND)
    message(STATUS "shaderc not found locally. Fetching from source...")
    
    # Fetch SPIRV-Headers (required by SPIRV-Tools)
    FetchContent_Declare(
        spirv-headers
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG main
        GIT_SHALLOW TRUE
    )
    
    # Fetch SPIRV-Tools (required by shaderc)
    FetchContent_Declare(
        spirv-tools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG main
        GIT_SHALLOW TRUE
    )

    # Fetch glslang (required by shaderc)
    FetchContent_Declare(
        glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG main
        GIT_SHALLOW TRUE
    )

    # Configure build options for dependencies
    set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
    set(SPIRV_HEADERS_SKIP_INSTALL ON CACHE BOOL "" FORCE)
    
    set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
    set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
    set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
    
    set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "" FORCE)
    set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
    set(ENABLE_SPVREMAPPER OFF CACHE BOOL "" FORCE)
    set(ENABLE_CTEST OFF CACHE BOOL "" FORCE)
    
    # Make dependencies available
    FetchContent_MakeAvailable(spirv-headers spirv-tools glslang)


    FetchContent_Declare(
        shaderc
        GIT_REPOSITORY https://github.com/google/shaderc.git
        GIT_TAG main
        GIT_SHALLOW TRUE
    )
    
    # Configure shaderc build options
    set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
    set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
    set(SHADERC_SKIP_INSTALL OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(shaderc)

    # Get the source directory for shaderc includes
    FetchContent_GetProperties(shaderc SOURCE_DIR SHADERC_SOURCE_DIR)
    set(SHADERC_INCLUDE_DIRS "${SHADERC_SOURCE_DIR}/libshaderc/include")
    
    set(SHADERC_LIB shaderc_combined)
    message(STATUS "Using fetched shaderc with dependencies")
    message(STATUS "  Include dir: ${SHADERC_INCLUDE_DIRS}")
endif()


# === LavaCake Library ===
file(GLOB_RECURSE LAVACAKE_HEADERS "Library/LavaCake/*.hpp" "Library/LavaCake/*.h")
add_library(LavaCake INTERFACE)
target_include_directories(LavaCake INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Library>
    $<BUILD_INTERFACE:${Vulkan_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${vmahpp_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
# Note: shaderc includes are not added to the interface as they're in the build directory
# Users will need to have their own shaderc installation
target_compile_definitions(LavaCake INTERFACE VMA_VULKAN_HPP)
target_link_libraries(LavaCake INTERFACE
    Vulkan::Vulkan
    $<BUILD_INTERFACE:VulkanMemoryAllocator>
    $<BUILD_INTERFACE:${SHADERC_LIB}>
)

# Add shaderc includes to interface (only when fetched from source)
if(DEFINED SHADERC_INCLUDE_DIRS)
    target_include_directories(LavaCake INTERFACE
        $<BUILD_INTERFACE:${SHADERC_INCLUDE_DIRS}>
    )
endif()

# Note: INSTALL_INTERFACE for shaderc is already covered by CMAKE_INSTALL_INCLUDEDIR above

# Create an alias for consistent usage
add_library(LavaCake::LavaCake ALIAS LavaCake)




# === Installation ===
include(GNUInstallDirs)

install(TARGETS LavaCake
    EXPORT LavaCakeTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY Library/LavaCake
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

# Install VulkanMemoryAllocator headers
FetchContent_GetProperties(VulkanMemoryAllocator SOURCE_DIR VMA_SOURCE_DIR)
install(FILES ${VMA_SOURCE_DIR}/include/vk_mem_alloc.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install shaderc headers
if(DEFINED SHADERC_INCLUDE_DIRS)
    install(DIRECTORY ${SHADERC_INCLUDE_DIRS}/shaderc
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
    )
endif()

install(EXPORT LavaCakeTargets
    FILE LavaCakeTargets.cmake
    NAMESPACE LavaCake::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LavaCake
)

# Create and install package config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/LavaCakeConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LavaCakeConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LavaCake
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/LavaCakeConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LavaCake
)




# Add examples subdirectory (optional)
if(LAVACAKE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

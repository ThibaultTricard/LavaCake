cmake_minimum_required(VERSION 3.10)
project(VulkanHPP_FullExample)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


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
find_package(glfw3 REQUIRED)


include(FetchContent)

FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG master
)

FetchContent_MakeAvailable(VulkanMemoryAllocator)

if(DEFINED SHADERC_ROOT)
    # Use custom shaderc build
    find_library(SHADERC_LIB
        NAMES shaderc_combined
        PATHS ${SHADERC_ROOT}/lib
        NO_DEFAULT_PATH
        REQUIRED
    )
    
    include_directories(${SHADERC_ROOT}/include)
    message(STATUS "Using custom shaderc: ${SHADERC_LIB}")
else()
    # === OPTION 2: Try system shaderc ===
    find_package(unofficial-shaderc CONFIG QUIET)
    
    if(unofficial-shaderc_FOUND)
        set(SHADERC_LIB unofficial::shaderc::shaderc)
        message(STATUS "Using system shaderc package")
    else()
        # Try to find in Vulkan SDK
        find_library(SHADERC_LIB 
            NAMES shaderc_combined shaderc
            PATHS 
                ${Vulkan_LIBRARY_DIR}
                $ENV{VULKAN_SDK}/Lib
                $ENV{VULKAN_SDK}/lib
        )
        
        if(SHADERC_LIB)
            message(STATUS "Using Vulkan SDK shaderc: ${SHADERC_LIB}")
            # Add include directory
            if(EXISTS "$ENV{VULKAN_SDK}/include")
                include_directories($ENV{VULKAN_SDK}/include)
            endif()
        else()
            message(FATAL_ERROR 
                "shaderc library not found!\n"
                "Please either:\n"
                "1. Build shaderc from source (recommended for macOS):\n"
                "   Run: ./build_shaderc.sh\n"
                "   Then set: -DSHADERC_ROOT=~/shaderc_build/install\n"
                "2. Install via Homebrew: brew install shaderc\n"
                "3. Ensure Vulkan SDK is properly installed")
        endif()
    endif()
endif()

add_executable(testing main.cpp)

target_include_directories(testing PRIVATE ${Vulkan_INCLUDE_DIRS} ${vmahpp_SOURCE_DIR})

target_compile_definitions(testing PRIVATE VMA_VULKAN_HPP)

target_link_libraries(testing PRIVATE 
Vulkan::Vulkan 
glfw 
VulkanMemoryAllocator 

${SHADERC_LIB}

)


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

# === SHADERC DETECTION ===
set(SHADERC_FOUND FALSE)

if(DEFINED SHADERC_ROOT)
    # Use custom shaderc build
    find_library(SHADERC_LIB
        NAMES shaderc_combined
        PATHS ${SHADERC_ROOT}/lib
        NO_DEFAULT_PATH
        REQUIRED
    )
    
    include_directories(${SHADERC_ROOT}/include)
    set(SHADERC_FOUND TRUE)
    message(STATUS "Using custom shaderc: ${SHADERC_LIB}")
else()
    # === OPTION 2: Try system shaderc ===
    find_package(unofficial-shaderc CONFIG QUIET)
    
    if(unofficial-shaderc_FOUND)
        set(SHADERC_LIB unofficial::shaderc::shaderc)
        set(SHADERC_FOUND TRUE)
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
            set(SHADERC_FOUND TRUE)
            message(STATUS "Using Vulkan SDK shaderc: ${SHADERC_LIB}")
            # Add include directory
            if(EXISTS "$ENV{VULKAN_SDK}/include")
                include_directories($ENV{VULKAN_SDK}/include)
            endif()
        endif()
    endif()
endif()


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
    set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(shaderc)

    # Get the source directory for shaderc includes
    FetchContent_GetProperties(shaderc SOURCE_DIR SHADERC_SOURCE_DIR)
    set(SHADERC_INCLUDE_DIRS "${SHADERC_SOURCE_DIR}/libshaderc/include")
    
    set(SHADERC_LIB shaderc_combined)
    message(STATUS "Using fetched shaderc with dependencies")
     message(STATUS "  Include dir: ${SHADERC_INCLUDE_DIRS}")
endif()

add_executable(testing main.cpp)

target_include_directories(testing PRIVATE ${Vulkan_INCLUDE_DIRS} ${vmahpp_SOURCE_DIR} ${SHADERC_INCLUDE_DIRS})

target_compile_definitions(testing PRIVATE VMA_VULKAN_HPP)

target_link_libraries(testing PRIVATE 
Vulkan::Vulkan 
glfw 
VulkanMemoryAllocator 

${SHADERC_LIB}

)


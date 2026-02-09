cmake_minimum_required(VERSION 3.13)

# Find required dependencies
find_package(Vulkan REQUIRED)

if(WIN32)
    include(FetchContent)

    # Fetch GLFW from GitHub
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)

    # Fetch GLM from GitHub
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
    )
    FetchContent_MakeAvailable(glm)
else()
    find_package(glfw3 REQUIRED) 
    find_package(glm REQUIRED)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/Library)
include_directories(${Vulkan_INCLUDE_DIRS})


add_compile_definitions(PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/")

# Helper function to create examples
function(add_lavacake_example TARGET_NAME SOURCE_FILE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE LavaCake::LavaCake glfw glm::glm)
endfunction()

# Add example subdirectories
add_subdirectory(basic)
add_subdirectory(compute)
add_subdirectory(advanced)
add_subdirectory(imgui)
add_subdirectory(raytracing)

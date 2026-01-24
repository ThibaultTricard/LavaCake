cmake_minimum_required(VERSION 3.10)

# Find required dependencies
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/Library)
include_directories(${Vulkan_INCLUDE_DIRS})


add_compile_definitions(PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/")

# Helper function to create examples
function(add_lavacake_example TARGET_NAME SOURCE_FILE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE LavaCake::LavaCake glfw glm::glm VulkanMemoryAllocator ${SHADERC_LIB})
    target_include_directories(${TARGET_NAME} PRIVATE ${SHADERC_INCLUDE_DIRS})
endfunction()

# Add example subdirectories
add_subdirectory(basic)
add_subdirectory(compute)

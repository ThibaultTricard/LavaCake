cmake_minimum_required(VERSION 3.10)
project(VulkanHPP_FullExample)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)


include(FetchContent)

FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG master
)

FetchContent_MakeAvailable(VulkanMemoryAllocator)

find_package(glslang  CONFIG REQUIRED)
find_package(SPIRV-Tools CONFIG REQUIRED)

add_executable(testing main.cpp)

target_include_directories(testing PRIVATE ${Vulkan_INCLUDE_DIRS} ${vmahpp_SOURCE_DIR})

target_compile_definitions(testing PRIVATE VMA_VULKAN_HPP)

target_link_libraries(testing PRIVATE Vulkan::Vulkan glfw VulkanMemoryAllocator glslang SPIRV-Tools SPIRV-Tools-opt glslang::OSDependent glslang::SPIRV glslang::glslang)

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

add_executable(VulkanHPP_FullExample main.cpp)

target_include_directories(VulkanHPP_FullExample PRIVATE ${Vulkan_INCLUDE_DIRS})

target_compile_definitions(VulkanHPP_FullExample PRIVATE VMA_VULKAN_HPP)

target_link_libraries(VulkanHPP_FullExample PRIVATE Vulkan::Vulkan glfw VulkanMemoryAllocator)

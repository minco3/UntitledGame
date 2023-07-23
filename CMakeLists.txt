cmake_minimum_required(VERSION 3.7)

project(SDL2Test)

set(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 20)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(fmt REQUIRED)

include_directories(/opt/homebrew/include)
include_directories(/usr/local/include)
include_directories(~VulkanSDK)

add_executable(SDL2Test main.cpp)
target_link_libraries(SDL2Test SDL2::SDL2)
target_link_libraries(SDL2Test SDL2_image::SDL2_image)
target_link_libraries(SDL2Test Vulkan::Vulkan)
target_link_libraries(SDL2Test glm::glm)
target_link_libraries(SDL2Test fmt::fmt)

file(GLOB shader_files "${PROJECT_SOURCE_DIR}/shader/*")

foreach(shader_file ${shader_files})
    execute_process(COMMAND bash -c "${Vulkan_GLSLC_EXECUTABLE} -c ${shader_file}")
endforeach()
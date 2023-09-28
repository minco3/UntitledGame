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

file(GLOB source_files CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/*.cpp" )

add_executable(SDL2Test main.cpp ${source_files})
target_link_libraries(SDL2Test SDL2::SDL2main)
target_link_libraries(SDL2Test SDL2::SDL2)
target_link_libraries(SDL2Test SDL2_image::SDL2_image)
target_link_libraries(SDL2Test Vulkan::Vulkan)
if (WIN32)
target_link_libraries(SDL2Test glm)
else()
target_link_libraries(SDL2Test glm::glm)
endif (WIN32)
target_link_libraries(SDL2Test fmt::fmt)

# maybe switch this to a batch file so its one command to compile all shaders rather than one per shader
file(GLOB shader_files CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/shader/*")
foreach(shader_file ${shader_files})
    add_custom_command(TARGET SDL2Test
        POST_BUILD
        COMMAND bash -c "${Vulkan_GLSLC_EXECUTABLE} -c -g ${shader_file}")
endforeach()
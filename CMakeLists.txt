cmake_minimum_required(VERSION 3.7)

project(UntitledGame)

add_definitions(-DWORKING_DIRECTORY=\"${PROJECT_SOURCE_DIR}\")

set(CMAKE_CXX_STANDARD 20)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(fmt REQUIRED)
# find_package(shaderc REQUIRED)

if(APPLE)
include_directories(/opt/homebrew/include)
include_directories(/usr/local/include)
endif()

# add_subdirectory(fmt)

file(GLOB source_files CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/*.cpp" )

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shader)

# maybe switch this to a batch file so its one command to compile all shaders rather than one per shader
file(GLOB shader_files CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/shader/*")
foreach(shader_file ${shader_files})
    get_filename_component(FILENAME ${shader_file} NAME)
    add_custom_command(
        TARGET shaders
        COMMAND bash -c "${Vulkan_GLSLC_EXECUTABLE} -c -g ${shader_file} -o ${PROJECT_BINARY_DIR}/shader/${FILENAME}.spv"
        DEPENDS ${shader_file}
        COMMENT "Compiling ${FILENAME}")
    list(APPEND SPV_SHADERS ${SHADER_DIR}/shaders/${FILENAME}.spv)
endforeach()

add_executable(UntitledGame main.cpp ${source_files} ${SHADERS})
add_dependencies(UntitledGame shaders)
target_link_libraries(UntitledGame SDL2::SDL2main)
target_link_libraries(UntitledGame SDL2::SDL2)
target_link_libraries(UntitledGame SDL2_image::SDL2_image)
target_link_libraries(UntitledGame Vulkan::Vulkan)
if (WIN32)
target_link_libraries(UntitledGame glm)
else()
target_link_libraries(UntitledGame glm::glm)
endif (WIN32)
target_link_libraries(UntitledGame fmt::fmt)
# target_link_libraries(UntitledGame libshaderc_combined)
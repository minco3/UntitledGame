find_package(Vulkan)
find_package(SPIRV-Headers)

set(CMAKE_CXX_STANDARD 14)

add_library(SPIRV-Reflect spirv_reflect.cpp)

target_compile_definitions(SPIRV-Reflect PUBLIC SPIRV_REFLECT_USE_SYSTEM_SPIRV_H) # is public what I want here

target_link_libraries(SPIRV-Reflect Vulkan::Vulkan)
target_link_libraries(SPIRV-Reflect SPIRV-Headers::SPIRV-Headers)

target_include_directories(SPIRV-Reflect PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
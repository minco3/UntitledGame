file(GLOB imgui_source CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" )
find_package(SDL2)
add_library(imgui ${imgui_source})
target_link_libraries(imgui SDL2::SDL2)
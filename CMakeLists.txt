cmake_minimum_required (VERSION 3.22)

project (untitled LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BASE_SRCS
    "src/rendering/renderer.cpp"
    "src/rendering/renderer.h"
    "src/rendering/shader.cpp"
    "src/rendering/shader.h"
    "src/util/geometry.h"
    "src/main.cpp"
)

add_executable (untitled ${BASE_SRCS})

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/copy-assets.cmake
)

add_dependencies(untitled copy_assets)

find_package(Vulkan REQUIRED)
target_include_directories(untitled PRIVATE C:/VulkanSDK/1.3.296.0/Include)
add_subdirectory(libs/glfw-3.4)

target_link_libraries(untitled ${Vulkan_LIBRARY} glfw)
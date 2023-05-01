cmake_minimum_required(VERSION 3.20)
project(etna-spring-physics)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

add_subdirectory(lib)

find_package(GLFW REQUIRED)
find_package(Vulkan REQUIRED)

set(CMAKE_CXX_FLAGS "-O0 -fsanitize=address")
set(CMAKE_CXX_STANDARD 20)

message(STATUS "GLFW_INCLUDE_DIR: ${GLFW_INCLUDE_DIR}")
message(STATUS "GLFW_LIBRARY: ${GLFW_LIBRARY}")
message(STATUS "Vulkan_INCLUDE_DIR: ${Vulkan_INCLUDE_DIR}")
message(STATUS "Vulkan_LIBRARY: ${Vulkan_LIBRARY}")

file(GLOB_RECURSE files "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.hpp")
add_executable(etna-spring-physics ${files})
include_directories(etna-spring-physics lib/etna/include ${GLFW_INCLUDE_DIR} ${Vulkan_INCLUDE_DIR})
target_link_libraries(etna-spring-physics Etna ${GLFW_LIBRARY} Vulkan::Vulkan)
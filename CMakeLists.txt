cmake_minimum_required(VERSION 3.21)
project(opengl_flying_sim)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SFML 2.5 COMPONENTS graphics REQUIRED)
find_package(FreeImage REQUIRED)

# Find the hidapi package
find_package(PkgConfig REQUIRED)
pkg_search_module(HIDAPI REQUIRED hidapi)

# Include the hidapi directories and link the library
include_directories(${HIDAPI_INCLUDE_DIRS})
link_directories(${HIDAPI_LIBRARY_DIRS})

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-O3")
#set(CMAKE_CXX_FLAGS "-O3 -fsanitize=address")
#set(CMAKE_CXX_FLAGS "-O2 -fsanitize=address")
#set(CMAKE_CXX_FLAGS "-O1 -fsanitize=address")
#set(CMAKE_CXX_FLAGS "-O0 -fsanitize=address")
file(GLOB_RECURSE files "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.hpp")
add_executable(opengl_flying_sim ${files})
target_link_libraries(opengl_flying_sim ${HIDAPI_LIBRARIES} sfml-graphics sfml-audio GLEW::GLEW OpenGL::GL ${FREEIMAGE_LIBRARIES})
include_directories(opengl_flying_sim ${GLEW_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${SFML_INCLUDE_DIRS} ${FREEIMAGE_INCLUDE_DIRS})

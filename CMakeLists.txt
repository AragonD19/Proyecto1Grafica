cmake_minimum_required(VERSION 3.23)
project(Proyecto1Grafica)

set(CMAKE_CXX_STANDARD 20)

add_executable(Proyecto1Grafica main.cpp color.h fragment.h framebuffer.h line.h point.h triangle.h uniform.h FastNoiseLite.h)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(SDL2_PATH "C:/SDL2-2.28.5/x86_64-w64-mingw32")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})


target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
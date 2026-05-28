cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 23)
project(glad)

add_library(glad)
target_sources(glad PRIVATE src/glad.c)
target_include_directories(glad PUBLIC include)
cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 23)
set(project_name "korall_main")
project(${project_name} LANGUAGES C)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/korall/")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/examples/example1/")
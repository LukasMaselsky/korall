cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 23)
set(project_name "korall_tests")

project(${project_name} LANGUAGES C)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_executable(${project_name})
target_sources(${project_name} PRIVATE ${PROJECT_SOURCES})

# IMPORTANT, GIVES ACCESS TO INTERNALS OF PROJECT
target_include_directories(${project_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src/")

# Link library
target_link_libraries(${project_name} PRIVATE korall)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_compile_definitions.cmake")
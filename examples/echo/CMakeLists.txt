cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 23)
set(project_name "korall_example_echo")

project(${project_name} LANGUAGES C)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

file(GLOB MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_executable(${project_name})
target_sources(${project_name} PRIVATE ${MY_SOURCES})

# This is useful to get an ASSETS_PATH in your IDE during development
target_compile_definitions(${project_name} PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")

# Include directories
target_include_directories(${project_name} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

# Link libraries that use CMake
target_link_libraries(${project_name} PRIVATE korall)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_compile_definitions.cmake")
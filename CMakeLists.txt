cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 23)
set(project_name "korall")
project(${project_name} LANGUAGES C)

# 3rd party libraries
add_subdirectory(deps/glfw-3.4)
add_subdirectory(deps/glad)

# 3rd party (vcpkg)
find_package(OpenSSL CONFIG REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_library(${project_name} STATIC ${PROJECT_SOURCES})

# PUBLIC to allow examples access to it
target_include_directories(${project_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_include_directories(${project_name} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_link_libraries(${project_name} PUBLIC glfw glad OpenSSL::SSL)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests/")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/examples/http/")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/examples/echo/")
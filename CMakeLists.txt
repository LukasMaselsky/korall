cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 23)
set(project_name "korall")
project(${project_name} LANGUAGES C)

# 3rd party
find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)
if (NOT OpenSSL_FOUND)
	message(FATAL_ERROR "OpenSSL installation not found")
endif()
add_subdirectory(deps/cjson)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_library(${project_name} STATIC ${PROJECT_SOURCES})
target_compile_definitions(${project_name} PUBLIC KORALL_RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")

# Build type flag
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(${project_name} PRIVATE KORALL_BUILD_TYPE_DEBUG=1)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	target_compile_definitions(${project_name} PRIVATE KORALL_BUILD_TYPE_RELEASE=1)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
	target_compile_definitions(${project_name} PRIVATE KORALL_BUILD_TYPE_DISTRIBUTION=1)
endif()

# PUBLIC to allow examples access to it
target_include_directories(${project_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_include_directories(${project_name} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_link_libraries(${project_name} PUBLIC cjson OpenSSL::SSL)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests/")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/examples/http/")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/examples/echo/")
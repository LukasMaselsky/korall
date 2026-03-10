cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 23)
set(project_name "korall_example1")

if(MSVC) 
	add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
endif()

project(${project_name} LANGUAGES C)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

file(GLOB MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_executable(${project_name})
target_sources(${project_name} PRIVATE ${MY_SOURCES})

# This is useful to get an ASSETS_PATH in your IDE during development
target_compile_definitions(${project_name} PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
   
# If using the VS compiler...
if(MSVC)
    target_compile_definitions(${project_name} PUBLIC _CRT_SECURE_NO_WARNINGS)
	# set_target_properties(example1 PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") # no console	
endif()

# Force remove unicode
if (WIN32)
	target_compile_options(${project_name} PRIVATE /UUNICODE /U_UNICODE)
endif()


# Include directories
target_include_directories(${project_name} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

# Link libraries that use CMake
target_link_libraries(${project_name} PRIVATE korall)

# Compiler flags

if(MSVC)
    add_compile_options("/Wall" "$<$<CONFIG:RELEASE>:/O2>" "/WX" "/fsanitize=address")
else()
    add_compile_options("-Wall" "-Wextra" "-Werror" "$<$<CONFIG:RELEASE>:-O3>" "-fsanitize=address")
    if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options("-stdlib=libc++")
    else()
        # nothing special for gcc at the moment
    endif()
endif()
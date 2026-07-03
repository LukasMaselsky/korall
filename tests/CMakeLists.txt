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




# If using the VS compiler...
if(MSVC)
	target_compile_definitions(${project_name} PUBLIC _CRT_SECURE_NO_WARNINGS)
	# set_target_properties(client PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") # no console
endif()

# Force remove unicode
if (WIN32)
	target_compile_options(${project_name} PRIVATE /UUNICODE /U_UNICODE)
endif()

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
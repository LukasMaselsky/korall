cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 23)
set(project_name "korall")

###############################################################################
# PROJECT SETUP
###############################################################################

project(${project_name} LANGUAGES C)
file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_library(${project_name} STATIC ${PROJECT_SOURCES})
# PUBLIC to allow examples access to it
target_include_directories(${project_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_include_directories(${project_name} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

###############################################################################
# EXECUTABLE TARGETS
###############################################################################

file(GLOB PROJECT_SOURCES_TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.c")

add_executable(tests)
target_sources(tests PRIVATE ${PROJECT_SOURCES_TESTS})

# Include directories
target_include_directories(tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_link_libraries(tests PRIVATE ${project_name})

# If using the VS compiler...
if(MSVC)
		target_compile_definitions(tests PUBLIC _CRT_SECURE_NO_WARNINGS)
		# set_target_properties(client PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") # no console
endif()

# Force remove unicode
if (WIN32)
	target_compile_options(tests PRIVATE /UUNICODE /U_UNICODE)
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
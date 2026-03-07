cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 23)

# ! ! ! ! ! ! !
# set this to ON instead of OFF to ship the game!
# basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !
set(PRODUCTION_BUILD OFF CACHE BOOL "Make this a production build" FORCE)
# delete the out folder after changing if visual studio doesn recognize the change!

#may give problems on linux
if(PRODUCTION_BUILD)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization
else()
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()

if(MSVC) 
	add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
endif()

###############################################################################
# PROJECT SETUP
###############################################################################

project(websocketexamples LANGUAGES C)
# include(cmake/configs.cmake)
# add_subdirectory(tests)

###############################################################################
# ADD SUBDIRECTORIES
###############################################################################

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory("http_server")

###############################################################################
# EXECUTABLE TARGETS
###############################################################################

# MY_SOURCES is defined to be a list of all the source files for my game 
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro

file(GLOB MY_SOURCES_EXAMPLE1 CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/examples/src/example1/*.c")

add_executable(example1)
target_sources(example1 PRIVATE ${MY_SOURCES_EXAMPLE1})

if(PRODUCTION_BUILD)
	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions(example1 PUBLIC RESOURCES_PATH="./resources/") 
	# remove the option to debug asserts.
	target_compile_definitions(example1 PUBLIC PRODUCTION_BUILD=1) 
	target_compile_definitions(example1 PUBLIC DEVELOPLEMT_BUILD=0) 
else()
	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions(example1 PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/examples/resources/example1/")
    #
	target_compile_definitions(example1 PUBLIC PRODUCTION_BUILD=0) 
	target_compile_definitions(example1 PUBLIC DEVELOPLEMT_BUILD=1) 
endif()

# If using the VS compiler...
if(MSVC)
    	target_compile_definitions(example1 PUBLIC _CRT_SECURE_NO_WARNINGS)
		# set_target_properties(example1 PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") # no console	
endif()

# Force remove unicode
if (WIN32)
	target_compile_options(example1 PRIVATE /UUNICODE /U_UNICODE)
endif()


# Include directories
target_include_directories(example1 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/examples/include/example1/")

# Link libraries that use CMake
target_link_libraries(example1 PRIVATE http_server)

# Compiler flags

if(MSVC)
    add_compile_options("/Wall" "$<$<CONFIG:RELEASE>:/O2>" "/WX")
else()
    add_compile_options("-Wall" "-Wextra" "-Werror" "$<$<CONFIG:RELEASE>:-O3>")
    if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options("-stdlib=libc++")
    else()
        # nothing special for gcc at the moment
    endif()
endif()
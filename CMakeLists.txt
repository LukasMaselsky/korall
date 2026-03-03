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

project(websocket LANGUAGES C)
# include(cmake/configs.cmake)
# add_subdirectory(tests)

###############################################################################
# ADD SUBDIRECTORIES
###############################################################################

# add_subdirectory(extern/tiny_regex_c)
# set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

###############################################################################
# EXECUTABLE TARGETS
###############################################################################

# MY_SOURCES is defined to be a list of all the source files for my game 
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro

file(GLOB_RECURSE MY_SOURCES_SERVER CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.c")
file(GLOB_RECURSE MY_SOURCES_CLIENT CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/client/*.c")
file(GLOB MY_SOURCES_GLOBAL CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
file(GLOB MY_SOURCES_TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.c")

add_executable(server)
target_sources(server PRIVATE ${MY_SOURCES_SERVER})
target_sources(server PRIVATE ${MY_SOURCES_GLOBAL})

add_executable(client)
target_sources(client PRIVATE ${MY_SOURCES_CLIENT})
target_sources(client PRIVATE ${MY_SOURCES_GLOBAL})

# filter out main files for test so no main() conflicts in test binary
list(REMOVE_ITEM MY_SOURCES_CLIENT "${CMAKE_CURRENT_SOURCE_DIR}/src/client/main.c") 
list(REMOVE_ITEM MY_SOURCES_SERVER "${CMAKE_CURRENT_SOURCE_DIR}/src/server/main.c") 
add_executable(tests)
target_sources(tests PRIVATE ${MY_SOURCES_TESTS})
target_sources(tests PRIVATE ${MY_SOURCES_GLOBAL})
target_sources(tests PRIVATE ${MY_SOURCES_CLIENT})
target_sources(tests PRIVATE ${MY_SOURCES_SERVER})

if(PRODUCTION_BUILD)
	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions(server PUBLIC RESOURCES_PATH="./resources/") 
	# remove the option to debug asserts.
	target_compile_definitions(server PUBLIC PRODUCTION_BUILD=1) 
	target_compile_definitions(server PUBLIC DEVELOPLEMT_BUILD=0) 

	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions(client PUBLIC RESOURCES_PATH="./resources/") 
	# remove the option to debug asserts.
	target_compile_definitions(client PUBLIC PRODUCTION_BUILD=1) 
	target_compile_definitions(client PUBLIC DEVELOPLEMT_BUILD=0) 
else()
	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions(server PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
    #
	target_compile_definitions(server PUBLIC PRODUCTION_BUILD=0) 
	target_compile_definitions(server PUBLIC DEVELOPLEMT_BUILD=1) 


	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions(client PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
    #
	target_compile_definitions(client PUBLIC PRODUCTION_BUILD=0) 
	target_compile_definitions(client PUBLIC DEVELOPLEMT_BUILD=1) 
endif()

# If using the VS compiler...
if(MSVC)
    	target_compile_definitions(server PUBLIC _CRT_SECURE_NO_WARNINGS)
		# set_target_properties(server PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") # no console

    	target_compile_definitions(client PUBLIC _CRT_SECURE_NO_WARNINGS)
		# set_target_properties(client PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") # no console
    	
		target_compile_definitions(tests PUBLIC _CRT_SECURE_NO_WARNINGS)
		# set_target_properties(client PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") # no console
endif()

# Force remove unicode
if (WIN32)
	target_compile_options(server PRIVATE /UUNICODE /U_UNICODE)
	target_compile_options(client PRIVATE /UUNICODE /U_UNICODE)
	target_compile_options(tests PRIVATE /UUNICODE /U_UNICODE)
endif()


# Include directories
target_include_directories(server PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(client PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(server PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/server/")
target_include_directories(client PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/client/")
target_include_directories(tests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(tests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/server/")
target_include_directories(tests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/client/")

# Link libraries that use CMake
# target_link_libraries(server PRIVATE tiny_regex_c)
# target_link_libraries(client PRIVATE tiny_regex_c)

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
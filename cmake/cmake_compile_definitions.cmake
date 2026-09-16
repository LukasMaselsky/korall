
# If using the VS compiler..if(MSVC) 
if(MSVC)
	add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
    target_compile_definitions(${project_name} PUBLIC _CRT_SECURE_NO_WARNINGS)
	# set_target_properties(example1 PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") # no console	
endif()

# Force remove unicode
if (WIN32)
	target_compile_options(${project_name} PRIVATE /UUNICODE /U_UNICODE)
endif()


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
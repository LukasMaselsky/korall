
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(${project_name} PRIVATE BUILD_TYPE_DEBUG=1)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	target_compile_definitions(${project_name} PRIVATE BUILD_TYPE_RELEASE=1)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
	target_compile_definitions(${project_name} PRIVATE BUILD_TYPE_DISTRIBUTION=1)
endif()
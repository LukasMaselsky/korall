cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 23)
set(project_name "cjson")
project(${project_name})

add_library(${project_name})
target_sources(${project_name} PRIVATE cJSON.c)
target_include_directories(${project_name} PUBLIC ..)
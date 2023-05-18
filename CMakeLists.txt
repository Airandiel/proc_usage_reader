cmake_minimum_required(VERSION 3.10)
project(ProcUsage LANGUAGES C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
# Find the pthread library
find_package(Threads REQUIRED)
add_subdirectory(libraries)
add_executable(ProcUsage main.c)
# target_include_directories(ProcUsage PUBLIC
#                            "${PROJECT_BINARY_DIR}"
#                            )
# target_include_directories (ProcUsage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ProcUsage util_lib Threads::Threads)

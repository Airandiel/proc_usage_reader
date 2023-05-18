add_library(util_lib logger.c logger.h watchdog.c watchdog.h)

# Specify any include directories needed by the library
target_include_directories(util_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set(LIB_FILES
    logger.c 
    logger.h 
    watchdog.c 
    watchdog.h 
    queue.c 
    queue.h 
    reader.c 
    reader.h 
    analyser.c 
    analyser.h
    printer.c
    printer.h
)

add_library(util_lib STATIC ${LIB_FILES})

# Specify any include directories needed by the library
target_include_directories(util_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
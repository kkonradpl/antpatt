cmake_minimum_required(VERSION 3.6)
project(antpatt)

find_package(PkgConfig REQUIRED)

#pkg_check_modules(GTK REQUIRED gtk+-3.0)
pkg_check_modules(GTK REQUIRED gtk+-2.0)
include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})
add_definitions(${GTK_CFLAGS_OTHER})

pkg_check_modules(GSL REQUIRED gsl)
include_directories(${GSL_INCLUDE_DIRS})
link_directories(${GSL_LIBRARY_DIRS})
add_definitions(${GSL_CFLAGS_OTHER})

pkg_check_modules(JSON-C REQUIRED json-c)
include_directories(${JSON-C_INCLUDE_DIRS})
link_directories(${JSON-C_LIBRARY_DIRS})
add_definitions(${JSON-C_CFLAGS_OTHER})

pkg_check_modules(ZLIB REQUIRED zlib)
include_directories(${ZLIB_INCLUDE_DIRS})
link_directories(${ZLIB_LIBRARY_DIRS})
add_definitions(${ZLIB_CFLAGS_OTHER})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

set(SOURCE_FILES
        main.c
        pattern.c
        pattern.h
        pattern-color.c
        pattern-color.h
        pattern-data.c
        pattern-data.h
        pattern-import.c
        pattern-import.h
        pattern-json.c
        pattern-json.h
        pattern-misc.c
        pattern-misc.h
        pattern-plot.c
        pattern-plot.h
        pattern-signal.c
        pattern-signal.h
        pattern-ui-dialogs.c
        pattern-ui-dialogs.h
        pattern-ui-window.c
        pattern-ui-window.h
        pattern-ui.c
        pattern-ui.h
        version.h)

set(SOURCE_FILES_MINGW
        icon.rc
        win32.c
        win32.h)

set(LIBRARIES
        ${GTK_LIBRARIES}
        ${GSL_LIBRARIES}
        ${JSON-C_LIBRARIES}
        ${ZLIB_LIBRARIES})

if(MINGW)
    IF(NOT (CMAKE_BUILD_TYPE MATCHES Debug))
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows")
    ENDIF()
    set(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -i <SOURCE> -o <OBJECT>")
    add_executable(antpatt ${SOURCE_FILES} ${SOURCE_FILES_MINGW})
ELSE()
    add_executable(antpatt ${SOURCE_FILES})
ENDIF()

target_link_libraries(antpatt ${LIBRARIES})

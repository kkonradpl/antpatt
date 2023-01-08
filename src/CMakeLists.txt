cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        main.c
        pattern.c
        pattern.h
        pattern-color.c
        pattern-color.h
        pattern-data.c
        pattern-data.h
        pattern-export.c
        pattern-export.h
        pattern-import.c
        pattern-import.h
        pattern-ipc.c
        pattern-ipc.h
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
        pattern-ui-plot.c
        pattern-ui-plot.h
        pattern-ui-window.c
        pattern-ui-window.h
        pattern-ui.c
        pattern-ui.h
        version.h)

set(SOURCE_FILES_MINGW
        icon.rc
        mingw.c
        mingw.h)

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


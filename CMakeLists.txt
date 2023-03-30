cmake_minimum_required(VERSION 3.6)
project(antpatt)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GTK REQUIRED gtk+-3.0)
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

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)
execute_process(COMMAND ${GLIB_COMPILE_RESOURCES} --generate-source --sourcedir=${CMAKE_SOURCE_DIR} --target=${CMAKE_BINARY_DIR}/resources.c ${CMAKE_SOURCE_DIR}/icons/icons.xml)
execute_process(COMMAND ${GLIB_COMPILE_RESOURCES} --generate-header --sourcedir=${CMAKE_SOURCE_DIR} --target=${CMAKE_BINARY_DIR}/resources.h ${CMAKE_SOURCE_DIR}/icons/icons.xml)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-overlength-strings")

add_subdirectory(src)

if(NOT MINGW)
    install(TARGETS antpatt DESTINATION bin)
    install(FILES antpatt.desktop DESTINATION share/applications)
    install(DIRECTORY icons/ DESTINATION share/icons/hicolor
            FILES_MATCHING
            PATTERN "antpatt\.png"
            PATTERN "antpatt\.svg")
    install(CODE "execute_process(COMMAND gtk-update-icon-cache /usr/share/icons/hicolor)")
endif()

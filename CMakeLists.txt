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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-deprecated-declarations")

add_subdirectory(src)

if(NOT MINGW)
    install(TARGETS antpatt DESTINATION bin)
    install(FILES antpatt.desktop DESTINATION share/applications)
    install(DIRECTORY icons/ DESTINATION share/icons/hicolor
            PATTERN "ico" EXCLUDE)
    install(CODE "execute_process(COMMAND gtk-update-icon-cache /usr/share/icons/hicolor)")
endif()

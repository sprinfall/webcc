# webcc

# Don't use any deprecated definitions (e.g., io_service).
add_definitions(-DBOOST_ASIO_NO_DEPRECATED)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

# Adhere to GNU filesystem layout conventions.
include(GNUInstallDirs)

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

file(GLOB HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

if(NOT WEBCC_ENABLE_GZIP)
    list(REMOVE_ITEM SOURCES "gzip.cc")
    list(REMOVE_ITEM HEADERS "gzip.h")
endif()

set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Add a postfix to the debug library")
mark_as_advanced(CMAKE_DEBUG_POSTFIX)

set(TARGET webcc)

add_library(${TARGET} STATIC ${SOURCES} ${HEADERS})

# Install lib and header files.
# On Linux, if CMAKE_INSTALL_PREFIX is ~, the lib (libwebcc.a) will be installed
# to ~/lib and header files will be installed to ~/include.
install(TARGETS ${TARGET} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/webcc)
install(FILES ${PROJECT_BINARY_DIR}/webcc/config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/webcc)

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

# Link to pthread for Linux. 
# See: https://stackoverflow.com/a/29871891
if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET} Threads::Threads)
endif()

# Boost
target_link_libraries(${TARGET} ${Boost_LIBRARIES})

# ZLIB
if(WEBCC_ENABLE_GZIP)
	# The imported target ZLIB::ZLIB could be used instead.
    target_link_libraries(${TARGET} ${ZLIB_LIBRARIES})
endif()

# OpenSSL
if(WEBCC_ENABLE_SSL)
    target_link_libraries(${TARGET} ${OPENSSL_LIBRARIES})
    if(WIN32)
        target_link_libraries(${TARGET} crypt32)
    endif()
endif()

# Install lib and header files.
# On Linux, if CMAKE_INSTALL_PREFIX is ~, the lib (libwebcc.a) will be installed
# to ~/lib and header files will be installed to ~/include.
install(TARGETS ${TARGET} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/webcc)
install(FILES ${PROJECT_BINARY_DIR}/webcc/config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/webcc)

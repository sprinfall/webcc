option(WEBCC_DEBUG_OUTPUT "Enable debug output?" OFF)

if(WEBCC_DEBUG_OUTPUT)
    add_definitions(-DWEBCC_DEBUG_OUTPUT)
endif()

# Don't use any deprecated definitions (e.g., io_service).
add_definitions(-DBOOST_ASIO_NO_DEPRECATED)

file(GLOB SRCS *.cc *.h)

add_library(webcc ${SRCS})

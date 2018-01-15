if(UNIX)
    add_definitions(-D_GLIBCXX_USE_WCHAR_T -std=c++11)
endif()

# Don't use any deprecated definitions (e.g., io_service).
add_definitions(-DBOOST_ASIO_NO_DEPRECATED)

file(GLOB SRCS *.cc *.h)

add_library(csoap ${SRCS})


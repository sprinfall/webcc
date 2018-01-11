if(UNIX)
    add_definitions(-D_GLIBCXX_USE_WCHAR_T -std=c++11)
endif()

file(GLOB SRCS *.cc *.h)

add_executable(calculator_server ${SRCS})

target_link_libraries(calculator_server csoap pugixml)

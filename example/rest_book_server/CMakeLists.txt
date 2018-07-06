set(SRCS book_services.cc book_services.h main.cc)

add_executable(rest_book_server ${SRCS})

target_link_libraries(rest_book_server webcc jsoncpp ${Boost_LIBRARIES})
target_link_libraries(rest_book_server "${CMAKE_THREAD_LIBS_INIT}")

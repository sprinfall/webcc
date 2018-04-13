add_executable(rest_book_client main.cc)

target_link_libraries(rest_book_client webcc jsoncpp ${Boost_LIBRARIES})
target_link_libraries(rest_book_client "${CMAKE_THREAD_LIBS_INIT}")

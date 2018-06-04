add_executable(rest_book_async_client main.cc)

target_link_libraries(rest_book_async_client webcc jsoncpp ${Boost_LIBRARIES})
target_link_libraries(rest_book_async_client "${CMAKE_THREAD_LIBS_INIT}")

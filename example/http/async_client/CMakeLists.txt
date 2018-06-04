add_executable(http_async_client main.cc)

target_link_libraries(http_async_client webcc ${Boost_LIBRARIES})
target_link_libraries(http_async_client "${CMAKE_THREAD_LIBS_INIT}")

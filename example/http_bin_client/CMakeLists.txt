add_executable(http_bin_client main.cc)

target_link_libraries(http_bin_client webcc ${Boost_LIBRARIES})
target_link_libraries(http_bin_client "${CMAKE_THREAD_LIBS_INIT}")

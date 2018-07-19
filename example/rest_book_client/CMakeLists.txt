set(TARGET_NAME rest_book_client)

set(SRCS main.cc)

add_executable(${TARGET_NAME} ${SRCS})

target_link_libraries(${TARGET_NAME} webcc jsoncpp ${Boost_LIBRARIES})
target_link_libraries(${TARGET_NAME} "${CMAKE_THREAD_LIBS_INIT}")

file(GLOB SRCS *.cc *.h)

add_executable(rest_book_client ${SRCS})

target_link_libraries(rest_book_client webcc)

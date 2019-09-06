set(SRCS
	book.cc
	book.h
	book_json.cc
	book_json.h
    book_client.cc
    book_client.h
    main.cc
    )

add_executable(book_client ${SRCS})
target_link_libraries(book_client ${EXAMPLE_LIBS} jsoncpp)

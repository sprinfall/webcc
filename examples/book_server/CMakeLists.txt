set(SRCS
    book.cc
    book.h
    book_db.cc
    book_db.h
    book_json.cc
    book_json.h
    views.cc
    views.h
    main.cc
    )

add_executable(book_server ${SRCS})
target_link_libraries(book_server ${EXAMPLE_LIBS} jsoncpp)

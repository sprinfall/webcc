set(TARGET_NAME soap_book_client)

set(SRCS
	../common/book.cc
	../common/book.h
	../common/book_xml.cc
	../common/book_xml.h
	book_client.cc
	book_client.h
	main.cc)

add_executable(${TARGET_NAME} ${SRCS})

target_link_libraries(${TARGET_NAME} webcc pugixml ${Boost_LIBRARIES})
target_link_libraries(${TARGET_NAME} "${CMAKE_THREAD_LIBS_INIT}")

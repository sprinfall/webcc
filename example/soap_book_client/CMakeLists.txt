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

target_link_libraries(${TARGET_NAME} ${EXAMPLE_COMMON_LIBS} pugixml)

# Install VLD DLLs to build dir so that the example can be launched from
# inside VS.
if(WEBCC_ENABLE_VLD)
	install(DIRECTORY ${THIRD_PARTY_DIR}/win32/bin/debug/vld/
		CONFIGURATIONS Debug
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
endif()

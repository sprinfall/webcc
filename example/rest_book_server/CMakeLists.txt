set(TARGET_NAME rest_book_server)

set(SRCS
	../common/book.cc
	../common/book.h
	../common/book_json.cc
	../common/book_json.h
 	services.cc
 	services.h
 	main.cc)

add_executable(${TARGET_NAME} ${SRCS})

target_link_libraries(${TARGET_NAME} webcc jsoncpp ${Boost_LIBRARIES})
target_link_libraries(${TARGET_NAME} "${CMAKE_THREAD_LIBS_INIT}")

# Install VLD DLLs to build dir so that the example can be launched from
# inside VS.
if(WEBCC_ENABLE_VLD)
	install(DIRECTORY ${THIRD_PARTY_DIR}/win32/bin/debug/vld/
		CONFIGURATIONS Debug
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
endif()

# Tests

# Common libraries to link.
set(TEST_COMMON_LIBS webcc ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES}
	"${CMAKE_THREAD_LIBS_INIT}")
if(WIN32)
	set(TEST_COMMON_LIBS ${TEST_COMMON_LIBS} zlibstatic crypt32)
else()
	set(TEST_COMMON_LIBS ${TEST_COMMON_LIBS} ${ZLIB_LIBRARIES})
endif()

if(UNIX)
	# Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
	set(TEST_COMMON_LIBS ${TEST_COMMON_LIBS} ${CMAKE_DL_LIBS})
endif()

add_executable(test_logger test_logger.cc)
target_link_libraries(test_logger ${TEST_COMMON_LIBS})


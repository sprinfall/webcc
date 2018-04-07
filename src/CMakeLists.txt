if(UNIX)
    add_definitions(-std=c++11)
endif()

if(WEBCC_ENABLE_UT)
    add_subdirectory(gtest)
    add_subdirectory(webcc_unittest)
endif()

if(WEBCC_ENABLE_SOAP)
	add_subdirectory(pugixml)
endif()

add_subdirectory(webcc)

if(WEBCC_ENABLE_DEMO)
	if(WEBCC_ENABLE_SOAP)
		add_subdirectory(demo/soap/calc_client)
		add_subdirectory(demo/soap/calc_server)
	endif()
	add_subdirectory(demo/rest/book_client)
	add_subdirectory(demo/rest/book_server)
endif()

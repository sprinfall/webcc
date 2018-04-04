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
endif()
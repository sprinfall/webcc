if(WEBCC_ENABLE_UT)
    add_subdirectory(gtest)
    add_subdirectory(webcc_unittest)
endif()

add_subdirectory(pugixml)

add_subdirectory(webcc)

if(WEBCC_ENABLE_DEMO)
	add_subdirectory(demo/soap/calc_client)
	add_subdirectory(demo/soap/calc_server)
endif()
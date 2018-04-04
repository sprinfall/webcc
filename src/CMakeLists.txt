if(CSOAP_ENABLE_UT)
    add_subdirectory(gtest)
    add_subdirectory(csoap_unittest)
endif()

add_subdirectory(pugixml)

add_subdirectory(csoap)

add_subdirectory(demo/soap/calc_client)
add_subdirectory(demo/soap/calc_server)

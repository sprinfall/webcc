if(CSOAP_USE_TINYXML)
    add_subdirectory(tinyxml)
else()
    add_subdirectory(pugixml)
endif()

add_subdirectory(csoap)
add_subdirectory(demo)

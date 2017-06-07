if(UNIX)
    add_definitions(-D_GLIBCXX_USE_WCHAR_T -std=c++11)
endif()

file(GLOB SRCS *.cc *.h)

add_executable(demo ${SRCS})

if(CSOAP_USE_TINYXML)
    set(XML_LIB tinyxml)
else()
    set(XML_LIB pugixml)
endif()

target_link_libraries(demo csoap ${XML_LIB})


project(pugixml)

set(HEADERS pugixml.hpp pugiconfig.hpp)
set(SOURCES ${HEADERS} pugixml.cpp)

add_library(pugixml STATIC ${SOURCES})

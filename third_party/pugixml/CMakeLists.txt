project(pugixml)

cmake_minimum_required(VERSION 2.8)

set(HEADERS pugixml.hpp pugiconfig.hpp)
set(SOURCES ${HEADERS} pugixml.cpp)

add_library(pugixml STATIC ${SOURCES})

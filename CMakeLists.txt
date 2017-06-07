cmake_minimum_required(VERSION 2.8)
project(csoap)

option(CSOAP_USE_TINYXML "Use TinyXml instead of PugiXml?" ON)

if(CSOAP_USE_TINYXML)
    add_definitions(-DCSOAP_USE_TINYXML)

    # TinyXml uses STL.
    add_definitions(-DTIXML_USE_STL)
endif()

add_definitions(-DUNICODE -D_UNICODE)

if(MSVC)
    # Win7 (Boost.Asio needs this)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

include_directories(${PROJECT_SOURCE_DIR}/src)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(src)


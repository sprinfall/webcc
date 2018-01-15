cmake_minimum_required(VERSION 3.0)
project(csoap)

# TODO: Replace with log level.
option(CSOAP_ENABLE_OUTPUT "Enable output for request & response?" OFF)

if(CSOAP_ENABLE_OUTPUT)
    add_definitions(-DCSOAP_ENABLE_OUTPUT)
endif()

add_definitions(-DUNICODE -D_UNICODE)

if(MSVC)
    # Win7 (Boost.Asio needs this)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

# Boost version: 1.66+
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


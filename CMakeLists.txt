cmake_minimum_required(VERSION 3.1.0)
project(webcc)

option(WEBCC_ENABLE_SOAP "Enable SOAP support (need pugixml)?" ON)
option(WEBCC_BUILD_UNITTEST "Build unit test?" ON)
option(WEBCC_BUILD_REST_EXAMPLE "Build REST example?" ON)
option(WEBCC_BUILD_SOAP_EXAMPLE "Build SOAP example?" ON)

if(WEBCC_ENABLE_SOAP)
    add_definitions(-DWEBCC_ENABLE_SOAP)
endif()

if(WEBCC_BUILD_UNITTEST)
    enable_testing()
endif()

# See: https://stackoverflow.com/a/40217291
if(WIN32)
	macro(get_WIN32_WINNT version)
		if(CMAKE_SYSTEM_VERSION)
			set(ver ${CMAKE_SYSTEM_VERSION})
			string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
			string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
			# Check for Windows 10, b/c we'll need to convert to hex 'A'.
			if("${verMajor}" MATCHES "10")
				set(verMajor "A")
				string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
			endif("${verMajor}" MATCHES "10")
			# Remove all remaining '.' characters.
			string(REPLACE "." "" ver ${ver})
			# Prepend each digit with a zero.
			string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
			set(${version} "0x${ver}")
		endif(CMAKE_SYSTEM_VERSION)
	endmacro(get_WIN32_WINNT)

	get_WIN32_WINNT(ver)
	# E.g., 0x0601 for Win7.
	message(STATUS "_WIN32_WINNT=${ver}")
    # Asio needs this!
	add_definitions(-D_WIN32_WINNT=${ver})
endif(WIN32)

# Group sources by dir.
# Usage: source_group_by_dir(SRCS)
macro(GROUP_SOURCES_BY_DIR source_files)
    set(sgbd_cur_dir ${CMAKE_CURRENT_BINARY_DIR})
    foreach(sgbd_file ${${source_files}})
        #message("sgbd_fpath=${sgbd_fpath}")
        string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
        string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
        string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
        string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
        if(sgbd_nogroup)
            set(sgbd_group_name "\\")
        endif()
        #message("group name=${sgbd_group_name}")
        source_group(${sgbd_group_name} FILES ${sgbd_file})
    endforeach()
endmacro()

# CMake 3.1.0+
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Boost version: 1.66+
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
if(WIN32)
	# TODO: Fix COMPONENTS on Windows.
	find_package(Boost 1.66.0 REQUIRED)
else()
	find_package(Boost 1.66.0 REQUIRED COMPONENTS system thread)
endif()
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    message(STATUS ${Boost_LIBRARIES})
endif()

# For including its own headers as "webcc/http_client.h".
include_directories(${PROJECT_SOURCE_DIR}/src)

# SOAP support needs pugixml to parse and create XML.
if(WEBCC_ENABLE_SOAP)
	add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/pugixml)

	# For including pugixml as "pugixml/pugixml.hpp".
	# TODO: Remove "pugixml" prefix?
	include_directories(${PROJECT_SOURCE_DIR}/third_party)
endif()

add_subdirectory(src/webcc)

if(WEBCC_BUILD_REST_EXAMPLE)
	# REST example needs jsoncpp to parse and create JSON.
	add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/jsoncpp)
	include_directories(${PROJECT_SOURCE_DIR}/third_party/jsoncpp)

	add_subdirectory(${PROJECT_SOURCE_DIR}/example/rest_book_server)
	add_subdirectory(${PROJECT_SOURCE_DIR}/example/rest_book_client)
endif()

if(WEBCC_BUILD_SOAP_EXAMPLE)
	add_subdirectory(${PROJECT_SOURCE_DIR}/example/soap_calc_server)
	add_subdirectory(${PROJECT_SOURCE_DIR}/example/soap_calc_client)
endif()

if(WEBCC_BUILD_UNITTEST)
    add_subdirectory(third_party/gtest)
    add_subdirectory(unittest)
endif()

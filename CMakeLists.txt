cmake_minimum_required(VERSION 3.1.0)

if(POLICY CMP0074)
    # find_package() uses <PackageName>_ROOT variables.
    # This policy was introduced in CMake version 3.12.
    cmake_policy(SET CMP0074 NEW)
endif()

project(webcc)

option(WEBCC_ENABLE_SOAP "Enable SOAP support (need pugixml)?" ON)
option(WEBCC_ENABLE_UNITTEST "Build unit test?" ON)
option(WEBCC_ENABLE_EXAMPLES "Build examples?" ON)

if(WIN32)
    option(WEBCC_ENABLE_VLD "Enable VLD (Visual Leak Detector)?" OFF)
    if(WEBCC_ENABLE_VLD)
        add_definitions(-DWEBCC_ENABLE_VLD)
    endif()
endif()

set(WEBCC_ENABLE_LOG 1 CACHE STRING "Enable logging? (0:OFF, 1:ON)")
set(WEBCC_LOG_LEVEL 2 CACHE STRING "Log level (0:VERB, 1:INFO, 2:WARN, 3:ERRO or 4:FATA)")

if(WEBCC_ENABLE_UNITTEST)
    enable_testing()
endif()

# Automatically detect _WIN32_WINNT for Asio.
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
endif()

if(WIN32)
    # Disable warning on boost string algorithms.
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

# C++ standard requirements.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake 3.1.0+ required.
# See: https://stackoverflow.com/a/29871891
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Boost 1.66+ required.
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
if(WIN32)
    find_package(Boost 1.66.0 REQUIRED)
else()
    find_package(Boost 1.66.0 REQUIRED COMPONENTS system filesystem)
endif()
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    message(STATUS ${Boost_LIBRARIES})
endif()

set(OPENSSL_USE_STATIC_LIBS ON)
set(OPENSSL_MSVC_STATIC_RT ON)
find_package(OpenSSL)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    message(STATUS "OpenSSL libs: " ${OPENSSL_LIBRARIES})
endif()

include_directories(
    # For including its own headers as "webcc/http_client.h".
    ${PROJECT_SOURCE_DIR}
    # For including config.h as "webcc/config.h".
    ${PROJECT_BINARY_DIR}
    )

set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)

if(WIN32)
    include_directories(${THIRD_PARTY_DIR}/win32/include)
    link_directories(${THIRD_PARTY_DIR}/win32/lib)
endif()

# For including pugixml as "pugixml/pugixml.hpp" or gtest as
# "gtest/gtest.h".
include_directories(${THIRD_PARTY_DIR}/src)

# For using zlib on Windows.
if(WIN32)
    add_subdirectory(${THIRD_PARTY_DIR}/src/zlib)

    include_directories(${THIRD_PARTY_DIR}/src/zlib)

    # For including CMake generated zconf.h.
    include_directories(${PROJECT_BINARY_DIR}/third_party/src/zlib)
endif()

# SOAP support needs pugixml to parse and create XML.
if(WEBCC_ENABLE_SOAP)
    add_subdirectory(${THIRD_PARTY_DIR}/src/pugixml)
endif()

add_subdirectory(webcc)

if(WEBCC_ENABLE_EXAMPLES)
    # For including jsoncpp as "json/json.h".
    include_directories(${THIRD_PARTY_DIR}/src/jsoncpp)
    # REST examples need jsoncpp to parse and create JSON.
    add_subdirectory(${THIRD_PARTY_DIR}/src/jsoncpp)

    add_subdirectory(examples)
endif()

if(WEBCC_ENABLE_UNITTEST)
    add_subdirectory(${THIRD_PARTY_DIR}/src/gtest)
    add_subdirectory(unittest)
endif()

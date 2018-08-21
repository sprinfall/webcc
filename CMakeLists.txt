cmake_minimum_required(VERSION 3.1.0)

if(POLICY CMP0074)
    # find_package() uses <PackageName>_ROOT variables.
    # This policy was introduced in CMake version 3.12.
    cmake_policy(SET CMP0074 NEW)
endif()

project(webcc)

option(WEBCC_ENABLE_LOG "Enable logging?" ON)
option(WEBCC_ENABLE_SOAP "Enable SOAP support (need pugixml)?" ON)
option(WEBCC_ENABLE_SSL "Enable SSL/HTTPS support (need OpenSSL)?" OFF)
option(WEBCC_BUILD_UNITTEST "Build unit test?" ON)
option(WEBCC_BUILD_EXAMPLE "Build examples?" ON)

if(WIN32)
    option(WEBCC_ENABLE_VLD "Enable VLD (Visual Leak Detector)?" OFF)
    if(WEBCC_ENABLE_VLD)
        add_definitions(-DWEBCC_ENABLE_VLD)
    endif()
endif()

set(WEBCC_LOG_LEVEL "VERB" CACHE STRING "Log level (VERB, INFO, WARN, ERRO or FATA)")

if(WEBCC_ENABLE_LOG)
    add_definitions(-DWEBCC_ENABLE_LOG)

    message(STATUS "WEBCC_LOG_LEVEL: ${WEBCC_LOG_LEVEL}")

    # Add definition of WEBCC_LOG_LEVEL according variable WEBCC_LOG_LEVEL.
    if(WEBCC_LOG_LEVEL MATCHES "^VERB$")
        add_definitions(-DWEBCC_LOG_LEVEL=0)
    elseif(WEBCC_LOG_LEVEL MATCHES "^INFO$")
        add_definitions(-DWEBCC_LOG_LEVEL=1)
    elseif(WEBCC_LOG_LEVEL MATCHES "^WARN$")
        add_definitions(-DWEBCC_LOG_LEVEL=2)
    elseif(WEBCC_LOG_LEVEL MATCHES "^ERRO$")
        add_definitions(-DWEBCC_LOG_LEVEL=3)
    elseif(WEBCC_LOG_LEVEL MATCHES "^FATA$")
        add_definitions(-DWEBCC_LOG_LEVEL=4)
    endif()
endif()

if(WEBCC_ENABLE_SOAP)
    add_definitions(-DWEBCC_ENABLE_SOAP)
endif()

if(WEBCC_ENABLE_SSL)
    add_definitions(-DWEBCC_ENABLE_SSL)
endif()

if(WEBCC_BUILD_UNITTEST)
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

if(WEBCC_ENABLE_SSL)
    set(OPENSSL_USE_STATIC_LIBS ON)
    set(OPENSSL_MSVC_STATIC_RT ON)
    find_package(OpenSSL)
    if(OPENSSL_FOUND)
        include_directories(${OPENSSL_INCLUDE_DIR})
    endif()
endif()

# For including its own headers as "webcc/http_client.h".
include_directories(${PROJECT_SOURCE_DIR})

if(WIN32)
    include_directories(${PROJECT_SOURCE_DIR}/third_party/win32/include)
    link_directories(${PROJECT_SOURCE_DIR}/third_party/win32/lib)
endif()

# SOAP support needs pugixml to parse and create XML.
if(WEBCC_ENABLE_SOAP)
    add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/src/pugixml)

    # For including pugixml as "pugixml/pugixml.hpp".
    include_directories(${PROJECT_SOURCE_DIR}/third_party/src)
endif()

add_subdirectory(webcc)

if(WEBCC_BUILD_EXAMPLE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/example/http_hello_client)
    add_subdirectory(${PROJECT_SOURCE_DIR}/example/http_hello_async_client)

    # REST example needs jsoncpp to parse and create JSON.
    add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/src/jsoncpp)
    include_directories(${PROJECT_SOURCE_DIR}/third_party/src/jsoncpp)

    add_subdirectory(${PROJECT_SOURCE_DIR}/example/rest_book_server)
    add_subdirectory(${PROJECT_SOURCE_DIR}/example/rest_book_client)
    add_subdirectory(${PROJECT_SOURCE_DIR}/example/rest_book_async_client)

    if(WEBCC_ENABLE_SOAP)
        add_subdirectory(${PROJECT_SOURCE_DIR}/example/soap_calc_server)
        add_subdirectory(${PROJECT_SOURCE_DIR}/example/soap_calc_client)
        add_subdirectory(${PROJECT_SOURCE_DIR}/example/soap_calc_client_parasoft)
        add_subdirectory(${PROJECT_SOURCE_DIR}/example/soap_book_server)
        add_subdirectory(${PROJECT_SOURCE_DIR}/example/soap_book_client)
    endif()

    if(WEBCC_ENABLE_SSL)
        add_subdirectory(${PROJECT_SOURCE_DIR}/example/http_ssl_client)
    endif()
endif()

if(WEBCC_BUILD_UNITTEST)
    add_subdirectory(third_party/src/gtest)
    add_subdirectory(unittest)
endif()

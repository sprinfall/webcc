cmake_minimum_required(VERSION 3.11)

project(webcc)

# Output directories
set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${BUILD_DIR}/bin/debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BUILD_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BUILD_DIR}/bin/debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${BUILD_DIR}/bin/release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BUILD_DIR}/bin/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BUILD_DIR}/bin/release)

option(WEBCC_ENABLE_AUTOTEST "Build automation test?" OFF)
option(WEBCC_ENABLE_UNITTEST "Build unit test?" OFF)
option(WEBCC_ENABLE_EXAMPLES "Build examples?" OFF)

set(WEBCC_ENABLE_LOG   1 CACHE STRING "Enable logging? (1:Yes, 0:No)")
set(WEBCC_ENABLE_SSL   0 CACHE STRING "Enable SSL/HTTPS (need OpenSSL)? (1:Yes, 0:No)")
set(WEBCC_ENABLE_GZIP  0 CACHE STRING "Enable gzip compression (need Zlib)? (1:Yes, 0:No)")

set(WEBCC_LOG_LEVEL    2 CACHE STRING "Log level (0:VERB, 1:INFO, 2:USER, 3:WARN or 4:ERRO)")

if(WEBCC_ENABLE_UNITTEST)
    enable_testing()
endif()

if(WIN32)
    # Asio needs this!
    # 0x0601 means Win7. So our application targets Win7 and above.
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

# C++ standard requirements.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake 3.1.0+ required.
# See: https://stackoverflow.com/a/29871891
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(WEBCC_ENABLE_SSL)
    # Commented on 20190529.
    # The static libs have linkage issues with VS2015 on Win10.
    #   set(OPENSSL_USE_STATIC_LIBS ON)
    #   set(OPENSSL_MSVC_STATIC_RT ON)
    find_package(OpenSSL)
    if(OPENSSL_FOUND)
        include_directories(${OPENSSL_INCLUDE_DIR})
        message(STATUS "OpenSSL libs: " ${OPENSSL_LIBRARIES})
    endif()
endif()

include_directories(
    # For including its own headers as "webcc/client.h".
    ${PROJECT_SOURCE_DIR}
    # For including config.h as "webcc/config.h".
    ${PROJECT_BINARY_DIR}
    )

set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)

# For Asio
include_directories(${THIRD_PARTY_DIR}/include)

# For jsoncpp
include_directories(${THIRD_PARTY_DIR}/src)

if(WEBCC_ENABLE_GZIP)
    # For using zlib on Windows.
    if(WIN32)
        add_subdirectory(${THIRD_PARTY_DIR}/src/zlib)

        include_directories(${THIRD_PARTY_DIR}/src/zlib)

        # For including CMake generated zconf.h.
        include_directories(${PROJECT_BINARY_DIR}/third_party/src/zlib)
    else()
        find_package(ZLIB REQUIRED)
        if(ZLIB_FOUND)
            include_directories(${ZLIB_INCLUDE_DIRS})
        endif()
    endif()
endif()

add_subdirectory(webcc)

if(WEBCC_ENABLE_AUTOTEST OR WEBCC_ENABLE_EXAMPLES)
    # For including jsoncpp as "json/json.h".
    include_directories(${THIRD_PARTY_DIR}/src/jsoncpp)

    add_subdirectory(${THIRD_PARTY_DIR}/src/jsoncpp)
endif()

# GTest
if(WEBCC_ENABLE_AUTOTEST OR WEBCC_ENABLE_UNITTEST)
    find_package(GTest REQUIRED)
    if(GTEST_FOUND)
        add_definitions(-DGTEST_LANG_CXX11=1)
        include_directories(${GTEST_INCLUDE_DIRS})
    endif()
endif()

if(WEBCC_ENABLE_AUTOTEST)
    add_subdirectory(autotest)
endif()

if(WEBCC_ENABLE_UNITTEST)
    add_subdirectory(unittest)
endif()

if(WEBCC_ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()

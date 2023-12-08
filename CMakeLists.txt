cmake_minimum_required(VERSION 3.11)

if(POLICY CMP0074)
    # find_package() uses <PackageName>_ROOT variables.
    # This policy was introduced in CMake version 3.12.
    cmake_policy(SET CMP0074 NEW)
endif()

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

option(WEBCC_BUILD_UNITTEST "Build unit test?" OFF)
option(WEBCC_BUILD_AUTOTEST "Build automation test?" OFF)
option(WEBCC_BUILD_EXAMPLES "Build examples?" ON)
option(WEBCC_BUILD_QT_EXAMPLES "Build Qt application examples?" OFF)

# Only for 'webcc' library itself.
# The tests and examples are not affected by this option.
# Webcc always uses boost::asio::chrono to avoid the direct references to
# std::chrono or boost::chrono.
option(WEBCC_DISABLE_STD_CHRONO "Disable std::chrono and use boost::chrono instead?" OFF)

set(WEBCC_LOG_LEVEL 2
    CACHE STRING "Log level (0:VERB, 1:INFO, 2:USER, 3:WARN, 4:ERRO, >=5:NONE)"
    )
set(WEBCC_ENABLE_GZIP 0
    CACHE STRING "Enable gzip compression (need Zlib)? (1:Yes, 0:No)"
    )

if(WEBCC_BUILD_UNITTEST)
    enable_testing()
endif()

if(WIN32)
    # Asio needs this!
    # 0x0601 means Win7. So our application targets Win7 and above.
    add_compile_definitions(_WIN32_WINNT=0x0601)
endif()

# C++ standard requirements.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Boost

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

set(BOOST_MIN_VERSION 1.74.0)

if(WEBCC_DISABLE_STD_CHRONO)
    add_compile_definitions(BOOST_ASIO_DISABLE_STD_CHRONO)
endif()

if(WEBCC_DISABLE_STD_CHRONO)
    find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS chrono)
else()
    find_package(Boost ${BOOST_MIN_VERSION} REQUIRED)
endif()

if(Boost_FOUND)
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# Commented on 20190529.
# The static libs have linkage issues with VS2015 on Win10.
#   set(OPENSSL_USE_STATIC_LIBS ON)
#   set(OPENSSL_MSVC_STATIC_RT ON)
find_package(OpenSSL)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    message(STATUS "OpenSSL libs: " ${OPENSSL_LIBRARIES})
endif()

if(WEBCC_ENABLE_GZIP)
    find_package(ZLIB REQUIRED)
    if(ZLIB_FOUND)
        # You can link to ${ZLIB_LIBRARIES} or the imported target ZLIB::ZLIB.
        include_directories(${ZLIB_INCLUDE_DIRS})
    endif()
endif()

include_directories(
    # For including its own headers as "webcc/client.h".
    ${PROJECT_SOURCE_DIR}
    # For including config.h as "webcc/config.h".
    ${PROJECT_BINARY_DIR}
    )

set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)

# For jsoncpp
include_directories(${THIRD_PARTY_DIR}/src)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(webcc)

if(WEBCC_BUILD_AUTOTEST OR WEBCC_BUILD_EXAMPLES)
    # For including jsoncpp as "json/json.h".
    include_directories(${THIRD_PARTY_DIR}/src/jsoncpp)
    add_subdirectory(${THIRD_PARTY_DIR}/src/jsoncpp)
endif()

# GTest
if(WEBCC_BUILD_AUTOTEST OR WEBCC_BUILD_UNITTEST)
    find_package(GTest REQUIRED)
    if(GTEST_FOUND)
        add_definitions(-DGTEST_LANG_CXX11=1)
        include_directories(${GTEST_INCLUDE_DIRS})
    endif()
endif()

if(WEBCC_BUILD_AUTOTEST)
    add_subdirectory(autotest)
endif()

if(WEBCC_BUILD_UNITTEST)
    add_subdirectory(unittest)
endif()

if(WEBCC_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

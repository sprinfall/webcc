# Unit test

file(GLOB UT_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

set(UT_TARGET_NAME webcc_unittest)

# Common libraries to link.
set(UT_LIBS
    webcc
    GTest::GTest
    GTest::Main)

if(UNIX)
    # Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
    set(UT_LIBS ${UT_LIBS} ${CMAKE_DL_LIBS})
endif()

add_executable(${UT_TARGET_NAME} ${UT_SRCS})
target_link_libraries(${UT_TARGET_NAME} ${UT_LIBS})

add_test(${UT_TARGET_NAME} ${UT_TARGET_NAME})

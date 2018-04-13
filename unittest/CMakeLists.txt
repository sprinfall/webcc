# Unit test
if(WEBCC_BUILD_UNITTEST)
    set(UT_SRCS
        rest_service_manager_test.cc
        )

    set(UT_TARGET_NAME webcc_unittest)

    add_executable(${UT_TARGET_NAME} ${UT_SRCS})
    target_link_libraries(${UT_TARGET_NAME} webcc pugixml gtest ${Boost_LIBRARIES})
    target_link_libraries(${UT_TARGET_NAME} "${CMAKE_THREAD_LIBS_INIT}")

    add_test(${UT_TARGET_NAME} ${UT_TARGET_NAME})
endif()

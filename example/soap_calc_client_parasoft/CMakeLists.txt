set(TARGET_NAME soap_calc_client_parasoft)

add_executable(${TARGET_NAME} main.cc)

target_link_libraries(${TARGET_NAME} webcc pugixml ${Boost_LIBRARIES})
target_link_libraries(${TARGET_NAME} "${CMAKE_THREAD_LIBS_INIT}")

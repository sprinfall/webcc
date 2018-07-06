set(SRCS calc_client.cc calc_client.h main.cc)

add_executable(soap_calc_client ${SRCS})

target_link_libraries(soap_calc_client webcc pugixml ${Boost_LIBRARIES})
target_link_libraries(soap_calc_client "${CMAKE_THREAD_LIBS_INIT}")


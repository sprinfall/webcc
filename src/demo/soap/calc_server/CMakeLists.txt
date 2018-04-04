file(GLOB SRCS *.cc *.h)

add_executable(soap_calc_server ${SRCS})

target_link_libraries(soap_calc_server webcc pugixml)

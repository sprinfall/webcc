file(GLOB SRCS *.cc *.h)

add_executable(soap_calc_client ${SRCS})

target_link_libraries(soap_calc_client webcc pugixml)

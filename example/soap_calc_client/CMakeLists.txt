set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SRCS *.cc *.h)

add_executable(soap_calc_client ${SRCS})

target_link_libraries(soap_calc_client webcc pugixml ${Boost_LIBRARIES})
target_link_libraries(soap_calc_client "${CMAKE_THREAD_LIBS_INIT}")


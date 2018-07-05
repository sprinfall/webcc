set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRCS
	json/json.h
	json/json-forwards.h
	jsoncpp.cpp
	)

add_library(jsoncpp STATIC ${SRCS})

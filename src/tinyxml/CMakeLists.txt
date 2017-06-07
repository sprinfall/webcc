# TinyXml uses STL.
add_definitions(-DTIXML_USE_STL)

file(GLOB SRCS *.cpp *.h)

add_library(tinyxml ${SRCS})

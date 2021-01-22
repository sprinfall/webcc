project(qt_app_server)

find_package(Qt5 CONFIG REQUIRED Core Gui Widgets)

set(CMAKE_AUTOMOC ON)

file(GLOB SRCS *.cpp *.h)

add_executable(qt_app_server ${SRCS})

target_link_libraries(qt_app_server Qt5::Core Qt5::Gui Qt5::Widgets ${EXAMPLE_LIBS})

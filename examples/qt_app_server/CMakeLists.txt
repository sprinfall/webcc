find_package(Qt5 CONFIG REQUIRED Core Gui Widgets)

set(CMAKE_AUTOMOC ON)

set(SRCS main.cc main_window.cc main_window.h)

add_executable(qt_app_server ${SRCS})

target_link_libraries(qt_app_server Qt5::Core Qt5::Gui Qt5::Widgets ${EXAMPLE_LIBS})
set_target_properties(qt_app_server PROPERTIES FOLDER "Examples")

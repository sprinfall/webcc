add_executable(github_client main.cc)
target_link_libraries(github_client ${EXAMPLE_COMMON_LIBS} jsoncpp)

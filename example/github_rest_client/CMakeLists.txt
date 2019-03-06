add_executable(github_rest_client main.cc)
target_link_libraries(github_rest_client ${EXAMPLE_COMMON_LIBS} jsoncpp)

include_directories(../include)

add_executable(http_rpc_relay_tests_exe
    test_general.cpp
    environment.cpp
    ${GTEST_PATH}/src/gtest_main.cc
    )

target_include_directories(http_rpc_relay_tests_exe PRIVATE ${GTEST_PATH}/include)

target_link_libraries(http_rpc_relay_tests_exe
    gtest
    http_rpc_relay_lib
    -ljsoncpp
    ${CONAN_LIBS}
    )

add_test(
    NAME etcd-beast-tests
    COMMAND etcd-beast-tests
    --datadir ${CMAKE_CURRENT_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

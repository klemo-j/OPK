find_package(GTest REQUIRED)

add_executable(unit_tests test.cpp)


target_link_libraries(unit_tests robot_lib GTest::gtest GTest::gtest_main)

add_test(NAME AllTests COMMAND unit_tests)
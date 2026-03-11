
add_executable(fifo_tests tests.cpp)


target_link_libraries(fifo_tests PRIVATE fifo_lib GTest::gtest GTest::gtest_main)


target_include_directories(fifo_tests PRIVATE ${CMAKE_SOURCE_DIR}/include/)
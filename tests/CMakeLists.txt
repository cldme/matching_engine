add_executable(test_matching_engine test_matching_engine.cpp)
target_link_libraries(test_matching_engine libs GTest::GTest GTest::Main)

include(GoogleTest)
gtest_discover_tests(test_matching_engine)

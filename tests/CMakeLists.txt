# CMake configuration for unit tests

# Enable testing
enable_testing()

# Find GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add test executables
add_executable(test_logger test_logger.cpp ../src/logger.cpp)
target_link_libraries(test_logger GTest::GTest GTest::Main pthread)
add_test(NAME LoggerTest COMMAND test_logger)

add_executable(test_tlp_manager test_tlp_manager.cpp ../src/tlp_manager.cpp)
target_link_libraries(test_tlp_manager GTest::GTest GTest::Main pthread)
add_test(NAME TLPManagerTest COMMAND test_tlp_manager)

add_executable(test_activity_monitor test_activity_monitor.cpp ../src/activity_monitor.cpp)
target_link_libraries(test_activity_monitor GTest::GTest GTest::Main pthread)
add_test(NAME ActivityMonitorTest COMMAND test_activity_monitor)

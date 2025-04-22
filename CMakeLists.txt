cmake_minimum_required(VERSION 3.10)
project(matching_engine)

# setting cpp standard
set(CMAKE_CXX_STANDARD 17)

# find packages
find_package(absl REQUIRED)
find_package(GTest REQUIRED)

# enable testing
enable_testing()

# add project subdirectories
add_subdirectory(libs)
add_subdirectory(tests)

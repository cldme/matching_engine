add_executable(app app.cpp)
target_link_libraries(app PRIVATE libs replxx::replxx CLI11::CLI11)
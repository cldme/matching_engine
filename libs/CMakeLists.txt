add_library(libs level.cpp order_book.cpp order.cpp)
target_link_libraries(libs PRIVATE absl::flat_hash_map)
target_include_directories(libs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

cmake_minimum_required(VERSION 3.16)
project(lru_kvstore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(kvstore src/kv_store.cpp)
target_include_directories(kvstore PUBLIC include)

add_executable(kvcli src/main.cpp)
target_link_libraries(kvcli PRIVATE kvstore)

enable_testing()
add_executable(kv_tests tests/test_main.cpp)
target_link_libraries(kv_tests PRIVATE kvstore)
add_test(NAME kv_tests COMMAND kv_tests)

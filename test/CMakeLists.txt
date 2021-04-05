add_executable(${PROJECT_NAME}-test test.cpp)
cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)

include_directories(${GTEST_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME}-test
  PUBLIC
  ../src
  )

target_link_libraries(${PROJECT_NAME}-test
  GTest::GTest GTest::Main
  )

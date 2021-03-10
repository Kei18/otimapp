cmake_minimum_required(VERSION 3.16)
project(otimapp CXX)

add_subdirectory(./otimapp)
add_subdirectory(./third_party/googletest)

add_executable(app app.cpp)
target_compile_features(app PUBLIC cxx_std_17)
target_link_libraries(app lib-otimapp)

add_executable(exec exec.cpp)
target_compile_features(exec PUBLIC cxx_std_17)
target_link_libraries(exec lib-otimapp)

# format
add_custom_target(clang-format
  COMMAND clang-format -i
  ../otimapp/include/*.hpp
  ../otimapp/src/*.cpp
  ../tests/*.cpp
  ../app.cpp)

# test
set(TEST_MAIN_FUNC ./third_party/googletest/googletest/src/gtest_main.cc)
set(TEST_ALL_SRC ${TEST_MAIN_FUNC})
macro(add_test name target)
  add_executable(${name} ${target} ${TEST_MAIN_FUNC})
  target_link_libraries(${name} lib-otimapp gtest)
  list(APPEND TEST_ALL_SRC ${target})
endmacro(add_test)

# basic
add_test(test_problem ./tests/test_problem.cpp)
add_test(test_agent ./tests/test_agent.cpp)
add_test(test_execution ./tests/test_execution.cpp)
add_test(test_fragment ./tests/test_fragment.cpp)
# solver
add_test(test_pp ./tests/test_pp.cpp)
add_test(test_cp ./tests/test_cp.cpp)
#
add_executable(test ${TEST_ALL_SRC})
target_link_libraries(test lib-otimapp gtest)

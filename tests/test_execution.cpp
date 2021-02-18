#include <execution.hpp>

#include "gtest/gtest.h"

TEST(Execution, basic)
{
  Problem P = Problem("../tests/instances/toy_problem.txt");
  auto exec = Execution(&P, "../tests/instances/toy_problem_plan.txt");
  exec.run();
  ASSERT_TRUE(validate(exec.getExecResult(), &P));
}

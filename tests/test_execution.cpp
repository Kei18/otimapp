#include <execution.hpp>

#include "gtest/gtest.h"

TEST(MAPF_DP_Execution, basic)
{
  Problem P = Problem("../tests/instances/toy_problem.txt");
  auto exec = MAPF_DP_Execution(&P, "../tests/instances/toy_problem_plan.txt");
  exec.run();
  ASSERT_TRUE(validateMAPFPlan(exec.getExecResult(), &P));
}

TEST(PrimitiveExecution, basic)
{
  Problem P = Problem("../tests/instances/toy_problem.txt");
  auto exec = PrimitiveExecution(&P, "../tests/instances/toy_problem_plan.txt");
  exec.run();
}

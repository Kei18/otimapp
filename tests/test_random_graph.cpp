#include <dbs.hpp>
#include <default_params.hpp>
#include <problem.hpp>

#include "gtest/gtest.h"

TEST(Problem, random_graph)
{
  auto P = Problem(50, 0.1, 3, 0);
  auto solver = std::make_unique<DBS>(&P);
  solver->solve();
  ASSERT_TRUE(solver->succeed());
  solver->makeLog(DEFAULT_PLAN_OUTPUT_FILE);
}

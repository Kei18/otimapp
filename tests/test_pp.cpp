#include <prioritized_planning.hpp>

#include "gtest/gtest.h"

TEST(PrioritizedPlanning, solve)
{
  Problem P = Problem("../tests/instances/example.txt");
  auto solver = std::make_unique<PrioritizedPlanning>(&P);
  solver->solve();

  ASSERT_TRUE(solver->succeed());
}

#include <complete_planning.hpp>

#include "gtest/gtest.h"

TEST(CompletePlanning, solve)
{
  Problem P = Problem("../tests/instances/example.txt");
  auto solver = std::make_unique<CompletePlanning>(&P);
  solver->solve();

  ASSERT_TRUE(solver->succeed());
}

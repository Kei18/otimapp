#include <dbs.hpp>

#include "gtest/gtest.h"

TEST(DBS, solve)
{
  Problem P = Problem("../tests/instances/example.txt");
  auto solver = std::make_unique<DBS>(&P);
  solver->solve();

  ASSERT_TRUE(solver->succeed());
}

#include <pp.hpp>

#include "gtest/gtest.h"

TEST(PP, solve)
{
  Problem P = Problem("../tests/instances/example.txt");
  auto solver = std::make_unique<PP>(&P);
  solver->solve();

  ASSERT_TRUE(solver->succeed());
}

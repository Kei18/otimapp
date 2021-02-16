#include <problem.hpp>

#include "gtest/gtest.h"

TEST(Problem, loading)
{
  Problem P = Problem("../tests/instances/toy_problem.txt");
  Graph* G = P.getG();

  ASSERT_EQ(P.getNum(), 2);
  ASSERT_EQ(P.getMaxCompTime(), 1000);

  Config starts = P.getConfigStart();
  ASSERT_EQ(starts.size(), 2);
  ASSERT_EQ(starts[0], G->getNode(0, 0));
  ASSERT_EQ(starts[1], G->getNode(1, 1));

  Config goals = P.getConfigGoal();
  ASSERT_EQ(goals.size(), 2);
  ASSERT_EQ(goals[0], G->getNode(1, 0));
  ASSERT_EQ(goals[1], G->getNode(0, 1));
}

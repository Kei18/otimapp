#include <cycle_candidate.hpp>

#include "gtest/gtest.h"

TEST(CycleCandidate, createNewEntry)
{
  auto G = Grid("8x8.map");
  auto table = TableCycle(G.getNodesSize());

  auto c1 = table.createNewCycleCandidate(0, G.getNode(0), nullptr, G.getNode(1));

  // add tail
  auto c2 = table.createNewCycleCandidate(1, G.getNode(0), c1, G.getNode(2));
  ASSERT_EQ(c2->path[0], G.getNode(0));
  ASSERT_EQ(c2->path[1], G.getNode(1));
  ASSERT_EQ(c2->path[2], G.getNode(2));
  ASSERT_EQ(c2->agents[0], 0);
  ASSERT_EQ(c2->agents[1], 1);

  // add head
  auto c3 = table.createNewCycleCandidate(2, G.getNode(8), c2, G.getNode(2));
  ASSERT_EQ(c3->path[0], G.getNode(8));
  ASSERT_EQ(c3->path[1], G.getNode(0));
  ASSERT_EQ(c3->path[2], G.getNode(1));
  ASSERT_EQ(c3->path[3], G.getNode(2));
  ASSERT_EQ(c3->agents[0], 2);
  ASSERT_EQ(c3->agents[1], 0);
  ASSERT_EQ(c3->agents[2], 1);

  std::deque<Node*> path = { G.getNode(0), G.getNode(1), G.getNode(2) };
  std::deque<int> agents = { 1, 0 };
  ASSERT_TRUE(table.existDuplication(path, agents));
}

TEST(CycleCandidate, registerNewPath)
{
  auto G = Grid("8x8.map");
  auto table = TableCycle(G.getNodesSize());

  Path p1 = { G.getNode(0), G.getNode(1), G.getNode(2) };
  auto c1 = table.registerNewPath(0, p1);
  ASSERT_EQ(c1, nullptr);

  Path p2 = { G.getNode(3), G.getNode(2), G.getNode(1) };
  auto c2 = table.registerNewPath(1, p2);
  ASSERT_NE(c2, nullptr);

  // self loop
  Path p3 = { G.getNode(8), G.getNode(9), G.getNode(17), G.getNode(16), G.getNode(8) };
  auto c3 = table.registerNewPath(2, p3);
  ASSERT_EQ(c3, nullptr);
}

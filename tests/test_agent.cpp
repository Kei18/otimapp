#include <agent.hpp>

#include "gtest/gtest.h"

TEST(Agent, basic)
{
  Grid G = Grid("8x8.map");
  auto p = G.getPath(G.getNode(0), G.getNode(1));
  std::vector<int> occupancy(G.getNodesSize(), Agent::NIL);
  auto a = Agent(0, p);

  // initial condition
  ASSERT_EQ(a.mode, Agent::CONTRACTED);
  ASSERT_EQ(a.head, nullptr);
  ASSERT_EQ(a.tail, G.getNode(0));
  ASSERT_EQ(a.t, 0);

  ASSERT_EQ(a.getNextNode(), G.getNode(1));

  // move one step
  a.activate(occupancy);
  ASSERT_EQ(a.mode, Agent::EXTENDED);
  ASSERT_EQ(a.head, G.getNode(1));
  ASSERT_EQ(a.tail, G.getNode(0));
  ASSERT_EQ(a.t, 1);

  // move one step
  a.activate(occupancy);
  ASSERT_EQ(a.mode, Agent::CONTRACTED);
  ASSERT_EQ(a.head, nullptr);
  ASSERT_EQ(a.tail, G.getNode(1));
  ASSERT_EQ(a.t, 1);

  ASSERT_TRUE(a.isFinished());
}

#include <agent.hpp>

#include "gtest/gtest.h"

TEST(MAPF_DP_Agent, basic)
{
  Grid G = Grid("8x8.map");
  auto p = G.getPath(G.getNode(0), G.getNode(1));
  std::vector<int> occupancy(G.getNodesSize(), MAPF_DP_Agent::NIL);
  auto a = MAPF_DP_Agent(0, p);

  // initial condition
  ASSERT_EQ(a.mode, MAPF_DP_Agent::CONTRACTED);
  ASSERT_EQ(a.head, nullptr);
  ASSERT_EQ(a.tail, G.getNode(0));
  ASSERT_EQ(a.t, 0);

  ASSERT_EQ(a.getNextNode(), G.getNode(1));

  // move one step
  a.activate(occupancy);
  ASSERT_EQ(a.mode, MAPF_DP_Agent::EXTENDED);
  ASSERT_EQ(a.head, G.getNode(1));
  ASSERT_EQ(a.tail, G.getNode(0));
  ASSERT_EQ(a.t, 1);

  // move one step
  a.activate(occupancy);
  ASSERT_EQ(a.mode, MAPF_DP_Agent::CONTRACTED);
  ASSERT_EQ(a.head, nullptr);
  ASSERT_EQ(a.tail, G.getNode(1));
  ASSERT_EQ(a.t, 1);

  ASSERT_TRUE(a.isFinished());
}

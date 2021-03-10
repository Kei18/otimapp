#include <fragment.hpp>

#include "gtest/gtest.h"

TEST(TableFragment, registerNewPath)
{
  auto G = Grid("8x8.map");
  auto table = TableFragment(&G);

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

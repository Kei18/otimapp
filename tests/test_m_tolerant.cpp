#include <pp.hpp>
#include <dbs.hpp>

#include "gtest/gtest.h"

TEST(PP, m_tolerant1)
{
  auto P = Problem("../tests/instances/m-tolerant.txt");

  char argv0[] = "-m";
  char argv1[] = "1";
  char argv2[] = "-f";
  char argv3[] = "1";

  auto solver1 = std::make_unique<PP>(&P);
  char* argv_solver1[] = {argv0, argv1};
  solver1->setParams(2, argv_solver1);
  solver1->solve();
  ASSERT_FALSE(solver1->succeed());

  auto solver2 = std::make_unique<PP>(&P);
  char* argv_solver2[] = {argv0, argv1, argv2, argv3};
  solver2->setParams(4, argv_solver2);
  solver2->solve();
  ASSERT_TRUE(solver2->succeed());
}

TEST(PP, m_tolerant2)
{
  auto P = Problem("../tests/instances/m-tolerant2.txt");

  char argv0[] = "-m";
  char argv1[] = "1";
  char argv2[] = "-f";
  char argv3[] = "3";

  auto solver1 = std::make_unique<PP>(&P);
  char* argv_solver1[] = {argv0, argv1};
  solver1->setParams(2, argv_solver1);
  solver1->solve();
  ASSERT_FALSE(solver1->succeed());

  auto solver2 = std::make_unique<PP>(&P);
  char* argv_solver2[] = {argv0, argv1, argv2, argv3};
  solver2->setParams(4, argv_solver2);
  solver2->solve();
  ASSERT_TRUE(solver2->succeed());
}

TEST(DBS, m_tolerant1)
{
  auto P = Problem("../tests/instances/m-tolerant.txt");

  char argv0[] = "-m";
  char argv1[] = "1";
  char argv2[] = "-f";
  char argv3[] = "1";

  auto solver1 = std::make_unique<DBS>(&P);
  char* argv_solver1[] = {argv0, argv1};
  solver1->setParams(2, argv_solver1);
  solver1->solve();
  ASSERT_FALSE(solver1->succeed());

  auto solver2 = std::make_unique<DBS>(&P);
  char* argv_solver2[] = {argv0, argv1, argv2, argv3};
  solver2->setParams(4, argv_solver2);
  solver2->solve();
  ASSERT_TRUE(solver2->succeed());
}

TEST(DBS, m_tolerant2)
{
  auto P = Problem("../tests/instances/m-tolerant2.txt");

  char argv0[] = "-m";
  char argv1[] = "1";
  char argv2[] = "-f";
  char argv3[] = "3";

  auto solver1 = std::make_unique<DBS>(&P);
  char* argv_solver1[] = {argv0, argv1};
  solver1->setParams(2, argv_solver1);
  solver1->solve();
  ASSERT_FALSE(solver1->succeed());

  auto solver2 = std::make_unique<DBS>(&P);
  char* argv_solver2[] = {argv0, argv1, argv2, argv3};
  solver2->setParams(4, argv_solver2);
  solver2->solve();
  ASSERT_TRUE(solver2->succeed());
}

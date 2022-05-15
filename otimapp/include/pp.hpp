/*
 * Implementation of PP: prioritized planning
 */

#pragma once
#include "solver.hpp"

class PP : public Solver
{
public:
  static const std::string SOLVER_NAME;

private:
  int iter_cnt_max;  // maximum number of randomization of priorities
  static constexpr int DEFAULT_ITER_CNT_MAX = 10;

  int max_fragment_size;  // maximum fragment size
  static constexpr int DEFAULT_MAX_FRAGMENT_SIZE = -1;

  // main
  void run();

public:
  PP(Problem* _P);
  ~PP();

  void setParams(int argc, char* argv[]);
  static void printHelp();
};

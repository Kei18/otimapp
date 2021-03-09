/*
 * Implementation of Prioritized Planning
 */

#pragma once
#include "solver.hpp"

class PrioritizedPlanning : public Solver
{
public:
  static const std::string SOLVER_NAME;

private:
  int iter_cnt_max;
  static constexpr int DEFAULT_ITER_CNT_MAX = 10;

  int max_fragment_size;
  static constexpr int DEFAULT_MAX_FRAGMENT_SIZE = -1;

  // main
  void run();

public:
  PrioritizedPlanning(Problem* _P);
  ~PrioritizedPlanning();

  void setParams(int argc, char* argv[]);
  static void printHelp();
};

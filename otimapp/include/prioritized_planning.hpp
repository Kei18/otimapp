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
  std::vector<bool> table_goals;

  // main
  void run();

  Path getPrioritizedPath(const int id, const std::vector<Path>& paths);

public:
  PrioritizedPlanning(Problem* _P);
  ~PrioritizedPlanning() {}

  void setParams(int argc, char* argv[]);
  static void printHelp();
};

/*
 * Implementation of Prioritized Planning
 */

#pragma once
#include "solver.hpp"
#include "cycle_candidate.hpp"

class PrioritizedPlanning : public Solver
{
public:
  static const std::string SOLVER_NAME;

private:
  // main
  void run();

  // get a single-agent path
  Path getPrioritizedPath(const int id, const Plan& paths, TableCycle& table);

public:
  PrioritizedPlanning(Problem* _P);
  ~PrioritizedPlanning();

  void setParams(int argc, char* argv[]);
  static void printHelp();
};

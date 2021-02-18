/*
 * Implementation of Prioritized Planning
 */

#pragma once
#include "solver.hpp"
#include <deque>


class PrioritizedPlanning : public Solver
{
public:
  static const std::string SOLVER_NAME;

private:
  // main
  void run();

  // get a single-agent path
  Path getPrioritizedPath(const int id, const Plan& paths);

  // to detect potential deadlocks efficiently
  using CycleCandidate = std::deque<Node*>;
  // potential cycles ends at the node
  std::vector<std::vector<CycleCandidate*>> table_cycle_tail;
  // potential cycles starts from the node
  std::vector<std::vector<CycleCandidate*>> table_cycle_head;

  // update cycle
  void registerCycle(const int id, const Path path);
  // used in getPrioritizedPath to detect prohibited path
  bool detectLoopByCycle(const int id, Node* child, Node* parent);

public:
  PrioritizedPlanning(Problem* _P);
  ~PrioritizedPlanning();

  void setParams(int argc, char* argv[]);
  static void printHelp();
};

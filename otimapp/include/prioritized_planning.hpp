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

  Path getPrioritizedPath(const int id, const Plan& paths);

  using Cycle = std::deque<Node*>;
  std::vector<std::vector<Cycle*>> table_cycle_tail;
  std::vector<std::vector<Cycle*>> table_cycle_head;

  void registerCycle(const int id, const Path path);
  bool detectLoopByCycle(const int id, Node* child, Node* parent);

public:
  PrioritizedPlanning(Problem* _P);
  ~PrioritizedPlanning();

  void setParams(int argc, char* argv[]);
  static void printHelp();
};

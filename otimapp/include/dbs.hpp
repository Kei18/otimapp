/*
 * Implementation of DBS
 */

#pragma once
#include <memory>

#include "solver.hpp"

class DBS : public Solver
{
public:
  static const std::string SOLVER_NAME;

private:
  // main
  void run();

  struct Constraint {
    int agent;     // agent
    Node* parent;  // from
    Node* child;   // to

    Constraint(int i, Node* u, Node* v) : agent(i), parent(u), child(v) {}
  };
  using Constraint_p = std::shared_ptr<Constraint>;
  using Constraints = std::vector<Constraint_p>;

  struct HighLevelNode {
    Plan paths;               // solution
    Constraints constraints;  // constraints
    int f;                    // #(head-on collisions)
    bool valid;               // false -> no path is found

    HighLevelNode() : constraints({}), f(0), valid(true) {}
  };
  using HighLevelNode_p = std::shared_ptr<HighLevelNode>;
  using HighLevelNodes = std::vector<HighLevelNode_p>;

  // setup initial node
  HighLevelNode_p getInitialNode();

  // invoke high-level node
  HighLevelNode_p invoke(HighLevelNode_p n, Constraint_p c);

  // low-level search
  Path getConstrainedPath(const int id, HighLevelNode_p node);
  Path getConstrainedPath(const int id, Constraints& _constraints);

  // get constraints
  Constraints getConstraints(const Plan& paths) const;

  // count #(head-on collisions)
  int countsSwapConlicts(const Plan& paths);

public:
  DBS(Problem* _P);
  ~DBS();

  void setParams(int argc, char* argv[]);
  static void printHelp();
};

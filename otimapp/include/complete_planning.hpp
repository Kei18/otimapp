/*
 * Implementation of Prioritized Planning
 */

#pragma once
#include "solver.hpp"
#include <memory>


class CompletePlanning : public Solver
{
public:
  static const std::string SOLVER_NAME;

private:
  // main
  void run();

  struct Constraint {
    int agent;
    Node* parent;
    Node* child;

    Constraint(int i, Node* u, Node* v)
      : agent(i), parent(u), child(v) {}

    void println()
    {
      std::cout << "constraint"
                << ", agent: " << agent
                << ", parent: " << parent->id
                << ", child: " << child->id
                << std::endl;
    }
  };
  using Constraint_p = std::shared_ptr<Constraint>;
  using Constraints = std::vector<Constraint_p>;

  struct HighLevelNode {
    Plan paths;
    Constraints constraints;
    int f;                     // sum of path lengths
    bool valid;

    HighLevelNode()
      : constraints({}), f(0), valid(true) {}
    HighLevelNode(const Plan& p, Constraints c, int _f, bool _valid)
      : paths(p), constraints(c), f(_f), valid(_valid) {}
  };
  using HighLevelNode_p = std::shared_ptr<HighLevelNode>;
  using HighLevelNodes = std::vector<HighLevelNode_p>;

  HighLevelNode_p getInitialNode();
  HighLevelNode_p invoke(HighLevelNode_p n, Constraint_p c);
  Path getConstrainedPath(const int id, const Constraints& constraints={});
  Constraints getConstraints(const Plan& paths) const;

public:
  CompletePlanning(Problem* _P);
  ~CompletePlanning();

  void setParams(int argc, char* argv[]);
  static void printHelp();
};

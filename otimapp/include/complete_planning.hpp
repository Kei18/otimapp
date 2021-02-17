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
  std::vector<bool> table_goals;

  // main
  void run();

  struct Constraint {
    int agent;
    Node* parent;
    Node* child;

    Constraint(int i, Node* u, Node* v)
      : agent(i), parent(u), child(v)
    {}

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
    int f;  // TODO: set efficient f

    HighLevelNode()
      : constraints({}), f(0)
    {}
    HighLevelNode(const Plan& p, Constraints c, int _f)
      : paths(p), constraints(c), f(_f)
    {}
  };
  using HighLevelNode_p = std::shared_ptr<HighLevelNode>;
  using HighLevelNodes = std::vector<HighLevelNode_p>;


  Path getPath(const int id, const Constraints& constraints={});
  Constraints getConstraints(const Plan& paths) const;
  Constraints detectLoop
  (const Plan& paths, const Path& loop, Node* origin, const std::vector<int>& U) const;

public:
  CompletePlanning(Problem* _P);
  ~CompletePlanning();

  void setParams(int argc, char* argv[]);
  static void printHelp();
};

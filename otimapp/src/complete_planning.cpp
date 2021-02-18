#include "../include/complete_planning.hpp"


const std::string CompletePlanning::SOLVER_NAME = "CompletePlanning";


CompletePlanning::CompletePlanning(Problem* _P)
  : Solver(_P)
{
  solver_name = SOLVER_NAME;
}

CompletePlanning::~CompletePlanning()
{
}

void CompletePlanning::run()
{
  // set objective function
  auto compare = [] (HighLevelNode_p a, HighLevelNode_p b) {
    if (a->f != b->f) return a->f > b->f;  // tie-breaker
    return false;
  };

  // OPEN
  std::priority_queue<HighLevelNode_p, HighLevelNodes, decltype(compare)> Tree(compare);

  // initial node
  HighLevelNode_p n = std::make_shared<HighLevelNode>();
  for (int i = 0; i < P->getNum(); ++i) {
    n->paths.push_back(getConstrainedPath(i));
    if (n->paths[i].empty()) {
      info("  ", "failed to find a path");
      return;
    }
    n->f += n->paths[i].size()-1;
  }
  Tree.push(n);

  // start high-level search
  int h_node_num = 1;
  int iteration = 0;
  while (!Tree.empty()) {
    ++iteration;
    // check limitation
    if (overCompTime()) {
      info(" ", "timeout");
      break;
    }

    n = Tree.top();
    Tree.pop();

    info(" ", "elapsed:", getSolverElapsedTime(),
         ", explored_node_num:", iteration, ", nodes_num:", h_node_num,
         ", constraints:", n->constraints.size());

    // check conflict
    auto constraints = getConstraints(n->paths);
    if (constraints.empty()) {
      solved = true;
      break;
    }

    // create new nodes
    for (auto c : constraints) {
      auto new_constraints = n->constraints;
      new_constraints.push_back(c);
      auto paths = n->paths;
      paths[c->agent] = getConstrainedPath(c->agent, new_constraints);
      if (paths[c->agent].empty()) continue;  // failed to find path
      int f = n->f - (n->paths[c->agent].size()-1) + (paths[c->agent].size()-1);
      auto m = std::make_shared<HighLevelNode>(paths, new_constraints, f);
      Tree.push(m);
      ++h_node_num;
    }

  }

  if (solved) solution = n->paths;
}

Path CompletePlanning::getConstrainedPath(const int id, const Constraints& _constraints)
{
  Node* const g = P->getGoal(id);

  Constraints constraints;
  for (auto c : _constraints) {
    if (c->agent == id) constraints.push_back(c);
  }

  auto checkInvalidNode = [&](Node* child, Node* parent) {
    // condition 1, avoid goals
    if (child != g && table_goals[child->id]) return true;

    // condition 2, follow limitation
    for (auto c : constraints) {
      if (c->child  == child && c->parent == parent) return true;
    }

    return false;
  };

  return Solver::getPath(id, checkInvalidNode);
}

CompletePlanning::Constraints CompletePlanning::detectLoop
(const Plan& paths, const Path& loop, Node* origin, const std::vector<int>& U) const
{
  auto _U = U;

  for (int i = 0; i < P->getNum(); ++i) {
    if (inArray(i, U)) continue;

    _U.push_back(i);

    auto p = paths[i];
    for (int t = 0; t < p.size()-1; ++t) {
      if (p[t] == *(loop.end()-1)) {
        if (p[t+1] == origin) {
          // create constraints
          Constraints constraints = { std::make_shared<Constraint>(U[0], origin, loop[0]) };
          for (int j = 1; j < U.size(); ++j) {
            constraints.push_back( std::make_shared<Constraint>(U[j], loop[j-1], loop[j]) );
          }
          constraints.push_back( std::make_shared<Constraint>(i, p[t], p[t+1]) );
          return constraints;
        } else {
          Path _loop = loop;
          _loop.push_back(p[t+1]);
          auto res = detectLoop(paths, _loop, origin, _U);
          if (!res.empty()) return res;
        }
      }
    }

    _U.erase(_U.end()-1);
  }
  return {};
}

CompletePlanning::Constraints CompletePlanning::getConstraints(const Plan& paths) const
{
  for (int i = 0; i < P->getNum(); ++i) {
    auto p = paths[i];
    for (int t = 1; t < p.size(); ++t) {
      Node* v_before = p[t-1];
      Node* v_next = p[t];
      auto res = detectLoop(paths, { v_next }, v_before, { i });
      if (!res.empty()) return res;
    }
  }

  return {};
}

void CompletePlanning::setParams(int argc, char* argv[])
{

}

void CompletePlanning::printHelp()
{
  printHelpWithoutOption(SOLVER_NAME);
}

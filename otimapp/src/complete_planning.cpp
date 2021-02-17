#include "../include/complete_planning.hpp"


const std::string CompletePlanning::SOLVER_NAME = "CompletePlanning";


CompletePlanning::CompletePlanning(Problem* _P)
  : Solver(_P),
    table_goals(G->getNodesSize(), false)
{
  solver_name = SOLVER_NAME;
}

CompletePlanning::~CompletePlanning()
{
}

void CompletePlanning::run()
{
  for (int i = 0; i < P->getNum(); ++i) table_goals[P->getGoal(i)->id] = true;

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
    n->paths.push_back(getPath(i));
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
      paths[c->agent] = getPath(c->agent, new_constraints);
      if (paths[c->agent].empty()) continue;  // failed to find path
      int f = n->f - (n->paths[c->agent].size()-1) + (paths[c->agent].size()-1);
      auto m = std::make_shared<HighLevelNode>(paths, new_constraints, f);
      Tree.push(m);
      ++h_node_num;
    }

  }

  if (solved) solution = n->paths;
}

Path CompletePlanning::getPath(const int id, const Constraints& _constraints)
{
  Node* const s = P->getStart(id);
  Node* const g = P->getGoal(id);

  struct AstarNode {
    Node* v;
    int g;
    int f;
    AstarNode* p;  // parent
  };
  using AstarNodes = std::vector<AstarNode*>;

  AstarNodes GC;  // garbage collection
  auto createNewNode = [&GC](Node* v, int g, int f, AstarNode* p) {
    AstarNode* new_node = new AstarNode{ v, g, f, p };
    GC.push_back(new_node);
    return new_node;
  };

  auto compare = [&](AstarNode* a, AstarNode* b) {
    if (a->f != b->f) return a->f > b->f;
    if (a->g != b->g) return a->g < b->g;
    return false;
  };

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

    // OPEN and CLOSE list
  std::priority_queue<AstarNode*, AstarNodes, decltype(compare)> OPEN(compare);
  std::vector<bool> CLOSE(G->getNodesSize(), false);

  // initial node
  AstarNode* n = createNewNode(s, 0, pathDist(id, s), nullptr);
  OPEN.push(n);

  // main loop
  bool invalid = true;
  while (!OPEN.empty()) {
    // check time limit
    if (overCompTime()) break;

    // minimum node
    n = OPEN.top();
    OPEN.pop();

    // check CLOSE list
    if (CLOSE[n->v->id]) continue;
    CLOSE[n->v->id] = true;

    // check goal condition
    if (n->v == g) {
      invalid = false;
      break;
    }

    // expand
    Nodes C = n->v->neighbor;
    std::shuffle(C.begin(), C.end(), *MT);  // randomize
    for (auto u : C) {
      // already searched?
      if (CLOSE[u->id]) continue;
      // check constraints
      if (checkInvalidNode(u, n->v)) continue;
      int g_cost = n->g + 1;
      OPEN.push(createNewNode(u, g_cost, g_cost + pathDist(id, u), n));
    }
  }

  Path path;
  if (!invalid) {  // success
    while (n != nullptr) {
      path.push_back(n->v);
      n = n->p;
    }
    std::reverse(path.begin(), path.end());
  }

  // free
  for (auto p : GC) delete p;

  return path;
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

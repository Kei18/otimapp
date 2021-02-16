#include "../include/prioritized_planning.hpp"

const std::string PrioritizedPlanning::SOLVER_NAME = "PrioritizedPlanning";

PrioritizedPlanning::PrioritizedPlanning(Problem* _P)
  : Solver(_P),
    table_goals(G->getNodesSize(), false)
{
  solver_name = PrioritizedPlanning::SOLVER_NAME;
}

void PrioritizedPlanning::run()
{
  std::vector<int> ids(P->getNum());
  std::iota(ids.begin(), ids.end(), 0);
  std::sort(ids.begin(), ids.end(), [&](int a, int b) { return pathDist(a) < pathDist(b); });

  for (int i = 0; i < P->getNum(); ++i) table_goals[P->getGoal(i)->id] = true;

  std::vector<Path> paths(P->getNum());

  // main
  for (int j = 0; j < P->getNum(); ++j) {
    const int i = ids[j];

    info(" ", "elapsed:", getSolverElapsedTime(),
         ", agent-" + std::to_string(i), "starts planning,",
         "init-dist:", pathDist(i), ", progress:", j + 1, "/", P->getNum());
    paths[i] = getPrioritizedPath(i, paths);

    // failed
    if (paths[i].empty()) {
      info(" ", "failed to find a path");
      return;
    }
  }

  solved = true;
  solution = paths;
}

Path PrioritizedPlanning::getPrioritizedPath(const int id, const std::vector<Path>& paths)
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

  auto detectLoop = [&] (auto&& self, Node* v_next, Node* origin, std::vector<int> _U) -> bool
  {
    for (int i = 0; i < P->getNum(); ++i) {
      if (inArray(i, _U)|| paths[i].empty()) continue;
      auto p = paths[i];
      for (int t = 0; t < p.size()-1; ++t) {
        if (p[t] == v_next) {
          auto U = _U;
          U.push_back(i);
          if (p[t+1] == origin || self(self, p[t+1], origin, U)) return true;
        }
      }
    }
    return false;
  };

  // TODO: develop efficient check method
  auto checkInvalidNode = [&](Node* child, Node* parent) {
    // condition 1, avoid goals
    if (child != g && table_goals[child->id]) return true;

    // condition 2, avoid potential deadlocks
    if (detectLoop(detectLoop, child, parent, { id })) return true;

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

void PrioritizedPlanning::setParams(int argc, char* argv[])
{
}

void PrioritizedPlanning::printHelp()
{
  printHelpWithoutOption(SOLVER_NAME);
}

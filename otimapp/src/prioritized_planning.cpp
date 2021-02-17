#include "../include/prioritized_planning.hpp"

const std::string PrioritizedPlanning::SOLVER_NAME = "PrioritizedPlanning";

PrioritizedPlanning::PrioritizedPlanning(Problem* _P)
  : Solver(_P),
    table_goals(G->getNodesSize(), false),
    table_cycle_tail(G->getNodesSize()),
    table_cycle_head(G->getNodesSize())
{
  solver_name = PrioritizedPlanning::SOLVER_NAME;
}

PrioritizedPlanning::~PrioritizedPlanning()
{
  for (auto cycles : table_cycle_head) {
    for (auto c : cycles) {

      // for (auto itr = c->begin(); itr != c->end(); ++itr) std::cout << (*itr)->id << " -> ";
      // std::cout << std::endl;

      delete c;
    }
  }
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

    registerCycle(i, paths[i]);

    // failed
    if (paths[i].empty()) {
      info(" ", "failed to find a path");
      return;
    }
  }

  solved = true;
  solution = paths;
}

bool PrioritizedPlanning::detectLoopByCycle(const int id, Node* child, Node* parent)
{
  // check tail
  for (auto c : table_cycle_tail[parent->id]) {
    if (c->front() == child) return true;
  }
  // checking head is unnecessary

  return false;
}

void PrioritizedPlanning::registerCycle(const int id, const Path path)
{
  Path part_path;

  for (int t = 1; t < (int)path.size(); ++t) {
    auto v_before = path[t-1];
    auto v_next = path[t];

    part_path.push_back(v_before);

    // create new entry
    {
      // check duplication
      auto cycles = table_cycle_head[v_before->id];
      auto itr = std::find_if(cycles.begin(), cycles.end(),
                              [&v_next] (Cycle* c) { return c->back() == v_next; });
      if (itr == cycles.end()) {
        auto c = new Cycle;
        c->push_back(v_before);
        c->push_back(v_next);
        // register
        table_cycle_tail[c->back()->id].push_back(c);
        table_cycle_head[c->front()->id].push_back(c);
      }
    }

    // check tail
    if (!table_cycle_tail[v_before->id].empty()) {
      for (auto c : table_cycle_tail[v_before->id]) {

        // avoid duplication
        {
          auto cycles = table_cycle_head[c->front()->id];
          auto itr = std::find_if(cycles.begin(), cycles.end(),
                                  [&v_next] (Cycle* _c) { return _c->back() == v_next; });
          if (itr != cycles.end()) continue;
        }

        // avoid itself
        {
          bool use_itself = false;
          for (auto itr = c->begin(); itr != c->end() - 1; ++itr) {
            if (inArray(*itr, part_path)) {
              use_itself = true;
              break;
            }
          }
          if (use_itself) continue;
        }

        // create new path
        auto c_new = new Cycle;
        for (auto itr = c->begin(); itr != c->end(); ++itr) c_new->push_back(*itr);
        c_new->push_back(v_next);

        // check loop
        if (c_new->front() == c_new->back()) halt("detect potential deadlock");

        // register
        table_cycle_tail[c_new->back()->id].push_back(c_new);
        table_cycle_head[c_new->front()->id].push_back(c_new);
      }
    }

    // check head
    if (!table_cycle_head[v_next->id].empty()) {
      for (auto c : table_cycle_head[v_next->id]) {

        // avoid duplication
        {
          auto cycles = table_cycle_tail[c->back()->id];
          auto itr = std::find_if(cycles.begin(), cycles.end(),
                                  [&v_before] (Cycle* _c) { return _c->front() == v_before; });
          if (itr != cycles.end()) continue;
        }

        // avoid itself
        {
          bool use_itself = false;
          for (auto itr = c->begin() + 1; itr != c->end(); ++itr) {
            if (inArray(*itr, part_path)) {
              use_itself = true;
              break;
            }
          }
          if (use_itself) continue;
        }

        // create new path
        auto c_new = new Cycle;
        for (auto itr = c->begin(); itr != c->end(); ++itr) c_new->push_back(*itr);
        c_new->push_front(v_before);

        // check loop
        if (c_new->front() == c_new->back()) halt("detect potential deadlock");

        // register
        table_cycle_tail[c_new->back()->id].push_back(c_new);
        table_cycle_head[c_new->front()->id].push_back(c_new);
      }
    }
  }
}

Path PrioritizedPlanning::getPrioritizedPath(const int id, const Plan& paths)
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

  int elapsed = 0;

  // TODO: develop efficient check method
  auto checkInvalidNode = [&](Node* child, Node* parent) {
    // condition 1, avoid goals
    if (child != g && table_goals[child->id]) return true;

    // condition 2, avoid potential deadlocks
    // if (naiveDetectLoop(id, child, parent, paths)) return true;
    if (detectLoopByCycle(id, child, parent)) return true;

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

  // std::cout << elapsed << std::endl;

  return path;
}

bool PrioritizedPlanning::naiveDetectLoop
(const int id, Node* child, Node* parent, const Plan& paths) const
{
  auto detectLoop = [&] (auto&& self, Node* v_next, Node* origin, std::vector<int> _U) -> bool
  {
    for (int i = 0; i < P->getNum(); ++i) {
      if (paths[i].empty() || inArray(i, _U)) continue;
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

  return detectLoop(detectLoop, child, parent, { id });
}

void PrioritizedPlanning::setParams(int argc, char* argv[])
{
}

void PrioritizedPlanning::printHelp()
{
  printHelpWithoutOption(SOLVER_NAME);
}

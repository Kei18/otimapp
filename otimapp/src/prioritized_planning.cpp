#include "../include/prioritized_planning.hpp"

const std::string PrioritizedPlanning::SOLVER_NAME = "PrioritizedPlanning";

PrioritizedPlanning::PrioritizedPlanning(Problem* _P)
  : Solver(_P),
    table_cycle_tail(G->getNodesSize()),
    table_cycle_head(G->getNodesSize())
{
  solver_name = SOLVER_NAME;
}

PrioritizedPlanning::~PrioritizedPlanning()
{
  for (auto cycles : table_cycle_head) {
    for (auto c : cycles) delete c;
  }
}

void PrioritizedPlanning::run()
{
  // shorter path is prioritized
  std::vector<int> ids(P->getNum());
  std::iota(ids.begin(), ids.end(), 0);
  std::sort(ids.begin(), ids.end(), [&](int a, int b) { return pathDist(a) < pathDist(b); });

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

Path PrioritizedPlanning::getPrioritizedPath(const int id, const Plan& paths)
{
  Node* const g = P->getGoal(id);

  auto checkInvalidNode = [&](Node* child, Node* parent) {
    // condition 1, avoid goals
    if (child != g && table_goals[child->id]) return true;

    // condition 2, avoid potential deadlocks
    if (detectLoopByCycle(id, child, parent)) return true;

    return false;
  };

  return getPath(id, checkInvalidNode);
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
  Path path_until_t_minus_2;  // path[0] ... path[t-2]

  // check duplication
  auto existDuplication = [&] (Node* head, Node* tail)
  {
    auto cycles = table_cycle_head[head->id];
    return
      std::find_if(cycles.begin(), cycles.end(),
                   [&tail] (CycleCandidate* c)
                   { return c->back() == tail; }) != cycles.end();
  };

  // create new entry
  auto createNewCycleCandidate = [&] (Node* head, CycleCandidate* c_base, Node* tail)
  {
    // create new one
    auto c = new CycleCandidate;
    if ((c_base == nullptr || head != c_base->front()) && head != nullptr) c->push_back(head);
    if (c_base != nullptr) {
      for (auto itr = c_base->begin(); itr != c_base->end(); ++itr) c->push_back(*itr);
    }
    if ((c_base == nullptr || tail != c_base->back()) && tail != nullptr) c->push_back(tail);

    // check loop
    if (c->front() == c->back()) halt("detect potential deadlock");

    // register
    table_cycle_head[c->front()->id].push_back(c);
    table_cycle_tail[c->back()->id].push_back(c);
  };

  // avoid loop with own path
  auto usingOwnPath = [&] (CycleCandidate* c)
  {
    for (auto itr = c->begin(); itr != c->end(); ++itr) {
      if (inArray(*itr, path_until_t_minus_2)) {
        return true;
      }
    }
    return false;
  };

  // update cycles step by step
  for (int t = 1; t < (int)path.size(); ++t) {
    auto v_before = path[t-1];
    auto v_next = path[t];

    // update part of path
    if (t >= 2) path_until_t_minus_2.push_back(path[t-2]);

    if (!existDuplication(v_before, v_next)) {
      createNewCycleCandidate(v_before, nullptr, v_next);
    }

    // check existing cycle, tail
    if (!table_cycle_tail[v_before->id].empty()) {
      for (auto c : table_cycle_tail[v_before->id]) {
        if (existDuplication(c->front(), v_next)) continue;
        if (usingOwnPath(c)) continue;
        createNewCycleCandidate(nullptr, c, v_next);
      }
    }

    // check existing cycle, head
    if (!table_cycle_head[v_next->id].empty()) {
      for (auto c : table_cycle_head[v_next->id]) {
        if (existDuplication(v_before, c->back())) continue;
        if (usingOwnPath(c)) continue;
        createNewCycleCandidate(v_before, c, nullptr);
      }
    }
  }
}

void PrioritizedPlanning::setParams(int argc, char* argv[])
{
}

void PrioritizedPlanning::printHelp()
{
  printHelpWithoutOption(SOLVER_NAME);
}

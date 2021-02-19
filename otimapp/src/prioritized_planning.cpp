#include "../include/prioritized_planning.hpp"

const std::string PrioritizedPlanning::SOLVER_NAME = "PrioritizedPlanning";

PrioritizedPlanning::PrioritizedPlanning(Problem* _P)
  : Solver(_P)
{
  solver_name = SOLVER_NAME;
}

PrioritizedPlanning::~PrioritizedPlanning()
{
}

void PrioritizedPlanning::run()
{
  // id_list
  std::vector<int> id_list(P->getNum());
  std::iota(id_list.begin(), id_list.end(), 0);

  int itr_cnt = 0;
  while (!solved && !overCompTime()) {
    ++itr_cnt;

    // randomize order
    std::shuffle(id_list.begin(), id_list.end(), *MT);

    // initialize
    solution.clear();
    solution.resize(P->getNum());

    info(" ", "iter-" + std::to_string(itr_cnt), "start");

    // main
    bool invalid = false;
    TableCycle table_cycle(G->getNodesSize());
    for (int j = 0; j < P->getNum(); ++j) {
      const int i = id_list[j];

      info(" ", "elapsed:", getSolverElapsedTime(),
           ", iter:", itr_cnt,
           ", agent-" + std::to_string(i), "starts planning,",
           "init-dist:", pathDist(i), ", progress:", j + 1, "/", P->getNum());
      solution[i] = getPrioritizedPath(i, solution, table_cycle);

      // failed
      if (solution[i].empty() || overCompTime()) {
        invalid = true;
        break;
      }

      // register new
      auto c = table_cycle.registerNewPath(i, solution[i], G->getNodesSize());
      if (c != nullptr) halt("detect deadlock");
    }
    solved = !invalid;
  }
}

Path PrioritizedPlanning::getPrioritizedPath(const int id, const Plan& paths, TableCycle& table)
{
  Node* const g = P->getGoal(id);

  auto checkInvalidNode = [&](Node* child, Node* parent) {
    // condition 1, avoid goals
    if (child != g && table_goals[child->id]) return true;

    // condition 2, avoid potential deadlocks
    for (auto c : table.t_tail[parent->id]) {
      if (c->path.front() == child) return true;
    }

    return false;
  };

  return getPath(id, checkInvalidNode);
}

void PrioritizedPlanning::setParams(int argc, char* argv[])
{
}

void PrioritizedPlanning::printHelp()
{
  printHelpWithoutOption(SOLVER_NAME);
}

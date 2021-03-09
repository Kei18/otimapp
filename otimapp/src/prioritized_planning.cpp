#include "../include/prioritized_planning.hpp"

const std::string PrioritizedPlanning::SOLVER_NAME = "PrioritizedPlanning";

PrioritizedPlanning::PrioritizedPlanning(Problem* _P)
  : Solver(_P),
    iter_cnt_max(DEFAULT_ITER_CNT_MAX),
    max_fragment_size(DEFAULT_MAX_FRAGMENT_SIZE)
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
  while (!solved && !overCompTime() && itr_cnt < iter_cnt_max) {
    ++itr_cnt;

    // randomize order
    std::shuffle(id_list.begin(), id_list.end(), *MT);

    // initialize
    solution.clear();
    solution.resize(P->getNum());

    info(" ", "iter-" + std::to_string(itr_cnt), "start");

    // main
    bool invalid = false;
    TableCycle table_cycle(G, max_fragment_size);
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
      auto c = table_cycle.registerNewPath(i, solution[i]);
      if (c != nullptr) halt("detect deadlock");
    }
    solved = !invalid;
  }
}

void PrioritizedPlanning::setParams(int argc, char* argv[])
{
  struct option longopts[] = {
    {"iter-cnt-max", required_argument, 0, 'm'},
    {"max-fragment-size", required_argument, 0, 'f'},
    {0, 0, 0, 0},
  };
  optind = 1;  // reset
  int opt, longindex;
  while ((opt = getopt_long(argc, argv, "m:f:", longopts, &longindex)) != -1) {
    switch (opt) {
    case 'm':
      iter_cnt_max = std::atoi(optarg);
      break;
    case 'f':
      max_fragment_size = std::atoi(optarg);
      break;
    default:
      break;
    }
  }
}

void PrioritizedPlanning::printHelp()
{
  std::cout << SOLVER_NAME << "\n"

            << "  -m --iter-cnt-max"
            << "             "
            << "maximum iterations for randomizing priorities"

            << "\n"

            << "  -f --max-fragment-size"
            << "        "
            << "maximum fragment size except for potential deadlocks"

            << std::endl;
}

#include "../include/pp.hpp"

const std::string PP::SOLVER_NAME = "PP";

PP::PP(Problem* _P) : Solver(_P), iter_cnt_max(DEFAULT_ITER_CNT_MAX)
{
  solver_name = SOLVER_NAME;
}

PP::~PP() {}

void PP::run()
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
    auto table = new TableFragment(G);
    for (int j = 0; j < P->getNum(); ++j) {
      const int i = id_list[j];

      info(" ", "elapsed:", getSolverElapsedTime(), ", iter:", itr_cnt,
           ", agent-" + std::to_string(i), "starts planning,",
           "init-dist:", pathDist(i), ", progress:", j + 1, "/", P->getNum());

      // get prioritized path
      auto t_p = Time::now();
      solution[i] = getPrioritizedPath(i, solution, *table);
      elapsed_time_pathfinding += getElapsedTime(t_p);

      // failed
      if (solution[i].empty() || overCompTime()) {
        invalid = true;
        break;
      }

      // register new path
      auto t_d = Time::now();
      auto c = table->registerNewPath(i, solution[i], false, getRemainedTime());
      elapsed_time_deadlock_detection += getElapsedTime(t_d);
      if (c != nullptr) halt("detect deadlock");
    }
    solved = !invalid;

    auto t_d = Time::now();
    delete table;
    elapsed_time_deadlock_detection += getElapsedTime(t_d);
  }
}

void PP::setParams(int argc, char* argv[])
{
  struct option longopts[] = {
      {"iter-cnt-max", required_argument, 0, 'm'},
      {0, 0, 0, 0},
  };
  optind = 1;  // reset
  int opt, longindex;
  while ((opt = getopt_long(argc, argv, "m:", longopts, &longindex)) != -1) {
    switch (opt) {
      case 'm':
        iter_cnt_max = std::atoi(optarg);
        break;
      default:
        break;
    }
  }
}

void PP::printHelp()
{
  std::cout << SOLVER_NAME << "\n"

            << "  -m --iter-cnt-max"
            << "             "
            << "maximum iterations for randomizing priorities"

            << std::endl;
}

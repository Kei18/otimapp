#include <getopt.h>

#include <dbs.hpp>
#include <default_params.hpp>
#include <fstream>
#include <iostream>
#include <pp.hpp>
#include <problem.hpp>
#include <random>
#include <regex>
#include <vector>

void printHelp();
std::unique_ptr<Solver> getSolver(const std::string solver_name, Problem* P,
                                  bool verbose, int argc, char* argv[]);

int main(int argc, char* argv[])
{
  std::string output_file = DEFAULT_PLAN_OUTPUT_FILE;
  std::string solver_name;
  bool verbose = false;
  char* argv_copy[argc + 1];
  for (int i = 0; i < argc; ++i) argv_copy[i] = argv[i];

  struct option longopts[] = {
    {"output", required_argument, 0, 'o'},
    {"solver", required_argument, 0, 's'},
    {"verbose", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {"time-limit", required_argument, 0, 'T'},
    {"vertex", required_argument, 0, 'n'},
    {"prob", required_argument, 0, 'p'},
    {"agent", required_argument, 0, 'k'},
    {"seed", required_argument, 0, 'r'},
    {0, 0, 0, 0},
  };
  int max_comp_time = -1;

  int n = 0;  // #vertex
  float p = 0;  // prob
  int k = 0;  // #agent
  int seed = 0;  // seed

  // command line args
  int opt, longindex;
  opterr = 0;  // ignore getopt error
  while ((opt = getopt_long(argc, argv, "o:s:vhT:n:p:k:r:", longopts, &longindex)) != -1) {
    switch (opt) {
      case 'o':
        output_file = std::string(optarg);
        break;
      case 's':
        solver_name = std::string(optarg);
        break;
      case 'v':
        verbose = true;
        break;
      case 'h':
        printHelp();
        return 0;
      case 'T':
        max_comp_time = std::atoi(optarg);
        break;
    case 'n':
      n = std::atoi(optarg);
      break;
    case 'p':
      p = std::atof(optarg);
      break;
    case 'k':
      k = std::atoi(optarg);
      break;
    case 'r':
      seed = std::atoi(optarg);
      break;
    default:
      break;
    }
  }

  // set problem
  if (n == 0 || p == 0 || k == 0) {
    std::cout << "specify n (vertex), p (prob), k (agent), and r (seed, optionally), e.g.,"
              << std::endl;
    std::cout << "> ./app -n 1000 -p 0.01 -k 20 -r 0" << std::endl;
    return 0;
  }
  Problem P = Problem(n, p, k, seed);

  // set max computation time (otherwise, use param in instance_file)
  if (max_comp_time != -1) P.setMaxCompTime(max_comp_time);

  // solve
  auto solver = getSolver(solver_name, &P, verbose, argc, argv_copy);
  solver->solve();
  solver->printResult();
  solver->makeLog(output_file);

  if (verbose) {
    std::cout << "save planning result as " << output_file << std::endl;
  }

  return 0;
}

std::unique_ptr<Solver> getSolver(const std::string solver_name, Problem* P,
                                  bool verbose, int argc, char* argv[])
{
  std::unique_ptr<Solver> solver;
  if (solver_name == "PP") {
    solver = std::make_unique<PP>(P);
  } else if (solver_name == "DBS") {
    solver = std::make_unique<DBS>(P);
  } else {
    std::cout << "warn@app: "
              << "unknown solver name, " + solver_name +
                     ", continue by PP"
              << std::endl;
    solver = std::make_unique<PP>(P);
  }

  solver->setParams(argc, argv);
  solver->setVerbose(verbose);
  return solver;
}

void printHelp()
{
  std::cout << "\nUsage: ./app [OPTIONS] [SOLVER-OPTIONS]\n"
            << "\n**instance file is necessary to run MAPF simulator**\n\n"
            << "  -n --vertex [INT]             random graph G(n, p)\n"
            << "  -p --prob [FLOAT]             random graph G(n, p)\n"
            << "  -k --agent [INT]              number of agents\n"
            << "  -r --seed [INT]               seed\n"
            << "  -o --output [FILE_PATH]       ouptut file path\n"
            << "  -v --verbose                  print additional info\n"
            << "  -h --help                     help\n"
            << "  -s --solver [SOLVER_NAME]     solver, choose from the below\n"
            << "  -T --time-limit [INT]         max computation time (ms)\n"
            << "\n\nSolver Options:" << std::endl;
  // each solver
  PP::printHelp();
  DBS::printHelp();
}

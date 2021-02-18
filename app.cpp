#include <getopt.h>
#include <default_params.hpp>
#include <iostream>
#include <problem.hpp>
#include <prioritized_planning.hpp>
#include <complete_planning.hpp>
#include <fstream>
#include <random>
#include <vector>
#include <regex>

void printHelp();
std::unique_ptr<Solver> getSolver(const std::string solver_name, Problem* P,
                                  bool verbose, int argc, char* argv[]);


int main(int argc, char* argv[])
{
  std::string instance_file = "";
  std::string output_file = DEFAULT_PLAN_OUTPUT_FILE;
  std::string solver_name;
  bool verbose = false;
  char* argv_copy[argc + 1];
  for (int i = 0; i < argc; ++i) argv_copy[i] = argv[i];

  struct option longopts[] = {
      {"instance", required_argument, 0, 'i'},
      {"output", required_argument, 0, 'o'},
      {"solver", required_argument, 0, 's'},
      {"verbose", no_argument, 0, 'v'},
      {"help", no_argument, 0, 'h'},
      {"time-limit", required_argument, 0, 'T'},
      {"make-scen", no_argument, 0, 'P'},
      {0, 0, 0, 0},
  };
  bool make_scen = false;
  int max_comp_time = -1;

  // command line args
  int opt, longindex;
  opterr = 0;  // ignore getopt error
  while ((opt = getopt_long(argc, argv, "i:o:s:vhPT:", longopts, &longindex)) !=
         -1) {
    switch (opt) {
      case 'i':
        instance_file = std::string(optarg);
        break;
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
      case 'P':
        make_scen = true;
        break;
      case 'T':
        max_comp_time = std::atoi(optarg);
        break;
      default:
        break;
    }
  }

  if (instance_file.length() == 0) {
    std::cout << "specify instance file using -i [INSTANCE-FILE], e.g.,"
              << std::endl;
    std::cout << "> ./app -i ../instance/sample.txt" << std::endl;
    return 0;
  }

  // set problem
  Problem P = Problem(instance_file);

  // set max computation time (otherwise, use param in instance_file)
  if (max_comp_time != -1) P.setMaxCompTime(max_comp_time);

  // create scenario
  if (make_scen) {
    P.makeScenFile(output_file);
    return 0;
  }

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
  if (solver_name == "PrioritizedPlanning") {
    solver = std::make_unique<PrioritizedPlanning>(P);
  } else if (solver_name == "CompletePlanning") {
    solver = std::make_unique<CompletePlanning>(P);
  } else {
    std::cout << "warn@app: "
              << "unknown solver name, " + solver_name + ", continue by PrioritizedPlanning"
              << std::endl;
    solver = std::make_unique<PrioritizedPlanning>(P);
  }

  solver->setParams(argc, argv);
  solver->setVerbose(verbose);
  return solver;
}

void printHelp()
{
  std::cout << "\nUsage: ./app [OPTIONS] [SOLVER-OPTIONS]\n"
            << "\n**instance file is necessary to run MAPF simulator**\n\n"
            << "  -i --instance [FILE_PATH]     instance file path\n"
            << "  -o --output [FILE_PATH]       ouptut file path\n"
            << "  -v --verbose                  print additional info\n"
            << "  -h --help                     help\n"
            << "  -s --solver [SOLVER_NAME]     solver, choose from the below\n"
            << "  -T --time-limit [INT]         max computation time (ms)\n"
            << "  -P --make-scen                make scenario file using "
               "random starts/goals"
            << "\n\nSolver Options:" << std::endl;
  // each solver
  PrioritizedPlanning::printHelp();
  CompletePlanning::printHelp();
}

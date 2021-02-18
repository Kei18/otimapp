#include <getopt.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <problem.hpp>
#include <execution.hpp>

void printHelp();


int main(int argc, char* argv[])
{
  std::string instance_file = "";
  std::string plan_file = "";
  std::string output_file = DEFAULT_EXEC_OUTPUT_FILE;
  int seed = DEFAULT_SEED;
  float ub_delay_prob = DEFAULT_UB_DELAY_PROB;
  bool verbose = false;

  struct option longopts[] = {
      {"instance", required_argument, 0, 'i'},
      {"plan", required_argument, 0, 'p'},
      {"output", required_argument, 0, 'o'},
      {"seed", required_argument, 0, 's'},
      {"ub-delay-prob", required_argument, 0, 'u'},
      {"verbose", no_argument, 0, 'v'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0},
  };

  // command line args
  int opt, longindex;
  opterr = 0;  // ignore getopt error
  while ((opt = getopt_long(argc, argv, "i:p:o:s:u:vh", longopts, &longindex)) != -1) {
    switch (opt) {
      case 'i':
        instance_file = std::string(optarg);
        break;
      case 'p':
        plan_file = std::string(optarg);
        break;
      case 'o':
        output_file = std::string(optarg);
        break;
      case 's':
        seed = std::stoi(optarg);
        break;
      case 'u':
        ub_delay_prob = std::stof(optarg);
        break;
      case 'v':
        verbose = true;
        break;
      case 'h':
        printHelp();
        return 0;
      default:
        break;
    }
  }

  if (instance_file.length() == 0 || plan_file.length() == 0) {
    std::cout << "specify instance and plan file using -i and -p, e.g.,"
              << std::endl;
    std::cout << "> ./app -i ../instance/sample.txt -p ./plan.txt" << std::endl;
    return 0;
  }

  // read original problem
  Problem P = Problem(instance_file);

  // emulate execution
  auto exec = Execution(&P, plan_file, seed, ub_delay_prob, verbose);
  exec.run();
  exec.printResult();
  exec.makeLog(output_file);
  if (verbose) {
    std::cout << "save execution result as " << output_file << std::endl;
  }

  return 0;
}

void printHelp()
{
  std::cout << "\nUsage: ./exec [OPTIONS]\n"
            << "\n**instance and planning file is necessary to run execution simulator**\n\n"
            << "  -i --instance [FILE_PATH]     instance file path\n"
            << "  -p --plan [FILE_PATH]         plan file path\n"
            << "  -o --output [FILE_PATH]       ouptut file path\n"
            << "  -s --seed                     seed\n"
            << "  -u --ub-delay-prob            upper bound of delay probabilities\n"
            << "  -v --verbose                  print additional info\n"
            << "  -h --help                     help"
            << std::endl;
}

#include <getopt.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <problem.hpp>
#include <execution.hpp>

void printHelp();
float getUpperBoundDelayProb(const std::string& instance);


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
      {"ub_delay_prob", required_argument, 0, 'u'},
      {"verbose", no_argument, 0, 'v'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0},
  };

  // command line args
  int opt, longindex;
  opterr = 0;  // ignore getopt error
  while ((opt = getopt_long(argc, argv, "i:p:o:s:VHF:", longopts, &longindex)) != -1) {
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

std::tuple<bool, Plan> getPlan(const std::string& filename, Problem* P)
{
  std::ifstream file(filename);
  if (!file) {
    std::cout << "error@app: " << filename << " cannot be opened" << std::endl;
    std::exit(1);
  }

  bool solved = false;
  Plan plan;  // solution

  std::regex r_solved = std::regex(R"(solved=(\d))");
  std::regex r_plan = std::regex(R"(plan=)");
  std::regex r_path = std::regex(R"(\d+:(.+))");
  std::regex r_pos = std::regex(R"((\d+),)");

  std::string line;
  std::smatch results;
  while (getline(file, line)) {
    if (std::regex_match(line, results, r_solved)) solved = (bool)std::stoi(results[1].str());
    if (std::regex_match(line, results, r_plan)) {
      while (getline(file, line)) {
        if (std::regex_match(line, results, r_path)) {
          auto s = results[1].str();
          Path path;
          auto iter = s.cbegin();
          while (std::regex_search(iter, s.cend(), results, r_pos)) {
            iter = results[0].second;
            auto i = std::stoi(results[1].str());
            path.push_back(P->getG()->getNode(i));
          }
          plan.push_back(path);
        }
      }
    }
  }

  return std::make_tuple(solved, plan);
}

void printHelp()
{
}

#include "ofMain.h"
#include "../include/ofApp.hpp"
#include <iostream>
#include <fstream>
#include <regex>
#include "../include/result.hpp"

void readSetResult(const std::string& result_file, Result* result);
void readSetNode(const std::string& s, Config& config, Grid* G);


int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Put your result file as the first arg." << std::endl;
    return 0;
  }

  Result* res = new Result;  // deleted in ofApp destructor
  readSetResult(argv[1], res);
  ofSetupOpenGL(100, 100, OF_WINDOW);
  ofRunApp(new ofApp(res));
  return 0;
}

void readSetNode(const std::string& s, Config& config, Grid* G)
{
  if (G == nullptr) {
    std::cout << "error@main, no graph" << std::endl;
    std::exit(1);
  }
  std::regex r_pos = std::regex(R"(\((\d+),(\d+)\),)");
  std::smatch m;
  auto iter = s.cbegin();
  while (std::regex_search(iter, s.cend(), m, r_pos)) {
    iter = m[0].second;
    int x = std::stoi(m[1].str());
    int y = std::stoi(m[2].str());
    if (!G->existNode(x, y)) {
      std::cout << "error@main, node does not exist" << std::endl;
      delete G;
      std::exit(1);
    }
    config.push_back(G->getNode(x, y));
  }
}

void readSetResult(const std::string& result_file, Result* res)
{
  std::ifstream file(result_file);
  if (!file) {
    std::cout << "error@main," << "file " << result_file << " is not found." << std::endl;
    std::exit(1);
  };

  std::regex r_map       = std::regex(R"(map_file=(.+))");
  std::regex r_agents    = std::regex(R"(agents=(.+))");
  std::regex r_solver    = std::regex(R"(solver=(.+))");
  std::regex r_solved    = std::regex(R"(solved=(\d))");
  std::regex r_soc       = std::regex(R"(soc=(\d+))");
  std::regex r_makespan  = std::regex(R"(makespan=(\d+))");
  std::regex r_comp_time = std::regex(R"(comp_time=(\d+))");
  std::regex r_starts    = std::regex(R"(starts=(.+))");
  std::regex r_goals     = std::regex(R"(goals=(.+))");
  std::regex r_result    = std::regex(R"(result=)");
  std::regex r_act_cnts  = std::regex(R"(activate_cnts=(\d+))");
  std::regex r_execution = std::regex(R"(execution.+=)");
  std::regex r_exec_row  = std::regex(R"((\d+):\((\d+),(\d+),(\d+),(.+),(\d+)\))");
  std::regex r_config    = std::regex(R"(\d+:(.+))");

  std::string line;
  std::smatch results;
  while (getline(file, line)) {
    // read map
    if (std::regex_match(line, results, r_map)) {
      res->G = new Grid(results[1].str());  // deleted in destructor of MAPFPlan
      continue;
    }
    // set agent num
    if (std::regex_match(line, results, r_agents)) {
      res->num_agents = std::stoi(results[1].str());
      continue;
    }
    // solver
    if (std::regex_match(line, results, r_solver)) {
      res->solver = results[1].str();
      continue;
    }
    // solved?
    if (std::regex_match(line, results, r_solved)) {
      res->solved = (bool)std::stoi(results[1].str());
      continue;
    }
    // soc
    if (std::regex_match(line, results, r_soc)) {
      res->soc = std::stoi(results[1].str());
      continue;
    }
    // makespan
    if (std::regex_match(line, results, r_makespan)) {
      res->makespan = std::stoi(results[1].str());
      continue;
    }
    // comp_time
    if (std::regex_match(line, results, r_comp_time)) {
      res->comp_time = std::stoi(results[1].str());
      continue;
    }
    // starts
    if (std::regex_match(line, results, r_starts)) {
      readSetNode(results[1].str(), res->config_s, res->G);
      continue;
    }
    // goals
    if (std::regex_match(line, results, r_goals)) {
      readSetNode(results[1].str(), res->config_g, res->G);
      continue;
    }
    // activation counts
    if (std::regex_match(line, results, r_act_cnts)) {
      res->activated_cnt = std::stoi(results[1].str());
      continue;
    }
    // result
    if (std::regex_match(line, results, r_result)) {
      while (getline(file, line)) {
        if (std::regex_match(line, results, r_config)) {
          Config c;
          readSetNode(results[1].str(), c, res->G);
          res->transitions.push_back(c);
        } else {
          break;
        }
      }
    }
    // execution
    if (std::regex_match(line, results, r_execution)) {
      while (getline(file, line)) {
        if (std::regex_match(line, results, r_exec_row)) {
          res->exec.push_back
            (std::make_tuple(std::stoi(results[2]),
                             std::stoi(results[3]),
                             std::stoi(results[4]),
                             std::stoi(results[5]),
                             std::stoi(results[6])));
        }
      }
    }

  }
}

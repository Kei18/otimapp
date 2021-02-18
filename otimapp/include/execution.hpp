#pragma once

#include <memory>
#include <fstream>
#include "problem.hpp"
#include "agent.hpp"
#include "lib_execution.hpp"

class Execution {
private:
  Problem* const P;              // problem instance
  const std::string plan_file;   // file of planning result
  const bool solved;             // check validity of the plan
  const Plan plan;               // planning
  const int seed;                // seed
  std::mt19937* MT;              // seed
  const float ub_delay_prob;     // upper bound of delay probabilities
  const bool verbose;            // print info or not

  Configs exec_result;             // execution result
  std::vector<int> occupancy;      // occupancy[node_id] = agent_id or Agent::NIL
  std::vector<float> delay_probs;  // array of delay probabilities
  std::vector<Agent::State> HIST;  // execution history
  int emulation_time;              // time required for emulation

  // -------------------------------
  // utilities for debug
  void info(const std::string& msg) const;
  void warn(const std::string& msg) const;
  void halt(const std::string& msg) const;

  // -------------------------------
  // read planning
  Plan getPlan() const;
  bool getSolved() const;

public:
  Execution(Problem* _P,
            std::string _plan_file,
            int _seed = DEFAULT_SEED,
            float _ub_delay_prob = DEFAULT_UB_DELAY_PROB,
            bool _verbose = false);
  ~Execution();

  // -------------------------------
  // main
  void run();

  // -------------------------------
  // getter
  Plan getExecResult() const { return exec_result; }

  // -------------------------------
  // others
  void printResult() const;
  void makeLog(const std::string& logfile = DEFAULT_EXEC_OUTPUT_FILE) const;
};

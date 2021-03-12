#pragma once

#include <memory>
#include <fstream>
#include "problem.hpp"
#include "agent.hpp"
#include "lib_execution.hpp"

class Execution {
protected:
  std::string problem_name;      // problem name
  Problem* const P;              // problem instance
  const std::string plan_file;   // file of planning result
  bool solved;                   // check validity of the plan
  Plan plan;                     // planning
  bool exec_succeed;             // whether to succeed the execution
  const int seed;                // seed
  std::mt19937* MT;              // seed
  const bool verbose;            // print info or not
  const bool log_short;          // make log short

  Configs exec_result;             // execution result
  std::vector<MAPF_DP_Agent::State> HIST;  // execution history
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

  // -------------------------------
  // main
  virtual void simulate() {}

  // -------------------------------
  // log
  virtual void makeLogSpecific(std::ofstream& log) const {};

public:
  Execution(Problem* _P,
            std::string _plan_file,
            int _seed = DEFAULT_SEED,
            bool _verbose = false,
            bool _log_short = false);
  virtual ~Execution();

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

// MAPF_DP
class MAPF_DP_Execution : public Execution
{
private:
  static const std::string PROBLEM_NAME;

  const float ub_delay_prob;     // upper bound of delay probabilities
  std::vector<float> delay_probs;  // array of delay probabilities

  void makeLogSpecific(std::ofstream& log) const;

  // -------------------------------
  // main
  void simulate();

public:
  MAPF_DP_Execution(Problem* _P,
                    std::string _plan_file,
                    int _seed = DEFAULT_SEED,
                    float _ub_delay_prob = DEFAULT_UB_DELAY_PROB,
                    bool _verbose = false,
                    bool _log_short = false);
  ~MAPF_DP_Execution() {}
};

// Primitive Execution
class PrimitiveExecution : public Execution
{
private:
  static const std::string PROBLEM_NAME;

  // -------------------------------
  // main
  void simulate();

public:
  PrimitiveExecution(Problem* _P,
                     std::string _plan_file,
                     int _seed = DEFAULT_SEED,
                     bool _verbose = false,
                     bool _log_short = false);
  ~PrimitiveExecution() {}
};

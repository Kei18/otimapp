#pragma once
#include "../../third_party/grid_pathfinding/graph/include/graph.hpp"

using Config = std::vector<Node*>;
using Configs = std::vector<Config>;

// agent, internal_t, mode, head, tail
using State = std::tuple<int, int, int, int, int>;
using States = std::vector<State>;

struct Result {
  int num_agents;         // number of agents
  Grid* G;                // grid
  std::string solver;     // solver name
  bool solved;            // success or not
  int soc;                // sum of cost
  int makespan;           // makespan
  int comp_time;          // computation time
  Config config_s;        // start configuration
  Config config_g;        // goal configuration
  Configs transitions;    // plan
  int activated_cnt;      // activation counts
  float ub_delay_prob;    // upper bound of delay probabilities
  std::vector<float> delay_probs;  // delay probabilities
  States exec;            // execution

  ~Result() { delete G; }
};

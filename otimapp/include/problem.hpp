#pragma once
#include <graph.hpp>
#include <random>

#include "default_params.hpp"
#include "util.hpp"

using Config = std::vector<Node*>;  // < loc_0[t], loc_1[t], ... >
using Configs = std::vector<Config>;

using Plan = std::vector<Path>;

class Problem
{
private:
  std::string instance;  // instance name
  Graph* G;              // graph
  int seed;              // seed
  std::mt19937* MT;      // randomness
  Config config_s;       // initial configuration
  Config config_g;       // goal configuration
  int num_agents;        // number of agents
  int max_comp_time;     // comp_time limit, ms

  const bool is_random_graph;

  // set starts and goals randomly
  void setRandomStartsGoals();
  void setGoalAvoidanceInstance();

  // utilities
  void halt(const std::string& msg) const;
  void warn(const std::string& msg) const;

public:
  Problem(const std::string& _instance);
  Problem(int _nodes_size, float _prob, int _num_agents, int _seed);
  ~Problem();

  Graph* getG() { return G; }
  int getNum() { return num_agents; }
  std::mt19937* getMT() { return MT; }
  Node* getStart(int i) const;  // return start of a_i
  Node* getGoal(int i) const;   // return  goal of a_i
  Config getConfigStart() const { return config_s; };
  Config getConfigGoal() const { return config_g; };
  int getMaxCompTime() const { return max_comp_time; };
  std::string getInstanceFileName() const { return instance; };
  int getSeed() const { return seed; };
  bool isRandomGraph() const { return is_random_graph; }

  void setMaxCompTime(const int t) { max_comp_time = t; }

  // used when making new instance file
  void makeScenFile(const std::string& output_file);
};

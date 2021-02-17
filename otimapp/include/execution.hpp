#pragma once

#include <memory>
#include <fstream>
#include "problem.hpp"

class Execution {
private:
  static constexpr int NIL = -1;

  enum struct Mode { CONTRACTED, EXTENDED };
  using State = std::tuple<int, int, Mode, Node*, Node*>;  // id, t, mode, head, tail
  struct Agent {
    int id;
    int t;
    Mode mode;
    Node* head;
    Node* tail;
    Path path;

    Agent(int _id, const Path& _path);
    ~Agent() {}

    Node* getNextNode() const;
    bool isFinished() const;
    State getState() const;
  };
  using Agent_p = std::shared_ptr<Agent>;
  using Agents = std::vector<Agent_p>;

  Problem* P;
  std::mt19937* MT;
  const Plan solution;
  Configs result;
  const float ub_delay_prob;
  std::vector<int> occupancy;
  std::vector<float> delay_probs;
  Time::time_point t_start;
  Agents A;
  std::vector<State> HIST;
  int emulation_time;
  const bool verbose;

  void run();
  void activate(Agent_p a);
  bool isStable(Agent_p a) const;
  void info(const std::string& msg) const;

public:
  Execution(Problem* _P, const Plan& _plan, const float _ub_delay_prob=0.5, const bool _verbose=false);
  ~Execution() {}

  void printResult() const;
  void makeLog(const std::string& logfile = DEFAULT_OUTPUT_FILE) const;
};

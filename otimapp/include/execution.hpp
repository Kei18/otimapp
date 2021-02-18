#pragma once

#include <memory>
#include <fstream>
#include "problem.hpp"

class Execution {
private:
  Problem* const P;
  const std::string plan_file;
  const bool solved;
  const Plan solution;
  const int seed;
  std::mt19937* MT;
  const float ub_delay_prob;
  const bool verbose;


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

  // utilities
  Configs result;
  std::vector<int> occupancy;
  std::vector<float> delay_probs;
  Time::time_point t_start;
  Agents A;
  std::vector<State> HIST;
  int emulation_time;

  void activate(Agent_p a);
  bool isStable(Agent_p a) const;

  void info(const std::string& msg) const;
  void warn(const std::string& msg) const;

  Plan getPlan() const;
  bool getSolved() const;

public:
  Execution(Problem* _P,
            std::string _plan_file,
            int _seed = DEFAULT_SEED,
            float _ub_delay_prob = DEFAULT_UB_DELAY_PROB,
            bool _verbose = false);
  ~Execution();

  void run();
  void printResult() const;
  void makeLog(const std::string& logfile = DEFAULT_EXEC_OUTPUT_FILE) const;
};

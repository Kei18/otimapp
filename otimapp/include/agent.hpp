#pragma once
#include <memory>
#include <graph.hpp>

struct Agent {
  // omit REQUESTING mode
  // because it is unused in the execution of offline time-independent planning
  enum Mode { CONTRACTED, EXTENDED };

  // used to check occupancy
  static constexpr int NIL = -1;

  // to create a log
  // id, t, mode, head, tail
  using State = std::tuple<int, int, Mode, Node*, Node*>;

  int id;      // agent
  int t;       // internal timestep
  Mode mode;   // mode
  Node* head;  // head
  Node* tail;  // tail
  Path path;   // plan

  Agent(int _id, const Path& _path);
  ~Agent() {}

  void activate(std::vector<int>& occupancy);
  Node* getNextNode() const;
  bool isFinished() const;
  State getState() const;
};

using Agent_p = std::shared_ptr<Agent>;
using Agents = std::vector<Agent_p>;

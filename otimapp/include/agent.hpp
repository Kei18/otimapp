#pragma once
#include <memory>
#include <graph.hpp>

struct MAPF_DP_Agent;
using MAPF_DP_Agent_p = std::shared_ptr<MAPF_DP_Agent>;
using MAPF_DP_Agents = std::vector<MAPF_DP_Agent_p>;

struct PrimitiveAgent;
using PrimitiveAgent_p = std::shared_ptr<PrimitiveAgent>;
using PrimitiveAgents = std::vector<PrimitiveAgent_p>;


struct Agent {
  enum Mode { CONTRACTED, EXTENDED, REQUESTING };
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
  virtual ~Agent() {}

  virtual void activate(std::vector<int>& occupancy) {}

  Node* getNextNode() const;
  bool isFinished() const;
  State getState() const;
};

struct MAPF_DP_Agent : public Agent {
  MAPF_DP_Agent(int _id, const Path& _path);
  ~MAPF_DP_Agent() {}

  void activate(std::vector<int>& occupancy);
};

struct PrimitiveAgent : public Agent {
  PrimitiveAgent(int _id, const Path& _path);
  ~PrimitiveAgent() {}

  void activate(std::vector<int>& occupancy);
};

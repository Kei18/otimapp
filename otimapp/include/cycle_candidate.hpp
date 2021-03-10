#pragma once
#include <graph.hpp>
#include <queue>

struct CycleCandidate {
  std::deque<Node*> path;
  std::deque<int> agents;

  CycleCandidate() {}
};

struct TableCycle {
  std::vector<std::vector<CycleCandidate*>> t_head;  // table head
  std::vector<std::vector<CycleCandidate*>> t_tail;  // table tail
  Graph* G;
  int max_fragment_size;  // max fragment size except for potential deadlocks

  TableCycle(Graph* _G, const int _max_fragment_size=-1);
  ~TableCycle();

  bool existDuplication(const std::deque<Node*>& path, const std::deque<int>& agents);

  bool isValidTopologyCondition(CycleCandidate* const c) const;

  // create new entry
  CycleCandidate* createNewCycleCandidate(const int id, Node* head, CycleCandidate* c_base, Node* tail);

  // return deadlock or nullptr
  // force = false -> return when finding first cycle, false -> register all info
  CycleCandidate* registerNewPath(const int id, const Path path,
                                  const bool force = false,
                                  const int time_limit = -1);

  void println();
};

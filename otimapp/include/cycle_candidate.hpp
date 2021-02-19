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

  TableCycle(const int nodes_size);
  ~TableCycle();

  // check duplication
  bool existDuplication(Node* head, Node* tail);

  // create new entry
  CycleCandidate* createNewCycleCandidate(const int id, Node* head, CycleCandidate* c_base, Node* tail);

  // return deadlock or nullptr
  CycleCandidate* registerNewPath(const int id, const Path path, const int nodes_size);
};

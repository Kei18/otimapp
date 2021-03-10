#pragma once
#include <graph.hpp>
#include <queue>

struct Fragment {
  std::deque<Node*> path;  // head -> tail
  std::deque<int> agents;  // a_i, a_j, ..., a_l

  Fragment() {}
};

struct TableFragment {
  std::vector<std::vector<Fragment*>> t_from;  // table from
  std::vector<std::vector<Fragment*>> t_to;    // table to
  Graph* G;

  int max_fragment_size;  // maximum fragment size

  TableFragment(Graph* _G, const int _max_fragment_size=-1);
  ~TableFragment();

  // check duplication
  bool existDuplication(const std::deque<Node*>& path, const std::deque<int>& agents);

  // branching, valid only when max_fragment_size > 0
  bool isValidTopologyCondition(const std::deque<Node*>& path) const;

  // create new entry
  Fragment* createNewFragment(const std::deque<Node*>& path, const std::deque<int>& agents);

  // return potential deadlock if exists
  Fragment* getPotentialDeadlockIfExist(const int id, Node* head, Fragment* c_base, Node* tail);
  Fragment* getPotentialDeadlockIfExist(const std::deque<Node*>& path, const std::deque<int>& agents);


  // return deadlock or nullptr
  // force = false -> return when finding first cycle, false -> register all info
  Fragment* registerNewPath(const int id, const Path path,
                            const bool force = false,
                            const int time_limit = -1);

  // print registered info
  void println();
};

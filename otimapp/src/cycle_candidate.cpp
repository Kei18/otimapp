#include "../include/cycle_candidate.hpp"
#include <iostream>
#include <set>
#include "../include/util.hpp"

TableCycle::TableCycle(const int _nodes_size)
  : t_head(_nodes_size), t_tail(_nodes_size), nodes_size(_nodes_size)
{
}

TableCycle::~TableCycle()
{
  for (auto cycles : t_head) {
    for (auto c : cycles) delete c;
  }
}

bool TableCycle::existDuplication(const std::deque<Node*>& path, const std::deque<int>& agents)
{
  std::set<int> set_agents(agents.begin(), agents.end());
  for (auto c : t_head[path.front()->id]) {
    if (std::set(c->agents.begin(), c->agents.end()) != set_agents) continue;
    if (c->path != path) continue;
    return true;
  }
  return false;
}

// create new entry
CycleCandidate* TableCycle::createNewCycleCandidate
(const int id, Node* head, CycleCandidate* c_base, Node* tail)
{
  // create new entry
  auto c = new CycleCandidate();

  // agent
  std::deque<int> agents;
  if (c_base == nullptr) {
    agents.push_front(id);
  } else {
    agents = c_base->agents;
    if (c_base->path.front() != head) agents.push_front(id);
    if (c_base->path.back()  != tail) agents.push_back(id);
  }

  // path
  std::deque<Node*> path;
  if (c_base == nullptr || head != c_base->path.front()) path.push_back(head);
  if (c_base != nullptr)
    for (auto itr = c_base->path.begin(); itr != c_base->path.end(); ++itr) path.push_back(*itr);
  if (c_base == nullptr || tail != c_base->path.back()) path.push_back(tail);

  // check duplication
  if (existDuplication(path, agents)) return nullptr;

  // register
  c->agents = agents;
  c->path = path;
  t_head[c->path.front()->id].push_back(c);
  t_tail[c->path.back()->id].push_back(c);

  return c;
};

// return deadlock or nullptr
CycleCandidate* TableCycle::registerNewPath
(const int id, const Path path, const bool force)
{
  auto checkPotentialDeadlock =
    [&] (int id, Node* head, CycleCandidate* c_base, Node* tail) -> CycleCandidate*
    {
      // avoid loop with own path
      if (c_base != nullptr && inArray(id, c_base->agents)) return nullptr;

      // create new entry
      auto c = createNewCycleCandidate(id, head, c_base, tail);

      // detect deadlock
      if (c != nullptr && c->path.front() == c->path.back()) return c;
      return nullptr;
    };

  CycleCandidate* res = nullptr;

  // update cycles step by step
  for (int t = 1; t < (int)path.size(); ++t) {
    auto v_before = path[t-1];
    auto v_next = path[t];

    res = checkPotentialDeadlock(id, v_before, nullptr, v_next);
    if (!force && res != nullptr) return res;

    // check existing cycle, tail
    for (auto c : t_tail[v_before->id]) {
      res = checkPotentialDeadlock(id, c->path.front(), c, v_next);
      if (!force && res != nullptr) return res;
    }

    // check existing cycle, head
    for (auto c : t_head[v_next->id]) {
      res = checkPotentialDeadlock(id, v_before, c, c->path.back());
      if (!force && res != nullptr) return res;
    }

    // connect two cycle candidates
    // Note: this is heavy part for computation
    std::vector<CycleCandidate*> c_tails, c_heads;
    for (auto c_tail : t_tail[v_before->id])
      if (!inArray(id, c_tail->agents)) c_tails.push_back(c_tail);
    for (auto c_head : t_head[v_next->id])
      if (!inArray(id, c_head->agents)) c_heads.push_back(c_head);

    for (auto c_tail : c_tails) {
      for (auto c_head : c_heads) {
        // avoid self loop
        {
          bool self_loop = false;
          for (auto i : c_tail->agents) {
            if (inArray(i, c_head->agents)) {
              self_loop = true;
            }
          }
          if (self_loop) continue;
        }
        // create body
        std::deque<int> agents;
        std::deque<Node*> path;
        {
          // agents
          for (auto i : c_tail->agents) agents.push_back(i);
          agents.push_back(id);
          for (auto i : c_head->agents) agents.push_back(i);
          // path
          for (auto v : c_tail->path) path.push_back(v);
          for (auto v : c_head->path) path.push_back(v);
        }
        // check duplication
        {
          if (existDuplication(path, agents)) {
            continue;
          }
        }
        // register
        {
          auto c = new CycleCandidate();
          c->agents = agents;
          c->path = path;
          t_head[c->path.front()->id].push_back(c);
          t_tail[c->path.back()->id].push_back(c);
          // detect deadlock
          if (c->path.front() == c->path.back()) {
            res = c;
            if (!force) return res;
          }
        }
      }
    }
  }

  return res;
}

void TableCycle::println()
{
  for (auto cycles : t_head) {
    for (auto c : cycles) {
      for (auto v : c->path) std::cout << v->id << " -> ";
      std::cout << " : ";
      for (auto i : c->agents) std::cout << i << " -> ";
      std::cout << std::endl;
    }
  }
}

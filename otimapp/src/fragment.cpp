#include "../include/fragment.hpp"

#include <iostream>
#include <set>

#include "../include/util.hpp"

TableFragment::TableFragment(Graph* _G, const int _max_fragment_size)
    : t_from(_G->getNodesSize()),
      t_to(_G->getNodesSize()),
      G(_G),
      max_fragment_size(_max_fragment_size)
{
}

TableFragment::~TableFragment()
{
  for (auto cycles : t_from) {
    for (auto c : cycles) delete c;
  }
}

bool TableFragment::existDuplication(const std::deque<Node*>& path,
                                     const std::deque<int>& agents)
{
  std::set<int> set_agents;
  for (auto itr = agents.begin(); itr != agents.end(); ++itr)
    set_agents.insert(*itr);
  for (auto c : t_from[path.front()->id]) {
    // different paths
    if (c->path != path) continue;

    // different agents
    std::set<int> c_agents;
    for (auto itr = c->agents.begin(); itr != c->agents.end(); ++itr)
      c_agents.insert(*itr);
    if (c_agents != set_agents) continue;

    // duplication exists
    return true;
  }
  return false;
}

bool TableFragment::isValidTopologyCondition(
    const std::deque<Node*>& path) const
{
  if (max_fragment_size == -1) return true;

  auto head = path.front();
  auto tail = path.back();
  auto length = (int)path.size() - 1;  // number of agents in the fragment

  // fast check
  if (head->manhattanDist(tail) + length > max_fragment_size) return false;

  // finding shortest path
  Nodes prohibited;
  for (int t = 1; t < (int)path.size() - 1; ++t) prohibited.push_back(path[t]);
  auto p = G->getPath(tail, head, prohibited);
  if (p.empty()) return false;
  if ((int)p.size() - 1 + length > max_fragment_size) return false;

  return true;
}

Fragment* TableFragment::createNewFragment(const std::deque<Node*>& path,
                                           const std::deque<int>& agents)
{
  auto c = new Fragment();
  c->agents = agents;
  c->path = path;

  // register on tables
  t_from[c->path.front()->id].push_back(c);
  t_to[c->path.back()->id].push_back(c);

  return c;
}

Fragment* TableFragment::getPotentialDeadlockIfExist(
    const std::deque<Node*>& path, const std::deque<int>& agents)
{
  // check topology constraints
  if (path.front() != path.back() && !isValidTopologyCondition(path))
    return nullptr;

  // check duplication
  if (existDuplication(path, agents)) return nullptr;

  // create new fragment
  auto c = createNewFragment(path, agents);

  return (c->path.front() == c->path.back()) ? c : nullptr;
}

// create new entry
Fragment* TableFragment::getPotentialDeadlockIfExist(const int id, Node* head,
                                                     Fragment* c_base,
                                                     Node* tail)
{
  // avoid loop with own path
  if (c_base != nullptr && inArray(id, c_base->agents)) return nullptr;

  // check maximum fragment length
  if (c_base != nullptr && max_fragment_size != -1) {
    auto size = (int)c_base->agents.size() + 1;
    if (size > max_fragment_size) {
      return nullptr;
    } else if (size == max_fragment_size && head != tail) {
      return nullptr;
    }
  }

  // setup agents
  std::deque<int> agents;
  if (c_base == nullptr) {
    agents.push_front(id);
  } else {
    agents = c_base->agents;
    if (c_base->path.front() != head) agents.push_front(id);
    if (c_base->path.back() != tail) agents.push_back(id);
  }

  // setup path
  std::deque<Node*> path;
  if (c_base == nullptr || head != c_base->path.front()) path.push_back(head);
  if (c_base != nullptr)
    for (auto itr = c_base->path.begin(); itr != c_base->path.end(); ++itr)
      path.push_back(*itr);
  if (c_base == nullptr || tail != c_base->path.back()) path.push_back(tail);

  return getPotentialDeadlockIfExist(path, agents);
}

// return deadlock or nullptr
Fragment* TableFragment::registerNewPath(const int id, const Path path,
                                         const bool force, const int time_limit)
{
  Fragment* res = nullptr;
  auto t_s = Time::now();

  // update cycles step by step
  for (int t = 1; t < (int)path.size(); ++t) {
    // check time limit
    if (time_limit >= 0 && getElapsedTime(t_s) > time_limit) return nullptr;

    auto v_before = path[t - 1];
    auto v_next = path[t];

    // add own segment
    res = getPotentialDeadlockIfExist(id, v_before, nullptr, v_next);
    if (!force && res != nullptr) return res;

    // check existing fragments on table_to
    for (auto c : t_to[v_before->id]) {
      res = getPotentialDeadlockIfExist(id, c->path.front(), c, v_next);
      if (!force && res != nullptr) return res;
    }

    // check existing fragments on table_from
    for (auto c : t_from[v_next->id]) {
      res = getPotentialDeadlockIfExist(id, v_before, c, c->path.back());
      if (!force && res != nullptr) return res;
    }

    // connect two fragments
    std::vector<Fragment*> c_tails, c_heads;
    // 1. extract candidates
    for (auto c_tail : t_to[v_before->id])
      if (!inArray(id, c_tail->agents)) c_tails.push_back(c_tail);
    for (auto c_head : t_from[v_next->id])
      if (!inArray(id, c_head->agents)) c_heads.push_back(c_head);

    // 2. main loop
    for (auto c_tail : c_tails) {
      // check time limit
      if (time_limit >= 0 && getElapsedTime(t_s) > time_limit) return nullptr;

      for (auto c_head : c_heads) {
        // check length
        if (max_fragment_size != -1) {
          int size = (int)(c_tail->agents.size() + c_head->agents.size()) + 1;
          if (size > max_fragment_size) {
            continue;
          } else if (size == max_fragment_size &&
                     c_tail->path.front() != c_head->path.back()) {
            continue;
          }
        }

        // avoid self loop
        {
          // agents
          bool self_loop = false;
          for (auto i : c_tail->agents) {
            if (inArray(i, c_head->agents)) {
              self_loop = true;
            }
          }
          if (self_loop) continue;

          // path
          for (auto i : c_tail->path) {
            if (inArray(i, c_head->path)) {
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

        // register
        auto res = getPotentialDeadlockIfExist(path, agents);
        if (!force && res != nullptr) return res;
      }
    }
  }

  return res;
}

void TableFragment::println()
{
  for (auto cycles : t_from) {
    for (auto c : cycles) {
      for (auto v : c->path) std::cout << v->id << " -> ";
      std::cout << " : ";
      for (auto i : c->agents) std::cout << i << " -> ";
      std::cout << std::endl;
    }
  }
}

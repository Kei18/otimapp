#include "../include/cycle_candidate.hpp"

TableCycle::TableCycle(const int _nodes_size)
  : t_head(_nodes_size), t_tail(_nodes_size), nodes_size(_nodes_size)
{
}

TableCycle::~TableCycle()
{
  for (auto cycles : t_tail) {
    for (auto c : cycles) delete c;
  }
}

// check duplication
bool TableCycle::existDuplication(Node* head, Node* tail)
{
  auto cycles = t_head[head->id];
  return
    std::find_if(cycles.begin(), cycles.end(),
                 [&tail] (CycleCandidate* c)
                 { return c->path.back() == tail; }) != cycles.end();
};

// create new entry
CycleCandidate* TableCycle::createNewCycleCandidate
(const int id, Node* head, CycleCandidate* c_base, Node* tail)
{
  // check duplication
  if (existDuplication(head, tail)) return nullptr;

  // create new entry
  auto c = new CycleCandidate();

  // agent
  if (c_base == nullptr) {
    c->agents.push_front(id);
  } else {
    c->agents = c_base->agents;
    if (c_base->path.front() != head) c->agents.push_front(id);
    if (c_base->path.back()  != tail) c->agents.push_back(id);
  }

  // path
  if (c_base == nullptr || head != c_base->path.front()) c->path.push_back(head);
  if (c_base != nullptr)
    for (auto itr = c_base->path.begin(); itr != c_base->path.end(); ++itr) c->path.push_back(*itr);
  if (c_base == nullptr || tail != c_base->path.back()) c->path.push_back(tail);

  // register
  t_head[c->path.front()->id].push_back(c);
  t_tail[c->path.back()->id].push_back(c);

  return c;
};

// return deadlock or nullptr
CycleCandidate* TableCycle::registerNewPath
(const int id, const Path path, const bool force)
{
  // avoid loop with own path
  std::vector<bool> table_path_until_t_minus2(nodes_size, false);
  auto checkPotentialDeadlock =
    [&] (int id, Node* head, CycleCandidate* c_base, Node* tail) -> CycleCandidate*
    {
      // avoid loop with own path
      if (c_base != nullptr) {
        for (auto itr = c_base->path.begin(); itr != c_base->path.end(); ++itr) {
          if (table_path_until_t_minus2[(*itr)->id]) return nullptr;
        }
      }

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

    // update part of path
    if (t >= 2) table_path_until_t_minus2[path[t-2]->id] = true;

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
  }

  return res;
}

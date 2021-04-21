#include "../include/agent.hpp"

Agent::Agent(int _id, const Path& _path)
    : id(_id),
      t(0),
      mode(Mode::CONTRACTED),
      head(nullptr),
      tail(_path[0]),
      path(_path)
{
}

Node* Agent::getNextNode() const
{
  return (t < (int)path.size() - 1) ? path[t + 1] : nullptr;
}

bool Agent::isFinished() const
{
  return mode == Mode::CONTRACTED && t == (int)path.size() - 1;
}

Agent::State Agent::getState() const
{
  return std::make_tuple(id, t, mode, head, tail);
}

MAPF_DP_Agent::MAPF_DP_Agent(int _id, const Path& _path) : Agent(_id, _path) {}

void MAPF_DP_Agent::activate(std::vector<int>& occupancy)
{
  if (isFinished()) return;

  if (mode == Mode::EXTENDED) {
    // update occupancy
    occupancy[tail->id] = NIL;
    // update state
    mode = Mode::CONTRACTED;
    tail = head;
    head = nullptr;
  } else {
    auto v = getNextNode();
    if (occupancy[v->id] == NIL) {  // check occupancy
      // update state
      mode = Mode::EXTENDED;
      head = v;
      t += 1;
      // update occupancy
      occupancy[v->id] = id;
    }
  }
}

PrimitiveAgent::PrimitiveAgent(int _id, const Path& _path) : Agent(_id, _path)
{
}

void PrimitiveAgent::activate(std::vector<int>& occupancy)
{
  if (isFinished()) return;
  auto v = getNextNode();
  if (occupancy[v->id] == NIL) {
    // update state
    occupancy[tail->id] = NIL;
    tail = v;
    occupancy[v->id] = id;
    t += 1;
  }
}

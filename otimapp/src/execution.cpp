#include "../include/execution.hpp"

static int getSOC(const Configs& result)
{
  int soc = 0;
  const int makespan = result.size()-1;
  const int num_agents = result[0].size();
  for (int i = 0; i < num_agents; ++i) {
    int c = makespan;
    auto g = result[makespan][i];
    while (c > 0 && result[c-1][i] == g) --c;
    soc += c;
  }
  return soc;
}

Execution::Agent::Agent(int _id, const Path& _path)
  : id(_id),
    t(0),
    mode(Mode::CONTRACTED),
    head(nullptr),
    tail(_path[0]),
    path(_path)
{
}

Node* Execution::Agent::getNextNode() const
{
  return (t < path.size()-1) ? path[t+1] : nullptr;
}

bool Execution::Agent::isFinished() const
{
  return mode == Mode::CONTRACTED && t == (int)path.size()-1;
}

Execution::State Execution::Agent::getState() const
{
  return std::make_tuple(id, t, mode, head, tail);
}

Execution::Execution(Problem* _P, const Plan& _plan, const float _ub_delay_prob)
  : P(_P),
    MT(P->getMT()),
    solution(_plan),
    ub_delay_prob(_ub_delay_prob),
    occupancy(P->getG()->getNodesSize(), NIL)
{
  // setup delay probabilities
  for (int i = 0; i < P->getNum(); ++i) {
    delay_probs.push_back(getRandomFloat(0, ub_delay_prob, MT));
  }

  // emulate execution
  run();
}

void Execution::run()
{
  info("  emulate execution");
  t_start = Time::now();

  // initiate
  for (int i = 0; i < P->getNum(); ++i) {
    A.push_back(std::make_shared<Agent>(i, solution[i]));
    occupancy[A[i]->tail->id] = i;
  }

  info("  activate agents repeatedly");

  // repeat activation
  Agents unstable;
  int num_goal_agents = 0;
  // int timestep = 0;

  while (true) {

    // info("  elapsed:" + std::to_string((int)getElapsedTime(t_start))
    //      + ", timestep:" + std::to_string(++timestep));

    // step 1, check transition
    for (int i = 0; i < P->getNum(); ++i) {
      auto a = A[i];
      if (a->mode == Mode::CONTRACTED && !a->isFinished()) {
        unstable.push_back(a);
      } else if (a->mode == Mode::EXTENDED) {
        // skip according to probability
        if (getRandomFloat(0, 1, MT) <= delay_probs[i]) continue;

        activate(a);

        if (a->isFinished()) {
          ++num_goal_agents;
        } else {
          unstable.push_back(a);
        }
      }
    }

    // register all
    Config c;
    for (auto a : A) c.push_back(a->tail);
    result.push_back(c);

    // check goal condition
    if (num_goal_agents == P->getNum()) break;

    // step 2, activate unstable agents
    do {
      std::shuffle(unstable.begin(), unstable.end(), *MT);
      while (!unstable.empty()) {
        // pickup one agent
        auto a = randomChoose(unstable, MT);
        activate(a);

        // remove from unstable when agent is stable
        if (isStable(a)) {
          unstable.erase(std::find(unstable.begin(), unstable.end(), a));
        }
      }

      // check again whether agents are in stable
      for (auto a : A) {
        if (!isStable(a)) unstable.push_back(a);
      }

    } while (!unstable.empty());
  }

  emulation_time = getElapsedTime(t_start);
  info("  elapsed: "  + std::to_string(emulation_time) + ", finish emulation" +
       ", soc: " + std::to_string(getSOC(result)) +
       ", makespan: " + std::to_string(result.size()-1));
}

void Execution::activate(Agent_p a)
{
  if (a->mode == Mode::EXTENDED) {
    // update occupancy
    occupancy[a->tail->id] = NIL;
    // update state
    a->mode = Mode::CONTRACTED;
    a->tail = a->head;
    a->head = nullptr;

    // CONTRACTED
  } else if (!a->isFinished()) {
    Node* v = a->getNextNode();

    // check occupancy
    if (occupancy[v->id] == NIL) {
      // update state
      a->mode = Mode::EXTENDED;
      a->head = v;
      a->t += 1;
      // update occupancy
      occupancy[v->id] = a->id;
    }
  }

  // update record
  HIST.push_back(a->getState());
}

bool Execution::isStable(Agent_p a) const
{
  if (a->mode == Mode::EXTENDED || a->isFinished()) return true;

  // next location
  auto v_next = a->getNextNode();

  // agent who uses v_next
  auto a_j = occupancy[v_next->id];

  // no one uses v_next
  if (a_j == NIL) return false;

  // when a_j is in extended
  if (isStable(A[a_j])) return true;

  // when a_j is in contracted
  return false;
}

void Execution::makeLog(const std::string& logfile) const
{
  const int makespan = result.size() - 1;
  const int soc = getSOC(result);

  std::ofstream log;
  log.open(logfile, std::ios::app);
  log << "emulation_time=" << emulation_time << "\n";
  log << "ub_delay_prob=" << ub_delay_prob << "\n";
  log << "activate_cnts=" << HIST.size() << "\n";
  log << "makespan=" << result.size()-1 << "\n";
  log << "soc=" << soc << "\n";
  log << "result=\n";
  for (int t = 0; t <= makespan; ++t) {
    log << t << ":";
    auto c = result[t];
    for (auto v : c) {
      log << "(" << v->pos.x << "," << v->pos.y << "),";
    }
    log << "\n";
  }
  log << "execution(id,t,mode,head,tail)=\n";
  for (int i = 0; i < (int)HIST.size(); ++i) {
    auto s = HIST[i];
    auto m = std::get<2>(s);
    log << i+1 << ":("
        << std::get<0>(s) << ","
        << std::get<1>(s) << ","
        << (int)m << ","
        << ((m == Mode::EXTENDED) ? std::get<3>(s)->id : NIL)<< ","
        << std::get<4>(s)->id << ")\n";
  }
  log.close();
}

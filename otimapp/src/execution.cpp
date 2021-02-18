#include "../include/execution.hpp"
#include <fstream>
#include <regex>


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

Execution::Execution
(Problem* _P, std::string _plan_file, int _seed, float _ub_delay_prob, bool _verbose)
  : P(_P),
    plan_file(_plan_file),
    solved(getSolved()),
    solution(getPlan()),
    seed(_seed),
    MT(new std::mt19937(seed)),
    ub_delay_prob(_ub_delay_prob),
    verbose(_verbose),
    occupancy(P->getG()->getNodesSize(), NIL)
{
  // setup delay probabilities
  for (int i = 0; i < P->getNum(); ++i) {
    delay_probs.push_back(getRandomFloat(0, ub_delay_prob, MT));
  }
}

Execution::~Execution()
{
  delete MT;
}

void Execution::run()
{
  if (!solved) {
    warn("  " + P->getInstanceFileName() + " is unsolved in " + plan_file);
    return;
  }

  info("  emulate execution, ub_delay_prob=" + std::to_string(ub_delay_prob));
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

  while (true) {

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

void Execution::info(const std::string& msg) const
{
  if (verbose) std::cout << msg << std::endl;
}

void Execution::warn(const std::string& msg) const
{
  std::cout << "warn@Execution: " << msg << std::endl;
}

Plan Execution::getPlan() const
{
  std::ifstream file(plan_file);
  if (!file) {
    std::cout << "error@app: " << plan_file << " cannot be opened" << std::endl;
    std::exit(1);
  }

  Plan plan;  // solution

  std::regex r_plan = std::regex(R"(plan=)");
  std::regex r_path = std::regex(R"(\d+:(.+))");
  std::regex r_pos = std::regex(R"((\d+),)");

  std::string line;
  std::smatch results;
  while (getline(file, line)) {
    if (std::regex_match(line, results, r_plan)) {
      while (getline(file, line)) {
        if (std::regex_match(line, results, r_path)) {
          auto s = results[1].str();
          Path path;
          auto iter = s.cbegin();
          while (std::regex_search(iter, s.cend(), results, r_pos)) {
            iter = results[0].second;
            auto i = std::stoi(results[1].str());
            path.push_back(P->getG()->getNode(i));
          }
          plan.push_back(path);
        }
      }
    }
  }

  return plan;
}

bool Execution::getSolved() const
{
  std::ifstream file(plan_file);
  if (!file) {
    std::cout << "error@app: " << plan_file << " cannot be opened" << std::endl;
    std::exit(1);
  }
  std::regex r_solved = std::regex(R"(solved=(\d))");
  std::string line;
  std::smatch results;
  while (getline(file, line)) {
    if (std::regex_match(line, results, r_solved))
      return (bool)std::stoi(results[1].str());
  }
  return false;
}

void Execution::printResult() const
{
  std::cout << "finish emulation"
            << ", elapsed: " << emulation_time
            << ", soc: " << getSOC(result)
            << ", makespan: " << result.size()-1
            << std::endl;
}

void Execution::makeLog(const std::string& logfile) const
{
  const int makespan = result.size() - 1;
  const int soc = getSOC(result);

  std::ofstream log;
  log.open(logfile, std::ios::out);

  // copy plan file
  log << "// log from " << plan_file << "\n---\n";
  std::ifstream file(plan_file);
  if (!file) {
    std::cout << "error@app: " << plan_file << " cannot be opened" << std::endl;
    std::exit(1);
  }
  std::string line;
  while (getline(file, line)) log << line << "\n";
  file.close();

  // new info
  log << "---\n// exec result" << plan_file << "\n---\n";
  log << "ub_delay_prob=" << ub_delay_prob << "\n";
  log << "delay_probs=";
  for (auto p : delay_probs) log << p << ",";
  log << "\n";
  log << "exec_seed=" << seed << "\n";
  log << "starts=";
  for (int i = 0; i < P->getNum(); ++i) {
    Node* v = P->getStart(i);
    log << "(" << v->pos.x << "," << v->pos.y << "),";
  }
  log << "\ngoals=";
  for (int i = 0; i < P->getNum(); ++i) {
    Node* v = P->getGoal(i);
    log << "(" << v->pos.x << "," << v->pos.y << "),";
  }
  log << "plan=" << plan_file << "\n";
  log << "emulation_time=" << emulation_time << "\n";
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

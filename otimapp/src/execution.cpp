#include "../include/execution.hpp"
#include <fstream>
#include <regex>
#include <functional>

Execution::Execution
(Problem* _P, std::string _plan_file, int _seed, float _ub_delay_prob, bool _verbose)
  : P(_P),
    plan_file(_plan_file),
    solved(getSolved()),
    plan(getPlan()),
    seed(_seed),
    MT(new std::mt19937(seed)),
    ub_delay_prob(_ub_delay_prob),
    verbose(_verbose),
    occupancy(P->getG()->getNodesSize(), Agent::NIL)
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

// -------------------------------
// main
// -------------------------------
void Execution::run()
{
  if (!solved) {
    warn("  " + P->getInstanceFileName() + " is unsolved in " + plan_file);
    return;
  }

  info("  emulate execution, ub_delay_prob=" + std::to_string(ub_delay_prob));
  auto t_start = Time::now();

  // setup agents
  Agents A;
  for (int i = 0; i < P->getNum(); ++i) {
    A.push_back(std::make_shared<Agent>(i, plan[i]));
    occupancy[A[i]->tail->id] = i;
  }

  // setup utilities
  std::function<bool(Agent_p)> isStable = [&] (Agent_p a)
  {
    if (a->mode == Agent::EXTENDED || a->isFinished()) return true;
    // next location
    auto v_next = a->getNextNode();
    // agent who uses v_next
    auto a_j = occupancy[v_next->id];
    // no one uses v_next
    if (a_j == Agent::NIL) return false;
    // when a_j is in extended
    if (isStable(A[a_j])) return true;
    // when a_j is in contracted
    return false;
  };

  auto activate = [&] (Agent_p a)
  {
    a->activate(occupancy);
    HIST.push_back(a->getState());  // update record
  };

  info("  activate agents repeatedly");

  // repeat activation
  Agents unstable;
  int num_goal_agents = 0;

  while (true) {

    // step 1, check transition
    for (int i = 0; i < P->getNum(); ++i) {
      auto a = A[i];
      if (a->mode == Agent::CONTRACTED && !a->isFinished()) {
        unstable.push_back(a);
      } else if (a->mode == Agent::EXTENDED) {
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
    exec_result.push_back(c);

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

  // validation
  if (!validate(exec_result, P)) halt("invalid execution");
}

// -------------------------------
// utilities for debug
// -------------------------------
void Execution::info(const std::string& msg) const
{
  if (verbose) std::cout << msg << std::endl;
}

void Execution::warn(const std::string& msg) const
{
  std::cout << "warn@Execution: " << msg << std::endl;
}

void Execution::halt(const std::string& msg) const
{
  std::cout << "error@Execution: " << msg << std::endl;
  this->~Execution();
  std::exit(1);
}

// -------------------------------
// read planning
// -------------------------------
Plan Execution::getPlan() const
{
  std::ifstream file(plan_file);
  if (!file) halt(plan_file + " cannot be opened");
  Plan _plan;  // solution
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
          _plan.push_back(path);
        }
      }
    }
  }
  return _plan;
}

bool Execution::getSolved() const
{
  std::ifstream file(plan_file);
  if (!file) halt(plan_file + " cannot be opened");
  std::regex r_solved = std::regex(R"(solved=(\d))");
  std::string line;
  std::smatch results;
  while (getline(file, line)) {
    if (std::regex_match(line, results, r_solved))
      return (bool)std::stoi(results[1].str());
  }
  return false;
}

// -------------------------------
// others
// -------------------------------
void Execution::printResult() const
{
  std::cout << "finish emulation"
            << ", elapsed: " << emulation_time
            << ", soc: " << getSOC(exec_result)
            << ", makespan: " << getMakespan(exec_result)
            << std::endl;
}

void Execution::makeLog(const std::string& logfile) const
{
  const int makespan = getMakespan(exec_result);
  const int soc = getSOC(exec_result);

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
  log << "plan=" << plan_file << "\n";
  log << "ub_delay_prob=" << ub_delay_prob << "\n";
  log << "delay_probs=";
  for (auto p : delay_probs) log << p << ",";
  log << "\n";
  log << "exec_seed=" << seed << "\n";
  log << "emulation_time=" << emulation_time << "\n";
  log << "activate_cnts=" << HIST.size() << "\n";
  log << "makespan=" << makespan << "\n";
  log << "soc=" << soc << "\n";
  log << "result=\n";
  for (int t = 0; t <= makespan; ++t) {
    log << t << ":";
    auto c = exec_result[t];
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
        << ((m == Agent::EXTENDED) ? std::get<3>(s)->id : -1)<< ","
        << std::get<4>(s)->id << ")\n";
  }
  log.close();
}

#include "../include/execution.hpp"
#include <regex>
#include <functional>

const std::string MAPF_DP_Execution::PROBLEM_NAME = "MAPF_DP";
const std::string PrimitiveExecution::PROBLEM_NAME = "PRIMITIVE";

Execution::Execution
(Problem* _P, std::string _plan_file, int _seed, bool _verbose, bool _log_short)
  : P(_P),
    plan_file(_plan_file),
    exec_succeed(false),
    seed(_seed),
    MT(new std::mt19937(seed)),
    verbose(_verbose),
    log_short(_log_short)
{
  // read plan results
  solved = getSolved();
  plan = getPlan();
}

Execution::~Execution()
{
  delete MT;
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
  std::regex r_instance = std::regex(R"(instance=(.+))");
  std::regex r_plan = std::regex(R"(plan=)");
  std::regex r_path = std::regex(R"(\d+:(.+))");
  std::regex r_pos = std::regex(R"((\d+),)");
  std::string line;
  std::smatch results;
  while (getline(file, line)) {
    if (std::regex_match(line, results, r_instance)) {
      if (results[1].str() != P->getInstanceFileName()) {
        halt("different instance");
      }
    }
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
// main
void Execution::run()
{
  if (!solved) {
    warn("  " + P->getInstanceFileName() + " is unsolved in " + plan_file);
    return;
  }

  info("  emulate execution");
  auto t_start = Time::now();

  simulate();

  emulation_time = getElapsedTime(t_start);

  // validation
  if (!exec_result.empty() && exec_succeed) {
    if (!validateMAPFPlan(exec_result, P)) halt("invalid execution");
  }
}

// -------------------------------
// others
// -------------------------------
void Execution::printResult() const
{
  std::cout << "finish emulation"
            << ", elapsed: " << emulation_time
            << ", activation cnt: " << HIST.size()
            << ", succeed: " << exec_succeed;
  if (!exec_result.empty()) {
    std::cout << ", soc: " << getSOC(exec_result)
              << ", makespan: " << getMakespan(exec_result);
  }
  std::cout << std::endl;
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
  log << "---\n// exec result\n---\n";
  log << "problem_name=" << problem_name << "\n";
  log << "plan=" << plan_file << "\n";
  makeLogSpecific(log);
  log << "exec_succeed=" << exec_succeed << "\n";
  log << "exec_seed=" << seed << "\n";
  log << "emulation_time=" << emulation_time << "\n";
  log << "activate_cnts=" << HIST.size() << "\n";
  log << "makespan=" << makespan << "\n";
  log << "soc=" << soc << "\n";
  if (!log_short) {
    log << "result=\n";
    if (solved && !exec_result.empty()) {
      for (int t = 0; t <= makespan; ++t) {
        log << t << ":";
        auto c = exec_result[t];
        for (auto v : c) {
          log << "(" << v->pos.x << "," << v->pos.y << "),";
        }
        log << "\n";
      }
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
  }
  log.close();
}

// -------------------------------
// main
MAPF_DP_Execution::MAPF_DP_Execution
(Problem* _P, std::string _plan_file, int _seed, float _ub_delay_prob, bool _verbose, bool _log_short)
  : Execution(_P, _plan_file, _seed, _verbose, _log_short),
    ub_delay_prob(_ub_delay_prob)
{
  problem_name = PROBLEM_NAME;

  // setup delay probabilities
  for (int i = 0; i < P->getNum(); ++i) {
    delay_probs.push_back(getRandomFloat(0, ub_delay_prob, MT));
  }
}

// -------------------------------
// main
// -------------------------------
void MAPF_DP_Execution::simulate()
{
  info("  ub_delay_prob=" + std::to_string(ub_delay_prob));

  // occupied nodes
  std::vector<int> occupancy(P->getG()->getNodesSize(), MAPF_DP_Agent::NIL);

  // setup agents
  MAPF_DP_Agents A;
  for (int i = 0; i < P->getNum(); ++i) {
    A.push_back(std::make_shared<MAPF_DP_Agent>(i, plan[i]));
    occupancy[A[i]->tail->id] = i;
  }

  // setup utilities
  bool deadlock_detected = false;
  std::function<bool(MAPF_DP_Agent_p, MAPF_DP_Agents&)> isStable =
    [&] (MAPF_DP_Agent_p a, MAPF_DP_Agents& agents)
    {
      if (a->mode == MAPF_DP_Agent::EXTENDED || a->isFinished()) return true;
      // next location
      auto v_next = a->getNextNode();
      // agent who uses v_next
      auto a_j = occupancy[v_next->id];
      // no one uses v_next
      if (a_j == MAPF_DP_Agent::NIL) return false;
      // check deadlock
      if (inArray(A[a_j], agents)) {
        std::string msg = "detect deadlock: ";
        for (auto b : agents) {
          msg += std::to_string(b->id) + " at " + std::to_string(b->tail->id) + ", ";
        }
        info(msg);
        deadlock_detected = true;
        return false;
      }
      agents.push_back(A[a_j]);
      // when a_j is in extended
      if (isStable(A[a_j], agents)) return true;
      // when a_j is in contracted
      return false;
    };

  auto activate = [&] (MAPF_DP_Agent_p a)
  {
    a->activate(occupancy);
    HIST.push_back(a->getState());  // update record
  };

  // repeat activation
  MAPF_DP_Agents unstable;
  int num_goal_agents = 0;

  info("  activate agents repeatedly");
  while (!deadlock_detected) {

    // step 1, check transition
    for (int i = 0; i < P->getNum(); ++i) {
      auto a = A[i];
      if (a->mode == MAPF_DP_Agent::CONTRACTED && !a->isFinished()) {
        unstable.push_back(a);
      } else if (a->mode == MAPF_DP_Agent::EXTENDED) {
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
    if (num_goal_agents == P->getNum()) {
      exec_succeed = true;
      break;
    }

    // step 2, activate unstable agents
    do {
      std::shuffle(unstable.begin(), unstable.end(), *MT);
      while (!unstable.empty()) {
        // pickup one agent
        auto a = randomChoose(unstable, MT);
        activate(a);

        // remove from unstable when agent is stable
        MAPF_DP_Agents vec = { a };
        if (isStable(a, vec)) {
          unstable.erase(std::find(unstable.begin(), unstable.end(), a));
        }

        // check deadlock
        if (deadlock_detected) break;
      }

      // check again whether agents are in stable
      for (auto a : A) {
        MAPF_DP_Agents vec = { a };
        if (!isStable(a, vec)) unstable.push_back(a);

        // check deadlock
        if (deadlock_detected) break;
      }

    } while (!unstable.empty() && !deadlock_detected);
  }
}

void MAPF_DP_Execution::makeLogSpecific(std::ofstream& log) const
{
  log << "ub_delay_prob=" << ub_delay_prob << "\n";
  log << "delay_probs=";
  for (auto p : delay_probs) log << p << ",";
  log << "\n";
}

PrimitiveExecution::PrimitiveExecution
(Problem* _P, std::string _plan_file, int _seed, bool _verbose, bool _log_short)
  : Execution(_P, _plan_file, _seed, _verbose, _log_short)
{
  problem_name = PROBLEM_NAME;
}

void PrimitiveExecution::simulate()
{
  // occupied nodes
  std::vector<int> occupancy(P->getG()->getNodesSize(), Agent::NIL);

  // setup agents
  PrimitiveAgents A;
  for (int i = 0; i < P->getNum(); ++i) {
    A.push_back(std::make_shared<PrimitiveAgent>(i, plan[i]));
    occupancy[A[i]->tail->id] = i;
  }

  // setup utilities
  std::function<bool(PrimitiveAgent_p, PrimitiveAgents&)> checkNoDeadlock =
    [&] (PrimitiveAgent_p a, PrimitiveAgents& agents)
    {
      // i.e., agents cannot move
      if (a->isFinished()) return false;
      // next location
      auto v_next = a->getNextNode();
      // agent who uses v_next
      auto a_j = occupancy[v_next->id];
      // no one uses v_next
      if (a_j == Agent::NIL) return true;
      // check deadlock
      if (inArray(A[a_j], agents)) {
        std::string msg = "detect deadlock: ";
        for (auto b : agents) {
          msg += std::to_string(b->id) + " at " + std::to_string(b->tail->id) + ", ";
        }
        info(msg);
        return false;
      }
      agents.push_back(A[a_j]);
      return checkNoDeadlock(A[a_j], agents);
    };

  auto activate = [&] (PrimitiveAgent_p a)
  {
    a->activate(occupancy);
    HIST.push_back(a->getState());  // update record
  };

  int num_goal_agents = 0;

  while (true) {
    auto a = randomChoose(A, MT);
    if (a->isFinished()) continue;

    PrimitiveAgents vec = { a };
    if (!checkNoDeadlock(a, vec)) break;

    activate(a);
    if (a->isFinished()) {
      ++num_goal_agents;

      // finish
      if (num_goal_agents >= P->getNum()) {
        exec_succeed = true;
        break;
      }
    }
  }
}

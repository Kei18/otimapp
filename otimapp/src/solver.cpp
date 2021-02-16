#include "../include/solver.hpp"

#include <fstream>
#include <iomanip>

MinimumSolver::MinimumSolver(Problem* _P)
  : solver_name(""),
    P(_P),
    G(_P->getG()),
    MT(_P->getMT()),
    max_comp_time(P->getMaxCompTime()),
    solved(false),
    comp_time(0)
{
}

void MinimumSolver::solve()
{
  start();
  exec();
  end();
}

void MinimumSolver::start()
{
  t_start = Time::now();
}

void MinimumSolver::end()
{
  comp_time = getSolverElapsedTime();
}

int MinimumSolver::getSolverElapsedTime() const { return getElapsedTime(t_start); }

// -----------------------------------------------
// base class with utilities
// -----------------------------------------------

Solver::Solver(Problem* _P)
  : MinimumSolver(_P),
    verbose(false),
    distance_table(P->getNum(), std::vector<int>(G->getNodesSize(), G->getNodesSize()))
{
}

Solver::~Solver()
{
}

// -------------------------------
// main
// -------------------------------
void Solver::exec()
{
  // create distance table
  info("  pre-processing, create distance table by BFS");
  createDistanceTable();
  info("  done, elapsed: ", getSolverElapsedTime());

  // main
  run();
}

// -------------------------------
// utilities for time
// -------------------------------
int Solver::getRemainedTime() const
{
  return std::max(0, max_comp_time - getSolverElapsedTime());
}

bool Solver::overCompTime() const
{
  return getSolverElapsedTime() >= max_comp_time;
}

// -------------------------------
// utilities for debug
// -------------------------------
void Solver::info() const
{
  if (verbose) std::cout << std::endl;
}

void Solver::halt(const std::string& msg) const
{
  std::cout << "error@" << solver_name << ": " << msg << std::endl;
  this->~Solver();
  std::exit(1);
}

void Solver::warn(const std::string& msg) const
{
  std::cout << "warn@ " << solver_name << ": " << msg << std::endl;
}

// -------------------------------
// utilities for log
// -------------------------------
void Solver::makeLog(const std::string& logfile)
{
  std::ofstream log;
  log.open(logfile, std::ios::out);
  makeLogBasicInfo(log);
  makeLogSolution(log);
  log.close();
}

void Solver::makeLogBasicInfo(std::ofstream& log)
{
  Grid* grid = reinterpret_cast<Grid*>(P->getG());
  log << "instance=" << P->getInstanceFileName() << "\n";
  log << "agents=" << P->getNum() << "\n";
  log << "map_file=" << grid->getMapFileName() << "\n";
  log << "solver=" << solver_name << "\n";
  log << "solved=" << solved << "\n";
  log << "comp_time=" << getCompTime() << "\n";
}

void Solver::makeLogSolution(std::ofstream& log)
{
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
  log << "\n";
  log << "plan=\n";
  for (int i = 0; i < (int)solution.size(); ++i) {
    log << i << ":";
    for (auto v : solution[i]) log << v->id << ",";
    log << "\n";
  }

  // TODO: make solution
}

// -------------------------------
// print
// -------------------------------
void Solver::printResult()
{
  std::cout << "solved=" << solved << ", solver=" << std::right << std::setw(8)
            << solver_name << ", comp_time(ms)=" << std::right << std::setw(8)
            << getCompTime() << std::endl;
}

void Solver::printHelpWithoutOption(const std::string& solver_name)
{
  std::cout << solver_name << "\n"
            << "  (no option)" << std::endl;
}

// -------------------------------
// distance
// -------------------------------
int Solver::pathDist(const int i, Node* const s) const
{
  return distance_table[i][s->id];
}

int Solver::pathDist(const int i) const { return pathDist(i, P->getStart(i)); }

void Solver::createDistanceTable()
{
  for (int i = 0; i < P->getNum(); ++i) {
    // breadth first search
    std::queue<Node*> OPEN;
    Node* n = P->getGoal(i);
    OPEN.push(n);
    distance_table[i][n->id] = 0;
    while (!OPEN.empty()) {
      n = OPEN.front();
      OPEN.pop();
      const int d_n = distance_table[i][n->id];
      for (auto m : n->neighbor) {
        const int d_m = distance_table[i][m->id];
        if (d_n + 1 >= d_m) continue;
        distance_table[i][m->id] = d_n + 1;
        OPEN.push(m);
      }
    }
  }
}

#pragma once
#include <getopt.h>

#include <functional>
#include <memory>
#include <queue>
#include <unordered_map>
#include <functional>

#include "problem.hpp"
#include "util.hpp"

class MinimumSolver
{
protected:
  std::string solver_name;       // solver name
  Problem* const P;              // problem instance
  Graph* const G;                // graph
  std::mt19937* const MT;        // for randomness
  const int max_comp_time;       // time limit for computation, ms
  Plan solution;                 // solution
  bool solved;                   // success -> true, failed -> false (default)

private:
  int comp_time;             // computation time
  Time::time_point t_start;  // when to start solving

public:
  void solve();  // call start -> run -> end
private:
  void start();
  void end();
protected:
  virtual void exec() {};    // main

public:
  MinimumSolver(Problem* _P);
  virtual ~MinimumSolver() {};

  // getter
  Plan getSolution() const { return solution; };
  bool succeed() const { return solved; };
  std::string getSolverName() const { return solver_name; };
  int getCompTime() const { return comp_time; }
  int getSolverElapsedTime() const;  // get elapsed time from start
};

// -----------------------------------------------
// base class with utilities
// -----------------------------------------------
class Solver : public MinimumSolver
{
private:
  // useful info
  bool verbose;      // true -> print additional info

  // distance to goal
protected:
  using DistanceTable = std::vector<std::vector<int>>;  // [agent][node_id]
  DistanceTable distance_table;     // distance table

  // -------------------------------
  // main
private:
  void exec();
protected:
  virtual void run() {} // main

  // -------------------------------
  // utilities for time
public:
  int getRemainedTime() const;       // get remained time
  bool overCompTime() const;         // check time limit

  // -------------------------------
  // utilities for debug
protected:
  // print debug info (only when verbose=true)
  void info() const;
  template <class Head, class... Tail> void info(Head&& head, Tail&&... tail) const
  {
    if (!verbose) return;
    std::cout << head << " ";
    info(std::forward<Tail>(tail)...);
  }
  void halt(const std::string& msg) const;  // halt program
  void warn(const std::string& msg) const;  // just printing msg

  // -------------------------------
  // log
public:
  virtual void makeLog(const std::string& logfile = DEFAULT_OUTPUT_FILE);
protected:
  void makeLogBasicInfo(std::ofstream& log);
  void makeLogSolution(std::ofstream& log);

  // -------------------------------
  // utilities for solver options
public:
  virtual void setParams(int argc, char* argv[]){};
  void setVerbose(bool _verbose) { verbose = _verbose; }

  // -------------------------------
  // print
public:
  void printResult();
protected:
  static void printHelpWithoutOption(const std::string& solver_name);

  // -------------------------------
  // utilities for distance
public:
  int pathDist(const int i, Node* const s) const;  // get path distance between s -> g_i
  int pathDist(const int i) const;                 // get path distance between s_i -> g_i
  void createDistanceTable();                      // compute distance table
  // use grid-pathfinding
  int pathDist(Node* const s, Node* const g) const { return G->pathDist(s, g); }


  // -------------------------------
  // utilities for getting path
public:
  // use grid-pathfinding
  Path getPath(Node* const s, Node* const g, bool cache=false) const { return G->getPath(s, g, cache); }

public:
  Solver(Problem* _P);
  virtual ~Solver();
};

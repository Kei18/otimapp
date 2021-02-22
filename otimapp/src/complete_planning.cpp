#include "../include/complete_planning.hpp"
#include "../include/cycle_candidate.hpp"


const std::string CompletePlanning::SOLVER_NAME = "CompletePlanning";


CompletePlanning::CompletePlanning(Problem* _P)
  : Solver(_P)
{
  solver_name = SOLVER_NAME;
}

CompletePlanning::~CompletePlanning()
{
}

void CompletePlanning::run()
{
  // set objective function
  auto compare = [] (HighLevelNode_p a, HighLevelNode_p b) {
    if (a->f != b->f) return a->f > b->f;
    return false;
  };

  // OPEN
  std::priority_queue<HighLevelNode_p, HighLevelNodes, decltype(compare)> Tree(compare);

  // initial node
  auto n = getInitialNode();
  if (!n->valid) {
    info("  ", "failed to find a path");
    return;
  }
  Tree.push(n);

  // start high-level search
  int h_node_num = 1;
  int iteration = 0;
  while (!Tree.empty()) {
    ++iteration;
    // check limitation
    if (overCompTime()) {
      info(" ", "timeout");
      break;
    }

    n = Tree.top();
    Tree.pop();

    info(" ", "elapsed:", getSolverElapsedTime(),
         ", explored_node_num:", iteration, ", nodes_num:", h_node_num,
         ", constraints:", n->constraints.size(),
         ", f:", n->f);

    // check conflict
    auto constraints = getConstraints(n->paths);
    if (constraints.empty()) {
      solved = true;
      break;
    }

    // create new nodes
    for (auto c : constraints) {
      auto m = invoke(n, c);
      if (m->valid) {
        Tree.push(m);
        ++h_node_num;
      }
    }
  }

  if (solved) solution = n->paths;
}

CompletePlanning::HighLevelNode_p CompletePlanning::getInitialNode()
{
  TableCycle table(G->getNodesSize());
  auto n = std::make_shared<HighLevelNode>();
  for (int i = 0; i < P->getNum(); ++i) {
    auto p = getPrioritizedPath(i, n->paths, table);
    if (p.empty()) {
      p = getConstrainedPath(i, n);
      if (p.empty()) {
        n->valid = false;
        break;
      }
    }
    n->paths.push_back(p);
    table.registerNewPath(i, p, true);
  }
  n->f = countsSwapConlicts(n->paths);
  return n;
}

CompletePlanning::HighLevelNode_p CompletePlanning::invoke(HighLevelNode_p n, Constraint_p c)
{
  auto new_constraints = n->constraints;
  new_constraints.push_back(c);
  auto paths = n->paths;
  paths[c->agent] = getConstrainedPath(c->agent, n);
  bool valid = !paths[c->agent].empty();
  int f = countsSwapConlicts(c->agent, n->f, n->paths, paths[c->agent]);
  return std::make_shared<HighLevelNode>(paths, new_constraints, f, valid);
}

Path CompletePlanning::getConstrainedPath(const int id, HighLevelNode_p node)
{
  Node* const g = P->getGoal(id);

  Constraints constraints;
  for (auto c : node->constraints) {
    if (c->agent == id) constraints.push_back(c);
  }
  auto checkInvalidMove = [&](Node* child, Node* parent) {
    // condition 1, avoid goals
    if (child != g && table_goals[child->id]) return true;
    // condition 2, follow limitation
    for (auto c : constraints) {
      if (c->child  == child && c->parent == parent) return true;
    }
    return false;
  };

  // create heuristics
  std::vector<std::vector<int>> from_to_table(G->getNodesSize());
  for (int i = 0; i < (int)node->paths.size(); ++i) {
    if (i == id) continue;
    auto p = node->paths[i];
    for (int t = 1; t < (int)p.size(); ++t) {
      from_to_table[p[t-1]->id].push_back(p[t]->id);
    }
  }

  auto compare = [&](AstarNode* a, AstarNode* b) {
    // greedy search
    if (pathDist(id, a->v) != pathDist(id, b->v)) return pathDist(id, a->v) > pathDist(id, b->v);
    // tie break
    auto table_a = from_to_table[a->p->v->id];
    auto table_b = from_to_table[b->p->v->id];
    bool swap_a = std::find(table_a.begin(), table_a.end(), a->v->id) != table_a.end();
    bool swap_b = std::find(table_b.begin(), table_b.end(), b->v->id) != table_b.end();
    if (swap_a != swap_b) return (int)swap_a < (int)swap_b;
    if (a->g != b->g) return a->g < b->g;
    return false;
  };

  return Solver::getPath(id, checkInvalidMove, compare);
}

CompletePlanning::Constraints CompletePlanning::getConstraints(const Plan& paths) const
{
  Constraints constraints = {};
  TableCycle table(G->getNodesSize());

  // main loop
  for (int i = 0; i < P->getNum(); ++i) {
    auto c = table.registerNewPath(i, paths[i]);
    if (c != nullptr) {
      // create constraints
      for (int i = 0; i < c->agents.size(); ++i) {
        constraints.push_back(std::make_shared<Constraint>(c->agents[i], c->path[i], c->path[i+1]));
      }
      break;
    }
  }

  return constraints;
}

int CompletePlanning::countsSwapConlicts(const Plan& paths)
{
  // TODO: develop efficient counts method
  int cnt = 0;
  const int num_agents = paths.size();
  for (int i = 0; i < num_agents; ++i) {
    for (int j = i + 1; j < num_agents; ++j) {
      for (int k = 1; k < (int)paths[i].size(); ++k) {
        for (int l = 1; l < (int)paths[j].size(); ++l) {
          if (paths[i][k-1] == paths[j][l] && paths[i][k] == paths[j][l-1]) ++cnt;
        }
      }
    }
  }
  return cnt;
}

int CompletePlanning::countsSwapConlicts
(const int id, const int old_f_val, const Plan& old_paths, const Path& new_path)
{
  int cnt = old_f_val;
  const int num_agents = old_paths.size();

  // count old_conflicts
  for (int j = 0; j < num_agents; ++j) {
    if (j == id) continue;
    for (int l = 1; l < (int)old_paths[j].size(); ++l) {
      for (int k = 1; k < (int)old_paths[id].size(); ++k) {
        if (old_paths[id][k-1] == old_paths[j][l] && old_paths[id][k] == old_paths[j][l-1]) --cnt;
      }
      for (int k = 1; k < (int)new_path.size(); ++k) {
        if (new_path[k-1] == old_paths[j][l] && new_path[k] == old_paths[j][l-1]) ++cnt;
      }
    }
  }

  return cnt;
}

void CompletePlanning::setParams(int argc, char* argv[])
{
}

void CompletePlanning::printHelp()
{
  printHelpWithoutOption(SOLVER_NAME);
}

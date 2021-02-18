#include "../include/complete_planning.hpp"


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
  auto n = std::make_shared<HighLevelNode>();
  for (int i = 0; i < P->getNum(); ++i) {
    n->paths.push_back(getConstrainedPath(i, n));
    if (n->paths[i].empty()) {
      n->valid = false;
      break;
    }
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

  auto compare = [&from_to_table](AstarNode* a, AstarNode* b) {
    if (a->f != b->f) return a->f > b->f;
    // tie break
    auto table_a = from_to_table[a->p->v->id];
    auto table_b = from_to_table[a->p->v->id];
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

  Path path_until_t_minus_2;  // path[0] ... path[t-2]

  struct CycleCandidate {
    std::deque<Node*> path;
    std::deque<int> agents;

    CycleCandidate() {}
  };
  using CycleCandidate_p = std::shared_ptr<CycleCandidate>;
  std::vector<std::vector<CycleCandidate_p>> table_cycle_tail(G->getNodesSize());
  std::vector<std::vector<CycleCandidate_p>> table_cycle_head(G->getNodesSize());

  // check duplication
  auto existDuplication = [&] (Node* head, Node* tail)
  {
    auto cycles = table_cycle_head[head->id];
    return
      std::find_if(cycles.begin(), cycles.end(),
                   [&tail] (CycleCandidate_p c)
                   { return c->path.back() == tail; }) != cycles.end();
  };

  // create new entry
  auto createNewCycleCandidate = [&] (int id, Node* head, CycleCandidate_p c_base, Node* tail)
  {
    // create new entry
    auto c = std::make_shared<CycleCandidate>();

    // agent
    if (c_base != nullptr) c->agents = c_base->agents;
    if (tail != nullptr) {
      c->agents.push_back(id);
    } else {  // head != nullptr
      c->agents.push_front(id);
    }

    // path
    if ((c_base == nullptr || head != c_base->path.front()) && head != nullptr)
      c->path.push_back(head);
    if (c_base != nullptr)
      for (auto itr = c_base->path.begin(); itr != c_base->path.end(); ++itr) c->path.push_back(*itr);
    if ((c_base == nullptr || tail != c_base->path.back()) && tail != nullptr)
      c->path.push_back(tail);

    // register
    table_cycle_head[c->path.front()->id].push_back(c);
    table_cycle_tail[c->path.back()->id].push_back(c);

    // detect deadlock
    if (c->path.front() == c->path.back()) {
      // create constraints
      for (int i = 0; i < c->agents.size(); ++i) {
        constraints.push_back(std::make_shared<Constraint>(c->agents[i], c->path[i], c->path[i+1]));
      }
    }
  };

  // avoid loop with own path
  auto usingOwnPath = [&] (CycleCandidate_p c)
  {
    for (auto itr = c->path.begin(); itr != c->path.end(); ++itr) {
      if (inArray(*itr, path_until_t_minus_2)) return true;
    }
    return false;
  };


  for (int i = 0; i < P->getNum(); ++i) {
    auto path = paths[i];

    // update cycles step by step
    for (int t = 1; t < (int)path.size(); ++t) {
      auto v_before = path[t-1];
      auto v_next = path[t];

      // update part of path
      if (t >= 2) path_until_t_minus_2.push_back(path[t-2]);

      if (!existDuplication(v_before, v_next)) {
        createNewCycleCandidate(i, v_before, nullptr, v_next);
        if (!constraints.empty()) return constraints;
      }

      // check existing cycle, tail
      if (!table_cycle_tail[v_before->id].empty()) {
        for (auto c : table_cycle_tail[v_before->id]) {
          if (existDuplication(c->path.front(), v_next)) continue;
          if (usingOwnPath(c)) continue;
          createNewCycleCandidate(i, nullptr, c, v_next);
          if (!constraints.empty()) return constraints;
        }
      }

      // check existing cycle, head
      if (!table_cycle_head[v_next->id].empty()) {
        for (auto c : table_cycle_head[v_next->id]) {
          if (existDuplication(v_before, c->path.back())) continue;
          if (usingOwnPath(c)) continue;
          createNewCycleCandidate(i, v_before, c, nullptr);
          if (!constraints.empty()) return constraints;
        }
      }
    }

    path_until_t_minus_2.clear();
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

// -------------------------------
// helper functions
// ------------------------------
// check whether two configurations are the same
[[maybe_unused]]
static bool sameConfig(const Config& config_i, const Config& config_j)
{
  if (config_i.size() != config_j.size()) return false;
  const int size_i = config_i.size();
  for (int k = 0; k < size_i; ++k) {
    if (config_i[k] != config_j[k]) return false;
  }
  return true;
}

// makespan
[[maybe_unused]]
static int getMakespan(const Configs& result)
{
  return !result.empty() ? result.size() - 1 : 0;
}

// sum-of-costs
[[maybe_unused]]
static int getSOC(const Configs& result)
{
  if (result.empty()) return 0;

  int soc = 0;
  const int makespan = getMakespan(result);
  const int num_agents = result[0].size();
  for (int i = 0; i < num_agents; ++i) {
    int c = makespan;
    auto g = result[makespan][i];
    while (c > 0 && result[c-1][i] == g) --c;
    soc += c;
  }
  return soc;
}

// validate the results
[[maybe_unused]]
static bool validateMAPFPlan(const Configs& configs, Problem* P)
{
  if (configs.empty()) return false;
  const int makespan = getMakespan(configs);

  // start and goal
  if (!sameConfig(P->getConfigStart(), configs[0])) return false;
  if (!sameConfig(P->getConfigGoal(), configs[makespan])) return false;

  // check conflicts and continuity
  int num_agents = configs[0].size();
  for (int t = 1; t <= makespan; ++t) {
    if ((int)configs[t].size() != num_agents) return false;
    for (int i = 0; i < num_agents; ++i) {
      Node* v_i_t = configs[t][i];
      Node* v_i_t_1 = configs[t-1][i];
      Nodes cands = v_i_t_1->neighbor;
      cands.push_back(v_i_t_1);
      if (!inArray(v_i_t, cands)) return false;
      // see conflicts
      for (int j = i + 1; j < num_agents; ++j) {
        Node* v_j_t = configs[t][j];
        Node* v_j_t_1 = configs[t-1][j];
        if (v_i_t == v_j_t) return false;
        if (v_i_t == v_j_t_1 && v_i_t_1 == v_j_t) return false;
      }
    }
  }
  return true;
}

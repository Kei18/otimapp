#pragma once
#include <random>
#include <graph.hpp>
#include "util.hpp"

// ER-model
class RandomGraph : public Graph
{
private:
  const int nodes_size;
  const float prob;

public:
  RandomGraph(int _nodes_size, float _prob, int seed=0)
    : nodes_size(_nodes_size), prob(_prob)
  {
    auto MT = std::mt19937(seed);
    // create nodes
    for (int i = 0; i < nodes_size; ++i) {
      V.push_back(new Node(i, 0, 0));
    }
    // create edge
    for (int i = 0; i < nodes_size; ++i) {
      for (int j = i + 1; j < nodes_size; ++j) {
        if (getRandomFloat(0, 1, &MT) < prob) {
          V[i]->neighbor.push_back(V[j]);
          V[j]->neighbor.push_back(V[i]);
        }
      }
    }
  };
  ~RandomGraph(){};

  Node* getNode(int i) const { return V[i]; };
};

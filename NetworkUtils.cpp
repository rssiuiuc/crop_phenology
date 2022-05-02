/*
 * NetworkUtils.cpp
 *
 *  Author: RSSI (rssiuiuc@gmail.com)
 */

#include "NetworkUtils.h"

#include <queue>
#include <stack>

using simple_graph::Network;

namespace simple_graph {
namespace utils {

float GetClusteringCoefficient(const Network &network, std::size_t node_id) {
	if (network.Size() <= 2)
		return 0;
	std::size_t degree = network.GetDegree(node_id);
	if (degree < 2)
		return 0;

	std::vector<std::size_t> neighbors = network.GetNeighbors(node_id);
	int edge_count = 0;
	for (std::size_t i = 0; i < degree; ++i) {
		// For simple graphs, we only consider undirected ones.
		for (std::size_t j = i + 1; j < degree; ++j) {
			if (neighbors[j] != node_id
					&& network.IsEdge(neighbors[i], neighbors[j]))
				++edge_count;
		}
	}
	return static_cast<float>(edge_count) * 2.0
			/ static_cast<float>(degree * (degree - 1));
}

std::vector<float> GetNodeBetweennessCentrality(const Network &network) {
	std::size_t network_size = network.Size();
	std::vector<float> betweenness(network_size, 0.0);

	for (std::size_t i = 0; i < network_size; ++i) {
		// Nodes are stored in the reverse order in which they are visited.
		std::stack<std::size_t> stack;
		// Stores the predecessors of each node.
		std::vector<std::vector<std::size_t>> pre_visit_list(network_size,
				std::vector<std::size_t>());
		// A queue for breath first search.
		std::queue<std::size_t> queue;
		// Distance from each node to the current node (i). The default is
		// set to infinity (-1).
		std::vector<long> distance(network_size, -1);
		// The number of shortest paths passing through the nodes.
		std::vector<long> num_path(network_size, 0);

		distance[i] = 0;
		num_path[i] = 1;
		queue.push(i);
		while (!queue.empty()) {
			const auto node = queue.front();
			queue.pop();
			stack.push(node);

			std::vector<std::size_t> neighbors = network.GetNeighbors(node);
			for (auto neighbor : neighbors) {
				if (distance[neighbor] < 0) {
					queue.push(neighbor);
					distance[neighbor] = distance[node] + 1;
				}

				if (distance[neighbor] == (distance[node] + 1)) {
					num_path[neighbor] += num_path[node];
					pre_visit_list.at(neighbor).push_back(node);
				}
			}
		}

		std::vector<float> dependency(network_size, 0);
		while (!stack.empty()) {
			const auto cur = stack.top();
			stack.pop();

			for (auto node : pre_visit_list[cur]) {
				float partial_dep = static_cast<float>(num_path[node])
						/ num_path[cur] * (1 + dependency[cur]);
				dependency[node] += partial_dep;
			}
			if (cur != i)
				betweenness[cur] += dependency[cur];
		}
	}
	for (std::size_t i = 0; i < network_size; ++i) {
		betweenness[i] /= (network_size - 1) * (network_size - 2);
	}
	return betweenness;
}

std::vector<std::vector<std::size_t>> ExtractConnectedComponents(
		const Network &network) {
	std::vector<std::vector<std::size_t>> components;

	std::size_t network_size = network.Size();
	std::vector<bool> visited(network_size, false);

	for (std::size_t i = 0; i < network_size; ++i) {
		if (visited[i])
			continue;
		std::queue<std::size_t> queue;
		std::vector<std::size_t> component;
		queue.push(i);
		visited[i] = true;
		while (!queue.empty()) {
			std::size_t current = queue.front();
			queue.pop();
			component.push_back(current);
			std::vector<std::size_t> neighbors = network.GetNeighbors(current);
			for (auto neighbor : neighbors) {
				if (!visited[neighbor]) {
					queue.push(neighbor);
					visited[neighbor] = true;
				}
			}
		}
		components.push_back(component);
	}
	return components;
}

std::vector<std::size_t> ExtractGiantComponent(const Network &network) {
	std::vector<std::vector<std::size_t>> components =
			ExtractConnectedComponents(network);
	if (components.empty())
		return std::vector<std::size_t>();

	std::size_t giant_size = 0, giant_index = 0;
	for (std::size_t i = 1; i < components.size(); ++i) {
		if (components.at(i).size() > giant_size) {
			giant_size = components.at(i).size();
			giant_index = i;
		}
	}

	return components[giant_index];
}

UnionFind::UnionFind(std::size_t size) {
  parents_.resize(size);
  component_size_.resize(size);
  for (std::size_t i = 0; i < size; ++i) {
    parents_[i] = i;
    component_size_[i] = 1;
  }
  giant_root_ = size == 0 ? size : 0;
}

std::size_t UnionFind::FindRoot(std::size_t node) {
	if (!ValidateNode(node)) {
		return Size();
	}
	while (parents_[node] != node) {
		std::size_t pre = parents_[node];
		parents_[node] = parents_[pre];
		node = pre;
	}
	return node;
}

void UnionFind::Union(std::size_t node1, std::size_t node2) {
	if (!ValidateNode(node1) || !ValidateNode(node2))
		return;
	std::size_t root1 = FindRoot(node1), root2 = FindRoot(node2);
	if (root1 == root2)
		return;
	if (component_size_[root1] < component_size_[root2]) {
		component_size_[root2] += component_size_[root1];
		parents_[root1] = root2;
		if (giant_root_ == Size()
				|| component_size_[root2] > component_size_[giant_root_]) {
			giant_root_ = root2;
		}
	} else {
		component_size_[root1] += component_size_[root2];
		parents_[root2] = root1;
		if (giant_root_ == Size()
				|| component_size_[root1] > component_size_[giant_root_]) {
			giant_root_ = root1;
		}
	}
}

std::size_t UnionFind::GiantComponentSize() const {
	return giant_root_ == Size() ? 0 : component_size_[giant_root_];
}

} /* namespace utils */
} /* namespace simple_graph */

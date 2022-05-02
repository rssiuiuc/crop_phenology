/*
 * Network.cpp
 *
 *  Author: RSSI (rssiuiuc@gmail.com)
 */

#include "Network.h"
#include <iostream>
namespace simple_graph {

Network::Network(std::size_t size) {
	edges_.resize(size);
}

Network::~Network() {
}

void Network::AddOrUpdateEdge(std::size_t node_1, std::size_t node_2,
		float weight) {
	if (ValidateNodeId(node_1) && ValidateNodeId(node_2)) {
		edges_[node_1][node_2] = weight;
		edges_[node_2][node_1] = weight;
	}
}

bool Network::IsEdge(std::size_t node_1, std::size_t node_2) const {
	if (ValidateNodeId(node_1) && ValidateNodeId(node_2)) {
		return edges_.at(node_1).count(node_2);
	}
	return false;
}

std::vector<std::size_t> Network::GetNeighbors(std::size_t node_id) const {
	std::vector<std::size_t> neighbors;
	if (!ValidateNodeId(node_id))
		return neighbors;
	for (const auto edge : edges_.at(node_id)) {
		neighbors.push_back(edge.first);
	}
	return neighbors;
}

std::size_t Network::GetDegree(std::size_t node_id) const {
	if (!ValidateNodeId(node_id))
		return 0;
	return edges_.at(node_id).size();
}

bool Network::ValidateNodeId(std::size_t node_id) const {
	if (node_id >= Size()) {
		std::cerr << "invalid node_id: " << node_id << " v.s. network size: "
				<< Size() << std::endl;
		return false;
	}
	return true;
}
} /* namespace simple_graph */

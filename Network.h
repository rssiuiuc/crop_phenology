/*
 * Network.h
 *
 *  Author: RSSI (rssiuiuc@gmail.com)
 */

#ifndef SIMPLEGRAPH_NETWORK_H_
#define SIMPLEGRAPH_NETWORK_H_

#include <cstddef>
#include <vector>
#include <unordered_map>

namespace simple_graph {

/*
 * A simple undirected graph. Nodes are referenced by their node_id (a.k.a. 0 based
 * index).
 */
class Network {
public:
	Network(std::size_t size);

	virtual ~Network();

	inline bool IsEmpty() const {
		return edges_.empty();
	}

	inline std::size_t Size() const {
		return edges_.size();
	}

	// Set the edge weight between nodes.
	// Node: this will overwrite the weight of existing edges.
	void AddOrUpdateEdge(std::size_t node_1, std::size_t node_2, float weight);

	// Check if there is an edge between two nodes.
	bool IsEdge(std::size_t node_1, std::size_t node_2) const;

	std::vector<std::size_t> GetNeighbors(std::size_t node_id) const;

	std::size_t GetDegree(std::size_t node_id) const;

private:
	// Stores the edges using an adjacency list, where edges are represented
	// as <node_id, weight> pairs
	std::vector<std::unordered_map<std::size_t, float>> edges_;

	bool ValidateNodeId(std::size_t node_id) const;

};

} /* namespace simple_graph */

#endif /* SIMPLEGRAPH_NETWORK_H_ */

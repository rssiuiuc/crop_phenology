/*
 * NetworkUtils.h
 *
 *  Author: RSSI (rssiuiuc@gmail.com)
 */

#ifndef SIMPLEGRAPH_NETWORKUTILS_H_
#define SIMPLEGRAPH_NETWORKUTILS_H_

#include "Network.h"

#include <vector>

namespace simple_graph {
namespace utils {

float GetClusteringCoefficient(const simple_graph::Network &network,
		std::size_t node_id);

// Returns a vector of the node betweenness centrality. The order is the
// same as the node_id.
std::vector<float> GetNodeBetweennessCentrality(
		const simple_graph::Network &network);

// Returns a lit of nodes in the connected components.
std::vector<std::vector<std::size_t>> ExtractConnectedComponents(
		const simple_graph::Network &network);

// Returns a lit of nodes in the giant component (a.k.a. the largest
// connected component).
std::vector<std::size_t> ExtractGiantComponent(
		const simple_graph::Network &network);

// Returns the index of the element with the largest moving average.
template<typename T>
std::size_t FindMaxValueIndexMovingAverage(const std::vector<T> &values,
		std::size_t window_size);

class UnionFind {
public:
	UnionFind(std::size_t size);
	~UnionFind() {
	}
	void Union(std::size_t node1, std::size_t node2);
	inline bool IsConnected(std::size_t node1, std::size_t node2) {
		return ValidateNode(node1) && (FindRoot(node1) == FindRoot(node2));
	}
	inline std::size_t Size() const {
		return parents_.size();
	}
	std::size_t GiantComponentSize() const;
private:
	std::size_t giant_root_;
	std::vector<std::size_t> parents_;
	std::vector<std::size_t> component_size_;

	inline bool ValidateNode(std::size_t node) const {
		return node < Size();
	}
	// Return the root of node. If node is not valid (> the size of the
	// structure, an arbitrary value (>= size of the structure) will be
	// returned.
	std::size_t FindRoot(std::size_t node);
};

} /* namespace utils */
} /* namespace simple_graph */

#endif /* SIMPLEGRAPH_NETWORKUTILS_H_ */

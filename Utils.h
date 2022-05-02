/*
 * Utils.h
 *
 *  Author: RSSI (rssiuiuc@gmail.com)
 */

#ifndef SIMPLEGRAPH_PHENONET_UTILS_H_
#define SIMPLEGRAPH_PHENONET_UTILS_H_

#include <math.h>
#include <vector>

namespace remote_sensing {
namespace utils {

constexpr float EPSILON = 1e-6;

struct DecompositionSchema {
	// Each task handles data positioned at displacements[rank] (relative
	// to the global send buffer, with a size of counts[rank]. The size of
	// counts and displacements must equal to the size of the number of
	// tasks in the pool.
	std::vector<int> counts;
	std::vector<int> displacements;
	int pool_size;
	int root;

	DecompositionSchema(int num_tasks, int root_task) :
			pool_size(num_tasks), root(root_task) {
	}
};

template<typename T>
float SimilarityCosine(const std::vector<T> &v1, const std::vector<T> &v2) {
	float ret = 0, sum1 = 0, sum2 = 0;
	if (v1.size() != v2.size()) {
		// The two vectors need to have the same dimension.
		return 0;
	}

	for (std::size_t i = 0; i < v1.size(); i++) {
		sum1 += v1[i] * v1[i];
		sum2 += v2[i] * v2[i];
		ret += v1[i] * v2[i];
	}
	if (sum1 <= EPSILON || sum2 <= EPSILON)
		return 0.0;
	return ret / sqrt(sum1) / sqrt(sum2);
}

// Finds the element in the given range [start_pos, end_pos) that has the
// largest moving average. Returns an out of range index on errors.
int FindMaxValueIndexMovingAverage(const std::vector<float> &values,
		std::size_t window_size, std::size_t start_pos, std::size_t end_pos,
		float min_value, float max_value);

} /* namespace utils */
} /* namespace remote_sensing */

#endif /* SIMPLEGRAPH_PHENONET_UTILS_H_ */

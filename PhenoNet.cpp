/*
 * PhenoNet.cpp
 *
 *  Author: RSSI (rssiuiuc@gmail.com)
 */

#include "PhenoNet.h"

#include "Utils.h"
#include "NetworkUtils.h"

#include <mpi.h>
#include <climits>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

using simple_graph::Network;

namespace remote_sensing {

PhenoNet::PhenoNet(std::vector<TimeSeries<float>> &&pixel_time_series,
		float min_giant_component_fraction) :
		time_series_data_(std::move(pixel_time_series)), moving_window_size_(5) {

	start_time_.resize(time_series_data_.size(), 0);
	end_time_.resize(time_series_data_.size(), 0);
	for (std::size_t i = 0; i < time_series_data_.size(); ++i) {
		end_time_[i] = time_series_data_[i].GetNumTimeSlices();
	}
	if (!time_series_data_.empty()) {
		min_valid_time_range_ = 0;
		min_giant_component_size_ =
				static_cast<std::size_t>(time_series_data_[0].GetNumTimeSlices()
						* min_giant_component_fraction);
	} else {
		min_valid_time_range_ = INT_MAX;
		min_giant_component_size_ = INT_MAX;
	}
}

PhenoNet::~PhenoNet() {
}

Network PhenoNet::BuildPhenoNetworkByGiantComponentSize(
		const TimeSeries<float> &time_series, int start_time, int end_time,
		std::size_t min_giant_component_size) {
	std::size_t num_time_slices = time_series.GetNumTimeSlices();
	// Collects and sorts edges based on their weights.
	std::vector<std::pair<std::pair<int, int>, float>> edges;
	std::unordered_set<int> connected_nodes;
	for (int i = start_time; i < end_time; ++i) {
		for (int j = i + 1; j < end_time; ++j) {
			const auto* slice1 = time_series.GetTimeSlice(i);
			const auto* slice2 = time_series.GetTimeSlice(j);
			if (slice1 == nullptr || slice2 == nullptr) {
				continue;
			}
			float weight = utils::SimilarityCosine<float>(*slice1, *slice2);
			if (weight >= utils::EPSILON) {
				// Skips small values for performance optimization
				edges.push_back( { { i, j }, weight });
				connected_nodes.insert(i);
				connected_nodes.insert(j);
			}
		}
	}
	if (connected_nodes.size() < min_giant_component_size) {
		// Returns an empty network since the min_giant_component_size cannot
		// be met.
		return Network(0);
	}
	std::sort(edges.begin(), edges.end(),
			[](const std::pair<std::pair<int, int>, float> &edge1,
					const std::pair<std::pair<int, int>, float> &edge2) {
				return edge1.second > edge2.second;
			});
	// Adds the edges to the pheno net based on their weights in descending
	// order, until the desired minimum giant component size is reached.
	Network net(num_time_slices);
	simple_graph::utils::UnionFind uf(num_time_slices);
	for (const auto &edge : edges) {
		if (uf.GiantComponentSize() >= min_giant_component_size) {
			break;
		}
		const int node1 = edge.first.first;
		const int node2 = edge.first.second;
		net.AddOrUpdateEdge(node1, node2, edge.second);
		uf.Union(node1, node2);
	}
	return uf.GiantComponentSize() >= min_giant_component_size ?
			net : Network(0);
}

bool PhenoNet::FindPeak(const TimeSeries<float> &time_series, int start_time,
		int end_time, std::size_t min_giant_component_size,
		int &time_slice_index, float &bridging_coefficient) {
	const Network pheno_net = BuildPhenoNetworkByGiantComponentSize(time_series,
			start_time, end_time, min_giant_component_size);
	if (pheno_net.IsEmpty()) {
		std::clog<<"The pheno network is too fragmented to meet the given "
				<<"requirement\n";
		return false;
	}
	const std::vector<std::size_t> giant_component =
			simple_graph::utils::ExtractGiantComponent(pheno_net);
	const std::unordered_set<std::size_t> giant_nodes(giant_component.begin(),
			giant_component.end());
	std::vector<float> node_measures =
			simple_graph::utils::GetNodeBetweennessCentrality(pheno_net);
	for (std::size_t i = 0; i < pheno_net.Size(); ++i) {
		if (giant_nodes.count(i) == 0) {
			// Only considers nodes in the giant component (to exclude
			// outliers).
			node_measures[i] = 0;
			break;
		}
		float clustering_coefficient =
				simple_graph::utils::GetClusteringCoefficient(pheno_net, i);
		if (clustering_coefficient < utils::EPSILON) {
			node_measures[i] = 0;
		} else {
			node_measures[i] /= clustering_coefficient;
		}
	}
	time_slice_index = utils::FindMaxValueIndexMovingAverage(
			node_measures, moving_window_size_, start_time, end_time, /* min_value = */
			0, /* max_value = */INT_MAX);
	if (time_slice_index < 0
			|| time_slice_index
					>= static_cast<int>(time_series.GetNumTimeSlices())) {
		return false;
	}
	bridging_coefficient = node_measures[time_slice_index];
	return true;
}

void PhenoNet::Process() {
	std::size_t num_pixels = time_series_data_.size();
	peak_index_.resize(num_pixels, INT_MAX);
	for (std::size_t i = 0; i < num_pixels; ++i) {
		int peak_index = -1;
		float measure = 0;
		if (FindPeak(time_series_data_[i], start_time_[i], end_time_[i],
				min_giant_component_size_, peak_index, measure)) {
			peak_index_[i] = peak_index;
		}
	}
}

} /* namespace remote_sensing */

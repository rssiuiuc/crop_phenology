/*
 * PhenoNet.h
 *
 *  Author: RSSI (rssiuiuc@gmail.com)
 */

#ifndef SIMPLEGRAPH_PHENONET_PHENONET_H_
#define SIMPLEGRAPH_PHENONET_PHENONET_H_

#include "TimeSeries.h"
#include "Network.h"

#include <vector>

namespace remote_sensing {

class PhenoNet {
public:
	PhenoNet(std::vector<TimeSeries<float>> &&pixel_time_series,
			float min_giant_component_fraction);

	virtual ~PhenoNet();
	PhenoNet(const PhenoNet &other) = delete;
	PhenoNet& operator=(const PhenoNet &other) = delete;

	void Process();
	std::vector<int> GetPeakTimeSliceIndex() const {
		return peak_index_;
	}

private:
	std::vector<TimeSeries<float>> time_series_data_;
	// The ranges of the time series that will be considered for
	// calculations. end_time_ is exclusive. The default is all time
	// slices.
	std::vector<int> start_time_, end_time_;
	// This is an optimization option. If the connected nodes (degree >= 1)
	// within the pheno net is below this range, the pheno net will be
	// considered invalid (no sufficient data) and skipped.
	int min_valid_time_range_;
	// A pheno net is considered valid IFF its giant component is at least
	// this size.
	std::size_t min_giant_component_size_;
	// The size of the moving window that is used to select the peak
	// node from the pheno network.
	std::size_t moving_window_size_;
	// The index of the peak nodes (of the time slices).
	std::vector<int> peak_index_;

	// Builds a pheno network. Iteratively connect nodes based on their cosine
	// similarity from high (most similar) to low (least similar).
	// Once the giant component reaches the desired size
	// (min_gaint_component_size), the connection stops, i.e. the least
	// similar nodes are not connected in the network.
	// Returns an empty network if the requirement cannot be met.
	simple_graph::Network BuildPhenoNetworkByGiantComponentSize(
			const TimeSeries<float> &time_series, int start_time, int end_time,
			std::size_t min_gaint_component_size);
	// Finds the peak (transition) point of the given time series. The peak
	// is selected as the node with the highest bridging coeficient. Returns
	// false if no algorithm defined peak could not found.
	bool FindPeak(const TimeSeries<float> &time_series, int start_time,
			int end_time, std::size_t min_gaint_component_size,
			int &time_slice_index, float &bridging_coefficient);
};

} /* namespace remote_sensing */

#endif /* SIMPLEGRAPH_PHENONET_PHENONET_H_ */

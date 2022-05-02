/*
 * TimeSeries.h
 *
 *  Author: RSSI (rssiuiuc@gmail.com)
 */

#ifndef SIMPLEGRAPH_PHENONET_TIMESERIES_H_
#define SIMPLEGRAPH_PHENONET_TIMESERIES_H_

#include <vector>

namespace remote_sensing {

// A time series contains a list of time slice,
// each of which includes values from multiple bands.
template<typename DataType>
class TimeSeries {
public:
	TimeSeries() {
	}

	~TimeSeries() {
	}

	inline std::size_t GetNumTimeSlices() const {
		return time_slices_.size();
	}

	// The number of bands in each time slice
	inline std::size_t GetTimeSliceDimension() const {
		return GetNumTimeSlices() > 0 ? time_slices_[0].size() : 0;
	}

	bool AddTimeSlice(const std::vector<DataType> &time_slice) {
		if (!ValidateTimeSlice(time_slice)) {
			return false;
		}
		time_slices_.push_back(time_slice);
		return true;
	}

	const std::vector<DataType>* GetTimeSlice(
			std::size_t time_slice_index) const {
		if (time_slice_index >= GetNumTimeSlices()) {
			return nullptr;
		}
		return &time_slices_[time_slice_index];
	}

private:
	std::vector<std::vector<DataType>> time_slices_;

	bool ValidateTimeSlice(const std::vector<DataType> &time_slice) const {
		if (!time_slices_.empty()
				&& time_slice.size() != time_slices_[0].size()) {
			// All time slices should have the same dimension.
			return false;
		}
		return true;
	}
};

} /* namespace remote_sensing */

template class remote_sensing::TimeSeries<int>;
template class remote_sensing::TimeSeries<float>;

#endif /* SIMPLEGRAPH_PHENONET_TIMESERIES_H_ */

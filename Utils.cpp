/*
 * Utils.cpp
 *
 *  Author: RSSI (rssiuiuc@gmail.com)
 */

#include "Utils.h"

namespace remote_sensing {
namespace utils {
int FindMaxValueIndexMovingAverage(const std::vector<float> &values,
		std::size_t window_size, std::size_t start_pos, std::size_t end_pos,
		float min_value, float max_value) {
	if (window_size > values.size() || window_size == 0) {
		return 0;
	}
	const int size = static_cast<int>(values.size());
	const int left_range = (window_size - 1) / 2;
	const int right_range = window_size - 1 - left_range;
	int count = 0;
	int max_index = -1;
	float sum = 0, max_mean = -1.0;
	for (int i = 0;; ++i) {
		// Considers i as the last element in the moving window. mid is the
		// center of the window.
		int mid = i - right_range;
		// low is the first element of the window.
		int low = mid - left_range;
		if (mid >= size) {
			break;
		}
		if (i < size && values[i] >= min_value && values[i] <= max_value) {
			// While the last element is in range (of the input values),
			// updates the sum and count of elements in the window.
			sum += values[i];
			++count;
		}
		// As the window moves forward by 1 step, removes the element low - 1
		// from the window.
		// Update the sum and count accordingly
		if (low > 0 && values[low - 1] >= min_value
				&& values[low - 1] <= max_value) {
			sum -= values[low - 1];
			--count;
		}
		// Calculates the average value in the current window
		if (mid >= 0) {
			float mean = count ? sum / count : 0;
			// Compares the current average value to the max value£¬
			// if within the range [start, end).
			if (mid >= static_cast<int>(start_pos)
					&& mid < static_cast<int>(end_pos) && mean > max_mean) {
				max_mean = mean;
				max_index = mid;
			}
		}
	}
	return max_index;
}

}
}



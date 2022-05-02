/*
 * TimeSeriesDecompisition.h
 *
 *  Author: RSSI (rssiuiuc@gmail.com)
 */

#ifndef SIMPLEGRAPH_PHENONET_TIMESERIESDECOMPOSITION_H_
#define SIMPLEGRAPH_PHENONET_TIMESERIESDECOMPOSITION_H_

#include "Utils.h"
#include "TimeSeries.h"

#include <mpi.h>
#include <iostream>
#include <vector>
#include <type_traits>

namespace remote_sensing {

/*
 *
 */
  template <typename T>
    class TimeSeriesDecomposition {
  public:
  // This class assumes the following:
  // 1) Tasks may be assigned to distribute the data for a number of time
  //    slices. This information should be passed by
  //    time_slice_index_to_task;
  // 2) If a task is assigned to distribute the data for a given time slice,
  //    the task worker MUST contain all bands for the time slice;
  //    e.g. if the time series data contains 7 bands and task A is assigned
  //    to distribute the data for time slice #1, task A must contain all 7
  //    bands for time slice #1;
  // 3) The entire time series data should be stored in a 2d array,
  //    where the outer layer is indexed by the time slices, and the inner
  //    layer is indexed by the data bands. Each element should be a pointer
  //    that points to one matching band data (1d array) with num_pixels
  //    pixels.
  // 4) decomposition_schema specifies how the input data will be distributed
  //    to task workers;
  // 5) Once the class DistributeData(), each task will contain
  //    the full time series data for elements specified by
  //    decomposition_schema. e.g. pixels of range [xx, yy) in a scene of
  //    a satellite image.
 TimeSeriesDecomposition(const std::vector<int> &time_slice_index_to_task,
			 const std::vector<std::vector<T*>> &data, int num_bands,
			 int num_pixels,
			 utils::DecompositionSchema decomposition_schema,
			 int task_rank) :
  time_slice_index_to_task_(time_slice_index_to_task), data_(data), num_pixels_(
										num_pixels), num_bands_(num_bands), decomposition_schema_(
																	  decomposition_schema), rank_(task_rank) {
  }
  
  virtual ~TimeSeriesDecomposition() {
  }

  TimeSeriesDecomposition(const TimeSeriesDecomposition &other) = delete;
  TimeSeriesDecomposition(TimeSeriesDecomposition &&other) = delete;

  bool DistributeData() {
    // Validates the input data.
    if (data_.empty()) {
      if (rank_ == decomposition_schema_.root) {
	std::cerr << "No input data provided.\n";
      }
      return false;
    } else if (time_slice_index_to_task_.size() != data_.size()) {
      if (rank_ == decomposition_schema_.root) {
	std::cerr << "time_slice_index_to_task_ is not consistent with "
		  << "the bands: " << time_slice_index_to_task_.size()
		  << " v.s. " << data_.size() << std::endl;
      }
      return false;
    } else {
      for (int i = 0; i < decomposition_schema_.pool_size; ++i) {
	if (decomposition_schema_.displacements[i]
	    + decomposition_schema_.counts[i] > num_pixels_) {
	  if (rank_ == decomposition_schema_.root) {
	    std::cerr << "invalid decomposition_schema. The"
		      << "assigned data for task #" << i
		      << " will be out of bound: "
		      << (decomposition_schema_.displacements[i]
			  + decomposition_schema_.counts[i])
		      << " v.s. " << num_pixels_ << std::endl;
	  }
	  return false;
	}
      }
    }

    const int num_time_slices = static_cast<int>(data_.size());
    for (int i = 0; i < num_time_slices; ++i) {
      if (time_slice_index_to_task_[i] == rank_) {
	if (static_cast<int>(data_[i].size()) != num_bands_) {
	  std::cerr << "Inconsistent number of bands for time slice #"
		    << i << " : " << data_[i].size()
		    << " v.s. the input " << num_bands_ << "\n";
	  return false;
	}
	for (int j = 0; j < num_bands_; ++j) {
	  if (data_[i][j] == nullptr) {
	    std::cerr << "Null pointer provided for time slice #"
		      << i << " band #" << j << std::endl;
	    return false;
	  }
	}
      }
    }

    std::vector<std::vector<T*>> data(num_time_slices,
				      std::vector<T*>(num_bands_, nullptr));
    auto clean_buffer = [&data]() {
      for (auto &slice : data) {
	for (auto &band : slice) {
	  delete band;
	}
      }
    };
    
    for (int i = 0; i < num_time_slices; ++i) {
      for (int j = 0; j < num_bands_; j++) {
	data[i][j] = DistributeData(data_[i][j], decomposition_schema_,
				    rank_, time_slice_index_to_task_[i]);
	if (data[i][j] == nullptr) {
	  clean_buffer();
	  return false;
	}
      }
    }
    
    time_series_.resize(decomposition_schema_.counts[rank_]);
    for (int i = 0; i < decomposition_schema_.counts[rank_]; ++i) {
      for (int j = 0; j < num_time_slices; ++j) {
	std::vector<T> time_slice(num_bands_);
	for (int band = 0; band < num_bands_; ++band) {
	  time_slice[band] = data[j][band][i];
	}
	time_series_[i].AddTimeSlice(time_slice);
      }
    }

    clean_buffer();
    return true;
  }
  
  std::vector<TimeSeries<T>> GetTimeSeries() const {
    return time_series_;
  }

private:
  // Stores the map from the time slices to the tasks that will handle them.
  const std::vector<int> time_slice_index_to_task_;
  // Stores pointers to the input data. The outer layer is indexed by the
  // time slices, and the inner layer is indexed by the data layers/bands.
  // Each pointer should point to one matching layer/band (one dimension array).
  const std::vector<std::vector<T*>> data_;
  // Stores the number of elements in the input data (layer/band). All
  // layers/bands are supposed to be of equal dimension.
  const int num_pixels_;
  // Stores the number of bands for each timie slice.
  const int num_bands_;
  // Stores the decomposition schema for the task pool.
  const utils::DecompositionSchema decomposition_schema_;
  // Rank of the task.
  const int rank_;
  // The time series data.
  std::vector<TimeSeries<T>> time_series_;
  
  // Distributes the data of one time slice between tasks. The assigned
  // data will be stored in the returned address. The caller should
  // take ownership of the pointer. data should be significant at root.
  T* DistributeData(const T *data,
		    const utils::DecompositionSchema &decomposition_schema,
		    int rank, int root) {
    if (rank == root && data == nullptr) {
      std::cerr << "The input data is null at rank #"
		<<rank<<std::endl;
      return nullptr;
    }

    T* receive_buffer = nullptr;
    MPI_Datatype data_type;
    if (std::is_same<T, float>::value) {
      data_type = MPI_FLOAT;
    } else if (std::is_same<T, double>::value) {
      data_type = MPI_DOUBLE;
    } else if (std::is_same<T, int>::value) {
      data_type = MPI_INT;
    } else {
      // Unsupported data type.
      if (rank == decomposition_schema.root) {
	std::cerr << "The input data is not supported."
		  <<"Please use one of the following: float, double, int.\n";
      } 
      return receive_buffer;
    }
    receive_buffer = new T[decomposition_schema.counts[rank]];

    const int status = MPI_Scatterv(data, decomposition_schema.counts.data(),
				    decomposition_schema.displacements.data(), data_type, receive_buffer,
				    decomposition_schema.counts[rank], data_type, root,
				    MPI_COMM_WORLD);
    if (status != MPI_SUCCESS) {
      delete receive_buffer;
      receive_buffer = nullptr;
    }
    return receive_buffer;
  }
};

} /* namespace remote_sensing */

#endif /* SIMPLEGRAPH_PHENONET_TIMESERIESDECOMPOSITION_H_ */

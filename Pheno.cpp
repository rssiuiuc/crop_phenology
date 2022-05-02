//============================================================================
// Name        : PhenoNet.cpp
// Author      : RSSI (rssiuiuc@gmail.com)
// Description : An example for using the PhenoNet library
//============================================================================

#include "PhenoNet.h"
#include "TimeSeries.h"
#include "TimeSeriesDecomposition.h"
#include "Utils.h"

#include <mpi.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;
using namespace remote_sensing;

// These hard coded functions prepare the example data for demo purpose.
void AssignTasks(int num_time_slices, int num_pixels, int num_tasks,
		 int num_task_per_node, vector<int> &time_slice_index_to_task,
		 utils::DecompositionSchema &schema);

bool GetExampleData(int num_bands, int num_pixels, int rank,
		    const vector<int> &time_slice_index_to_task,
		    vector<vector<float*>> &data);

void CleanUp(vector<vector<float*>> &data);

int main(int argc, char* argv[]) {
  const int num_bands = 7;
  const int num_pixels = 114;
  const int num_time_slices = 365;
  const int root = 0;
  const double min_giant_fraction = 0.8;
  const int num_task_per_node = 4;

  int size, rank;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  utils::DecompositionSchema schema(size, root);
  
  vector<int> time_slice_index_to_task(num_time_slices, root);
  AssignTasks(num_time_slices, num_pixels, size, num_task_per_node,
	      time_slice_index_to_task, schema);

  vector<vector<float*>> example_data(num_time_slices,
				      vector<float*>(num_bands, nullptr));
  if (!GetExampleData(num_bands, num_pixels, rank, time_slice_index_to_task,
		      example_data)) {
    cerr << "Encountered error(s) while processing the example data "
	 << "for task #" << rank << endl;
    CleanUp(example_data);
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  }

  TimeSeriesDecomposition<float> time_series_distributor(
							 time_slice_index_to_task, example_data, num_bands, num_pixels,
							 schema, rank);
  if (!time_series_distributor.DistributeData()) {
    if (rank == root)
      std::cerr << "Encountered errors while distributing the data.\n";
    CleanUp(example_data);
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  }

  vector<TimeSeries<float>> time_series =
    time_series_distributor.GetTimeSeries();
  PhenoNet pheno_net(std::move(time_series), min_giant_fraction);
  pheno_net.Process();
  const auto peak_index = pheno_net.GetPeakTimeSliceIndex();
  
  int *global_peak_index =
    rank == root ? new int[num_time_slices] : nullptr;
  MPI_Gatherv(peak_index.data(), schema.counts[rank], MPI_INT,
	      global_peak_index, schema.counts.data(), schema.displacements.data(),
	      MPI_INT, root, MPI_COMM_WORLD);
  
  if (rank == root) {
    for (int i = 0; i < num_pixels; ++i) {
      cout << "pixel #" << i << " peak: " << (global_peak_index[i])
	   << endl;
    }
    delete global_peak_index;
  }

  CleanUp(example_data);
  MPI_Finalize();
  return 0;
}

// For demo purpose, only parallel on the time slices.
void AssignTasks(int num_time_slices, int num_pixels, int num_tasks,
		int num_task_per_node, vector<int> &time_slice_index_to_task,
		utils::DecompositionSchema &schema) {
  const int num_nodes = ceil(static_cast<float>(num_tasks) / num_task_per_node);
  time_slice_index_to_task.resize(num_time_slices, 0);
  for (int i = 0; i < num_time_slices; ++i) {
    time_slice_index_to_task[i] = i % num_nodes * num_task_per_node;
  }
  const int num_pixels_per_task = num_pixels / num_tasks;
  int unassigned_pixels = num_pixels % num_tasks;
  schema.counts.resize(num_tasks,  num_pixels_per_task);
  schema.displacements.resize(num_tasks, 0);
  for (int i = 0; i < num_tasks; ++i) {
    if (unassigned_pixels > 0) {
      ++schema.counts[i];
      --unassigned_pixels;
    }
    if (i > 0) {
      schema.displacements[i] = schema.displacements[i - 1]
	+ schema.counts[i - 1];
    }
  }
}

bool GetExampleData(int num_bands, int num_pixels,
		    int rank, const vector<int> &time_slice_index_to_task,
		    vector<vector<float*>> &data) {
  const int num_time_slices =
    static_cast<int>(time_slice_index_to_task.size());
  for (int i = 0; i < num_time_slices; i++) {
    if (time_slice_index_to_task[i] != rank) {
      continue;
    }
    const string input_path = "./test_data/day_" + to_string(i + 1)
      + ".txt";
    ifstream in(input_path.c_str(), ifstream::in);
    if (!in) {
      cerr << "Cannot open the input data. "
	   << "Please check if the file exists: " << input_path
	   << endl;
      CleanUp(data);
      return false;
    }
    for (int j = 0; j < num_bands; ++j) {
      data[i][j] = new float[num_pixels];
    }
    string line;
    for (int j = 0; j < num_pixels; ++j) {
      getline(in, line);
      istringstream iss(line);
      for (int band = 0; band < num_bands;
	   ++band) {
	float reflectance = 0;
	iss >> reflectance;
	data[i][band][j] = reflectance;
      }
    }
    in.close();
  }
  return true;
}

void CleanUp(vector<vector<float*>> &data) {
  for (auto &row : data) {
    for (auto &cell : row) {
      if (cell) {
	delete cell;
	cell = nullptr;
      }
    }
  }
}

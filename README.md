# Real-time Phenological Computational (RTPC) Framework
## About
The real-time phenological computational (RTPC) framework is an open-source toolkit to harness massive parallelism in modeling crop phenological progress towards scalable satellite-based precision farming. RTPC integrates dynamic complex networks with time series remote sensing to predict the real-time crop phenological progress at both fine spatial and temporal scales. The RTPC framework supports a hybrid computation model, including a node-level computation model and a system-wide data distribution model. The node-level computation model takes advantage of multi-core architecture of computing nodes to parallelize the compute-intensive RTPC in predicting dynamic network characteristics of crop phenology. The system-wide data distribution model devises a novel Space-and-Time parallel decomposition strategy in distributing massive remote sensing time series data to reduce memory requirements and to achieve high scalability. This open-source toolkit is designed to facilitate the open development and adoption of the remote sensing cyberinfrastructure across a broad range of disciplines. Through leveraging the power of high performance computing and this hybrid computation model, the cyberinfrastructure can analyze massive remotely sensed data in a highly scalable manner to conduct real-time monitoring of earth system dynamics.

Please refer to the [RSSI lab@UIUC website](https://diaorssilab.web.illinois.edu/nsf-crii-oac-project/) for more details about the toolkit.

## How to use RTPC
An [example](./Pheno.cpp) is provided to demostrate how to use the RTPC framework with Open MPI. While the example uses [text files](./test_data/) as the input for simplicity, [GDAL](https://gdal.org/) can be used to handle input data in binary formats (e.g. TIFF data from Landsat). 

The RTPC model is a dynamic complex network model consisting of two components: a base network and an adaptive node addition algorithm. For each pixel, a base network will be constructed according to its spectral reflectances collected over the course of a year. A base network of a mapping year is typically constructed with the collective spectral reflectances of a pixel from the immediately preceding year, and the structure of the network will serve as the prior information to characterize the crop phenological progress in the current year. An adaptive node addition algorithm will add a real-time node to the base network, and measure how the node addition alters the network structure. Specifically, the real-time node will be connected to existing nodes that share similar spectral reflectances, and the bridging coefficient will be recalculated for each node in the updated network. The real-time node that attains comparable bridging coefficient as those in the transition cluster of the base network is indicative of the phenological transition date in the current year. With the iterative addition of real-time nodes to the base network, the RTPC model can predict the phenological transition dates in a timely fashion. 

![image](https://user-images.githubusercontent.com/104749953/166404375-5f9db555-7968-4115-ae89-3a27ad8ba651.png)

Figure 1. Network measures of crop phenology to estimate the critical phenological transition date.

The hybrid computation model includes a node-level computation model and a system-wide data distribution model. In the node-level computation model, a computing node stores the information of a number of pixels, and processes them using OpenMP. In this model, the computation of each pixel is split among all cores of the computing node. These cores access the same memory space, and all communications between cores are within the same computing node. The design of the node-level computation model increases computation efficiency while decreasing memory requirement. The system-wide data distribution model encompasses a two-level data decomposition strategy (with space and time decompositions). The first level uses a space decomposition, i.e. each imagery is divided into a number of tiles. All computing nodes are assigned to different node groups such that each group is responsible for all imagery tiles covering the same spatial extent. At the second level, both space and time decomposition is applied within each group to improve the I/O performance. For the space decomposition, each tile is further divided into sub-tiles so that each computing node processes one sub-tile. For the time decomposition, tiles are grouped based on the date they are obtained to form tile groups. Using this model, the huge number of data and I/O operations are evenly distributed among all computing nodes. Thus, all the nodes could read the tiles simultaneously, and each computing node only uses a small amount of memory. This greatly reduces memory requirement and time for reading data files. Furthermore, this data distribution model is highly scalable and can be used in from small HPC to state-of-the-art petascale HPC clusters.

![image](https://user-images.githubusercontent.com/104749953/166404751-671f5662-c966-4835-b41f-bafe0bd98b4b.png)

Figure 2. The two-level data distribution of the hybrid computation model.

## Citing RTPC
If you use RTPC in your work,  please cite our paper:

```
@article{DIAO201996,
  title = {Innovative pheno-network model in estimating crop phenological stages with satellite time series},
  journal = {ISPRS Journal of Photogrammetry and Remote Sensing},
  volume = {153},
  pages = {96-109},
  year = {2019},
  issn = {0924-2716},
  doi = {https://doi.org/10.1016/j.isprsjprs.2019.04.012},
  url = {https://www.sciencedirect.com/science/article/pii/S092427161930108X},
}
```

## Acknowledgement
This project is supported by the National Science Foundation’s Office of Advanced Cyberinfrastructure under grant 1849821

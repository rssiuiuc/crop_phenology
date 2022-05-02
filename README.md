# Real-time Phenological Computational (RTPC) Model
## About
The real-time phenological model computational (RTPC) model is an open-source toolkit to harness massive parallelism in modeling crop phenological progress towards scalable satellite-based precision farming. RTPC integrates dynamic complex networks with time series remote sensing to predict the real-time crop phenological progress at both fine spatial and temporal scales. TRTPC supports a hybrid computation model, including a node-level computation model and a system-wide data distribution model. The node-level computation model takes advantage of multi-core architecture of computing nodes to parallelize the compute-intensive RTPC in predicting dynamic network characteristics of crop phenology. The system-wide data distribution model devises a novel Space-and-Time parallel decomposition strategy in distributing massive remote sensing time series data to reduce memory requirements and to achieve high scalability. This open-source toolkit is designed to facilitate the open development and adoption of the remote sensing cyberinfrastructure across a broad range of disciplines. Through leveraging the power of high performance computing and this hybrid computation model, the cyberinfrastructure can analyze massive remotely sensed data in a highly scalable manner to conduct real-time monitoring of earth system dynamics.

Please refer to the [RSSI lab@UIUC website](https://diaorssilab.web.illinois.edu/nsf-crii-oac-project/) for more details about the toolkit.

## How to use RTPC
An [example](./Pheno.cpp) is provided to demostrate how to use the RTPC model with Open MPI. While the example uses [text files](./test_data/) as the input for simplicity, [GDAL](https://gdal.org/) can be used to handle input data in binary formats (e.g. TIFF data from Landsat). 

## Citing RTPC
If you use RTPC in your work,  plese cite our paper:

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



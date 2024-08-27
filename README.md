

# Associated data and code

The files in this code repository are commented versions of the code used for the following work:

_The Integrated Vehicle and Pollster Routing Problem_ by Sandra Gutiérrez, Andrés Miniguano-Trujillo, Diego Recalde, Luis M. Torres, Ramiro Torres.

## Main files in this repository

* [`ReadMe.md`](README.md): This file.
* [`Combinatorial Optimization at Work`](Combinatorial%20Optimization%20At%20Work): Folder containing working examples presented at [CO@Work – TU Berlin](https://co-at-work.zib.de/berlin2020/).
* [`Gallery GYE`](Gallery%20GYE): Folder containing additional visualisations of the solution for the big instance based on information collected at Guayaquil.

Most of the reported results were obtained using Gurobi 8.

* [`Points of Interest.ipynb`](Points%20of%20Interest.ipynb): Samples 467 points of interest from OpenStreetMaps using OSMnx (2019). The code relies on the [`connect_poi.py`](connect_poi.py) file based on a previous version of Yuwen Chang's code in
[`ywnch/toolbox`](https://github.com/ywnch/toolbox/tree/e5c0b59556d4346cb4de6f716eaa0b9d2d9f354c).
Data is stored in [`poi.gpkg`](poi.gpkg) and summary is available at [`poi.xlsx`](poi.xlsx). [Documentation in Spanish]

* [`Sampling.ipynb`](Sampling.ipynb): Artificial instance generator uses data from the points of interest collected in `poi.gpkg` to generate an instance with η stores. [Documentation in Spanish]

The last file was used to generate the [`Instances`](Instances) folder. Here we find different instances of the IVPRP.
Each instance with $n$ stores consists of the following files:

| File             |   | Description                                                                                                                                                                 |
|------------------|---|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `Service_n.txt`  |   | A file with $n+1$ integers with service times at each store.  The first value is not used, it is just zero for convenience.                                                 |
| `Pollster_n.txt` |   | A file with a $n\times n$ matrix with pollsters walking times  in the pollsters subgraph. This matrix can be symmetric but in  some geometries it could be seen as not.     |
| `Vehicle_n.txt`  |   | An $(n+1)\times (n+1)$ matrix with vehicle travelling times.  This matrix won’t be symmetric in most cases as the street  network does not usually allow two-sided streets. |
| `Time_n.txt`     |   | The first row of this file is the time range for taking breaks,  and the second row is the length of the break and shift length.                                            |



Jupyter notebooks with code for [...] are included. 




---

For better visualisation of the notebooks, the `codefolding` for Jupyter [extension](https://jupyter-contrib-nbextensions.readthedocs.io/en/latest/) is recommended.








# Associated data and code

The files in this code repository are commented versions of the code used for the following work:

_The Integrated Vehicle and Pollster Routing Problem_ by Sandra Gutiérrez, Andrés Miniguano-Trujillo, Diego Recalde, Luis M. Torres, Ramiro Torres.

## Main files in this repository

* [`ReadMe.md`](README.md): This file.
* [`Combinatorial Optimization at Work`](Combinatorial%20Optimization%20At%20Work): Folder containing working examples presented at [CO@Work – TU Berlin](https://co-at-work.zib.de/berlin2020/).
* [`Gallery GYE`](Gallery%20GYE): Folder containing additional visualisations of the solution for the big instance based on information collected at Guayaquil.

### Simulated instances: 

* [`Sampling.ipynb`](Sampling.ipynb): Artificial instance generator uses data from the points of interest collected in `Sampling_Data` to generate an instance with η stores. The code also showcases how to build the dataset from scratch using OpenStreetMaps data and OSMnx 2.0.

The last file was used to generate the [`Instances`](Instances) folder. Here we find different instances of the IVPRP.
Each instance with $n$ stores consists of the following files:

| File             |   | Description                                                                                                                                                                 |
|------------------|---|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `Service_n.txt`  |   | A file with $n+1$ integers with service times at each store.  The first value is not used, it is just zero for convenience.                                                 |
| `Pollster_n.txt` |   | A file with a $n\times n$ matrix with pollsters walking times  in the pollsters subgraph. This matrix can be symmetric but in  some geometries it could be seen as not.     |
| `Vehicle_n.txt`  |   | An $(n+1)\times (n+1)$ matrix with vehicle travelling times.  This matrix won’t be symmetric in most cases as the street  network does not usually allow two-sided streets. |
| `Time_n.txt`     |   | The first row of this file is the time range for taking breaks,  and the second row is the length of the break and shift length.                                            |


The results of the tabu search algorithm from [`doi: 10.1007/s10878-018-0254-1`](https://doi.org/10.1007/s10878-018-0254-1) are included for each instance under the format `Partition_n_k.txt` where $k$ is the number of clusters of the $k$–way partition.



### Solution methods


Jupyter notebooks with code for solving the instances are included (Tables 3—5). 


* [`IVPRP testing - Iterative.ipynb`](IVPRP%20testing%20-%20Iterative.ipynb): Applies _MIP Solve_ iteratively.
* [`IVPRP testing - 3P Algorithm.ipynb`](IVPRP%20testing%20-%203P%20Algorithm.ipynb): Applies three phase heuristic to a selected partition of an instance.
* [`IVPRP testing - Fixing.ipynb`](IVPRP%20testing%20-%20Fixing.ipynb): Applies fixing result to an instance where additional information is known.

Three folders collect the results.


* [`Sol MIP`](Sol%20MIP): Results of _MIP Solve_. Outputs are formatted as `Out_n.xlsx`.

* [`Sol Heuristic`](Sol%20Heuristic): Results of the three-phase algorithm. Outputs are formatted under the name `Out_n_k-s.xlsx`, where $s$ identifies the subset label (possible values $s \in \{\text{A},\text{B},\text{C},\text{D}\}$).

* [`Sol Fix Heuristic`](Sol%20Fix%20Heuristic):
The _Fixing Variables Heuristic_ adds bounds from the Heuristic solve to full MIP model. The bounds for $n\geq 12$ are included in format `Fixing_n_k.txt`. The corresponding results are included in the files with format `Out_Fixing_n_k.xlsx`.


Additional files and folders:

* [`IVPRP - Bounds [Iterative]`](IVPRP%20-%20Bounds%20%5BIterative%5D.ipynb): Computes lower and upper bounds for each instance.
* [`Tabu`](Tabu): `C++` code with the KPART Tabu search heuristic for partitioning an instance. The folder is self-contained. To test the code, change $n$ (number of stores) and $k$ (number of partitions, should be even for 3P heuristic). For a given $n$, it is expected that the files `Service_n.txt` and `Pollster_n.txt` are available in the subfolder [`Tabu/data`](Tabu/data); where `Service_n.txt` ought not to have an entry for the depot. 
The code is based on the original KPART for weighted balanced $k$-clique partitioning, which first appeared at [`doi:10.1007/978-3-319-45587-7_31`](https://doi.org/10.1007/978-3-319-45587-7_31) and later at [`doi:10.1007/s10878-018-0254-1`](https://doi.org/10.1007/s10878-018-0254-1) and [`doi:10.17632/7yypjxy4f7.1`](http://dx.doi.org/10.17632/7yypjxy4f7.1). The associated files were last tested using `Xcode 16.2` and `GNU++11`.


## Self-contained tests

* [`IVPRP - Example`](IVPRP%20-%20Example.ipynb): Solves the example instance from the data of Table 2.




---

For better visualisation of the notebooks, the `codefolding` for Jupyter [extension](https://jupyter-contrib-nbextensions.readthedocs.io/en/latest/) is recommended.






FDTD D3: FDTD with novel subgridding techniques
============================================

What is FDTD D3?
-------------

FDTD D3 is a 2D FDTD solver which demonstrates the use of a novel subgridding technique.
FDTD D3 is in development; some features are missing and there are bugs.


Requirements
------------

You need QCustomPlot 1.3.2 or later. The subgridding makes use Eigen 3.2.8 and the sensors need FFTW 3.3.4.
QCustomPlot can be downloaded from http://www.qcustomplot.com/
Eigen can be downloaded from http://eigen.tuxfamily.org/
FFTW can be downloaded from http://www.fftw.org


Quick start
-----------

Download all files from this repository and install/add all aforementiones dependencies. 
Using Qt5 the program can be deployed on both Mac and Windows. A prebuild version for Mac is available in the download.

Once running, sources can be added by right clicking sources -> add new current source. Hereafter a menu pops up where the parameters can be set which define the source.
A similar method is used when selecting a different object in the list. In the preferences, the grid can be defined. Many more options are available in the menubar.


Development status
------------------

FDTD D3 is work in progress and is intended as a proof of concept for the novel subgridding technique.
Currently it contains:
- Total field/scattered field
- Dipole sinusoidal/gaussian sources
- Polygonial materials can be added
- Sensors can be added which monitor the fields. After the simulation, the time and frequency domain data can be examined.
- Subgridding regions can be added with any refinement ratio (minimal size is 3 cells)


License
-------

FDTD D3 is licensed under the terms of the GNU License (see the file COPYRIGHT).

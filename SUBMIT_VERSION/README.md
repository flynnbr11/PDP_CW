# Squirrel Ecosystem
Actor Pattern implementation of environment of squirrels subject to infection 
over time. 

##Compile
Compile using Makefile (requires mpic++):
```
$ make
```
##Files
Files are commented to explain individual functions. 
Adjustable parameters defined as global variables in: 
* parameters.h

Functionality (requires MPI) given by:
* ecosystem.cpp
* actorFunctions.cpp
* cellClass.cpp
* clockClass.cpp
* squirrelClass.cpp
* trackerClass.cpp


Biologists serial functions:
* pool.c
* squirrel-functions.c
* ran2.c

Other files: 
* Makefile: compiles program
* run.sh compiles and runs on 250 processes
* eco.sge runs executable eco on Morar


##Run
Note that in order to run, must provide enough processes that the program 
can never exceed try to begin a process which is not available. 
If MAX_NUM_SQUIRRELS = 200; NUM_CELLS=16
	then 250 processes is sufficient. If the max number of squirrels is increased, 
	increase the number of processes acordingly.
	
To run locally and copy output into a file 

```
$ mpiexec -n 250 ./eco | tee output.dat  
```

To run on Morar: first make, then submit the script eco.sge as follows:

```
$ qsub eco.sge  
```



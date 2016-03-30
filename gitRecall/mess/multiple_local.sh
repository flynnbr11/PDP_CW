#!/bin/bash

make clean
make

for a in 1 2 3 4 5 
do
	mpiexec -n 250 ./eco | output_$a.dat
done 


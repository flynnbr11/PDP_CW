#!/bin/bash

make clean
make

for a in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
do
	mpiexec -n 250 ./eco | tee outputs/	output_$a.dat 
done 



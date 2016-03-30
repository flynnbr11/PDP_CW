#!/bin/bash

make clean

make

mpiexec -n 250 ./eco

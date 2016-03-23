CC = mpic++ -w
#CC = mpic++ -Wno-write-strings
CC_C = mpicc
OBJ= ecosystem.cpp squirrelClass.cpp cellClass.cpp ran2.c squirrel-functions.c pool.c clockClass.cpp
LIB = -lm 

eco : $(OBJ)
	$(CC) $(OBJ) $(LIB) -o $@

clean : 
	rm eco


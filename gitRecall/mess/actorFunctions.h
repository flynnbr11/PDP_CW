#ifndef ACTOR_H
#define ACTOR_H

#include <stdio.h>
#include "mpi.h"
#include "pool.h"
#include "squirrel-functions.h"
#include "ran2.h"
#include <iostream>
#include "cellClass.h"
#include "squirrelClass.h"
#include "clockClass.h"
#include "trackerClass.h"
#include "parameters.h" 
#include "unistd.h"
using namespace std;


void masterCode();
void actorCode();
void trackerCode();
void clockTime();
void gridCode();
void squirrelCode();


#endif

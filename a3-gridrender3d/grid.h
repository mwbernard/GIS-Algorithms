#ifndef _grid_h
#define _grid_h

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <unistd.h>



/*
  stores the header of an .asc grid file
 */
typedef struct _grid {
  int nrow;
  int ncol;
  float xllcorner;
  float yllcorner;
  float cellSize;
  float noDataValue;
  float max;
  float min;
  
  float **data;
} Grid;



////////////////////////////////////////////////////////////


void readGridFromFile(char *gridfname, Grid *grid);

void findExtrema(Grid *grid);

void printGrid(Grid *grid);

void freeGrid(Grid *grid);




#endif

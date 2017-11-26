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

void initFlowGrids(Grid *grid, Grid *fdgrid, Grid *fagrid, Grid *vgrid);

void computeFD(Grid *grid, Grid *fdgrid);

float minFloat(float a, float b);

float maxFloat(float a, float b);

int isInBounds(Grid *grid, int i, int j);

void checkNeighborFD(Grid *grid, int i, int j, float *minElev, float *fd, float code);

void computeFA(Grid *grid, Grid *fdgrid, Grid *fagrid, Grid *vgrid);

int checkNeighborFA(Grid *fdgrid, int i, int j, float code);

float individualFA(Grid *fdgrid, Grid *fagrid, Grid *vgrid, int i, int j);

void gridToFile(Grid *grid, char *out_path);



#endif

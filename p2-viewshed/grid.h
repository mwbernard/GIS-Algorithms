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

/////////////////////////////////////////////////////////////

void initVisGrid(Grid *grid, Grid *vg);

void computeViewshed(Grid *grid, Grid *vg, int vprow, int vpcol);

float isVisible(Grid *grid, int vprow, int vpcol, int row, int col);

int isVertical(int vpcol, int col);

float computeSlope(float x, float x1, float y, float y1);

float computeY(float x, float x1, float y1, float slope);

float computeX(float y, float y1, float x1, float slope);

int min(int a, int b);

int max(int a, int b);

float dist(float x, float y, float x1, float y1);

void gridToFile(Grid *grid, char *out_path);

void checkViewPoint(Grid *grid, int vprow, int vpcol);

#endif

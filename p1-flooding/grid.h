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


typedef struct _node Node;
struct _node {
  int i;
  int j;
  Node *next;
};

////////////////////////////////////////////////////////////


void readGridFromFile(char *gridfname, Grid *grid);

void findExtrema(Grid *grid);

void printGrid(Grid *grid);

void freeGrid(Grid *grid);

////////////////////////////////////////////////////////////

void push(Node **head, Node **tail, int i, int j);

void pop(Node **head, Node **tail);

int isEmpty(Node *head);

////////////////////////////////////////////////////////////

float isWater(float elev, float noDataValue, float seaLevel);

void findWater(Grid *grid);

void flood(Grid *grid, float h);

void initVisitGrid(Grid *grid, int **visit);

void searchBorder(Grid *grid, int **visit, Node **head, Node **tail);

void floodHelper(Grid *grid, float h, int **visit, Node **head, Node **tail);

void checkNeighbors(Grid *grid, int **visit, int i, int j, Node **head, Node **tail);

void neighborHelper(Grid *grid, int **visit, int i, int j, Node **head, Node **tail);

int isInBounds(Grid *grid, int **visit, int i, int j);



#endif

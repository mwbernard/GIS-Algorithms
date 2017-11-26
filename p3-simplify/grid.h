#ifndef _grid_h
#define _grid_h

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <unistd.h>

#define FALSE 0
#define NORM 1
#define AB 2
#define BC 3
#define CA 4


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

//////////////////////////////////////////////////////////


typedef struct _plane {
  float i;
  float j;
  float k;
} Plane;


typedef struct _point3D {
  float x;
  float y;
  float z;
} Point3D;

//list to keep track of the points in each triangle
typedef struct _list List;
struct _list {
  Point3D p;
  List *next;
};

typedef struct _triangle {
  Point3D a;
  Point3D b;
  Point3D c;
  float error;
  Point3D e; //error point
  int errLoc; //error location (is it on edge or inside triangle?)

  Plane plane;

  List *head;
  List *tail;
} Triangle;


typedef struct _simpGrid {
  int numTriangles;
  int numPoints;
  Grid newGrid;
  Triangle *tin;
} SimpGrid;

///////////////////////////////////////////////////////////

typedef struct _node {
  Triangle t;
} Node;

typedef struct _maxHeap {
  int size;
  int space;
  Node *elem;
} MaxHeap;


////////////////////////////////////////////////////////////


void readGridFromFile(char *gridfname, Grid *grid);
void findExtrema(Grid *grid);
void printGrid(Grid *grid);
void freeGrid(Grid *grid);

//////////////////////////////////////////////////////////////

//Author: Robin Thomas <robinthomas2591@gmail.com>

//Max heap functions
int LCHILD(int x);
int RCHILD(int x);
int PARENT(int x);
MaxHeap initMaxHeap(int size);
void swap(Node *n1, Node *n2);
void heapify(MaxHeap *hp, int i);
void insertNode(MaxHeap *hp, Triangle t);
void deleteNode(MaxHeap *hp);
void deleteMaxHeap(MaxHeap *hp);

////////////////////////////////////////////////////////////////

//list functions
void push(List **head, List **tail, Point3D p);
void pop(List **head, List **tail);
int isEmpty(List *head);

//////////////////////////////////////////////////////////////////////

//geometry functions
float signed_area2D(Point3D a, Point3D b, Point3D c);
int left(Point3D a, Point3D b, Point3D c);
int isPointEqual(Point3D a, Point3D b);
void determinePlane(Triangle *t);
float computeEstimate(Point3D p, Point3D a, Plane plane);
float computeError(Point3D p, Point3D a, Plane plane);

////////////////////////////////////////////////////////////////////

int isCornerPoint(Grid *grid, int i, int j);
void initTin(Grid *grid, SimpGrid *simp, MaxHeap *hp);
void createTin(SimpGrid *simp, MaxHeap *hp, float epsilon);
void insertPointsCollinear(Triangle t, Triangle *t1, Triangle *t2);
void insertPoints(Triangle t, Triangle *t1, Triangle *t2, Triangle *t3);
void initTrianglesHelper(Triangle *t);
void initTrianglesCollinear(Triangle t, Triangle *t1, Triangle *t2);
void initTriangles(Triangle t, Triangle *t1, Triangle *t2, Triangle *t3);

/////////////////////////////////////////////////////////////////////////

void initSimpGrid(SimpGrid *simp, Grid *grid);
void removeTinFromHeap(SimpGrid *simp, MaxHeap *hp);
int searchTin(SimpGrid *simp, Point3D p);
int isOnEdge(Point3D p, Point3D a, Point3D b);
int isInside(Point3D p, Point3D a, Point3D b, Point3D c, int *errLoc);
void computeEstimateGrid(SimpGrid *simp, Grid *grid);
void simplify(SimpGrid *simp, Grid *grid, MaxHeap *hp, float epsilon);
void tinToFile(SimpGrid *simp, char *out_path);



#endif


#include <math.h>
#include <fcntl.h>
#include <unistd.h>

#include "grid.h"


/* Assuming f represents an .asc grid file, it reads header from f*/
void readGridFromFile(char *gridfname, Grid *grid) {
  
  assert(gridfname && grid);
  
  FILE *filePtr = fopen(gridfname, "rb");
  
  if (filePtr == NULL) {
    perror("Failed  to open input file: \n");
    exit(1);
  }
  
  if (fscanf(filePtr, "ncols\t%d\nnrows\t%d\n", 
	     &grid->ncol, &grid->nrow) != 2)  {
    perror("Could not read grid size.");
    fclose(filePtr);
    exit(1); 
  }
  if (fscanf(filePtr, "xllcorner\t%f\nyllcorner\t%f\ncellsize\t%f\n", 
	     &grid->xllcorner,
	     &grid->yllcorner, &grid->cellSize) != 3) {
    perror("Could not read grid meta data");
    fclose(filePtr);
    exit(1); 
  }
  if (fscanf(filePtr, "NODATA_value\t%f\n", &grid->noDataValue) != 1) {
    perror("Could not read grid NODATA_value");
    fclose(filePtr);
    exit(1); 
  }
  
  //allocate 2D array
  grid->data = malloc(grid->nrow * sizeof(float*));
  assert(grid->data);
  
  for (int i = 0; i < grid->nrow; i++) {
    grid->data[i] = malloc(grid->ncol * sizeof(float));
    assert(grid->data[i]);
  }
  
  for (int i = 0; i < grid->nrow; i++) {
    for (int j = 0; j < grid->ncol; j++) {
      if (fscanf(filePtr, "%f", &grid->data[i][j]) != 1) {
	perror("Could not read grid data");
	fclose(filePtr);
	exit(1);
      }
    }
    fscanf(filePtr, "\n");
  }
  
  fclose(filePtr);
  
}

void findExtrema(Grid *grid)
{
  int dataExists = 0;
  
  grid->max = grid->noDataValue;
  grid->min = grid->noDataValue;
  
  for (int i = 0; i < grid->nrow; i++) {
    for (int j = 0; j < grid->ncol; j++) {
      if (dataExists == 0 && (grid->data[i][j] != grid->noDataValue)) {
        dataExists = 1;
        grid->max = grid->data[i][j];
        grid->min = grid->data[i][j];
      }
      else {
        if (grid->data[i][j] > grid->max) {
          grid->max = grid->data[i][j];
        }
        if ((grid->data[i][j] < grid->min) && (grid->data[i][j] != grid->noDataValue)) {
          grid->min = grid->data[i][j];
        }
      }
    }
  }
}

void printGrid(Grid *grid)
{
  printf("\n");
  
  for (int i = 0; i < grid->nrow; i++) {
    for (int j = 0; j < grid->ncol; j++) {
      printf("%f ", grid->data[i][j]);
    }
    printf("\n");
  }
}


void freeGrid(Grid *grid) {
  if (grid) {
    free(grid->data);
  }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////

//Author: Robin Thomas <robinthomas2591@gmail.com>

int LCHILD(int x)
{
  return (2 * x) + 1;
}

int RCHILD(int x)
{
  return (2 * x) + 2;
}

int PARENT(int x)
{
  return x / 2;
}

/*
  Function to initialize the max heap with size = 0
*/
MaxHeap initMaxHeap(int size) 
{
  MaxHeap hp;
  hp.size = 0;
  hp.space = size;
  hp.elem = malloc(size * sizeof(Node));
  return hp;
}

/*
  Function to swap data within two nodes of the max heap using pointers
*/
void swap(Node *n1, Node *n2) 
{
  Node temp = *n1;
  *n1 = *n2;
  *n2 = temp;
}


/*
  Heapify function is used to make sure that the heap property is never violated
  In case of deletion of a node, or creating a max heap from an array, heap property
  may be violated. In such cases, heapify function can be called to make sure that
  heap property is never violated
*/
void heapify(MaxHeap *hp, int i) 
{
  int largest = (LCHILD(i) < hp->size && hp->elem[LCHILD(i)].t.error > hp->elem[i].t.error) ? LCHILD(i) : i;
  if(RCHILD(i) < hp->size && hp->elem[RCHILD(i)].t.error > hp->elem[largest].t.error) {
    largest = RCHILD(i);
  }
  if(largest != i) {
    swap(&(hp->elem[i]), &(hp->elem[largest]));
    heapify(hp, largest);
  }
}

/*
  Function to insert a node into the max heap, by allocating space for that node in the
  heap and also making sure that the heap property and shape propety are never violated.
*/
void insertNode(MaxHeap *hp, Triangle t) 
{
  
  if(hp->size) {
    if (hp->size == hp->space) {
      hp->space = 2 * hp->space;
      hp->elem = realloc(hp->elem, hp->space * sizeof(Node));
    }
  } else {
    if (hp->space == 0)	{
      hp->space = 1;
      hp->elem = malloc(sizeof(Node));
    }  
  }
  
  Node nd;
  nd.t = t;
  
  int i = (hp->size)++;
  
  while(i && nd.t.error > hp->elem[PARENT(i)].t.error) {
    hp->elem[i] = hp->elem[PARENT(i)];
    i = PARENT(i);
  }
  hp->elem[i] = nd;
  
  // Making sure that heap property is also satisfied
  for(i = (hp->size - 1) / 2; i >= 0; i--) {
    heapify(hp, i);
  }    
}



/*
  Function to delete a node from the max heap
  It shall remove the root node, and place the last node in its place
  and then call heapify function to make sure that the heap property
  is never violated
*/
void deleteNode(MaxHeap *hp) 
{
  if(hp->size) {
    //Triangle tmp = hp->elem[0].t;
    //printf("Deleting node %d\n\n", hp->elem[0].data);
    hp->elem[0] = hp->elem[--(hp->size)];
    //hp->elem = realloc(hp->elem, hp->size * sizeof(Node));
    heapify(hp, 0);
  } else {
    //printf("\nMax Heap is empty!\n");
    free(hp->elem);
  }
}



/*
  Function to clear the memory allocated for the max heap
*/
void deleteMaxHeap(MaxHeap *hp) 
{
  free(hp->elem);
}




//////////////////////////////////////////////////////////////////////////////

void push(List **head, List **tail, Point3D p)
{
  List* temp = (List*)malloc(sizeof(List));
  temp->p = p;
  temp->next = NULL;
  
  if (*head == NULL && *tail == NULL) {
    *head = *tail = temp;
    return;
  }
  
  (*tail)->next = temp;
  *tail = temp;
}

void pop(List **head, List **tail)
{
  List* temp = *head;
  
  if (isEmpty(*head)) {
    return;
  }
  
  if (*head == *tail) {
    *head = *tail = NULL;
  }
  else {
    *head = temp->next;
  }
  free(temp);
}

int isEmpty(List *head)
{
  if (head == NULL) {
    return 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

/* 
   returns the signed area of triangle abc. The area is positive if c
   is to the left of ab, and negative if c is to the right of ab
*/

float signed_area2D(Point3D a, Point3D b, Point3D c) 
{
  return (float)((((b.x - a.x) * (c.y - a.y)) - ((c.x - a.x) * (b.y - a.y))) / 2);
}


/* 
   return 1 if c is  strictly left of ab; 0 otherwise.
   If signed area is positive, we know that c is left of ab
   if c is collinear return -1
*/

int left(Point3D a, Point3D b, Point3D c) 
{ 
  float sign = signed_area2D(a, b, c);
  
  if (sign > 0) {
    return 1;
  }
  if (sign == 0) {
    return -1;
  }
  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////

/*
Function that finds the i, j, k components for the equation of a plane given a triangle.
Does this using the cross product
*/
void determinePlane(Triangle *t)
{
  float v1_i = t->b.x - t->a.x;
  float v1_j = t->b.y - t->a.y;
  float v1_k = t->b.z - t->a.z;
  
  float v2_i = t->c.x - t->a.x;
  float v2_j = t->c.y - t->a.y;
  float v2_k = t->c.z - t->a.z;
  
  t->plane.i = (v1_j * v2_k) - (v1_k * v2_j);
  t->plane.j = 0 - ((v1_i * v2_k) - (v1_k * v2_i));
  t->plane.k = (v1_i * v2_j) - (v1_j * v2_i);
  
}

/*
Given a point and the equation of a plane and the point in that plane,
computes the estimate of the elevation of point p
*/
float computeEstimate(Point3D p, Point3D a, Plane plane)
{
  //a(x - x0) + b(y - y0) + c(z - z0) = 0
  //solve for z 
  
  return (((-plane.i * (p.x - a.x)) - (plane.j * (p.y - a.y))) / plane.k) + a.z;
}

/*
takes the absolute value of the difference between the estimated elevation and the
actual elevation of point p
*/
float computeError(Point3D p, Point3D a, Plane plane)
{
  return fabsf(computeEstimate(p, a, plane) - p.z);
}

/*
  When we are initializing the grid we don't want to add the corner points because
  they make up the two initial triangles
*/
int isCornerPoint(Grid *grid, int i, int j)
{
  if ((i == 0) && (j == 0)) {
    return 1;
  }
  if ((i == 0) && (j == grid->ncol - 1)) {
    return 1;
  }
  if ((i == grid->nrow - 1) && (j == 0)) {
    return 1;
  }
  if ((i == grid->nrow - 1) && (j == grid->ncol - 1)) {
    return 1;
  }
  return 0;
}

/*
  Initializes the tin (which at this point is stored as a max heap vector of triangles)
  with the two big triangles that cover the whole grid. Then loops through each point
  and finds which triangle it belongs to. On the fly it calculates the error of each point
  and updates the error of the triangle
*/
void initTin(Grid *grid, SimpGrid *simp, MaxHeap *hp)
{
  Triangle t1;
  Triangle t2;
  
  t1.a.x = 0;
  t1.a.y = 0;
  t1.a.z = grid->data[0][0];
  
  t1.b.x = 0;
  t1.b.y = (float)(grid->nrow - 1);
  t1.b.z = grid->data[grid->nrow - 1][0];
  
  t1.c.x = (float)(grid->ncol - 1);
  t1.c.y = (float)(grid->nrow - 1);
  t1.c.z = grid->data[grid->nrow - 1][grid->ncol - 1];
  
  determinePlane(&t1);
  initTrianglesHelper(&t1);
  
  t2.a.x = 0;
  t2.a.y = 0;
  t2.a.z = grid->data[0][0];
  
  t2.b.x = (float)(grid->ncol - 1);
  t2.b.y = 0;
  t2.b.z = grid->data[0][grid->ncol - 1];
  
  t2.c.x = (float)(grid->ncol - 1);
  t2.c.y = (float)(grid->nrow - 1);
  t2.c.z = grid->data[grid->nrow - 1][grid->ncol - 1];
  
  determinePlane(&t2);
  initTrianglesHelper(&t2);
  
  for (int i = 0; i < grid->nrow; i++) {
    for (int j = 0; j < grid->ncol; j++) {
      
      if (isCornerPoint(grid, i, j)) {
        continue;
      }
      
      Point3D p;
      p.x = (float)j;
      p.y = (float)i;
      p.z = grid->data[i][j];
      
      float err = 0;
      int errLoc = 0;
      
      if (isInside(p, t1.a, t1.b, t1.c, &errLoc)) {
        err = computeError(p, t1.a, t1.plane);
	
        if (err > t1.error) {
          t1.e = p;
          t1.error = err;
          t1.errLoc = errLoc;
        }
        push(&t1.head, &t1.tail, p);
	
      }
      else if (isInside(p, t2.a, t2.b, t2.c, &errLoc)) {
        err = computeError(p, t2.a, t2.plane);
	
        if (err > t2.error) {
          t2.e = p;
          t2.error = err;
          t2.errLoc = errLoc;
        }
        push(&t2.head, &t2.tail, p);
      }
    }
  }
  //add two triangles to the heap;
  insertNode(hp, t1);
  insertNode(hp, t2);
  
  //we have 4 distinct points in our tin
  simp->numPoints = 4;
}


/*
THIS IS THE MAJOR FUNCTION OF THE PROGRAM. We have a max heap of triangles.
Each triangle has a list of points that belong (are inside or on edge) of the
given triangle. While the head of the heap (max error) is greater than the inputted
epsilon, we pop off the triangle and create 2 or 3 new triangles (depending on the collinear
case)
*/
void createTin(SimpGrid *simp, MaxHeap *hp, float epsilon)
{
  if (hp->size == 0) {
    return;
  }
  
  while (hp->elem[0].t.error > epsilon) {
    
    //printf("%f\n", hp->elem[0].t.error);
    simp->numPoints++;
    //removePoint(simp, hp->elem[0].t.e);
    
    Triangle *t1 = malloc(sizeof(Triangle));
    Triangle *t2 = malloc(sizeof(Triangle));
    Triangle *t3 = malloc(sizeof(Triangle));
    
    //non collinear case
    if (hp->elem[0].t.errLoc == NORM) {
      initTriangles(hp->elem[0].t, t1, t2, t3);
      insertPoints(hp->elem[0].t, t1, t2, t3);
      deleteNode(hp);
      insertNode(hp, *t1);
      insertNode(hp, *t2);
      insertNode(hp, *t3);  
    }
    //collinear case
    else if (hp->elem[0].t.errLoc != FALSE) {
      initTrianglesCollinear(hp->elem[0].t, t1, t2);
      insertPointsCollinear(hp->elem[0].t, t1, t2);
      deleteNode(hp);
      insertNode(hp, *t1);
      insertNode(hp, *t2);
      
    }
    free(t1);
    free(t2);
    free(t3);
  }
}

/*
  checks to see if two points are identical
*/
int isPointEqual(Point3D a, Point3D b)
{
  if ((a.x == b.x) && (a.y == b.y) && (a.z == b.z)) {
    return 1;
  }
  return 0;
}

/*
  Function loops through the current triangle with the greatest error and removes
  points one by one and adds them to either triangle t1 or triangle t2. We are
  creating 2 triangles because this is the collinear case
*/
void insertPointsCollinear(Triangle t, Triangle *t1, Triangle *t2)
{
  while (t.head != NULL) {
    float err = 0;
    int errLoc = 0;
    
    Point3D p = t.head->p;
    
    //if point is equal to the point that caused the error we dont
    //add it to either triangle because it is now a vertex of both triangle t1 and triangle t2
    if (isPointEqual(t.e, p) == 0) {
      if (isInside(p, t1->a, t1->b, t1->c, &errLoc)) {
	err = computeError(p, t1->a, t1->plane);
	
	if (err > t1->error) {
	  t1->e = p;
	  t1->error = err;
	  t1->errLoc = errLoc;
	}
	push(&t1->head, &t1->tail, p);
      }
      else if (isInside(p, t2->a, t2->b, t2->c, &errLoc)) {
	err = computeError(p, t2->a, t2->plane);
	
	if (err > t2->error) {
	  t2->e = p;
	  t2->error = err;
	  t2->errLoc = errLoc;
	}
	push(&t2->head, &t2->tail, p);
      }
    }
    pop(&t.head, &t.tail);
  }
}

/*
Loops through the points of the triangle that we are deleting and adds each point to
one of 3 triangles. computes errors and updates triangles on the fly
*/
void insertPoints(Triangle t, Triangle *t1, Triangle *t2, Triangle *t3)
{
  while (t.head != NULL) {
    float err = 0;
    int errLoc = 0;
    
    Point3D p = t.head->p;
    
    
    if (isPointEqual(t.e, p) == 0) {
      if (isInside(p, t1->a, t1->b, t1->c, &errLoc)) {
        err = computeError(p, t1->a, t1->plane);
	
        if (err > t1->error) {
          t1->e = p;
          t1->error = err;
          t1->errLoc = errLoc;
        }
        push(&t1->head, &t1->tail, p);
      }
      else if (isInside(p, t2->a, t2->b, t2->c, &errLoc)) {
        err = computeError(p, t2->a, t2->plane);
	
        if (err > t2->error) {
          t2->e = p;
          t2->error = err;
          t2->errLoc = errLoc;
        }
        push(&t2->head, &t2->tail, p);
      }
      else if (isInside(p, t3->a, t3->b, t3->c, &errLoc)) {
        err = computeError(p, t3->a, t3->plane);
	
        if (err > t3->error) {
          t3->e = p;
          t3->error = err;
          t3->errLoc = errLoc;
        }
        push(&t3->head, &t3->tail, p);
      }
    }

    pop(&t.head, &t.tail);
  }
}

/*
additional function for initialzing triangles
*/
void initTrianglesHelper(Triangle *t)
{
  t->error = 0;
  t->e.x = 0;
  t->e.y = 0;
  t->e.z = 0;
  t->errLoc = 0;
  t->head = NULL;
  t->tail = NULL;
}

/*
  Depending on the edge that the error point lies on, we initialize two
  new triangles
*/
void initTrianglesCollinear(Triangle t, Triangle *t1, Triangle *t2)
{
  assert(t.errLoc);
  if (t.errLoc == AB) {
    t1->a = t.a;
    t1->b = t.e;
    t1->c = t.c;
    determinePlane(t1);
    initTrianglesHelper(t1);
    
    t2->a = t.e;
    t2->b = t.b;
    t2->c = t.c;
    determinePlane(t2);
    initTrianglesHelper(t2);
  }
  else if (t.errLoc == BC) {
    t1->a = t.a;
    t1->b = t.b;
    t1->c = t.e;
    determinePlane(t1);
    initTrianglesHelper(t1);
    
    t2->a = t.a;
    t2->b = t.e;
    t2->c = t.c;
    determinePlane(t2);
    initTrianglesHelper(t2);
  }
  else if (t.errLoc == CA) {
    t1->a = t.a;
    t1->b = t.b;
    t1->c = t.e;
    determinePlane(t1);
    initTrianglesHelper(t1);
    
    t2->a = t.e;
    t2->b = t.b;
    t2->c = t.c;
    determinePlane(t2);
    initTrianglesHelper(t2);
  }
}

/*
  When we are adding 3 new triangles, this function initializes those triangles
*/
void initTriangles(Triangle t, Triangle *t1, Triangle *t2, Triangle *t3)
{
  
  t1->a = t.a;
  t1->b = t.b;
  t1->c = t.e;
  determinePlane(t1);
  initTrianglesHelper(t1);
  
  t2->a = t.e;
  t2->b = t.b;
  t2->c = t.c;
  determinePlane(t2);
  initTrianglesHelper(t2);
  
  
  t3->a = t.a;
  t3->b = t.e;
  t3->c = t.c;
  determinePlane(t3);
  initTrianglesHelper(t3);
  
}


///////////////////////////////////////////////////////////////////////////////////

/*
  Initializes the simplified grid. The simpGrid data structure holds a standard grid
  that we use to render a simulation of the tin (interpolated points using plane)
  Also holds actual Tin--which is an array of triangles, as well as data about the Tin,
  i.e. number of triangles and number of points
*/
void initSimpGrid(SimpGrid *simp, Grid *grid)
{
  simp->newGrid.nrow = grid->nrow;
  simp->newGrid.ncol = grid->ncol;
  simp->newGrid.xllcorner = grid->xllcorner;
  simp->newGrid.yllcorner = grid->yllcorner;
  simp->newGrid.cellSize = grid->cellSize;
  simp->newGrid.noDataValue = grid->noDataValue;
  simp->newGrid.max = grid->max;
  simp->newGrid.min = grid->min;
  
  
  
  simp->newGrid.data = malloc(grid->nrow * sizeof(float*));
  
  for (int i = 0; i < grid->nrow; i++) {
    simp->newGrid.data[i] = malloc(grid->ncol * sizeof(float));
    
    for (int j = 0; j < grid->ncol; j++) {
      simp->newGrid.data[i][j] = grid->data[i][j];
    }
  }
  
  simp->numTriangles = 0;
  simp->numPoints = 0;
  
}

/*
  This function deletes the elements from the heap one by one and
  adds them to an array that is the tin
*/
void removeTinFromHeap(SimpGrid *simp, MaxHeap *hp)
{
  simp->numTriangles = hp->size;
  
  simp->tin = malloc(simp->numTriangles * sizeof(Triangle));
  
  for (int i = 0; i < simp->numTriangles; i++) {
    simp->tin[i] = hp->elem[0].t;
    deleteNode(hp);
  }
  
  deleteMaxHeap(hp);
}

/*
  Linear search of the tin for rendering the simulated/interpolated tin
*/
int searchTin(SimpGrid *simp, Point3D p)
{
  int errLoc = 0;
  
  for (int i = 0; i < simp->numTriangles; i++) {
    if (isPointEqual(p, simp->tin[i].a) || isPointEqual(p, simp->tin[i].b) || isPointEqual(p, simp->tin[i].c)) {
      return i;
    }
    else if (isInside(p, simp->tin[i].a, simp->tin[i].b, simp->tin[i].c, &errLoc)) {
      List *tmp = simp->tin[i].head;
      
      while (tmp) {
        if (isPointEqual(p, tmp->p)) {
          return i;
        }
        tmp = tmp->next;
      }
    }
  }
  //tin not found
  return -1;
}

/*
  checks to see if point p is located on edge of point a and point b. Function
  is called when p is collinear to ab
*/
int isOnEdge(Point3D p, Point3D a, Point3D b)
{

  if ((p.x >= a.x) && (p.x <= b.x) && (p.y >= a.y) && (p.y <= b.y)) {
    return 1;
  }
  if ((p.x >= b.x) && (p.x <= a.x) && (p.y >= a.y) && (p.y <= b.y)) {
    return 1;
  }
  if ((p.x >= b.x) && (p.x <= a.x) && (p.y >= b.y) && (p.y <= a.y)) {
    return 1;
  }
  if ((p.x >= a.x) && (p.x <= b.x) && (p.y >= b.y) && (p.y <= a.y)) {
    return 1;
  }
  return 0;
}

/*
  Function that checks to see if a point p is inside (or on edge) of
  triangle made by points a, b, c. errLoc is the location of the point. 
*/
int isInside(Point3D p, Point3D a, Point3D b, Point3D c, int *errLoc)
{
  int edge1 = left(a, b, p);
  int edge2 = left(b, c, p);
  int edge3 = left(c, a, p);
  
  //if every edge is to the right or every edge is to the left or colli
  if ((edge1 == edge2) && (edge1 == edge3) && (edge1 != -1)) {
    *errLoc = NORM;
    return 1;
  }
  if ((edge1 == -1) && isOnEdge(p, a, b)) {
    *errLoc = AB;
    return 1;
  }
  if ((edge2 == -1) && isOnEdge(p, b, c)) {
    *errLoc = BC;
    return 1;
  }
  if ((edge3 == -1) && isOnEdge(p, c, a)) {
    *errLoc = CA;
    return 1;
  }
  *errLoc = FALSE;
  return 0;
}


/*
  Goes through each point in original grid and computes the elevation
  that the tin estimates. Therefore, there are two scenarios for a given point.
  If the point is part of the tin, the elevation of that point is its actual elevation.
  However, if it is inside a triangle that is part of the tin, we use the equation of the plane
  of that triangle to interpolate the points elevation
*/
void computeEstimateGrid(SimpGrid *simp, Grid *grid)
{
  for (int i = 0; i < simp->newGrid.nrow; i++) {
    for (int j = 0; j < simp->newGrid.ncol; j++) {
      if (simp->newGrid.data[i][j] != simp->newGrid.noDataValue) {
        Point3D p;
        p.x = (float)j;
        p.y = (float)i;
        p.z = grid->data[i][j];
	
        int index = searchTin(simp, p);
        assert(index != -1);
	
        simp->newGrid.data[i][j] = computeEstimate(p, simp->tin[index].a, simp->tin[index].plane);
      }
    }
  }
}

/*
Black box for simplifying terrain
*/
void simplify(SimpGrid *simp, Grid *grid, MaxHeap *hp, float epsilon)
{
  createTin(simp, hp, epsilon);
  removeTinFromHeap(simp, hp);
}

/*
outputs tin array of triangles to file
*/
void tinToFile(SimpGrid *simp, char *out_path) 
{
  
  FILE *out_file = fopen(out_path, "w");
  if (!out_file)  {
    printf("gridToFile: cannot open output file %s\n", out_path);
    exit(1); 
  }
  
  //header
  fprintf(out_file, "nb points in TIN: %d\n", simp->numPoints);
  fprintf(out_file, "nb triangles in TIN: %d\n\n", simp->numTriangles);
  fprintf(out_file, "---------\n");
  fprintf(out_file, "point0_x point0_y point0_z\n");
  fprintf(out_file, "point1_x point1_y point1_z\n");
  fprintf(out_file, "point2_x point2_y point2_z\n");
  fprintf(out_file, "---------\n\n");
  
  for (int i = 0; i < simp->numTriangles; i++) {
    fprintf(out_file, "%.2f %.2f %.2f\n", simp->tin[i].a.x, simp->tin[i].a.y, simp->tin[i].a.z);
    fprintf(out_file, "%.2f %.2f %.2f\n", simp->tin[i].b.x, simp->tin[i].b.y, simp->tin[i].b.z);
    fprintf(out_file, "%.2f %.2f %.2f\n", simp->tin[i].c.x, simp->tin[i].c.y, simp->tin[i].c.z);
    fprintf(out_file, "\n");
  }
  
  fclose(out_file);
}





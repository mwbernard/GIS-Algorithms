
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

void initFlowGrids(Grid *grid, Grid *fdgrid, Grid *fagrid, Grid *vgrid)
{
  //assumes *grid has already been initialized

  fdgrid->nrow = grid->nrow;
  fdgrid->ncol = grid->ncol;
  fdgrid->xllcorner = grid->xllcorner;
  fdgrid->xllcorner = grid->yllcorner;
  fdgrid->cellSize = grid->cellSize;
  fdgrid->noDataValue = grid->noDataValue;
  fdgrid->max = grid->max;
  fdgrid->min = grid->min;

  fagrid->nrow = grid->nrow;
  fagrid->ncol = grid->ncol;
  fagrid->xllcorner = grid->xllcorner;
  fagrid->xllcorner = grid->yllcorner;
  fagrid->cellSize = grid->cellSize;
  fagrid->noDataValue = grid->noDataValue;
  fagrid->max = grid->max;
  fagrid->min = grid->min;

  vgrid->nrow = grid->nrow;
  vgrid->ncol = grid->ncol;
  vgrid->xllcorner = grid->xllcorner;
  vgrid->xllcorner = grid->yllcorner;
  vgrid->cellSize = grid->cellSize;
  vgrid->noDataValue = grid->noDataValue;
  vgrid->max = grid->max;
  vgrid->min = grid->min;
  
  fdgrid->data = malloc(fdgrid->nrow * sizeof(float*));
  fagrid->data = malloc(fagrid->nrow * sizeof(float*));
  vgrid->data = malloc(vgrid->nrow * sizeof(float*));
  
  
  for (int i = 0; i < grid->nrow; i++) {
    
    fdgrid->data[i] = malloc(fdgrid->ncol * sizeof(float));
    fagrid->data[i] = malloc(fagrid->ncol * sizeof(float));
    vgrid->data[i] = malloc(vgrid->ncol * sizeof(float));
    
    for (int j = 0; j < grid->ncol; j++) {
      if (grid->data[i][j] != grid->noDataValue) {
        fdgrid->data[i][j] = 1;
        fagrid->data[i][j] = 1;
        vgrid->data[i][j] = 0;
      }
      else {
        fdgrid->data[i][j] = grid->noDataValue;
        fagrid->data[i][j] = grid->noDataValue;
        vgrid->data[i][j] = 1;
      }
    }
  }


}


void computeFD(Grid *grid, Grid *fdgrid)
{
  /*
  For coding flow direction, flowing towards the...
  -------------
  |32 |64 |128|
  -------------
  |16 |X  |1  |
  -------------
  |8  |4  |2  |
  -------------
  */

  for (int i = 0; i < grid->nrow; i++) {
    for (int j = 0; j < grid->ncol; j++) {
      if (fdgrid->data[i][j] != grid->noDataValue) {
        float minElev = grid->data[i][j];
        float fd = 0;
	
        //Check Neighbors counter-clockwise

        //Middle Right
        checkNeighborFD(grid, i, j + 1, &minElev, &fd, 1);
        //Upper Right
        checkNeighborFD(grid, i - 1, j + 1, &minElev, &fd, 128);
        //Upper Middle
        checkNeighborFD(grid, i - 1, j, &minElev, &fd, 64);
        //Upper Left
        checkNeighborFD(grid, i - 1, j - 1, &minElev, &fd, 32);
        //Middle Left
        checkNeighborFD(grid, i, j - 1, &minElev, &fd, 16);
        //Lower Left
        checkNeighborFD(grid, i + 1, j - 1, &minElev, &fd, 8);
        //Lower Middle
        checkNeighborFD(grid, i + 1, j, &minElev, &fd, 4);
        //Lower Right
        checkNeighborFD(grid, i + 1, j + 1, &minElev, &fd, 2);

        fdgrid->data[i][j] = fd;
      }
    }
  }


}


float minFloat(float a, float b)
{
  if (a < b) {
    return a;
  }
  return b;
}

float maxFloat(float a, float b)
{
  if (a >= b) {
    return a;
  }
  return b;
}

/*
returns 1 (True) if ij coordinate is in bounds
and is not a noDataValue
*/

int isInBounds(Grid *grid, int i, int j)
{
  if ((i >= grid->nrow) || (i < 0) || (j >= grid->ncol) || (j < 0)) {
    return 0;
  }
  
  if (grid->data[i][j] == grid->noDataValue) {
    return 0;
  }
  return 1;
}

void checkNeighborFD(Grid *grid, int i, int j, float *minElev, float *fd, float code)
{
  if (isInBounds(grid, i, j)) {
    if (grid->data[i][j] < *minElev) {
      *minElev = grid->data[i][j];
      *fd = code;
    }
  }
}


void computeFA(Grid *grid, Grid *fdgrid, Grid *fagrid, Grid *vgrid)
{

  for (int i = 0; i < grid->nrow; i++) {
    for (int j = 0; j < grid->ncol; j++) {
      if ((fagrid->data[i][j] != grid->noDataValue) && (vgrid->data[i][j] == 0)) {
	fagrid->data[i][j] = individualFA(fdgrid, fagrid, vgrid, i, j);
      }
    }
  }
}


int checkNeighborFA(Grid *fdgrid, int i, int j, float code)
{
  if (isInBounds(fdgrid, i, j)) {
    if (fdgrid->data[i][j] == code) {
      return 1;
    }
    
  }
  return 0;
}

float individualFA(Grid *fdgrid, Grid *fagrid, Grid *vgrid, int i, int j)
{
  if (vgrid->data[i][j] == 1) {
    return fagrid->data[i][j];
  }

  vgrid->data[i][j] = 1;
  
  
  //middle right
  if (checkNeighborFA(fdgrid, i, j + 1, 16)) {
    fagrid->data[i][j] += individualFA(fdgrid, fagrid, vgrid, i, j + 1);
  }

  //top right
  if (checkNeighborFA(fdgrid, i - 1, j + 1, 8)) {
    fagrid->data[i][j] += individualFA(fdgrid, fagrid, vgrid, i - 1, j + 1);
  }

  //top middle
  if (checkNeighborFA(fdgrid, i - 1, j, 4)) {
    fagrid->data[i][j] += individualFA(fdgrid, fagrid, vgrid, i - 1, j);
  }

  //top left
  if (checkNeighborFA(fdgrid, i - 1, j - 1, 2)) {
    fagrid->data[i][j] += individualFA(fdgrid, fagrid, vgrid, i - 1, j - 1);
  }
  
  //middle left
  if (checkNeighborFA(fdgrid, i, j - 1, 1)) {
    fagrid->data[i][j] += individualFA(fdgrid, fagrid, vgrid, i, j - 1);
  }

  //bottom left
  if (checkNeighborFA(fdgrid, i + 1, j - 1, 128)) {
    fagrid->data[i][j] += individualFA(fdgrid, fagrid, vgrid, i + 1, j - 1);
  }
  
  //bottom middle
  if (checkNeighborFA(fdgrid, i + 1, j, 64)) {
    fagrid->data[i][j] += individualFA(fdgrid, fagrid, vgrid, i + 1, j);
  }

  //bottom right
  if (checkNeighborFA(fdgrid, i + 1, j + 1, 32)) {
    fagrid->data[i][j] += individualFA(fdgrid, fagrid, vgrid, i + 1, j + 1);
  }

  return fagrid->data[i][j];
  
}


void gridToFile(Grid *grid, char *out_path) {

  assert(grid && out_path); 
  
  FILE *out_file = fopen(out_path, "w");
  if (!out_file)  {
    printf("gridToFile: cannot open output file %s\n", out_path);
    exit(1); 
  }
  
  //header
  fprintf(out_file, "ncols\t%d\n", grid->ncol);
  fprintf(out_file, "nrows\t%d\n", grid->nrow);
  fprintf(out_file, "xllcorner\t%f\n", grid->xllcorner);
  fprintf(out_file, "yllcorner\t%f\n", grid->yllcorner);
  fprintf(out_file, "cellsize\t%f\n", grid->cellSize);
  fprintf(out_file, "NODATA_value\t%f\n", grid->noDataValue);

  for (int i = 0; i < grid->nrow; i++) {
    for (int j = 0; j < grid->ncol; j++) {
      fprintf(out_file, "%d ", (int)grid->data[i][j]);
    }
    fprintf(out_file, "\n");
  }
  
  fclose(out_file);
}

















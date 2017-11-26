
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




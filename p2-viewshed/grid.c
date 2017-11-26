
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

/////////////////////////////////////////////////////////////////////////////

void initVisGrid(Grid *grid, Grid *vg)
{

  vg->nrow = grid->nrow;
  vg->ncol = grid->ncol;
  vg->xllcorner = grid->xllcorner;
  vg->yllcorner = grid->yllcorner;
  vg->cellSize = grid->cellSize;
  vg->noDataValue = grid->noDataValue;
  vg->max = grid->max;
  vg->min = grid->min;

  vg->data = malloc(grid->nrow * sizeof(float*));

  for (int i = 0; i < grid->nrow; i++)
  {
    vg->data[i] = malloc(grid->ncol * sizeof(float));

    for (int j = 0; j < grid->ncol; j++)
    {
        vg->data[i][j] = 0;
    }
  }
}

void computeViewshed(Grid *grid, Grid *vg, int vprow, int vpcol)
{
  //if viewpoint is no data just return
  if (grid->data[vprow][vpcol] == grid->noDataValue)
  {
    return;
  }

  for (int i = 0; i < grid->nrow; i++)
  {
    for (int j = 0; j < grid->ncol; j++)
    {
      if ((vprow == i) && (vpcol == j))
      {
        vg->data[i][j] = 1;
      }
      else if (grid->data[i][j] != grid->noDataValue)
      {
        vg->data[i][j] = isVisible(grid, vprow, vpcol, i, j);
      }
    }
  }

}

float isVisible(Grid *grid, int vprow, int vpcol, int row, int col)
{
  //compute elev angle from viewpoint to point (i, j)

  float elevAngle = atan2(grid->data[row][col] - grid->data[vprow][vpcol], dist(vpcol, vprow, col, row));

  if (isVertical(vpcol, col))
  {
    for (int i = min(vprow, row) + 1; i < max(vprow, row) - 1; i++)
    {
      //if elevation angle is greater than original elevation angle
      //return 0;
      if (grid->data[i][col] == grid->noDataValue)
      {
        continue;
      }

      if (atan2(grid->data[i][col] - grid->data[vprow][vpcol], dist(vpcol, vprow, col, i)) > elevAngle)
      {
        return 0;
      }
    }
    return 1;
  }


  float xySlope = computeSlope(vpcol, col, vprow, row);

  //traversing over known rows aka "y" values

  for (int i = min(vprow, row) + 1; i < max(vprow, row) - 1; i++)
  {
    //interpolate to find elevation
    //find 2 x coordinates that point lies between

    float x = computeX(i, vprow, vpcol, xySlope);
    int xStart = (int)floor(x);
    int xEnd = xStart + 1;

    //now we have an exact y (row value), exact x and the range that x lies between
    //and we want to estimate the elevation so we will calculate
    //the slope between the two values

    float zStart = grid->data[i][xStart];
    float zEnd = grid->data[i][xEnd];

    if ((zStart == grid->noDataValue) || (zEnd == grid->noDataValue))
    {
      continue;
    }


    float xzSlope = computeSlope(xStart, xEnd, zStart, zEnd);

    float interElev = computeY(xStart, xEnd, zEnd, xzSlope);

    if (atan2(interElev - grid->data[vprow][vpcol], dist(vpcol, vprow, x, i)) > elevAngle)
    {
      return 0;
    }
  }


  //traversing over known columns aka "x" values

  for (int j = min(vpcol, col) + 1; j < max(vpcol, col) - 1; j++)
  {
    //interpolate to find elevation
    //find 2 y coordinates that point lies between

    float y = computeY(j, vpcol, vprow, xySlope);
    int yStart = (int)floor(y);
    int yEnd = yStart + 1;

    //now we have an exact x (column value), exact y and the range that y lies between
    //and we want to estimate the elevation so we will calculate
    //the slope between the two values

    float zStart = grid->data[yStart][j];
    float zEnd = grid->data[yEnd][j];

    if ((zStart == grid->noDataValue) || (zEnd == grid->noDataValue))
    {
      continue;
    }


    float yzSlope = computeSlope(yStart, yEnd, zStart, zEnd);

    float interElev = computeY(yStart, yEnd, zEnd, yzSlope);

    if (atan2(interElev - grid->data[vprow][vpcol], dist(vpcol, vprow, j, y)) > elevAngle)
    {
      return 0;
    }
  }

  return 1;

}

int isVertical(int vpcol, int col)
{
  if (vpcol == col)
  {
    return 1;
  }
  return 0;
}



float computeSlope(float x, float x1, float y, float y1)
{
  return (y1 - y) / (x1 - x);
}

float computeY(float x, float x1, float y1, float slope)
{
  //y - y1 = m(x - x1)
  //y = m(x - x1) + y1

  return slope * (x - x1) + y1;
}

float computeX(float y, float y1, float x1, float slope)
{
  //y - y1 = m(x - x1)
  //x = (y - y1 + x1m) / m

  return (y - y1 + (x1 * slope)) / slope;
}


int min(int a, int b)
{
  if (a < b)
  {
    return a;
  }
  return b;
}

int max(int a, int b)
{
  if (a >= b) {
    return a;
  }
  return b;
}

float dist(float x, float y, float x1, float y1)
{
  return sqrt(pow(x1 - x, 2) + pow(y1 - y, 2));
}

void gridToFile(Grid *grid, char *out_path) {

  assert(grid && out_path); 

  FILE *out_file = fopen(out_path, "w");
  if (!out_file)  {
    printf("gridToFile: cannot open output file %s\n", out_path);
    exit(1); 
  }
  
  //header
  fprintf(out_file, "nrow: %d\nncol: %d\nnoDataValue: %f\n\n", 
    grid->nrow, grid->ncol, grid->noDataValue);

  for (int i = 0; i < grid->nrow; i++) {
    for (int j = 0; j < grid->ncol; j++) {
      fprintf(out_file, "%d ", (int)grid->data[i][j]);
    }
    fprintf(out_file, "\n");
  }
  
  fclose(out_file);
}

void checkViewPoint(Grid *grid, int vprow, int vpcol)
{
  if (vprow < 0 || vprow >= grid->nrow || vpcol < 0 || vpcol >= grid->ncol)
  {
    printf("Viewpoint is outside the grid\n");
    exit(1);
  }
}





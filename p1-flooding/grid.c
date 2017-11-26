
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


void push(Node **head, Node **tail, int i, int j)
{
  Node* temp = (Node*)malloc(sizeof(Node));
  temp->i = i;
  temp->j = j;
  temp->next = NULL;
  
  if (*head == NULL && *tail == NULL) {
    *head = *tail = temp;
    return;
  }

  (*tail)->next = temp;
  *tail = temp;
}

void pop(Node **head, Node **tail)
{
  Node* temp = *head;
  
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

int isEmpty(Node *head)
{
  if (head == NULL) {
    return 1;
  }
  return 0;
}


/////////////////////////////////////////////////////////////////////////////

float isWater(float elev, float noDataValue, float seaLevel)
{
  if (elev == noDataValue) {
    return seaLevel;
  }
  return elev;
}

void findWater(Grid *grid)
{
  for (int i = 0; i < grid->nrow; i++) {
    for (int j = 0; j < grid->ncol; j++) {
      if (grid->data[i][j] <= 0) {
        grid->data[i][j] = grid->noDataValue;
      }
    }
  }
}

void flood(Grid *grid, float h)
{

  Node* head = NULL;
  Node* tail = NULL;
  
  int **visit = malloc(grid->nrow * sizeof(int*));
  
  initVisitGrid(grid, visit);
  
  searchBorder(grid, visit, &head, &tail);
  
  assert(head);
  
  floodHelper(grid, h, visit, &head, &tail);
  
}

void initVisitGrid(Grid *grid, int **visit)
{

  for (int i = 0; i < grid->nrow; i++) {
    visit[i] = malloc(grid->ncol * sizeof(int));
    
    for (int j = 0; j < grid->ncol; j++) {
      visit[i][j] = 0;
    }
  }  
}

void searchBorder(Grid *grid, int **visit, Node **head, Node **tail)
{

  for (int j = 0; j < grid->ncol; j++) {
    if (grid->data[0][j] == grid->noDataValue) {
      push(head, tail, 0, j);
      visit[0][j] = 1;
    }
    
  }
  
  for (int j = 0; j < grid->ncol; j++) {
    if (grid->data[grid->nrow - 1][j] == grid->noDataValue) {
      push(head, tail, grid->nrow - 1, j);
      visit[grid->nrow - 1][j] = 1;
    }
  }
  
  for (int i = 1; i < grid->nrow - 1; i++) {
    if (grid->data[i][0] == grid->noDataValue) {
      push(head, tail, i, 0);
      visit[i][0] = 1;
    }
  }

  for (int i = 1; i < grid->nrow - 1; i++) {
    if (grid->data[i][grid->ncol - 1] == grid->noDataValue) {
      push(head, tail, i, grid->ncol - 1);
      visit[i][grid->ncol - 1] = 1;
    }
  }
}

void floodHelper(Grid *grid, float h, int **visit, Node **head, Node **tail)
{
  while (*head)  {
    if (grid->data[(*head)->i][(*head)->j] == grid->noDataValue) {
      checkNeighbors(grid, visit, (*head)->i, (*head)->j, head, tail);
    }
    else if (grid->data[(*head)->i][(*head)->j] <= h) {
      grid->data[(*head)->i][(*head)->j] = grid->noDataValue;
      checkNeighbors(grid, visit, (*head)->i, (*head)->j, head, tail);
    }
    
    pop(head, tail);
    
  }
  
}

void checkNeighbors(Grid *grid, int **visit, int i, int j, Node **head, Node **tail)
{
  //middle right
  neighborHelper(grid, visit, i, j + 1, head, tail);
  
  //top right
  neighborHelper(grid, visit, i - 1, j + 1, head, tail);
  
  //top middle
  neighborHelper(grid, visit, i - 1, j, head, tail);

  //top left
  neighborHelper(grid, visit, i - 1, j - 1, head, tail);

  //middle left
  neighborHelper(grid, visit, i, j - 1, head, tail);

  //bottom left
  neighborHelper(grid, visit, i + 1, j - 1, head, tail);

  //bottom middle
  neighborHelper(grid, visit, i + 1, j, head, tail);

  //bottom right
  neighborHelper(grid, visit, i + 1, j + 1, head, tail);

}

void neighborHelper(Grid *grid, int **visit, int i, int j, Node **head, Node **tail)
{
  if (isInBounds(grid, visit, i, j)) {
    push(head, tail, i, j);
    visit[i][j] = 1;
  }
}

int isInBounds(Grid *grid, int **visit, int i, int j)
{
  if ((i < 0) || (i >= grid->nrow) || (j < 0) || (j >= grid->ncol)) {
    return 0;
  }
  if (visit[i][j]) {
    return 0;
  }
  return 1;
}










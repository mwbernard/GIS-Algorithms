#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grid.h"


//forward declarations 
void getArgs(int argc, char *argv[], char **file, int *multiplier);
void directions();

void getArgs(int argc, char *argv[], char **file, int *multiplier)
{
  if (argc <= 1) {
    directions();
    exit(1);
  }
  
  for (int i = 1; i < argc; ++i) {
    
    //check for options
        
    if (i + 1 >= argc) {printf("No argument provided to %s.\nExiting.\n", argv[i]); exit(1);}
        
    if (strcmp(argv[i], "-input") == 0) {
            
      *file = argv[++i];
      if (access(*file, F_OK) == -1) {
        
        printf("No such input file: %s\n", *file);
        exit(1);
      }
      continue;
    }

    if (strcmp(argv[i], "-mult") == 0) {
      if (sscanf(argv[++i], "%d", multiplier) != 1) {
	printf("error\n");
	exit(1);
      }  
      continue;
    }
    
    printf("Unknown argument '%s'\nExiting.", argv[i]);
    
    directions();
    exit(1);
    
  }
  
  
  //check arguments 
  if (*file == NULL) {
    printf("ERROR: must enter value for file\n");
    exit(1);
  }
  if (multiplier == NULL) {
    printf("ERROR: must enter value for multiplier\n");
    exit(1);
  }
}



void directions()
{
  printf("\ngridMultiply: \n");
  printf("\t-input <input file>\n");
  printf("\t-mult <multiplier constant>\n");
}




int main(int argc, char * argv[])
{
  char* gridFileName;  //input grid file
  int multiplier; 

  getArgs(argc, argv, &gridFileName, &multiplier);
  
  Grid *grid = malloc(sizeof(Grid));
  assert(grid);
  readGridFromFile(gridFileName, grid);
  printGrid(grid);
  printf("\nmultiplying grid by %d\n\n", multiplier);
  multiplyGrid(grid, multiplier);
  printGrid(grid);

  return 0;
}

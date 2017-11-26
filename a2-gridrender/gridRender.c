#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grid.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define DISCRETE 0
#define SMOOTH 1
#define GRAY 2

GLfloat red[3] = {1.0, 0.0, 0.0};
GLfloat green[3] = {0.0, 1.0, 0.0};
GLfloat blue[3] = {0.0, 0.0, 1.0};
GLfloat black[3] = {0.0, 0.0, 0.0};
GLfloat white[3] = {1.0, 1.0, 1.0};
GLfloat gray[3] = {0.5, 0.5, 0.5};
GLfloat yellow[3] = {1.0, 1.0, 0.0};
GLfloat magenta[3] = {1.0, 0.0, 1.0};
GLfloat cyan[3] = {0.0, 1.0, 1.0};

GLint fillmode = 0;


const int WINDOWSIZE = 500;
const int NUMCOLORS = 4;
GLfloat pos[3] = {0, 0, 0};
GLfloat theta[3] = {0, 0, 0};
int res = 1;
int res2 = 1;
int mag = 528;
Grid *grid;

int render = 0;


//forward declarations 
void getArgs(int argc, char *argv[], char **file);
void directions();
void getColor(float elev, Grid *grid, GLfloat *color, int render);
void gridRender(Grid *grid, int render);
void display(void);
int max(int a, int b);
void keypress(unsigned char key, int x, int y);

void getArgs(int argc, char *argv[], char **file)
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
    
    printf("Unknown argument '%s'\nExiting.", argv[i]);
    
    directions();
    exit(1);
    
  }
  
  
  //check arguments 
  if (*file == NULL) {
    printf("ERROR: must enter value for file\n");
    exit(1);
  }
}

int max(int a, int b)
{
  if (a >= b) {
    return a;
  }
  return b;
}

void directions()
{
  printf("\ngridRender: \n");
  printf("\t-input <input file>\n");
}

void getColor(float elev, Grid *grid, GLfloat *color, int render)
{

  if ((grid->max == grid->noDataValue) || (grid->max == grid->min)) {
    //black
    color[0] = 0.0;
    color[1] = 0.0;
    color[2] = 0.0;
    return;
  }
  
  if (render == GRAY)
  {
    float gray = (elev - grid->min) / (grid->max - grid->min);

    color[0] = gray;
    color[1] = gray;
    color[2] = gray;
    return;
  }
  
  float interval = (grid->max - grid->min) / NUMCOLORS;
  
  int genColor = (int)((elev - grid->min) / interval);

  float shade = .18 + (.75 * ((elev - grid->min) - (genColor * interval)) / interval);

  if (render == DISCRETE)
  {
    shade = 1;
  }
  switch (genColor) {
    //blue
  case 0:
    color[0] = 0.0;
    color[1] = 0.0;
    color[2] = shade;
    break;
    //cyan
  case 1:
    color[0] = 0.0;
    color[1] = shade;
    color[2] = shade;
    break;
    //green
  case 2:
    color[0] = 0.0;
    color[1] = shade;
    color[2] = 0.0;
    break;
    //yellow
  case 3:
    color[0] = shade;
    color[1] = shade;
    color[2] = 0.0;
    break;
  }
}



int main(int argc, char * argv[])
{
  char* gridFileName;  //input grid file 
  
  getArgs(argc, argv, &gridFileName);
  
  grid = (Grid*)malloc(sizeof(Grid));
  assert(grid);
  readGridFromFile(gridFileName, grid);
  findExtrema(grid);
  mag = (grid->max - grid->min) * 4;
  printf("press 'g' for gray\n");
  printf("press 'h' for discrete\n");
  printf("press 'j' for smooth\n");
  fflush(stdout); 

  /* initialize GLUT  */
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(WINDOWSIZE, WINDOWSIZE);
  glutInitWindowPosition(100,100);
  glutCreateWindow(argv[0]);
  
  /* register callback functions */
  glutDisplayFunc(display); 
  glutKeyboardFunc(keypress);

  /* init GL */
  /* set background color black*/
  glClearColor(0, 0, 0, 0);  
  /* here we can enable depth testing and double buffering and so
     on */
  /* give control to event handler */
  glutMainLoop();

  return 0;
}


void gridRender(Grid *grid, int render)
{
  assert(grid);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
  
  GLfloat *color = malloc(3 * sizeof(GLfloat));
  
  for (int i = 0; i < grid->nrow - res; i += res) {
    for (int j = 0; j < grid->ncol - res2; j += res2) {
      getColor(grid->data[i][j], grid, color, render);
      glColor3fv(color);
      glBegin(GL_TRIANGLES);
      glVertex2f(j, grid->nrow - 1 - i); 
      glVertex2f(j + res2, grid->nrow - 1 - i);
      glVertex2f(j, grid->nrow - 1 - i - res);
      glEnd();
      
      glBegin(GL_TRIANGLES);
      glVertex2f(j + res2, grid->nrow - 1 - i - res); 
      glVertex2f(j + res2, grid->nrow - 1 - i);
      glVertex2f(j, grid->nrow - 1 - i - res);
      glEnd();
    }
  }
}


/* ****************************** */
void display(void) {
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW); 
  glLoadIdentity(); //clear the matrix
  
  
  /* The default GL window is [-1,1]x[-1,1] with the origin in the
     center. 
     
     The points are in the range (0,0) to (WINSIZE,WINSIZE), so they
     need to be mapped to [-1,1]x [-1,1]x */
  glScalef(1.5/max(grid->nrow, grid->ncol), 1.5/max(grid->nrow, grid->ncol), 1.0);  
  glTranslatef(-grid->ncol/2, -grid->nrow/2, 0); 
  glTranslatef(pos[0], pos[1], pos[2]);
  gridRender(grid, render);
  
  
  /* execute the drawing commands */
  glFlush();
}

void keypress(unsigned char key, int x, int y) { 
  switch(key) { 
  case 'q': 
    exit(0); 
    break; 
    
  case 'd': 
    //move left  
    pos[0] -= 5;  //move left 5 cols  
    //glTranslatef(-5,0,0 );  
    glutPostRedisplay();  
    break;  
    
  case 'a':  
    //move right 
    pos[0] += 5; //move right 5 cols  
    // glTranslatef(5,0,0 );  
    glutPostRedisplay();  
    break; 
    
  case 's':  
    //move up  
    pos[1] += 5; //move up 5 rows  
    //glTranslatef(0,5,0 );  
    glutPostRedisplay();  
    break;  
    
  case 'w':  
    //move down  
    pos[1] -= 5; //move down 5 rows  
    //glTranslatef(0,-5,0);  
    glutPostRedisplay();  
    break;
    
  case 'f':  
    pos[2] += .2;  
    glutPostRedisplay();  
    break; 
    
  case 'b':
    pos[2] -= .2;  
    glutPostRedisplay();  
    break; 

  case 'x':  
    //glRotatef(5, 1,0,0);  
    theta[0] += .05;  
    glutPostRedisplay();  
    break;  
  case 'X':  
    theta[0] -= .05;  
    //glRotatef(-1, 1,0,0);  
    glutPostRedisplay();  
    break;  
    
  case 'y':  
    theta[1] += .05;  
    //glRotatef(5, 0,1,0);  
    glutPostRedisplay();  
    break;  
    
  case 'Y': 
    theta[1] -= .05;  
    //glRotatef(-5, 0,1,0);  
    glutPostRedisplay();  
    break;  
    
  case 'z': 
    //rotate around z 
    theta[2] += 5;  
    /* we are in object system of coordinates; the default rotation is 
       wrt the origin (ie lower left corner of the grid), but we want 
       to rotate wrt the middle of the grid 
    */ 
    //glTranslatef(ncols/2, nrows/2, 0);  
    //glRotatef(5, 0,0,1);  
    //glTranslatef(-ncols/2, -nrows/2, 0);  
    glutPostRedisplay();  
    break;  
    
  case 'Z': 
    //rotate around z 
    theta[2] -= 5;  
    //glTranslatef(ncols/2, nrows/2, 0);  
    //glRotatef(5, 0,0,-1);  
    //glTranslatef(-ncols/2, -nrows/2, 0);  
    glutPostRedisplay();  
    break;  
    
  case 'r':
    //decrease resolution
    res += grid->nrow/100;
    res2 += grid->ncol/100;
    glutPostRedisplay();
    break;
    
  case 'R':
    // increase resolution
    if (res >= grid->nrow/100) {
      res -= grid->nrow/100;
      res2 -= grid->ncol/100;
    }
    glutPostRedisplay();
    break;  

  case 'g':
    render = GRAY;
    glutPostRedisplay();
    break;

  case 'h':
    render = DISCRETE;
    glutPostRedisplay();
    break;

  case 'j':
    render = SMOOTH;
    glutPostRedisplay();
    break;
  } 
 }

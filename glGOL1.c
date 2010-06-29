/*
 * glGOL.c
 *  implementation of John Conway's Game of Life using openGL to display cells
 *  the board is toroidal in shape
 *  b3/s23
 *
 Copyright (c) 2010  Scott P. Powers

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glu.h>
#ifdef FREEGLUT
#include <GL/freeglut_ext.h>
#else
#include <GL/glut.h>
#endif


/*
 *  Macro definitions
 *
 *  boardSize defines the x and y dimentions of the board
 *  endsim is used to exit the program and print the number of 
 *   generations when ended
 */
#define boardSize 400
#define exitsim glutDestroyWindow(ident);printf("Simulation ended at generation: %u\n", generation);

#define cellSize 2


/*
 *global variables
 */
static _Bool Paused=0;
static _Bool cellBoard[boardSize][boardSize];
static _Bool cellBuffer[boardSize][boardSize];
static uint64_t nLiving=0;
static uint8_t renderLoop=4;
static unsigned int menuid;
static unsigned int ident;
static unsigned int subid;
static float rgb[3]={1.0,1.0,1.0};
static uint64_t generation=0;

/*
 *function prototypes
 */
void seed(void);
void toroidTick(void);
void update(void);
_Bool determineState(int neighbors, int x, int y);
void gameMain(void);
void render(void);
void windowInit(int size);
void createMenu(void);
void menu(int val);
void reshape(int x, int y);
void keyb(unsigned char k, int x, int y);
void gpuInfo(void);

/*
 *MAIN
 */
int main(int argcp, char **argv)
{
    int s=boardSize*cellSize;

    printf("John Conway's Game of Life\n");
    printf("Serial Execution Version with Toroidal Board\n");
    printf("by Scott P. Powers\n");
    /* gpuInfo();*/

    seed();

    glutInit(&argcp, argv);
    windowInit(s);
    glutMainLoop();
    exitsim;
    return 0;
}

/*windowInit
 *initialize the window with glut, bind keyboard input functions
 */
void windowInit(int s)
{
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(s,s);
    glutInitWindowPosition(0, 0 );

    /*create our window, with returned value = the identifier number for the window*/
    ident = glutCreateWindow("LIFE");

    createMenu();

    /*set background to black*/
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    /*set the callback functions*/
    glutIdleFunc(NULL);
    glutCloseFunc(glutLeaveMainLoop());
    glutDisplayFunc(gameMain);
    glutKeyboardFunc(keyb);
    glutReshapeFunc(reshape);
}

/*gpuInfo
 *prints information about your GPU and openGL version to STDOUT
 *
void gpuInfo(){
    printf("%s", (char *)glGetString(GL_RENDERER));
//    printf("Using %s %s Graphics Processor\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
}*/

/*gameMain:
 *main game loop that calls toroidTick to gen the new board state,
 *then update to move the buffer onto the board,
 *finally renders the board and sleeps for 100ms
 */
void gameMain()
{
    uint8_t ctr=renderLoop;
    if(Paused==0){
	toroidTick();
	update();
    }
    for (ctr;ctr>0; ctr--){
	render();
    }

}

/*render:
 *draws the cells that are living using openGL triangle strips an moves the render 
 * buffer as double buffering is used
 */
void render(){
    int n,m;
    int x,y; 
    
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    /*begin the openGL drawing*/
    glBegin(GL_QUADS);
    glColor3f(rgb[0],rgb[1],rgb[2]);

    for(n=0;n<boardSize; n++)
    {
        for(m=0; m<boardSize;m++)
        {
            if(cellBoard[n][m]==1)
            {
                x=m*cellSize;
                y=n*cellSize;

                glVertex2f(x,y);
                glVertex2f(x, cellSize+y);
		glVertex2f(cellSize+x, cellSize+y);
                glVertex2f(cellSize+x, y);
		
            }
        }
    }
    glEnd();

    /*
      swap the buffer that is being used to map pixels to the one used to 
      draw the actual screen
    */
    glutSwapBuffers();
    glutPostRedisplay();
}

/*createMenu:
 *creates the menu that is generated when the right mouse button is 
 * pressed using GLUT
 */
void createMenu(void)
{
    subid = glutCreateMenu(menu);
    glutAddMenuEntry("RED", 1);
    glutAddMenuEntry("GREEN", 2);
    glutAddMenuEntry("BLUE", 3);
    glutAddMenuEntry("WHITE", 4);

    menuid = glutCreateMenu(menu);
    glutAddMenuEntry("EXIT", 0);
    glutAddSubMenu("COLOR", subid);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

/*menu
 *provides the logic behind the menu allowing the color of the cells
 * to be changed, and the user to quit the game
 */
void menu(int val)
{
  if(val==0)
    {
	glutLeaveMainLoop();
    }
    else if(val==1)
    {
        /*set to red*/
        rgb[0]=1.0;
        rgb[1]=0.0;
        rgb[2]=0.0;
    }
    else if(val==2)
    {
        /*set to green*/
        rgb[0]=0.0;
        rgb[1]=0.6;
        rgb[2]=0.2;
    }
    else if(val==3)
    {
        /*set to blue*/
        rgb[0]=0.0;
        rgb[1]=0.0;
        rgb[2]=1.0;
    }
    else if(val==4)
    {
	/*set to white*/
        rgb[0]=1.0;
        rgb[1]=1.0;
        rgb[2]=1.0;
    }

  glutPostRedisplay();
}

/*keyb:
 *defines the action to be taken when a keypress occures using a 
 * switch statement
 */
void keyb(unsigned char k, int x, int y)
 {
     switch(k)
     {
        case 'q':
            glutLeaveMainLoop();
	    break;
        case 'Q':
            glutLeaveMainLoop();
	    break;
        case 'p':
	    if(Paused==1)Paused=0;
	    else Paused=1;
	    break;
     }
 }

 /*
  *RESPAPE
  *defines the viewport values when window is reshaped
  */
 void reshape(int x, int y)
 {
     glViewport(0, 0, x, y);
     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
     gluOrtho2D(0.0,x,0.0,y);
     glMatrixMode(GL_MODELVIEW);
     glutPostRedisplay();
 }

/*seed
 *randomly seeds the board with living cells
 */
void seed(void){
    int r, n, m;
    srand(time(NULL));
    for(n=0; n<boardSize; n++){
        for(m=0; m<boardSize; m++){
            r= rand()%3;
            if(r==0){
                cellBoard[n][m]=1;
            }
            else
            {
                cellBoard[n][m]=0;
            }
        }
      }
}


/*update:
 *moves the state stored in the buffer to the board 
 */
void update(void){
    int x, y;
    for(x=0; x<boardSize; x++){
        for(y=0; y<boardSize; y++){
            cellBoard[y][x]=cellBuffer[y][x];
        }
    }
}

/*toroidTick:
 *calculates the state of the board for the next time tick 
 */
void toroidTick(void)
{
    int neighbors=0;
    int x, y, tmpx[2], tmpy[2];
    nLiving = 0;

    for(y=0; y<boardSize; y++)
    {
         /*
	   calculate tmpy values, tmpy[0] is y-1 or wrapped around to the top edge. 
	   tmp[1] is y+1 or wrapped around to bottom edge of the board
	 */ 
            if(y-1<0)
            {
                tmpy[0]=boardSize-1;
            }
            else
            {
                tmpy[0]=y-1;
            }
            if(y+1==boardSize)
            {
                tmpy[1]=0;
            }
            else
            {
                tmpy[1]=y+1;
            }


/*
   calculate tmpx values, tmpx[0] is x-1 or wrapped around to the right edge. 
   tmpx[1] is x+1 or wrapped around to left edge
*/
        for(x=0; x<boardSize; x++)
        {
           if(x-1<0)
            {
                tmpx[0]=boardSize-1;
            }
            else
            {
                tmpx[0]=x-1;
            }
            if(x+1==boardSize)
            {
                tmpx[1]=0;
            }
            else
            {
                tmpx[1]=x+1;
            }



/*
locations to check for living cells 

    tmpx[0],tmpy[1] |   x,tmpy[1]   |   tmpx[1],tmpy[1]
    --------------------------------------------------------------------------
    tmpx[0],y          |   THIS CELL   |   tmpx[1],y
    --------------------------------------------------------------------------
    tmpx[0],tmpy[0] |   x,tmpy[0]   |   tmpx[1],tmpy[0]
*/

        /*row tmpy[1]*/
        if(cellBoard[tmpy[1]][tmpx[0]]!=0)
        {
            neighbors++;
        }
        if(cellBoard[tmpy[1]][x]!=0)
        {
            neighbors++;
        }
        if(cellBoard[tmpy[1]][tmpx[1]]!=0)
        {
            neighbors++;
        }

        /*row y*/
        if(cellBoard[y][tmpx[0]]!=0)
        {
            neighbors++;
        }
        if(cellBoard[y][tmpx[1]]!=0)
        {
            neighbors++;
        }

        /*row tmpy[0]*/
        if(cellBoard[tmpy[0]][tmpx[0]]!=0)
        {
            neighbors++;
        }
        if(cellBoard[tmpy[0]][x]!=0)
        {
            neighbors++;
        }
        if(cellBoard[tmpy[0]][tmpx[1]]!=0)
        {
            neighbors++;
        }

        cellBuffer[y][x]=determineState(neighbors,x, y);
        if(cellBuffer[y][x]!=0)
        {
            nLiving++;
        }
        neighbors=0;
        }
    }
    generation++;
}


/*determineState:
 *this determines if a cell dies, lives, or is spawned based on the B3/S23 rules and
 * returns a boolean 1 for living, 0 for dead
 *
 *B3 means a cell is born if it has 3 living neighbors
 *S23 stays alive if 2 or 3 living neighbors are present
 */
_Bool determineState(int neighbors, int x, int y)
{
     if(neighbors==2&&cellBoard[y][x]==1)
     {
       return 1;
     }
     else if(neighbors==3)
     {
        return 1;
     }
     else
     {
        return 0;
     }
}

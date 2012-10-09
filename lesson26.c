/*
 *		This Code Was Created By Pet & Commented/Cleaned Up By Jeff Molofee
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit NeHe Productions At http://nehe.gamedev.net
 */

#include <math.h>			// Header File For Windows Math Library
#include <stdio.h>			// Header File For Standard Input/Output
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>	// Header File For The OpenGL32 Library
#include <OpenGL/glu.h>	// Header File For The GLu32 Library
#else
#include <GL/gl.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
#endif
#include "SDL.h"

#define BOOL    int
#define FALSE   0
#define TRUE    1

Uint8*		keys;									// Key Array
BOOL		active=TRUE;								// Program's Active
BOOL		fullscreen=FALSE;							// Default Fullscreen To True

GLfloat		xrot,yrot,zrot,								// X, Y & Z Rotation
			xspeed,yspeed,zspeed,						// X, Y & Z Spin Speed
			cx,cy,cz=-15;								// X, Y & Z Position

int			key=1;										// Used To Make Sure Same Morph Key Is Not Pressed
int			step=0,steps=200;							// Step Counter And Maximum Number Of Steps
BOOL		morph=FALSE;								// Default morph To False (Not Morphing)

typedef struct											// Structure For 3D Points
{
	float	x, y, z;									// X, Y & Z Points
} VERTEX;												// Called VERTEX

typedef struct											// Structure For An Object
{
 int		verts;										// Number Of Vertices For The Object
 VERTEX		*points;									// One Vertice (Vertex x,y & z)
} OBJECT;												// Called OBJECT

int			maxver;										// Will Eventually Hold The Maximum Number Of Vertices
OBJECT		morph1,morph2,morph3,morph4,				// Our 4 Morphable Objects (morph1,2,3 & 4)
			helper,*sour,*dest;							// Helper Object, Source Object, Destination Object

void objallocate(OBJECT *k,int n)						// Allocate Memory For Each Object
{														// And Defines points
	k->points=(VERTEX*)malloc(sizeof(VERTEX)*n);		// Sets points Equal To VERTEX * Number Of Vertices
}														// (3 Points For Each Vertice)

void objfree(OBJECT *k)									// Frees The Object (Releasing The Memory)
{
	free(k->points);									// Frees Points
}

void readstr(FILE *f,char *string)						// Reads A String From File (f)
{
	do													// Do This
	{
		fgets(string, 255, f);							// Gets A String Of 255 Chars Max From f (File)
	} while ((string[0] == '/') || (string[0] == '\r') || (string[0] == '\n'));// Until End Of Line Is Reached
	return;												// Return
}

void objload(char *name,OBJECT *k)						// Loads Object From File (name)
{
	int		ver;										// Will Hold Vertice Count
	float	rx,ry,rz;									// Hold Vertex X, Y & Z Position
	FILE	*filein;									// Filename To Open
	char	oneline[255];								// Holds One Line Of Text (255 Chars Max)
	int i;

	filein = fopen(name, "rt");							// Opens The File For Reading Text In Translated Mode
														// CTRL Z Symbolizes End Of File In Translated Mode
	readstr(filein,oneline);							// Jumps To Code That Reads One Line Of Text From The File
	sscanf(oneline, "Vertices: %d\n", &ver);			// Scans Text For "Vertices: ".  Number After Is Stored In ver
	k->verts=ver;										// Sets Objects verts Variable To Equal The Value Of ver
	objallocate(k,ver);									// Jumps To Code That Allocates Ram To Hold The Object

	for (i=0;i<ver;i++)								// Loops Through The Vertices
	{
		readstr(filein,oneline);						// Reads In The Next Line Of Text
		sscanf(oneline, "%f %f %f", &rx, &ry, &rz);		// Searches For 3 Floating Point Numbers, Store In rx,ry & rz
		k->points[i].x = rx;							// Sets Objects (k) points.x Value To rx
		k->points[i].y = ry;							// Sets Objects (k) points.y Value To ry
		k->points[i].z = rz;							// Sets Objects (k) points.z Value To rz
	}
	fclose(filein);										// Close The File

	if(ver>maxver) maxver=ver;							// If ver Is Greater Than maxver Set maxver Equal To ver
}														// Keeps Track Of Highest Number Of Vertices Used In Any Of The
														// Objects
VERTEX calculate(int i)									// Calculates Movement Of Points During Morphing
{
	VERTEX a;											// Temporary Vertex Called a
	a.x=(sour->points[i].x-dest->points[i].x)/steps;	// a.x Value Equals Source x - Destination x Divided By Steps
	a.y=(sour->points[i].y-dest->points[i].y)/steps;	// a.y Value Equals Source y - Destination y Divided By Steps
	a.z=(sour->points[i].z-dest->points[i].z)/steps;	// a.z Value Equals Source z - Destination z Divided By Steps
	return a;											// Return The Results
}														// This Makes Points Move At A Speed So They All Get To Their
														// Destination At The Same Time
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	int i;

	glBlendFunc(GL_SRC_ALPHA,GL_ONE);					// Set The Blending Function For Translucency
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// This Will Clear The Background Color To Black
	glClearDepth(1.0);									// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);								// The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glShadeModel(GL_SMOOTH);							// Enables Smooth Color Shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	maxver=0;											// Sets Max Vertices To 0 By Default
	objload("Data26/Sphere.txt",&morph1);					// Load The First Object Into morph1 From File sphere.txt
	objload("Data26/Torus.txt",&morph2);					// Load The Second Object Into morph2 From File torus.txt
	objload("Data26/Tube.txt",&morph3);					// Load The Third Object Into morph3 From File tube.txt

	objallocate(&morph4,486);							// Manually Reserver Ram For A 4th 468 Vertice Object (morph4)
	for(i=0;i<486;i++)								// Loop Through All 468 Vertices
	{
		morph4.points[i].x=((float)(rand()%14000)/1000)-7;	// morph4 x Point Becomes A Random Float Value From -7 to 7
		morph4.points[i].y=((float)(rand()%14000)/1000)-7;	// morph4 y Point Becomes A Random Float Value From -7 to 7
		morph4.points[i].z=((float)(rand()%14000)/1000)-7;	// morph4 z Point Becomes A Random Float Value From -7 to 7
	}

	objload("Data26/Sphere.txt",&helper);					// Load sphere.txt Object Into Helper (Used As Starting Point)
	sour=dest=&morph1;									// Source & Destination Are Set To Equal First Object (morph1)

	return TRUE;										// Initialization Went OK
}

int DrawGLScene(GLvoid)								// Here's Where We Do All The Drawing
{
	GLfloat tx,ty,tz;									// Temp X, Y & Z Variables
	VERTEX q;											// Holds Returned Calculated Values For One Vertex
	int i;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity();									// Reset The View
	glTranslatef(cx,cy,cz);								// Translate The The Current Position To Start Drawing
	glRotatef(xrot,1,0,0);								// Rotate On The X Axis By xrot
	glRotatef(yrot,0,1,0);								// Rotate On The Y Axis By yrot
	glRotatef(zrot,0,0,1);								// Rotate On The Z Axis By zrot

	xrot+=xspeed; yrot+=yspeed; zrot+=zspeed;			// Increase xrot,yrot & zrot by xspeed, yspeed & zspeed

	glBegin(GL_POINTS);									// Begin Drawing Points
		for(i=0;i<morph1.verts;i++)					// Loop Through All The Verts Of morph1 (All Objects Have
		{												// The Same Amount Of Verts For Simplicity, Could Use maxver Also)
			if(morph) q=calculate(i); else q.x=q.y=q.z=0;	// If morph Is True Calculate Movement Otherwise Movement=0
			helper.points[i].x-=q.x;					// Subtract q.x Units From helper.points[i].x (Move On X Axis)
			helper.points[i].y-=q.y;					// Subtract q.y Units From helper.points[i].y (Move On Y Axis)
			helper.points[i].z-=q.z;					// Subtract q.z Units From helper.points[i].z (Move On Z Axis)
			tx=helper.points[i].x;						// Make Temp X Variable Equal To Helper's X Variable
			ty=helper.points[i].y;						// Make Temp Y Variable Equal To Helper's Y Variable
			tz=helper.points[i].z;						// Make Temp Z Variable Equal To Helper's Z Variable

			glColor3f(0,1,1);							// Set Color To A Bright Shade Of Off Blue
			glVertex3f(tx,ty,tz);						// Draw A Point At The Current Temp Values (Vertex)
			glColor3f(0,0.5f,1);						// Darken Color A Bit
			tx-=2*q.x; ty-=2*q.y; ty-=2*q.y;			// Calculate Two Positions Ahead
			glVertex3f(tx,ty,tz);						// Draw A Second Point At The Newly Calculate Position
			glColor3f(0,0,1);							// Set Color To A Very Dark Blue
			tx-=2*q.x; ty-=2*q.y; ty-=2*q.y;			// Calculate Two More Positions Ahead
			glVertex3f(tx,ty,tz);						// Draw A Third Point At The Second New Position
		}												// This Creates A Ghostly Tail As Points Move
	glEnd();											// Done Drawing Points

	// If We're Morphing And We Haven't Gone Through All 200 Steps Increase Our Step Counter
	// Otherwise Set Morphing To False, Make Source=Destination And Set The Step Counter Back To Zero.
	if(morph && step<=steps)step++; else { morph=FALSE; sour=dest; step=0;}

	return TRUE;
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	objfree(&morph1);									// Jump To Code To Release morph1 Allocated Ram
	objfree(&morph2);									// Jump To Code To Release morph2 Allocated Ram
	objfree(&morph3);									// Jump To Code To Release morph3 Allocated Ram
	objfree(&morph4);									// Jump To Code To Release morph4 Allocated Ram
	objfree(&helper);									// Jump To Code To Release helper Allocated Ram
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

BOOL CreateGLWindow(char* title, int width, int height, int bits, BOOL fullscreenflag)
{
	Uint32 flags;

	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag
	flags = SDL_OPENGL;
	if ( fullscreenflag ) {
		flags |= SDL_FULLSCREEN;
	}
	if ( SDL_SetVideoMode(width, height, 0, flags) == NULL ) {
		return FALSE;
	}
	SDL_WM_SetCaption(title, "opengl");

	ReSizeGLScene(width, height);								// Set Up Our Perspective GL Screen

	if (!InitGL())												// Initialize Our Newly Created GL Window
	{
		KillGLWindow();											// Reset The Display
		return FALSE;											// Return FALSE
	}

	return TRUE;												// Success
}

main(int argc, char *argv[])
{
	BOOL	done=FALSE;								// Bool Variable To Exit Loop

	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't init SDL: %s\n", SDL_GetError());
		return 1;
	}

	// Create Our OpenGL Window
	if (!CreateGLWindow("Piotr Cieslak & NeHe's Morphing Points Tutorial",640,480,16,fullscreen))
	{
		SDL_Quit();
		return 0;												// Quit If Window Was Not Created
	}

	while(!done)												// Loop That Runs While done=FALSE
	{
		SDL_Event event;
		while ( SDL_PollEvent(&event) ) {
			switch (event.type) {
				case SDL_QUIT:
					done=TRUE;							// If So done=TRUE
					break;
				default:
					break;
			}
		}
		keys = SDL_GetKeyState(NULL);

		// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
		if ((active && !DrawGLScene()) || keys[SDLK_ESCAPE])	// Active?  Was There A Quit Received?
		{
			done=TRUE;							// ESC or DrawGLScene Signalled A Quit
		}
		else									// Not Time To Quit, Update Screen
		{
			SDL_GL_SwapBuffers();					// Swap Buffers (Double Buffering)
		}

		if(keys[SDLK_PAGEUP])								// Is Page Up Being Pressed?
			zspeed+=0.01f;								// Increase zspeed

		if(keys[SDLK_PAGEDOWN])								// Is Page Down Being Pressed?
			zspeed-=0.01f;								// Decrease zspeed

		if(keys[SDLK_DOWN])								// Is Page Up Being Pressed?
			xspeed+=0.01f;								// Increase xspeed

		if(keys[SDLK_UP])									// Is Page Up Being Pressed?
			xspeed-=0.01f;								// Decrease xspeed

		if(keys[SDLK_RIGHT])								// Is Page Up Being Pressed?
			yspeed+=0.01f;								// Increase yspeed

		if(keys[SDLK_LEFT])								// Is Page Up Being Pressed?
			yspeed-=0.01f;								// Decrease yspeed

		if (keys[SDLK_q])									// Is Q Key Being Pressed?
		 cz-=0.01f;										// Move Object Away From Viewer

		if (keys[SDLK_z])									// Is Z Key Being Pressed?
		 cz+=0.01f;										// Move Object Towards Viewer

		if (keys[SDLK_w])									// Is W Key Being Pressed?
		 cy+=0.01f;										// Move Object Up

		if (keys[SDLK_s])									// Is S Key Being Pressed?
		 cy-=0.01f;										// Move Object Down

		if (keys[SDLK_d])									// Is D Key Being Pressed?
		 cx+=0.01f;										// Move Object Right

		if (keys[SDLK_a])									// Is A Key Being Pressed?
		 cx-=0.01f;										// Move Object Left

		if (keys[SDLK_1] && (key!=1) && !morph)			// Is 1 Pressed, key Not Equal To 1 And Morph False?
		{
			key=1;										// Sets key To 1 (To Prevent Pressing 1 2x In A Row)
			morph=TRUE;									// Set morph To True (Starts Morphing Process)
			dest=&morph1;								// Destination Object To Morph To Becomes morph1
		}
		if (keys[SDLK_2] && (key!=2) && !morph)			// Is 2 Pressed, key Not Equal To 2 And Morph False?
		{
			key=2;										// Sets key To 2 (To Prevent Pressing 2 2x In A Row)
			morph=TRUE;									// Set morph To True (Starts Morphing Process)
			dest=&morph2;								// Destination Object To Morph To Becomes morph2
		}
		if (keys[SDLK_3] && (key!=3) && !morph)			// Is 3 Pressed, key Not Equal To 3 And Morph False?
		{
			key=3;										// Sets key To 3 (To Prevent Pressing 3 2x In A Row)
			morph=TRUE;									// Set morph To True (Starts Morphing Process)
			dest=&morph3;								// Destination Object To Morph To Becomes morph3
		}
		if (keys[SDLK_4] && (key!=4) && !morph)			// Is 4 Pressed, key Not Equal To 4 And Morph False?
		{
			key=4;										// Sets key To 4 (To Prevent Pressing 4 2x In A Row)
			morph=TRUE;									// Set morph To True (Starts Morphing Process)
			dest=&morph4;								// Destination Object To Morph To Becomes morph4
		}

		if (keys[SDLK_F1])								// Is F1 Being Pressed?
		{
			KillGLWindow();								// Kill Our Current Window
			fullscreen=!fullscreen;						// Toggle Fullscreen / Windowed Mode
			// Recreate Our OpenGL Window
			if (!CreateGLWindow("Piotr Cieslak & NeHe's Morphing Points Tutorial",640,480,16,fullscreen))
			{
				done=TRUE;								// Quit If Window Was Not Created
			}
		}
	}

	// Shutdown
	KillGLWindow();												// Kill The Window
	SDL_Quit();
	return 0;													// Exit The Program
}

/*
 *		This Code Was Created By Jeff Molofee 2000
 *		And Modified By Giuseppe D'Agata (waveform@tiscalinet.it)
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
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


#define BOOL	int
#define FALSE	0
#define TRUE	1

Uint8 *keys;
BOOL	active=TRUE;		// Window Active Flag Set To TRUE By Default
BOOL	fullscreen=FALSE;	// Fullscreen Flag Set To Fullscreen Mode By Default

GLuint	base;				// Base Display List For The Font
GLuint	texture[2];			// Storage For Our Font Texture
GLuint	loop;				// Generic Loop Variable

GLfloat	cnt1;				// 1st Counter Used To Move Text & For Coloring
GLfloat	cnt2;				// 2nd Counter Used To Move Text & For Coloring


SDL_Surface *LoadBMP(char *filename)
{
	Uint8 *rowhi, *rowlo;
	Uint8 *tmpbuf, tmpch;
	SDL_Surface *image;
	int i, j;

	image = SDL_LoadBMP(filename);
	if ( image == NULL ) {
		fprintf(stderr, "Unable to load %s: %s\n", filename, SDL_GetError());
		return(NULL);
	}

	/* GL surfaces are upsidedown and RGB, not BGR :-) */
	tmpbuf = (Uint8 *)malloc(image->pitch);
	if ( tmpbuf == NULL ) {
		fprintf(stderr, "Out of memory\n");
		return(NULL);
	}
	rowhi = (Uint8 *)image->pixels;
	rowlo = rowhi + (image->h * image->pitch) - image->pitch;
	for ( i=0; i<image->h/2; ++i ) {
		for ( j=0; j<image->w; ++j ) {
			tmpch = rowhi[j*3];
			rowhi[j*3] = rowhi[j*3+2];
			rowhi[j*3+2] = tmpch;
			tmpch = rowlo[j*3];
			rowlo[j*3] = rowlo[j*3+2];
			rowlo[j*3+2] = tmpch;
		}
		memcpy(tmpbuf, rowhi, image->pitch);
		memcpy(rowhi, rowlo, image->pitch);
		memcpy(rowlo, tmpbuf, image->pitch);
		rowhi += image->pitch;
		rowlo -= image->pitch;
	}
	free(tmpbuf);
	return(image);
}

int LoadGLTextures()									// Load Bitmaps And Convert To Textures
{
		int Status=FALSE;							   // Status Indicator
		SDL_Surface *TextureImage[2];			   // Create Storage Space For The Textures
		memset(TextureImage,0,sizeof(void *)*2);		// Set The Pointer To NULL

		if ((TextureImage[0]=LoadBMP("Data17/Font.bmp")) &&
			(TextureImage[1]=LoadBMP("Data17/Bumps.bmp")))
		{
				Status=TRUE;							// Set The Status To TRUE
				glGenTextures(2, &texture[0]);		  // Create Two Texture

				for (loop=0; loop<2; loop++)
				{
					glBindTexture(GL_TEXTURE_2D, texture[loop]);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
					glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[loop]->w, TextureImage[loop]->h, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[loop]->pixels);
				}
		}
		for (loop=0; loop<2; loop++)
		{
			if (TextureImage[loop])							// If Texture Exists
			{
				SDL_FreeSurface(TextureImage[loop]);		// Free The Image Structure
			}
		}
		return Status;								  // Return The Status
}

GLvoid BuildFont(GLvoid)								// Build Our Font Display List
{
	float	cx;											// Holds Our X Character Coord
	float	cy;											// Holds Our Y Character Coord

	base=glGenLists(256);								// Creating 256 Display Lists
	glBindTexture(GL_TEXTURE_2D, texture[0]);			// Select Our Font Texture
	for (loop=0; loop<256; loop++)						// Loop Through All 256 Lists
	{
		cx=(float)(loop%16)/16.0f;						// X Position Of Current Character
		cy=(float)(loop/16)/16.0f;						// Y Position Of Current Character

		glNewList(base+loop,GL_COMPILE);				// Start Building A List
			glBegin(GL_QUADS);							// Use A Quad For Each Character
				glTexCoord2f(cx,1-cy-0.0625f);			// Texture Coord (Bottom Left)
				glVertex2i(0,0);						// Vertex Coord (Bottom Left)
				glTexCoord2f(cx+0.0625f,1-cy-0.0625f);	// Texture Coord (Bottom Right)
				glVertex2i(16,0);						// Vertex Coord (Bottom Right)
				glTexCoord2f(cx+0.0625f,1-cy);			// Texture Coord (Top Right)
				glVertex2i(16,16);						// Vertex Coord (Top Right)
				glTexCoord2f(cx,1-cy);					// Texture Coord (Top Left)
				glVertex2i(0,16);						// Vertex Coord (Top Left)
			glEnd();									// Done Building Our Quad (Character)
			glTranslated(10,0,0);						// Move To The Right Of The Character
		glEndList();									// Done Building The Display List
	}													// Loop Until All 256 Are Built
}

GLvoid KillFont(GLvoid)									// Delete The Font From Memory
{
	glDeleteLists(base,256);							// Delete All 256 Display Lists
}

GLvoid glPrint(GLint x, GLint y, char *string, int set)	// Where The Printing Happens
{
	if (set>1)
	{
		set=1;
	}
	glBindTexture(GL_TEXTURE_2D, texture[0]);			// Select Our Font Texture
	glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glPushMatrix();										// Store The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	glOrtho(0,640,0,480,-1,1);							// Set Up An Ortho Screen
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glPushMatrix();										// Store The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
	glTranslated(x,y,0);								// Position The Text (0,0 - Bottom Left)
	glListBase(base-32+(128*set));						// Choose The Font Set (0 or 1)
	glCallLists(strlen(string),GL_BYTE,string);			// Write The Text To The Screen
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glPopMatrix();										// Restore The Old Projection Matrix
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glPopMatrix();										// Restore The Old Projection Matrix
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}
	glViewport(0,0,width,height);						// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);	// Calculate Window Aspect Ratio
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	if (!LoadGLTextures())								// Jump To Texture Loading Routine
	{
		return FALSE;									// If Texture Didn't Load Return FALSE
	}
	BuildFont();										// Build The Font
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// Clear The Background Color To Black
	glClearDepth(1.0);									// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Test To Do
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);					// Select The Type Of Blending
	glShadeModel(GL_SMOOTH);							// Enables Smooth Color Shading
	glEnable(GL_TEXTURE_2D);							// Enable 2D Texture Mapping
	return TRUE;										// Initialization Went OK
}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity();									// Reset The Modelview Matrix
	glBindTexture(GL_TEXTURE_2D, texture[1]);			// Select Our Second Texture
	glTranslatef(0.0f,0.0f,-5.0f);						// Move Into The Screen 5 Units
	glRotatef(45.0f,0.0f,0.0f,1.0f);					// Rotate On The Z Axis 45 Degrees (Clockwise)
	glRotatef(cnt1*30.0f,1.0f,1.0f,0.0f);				// Rotate On The X & Y Axis By cnt1 (Left To Right)
	glDisable(GL_BLEND);								// Disable Blending Before We Draw In 3D
	glColor3f(1.0f,1.0f,1.0f);							// Bright White
	glBegin(GL_QUADS);									// Draw Our First Texture Mapped Quad
		glTexCoord2d(0.0f,0.0f);						// First Texture Coord
		glVertex2f(-1.0f, 1.0f);						// First Vertex
		glTexCoord2d(1.0f,0.0f);						// Second Texture Coord
		glVertex2f( 1.0f, 1.0f);						// Second Vertex
		glTexCoord2d(1.0f,1.0f);						// Third Texture Coord
		glVertex2f( 1.0f,-1.0f);						// Third Vertex
		glTexCoord2d(0.0f,1.0f);						// Fourth Texture Coord
		glVertex2f(-1.0f,-1.0f);						// Fourth Vertex
	glEnd();											// Done Drawing The First Quad
	glRotatef(90.0f,1.0f,1.0f,0.0f);					// Rotate On The X & Y Axis By 90 Degrees (Left To Right)
	glBegin(GL_QUADS);									// Draw Our Second Texture Mapped Quad
		glTexCoord2d(0.0f,0.0f);						// First Texture Coord
		glVertex2f(-1.0f, 1.0f);						// First Vertex
		glTexCoord2d(1.0f,0.0f);						// Second Texture Coord
		glVertex2f( 1.0f, 1.0f);						// Second Vertex
		glTexCoord2d(1.0f,1.0f);						// Third Texture Coord
		glVertex2f( 1.0f,-1.0f);						// Third Vertex
		glTexCoord2d(0.0f,1.0f);						// Fourth Texture Coord
		glVertex2f(-1.0f,-1.0f);						// Fourth Vertex
	glEnd();											// Done Drawing Our Second Quad
	glEnable(GL_BLEND);									// Enable Blending

	glLoadIdentity();									// Reset The View
	// Pulsing Colors Based On Text Position
	glColor3f(1.0f*(float)(cos(cnt1)),1.0f*(float)(sin(cnt2)),1.0f-0.5f*(float)(cos(cnt1+cnt2)));
	glPrint((int)((280+250*cos(cnt1))),(int)(235+200*sin(cnt2)),"NeHe",0);		// Print GL Text To The Screen

	glColor3f(1.0f*(float)(sin(cnt2)),1.0f-0.5f*(float)(cos(cnt1+cnt2)),1.0f*(float)(cos(cnt1)));
	glPrint((int)((280+230*cos(cnt2))),(int)(235+200*sin(cnt1)),"OpenGL",1);	// Print GL Text To The Screen

	glColor3f(0.0f,0.0f,1.0f);
	glPrint((int)(240+200*cos((cnt2+cnt1)/5)),2,"Giuseppe D'Agata",0);

	glColor3f(1.0f,1.0f,1.0f);
	glPrint((int)(242+200*cos((cnt2+cnt1)/5)),2,"Giuseppe D'Agata",0);

	cnt1+=0.01f;										// Increase The First Counter
	cnt2+=0.0081f;										// Increase The Second Counter
	return TRUE;										// Everything Went OK
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	KillFont();											// Kill The Font
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

	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

int main(int argc, char *argv[])
{
	BOOL	done=FALSE;								// Bool Variable To Exit Loop

	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't init SDL: %s\n", SDL_GetError());
		return 1;
	}

	// Create Our OpenGL Window
	if (!CreateGLWindow("NeHe & Giuseppe D'Agata's 2D Font Tutorial",640,480,16,fullscreen))
	{
		SDL_Quit();
		return 0;									// Quit If Window Was Not Created
	}

	while(!done)									// Loop That Runs While done=FALSE
	{
		SDL_Event event;
		while ( SDL_PollEvent(&event) ) {
			switch (event.type) {
				case SDL_QUIT:
					done=TRUE;							// If So done=TRUE
					break;

				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_F1)						// Is F1 Being Pressed?
					{
						KillGLWindow();						// Kill Our Current Window
						fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
						// Recreate Our OpenGL Window
						if (!CreateGLWindow("NeHe & Giuseppe D'Agata's 2D Font Tutorial",640,480,16,fullscreen))
						{
							done=TRUE;						// Quit If Window Was Not Created
						}
					}
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
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	SDL_Quit();
	return 0;
}

/*
 *		This Code Was Created By Jeff Molofee 2000
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 */

#include <stdio.h>					// Header File For Standard Input/Output
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

#define	MAX_PARTICLES	1000		// Number Of Particles To Create

#define BOOL    int
#define FALSE   0
#define TRUE    1

Uint8 *keys;
BOOL	active=TRUE;				// Window Active Flag Set To TRUE By Default
BOOL	fullscreen=FALSE;			// Fullscreen Flag Set To Fullscreen Mode By Default
BOOL	rainbow=TRUE;				// Rainbow Mode?
BOOL	sp;							// Spacebar Pressed?
BOOL	rp;							// Enter Key Pressed?

float	slowdown=2.0f;				// Slow Down Particles
float	xspeed;						// Base X Speed (To Allow Keyboard Direction Of Tail)
float	yspeed;						// Base Y Speed (To Allow Keyboard Direction Of Tail)
float	zoom=-40.0f;				// Used To Zoom Out

GLuint	loop;						// Misc Loop Variable
GLuint	col;						// Current Color Selection
GLuint	delay;						// Rainbow Effect Delay
GLuint	texture[1];					// Storage For Our Particle Texture

typedef struct						// Create A Structure For Particle
{
	BOOL	active;					// Active (Yes/No)
	float	life;					// Particle Life
	float	fade;					// Fade Speed
	float	r;						// Red Value
	float	g;						// Green Value
	float	b;						// Blue Value
	float	x;						// X Position
	float	y;						// Y Position
	float	z;						// Z Position
	float	xi;						// X Direction
	float	yi;						// Y Direction
	float	zi;						// Z Direction
	float	xg;						// X Gravity
	float	yg;						// Y Gravity
	float	zg;						// Z Gravity
}
particles;							// Particles Structure

particles particle[MAX_PARTICLES];	// Particle Array (Room For Particle Info)

static GLfloat colors[12][3]=		// Rainbow Of Colors
{
	{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
	{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
	{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
};

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

int LoadGLTextures()									// Load Bitmap And Convert To A Texture
{
        int Status=FALSE;								// Status Indicator
        SDL_Surface *TextureImage[1];				// Create Storage Space For The Textures
        memset(TextureImage,0,sizeof(void *)*1);		// Set The Pointer To NULL

        if (TextureImage[0]=LoadBMP("Data19/Particle.bmp"))	// Load Particle Texture
        {
			Status=TRUE;								// Set The Status To TRUE
			glGenTextures(1, &texture[0]);				// Create One Texture

			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->w, TextureImage[0]->h, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->pixels);
        }

        if (TextureImage[0])							// If Texture Exists
		{
			SDL_FreeSurface(TextureImage[0]);
		}
        return Status;									// Return The Status
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

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,200.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	if (!LoadGLTextures())								// Jump To Texture Loading Routine
	{
		return FALSE;									// If Texture Didn't Load Return FALSE
	}

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f,0.0f,0.0f,0.0f);					// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);							// Disable Depth Testing
	glEnable(GL_BLEND);									// Enable Blending
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);					// Type Of Blending To Perform
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);	// Really Nice Perspective Calculations
	glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);				// Really Nice Point Smoothing
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glBindTexture(GL_TEXTURE_2D,texture[0]);			// Select Our Texture

	for (loop=0;loop<MAX_PARTICLES;loop++)				// Initials All The Textures
	{
		particle[loop].active=TRUE;						// Make All The Particles Active
		particle[loop].life=1.0f;						// Give All The Particles Full Life
		particle[loop].fade=(float)(rand()%100)/1000.0f+0.003f;	// Random Fade Speed
		particle[loop].r=colors[(loop+1)/(MAX_PARTICLES/12)][0];	// Select Red Rainbow Color
		particle[loop].g=colors[(loop+1)/(MAX_PARTICLES/12)][1];	// Select Green Rainbow Color
		particle[loop].b=colors[(loop+1)/(MAX_PARTICLES/12)][2];	// Select Blue Rainbow Color
		particle[loop].xi=(float)((rand()%50)-26.0f)*10.0f;	// Random Speed On X Axis
		particle[loop].yi=(float)((rand()%50)-25.0f)*10.0f;	// Random Speed On Y Axis
		particle[loop].zi=(float)((rand()%50)-25.0f)*10.0f;	// Random Speed On Z Axis
		particle[loop].xg=0.0f;							// Set Horizontal Pull To Zero
		particle[loop].yg=-0.8f;						// Set Vertical Pull Downward
		particle[loop].zg=0.0f;							// Set Pull On Z Axis To Zero
	}

	return TRUE;										// Initialization Went OK
}

int DrawGLScene(GLvoid)										// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
	glLoadIdentity();										// Reset The ModelView Matrix

	for (loop=0;loop<MAX_PARTICLES;loop++)					// Loop Through All The Particles
	{
		if (particle[loop].active)							// If The Particle Is Active
		{
			float x=particle[loop].x;						// Grab Our Particle X Position
			float y=particle[loop].y;						// Grab Our Particle Y Position
			float z=particle[loop].z+zoom;					// Particle Z Pos + Zoom

			// Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
			glColor4f(particle[loop].r,particle[loop].g,particle[loop].b,particle[loop].life);

			glBegin(GL_TRIANGLE_STRIP);						// Build Quad From A Triangle Strip
			    glTexCoord2d(1,1); glVertex3f(x+0.5f,y+0.5f,z); // Top Right
				glTexCoord2d(0,1); glVertex3f(x-0.5f,y+0.5f,z); // Top Left
				glTexCoord2d(1,0); glVertex3f(x+0.5f,y-0.5f,z); // Bottom Right
				glTexCoord2d(0,0); glVertex3f(x-0.5f,y-0.5f,z); // Bottom Left
			glEnd();										// Done Building Triangle Strip

			particle[loop].x+=particle[loop].xi/(slowdown*1000);// Move On The X Axis By X Speed
			particle[loop].y+=particle[loop].yi/(slowdown*1000);// Move On The Y Axis By Y Speed
			particle[loop].z+=particle[loop].zi/(slowdown*1000);// Move On The Z Axis By Z Speed

			particle[loop].xi+=particle[loop].xg;			// Take Pull On X Axis Into Account
			particle[loop].yi+=particle[loop].yg;			// Take Pull On Y Axis Into Account
			particle[loop].zi+=particle[loop].zg;			// Take Pull On Z Axis Into Account
			particle[loop].life-=particle[loop].fade;		// Reduce Particles Life By 'Fade'

			if (particle[loop].life<0.0f)					// If Particle Is Burned Out
			{
				particle[loop].life=1.0f;					// Give It New Life
				particle[loop].fade=(float)(rand()%100)/1000.0f+0.003f;	// Random Fade Value
				particle[loop].x=0.0f;						// Center On X Axis
				particle[loop].y=0.0f;						// Center On Y Axis
				particle[loop].z=0.0f;						// Center On Z Axis
				particle[loop].xi=xspeed+(float)((rand()%60)-32.0f);	// X Axis Speed And Direction
				particle[loop].yi=yspeed+(float)((rand()%60)-30.0f);	// Y Axis Speed And Direction
				particle[loop].zi=(float)((rand()%60)-30.0f);	// Z Axis Speed And Direction
				particle[loop].r=colors[col][0];			// Select Red From Color Table
				particle[loop].g=colors[col][1];			// Select Green From Color Table
				particle[loop].b=colors[col][2];			// Select Blue From Color Table
			}

			// If Number Pad 8 And Y Gravity Is Less Than 1.5 Increase Pull Upwards
			if (keys[SDLK_KP8] && (particle[loop].yg<1.5f)) particle[loop].yg+=0.01f;

			// If Number Pad 2 And Y Gravity Is Greater Than -1.5 Increase Pull Downwards
			if (keys[SDLK_KP2] && (particle[loop].yg>-1.5f)) particle[loop].yg-=0.01f;

			// If Number Pad 6 And X Gravity Is Less Than 1.5 Increase Pull Right
			if (keys[SDLK_KP6] && (particle[loop].xg<1.5f)) particle[loop].xg+=0.01f;

			// If Number Pad 4 And X Gravity Is Greater Than -1.5 Increase Pull Left
			if (keys[SDLK_KP4] && (particle[loop].xg>-1.5f)) particle[loop].xg-=0.01f;

			if (keys[SDLK_TAB])										// Tab Key Causes A Burst
			{
				particle[loop].x=0.0f;								// Center On X Axis
				particle[loop].y=0.0f;								// Center On Y Axis
				particle[loop].z=0.0f;								// Center On Z Axis
				particle[loop].xi=(float)((rand()%50)-26.0f)*10.0f;	// Random Speed On X Axis
				particle[loop].yi=(float)((rand()%50)-25.0f)*10.0f;	// Random Speed On Y Axis
				particle[loop].zi=(float)((rand()%50)-25.0f)*10.0f;	// Random Speed On Z Axis
			}
		}
    }
	return TRUE;											// Everything Went OK
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
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

main(int argc, char *argv[])
{
	BOOL	done=FALSE;								// Bool Variable To Exit Loop

	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't init SDL: %s\n", SDL_GetError());
		return 1;
	}

	// Create Our OpenGL Window
	if (!CreateGLWindow("NeHe's Particle Tutorial",640,480,16,fullscreen))
	{
		SDL_Quit();
		return 0;									// Quit If Window Was Not Created
	}

	if (fullscreen)									// Are We In Fullscreen Mode
	{
		slowdown=1.0f;								// If So, Speed Up The Particles (3dfx Issue)
	}

	while(!done)									// Loop That Runs While done=FALSE
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

		if (keys[SDLK_PLUS] && (slowdown>1.0f)) slowdown-=0.01f;		// Speed Up Particles
		if (keys[SDLK_MINUS] && (slowdown<4.0f)) slowdown+=0.01f;	// Slow Down Particles

		if (keys[SDLK_PAGEUP])	zoom+=0.1f;		// Zoom In
		if (keys[SDLK_PAGEDOWN])	zoom-=0.1f;		// Zoom Out

		if (keys[SDLK_RETURN] && !rp)			// Return Key Pressed
		{
			rp=TRUE;						// Set Flag Telling Us It's Pressed
			rainbow=!rainbow;				// Toggle Rainbow Mode On / Off
		}
		if (!keys[SDLK_RETURN]) rp=FALSE;		// If Return Is Released Clear Flag
		
		if ((keys[SDLK_SPACE] && !sp) || (rainbow && (delay>25)))	// Space Or Rainbow Mode
		{
			if (keys[SDLK_SPACE])	rainbow=FALSE;	// If Spacebar Is Pressed Disable Rainbow Mode
			sp=TRUE;						// Set Flag Telling Us Space Is Pressed
			delay=0;						// Reset The Rainbow Color Cycling Delay
			col++;							// Change The Particle Color
			if (col>11)	col=0;				// If Color Is To High Reset It
		}
		if (!keys[SDLK_SPACE])	sp=FALSE;			// If Spacebar Is Released Clear Flag

		// If Up Arrow And Y Speed Is Less Than 200 Increase Upward Speed
		if (keys[SDLK_UP] && (yspeed<200)) yspeed+=1.0f;

		// If Down Arrow And Y Speed Is Greater Than -200 Increase Downward Speed
		if (keys[SDLK_DOWN] && (yspeed>-200)) yspeed-=1.0f;

		// If Right Arrow And X Speed Is Less Than 200 Increase Speed To The Right
		if (keys[SDLK_RIGHT] && (xspeed<200)) xspeed+=1.0f;

		// If Left Arrow And X Speed Is Greater Than -200 Increase Speed To The Left
		if (keys[SDLK_LEFT] && (xspeed>-200)) xspeed-=1.0f;

		delay++;							// Increase Rainbow Mode Color Cycling Delay Counter

		if (keys[SDLK_F1])						// Is F1 Being Pressed?
		{
			KillGLWindow();						// Kill Our Current Window
			fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
			// Recreate Our OpenGL Window
			if (!CreateGLWindow("NeHe's Particle Tutorial",640,480,16,fullscreen))
			{
				done=TRUE;						// Quit If Window Was Not Created
			}
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	SDL_Quit();
	return 0;
}


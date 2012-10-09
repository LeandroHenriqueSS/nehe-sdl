/*
 *		This Code Was Created By Jeff Molofee 2000
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 */

#include <math.h>			// Header File For Windows Math Library
#include <stdio.h>			// Header File For Standard Input/Output
#include <stdarg.h>
#include <string.h>
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

Uint8*		keys;										// Array Used For The Keyboard Routine
BOOL		active=TRUE;									// Window Active Flag Set To TRUE By Default
BOOL		fullscreen=FALSE;								// Fullscreen Flag Set To Fullscreen Mode By Default

int			scroll;											// Used For Scrolling The Screen
int			maxtokens;										// Keeps Track Of The Number Of Extensions Supported
int			swidth;											// Scissor Width
int			sheight;										// Scissor Height

GLuint		base;											// Base Display List For The Font

typedef struct												// Create A Structure
{
	GLubyte	*imageData;										// Image Data (Up To 32 Bits)
	GLuint	bpp;											// Image Color Depth In Bits Per Pixel.
	GLuint	width;											// Image Width
	GLuint	height;											// Image Height
	GLuint	texID;											// Texture ID Used To Select A Texture
} TextureImage;												// Structure Name

TextureImage textures[1];									// Storage For One Texture

BOOL LoadTGA(TextureImage *texture, char *filename)			// Loads A TGA File Into Memory
{    
	GLubyte		TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
	GLubyte		TGAcompare[12];								// Used To Compare TGA Header
	GLubyte		header[6];									// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;								// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;									// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;										// Temporary Variable
	GLuint		type=GL_RGBA;								// Set The Default GL Mode To RBGA (32 BPP)
	int i;

	FILE *file = fopen(filename, "rb");						// Open The TGA File

	if(	file==NULL ||										// Does File Even Exist?
		fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0				||	// Does The Header Match What We Want?
		fread(header,1,sizeof(header),file)!=sizeof(header))				// If So Read Next 6 Header Bytes
	{
		if (file == NULL)									// Did The File Even Exist? *Added Jim Strong*
			return FALSE;									// Return False
		else
		{
			fclose(file);									// If Anything Failed, Close The File
			return FALSE;									// Return False
		}
	}

	texture->width  = header[1] * 256 + header[0];			// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];			// Determine The TGA Height	(highbyte*256+lowbyte)
    
 	if(	texture->width	<=0	||								// Is The Width Less Than Or Equal To Zero
		texture->height	<=0	||								// Is The Height Less Than Or Equal To Zero
		(header[4]!=24 && header[4]!=32))					// Is The TGA 24 or 32 Bit?
	{
		fclose(file);										// If Anything Failed, Close The File
		return FALSE;										// Return False
	}

	texture->bpp	= header[4];							// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel	= texture->bpp/8;						// Divide By 8 To Get The Bytes Per Pixel
	imageSize		= texture->width*texture->height*bytesPerPixel;	// Calculate The Memory Required For The TGA Data

	texture->imageData=(GLubyte *)malloc(imageSize);		// Reserve Memory To Hold The TGA Data

	if(	texture->imageData==NULL ||							// Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file)!=imageSize)	// Does The Image Size Match The Memory Reserved?
	{
		if(texture->imageData!=NULL)						// Was Image Data Loaded
			free(texture->imageData);						// If So, Release The Image Data

		fclose(file);										// Close The File
		return FALSE;										// Return False
	}

	for(i=0; i<(int)(imageSize); i+=bytesPerPixel)		// Loop Through The Image Data
	{														// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp=texture->imageData[i];							// Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];	// Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;					// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose (file);											// Close The File

	// Build A Texture From The Data
	glGenTextures(1, &texture[0].texID);					// Generate OpenGL texture IDs

	glBindTexture(GL_TEXTURE_2D, texture[0].texID);			// Bind Our Texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Filtered
	
	if (texture[0].bpp==24)									// Was The TGA 24 Bits
	{
		type=GL_RGB;										// If So Set The 'type' To GL_RGB
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);

	return TRUE;											// Texture Building Went Ok, Return True
}

GLvoid BuildFont(GLvoid)									// Build Our Font Display List
{
	int loop1;
	base=glGenLists(256);									// Creating 256 Display Lists
	glBindTexture(GL_TEXTURE_2D, textures[0].texID);		// Select Our Font Texture
	for (loop1=0; loop1<256; loop1++)					// Loop Through All 256 Lists
	{
		float cx=(float)(loop1%16)/16.0f;						// X Position Of Current Character
		float cy=(float)(loop1/16)/16.0f;						// Y Position Of Current Character

		glNewList(base+loop1,GL_COMPILE);					// Start Building A List
			glBegin(GL_QUADS);								// Use A Quad For Each Character
				glTexCoord2f(cx,1.0f-cy-0.0625f);			// Texture Coord (Bottom Left)
				glVertex2d(0,16);							// Vertex Coord (Bottom Left)
				glTexCoord2f(cx+0.0625f,1.0f-cy-0.0625f);	// Texture Coord (Bottom Right)
				glVertex2i(16,16);							// Vertex Coord (Bottom Right)
				glTexCoord2f(cx+0.0625f,1.0f-cy-0.001f);	// Texture Coord (Top Right)
				glVertex2i(16,0);							// Vertex Coord (Top Right)
				glTexCoord2f(cx,1.0f-cy-0.001f);			// Texture Coord (Top Left)
				glVertex2i(0,0);							// Vertex Coord (Top Left)
			glEnd();										// Done Building Our Quad (Character)
			glTranslated(14,0,0);							// Move To The Right Of The Character
		glEndList();										// Done Building The Display List
	}														// Loop Until All 256 Are Built
}

GLvoid KillFont(GLvoid)										// Delete The Font From Memory
{
	glDeleteLists(base,256);								// Delete All 256 Display Lists
}

GLvoid glPrint(GLint x, GLint y, int set, const char *fmt, ...)	// Where The Printing Happens
{
	char		text[1024];									// Holds Our String
	va_list		ap;											// Pointer To List Of Arguments

	if (fmt == NULL)										// If There's No Text
		return;												// Do Nothing

	va_start(ap, fmt);										// Parses The String For Variables
	    vsprintf(text, fmt, ap);							// And Converts Symbols To Actual Numbers
	va_end(ap);												// Results Are Stored In Text

	if (set>1)												// Did User Choose An Invalid Character Set?
	{
		set=1;												// If So, Select Set 1 (Italic)
	}

	glEnable(GL_TEXTURE_2D);								// Enable Texture Mapping
	glLoadIdentity();										// Reset The Modelview Matrix
	glTranslated(x,y,0);									// Position The Text (0,0 - Top Left)
	glListBase(base-32+(128*set));							// Choose The Font Set (0 or 1)

	glScalef(1.0f,2.0f,1.0f);								// Make The Text 2X Taller

	glCallLists(strlen(text),GL_UNSIGNED_BYTE, text);		// Write The Text To The Screen
	glDisable(GL_TEXTURE_2D);								// Disable Texture Mapping
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)			// Resize And Initialize The GL Window
{
	swidth=width;											// Set Scissor Width To Window Width
	sheight=height;											// Set Scissor Height To Window Height
	if (height==0)											// Prevent A Divide By Zero By
	{
		height=1;											// Making Height Equal One
	}
	glViewport(0,0,width,height);							// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);							// Select The Projection Matrix
	glLoadIdentity();										// Reset The Projection Matrix
	glOrtho(0.0f,640,480,0.0f,-1.0f,1.0f);					// Create Ortho 640x480 View (0,0 At Top Left)
	glMatrixMode(GL_MODELVIEW);								// Select The Modelview Matrix
	glLoadIdentity();										// Reset The Modelview Matrix
}

int InitGL(GLvoid)											// All Setup For OpenGL Goes Here
{
	if (!LoadTGA(&textures[0],"Data25/Font.tga"))				// Load The Font Texture
	{
		return FALSE;										// If Loading Failed, Return False
	}

	BuildFont();											// Build The Font

	glShadeModel(GL_SMOOTH);								// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);					// Black Background
	glClearDepth(1.0f);										// Depth Buffer Setup
	glBindTexture(GL_TEXTURE_2D, textures[0].texID);		// Select Our Font Texture

	return TRUE;											// Initialization Went OK
}

int DrawGLScene(GLvoid)										// Here's Where We Do All The Drawing
{
	char	*token;											// Storage For Our Token
	char	*text;
	int		cnt=0;											// Local Counter Variable

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer

	glColor3f(1.0f,0.5f,0.5f);								// Set Color To Bright Red
	glPrint(50,16,1,"Renderer");							// Display Renderer
	glPrint(80,48,1,"Vendor");								// Display Vendor Name
	glPrint(66,80,1,"Version");								// Display Version

	glColor3f(1.0f,0.7f,0.4f);								// Set Color To Orange
	glPrint(200,16,1,(char *)glGetString(GL_RENDERER));		// Display Renderer
	glPrint(200,48,1,(char *)glGetString(GL_VENDOR));		// Display Vendor Name
	glPrint(200,80,1,(char *)glGetString(GL_VERSION));		// Display Version

	glColor3f(0.5f,0.5f,1.0f);								// Set Color To Bright Blue
	glPrint(192,432,1,"NeHe Productions");					// Write NeHe Productions At The Bottom Of The Screen

	glLoadIdentity();										// Reset The ModelView Matrix
	glColor3f(1.0f,1.0f,1.0f);								// Set The Color To White
	glBegin(GL_LINE_STRIP);									// Start Drawing Line Strips (Something New)
		glVertex2d(639,417);								// Top Right Of Bottom Box
		glVertex2d(  0,417);								// Top Left Of Bottom Box
		glVertex2d(  0,480);								// Lower Left Of Bottom Box
		glVertex2d(639,480);								// Lower Right Of Bottom Box
		glVertex2d(639,128);								// Up To Bottom Right Of Top Box
	glEnd();												// Done First Line Strip
	glBegin(GL_LINE_STRIP);									// Start Drawing Another Line Strip
		glVertex2d(  0,128);								// Bottom Left Of Top Box
		glVertex2d(639,128);								// Bottom Right Of Top Box								
		glVertex2d(639,  1);								// Top Right Of Top Box
		glVertex2d(  0,  1);								// Top Left Of Top Box
		glVertex2d(  0,417);								// Down To Top Left Of Bottom Box
	glEnd();												// Done Second Line Strip

	glScissor(1	,(int)(0.135416f*sheight),swidth-2,(int)(0.597916f*sheight));	// Define Scissor Region
	glEnable(GL_SCISSOR_TEST);								// Enable Scissor Testing

	text=(char *)malloc(strlen((char *)glGetString(GL_EXTENSIONS))+1);	// Allocate Memory For Our Extension String
	strcpy (text,(char *)glGetString(GL_EXTENSIONS));		// Grab The Extension List, Store In Text

	token=strtok(text," ");									// Parse 'text' For Words, Seperated By " " (spaces)
	while(token!=NULL)										// While The Token Isn't NULL
	{
		cnt++;												// Increase The Counter
		if (cnt>maxtokens)									// Is 'maxtokens' Less Than 'cnt'
		{
			maxtokens=cnt;									// If So, Set 'maxtokens' Equal To 'cnt'
		}

		glColor3f(0.5f,1.0f,0.5f);							// Set Color To Bright Green
		glPrint(0,96+(cnt*32)-scroll,0,"%i",cnt);			// Print Current Extension Number
		glColor3f(1.0f,1.0f,0.5f);							// Set Color To Yellow
		glPrint(50,96+(cnt*32)-scroll,0,token);				// Print The Current Token (Parsed Extension Name)
		token=strtok(NULL," ");								// Search For The Next Token
	}

	glDisable(GL_SCISSOR_TEST);								// Disable Scissor Testing

	free(text);												// Free Allocated Memory

	glFlush();												// Flush The Rendering Pipeline
	return TRUE;											// Everything Went OK
}

GLvoid KillGLWindow(GLvoid)									// Properly Kill The Window
{
	KillFont();												// Kill The Font
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
		fullscreen=TRUE;
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
	if (!CreateGLWindow("NeHe's Token, Extensions, Scissoring & TGA Loading Tutorial",640,480,16,fullscreen))
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

		if (keys[SDLK_F1])								// Is F1 Being Pressed?
		{
			KillGLWindow();								// Kill Our Current Window
			fullscreen=!fullscreen;						// Toggle Fullscreen / Windowed Mode
			// Recreate Our OpenGL Window
			if (!CreateGLWindow("NeHe's Token, Extensions, Scissoring & TGA Loading Tutorial",640,480,16,fullscreen))
			{
				done=TRUE;								// Quit If Window Was Not Created
			}
		}

		if (keys[SDLK_UP] && (scroll>0))					// Is Up Arrow Being Pressed?
		{
			scroll-=2;									// If So, Decrease 'scroll' Moving Screen Down
		}

		if (keys[SDLK_DOWN] && (scroll<32*(maxtokens-9)))	// Is Down Arrow Being Pressed?
		{
			scroll+=2;									// If So, Increase 'scroll' Moving Screen Up
		}
	}

	// Shutdown
	KillGLWindow();												// Kill The Window
	SDL_Quit();
	return 0;										// Exit The Program
}

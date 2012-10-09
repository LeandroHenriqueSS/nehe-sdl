/////////////////////////////////////////////////////////////////
//                                                             //
//  The OpenGL Basecode Used In This Project Was Created By	   //
//  Jeff Molofee ( NeHe ).  1997-2000.  If You Find This Code  //
//  Useful, Please Let Me Know.								   //
//                                                             //
//	Original Code & Tutorial Text By Andreas Löffler           //
//  Excellent Job Andreas!									   //
//                                                             //
//  Code Heavily Modified By Rob Fletcher ( rpf1@york.ac.uk )  //
//  Proper Image Structure, Better Blitter Code, Misc Fixes    //
//  Thanks Rob!												   //
//															   //
//	0% CPU Usage While Minimized Thanks To Jim Strong		   //
//  ( jim@scn.net ).  Thanks Jim!                              //
//                                                             //
//  This Code Also Has The ATI Fullscreen Fix!                 //
//                                                             //
//  Visit Me At nehe.gamedev.net							   //
//                                                             //
/////////////////////////////////////////////////////////////////

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

Uint8*		keys;											// Array Used For The Keyboard Routine
BOOL		active=TRUE;										// Window Active Flag Set To TRUE By Default
BOOL		fullscreen=FALSE;									// Fullscreen Flag Set To Fullscreen Mode By Default

GLfloat		xrot;												// X Rotation
GLfloat		yrot;												// Y Rotation
GLfloat		zrot;												// Z Rotation

GLuint		texture[1];											// Storage For 1 Texture

typedef struct
{
	int width;													// Width Of Image In Pixels
	int height;													// Height Of Image In Pixels
	int format;													// Number Of Bytes Per Pixel
	unsigned char *data;										// Texture Data
} TEXTURE_IMAGE;

typedef TEXTURE_IMAGE *P_TEXTURE_IMAGE;							// A Pointer To The Texture Image Data Type

P_TEXTURE_IMAGE t1;												// Pointer To The Texture Image Data Type
P_TEXTURE_IMAGE t2;												// Pointer To The Texture Image Data Type

// Allocate An Image Structure And Inside Allocate Its Memory Requirements
P_TEXTURE_IMAGE AllocateTextureBuffer( GLint w, GLint h, GLint f)
{
    P_TEXTURE_IMAGE ti=NULL;									// Pointer To Image Struct
    unsigned char *c=NULL;										// Pointer To Block Memory For Image

    ti = (P_TEXTURE_IMAGE)malloc(sizeof(TEXTURE_IMAGE));		// One Image Struct Please
    
    if( ti != NULL ) {
        ti->width  = w;											// Set Width
        ti->height = h;											// Set Height
        ti->format = f;											// Set Format
        c = (unsigned char *)malloc( w * h * f);
        if ( c != NULL ) {
            ti->data = c;
        }
        else {
			fprintf(stderr, "Could Not Allocate Memory For A Texture Buffer\n");
            return NULL;
        }
    }
    else
	{
		fprintf(stderr, "Could Not Allocate An Image Structure\n");
		return NULL;
    }
	return ti;													// Return Pointer To Image Struct
}

// Free Up The Image Data
void DeallocateTexture( P_TEXTURE_IMAGE t )
{
	if (t->data)
	{
		free( t->data );										// Free Its Image Buffer
	}

	if (t)
	{
		free(t);												// Free Itself
	}
}

// Read A .RAW File In To The Allocated Image Buffer Using Data In The Image Structure Header.
// Flip The Image Top To Bottom.  Returns 0 For Failure Of Read, Or Number Of Bytes Read.
int ReadTextureData ( char *filename, P_TEXTURE_IMAGE buffer)
{
	FILE *f;
	int i,j,k,done=0;
	int stride = buffer->width * buffer->format;				// Size Of A Row (Width * Bytes Per Pixel)
	unsigned char *p = NULL;

    f = fopen(filename, "rb");									// Open "filename" For Reading Bytes
    if( f != NULL )												// If File Exists
    {
		for( i = buffer->height-1; i >= 0 ; i-- )				// Loop Through Height (Bottoms Up - Flip Image)
		{
			p = buffer->data + (i * stride );					// 
			for ( j = 0; j < buffer->width ; j++ )				// Loop Through Width
			{
				for ( k = 0 ; k < buffer->format-1 ; k++, p++, done++ )
				{
					*p = fgetc(f);								// Read Value From File And Store In Memory
				}
				*p = 255; p++;									// Store 255 In Alpha Channel And Increase Pointer
			}
		}
		fclose(f);												// Close The File
	}
	else														// Otherwise
	{
		fprintf(stderr, "Unable To Open Image File: %s\n", filename);
    }
	return done;												// Returns Number Of Bytes Read In
}

void BuildTexture (P_TEXTURE_IMAGE tex)
{
	glGenTextures(1, &texture[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, tex->width, tex->height, GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
}

void Blit( P_TEXTURE_IMAGE src, P_TEXTURE_IMAGE dst, int src_xstart, int src_ystart, int src_width, int src_height,
           int dst_xstart, int dst_ystart, int blend, int alpha)
{
	int i,j,k;
	unsigned char *s, *d;										// Source & Destination

	// Clamp Alpha If Value Is Out Of Range
    if( alpha > 255 ) alpha = 255;
    if( alpha < 0 ) alpha = 0;

	// Check For Incorrect Blend Flag Values
    if( blend < 0 ) blend = 0;
    if( blend > 1 ) blend = 1;

    d = dst->data + (dst_ystart * dst->width * dst->format);    // Start Row - dst (Row * Width In Pixels * Bytes Per Pixel)
    s = src->data + (src_ystart * src->width * src->format);    // Start Row - src (Row * Width In Pixels * Bytes Per Pixel)

    for (i = 0 ; i < src_height ; i++ )							// Height Loop
    {
        s = s + (src_xstart * src->format);						// Move Through Src Data By Bytes Per Pixel
        d = d + (dst_xstart * dst->format);						// Move Through Dst Data By Bytes Per Pixel
        for (j = 0 ; j < src_width ; j++ )						// Width Loop
        {
            for( k = 0 ; k < src->format ; k++, d++, s++)		// "n" Bytes At A Time
            {
                if (blend)										// If Blending Is On
                    *d = ( (*s * alpha) + (*d * (255-alpha)) ) >> 8; // Multiply Src Data*alpha Add Dst Data*(255-alpha)
                else											// Keep in 0-255 Range With >> 8
                    *d = *s;									// No Blending Just Do A Straight Copy
            }
        }
        d = d + (dst->width - (src_width + dst_xstart))*dst->format;	// Add End Of Row */
        s = s + (src->width - (src_width + src_xstart))*src->format;	// Add End Of Row */
    }
}

int InitGL(GLvoid)												// This Will Be Called Right After The GL Window Is Created
{
    t1 = AllocateTextureBuffer( 256, 256, 4 );					// Get An Image Structure
    if (ReadTextureData("Data30/Monitor.raw",t1)==0)				// Fill The Image Structure With Data
	{															// Nothing Read?
		fprintf(stderr, "Could Not Read 'Monitor.raw' Image Data\n");
        return FALSE;
    }

    t2 = AllocateTextureBuffer( 256, 256, 4 );					// Second Image Structure
	if (ReadTextureData("Data30/GL.raw",t2)==0)					// Fill The Image Structure With Data
	{															// Nothing Read?
		fprintf(stderr, "Could Not Read 'GL.raw' Image Data\n");
        return FALSE;
    }

	// Image To Blend In, Original Image, Src Start X & Y, Src Width & Height, Dst Location X & Y, Blend Flag, Alpha Value
    Blit(t2,t1,127,127,128,128,64,64,1,127);					// Call The Blitter Routine

    BuildTexture (t1);											// Load The Texture Map Into Texture Memory

    DeallocateTexture( t1 );									// Clean Up Image Memory Because Texture Is
    DeallocateTexture( t2 );									// In GL Texture Memory Now

	glEnable(GL_TEXTURE_2D);									// Enable Texture Mapping

	glShadeModel(GL_SMOOTH);									// Enables Smooth Color Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);						// This Will Clear The Background Color To Black
	glClearDepth(1.0);											// Enables Clearing Of The Depth Buffer
	glEnable(GL_DEPTH_TEST);									// Enables Depth Testing
	glDepthFunc(GL_LESS);										// The Type Of Depth Test To Do

	return TRUE;
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
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

int DrawGLScene(GLvoid)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
	glLoadIdentity();										// Reset The View
	glTranslatef(0.0f,0.0f,-5.0f);

	glRotatef(xrot,1.0f,0.0f,0.0f);
	glRotatef(yrot,0.0f,1.0f,0.0f);
	glRotatef(zrot,0.0f,0.0f,1.0f);

	glBindTexture(GL_TEXTURE_2D, texture[0]);

	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		// Back Face
		glNormal3f( 0.0f, 0.0f,-1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		// Top Face
		glNormal3f( 0.0f, 1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		// Bottom Face
		glNormal3f( 0.0f,-1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		// Right Face
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		// Left Face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
	glEnd();

	xrot+=0.3f;
	yrot+=0.2f;
	zrot+=0.4f;

	return TRUE;
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
	int size;

	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag
	flags = SDL_OPENGL;
	if ( fullscreenflag ) {
		flags |= SDL_FULLSCREEN;
	}
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 1 );
	if ( SDL_SetVideoMode(width, height, 0, flags) == NULL ) {
		return FALSE;
	}
	SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &size);
	printf("Got a stencil buffer %d bits deep\n", size);

	SDL_WM_SetCaption(title, "opengl");

	ReSizeGLScene(width, height);						// Set Up Our Perspective GL Screen

	if (!InitGL())										// Initialize Our Newly Created GL Window
	{
		KillGLWindow();									// Reset The Display
		return FALSE;									// Return FALSE
	}

	return TRUE;										// Success
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
	if (!CreateGLWindow("Andreas Löffler, Rob Fletcher & NeHe's Blitter & Raw Image Loading Tutorial", 640, 480, 32, fullscreen))
	{
		SDL_Quit();
		return 0;										// Quit If Window Was Not Created
	}

	while(!done)										// Loop That Runs While done=FALSE
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
			if (!CreateGLWindow("Andreas Löffler, Rob Fletcher & NeHe's Blitter & Raw Image Loading Tutorial",640,480,16,fullscreen))
			{
				done=TRUE;								// Quit If Window Was Not Created
			}
		}
	}

	// Shutdown
	KillGLWindow();										// Kill The Window
	SDL_Quit();
	return 0;											// Exit The Program
}


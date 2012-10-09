/*		This Code Was Created by Jens Schneider (WizardSoft) 2000 
 *		Lesson22 to the series of OpenGL tutorials by NeHe-Production
 *
 *		This Code is loosely based upon Lesson06 by Jeff Molofee.
 *
 *		contact me at: schneide@pool.informatik.rwth-aachen.de
 *
 *		Basecode Was Created By Jeff Molofee 2000
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 */

// [HTML]: Point Out That Bump Maps Have To Be "Sharper" To Do Nice Effects!

#include <math.h>			// Header File For Windows Math Library
#include <string.h>			// Header File For The String Library
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>	// Header File For The OpenGL32 Library
#include <OpenGL/glu.h>	// Header File For The GLu32 Library
#else
#define __glext_h_  // Don't let gl.h include glext.h
#include <GL/gl.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
#undef __glext_h_
#endif
#include "Glext.h"	// Header File For Multitexturing
#include "SDL.h"

#define BOOL    int
#define FALSE   0
#define TRUE    1

#define MAX_EMBOSS (GLfloat)0.008f	// Maximum Emboss-Translate. Increase To Get Higher Immersion
									// At A Cost Of Lower Quality (More Artifacts Will Occur!)

/*	Here Comes The ARB-Multitexture Support.
 *	There Are (Optimally) 6 New Commands To The OpenGL Set:
 *	glMultiTexCoordifARB i=1..4	: Sets Texture-Coordinates For Texel-Pipeline #i
 *	glActiveTextureARB			: Sets Active Texel-Pipeline
 *	glClientActiveTextureARB	: Sets Active Texel-Pipeline For The Pointer-Array-Commands
 *
 *	There Are Even More For The Various Formats Of glMultiTexCoordi{f,fv,d,i}, But We Don't Need Them.
 */

#define __ARB_ENABLE TRUE											// Used To Disable ARB Extensions Entirely

#define EXT_INFO													// Do You Want To See Your Extensions At Start-Up?
#define MAX_EXTENSION_SPACE 10240									// Characters for Extension-Strings
#define MAX_EXTENSION_LENGTH 256									// Maximum Of Characters In One Extension-String
BOOL multitextureSupported=FALSE;									// Flag Indicating Whether Multitexturing Is Supported
BOOL useMultitexture=TRUE;											// Use It If It Is Supported?
GLint maxTexelUnits=1;												// Number Of Texel-Pipelines. This Is At Least 1.

PFNGLMULTITEXCOORD1FARBPROC		glMultiTexCoord1fARB_f	= NULL;
PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB_f	= NULL;
PFNGLMULTITEXCOORD3FARBPROC		glMultiTexCoord3fARB_f	= NULL;
PFNGLMULTITEXCOORD4FARBPROC		glMultiTexCoord4fARB_f	= NULL;
PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB_f	= NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTextureARB_f = NULL;	

Uint8*	keys;													// Array Used For The Keyboard Routine
BOOL	active=TRUE;												// Window Active Flag Set To TRUE By Default
BOOL	fullscreen=FALSE;											// Fullscreen Flag Set To Fullscreen Mode By Default
BOOL	emboss=FALSE;												// Emboss Only, No Basetexture?
BOOL    bumps=TRUE;													// Do Bumpmapping?

GLfloat	xrot;														// X Rotation
GLfloat	yrot;														// Y Rotation
GLfloat xspeed;														// X Rotation Speed
GLfloat yspeed;														// Y Rotation Speed
GLfloat	z=-5.0f;													// Depth Into The Screen

GLuint	filter=1;													// Which Filter To Use
GLuint	texture[3];													// Storage For 3 Textures
GLuint  bump[3];													// Our Bumpmappings
GLuint  invbump[3];													// Inverted Bumpmaps
GLuint	glLogo;														// Handle For OpenGL-Logo
GLuint  multiLogo;													// Handle For Multitexture-Enabled-Logo

GLfloat LightAmbient[]	= { 0.2f, 0.2f, 0.2f};						// Ambient Light is 20% white
GLfloat LightDiffuse[]	= { 1.0f, 1.0f, 1.0f};						// Diffuse Light is white
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f};						// Position is somewhat in front of screen

GLfloat Gray[]= {0.5f,0.5f,0.5f,1.0f};

// Data Contains The Faces For The Cube In Format 2xTexCoord, 3xVertex;
// Note That The Tesselation Of The Cube Is Only Absolute Minimum.
GLfloat data[]= {
		// FRONT FACE
		0.0f, 0.0f,		-1.0f, -1.0f, +1.0f,
		1.0f, 0.0f,		+1.0f, -1.0f, +1.0f,
		1.0f, 1.0f,		+1.0f, +1.0f, +1.0f,
		0.0f, 1.0f,		-1.0f, +1.0f, +1.0f,
		// BACK FACE
		1.0f, 0.0f,		-1.0f, -1.0f, -1.0f,
		1.0f, 1.0f,		-1.0f, +1.0f, -1.0f,
		0.0f, 1.0f,		+1.0f, +1.0f, -1.0f,
		0.0f, 0.0f,		+1.0f, -1.0f, -1.0f,
		// Top Face
		0.0f, 1.0f,		-1.0f, +1.0f, -1.0f,
		0.0f, 0.0f,		-1.0f, +1.0f, +1.0f,
		1.0f, 0.0f,		+1.0f, +1.0f, +1.0f,
		1.0f, 1.0f,		+1.0f, +1.0f, -1.0f,
		// Bottom Face
		1.0f, 1.0f,		-1.0f, -1.0f, -1.0f,
		0.0f, 1.0f,		+1.0f, -1.0f, -1.0f,
		0.0f, 0.0f,		+1.0f, -1.0f, +1.0f,
		1.0f, 0.0f,		-1.0f, -1.0f, +1.0f,
		// Right Face
		1.0f, 0.0f,		+1.0f, -1.0f, -1.0f,
		1.0f, 1.0f,		+1.0f, +1.0f, -1.0f,
		0.0f, 1.0f,		+1.0f, +1.0f, +1.0f,
		0.0f, 0.0f,		+1.0f, -1.0f, +1.0f,
		// Left Face
		0.0f, 0.0f,		-1.0f, -1.0f, -1.0f,
		1.0f, 0.0f,		-1.0f, -1.0f,  1.0f,
		1.0f, 1.0f,		-1.0f,  1.0f,  1.0f,
		0.0f, 1.0f,		-1.0f,  1.0f, -1.0f
};

// Always Check For Extension-Availability During Run-Time!
// Here We Go!
BOOL isInString(char *string, const char *search) {
#if 1
	return (strstr(string, search) != NULL);
#else // This crashes ??
	int i, pos=0;
	int maxpos=strlen(search)-1;
	int len=strlen(string);	
	char *other;
	for (i=0; i<len; i++) {
		if ((i==0) || ((i>1) && string[i-1]=='\n')) {				// New Extension Begins Here!
			other=&string[i];			
			pos=0;													// Begin New Search
			while (string[i]!='\n') {								// Search Whole Extension-String
				if (string[i]==search[pos]) pos++;					// Next Position
				if ((pos>maxpos) && string[i+1]=='\n') return TRUE; // We Have A Winner!
				i++;
			}			
		}
	}	
	return FALSE;													// Sorry, Not Found!
#endif
}

// isMultitextureSupported() Checks At Run-Time If Multitexturing Is Supported
BOOL initMultitexture(void) {
	char *extensions;	
	int i, len;
	extensions=strdup((char *) glGetString(GL_EXTENSIONS));			// Fetch Extension String
	len=strlen(extensions);
	for (i=0; i<len; i++)										// Separate It By Newline Instead Of Blank
		if (extensions[i]==' ') extensions[i]='\n';

#ifdef EXT_INFO
	printf("Supported GL extensions:\n%s\n", extensions);
#endif

	if (isInString(extensions,"GL_ARB_multitexture")				// Is Multitexturing Supported?
		&& __ARB_ENABLE												// Override-Flag
		&& isInString(extensions,"GL_EXT_texture_env_combine"))		// Is texture_env_combining Supported?
	{	
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB,&maxTexelUnits);
		glMultiTexCoord1fARB_f	= (PFNGLMULTITEXCOORD1FARBPROC)		SDL_GL_GetProcAddress("glMultiTexCoord1fARB");
		glMultiTexCoord2fARB_f	= (PFNGLMULTITEXCOORD2FARBPROC)		SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
		glMultiTexCoord3fARB_f	= (PFNGLMULTITEXCOORD3FARBPROC)		SDL_GL_GetProcAddress("glMultiTexCoord3fARB");
		glMultiTexCoord4fARB_f	= (PFNGLMULTITEXCOORD4FARBPROC)		SDL_GL_GetProcAddress("glMultiTexCoord4fARB");
		glActiveTextureARB_f	= (PFNGLACTIVETEXTUREARBPROC)		SDL_GL_GetProcAddress("glActiveTextureARB");
		glClientActiveTextureARB_f = (PFNGLCLIENTACTIVETEXTUREARBPROC)	SDL_GL_GetProcAddress("glClientActiveTextureARB");		
#ifdef EXT_INFO
		printf("The GL_ARB_multitexture extension will be used.\n");
#endif
		return TRUE;
	}
#ifdef EXT_INFO
	printf("** Environment combining not supported!\n");
#endif
	useMultitexture=FALSE;											// We Can't Use It If It Isn't Supported!
	return FALSE;
}

void initLights(void) {

	glLightfv( GL_LIGHT1, GL_AMBIENT, LightAmbient);				// Load Light-Parameters Into GL_LIGHT1
	glLightfv( GL_LIGHT1, GL_DIFFUSE, LightDiffuse);	
	glLightfv( GL_LIGHT1, GL_POSITION, LightPosition);

	glEnable(GL_LIGHT1);	
}

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

int LoadGLTextures()
{												// Load Bitmaps And Convert To Textures
	BOOL status=TRUE;												// Status Indicator
	SDL_Surface *Image=NULL;									// Create Storage Space For The Texture
	int i;

	// Load The Tile-Bitmap For Base-Texture
	if (Image=LoadBMP("Data22/Base.bmp")) {											
		glGenTextures(3, texture);									// Create Three Textures

		// Create Nearest Filtered Texture
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->w, Image->h, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->pixels);
		//                             ========
		// Use GL_RGB8 Instead Of "3" In glTexImage2D. Also Defined By GL: GL_RGBA8 Etc.
		// NEW: Now Creating GL_RGBA8 Textures, Alpha Is 1.0f Where Not Specified By Format.

		// Create Linear Filtered Texture
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->w, Image->h, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->pixels);

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, Image->w, Image->h, GL_RGB, GL_UNSIGNED_BYTE, Image->pixels);
	}
	else status=FALSE;
	if (Image) {													// If Texture Exists
		SDL_FreeSurface(Image);
		Image=NULL;
	}	
	
	// Load The Bumpmaps
	if (Image=LoadBMP("Data22/Bump.bmp")) {			
		glPixelTransferf(GL_RED_SCALE,0.5f);						// Scale RGB By 50%, So That We Have Only			
		glPixelTransferf(GL_GREEN_SCALE,0.5f);						// Half Intenstity
		glPixelTransferf(GL_BLUE_SCALE,0.5f);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);	// No Wrapping, Please!
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,Gray);

		glGenTextures(3, bump);										// Create Three Textures

		// Create Nearest Filtered Texture
		glBindTexture(GL_TEXTURE_2D, bump[0]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->w, Image->h, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->pixels);
		
		// Create Linear Filtered Texture
		glBindTexture(GL_TEXTURE_2D, bump[1]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->w, Image->h, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->pixels);

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, bump[2]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, Image->w, Image->h, GL_RGB, GL_UNSIGNED_BYTE, Image->pixels);
		
		for (i=0; i<3*Image->w*Image->h; i++)		// Invert The Bumpmap
			((Uint8 *)Image->pixels)[i]=255-((Uint8 *)Image->pixels)[i];

		glGenTextures(3, invbump);								// Create Three Textures

		// Create Nearest Filtered Texture
		glBindTexture(GL_TEXTURE_2D, invbump[0]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->w, Image->h, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->pixels);
		
		// Create Linear Filtered Texture
		glBindTexture(GL_TEXTURE_2D, invbump[1]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->w, Image->h, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->pixels);

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, invbump[2]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, Image->w, Image->h, GL_RGB, GL_UNSIGNED_BYTE, Image->pixels);
		
		glPixelTransferf(GL_RED_SCALE,1.0f);				// Scale RGB Back To 100% Again		
		glPixelTransferf(GL_GREEN_SCALE,1.0f);			
		glPixelTransferf(GL_BLUE_SCALE,1.0f);
	}
	else status=FALSE;
	if (Image) {											// If Texture Exists
		SDL_FreeSurface(Image);
	}

	// Load The Logo-Bitmaps
	if (Image=LoadBMP("Data22/opengl_alpha.bmp")) {							
		char alpha[4*Image->w*Image->h];		// Create Memory For RGBA8-Texture
		int a;
		for (a=0; a<Image->w*Image->h; a++)
			alpha[4*a+3]=((Uint8 *)Image->pixels)[a*3];					// Pick Only Red Value As Alpha!
		if (!(Image=LoadBMP("Data22/opengl.bmp"))) status=FALSE;
		for (a=0; a<Image->w*Image->h; a++) {		
			alpha[4*a]=((Uint8 *)Image->pixels)[a*3];					// R
			alpha[4*a+1]=((Uint8 *)Image->pixels)[a*3+1];				// G
			alpha[4*a+2]=((Uint8 *)Image->pixels)[a*3+2];				// B
		}
					
		glGenTextures(1, &glLogo);							// Create One Textures

		// Create Linear Filtered RGBA8-Texture
		glBindTexture(GL_TEXTURE_2D, glLogo);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Image->w, Image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, alpha);
	}
	else status=FALSE;
	if (Image) {											// If Texture Exists
		SDL_FreeSurface(Image);
		Image=NULL;
	}	
	
	// Load The "Extension Enabled"-Logo
	if (Image=LoadBMP("Data22/multi_on_alpha.bmp")) {							
		char alpha[4*Image->w*Image->h];		// Create Memory For RGBA8-Texture
		int a;
		for (a=0; a<Image->w*Image->h; a++)
			alpha[4*a+3]=((Uint8 *)Image->pixels)[a*3];					// Pick Only Red Value As Alpha!
		if (!(Image=LoadBMP("Data22/multi_on.bmp"))) status=FALSE;
		for (a=0; a<Image->w*Image->h; a++) {		
			alpha[4*a]=((Uint8 *)Image->pixels)[a*3];					// R
			alpha[4*a+1]=((Uint8 *)Image->pixels)[a*3+1];				// G
			alpha[4*a+2]=((Uint8 *)Image->pixels)[a*3+2];				// B
		}
					
		glGenTextures(1, &multiLogo);						// Create One Textures

		// Create Linear Filtered RGBA8-Texture
		glBindTexture(GL_TEXTURE_2D, multiLogo);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Image->w, Image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, alpha);
	}
	else status=FALSE;
	if (Image) {											// If Texture Exists
		SDL_FreeSurface(Image);
		Image=NULL;
	}		
	
	return status;											// Return The Status
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height) {		// Resize And Initialize The GL Window
	if (height==0)											// Prevent A Divide By Zero By
		height=1;											// Making Height Equal One

	glViewport(0,0,width,height);							// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);							// Select The Projection Matrix
	glLoadIdentity();										// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);								// Select The Modelview Matrix
	glLoadIdentity();										// Reset The Modelview Matrix
}

void doCube (void) {
	int i;
	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, +1.0f);
		for (i=0; i<4; i++) {
			glTexCoord2f(data[5*i],data[5*i+1]);
			glVertex3f(data[5*i+2],data[5*i+3],data[5*i+4]);
		}
		// Back Face
		glNormal3f( 0.0f, 0.0f,-1.0f);
		for (i=4; i<8; i++) {
			glTexCoord2f(data[5*i],data[5*i+1]);
			glVertex3f(data[5*i+2],data[5*i+3],data[5*i+4]);
		}
		// Top Face
		glNormal3f( 0.0f, 1.0f, 0.0f);
		for (i=8; i<12; i++) {
			glTexCoord2f(data[5*i],data[5*i+1]);
			glVertex3f(data[5*i+2],data[5*i+3],data[5*i+4]);
		}
		// Bottom Face
		glNormal3f( 0.0f,-1.0f, 0.0f);
		for (i=12; i<16; i++) {
			glTexCoord2f(data[5*i],data[5*i+1]);
			glVertex3f(data[5*i+2],data[5*i+3],data[5*i+4]);
		}
		// Right face
		glNormal3f( 1.0f, 0.0f, 0.0f);
		for (i=16; i<20; i++) {
			glTexCoord2f(data[5*i],data[5*i+1]);
			glVertex3f(data[5*i+2],data[5*i+3],data[5*i+4]);
		}
		// Left Face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		for (i=20; i<24; i++) {
			glTexCoord2f(data[5*i],data[5*i+1]);
			glVertex3f(data[5*i+2],data[5*i+3],data[5*i+4]);
		}
	glEnd();	
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	multitextureSupported=initMultitexture();
	if (!LoadGLTextures()) return FALSE;				// Jump To Texture Loading Routine
	
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	initLights();										// Initialize OpenGL Light	
	return TRUE;										// Initialization Went OK
}

// Calculates v=vM, M Is 4x4 In Column-Major, v Is 4dim. Row (i.e. "Transposed")
void VMatMult(GLfloat *M, GLfloat *v) {
	GLfloat res[3];
	res[0]=M[ 0]*v[0]+M[ 1]*v[1]+M[ 2]*v[2]+M[ 3]*v[3];
	res[1]=M[ 4]*v[0]+M[ 5]*v[1]+M[ 6]*v[2]+M[ 7]*v[3];
	res[2]=M[ 8]*v[0]+M[ 9]*v[1]+M[10]*v[2]+M[11]*v[3];;	
	v[0]=res[0];
	v[1]=res[1];
	v[2]=res[2];
	v[3]=M[15];											// Homogenous Coordinate
}

/*	Okay, Here Comes The Important Stuff:
	
	On http://www.nvidia.com/marketing/Developer/DevRel.nsf/TechnicalDemosFrame?OpenPage
	You Can Find A Demo Called GL_BUMP That Is A Little Bit More Complicated.
	GL_BUMP:   Copyright Diego Tártara, 1999.			
		     -  diego_tartara@ciudad.com.ar  -

	The Idea Behind GL_BUMP Is, That You Compute The Texture-Coordinate Offset As Follows:
		0) All Coordinates Either In Object Or In World Space.
		1) Calculate Vertex v From Actual Position (The Vertex You're At) To The Lightposition
		2) Normalize v
		3) Project This v Into Tangent Space.
			Tangent Space Is The Plane "Touching" The Object In Our Current Position On It.
			Typically, If You're Working With Flat Surfaces, This Is The Surface Itself.
		4) Offset s,t-Texture-Coordinates By The Projected v's x And y-Component.

	* This Would Be Called Once Per Vertex In Our Geometry, If Done Correctly.
	* This Might Lead To Incoherencies In Our Texture Coordinates, But Is Ok As Long As You Did Not
	* Wrap The Bumpmap.
		
	Basically, We Do It The Same Way With Some Exceptions:
		ad 0) We'll Work In Object Space All Time. This Has The Advantage That We'll Only
		      Have To Transform The Lightposition From Frame To Frame. This Position Obviously
			  Has To Be Transformed Using The Inversion Of The Modelview Matrix. This Is, However,
			  A Considerable Drawback, If You Don't Know How Your Modelview Matrix Was Built, Since
			  Inverting A Matrix Is Costly And Complicated.
		ad 1) Do It Exactly That Way.
		ad 2) Do It Exactly That Way.
		ad 3) To Project The Lightvector Into Tangent Space, We'll Support The Setup-Routine
			  With Two Directions: One Of Increasing s-Texture-Coordinate Axis, The Other In
			  Increasing t-Texture-Coordinate Axis. The Projection Simply Is (Assumed Both
			  texCoord Vectors And The Lightvector Are Normalized) The Dotproduct Between The
			  Respective texCoord Vector And The Lightvector. 
		ad 4) The Offset Is Computed By Taking The Result Of Step 3 And Multiplying The Two
			  Numbers With MAX_EMBOSS, A Constant That Specifies How Much Quality We're Willing To
			  Trade For Stronger Bump-Effects. Just Temper A Little Bit With MAX_EMBOSS!

	WHY THIS IS COOL:
		* Have A Look!
		* Very Cheap To Implement (About One Squareroot And A Couple Of MULs)!
		* Can Even Be Further Optimized!
		* SetUpBump Doesn't Disturb glBegin()/glEnd()
		* THIS DOES ALWAYS WORK - Not Only With XY-Tangent Spaces!!

	DRAWBACKS:
		* Must Know "Structure" Of Modelview-Matrix Or Invert It. Possible To Do The Whole Thing
		* In World Space, But This Involves One Transformation For Each Vertex!
*/	
	
void SetUpBumps(GLfloat *n, GLfloat *c, GLfloat *l, GLfloat *s, GLfloat *t) {
	GLfloat v[3];							// Vertex From Current Position To Light	
	GLfloat lenQ;							// Used To Normalize		
		
	// Calculate v From Current Vector c To Lightposition And Normalize v	
	v[0]=l[0]-c[0];		
	v[1]=l[1]-c[1];		
	v[2]=l[2]-c[2];		
	lenQ=(GLfloat) sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	v[0]/=lenQ;		v[1]/=lenQ;		v[2]/=lenQ;
	// Project v Such That We Get Two Values Along Each Texture-Coordinat Axis.
	c[0]=(s[0]*v[0]+s[1]*v[1]+s[2]*v[2])*MAX_EMBOSS;
	c[1]=(t[0]*v[0]+t[1]*v[1]+t[2]*v[2])*MAX_EMBOSS;	
}

void doLogo(void) {			// MUST CALL THIS LAST!!!, Billboards The Two Logos.
	glDepthFunc(GL_ALWAYS);		
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glLoadIdentity();	
	glBindTexture(GL_TEXTURE_2D,glLogo);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f);	glVertex3f(0.23f, -0.4f,-1.0f);
		glTexCoord2f(1.0f,0.0f);	glVertex3f(0.53f, -0.4f,-1.0f);
		glTexCoord2f(1.0f,1.0f);	glVertex3f(0.53f, -0.25f,-1.0f);
		glTexCoord2f(0.0f,1.0f);	glVertex3f(0.23f, -0.25f,-1.0f);
	glEnd();		
	if (useMultitexture) {
		glBindTexture(GL_TEXTURE_2D,multiLogo);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f);	glVertex3f(-0.53f, -0.4f,-1.0f);
			glTexCoord2f(1.0f,0.0f);	glVertex3f(-0.33f, -0.4f,-1.0f);
			glTexCoord2f(1.0f,1.0f);	glVertex3f(-0.33f, -0.3f,-1.0f);
			glTexCoord2f(0.0f,1.0f);	glVertex3f(-0.53f, -0.3f,-1.0f);
		glEnd();		
	}
	glDepthFunc(GL_LEQUAL);
}

BOOL doMesh1TexelUnits(void) {

	GLfloat c[4]={0.0f,0.0f,0.0f,1.0f};					// Holds Current Vertex
	GLfloat n[4]={0.0f,0.0f,0.0f,1.0f};					// Normalized Normal Of Current Surface		
	GLfloat s[4]={0.0f,0.0f,0.0f,1.0f};					// s-Texture Coordinate Direction, Normalized
	GLfloat t[4]={0.0f,0.0f,0.0f,1.0f};					// t-Texture Coordinate Direction, Normalized
	GLfloat l[4];										// Holds Our Lightposition To Be Transformed Into Object Space
	GLfloat Minv[16];									// Holds The Inverted Modelview Matrix To Do So.
	int i;								

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
		
	// Build Inverse Modelview Matrix First. This Substitutes One Push/Pop With One glLoadIdentity();
	// Simply Build It By Doing All Transformations Negated And In Reverse Order.
	glLoadIdentity();								
	glRotatef(-yrot,0.0f,1.0f,0.0f);
	glRotatef(-xrot,1.0f,0.0f,0.0f);
	glTranslatef(0.0f,0.0f,-z);
	glGetFloatv(GL_MODELVIEW_MATRIX,Minv);
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,z);

	glRotatef(xrot,1.0f,0.0f,0.0f);
	glRotatef(yrot,0.0f,1.0f,0.0f);	
	
	// Transform The Lightposition Into Object Coordinates:
	l[0]=LightPosition[0];
	l[1]=LightPosition[1];
	l[2]=LightPosition[2];
	l[3]=1.0f;											// Homogenous Coordinate
	VMatMult(Minv,l);
	
/*	PASS#1: Use Texture "Bump"
			No Blend
			No Lighting
			No Offset Texture-Coordinates */
	glBindTexture(GL_TEXTURE_2D, bump[filter]);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	doCube();

/* PASS#2:	Use Texture "Invbump"
			Blend GL_ONE To GL_ONE
			No Lighting
			Offset Texture Coordinates 
			*/
	glBindTexture(GL_TEXTURE_2D,invbump[filter]);
	glBlendFunc(GL_ONE,GL_ONE);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);	

	glBegin(GL_QUADS);	
		// Front Face	
		n[0]=0.0f;		n[1]=0.0f;		n[2]=1.0f;			
		s[0]=1.0f;		s[1]=0.0f;		s[2]=0.0f;
		t[0]=0.0f;		t[1]=1.0f;		t[2]=0.0f;
		for (i=0; i<4; i++) {	
			c[0]=data[5*i+2];		
			c[1]=data[5*i+3];
			c[2]=data[5*i+4];
			SetUpBumps(n,c,l,s,t);
			glTexCoord2f(data[5*i]+c[0], data[5*i+1]+c[1]); 
			glVertex3f(data[5*i+2], data[5*i+3], data[5*i+4]);
		}
		// Back Face	
		n[0]=0.0f;		n[1]=0.0f;		n[2]=-1.0f;	
		s[0]=-1.0f;		s[1]=0.0f;		s[2]=0.0f;
		t[0]=0.0f;		t[1]=1.0f;		t[2]=0.0f;
		for (i=4; i<8; i++) {	
			c[0]=data[5*i+2];		
			c[1]=data[5*i+3];
			c[2]=data[5*i+4];
			SetUpBumps(n,c,l,s,t);
			glTexCoord2f(data[5*i]+c[0], data[5*i+1]+c[1]); 
			glVertex3f(data[5*i+2], data[5*i+3], data[5*i+4]);
		}
		// Top Face	
		n[0]=0.0f;		n[1]=1.0f;		n[2]=0.0f;		
		s[0]=1.0f;		s[1]=0.0f;		s[2]=0.0f;
		t[0]=0.0f;		t[1]=0.0f;		t[2]=-1.0f;
		for (i=8; i<12; i++) {	
			c[0]=data[5*i+2];		
			c[1]=data[5*i+3];
			c[2]=data[5*i+4];
			SetUpBumps(n,c,l,s,t);
			glTexCoord2f(data[5*i]+c[0], data[5*i+1]+c[1]); 
			glVertex3f(data[5*i+2], data[5*i+3], data[5*i+4]);
		}
		// Bottom Face
		n[0]=0.0f;		n[1]=-1.0f;		n[2]=0.0f;		
		s[0]=-1.0f;		s[1]=0.0f;		s[2]=0.0f;
		t[0]=0.0f;		t[1]=0.0f;		t[2]=-1.0f;
		for (i=12; i<16; i++) {	
			c[0]=data[5*i+2];		
			c[1]=data[5*i+3];
			c[2]=data[5*i+4];
			SetUpBumps(n,c,l,s,t);
			glTexCoord2f(data[5*i]+c[0], data[5*i+1]+c[1]); 
			glVertex3f(data[5*i+2], data[5*i+3], data[5*i+4]);
		}
		// Right Face	
		n[0]=1.0f;		n[1]=0.0f;		n[2]=0.0f;		
		s[0]=0.0f;		s[1]=0.0f;		s[2]=-1.0f;
		t[0]=0.0f;		t[1]=1.0f;		t[2]=0.0f;
		for (i=16; i<20; i++) {	
			c[0]=data[5*i+2];		
			c[1]=data[5*i+3];
			c[2]=data[5*i+4];
			SetUpBumps(n,c,l,s,t);
			glTexCoord2f(data[5*i]+c[0], data[5*i+1]+c[1]); 
			glVertex3f(data[5*i+2], data[5*i+3], data[5*i+4]);
		}
		// Left Face
		n[0]=-1.0f;		n[1]=0.0f;		n[2]=0.0f;		
		s[0]=0.0f;		s[1]=0.0f;		s[2]=1.0f;
		t[0]=0.0f;		t[1]=1.0f;		t[2]=0.0f;
		for (i=20; i<24; i++) {	
			c[0]=data[5*i+2];		
			c[1]=data[5*i+3];
			c[2]=data[5*i+4];
			SetUpBumps(n,c,l,s,t);
			glTexCoord2f(data[5*i]+c[0], data[5*i+1]+c[1]); 
			glVertex3f(data[5*i+2], data[5*i+3], data[5*i+4]);
		}		
	glEnd();
	
/* PASS#3:	Use Texture "Base"
			Blend GL_DST_COLOR To GL_SRC_COLOR (Multiplies By 2)
			Lighting Enabled
			No Offset Texture-Coordinates
			*/
	if (!emboss) {
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D,texture[filter]);
		glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);	
		glEnable(GL_LIGHTING);
		doCube();
	}

	xrot+=xspeed;
	yrot+=yspeed;
	if (xrot>360.0f) xrot-=360.0f;
	if (xrot<0.0f) xrot+=360.0f;
	if (yrot>360.0f) yrot-=360.0f;
	if (yrot<0.0f) yrot+=360.0f;

/*	LAST PASS:	Do The Logos! */
	doLogo();
	
	return TRUE;										// Keep Going
}

BOOL doMesh2TexelUnits(void) {
	
	GLfloat c[4]={0.0f,0.0f,0.0f,1.0f};					// holds current vertex
	GLfloat n[4]={0.0f,0.0f,0.0f,1.0f};					// normalized normal of current surface		
	GLfloat s[4]={0.0f,0.0f,0.0f,1.0f};					// s-texture coordinate direction, normalized
	GLfloat t[4]={0.0f,0.0f,0.0f,1.0f};					// t-texture coordinate direction, normalized
	GLfloat l[4];										// holds our lightposition to be transformed into object space
	GLfloat Minv[16];									// holds the inverted modelview matrix to do so.
	int i;								

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
		
	// Build Inverse Modelview Matrix First. This Substitutes One Push/Pop With One glLoadIdentity();
	// Simply Build It By Doing All Transformations Negated And In Reverse Order.
	glLoadIdentity();								
	glRotatef(-yrot,0.0f,1.0f,0.0f);
	glRotatef(-xrot,1.0f,0.0f,0.0f);
	glTranslatef(0.0f,0.0f,-z);
	glGetFloatv(GL_MODELVIEW_MATRIX,Minv);
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,z);

	glRotatef(xrot,1.0f,0.0f,0.0f);
	glRotatef(yrot,0.0f,1.0f,0.0f);	

	// Transform The Lightposition Into Object Coordinates:
	l[0]=LightPosition[0];
	l[1]=LightPosition[1];
	l[2]=LightPosition[2];
	l[3]=1.0f;											// Homogenous Coordinate
	VMatMult(Minv,l);
		
/*	PASS#1: Texel-Unit 0:	Use Texture "Bump"
							No Blend
							No Lighting
							No Offset Texture-Coordinates 
							Texture-Operation "Replace"
			Texel-Unit 1:	Use Texture "Invbump"
							No Lighting
							Offset Texture Coordinates 
							Texture-Operation "Replace"
*/
	// TEXTURE-UNIT #0		
	glActiveTextureARB_f(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, bump[filter]);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);	
	// TEXTURE-UNIT #1:
	glActiveTextureARB_f(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, invbump[filter]);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
	// General Switches:
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);	
	glBegin(GL_QUADS);	
		// Front Face	
		n[0]=0.0f;		n[1]=0.0f;		n[2]=1.0f;			
		s[0]=1.0f;		s[1]=0.0f;		s[2]=0.0f;
		t[0]=0.0f;		t[1]=1.0f;		t[2]=0.0f;
		for (i=0; i<4; i++) {	
			c[0]=data[5*i+2];		
			c[1]=data[5*i+3];
			c[2]=data[5*i+4];
			SetUpBumps(n,c,l,s,t);
			glMultiTexCoord2fARB_f(GL_TEXTURE0_ARB,data[5*i]     , data[5*i+1]); 
			glMultiTexCoord2fARB_f(GL_TEXTURE1_ARB,data[5*i]+c[0], data[5*i+1]+c[1]); 
			glVertex3f(data[5*i+2], data[5*i+3], data[5*i+4]);
		}
		// Back Face	
		n[0]=0.0f;		n[1]=0.0f;		n[2]=-1.0f;	
		s[0]=-1.0f;		s[1]=0.0f;		s[2]=0.0f;
		t[0]=0.0f;		t[1]=1.0f;		t[2]=0.0f;
		for (i=4; i<8; i++) {	
			c[0]=data[5*i+2];		
			c[1]=data[5*i+3];
			c[2]=data[5*i+4];
			SetUpBumps(n,c,l,s,t);
			glMultiTexCoord2fARB_f(GL_TEXTURE0_ARB,data[5*i]     , data[5*i+1]); 
			glMultiTexCoord2fARB_f(GL_TEXTURE1_ARB,data[5*i]+c[0], data[5*i+1]+c[1]); 
			glVertex3f(data[5*i+2], data[5*i+3], data[5*i+4]);
		}
		// Top Face	
		n[0]=0.0f;		n[1]=1.0f;		n[2]=0.0f;		
		s[0]=1.0f;		s[1]=0.0f;		s[2]=0.0f;
		t[0]=0.0f;		t[1]=0.0f;		t[2]=-1.0f;
		for (i=8; i<12; i++) {	
			c[0]=data[5*i+2];		
			c[1]=data[5*i+3];
			c[2]=data[5*i+4];
			SetUpBumps(n,c,l,s,t);
			glMultiTexCoord2fARB_f(GL_TEXTURE0_ARB,data[5*i]     , data[5*i+1]     ); 
			glMultiTexCoord2fARB_f(GL_TEXTURE1_ARB,data[5*i]+c[0], data[5*i+1]+c[1]); 
			glVertex3f(data[5*i+2], data[5*i+3], data[5*i+4]);
		}
		// Bottom Face
		n[0]=0.0f;		n[1]=-1.0f;		n[2]=0.0f;		
		s[0]=-1.0f;		s[1]=0.0f;		s[2]=0.0f;
		t[0]=0.0f;		t[1]=0.0f;		t[2]=-1.0f;
		for (i=12; i<16; i++) {	
			c[0]=data[5*i+2];		
			c[1]=data[5*i+3];
			c[2]=data[5*i+4];
			SetUpBumps(n,c,l,s,t);
			glMultiTexCoord2fARB_f(GL_TEXTURE0_ARB,data[5*i]     , data[5*i+1]     ); 
			glMultiTexCoord2fARB_f(GL_TEXTURE1_ARB,data[5*i]+c[0], data[5*i+1]+c[1]); 
			glVertex3f(data[5*i+2], data[5*i+3], data[5*i+4]);
		}
		// Right Face	
		n[0]=1.0f;		n[1]=0.0f;		n[2]=0.0f;		
		s[0]=0.0f;		s[1]=0.0f;		s[2]=-1.0f;
		t[0]=0.0f;		t[1]=1.0f;		t[2]=0.0f;
		for (i=16; i<20; i++) {	
			c[0]=data[5*i+2];		
			c[1]=data[5*i+3];
			c[2]=data[5*i+4];
			SetUpBumps(n,c,l,s,t);
			glMultiTexCoord2fARB_f(GL_TEXTURE0_ARB,data[5*i]     , data[5*i+1]     ); 
			glMultiTexCoord2fARB_f(GL_TEXTURE1_ARB,data[5*i]+c[0], data[5*i+1]+c[1]); 
			glVertex3f(data[5*i+2], data[5*i+3], data[5*i+4]);
		}
		// Left Face
		n[0]=-1.0f;		n[1]=0.0f;		n[2]=0.0f;		
		s[0]=0.0f;		s[1]=0.0f;		s[2]=1.0f;
		t[0]=0.0f;		t[1]=1.0f;		t[2]=0.0f;
		for (i=20; i<24; i++) {	
			c[0]=data[5*i+2];		
			c[1]=data[5*i+3];
			c[2]=data[5*i+4];
			SetUpBumps(n,c,l,s,t);
			glMultiTexCoord2fARB_f(GL_TEXTURE0_ARB,data[5*i]     , data[5*i+1]     ); 
			glMultiTexCoord2fARB_f(GL_TEXTURE1_ARB,data[5*i]+c[0], data[5*i+1]+c[1]); 
			glVertex3f(data[5*i+2], data[5*i+3], data[5*i+4]);
		}		
	glEnd();
	
/* PASS#2	Use Texture "Base"
			Blend GL_DST_COLOR To GL_SRC_COLOR (Multiplies By 2)
			Lighting Enabled
			No Offset Texture-Coordinates
			*/	
	glActiveTextureARB_f(GL_TEXTURE1_ARB);		
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB_f(GL_TEXTURE0_ARB);			
	if (!emboss) {						
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D,texture[filter]);
		glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);
		glEnable(GL_BLEND);
		glEnable(GL_LIGHTING);
		doCube();
	}

	xrot+=xspeed;
	yrot+=yspeed;
	if (xrot>360.0f) xrot-=360.0f;
	if (xrot<0.0f) xrot+=360.0f;
	if (yrot>360.0f) yrot-=360.0f;
	if (yrot<0.0f) yrot+=360.0f;

/* LAST PASS:	Do The Logos! */	
	doLogo();

	return TRUE;										// Keep Going
}
BOOL doMeshNoBumps(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity();									// Reset The View
	glTranslatef(0.0f,0.0f,z);

	glRotatef(xrot,1.0f,0.0f,0.0f);
	glRotatef(yrot,0.0f,1.0f,0.0f);	
	if (useMultitexture) {
		glActiveTextureARB_f(GL_TEXTURE1_ARB);		
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB_f(GL_TEXTURE0_ARB);
	}
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D,texture[filter]);	
	glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);
	glEnable(GL_LIGHTING);
	doCube();
	
	xrot+=xspeed;
	yrot+=yspeed;
	if (xrot>360.0f) xrot-=360.0f;
	if (xrot<0.0f) xrot+=360.0f;
	if (yrot>360.0f) yrot-=360.0f;
	if (yrot<0.0f) yrot+=360.0f;

/* LAST PASS:	Do The Logos! */	
	doLogo();

	return TRUE;										// Keep Going
}

BOOL DrawGLScene(GLvoid)								// Here's Where We Do All The Drawing
{	if (bumps) {
		if (useMultitexture && maxTexelUnits>1)
			return doMesh2TexelUnits();	
		else return doMesh1TexelUnits();	
	}
	else return doMeshNoBumps();
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
	if (!CreateGLWindow("NeHe's GL_ARB_multitexture & Bump Mapping Tutorial",640,480,16,fullscreen))
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
					switch (event.key.keysym.sym) {
						case SDLK_e:
							emboss=!emboss;
							break;
						case SDLK_m:
							useMultitexture=((!useMultitexture) && multitextureSupported);
							break;
						case SDLK_b:
							bumps=!bumps;
							break;
						case SDLK_f:
							filter++;
							filter%=3;
							break;
						default:
							break;
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

		if (keys[SDLK_PAGEUP])
		{
			z-=0.02f;
		}
		if (keys[SDLK_PAGEDOWN])
		{
			z+=0.02f;
		}
		if (keys[SDLK_UP])
		{
			xspeed-=0.01f;
		}
		if (keys[SDLK_DOWN])
		{
			xspeed+=0.01f;
		}
		if (keys[SDLK_RIGHT])
		{
			yspeed+=0.01f;
		}
		if (keys[SDLK_LEFT])
		{
			yspeed-=0.01f;
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	SDL_Quit();
	return 0;										// Exit The Program
}


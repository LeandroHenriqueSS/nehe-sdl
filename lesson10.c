//
// This code was created by Jeff Molofee '99
// (ported to SDL by Sam Lantinga '2000)
//
// If you've found this code useful, please let me know.
//
// Visit me at www.demonews.com/hosted/nehe 
//
#include <math.h>
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


GLuint loop;             // general loop variable
GLuint texture[3];       // storage for 3 textures;

int light = 0;           // lighting on/off
int blend = 0;        // blending on/off

GLfloat xrot;            // x rotation
GLfloat yrot;            // y rotation
GLfloat xspeed;          // x rotation speed
GLfloat yspeed;          // y rotation speed

GLfloat walkbias = 0;
GLfloat walkbiasangle = 0;

GLfloat lookupdown = 0.0;
const float piover180 = 0.0174532925f;

float heading, xpos, zpos;

GLfloat camx = 0, camy = 0, camz = 0; // camera location.
GLfloat therotate;

GLfloat z=0.0f;                       // depth into the screen.

GLfloat LightAmbient[]  = {0.5f, 0.5f, 0.5f, 1.0f}; 
GLfloat LightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f}; 
GLfloat LightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};

GLuint filter = 0;       // texture filtering method to use (nearest, linear, linear + mipmaps)

typedef struct {         // vertex coordinates - 3d and texture
    GLfloat x, y, z;     // 3d coords.
    GLfloat u, v;        // texture coords.
} VERTEX;

typedef struct {         // triangle
    VERTEX vertex[3];    // 3 vertices array
} TRIANGLE;

typedef struct {         // sector of a 3d environment
    int numtriangles;    // number of triangles in the sector
    TRIANGLE* triangle;  // pointer to array of triangles.
} SECTOR;

SECTOR sector1;

// degrees to radians...2 PI radians = 360 degrees
float rad(float angle)
{
    return angle * piover180;
}

// helper for SetupWorld.  reads a file into a string until a nonblank, non-comment line
// is found ("/" at the start indicating a comment); assumes lines < 255 characters long.
void readstr(FILE *f, char *string)
{
    do {
	fgets(string, 255, f); // read the line
    } while ((string[0] == '/') || (string[0] == '\n'));
    return;
}

// loads the world from a text file.
void SetupWorld() 
{
    float x, y, z, u, v;
    int vert;
    int numtriangles;
    FILE *filein;        // file to load the world from
    char oneline[255];

    filein = fopen("Data10/world.txt", "rt");

    readstr(filein, oneline);
    sscanf(oneline, "NUMPOLLIES %d\n", &numtriangles);

    sector1.numtriangles = numtriangles;
    sector1.triangle = (TRIANGLE *) malloc(sizeof(TRIANGLE)*numtriangles);
    
    for (loop = 0; loop < numtriangles; loop++) {
	for (vert = 0; vert < 3; vert++) {
	    readstr(filein,oneline);
	    sscanf(oneline, "%f %f %f %f %f", &x, &y, &z, &u, &v);
	    sector1.triangle[loop].vertex[vert].x = x;
	    sector1.triangle[loop].vertex[vert].y = y;
	    sector1.triangle[loop].vertex[vert].z = z;
	    sector1.triangle[loop].vertex[vert].u = u;
	    sector1.triangle[loop].vertex[vert].v = v;
	}
    }

    fclose(filein);
    return;
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
 
// Load Bitmaps And Convert To Textures
void LoadGLTextures(void)
{	
    // Load Texture
    SDL_Surface *image1;
    
    image1 = LoadBMP("Data10/mud.bmp");
    if (!image1) {
        SDL_Quit();
        exit(1);
    }

    // Create Textures	
    glGenTextures(3, &texture[0]);

    // nearest filtered texture
    glBindTexture(GL_TEXTURE_2D, texture[0]);   // 2d texture (x and y size)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); // scale cheaply when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); // scale cheaply when image smalled than texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->w, image1->h, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->pixels);

    // linear filtered texture
    glBindTexture(GL_TEXTURE_2D, texture[1]);   // 2d texture (x and y size)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->w, image1->h, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->pixels);

    // mipmapped texture
    glBindTexture(GL_TEXTURE_2D, texture[2]);   // 2d texture (x and y size)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST); // scale mipmap when image smalled than texture
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image1->w, image1->h, GL_RGB, GL_UNSIGNED_BYTE, image1->pixels);
};

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
void InitGL(int Width, int Height)	        // We call this right after our OpenGL window is created.
{
    glViewport(0, 0, Width, Height);
    LoadGLTextures();                           // load the textures.
    glEnable(GL_TEXTURE_2D);                    // Enable texture mapping.

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);          // Set the blending function for translucency (note off at init time)
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// This Will Clear The Background Color To Black
    glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
    glDepthFunc(GL_LESS);                       // type of depth test to do.
    glEnable(GL_DEPTH_TEST);                    // enables depth testing.
    glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();				// Reset The Projection Matrix
    
    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window
    
    glMatrixMode(GL_MODELVIEW);

    // set up lights.
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
    glEnable(GL_LIGHT1);
}

/* The main drawing function. */
void DrawGLScene()
{
    GLfloat x_m, y_m, z_m, u_m, v_m;
    GLfloat xtrans, ztrans, ytrans;
    GLfloat sceneroty;
    int numtriangles;

    // calculate translations and rotations.
    xtrans = -xpos;
    ztrans = -zpos;
    ytrans = -walkbias-0.25f;
    sceneroty = 360.0f - yrot;
    	
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
    glLoadIdentity();

    glRotatef(lookupdown, 1.0f, 0, 0);
    glRotatef(sceneroty, 0, 1.0f, 0);

    glTranslatef(xtrans, ytrans, ztrans);    

    glBindTexture(GL_TEXTURE_2D, texture[filter]);    // pick the texture.

    numtriangles = sector1.numtriangles;

    for (loop=0; loop<numtriangles; loop++) {        // loop through all the triangles
	glBegin(GL_TRIANGLES);		
	glNormal3f( 0.0f, 0.0f, 1.0f);
	
	x_m = sector1.triangle[loop].vertex[0].x;
	y_m = sector1.triangle[loop].vertex[0].y;
	z_m = sector1.triangle[loop].vertex[0].z;
	u_m = sector1.triangle[loop].vertex[0].u;
	v_m = sector1.triangle[loop].vertex[0].v;
	glTexCoord2f(u_m,v_m); 
	glVertex3f(x_m,y_m,z_m);
	
	x_m = sector1.triangle[loop].vertex[1].x;
	y_m = sector1.triangle[loop].vertex[1].y;
	z_m = sector1.triangle[loop].vertex[1].z;
	u_m = sector1.triangle[loop].vertex[1].u;
	v_m = sector1.triangle[loop].vertex[1].v;
	glTexCoord2f(u_m,v_m); 
	glVertex3f(x_m,y_m,z_m);
	
	x_m = sector1.triangle[loop].vertex[2].x;
	y_m = sector1.triangle[loop].vertex[2].y;
	z_m = sector1.triangle[loop].vertex[2].z;
	u_m = sector1.triangle[loop].vertex[2].u;
	v_m = sector1.triangle[loop].vertex[2].v;
	glTexCoord2f(u_m,v_m); 
	glVertex3f(x_m,y_m,z_m);	
	
	glEnd();	
    }

    // swap buffers to display, since we're double buffered.
    SDL_GL_SwapBuffers();
}

int main(int argc, char **argv) 
{  
    int done;
    Uint8 *keys;

    /* Load our world from disk */
    SetupWorld();

    /* Initialize SDL for video output */
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    /* Create a 640x480 OpenGL screen */
    if ( SDL_SetVideoMode(640, 480, 0, SDL_OPENGL) == NULL ) {
        fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
        SDL_Quit();
        exit(2);
    }

    /* Set the title bar in environments that support it */
    SDL_WM_SetCaption("Jeff Molofee's GL Code Tutorial ... NeHe '99", NULL);

    /* Loop, drawing and checking events */
    InitGL(640, 480);
    done = 0;
    while ( ! done ) {
        DrawGLScene();

        /* This could go in a separate function */
        { SDL_Event event;
          while ( SDL_PollEvent(&event) ) {
              if ( event.type == SDL_QUIT ) {
                  done = 1;
              }
              if ( event.type == SDL_KEYDOWN ) {
                  switch(event.key.keysym.sym) {
                      case SDLK_ESCAPE:
                          done = 1;
                          break;
                      case SDLK_b:
	                  printf("Blend was: %d\n", blend);
	                  blend = blend ? 0 : 1;              // switch the current value of blend, between 0 and 1.
	                  printf("Blend is now: %d\n", blend);
	                  if (!blend) {
	                      glEnable(GL_BLEND);		    // Turn Blending On
	                      glDisable(GL_DEPTH_TEST);         // Turn Depth Testing Off
	                  } else {
	                      glDisable(GL_BLEND);              // Turn Blending Off
	                      glEnable(GL_DEPTH_TEST);          // Turn Depth Testing On
	                  }
	                  break;
                      case SDLK_l:
	                  printf("Light was: %d\n", light);
	                  light = light ? 0 : 1;              // switch the current value of light, between 0 and 1.
	                  printf("Light is now: %d\n", light);
	                  if (!light) {
	                      glDisable(GL_LIGHTING);
	                  } else {
	                      glEnable(GL_LIGHTING);
	                  }
	                  break;
                      case SDLK_f:
	                  printf("Filter was: %d\n", filter);
	                  filter += 1;
	                  if (filter>2)
                              filter = 0;
	                  printf("Filter is now: %d\n", filter);
	                  break;
                  }
              }
          }
        }

        /* Check current key state for special commands */
        keys = SDL_GetKeyState(NULL);
        if ( keys[SDLK_PAGEUP] == SDL_PRESSED ) {
	    z -= 0.2f;
	    lookupdown -= 0.2f;
        }
        if ( keys[SDLK_PAGEDOWN] == SDL_PRESSED ) {
	    z += 0.2f;
	    lookupdown += 1.0f;
        }
        if ( keys[SDLK_UP] == SDL_PRESSED ) {
	    xpos -= (float)sin(yrot*piover180) * 0.05f;
	    zpos -= (float)cos(yrot*piover180) * 0.05f;	
	    if (walkbiasangle >= 359.0f)
	        walkbiasangle = 0.0f;	
	    else 
	        walkbiasangle+= 10;
	    walkbias = (float)sin(walkbiasangle * piover180)/20.0f;
        }
        if ( keys[SDLK_DOWN] == SDL_PRESSED ) {
	    xpos += (float)sin(yrot*piover180) * 0.05f;
	    zpos += (float)cos(yrot*piover180) * 0.05f;	
	    if (walkbiasangle <= 1.0f)
	        walkbiasangle = 359.0f;	
	    else 
	        walkbiasangle-= 10;
	    walkbias = (float)sin(walkbiasangle * piover180)/20.0f;
        }
        if ( keys[SDLK_LEFT] == SDL_PRESSED ) {
	    yrot += 1.5f;
        }
        if ( keys[SDLK_RIGHT] == SDL_PRESSED ) {
	    yrot -= 1.5f;
        }
    }
    SDL_Quit();
    return 1;
}

//
// This code was created by Jeff Molofee '99
// (ported to SDL by Sam Lantinga '2000)
//
// If you've found this code useful, please let me know.
//
// Visit me at www.demonews.com/hosted/nehe 
//
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

/* number of stars to have */
#define STAR_NUM 50

/* twinkle on/off (1 = on, 0 = off) */
int twinkle = 0;

typedef struct {         // Star structure
    int r, g, b;         // stars' color
    GLfloat dist;        // stars' distance from center
    GLfloat angle;       // stars' current angle
} stars;                 // name is stars

stars star[STAR_NUM];    // make 'star' array of STAR_NUM size using info from the structure 'stars'

GLfloat zoom = -15.0f;   // viewing distance from stars.
GLfloat tilt = 90.0f;    // tilt the view
GLfloat spin;            // spin twinkling stars

GLuint loop;             // general loop variable
GLuint texture[1];       // storage for one texture;

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
GLvoid LoadGLTextures(GLvoid)
{	
    // Load Texture
    SDL_Surface *image1;
    
    image1 = LoadBMP("Data09/Star.bmp");
    if (!image1) {
        SDL_Quit();
        exit(1);
    }

    // Create Textures	
    glGenTextures(3, &texture[0]);

    // linear filtered texture
    glBindTexture(GL_TEXTURE_2D, texture[0]);   // 2d texture (x and y size)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->w, image1->h, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->pixels);
};

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
GLvoid InitGL(GLsizei Width, GLsizei Height)	// We call this right after our OpenGL window is created.
{
    glViewport(0, 0, Width, Height);
    LoadGLTextures();                           // load the textures.
    glEnable(GL_TEXTURE_2D);                    // Enable texture mapping.

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// This Will Clear The Background Color To Black
    glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer

    glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();				// Reset The Projection Matrix
    
    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window
    
    glMatrixMode(GL_MODELVIEW);

    /* setup blending */
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);	        // Set The Blending Function For Translucency
    glEnable(GL_BLEND);                         // Enable Blending

    /* set up the stars */
    for (loop=0; loop<STAR_NUM; loop++) {
	star[loop].angle = 0.0f;                // initially no rotation.
	
	star[loop].dist = loop * 1.0f / STAR_NUM * 5.0f; // calculate distance form the center
	star[loop].r = rand() % 256;            // random red intensity;
	star[loop].g = rand() % 256;            // random green intensity;
	star[loop].b = rand() % 256;            // random blue intensity;
    }    
}

/* The main drawing function. */
void DrawGLScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
    
    glBindTexture(GL_TEXTURE_2D, texture[0]);    // pick the texture.

    for (loop=0; loop<STAR_NUM; loop++) {        // loop through all the stars.
	glLoadIdentity();                        // reset the view before we draw each star.
	glTranslatef(0.0f, 0.0f, zoom);          // zoom into the screen.
	glRotatef(tilt, 1.0f, 0.0f, 0.0f);       // tilt the view.
	
	glRotatef(star[loop].angle, 0.0f, 1.0f, 0.0f); // rotate to the current star's angle.
	glTranslatef(star[loop].dist, 0.0f, 0.0f); // move forward on the X plane (the star's x plane).

	glRotatef(-star[loop].angle, 0.0f, 1.0f, 0.0f); // cancel the current star's angle.
	glRotatef(-tilt, 1.0f, 0.0f, 0.0f);      // cancel the screen tilt.

	if (twinkle) {                           // twinkling stars enabled ... draw an additional star.
	    // assign a color using bytes
	    glColor4ub(star[STAR_NUM - loop].r, star[STAR_NUM - loop].g, star[STAR_NUM - loop].b, 255);

	    glBegin(GL_QUADS);                   // begin drawing the textured quad.
	    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
	    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
	    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, 0.0f);
	    glEnd();                             // done drawing the textured quad.
	}

	// main star
	glRotatef(spin, 0.0f, 0.0f, 1.0f);       // rotate the star on the z axis.

        // Assign A Color Using Bytes
	glColor4ub(star[loop].r,star[loop].g,star[loop].b,255);
	glBegin(GL_QUADS);			// Begin Drawing The Textured Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,-1.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,-1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
	glEnd();				// Done Drawing The Textured Quad
	
	spin +=0.01f;                           // used to spin the stars.
	star[loop].angle += loop * 1.0f / STAR_NUM * 1.0f;    // change star angle.
	star[loop].dist  -= 0.01f;              // bring back to center.

	if (star[loop].dist<0.0f) {             // star hit the center
	    star[loop].dist += 5.0f;            // move 5 units from the center.
	    star[loop].r = rand() % 256;        // new red color.
	    star[loop].g = rand() % 256;        // new green color.
	    star[loop].b = rand() % 256;        // new blue color.
	}
    }

    // swap buffers to display, since we're double buffered.
    SDL_GL_SwapBuffers();
}

int main(int argc, char **argv) 
{  
    int done;
    Uint8 *keys;

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
                       case SDLK_t:
	                   printf("Twinkle was: %d\n", twinkle);
	                   twinkle = twinkle ? 0 : 1;              // switch the current value of twinkle, between 0 and 1.
	                   printf("Twinkle is now: %d\n", twinkle);
                   }
               }
           }
         }

         /* Check current key state for special commands */
         keys = SDL_GetKeyState(NULL);
         if ( keys[SDLK_PAGEUP] == SDL_PRESSED ) {
             zoom -= 0.02f;	// zoom out
         }
         if ( keys[SDLK_PAGEDOWN] == SDL_PRESSED ) {
             zoom += 0.02f;	// zoom in
         }
         if ( keys[SDLK_UP] == SDL_PRESSED ) {
             tilt -= 0.5f;	// tilt up
         }
         if ( keys[SDLK_DOWN] == SDL_PRESSED ) {
             tilt += 0.5f;	// tilt down
         }
    }
    SDL_Quit();
    return 1;
}

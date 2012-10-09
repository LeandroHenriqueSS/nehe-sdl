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

GLuint    texture[1];        // Storage For 1 Texture
GLuint    cube;            // Storage For The Display List
GLuint    top;            // Storage For The Second Display List
GLuint    xloop;            // Loop For X Axis
GLuint    yloop;            // Loop For Y Axis

GLfloat    xrot;            // Rotates Cube On The X Axis
GLfloat    yrot;            // Rotates Cube On The Y Axis

static GLfloat boxcol[5][3]=
{
    {1.0f,0.0f,0.0f},{1.0f,0.5f,0.0f},{1.0f,1.0f,0.0f},{0.0f,1.0f,0.0f},{0.0f,1.0f,1.0f}
};

static GLfloat topcol[5][3]=
{
    {.5f,0.0f,0.0f},{0.5f,0.25f,0.0f},{0.5f,0.5f,0.0f},{0.0f,0.5f,0.0f},{0.0f,0.5f,0.5f}
};

// Build Cube Display List
GLvoid BuildList()
{
    cube=glGenLists(2);
    glNewList(cube,GL_COMPILE);
        glBegin(GL_QUADS);
            // Bottom Face
            glNormal3f( 0.0f,-1.0f, 0.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            // Front Face
            glNormal3f( 0.0f, 0.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
            // Back Face
            glNormal3f( 0.0f, 0.0f,-1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            // Right face
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
    glEndList();
    top=cube+1;
    glNewList(top,GL_COMPILE);
        glBegin(GL_QUADS);
            // Top Face
            glNormal3f( 0.0f, 1.0f, 0.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
        glEnd();
    glEndList();
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
    SDL_Surface *texture1;
    
    texture1 = LoadBMP("Data12/cube.bmp");
    if (!texture1) {
        SDL_Quit();
        exit(1);
    }

    // Create Texture    
    glGenTextures(1, &texture[0]);
    glBindTexture(GL_TEXTURE_2D, texture[0]);   // 2d texture (x and y size)

    // Create MipMapped Texture
    glGenTextures(1, &texture[0]);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texture1->w, texture1->h, GL_RGB, GL_UNSIGNED_BYTE, texture1->pixels);
};

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
void InitGL(int Width, int Height)            // We call this right after our OpenGL window is created.
{
    glViewport(0, 0, Width, Height);
    LoadGLTextures();                            // Load The Texture
    BuildList();                                // Build The Display List
    glEnable(GL_TEXTURE_2D);                    // Enable Texture Mapping

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);        // This Will Clear The Background Color To Black
    glClearDepth(1.0);                            // Enables Clearing Of The Depth Buffer
    glDepthFunc(GL_LESS);                        // The Type Of Depth Test To Do
    glEnable(GL_DEPTH_TEST);                    // Enables Depth Testing
    glShadeModel(GL_SMOOTH);                    // Enables Smooth Color Shading

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();                            // Reset The Projection Matrix

    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);    // Calculate The Aspect Ratio Of The Window

    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
}

/* The main drawing function. */
void DrawGLScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        // Clear The Screen And The Depth Buffer

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    for (yloop=1;yloop<6;yloop++)
    {
        for (xloop=0;xloop<yloop;xloop++)
        {
            glLoadIdentity();                                        // Reset The View
            glTranslatef(1.4f+((float)(xloop)*2.8f)-((float)(yloop)*1.4f),((6.0f-(float)(yloop))*2.4f)-7.0f,-20.0f);
            glRotatef(45.0f-(2.0f*yloop)+xrot,1.0f,0.0f,0.0f);
            glRotatef(45.0f+yrot,0.0f,1.0f,0.0f);
            glColor3fv(boxcol[yloop-1]);
            glCallList(cube);
            glColor3fv(topcol[yloop-1]);
            glCallList(top);
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
                  if ( event.key.keysym.sym == SDLK_ESCAPE ) {
                      done = 1;
                  }
              }
          }
        }

        /* Check current key state for special commands */
        keys = SDL_GetKeyState(NULL);
        if ( keys[SDLK_UP] == SDL_PRESSED ) {
            xrot -= 0.2f;
        }
        if ( keys[SDLK_DOWN] == SDL_PRESSED ) {
            xrot += 0.2f;
        }
        if ( keys[SDLK_LEFT] == SDL_PRESSED ) {
	    yrot -= 0.2f;
        }
        if ( keys[SDLK_RIGHT] == SDL_PRESSED ) {
	    yrot += 0.2f;
        }
    }
    SDL_Quit();
    return 1;
}

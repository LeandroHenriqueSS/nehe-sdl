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

float points[ 45 ][ 45 ][3];    // The array for the points on the grid of our "wave"
int wiggle_count = 0;

GLfloat    xrot = 0;            // X Rotation
GLfloat    yrot = 0;            // Y Rotation
GLfloat    zrot = 0;            // Z Rotation

GLuint    texture[1];        // Storage for 1 texture

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
    
    texture1 = LoadBMP("Data11/tim.bmp");
    if (!texture1) {
        SDL_Quit();
        exit(1);
    }

    // Create Texture	
    glGenTextures(1, &texture[0]);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, texture1->w, texture1->h, 0, GL_RGB, GL_UNSIGNED_BYTE, texture1->pixels);
};

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
void InitGL(int Width, int Height)	        // We call this right after our OpenGL window is created.
{
    float float_x, float_y;

    glViewport(0, 0, Width, Height);
    LoadGLTextures();                            // Load The Texture(s)
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

    for( float_x = 0.0f; float_x < 9.0f; float_x +=  0.2f )
    {
        for( float_y = 0.0f; float_y < 9.0f; float_y += 0.2f)
        {
            points[ (int)(float_x*5) ][ (int)(float_y*5) ][0] = float_x - 4.4f;
            points[ (int)(float_x*5) ][ (int)(float_y*5) ][1] = float_y - 4.4f;
            points[ (int)(float_x*5) ][ (int)(float_y*5) ][2] = (float)(sin( ( (float_x*5*8)/360 ) * 3.14159 * 2 ));
        }
    }
}

/* The main drawing function. */
void DrawGLScene()
{
      int x, y;
      float float_x, float_y, float_xb, float_yb;

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
      glLoadIdentity();                            
      glTranslatef(0.0f,0.0f,-12.0f);
      
      glRotatef(xrot,1.0f,0.0f,0.0f);
      glRotatef(yrot,0.0f,1.0f,0.0f);  
      glRotatef(zrot,0.0f,0.0f,1.0f);

      glBindTexture(GL_TEXTURE_2D, texture[0]);

      glPolygonMode( GL_BACK, GL_FILL );
      glPolygonMode( GL_FRONT, GL_LINE );


      glBegin( GL_QUADS );

      for( x = 0; x < 44; x++ )
      {
         for( y = 0; y < 44; y++ )
         {

             float_x = (float)(x)/44;
             float_y = (float)(y)/44;
             float_xb = (float)(x+1)/44;
             float_yb = (float)(y+1)/44;
             

             glTexCoord2f( float_x, float_y);
             glVertex3f( points[x][y][0], points[x][y][1], points[x][y][2] );

             glTexCoord2f( float_x, float_yb );
             glVertex3f( points[x][y+1][0], points[x][y+1][1], points[x][y+1][2] );
             
             glTexCoord2f( float_xb, float_yb );
             glVertex3f( points[x+1][y+1][0], points[x+1][y+1][1], points[x+1][y+1][2] );

             glTexCoord2f( float_xb, float_y );
             glVertex3f( points[x+1][y][0], points[x+1][y][1], points[x+1][y][2] );
         }
    }

    glEnd();

    if( wiggle_count == 2 )
    {
        for( y = 0; y < 45; y++)
       {
            points[44][y][2] = points[0][y][2];
       }

       for( x = 0; x < 44; x++ )
       {
           for( y = 0; y < 45; y++)
           {
                   points[x][y][2] = points[x+1][y][2];
            }
       }

           wiggle_count = 0;
     }

    wiggle_count++;

    xrot+=0.3f;
    yrot+=0.2f;
    zrot+=0.4f;

    // swap buffers to display, since we're double buffered.
    SDL_GL_SwapBuffers();
}

int main(int argc, char **argv) 
{  
  int done;

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
  }
  SDL_Quit();
  return 1;
}

/*		This code has been created by Banu Cosmin aka Choko - 20 may 2000
 *		and uses NeHe tutorials as a starting point (window initialization,
 *		texture loading, GL initialization and code for keypresses) - very good
 *		tutorials, Jeff. If anyone is interested about the presented algorithm
 *		please e-mail me at boct@romwest.ro
 *
 *		Attention!!! This code is not for beginners.
 *
 *		The code has been rewritten from the ground-up for the tutorial by Brett Porter.
 *		Visit my website at http://www.geocities.com/brettporter/programming
 *		if you are interested in more OpenGL 3D tutorials.
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

/************** OBJECT RELATED ROUTINES AND STRUCTURES **************/
// (Yes these could all be in separate files :)

// Definition Of "INFINITY" For Calculating The Extension Vector For The Shadow Volume
#define INFINITY	100

// Structure Describing A Vertex In An Object
typedef struct
{
	GLfloat x, y, z;
} Point3f;

// Structure Describing A Plane, In The Format: ax + by + cz + d = 0
typedef struct
{
	GLfloat a, b, c, d;
} Plane;

// Structure Describing An Object's Face
typedef struct
{
	int vertexIndices[3];				// Index Of Each Vertex Within An Object That Makes Up The Triangle Of This Face
	Point3f normals[3];					// Normals To Each Vertex
	Plane planeEquation;				// Equation Of A Plane That Contains This Triangle
	int neighbourIndices[3];			// Index Of Each Face That Neighbours This One Within The Object
	BOOL visible;						// Is The Face Visible By The Light?
} Face;

typedef struct
{
	int nVertices;
	Point3f *pVertices;					// Will Be Dynamically Allocated

	int nFaces;
	Face *pFaces;						// Will Be Dynamically Allocated
} ShadowedObject;

static void doShadowPass( ShadowedObject* object, GLfloat *lightPosition );
static BOOL readObject( const char *filename, ShadowedObject* object );
static void killObject( ShadowedObject* object );
static void setConnectivity( ShadowedObject* object );
static void calculatePlane( const ShadowedObject* object, Face* face );
static void drawObject( const ShadowedObject* object );
static void castShadow( ShadowedObject* object, GLfloat *lightPosition );

/*
	Load An Object From A File.
		filename	-	The File To Load The Object From
		object		-	Reference To Object To Store The Information In
	Returns:
		Whether The Function Was Successful.
*/
BOOL readObject( const char *filename, ShadowedObject* object )
{
	FILE *pInputFile;
	int i;

	pInputFile = fopen( filename, "r" );
	if ( pInputFile == NULL )
	{
		fprintf(stderr, "Unable to open the object file: %s\n", filename);
		return FALSE;
	}

	// Read Vertices
	fscanf( pInputFile, "%d", &object->nVertices );
	object->pVertices = (Point3f *)malloc(object->nVertices*sizeof(Point3f));
	for ( i = 0; i < object->nVertices; i++ )
	{
		fscanf( pInputFile, "%f", &object->pVertices[i].x );
		fscanf( pInputFile, "%f", &object->pVertices[i].y );
		fscanf( pInputFile, "%f", &object->pVertices[i].z );
	}

	// Read Faces
	fscanf( pInputFile, "%d", &object->nFaces );
	object->pFaces = (Face *)malloc(object->nFaces*sizeof(Face));
	for ( i = 0; i < object->nFaces; i++ )
	{
		int j;
		Face *pFace = &object->pFaces[i];

		for ( j = 0; j < 3; j++ )
			pFace->neighbourIndices[j] = -1;	// No Neigbours Set Up Yet

		for ( j = 0; j < 3; j++ )
		{
			fscanf( pInputFile, "%d", &pFace->vertexIndices[j] );
			pFace->vertexIndices[j]--;			// Files Specify Them With A 1 Array Base, But We Use A 0 Array Base
		}

		for ( j = 0; j < 3; j++ )
		{
			fscanf( pInputFile, "%f", &pFace->normals[j].x );
			fscanf( pInputFile, "%f", &pFace->normals[j].y );
			fscanf( pInputFile, "%f", &pFace->normals[j].z );
		}
	}
	return TRUE;
}

/*
	Free Any Dynamic Memory Associated With An Object.
		object	-	The Object To Free The Memory Within. The Structure Itself Is Not Destroyed.
*/
void killObject( ShadowedObject* object )
{
	free(object->pFaces);
	object->pFaces = NULL;
	object->nFaces = 0;

	free(object->pVertices);
	object->pVertices = NULL;
	object->nVertices = 0;
}

/*
	Sets Up The Connectivity Coeffecients Of An Object.
	Determines, For Each Triangle, What Other Triangles Are Connected To Each Edge Of The Triangle.
	Please Refer To: http://www.gamasutra.com/features/19991115/bestimt_freitag_02.htm
*/
void setConnectivity( ShadowedObject* object )
{
	int faceA, faceB;
	int edgeA, edgeB;

	// Go Through Each Triangle
	for ( faceA = 0; faceA < object->nFaces; faceA++ )
	{
		// Get Point To This Face
		Face *pFaceA = &object->pFaces[faceA];

		// Go Through Each Edge Of This Triangle
		for ( edgeA = 0; edgeA < 3; edgeA++ )
		{
			// If No Neighbour Has Been Found, Then It Is 0, So Look For One
			if ( pFaceA->neighbourIndices[edgeA] == -1 )
			{
				// Go Through All Other Triangles That Haven't Already Been Evaluated For Connectivity
				// (Triangles Before This One In The Array Have Already Been Checked Against This One)
				for ( faceB = faceA+1; faceB < object->nFaces; faceB++ )
				{
					// Get Point To This Face
					Face *pFaceB = &object->pFaces[faceB];
					BOOL edgeFound = FALSE;

					// Go Through All Face B's Edges And See If They Are The Same As The Edge Of
					// Face A We Are Currently Looking At
					for ( edgeB = 0; edgeB < 3; edgeB++ )
					{
						int vertA1 = pFaceA->vertexIndices[edgeA];
						int vertA2 = pFaceA->vertexIndices[( edgeA+1 )%3];

						int vertB1 = pFaceB->vertexIndices[edgeB];
						int vertB2 = pFaceB->vertexIndices[( edgeB+1 )%3];

						// Check If They Are Neighbours - IE, The Edges Are The Same
						if (( vertA1 == vertB1 && vertA2 == vertB2 ) ||
							( vertA1 == vertB2 && vertA2 == vertB1 ))
						{
							pFaceA->neighbourIndices[edgeA] = faceB;
							pFaceB->neighbourIndices[edgeB] = faceA;
							edgeFound = TRUE;
							break;
						}
					}
					if ( edgeFound )
						break;
				}
			}
		}
	}
}

/*
	Calculate The Equation Of The Plane Of A Face In An Object.
		object	-	The Object Containing The Face
		face	-	The Face To Calculate The Plane For
*/
void calculatePlane( const ShadowedObject* object, Face* face )
{
	// Get Shortened Names For The Vertices Of The Face
	const Point3f* v1 = &object->pVertices[face->vertexIndices[0]];
	const Point3f* v2 = &object->pVertices[face->vertexIndices[1]];
	const Point3f* v3 = &object->pVertices[face->vertexIndices[2]];

	face->planeEquation.a = v1->y*(v2->z-v3->z) + v2->y*(v3->z-v1->z) + v3->y*(v1->z-v2->z);
	face->planeEquation.b = v1->z*(v2->x-v3->x) + v2->z*(v3->x-v1->x) + v3->z*(v1->x-v2->x);
	face->planeEquation.c = v1->x*(v2->y-v3->y) + v2->x*(v3->y-v1->y) + v3->x*(v1->y-v2->y);
	face->planeEquation.d = -( v1->x*( v2->y*v3->z - v3->y*v2->z ) +
							v2->x*(v3->y*v1->z - v1->y*v3->z) +
							v3->x*(v1->y*v2->z - v2->y*v1->z) );
}

/*	Draw An Object - Simply Draw Each Triangular Face. */
void drawObject( const ShadowedObject* object )
{
	int i, j;
	glBegin( GL_TRIANGLES );
	for ( i = 0; i < object->nFaces; i++ )
	{
		const Face* face = &object->pFaces[i];

		for ( j = 0; j < 3; j++ )
		{
			const Point3f* vertex = &object->pVertices[face->vertexIndices[j]];

			glNormal3f( face->normals[j].x, face->normals[j].y, face->normals[j].z );
			glVertex3f( vertex->x, vertex->y, vertex->z );
		}
	}
	glEnd();
}

/*
	Cast a shadow for an object, with the given light position.
	This uses the stencil buffer to achieve this.
	Note that visible in the following definiton refers to the position of the light source.
	What happens is that you go through every face, and if it is visible, then you check all its edges.
	If at the edge, there is no neighbouring face, or the neighbouring face is not visible, the edge casts a shadow.
	By drawing a quadrilateral (as two triangles) comprising of the points of the edge, and the edge projected backwards through
	the scene you get the shadow cast by it. The brute force approach used here just draws to "infinity", and the shadow
	polygon is clipped against all the polygons it encounters. This causes piercing, which will stress the video hardware.
	For a high-performance modification to this algorithm, you should clip the polygon to the objects behind it.
	For more information on this topic, please refer to: http://www.gamasutra.com/features/19991115/bestimt_freitag_02.htm

		object			-	The Object To Cast A Shadow For
		lightPosition	-	Light position vector (in object's space coordinates)
*/
void castShadow( ShadowedObject* object, GLfloat *lightPosition )
{
	int i;
	// Determine Which Faces Are Visible By The Light.
	for ( i = 0; i < object->nFaces; i++ )
	{
		const Plane* plane = &object->pFaces[i].planeEquation;

		GLfloat side = plane->a*lightPosition[0]+
			plane->b*lightPosition[1]+
			plane->c*lightPosition[2]+
			plane->d;

		if ( side > 0 )
			object->pFaces[i].visible = TRUE;
		else
			object->pFaces[i].visible = FALSE;
	}

	// Preserve Attributes We Modify
	glPushAttrib( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT );

	glDisable( GL_LIGHTING );								// Turn Off Lighting
	glDepthMask( GL_FALSE );								// Turn Off Writing To The Depth-Buffer
	glDepthFunc( GL_LEQUAL );

	glEnable( GL_STENCIL_TEST );							// Turn On Stencil Buffer Testing
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );	// Don't Draw Into The Colour Buffer
	glStencilFunc( GL_ALWAYS, 1, 0xFFFFFFFFL );

	// First Pass. Increase Stencil Value In The Shadow
	glFrontFace( GL_CCW );
	glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );
	doShadowPass( object, lightPosition ); 

	// Second Pass. Decrease Stencil Value In The Shadow
	glFrontFace( GL_CW );
	glStencilOp( GL_KEEP, GL_KEEP, GL_DECR );
	doShadowPass( object, lightPosition ); 

	glFrontFace( GL_CCW );
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );		// Enable rendering to colour buffer for all components

	// Draw A Shadowing Rectangle Covering The Entire Screen
	glColor4f( 0.0f, 0.0f, 0.0f, 0.4f );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glStencilFunc( GL_NOTEQUAL, 0, 0xFFFFFFFFL );
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	glPushMatrix();
	glLoadIdentity();
	glBegin( GL_TRIANGLE_STRIP );
		glVertex3f(-0.1f, 0.1f,-0.10f);
		glVertex3f(-0.1f,-0.1f,-0.10f);
		glVertex3f( 0.1f, 0.1f,-0.10f);
		glVertex3f( 0.1f,-0.1f,-0.10f);
	glEnd();
	glPopMatrix();
	glPopAttrib();
}

/*
	Operation Is Described Above, Does One Pass Of Rendering A Shadow.
 */
static void doShadowPass( ShadowedObject* object, GLfloat *lightPosition )
{
	int i, j;

	for ( i = 0; i < object->nFaces; i++ )
	{
		const Face* face = &object->pFaces[i];

		if ( face->visible )
		{
			// Go Through Each Edge
			for ( j = 0; j < 3; j++ )
			{
				int neighbourIndex = face->neighbourIndices[j];
				// If There Is No Neighbour, Or Its Neighbouring Face Is Not Visible, Then This Edge Casts A Shadow
				if ( neighbourIndex == -1 || object->pFaces[neighbourIndex].visible == FALSE )
				{
					// Get The Points On The Edge
					const Point3f* v1 = &object->pVertices[face->vertexIndices[j]];
					const Point3f* v2 = &object->pVertices[face->vertexIndices[( j+1 )%3]];

					// Calculate The Two Vertices In Distance
					Point3f v3, v4;

					v3.x = ( v1->x-lightPosition[0] )*INFINITY;
					v3.y = ( v1->y-lightPosition[1] )*INFINITY;
					v3.z = ( v1->z-lightPosition[2] )*INFINITY;

					v4.x = ( v2->x-lightPosition[0] )*INFINITY;
					v4.y = ( v2->y-lightPosition[1] )*INFINITY;
					v4.z = ( v2->z-lightPosition[2] )*INFINITY;

					// Draw The Quadrilateral (As A Triangle Strip)
					glBegin( GL_TRIANGLE_STRIP );
						glVertex3f( v1->x, v1->y, v1->z );
						glVertex3f( v1->x+v3.x, v1->y+v3.y, v1->z+v3.z );
						glVertex3f( v2->x, v2->y, v2->z );
						glVertex3f( v2->x+v4.x, v2->y+v4.y, v2->z+v4.z );
					glEnd();
				}
			}
		}
	}
}

/************** NOW WE ENCOUNTER THE MAIN APPLICATION **************/

typedef float GLvector4f[4];							// Typedef's For VMatMult Procedure
typedef float GLmatrix16f[16];							// Typedef's For VMatMult Procedure

ShadowedObject	obj;									// Object
GLfloat			xspeed=0, yspeed=0;						// X & Y Rotation Speed
GLfloat			xrot = 0, yrot = 0;						// X & Y Rotation

float LightPos[] = { 0.0f, 5.0f,-4.0f, 1.0f};			// Light Position
float LightAmb[] = { 0.2f, 0.2f, 0.2f, 1.0f};			// Ambient Light Values
float LightDif[] = { 0.6f, 0.6f, 0.6f, 1.0f};			// Diffuse Light Values
float LightSpc[] = {-0.2f, -0.2f, -0.2f, 1.0f};			// Specular Light Values

float MatAmb[] = {0.4f, 0.4f, 0.4f, 1.0f};				// Material - Ambient Values
float MatDif[] = {0.2f, 0.6f, 0.9f, 1.0f};				// Material - Diffuse Values
float MatSpc[] = {0.0f, 0.0f, 0.0f, 1.0f};				// Material - Specular Values
float MatShn[] = {0.0f};								// Material - Shininess

float ObjPos[] = {-2.0f,-2.0f,-5.0f};					// Object Position

GLUquadricObj	*q;										// Quadratic For Drawing A Sphere
float SpherePos[] = {-4.0f,-5.0f,-6.0f};

Uint8* keys;						// Array Used For The Keyboard Routine

static BOOL drawGLScene();
static void VMatMult(GLmatrix16f M, GLvector4f v);
static void DrawGLRoom();
static void KillGLObjects();

void VMatMult(GLmatrix16f M, GLvector4f v)
{
	GLfloat res[4];										// Hold Calculated Results
	res[0]=M[ 0]*v[0]+M[ 4]*v[1]+M[ 8]*v[2]+M[12]*v[3];
	res[1]=M[ 1]*v[0]+M[ 5]*v[1]+M[ 9]*v[2]+M[13]*v[3];
	res[2]=M[ 2]*v[0]+M[ 6]*v[1]+M[10]*v[2]+M[14]*v[3];
	res[3]=M[ 3]*v[0]+M[ 7]*v[1]+M[11]*v[2]+M[15]*v[3];
	v[0]=res[0];										// Results Are Stored Back In v[]
	v[1]=res[1];
	v[2]=res[2];
	v[3]=res[3];										// Homogenous Coordinate
}

void DrawGLRoom()										// Draw The Room (Box)
{
	glBegin(GL_QUADS);									// Begin Drawing Quads
		// Floor
		glNormal3f(0.0f, 1.0f, 0.0f);					// Normal Pointing Up
		glVertex3f(-10.0f,-10.0f,-20.0f);				// Back Left
		glVertex3f(-10.0f,-10.0f, 20.0f);				// Front Left
		glVertex3f( 10.0f,-10.0f, 20.0f);				// Front Right
		glVertex3f( 10.0f,-10.0f,-20.0f);				// Back Right
		// Ceiling
		glNormal3f(0.0f,-1.0f, 0.0f);					// Normal Point Down
		glVertex3f(-10.0f, 10.0f, 20.0f);				// Front Left
		glVertex3f(-10.0f, 10.0f,-20.0f);				// Back Left
		glVertex3f( 10.0f, 10.0f,-20.0f);				// Back Right
		glVertex3f( 10.0f, 10.0f, 20.0f);				// Front Right
		// Front Wall
		glNormal3f(0.0f, 0.0f, 1.0f);					// Normal Pointing Away From Viewer
		glVertex3f(-10.0f, 10.0f,-20.0f);				// Top Left
		glVertex3f(-10.0f,-10.0f,-20.0f);				// Bottom Left
		glVertex3f( 10.0f,-10.0f,-20.0f);				// Bottom Right
		glVertex3f( 10.0f, 10.0f,-20.0f);				// Top Right
		// Back Wall
		glNormal3f(0.0f, 0.0f,-1.0f);					// Normal Pointing Towards Viewer
		glVertex3f( 10.0f, 10.0f, 20.0f);				// Top Right
		glVertex3f( 10.0f,-10.0f, 20.0f);				// Bottom Right
		glVertex3f(-10.0f,-10.0f, 20.0f);				// Bottom Left
		glVertex3f(-10.0f, 10.0f, 20.0f);				// Top Left
		// Left Wall
		glNormal3f(1.0f, 0.0f, 0.0f);					// Normal Pointing Right
		glVertex3f(-10.0f, 10.0f, 20.0f);				// Top Front
		glVertex3f(-10.0f,-10.0f, 20.0f);				// Bottom Front
		glVertex3f(-10.0f,-10.0f,-20.0f);				// Bottom Back
		glVertex3f(-10.0f, 10.0f,-20.0f);				// Top Back
		// Right Wall
		glNormal3f(-1.0f, 0.0f, 0.0f);					// Normal Pointing Left
		glVertex3f( 10.0f, 10.0f,-20.0f);				// Top Back
		glVertex3f( 10.0f,-10.0f,-20.0f);				// Bottom Back
		glVertex3f( 10.0f,-10.0f, 20.0f);				// Bottom Front
		glVertex3f( 10.0f, 10.0f, 20.0f);				// Top Front
	glEnd();											// Done Drawing Quads
}

BOOL drawGLScene()
{
	GLmatrix16f Minv;
	GLvector4f lp;

	// Clear Color Buffer, Depth Buffer, Stencil Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	glLoadIdentity();									// Reset Modelview Matrix
	glLightfv(GL_LIGHT1, GL_POSITION, LightPos);		// Position Light1
	glTranslatef(0.0f, 0.0f, -20.0f);					// Zoom Into Screen 20 Units
	glTranslatef(SpherePos[0], SpherePos[1], SpherePos[2]);	// Position The Sphere
	gluSphere(q, 1.5f, 32, 16);							// Draw A Sphere

	// calculate light's position relative to local coordinate system
	// dunno if this is the best way to do it, but it actually works
	// if u find another aproach, let me know ;)

	// we build the inversed matrix by doing all the actions in reverse order
	// and with reverse parameters (notice -xrot, -yrot, -ObjPos[], etc.)
	glLoadIdentity();									// Reset Matrix
	glRotatef(-yrot, 0.0f, 1.0f, 0.0f);					// Rotate By -yrot On Y Axis
	glRotatef(-xrot, 1.0f, 0.0f, 0.0f);					// Rotate By -xrot On X Axis
	glTranslatef(-ObjPos[0], -ObjPos[1], -ObjPos[2]);	// Move Negative On All Axis Based On ObjPos[] Values (X, Y, Z)
	glGetFloatv(GL_MODELVIEW_MATRIX,Minv);				// Retrieve ModelView Matrix (Stores In Minv)
	lp[0] = LightPos[0];								// Store Light Position X In lp[0]
	lp[1] = LightPos[1];								// Store Light Position Y In lp[1]
	lp[2] = LightPos[2];								// Store Light Position Z In lp[2]
	lp[3] = LightPos[3];								// Store Light Direction In lp[3]
	VMatMult(Minv, lp);									// We Store Rotated Light Vector In 'lp' Array

	glLoadIdentity();									// Reset Modelview Matrix
	glTranslatef(0.0f, 0.0f, -20.0f);					// Zoom Into The Screen 20 Units
	DrawGLRoom();										// Draw The Room
	glTranslatef(ObjPos[0], ObjPos[1], ObjPos[2]);		// Position The Object
	glRotatef(xrot, 1.0f, 0.0f, 0.0f);					// Spin It On The X Axis By xrot
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);					// Spin It On The Y Axis By yrot
	drawObject(&obj);									// Procedure For Drawing The Loaded Object
	castShadow(&obj, lp);								// Procedure For Casting The Shadow Based On The Silhouette

	glColor4f(0.7f, 0.4f, 0.0f, 1.0f);					// Set Color To An Orange
	glDisable(GL_LIGHTING);								// Disable Lighting
	glDepthMask(GL_FALSE);								// Disable Depth Mask
	glTranslatef(lp[0], lp[1], lp[2]);					// Translate To Light's Position
														// Notice We're Still In Local Coordinate System
	gluSphere(q, 0.2f, 16, 8);							// Draw A Little Yellow Sphere (Represents Light)
	glEnable(GL_LIGHTING);								// Enable Lighting
	glDepthMask(GL_TRUE);								// Enable Depth Mask

	xrot += xspeed;										// Increase xrot By xspeed
	yrot += yspeed;										// Increase yrot By yspeed

	glFlush();											// Flush The OpenGL Pipeline
	return TRUE;										// Everything Went OK
}

int InitGLObjects()										// Initialize Objects
{
	int i;

	if (!readObject("Data28/Object2.txt", &obj))			// Read Object2 Into obj
	{
		return FALSE;									// If Failed Return False
	}

	setConnectivity(&obj);								// Set Face To Face Connectivity

	for ( i=0;i<obj.nFaces;i++)						// Loop Through All Object Faces
		calculatePlane(&obj, &obj.pFaces[i]);				// Compute Plane Equations For All Faces

	return TRUE;										// Return True
}

void KillGLObjects()
{
	killObject( &obj );	
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	if (!InitGLObjects()) return FALSE;					// Function For Initializing Our Object(s)
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glClearStencil(0);									// Stencil Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	glLightfv(GL_LIGHT1, GL_POSITION, LightPos);		// Set Light1 Position
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmb);			// Set Light1 Ambience
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDif);			// Set Light1 Diffuse
	glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpc);		// Set Light1 Specular
	glEnable(GL_LIGHT1);								// Enable Light1
	glEnable(GL_LIGHTING);								// Enable Lighting

	glMaterialfv(GL_FRONT, GL_AMBIENT, MatAmb);			// Set Material Ambience
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MatDif);			// Set Material Diffuse
	glMaterialfv(GL_FRONT, GL_SPECULAR, MatSpc);		// Set Material Specular
	glMaterialfv(GL_FRONT, GL_SHININESS, MatShn);		// Set Material Shininess

	glCullFace(GL_BACK);								// Set Culling Face To Back Face
	glEnable(GL_CULL_FACE);								// Enable Culling
	glClearColor(0.1f, 1.0f, 0.5f, 1.0f);				// Set Clear Color (Greenish Color)

	q = gluNewQuadric();								// Initialize Quadratic
	gluQuadricNormals(q, GL_SMOOTH);					// Enable Smooth Normal Generation
	gluQuadricTexture(q, GL_FALSE);						// Disable Auto Texture Coords

	return TRUE;										// Initialization Went OK
}

void ProcessKeyboard()									// Process Key Presses
{
	// Spin Object
	if (keys[SDLK_LEFT])	yspeed -= 0.1f;					// 'Arrow Left' Decrease yspeed
	if (keys[SDLK_RIGHT])	yspeed += 0.1f;					// 'Arrow Right' Increase yspeed
	if (keys[SDLK_UP])	xspeed -= 0.1f;					// 'Arrow Up' Decrease xspeed
	if (keys[SDLK_DOWN])	xspeed += 0.1f;					// 'Arrow Down' Increase xspeed

	// Adjust Light's Position
	if (keys[SDLK_l]) LightPos[0] += 0.05f;				// 'L' Moves Light Right
	if (keys[SDLK_j]) LightPos[0] -= 0.05f;				// 'J' Moves Light Left

	if (keys[SDLK_i]) LightPos[1] += 0.05f;				// 'I' Moves Light Up
	if (keys[SDLK_k]) LightPos[1] -= 0.05f;				// 'K' Moves Light Down

	if (keys[SDLK_o]) LightPos[2] += 0.05f;				// 'O' Moves Light Toward Viewer
	if (keys[SDLK_u]) LightPos[2] -= 0.05f;				// 'U' Moves Light Away From Viewer

	// Adjust Object's Position
	if (keys[SDLK_KP6]) ObjPos[0] += 0.05f;			// 'Numpad6' Move Object Right
	if (keys[SDLK_KP4]) ObjPos[0] -= 0.05f;			// 'Numpad4' Move Object Left

	if (keys[SDLK_KP8]) ObjPos[1] += 0.05f;			// 'Numpad8' Move Object Up
	if (keys[SDLK_KP5]) ObjPos[1] -= 0.05f;			// 'Numpad5' Move Object Down

	if (keys[SDLK_KP9]) ObjPos[2] += 0.05f;			// 'Numpad9' Move Object Toward Viewer
	if (keys[SDLK_KP7]) ObjPos[2] -= 0.05f;			// 'Numpad7' Move Object Away From Viewer

	// Adjust Ball's Position
	if (keys[SDLK_d]) SpherePos[0] += 0.05f;				// 'D' Move Ball Right
	if (keys[SDLK_a]) SpherePos[0] -= 0.05f;				// 'A' Move Ball Left

	if (keys[SDLK_w]) SpherePos[1] += 0.05f;				// 'W' Move Ball Up
	if (keys[SDLK_s]) SpherePos[1] -= 0.05f;				// 'S' Move Ball Down

	if (keys[SDLK_e]) SpherePos[2] += 0.05f;				// 'E' Move Ball Toward Viewer
	if (keys[SDLK_q]) SpherePos[2] -= 0.05f;				// 'Q' Move Ball Away From Viewer
}

/************** THIS IS THE STANDARD NEHE TUTORIAL STUFF - LOOK BACK TO LESSON ONE! **************/

BOOL active=TRUE;										// Window Active Flag Set To TRUE By Default
BOOL fullscreen=FALSE;									// Fullscreen Flag Set To Fullscreen Mode By Default

static const char *windowTitle = "Banu Octavian, Brett Porter & NeHe's Shadow Casting Tutorial";

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

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	KillGLObjects();
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
 
BOOL CreateGLWindow(const char* title, int width, int height, int bits, BOOL fullscreenflag)
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
	if (!CreateGLWindow(windowTitle,800,600,32,fullscreen))
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
		if ((active && !drawGLScene()) || keys[SDLK_ESCAPE])	// Active?  Was There A Quit Received?
		{
			done=TRUE;							// ESC or DrawGLScene Signalled A Quit
		}
		else									// Not Time To Quit, Update Screen
		{
			SDL_GL_SwapBuffers();					// Swap Buffers (Double Buffering)
			ProcessKeyboard();						// Process Key Presses
		}

		if ( keys[SDLK_F1] )
		{
			KillGLWindow();
			if (!CreateGLWindow( windowTitle,800,600,32,!fullscreen))
			{
				done=FALSE;										// Quit If Window Was Not Created
			}
		}
	}

	// Shutdown
	KillGLWindow();										// Kill The Window
	SDL_Quit();
	return 0;											// Exit The Program
}

#include <stdio.h>
// yes, I know stdio.h is not good C++, but I like the *printf( )
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"
#include "glui.h"

//	This is a sample OpenGL / GLUT / GLUI program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with radio buttons
//
//	The left mouse button allows rotation
//	The middle mouse button allows scaling
//	The glui window allows:
//		1. The 3d object to be transformed
//		2. The projection to be changed
//		3. The color of the axes to be changed
//		4. The axes to be turned on and off
//		5. The transformations to be reset
//		6. The program to quit
//
//	Author:			Joe Graphics
//
//  Latest update:	March 26, 2015

// constants:
//
// NOTE: There are a bunch of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.
//

#define NUMLNGS		201
#define NUMLATS		105
// title of these windows:

const char *WINDOWTITLE = { "OpenGL / GLUT / GLUI  Project #8 -- Author: Yu Zhang" };
const char *GLUITITLE = { "User Interface Window" };

const char *SUBWINDOWTITLE = { "CS553 Pro#8 Subwindow" };
// what the glui package defines as true and false:

const int GLUITRUE = { true };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE		0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// size of the box:

const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// able to use the left mouse for either rotation or scaling,
// in case have only a 2-button mouse:

enum LeftButton
{
	ROTATE,
	SCALE
};


// minimum allowable scale factor:

const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):

const int LEFT = { 4 };
const int MIDDLE = { 2 };
const int RIGHT = { 1 };


// which projection:

enum Projections
{
	ORTHO,
	PERSP
};


// which button:

enum ButtonVals
{
	RESET,
	QUIT
};


// window background color (rgba):

const float BACKCOLOR[] = { 0., 0., 0., 0. };


// line width for the axes:

const GLfloat AXES_WIDTH = { 3. };


// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

enum WhichTexture
{
	NoTexture,
	GL_Rep,
	GL_Mod
};

enum TextureFilter
{
	Nearest,
	Linear
};

// the color definitions:
// this order must match the radio button order

const GLfloat Colors[][3] =
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};


// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE = { GL_LINEAR };
const GLfloat FOGDENSITY = { 0.30f };
const GLfloat FOGSTART = { 1.5 };
const GLfloat FOGEND = { 4. };

float HeightExa;
float LHF;

const float LNGMIN = { -289.6f };
const float LNGMAX = { 289.6f };
const float LATMIN = { -197.5f };
const float LATMAX = { 211.2f };
const float HGTMIN = { 0.0f };
const float HGTMAX = { 2.891f };
const int PERIOD_MS = 10 * 1000;

const int n = 1000;
const GLfloat r = 50.f;
const GLfloat Pi = 3.1415926536f;

// non-constant global variables:

int	ActiveButton;			// current button that is down
GLuint	AxesList;			// list to hold the axes
int	AxesOn;					// != 0 means to draw the axes
int	DebugOn;				// != 0 means to print debugging info
int	DepthCueOn;				// != 0 means to use intensity depth cueing
GLUI *	Glui;				// instance of glui window
int	GluiWindow;				// the glut id for the glui window
int	LeftButton;				// either ROTATE or SCALE
GLuint	BoxList;			// object display list
int	MainWindow;				// window id for main graphics window
int SubWindowA;
int SubWindowB;
GLfloat	RotMatrix[4][4];	// set by glui rotation widget
float	Scale, Scale2;		// scaling factors
int	WhichColor;				// index into Colors[ ]
int	WhichProjection;		// ORTHO or PERSP
int WhichLighting;
int WhichTextureFilter;
int WhichTextureEnvironment;
int	Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;			// rotation angles in degrees
float	TransXYZ[3];		// set by glui translation widgets
int CloudOn;
int Paused;
GLuint TerrainTex;
GLuint subTerrainTex;
float Time;
float R = 200;
// function prototypes:

void	Animate();
void	Buttons(int);
void	Display();
void	DoRasterString(float, float, float, char *);
void	DoStrokeString(float, float, float, float, char *);
float	ElapsedSeconds();
void	InitGlui();
void	InitGraphics();
void	InitLists();
void	Keyboard(unsigned char, int, int);
void	MouseButton(int, int, int, int);
void	MouseMotion(int, int);
void	Reset();
void	Resize(int, int);
void	Visibility(int);

void	Arrow(float[3], float[3]);
void	Cross(float[3], float[3], float[3]);
float	Dot(float[3], float[3]);
float	Unit(float[3], float[3]);
void	Axes(float);
void	HsvRgb(float[3], float[3]);
unsigned char *BmpToTexture(char *, int *, int *);
int		ReadInt(FILE *);
short	ReadShort(FILE *);
void DrawCircle();
void DisplayA();
void DisplayB();

struct bmfh
{
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	int bfOffBits;
} FileHeader;

struct bmih
{
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} InfoHeader;

const int birgb = { 0 };

struct LngLatHgt
{
	float lng, lat, hgt;
	float s, t;
};
struct LngLatHgt Points[NUMLATS][NUMLNGS];

void TexFi(int id)
{

}

void TexEn(int id)
{

}

void DisplayA()
{
	glDisable(GL_DEPTH_TEST);

	if (DebugOn != 0)
	{
		fprintf(stderr, "Display\n");
	}

	// set which window we want to do the graphics into:

	glutSetWindow(SubWindowA);

	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_FLAT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1., 1., -1., 1.);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1., 1., 1.);

	//draw things here
	glLineWidth(2.0);
	glColor3f(1., 1., 0.);
	glBegin(GL_LINES);
	glVertex2f(-0.9,0.2);
	glVertex2f(-0.9,0.9);

	glVertex2f(-0.9, 0.2);
	glVertex2f(0.9, 0.2);

	glVertex2f(-0.9, 0.55);
	glVertex2f(0.9, 0.55);

	glVertex2f(-0.9, -0.9);
	glVertex2f(-0.9, -0.2);

	glVertex2f(-0.9, -0.9);
	glVertex2f(0.9, -0.9);

	glVertex2f(-0.9, -0.55);
	glVertex2f(0.9, -0.55);
	glEnd();

	glLineWidth(2.0);
	glColor3f(0., 1., 0.);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i <= 100; i++)
	{
		glVertex2f(1.8/100 * i - 0.9, 0.35 * cos(2*Pi * i / 100)+ 0.55);
		glVertex2f(1.8 / 100 * (i + 1) - 0.9, 0.35* cos(2 * Pi * (i + 1) / 100) + 0.55);
	}
	glEnd();

	glColor3f(0., 1., 0.);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i <= 100; i++)
	{
		glVertex2f(1.8 / 100 * i - 0.9, -(0.35 * sin(2 * Pi * i / 100)) - 0.55);
		glVertex2f(1.8 / 100 * (i + 1) - 0.9, -(0.35 * sin(2 * Pi * (i + 1) / 100)) -0.55);
	}
	glEnd();

	glColor3f(1., 0., 0.);
	glBegin(GL_QUADS);
	glVertex2f(1.8  * Time - 0.9 + 0.03, (0.35 * cos(2 * Pi * Time)) + 0.55 + 0.03);
	glVertex2f(1.8 * Time - 0.9 + 0.03, (0.35 * cos(2 * Pi * Time)) + 0.55 - 0.03);
	glVertex2f(1.8  * Time - 0.9 - 0.03, (0.35 * cos(2 * Pi * Time)) + 0.55 - 0.03 );
	glVertex2f(1.8  * Time - 0.9 - 0.03, (0.35 * cos(2 * Pi * Time)) + 0.55 + 0.03);
	glEnd();

	glBegin(GL_QUADS);
	glVertex2f(1.8 * Time - 0.9 + 0.03, -(0.35 * sin(2 * Pi * Time)) - 0.55 + 0.03);
	glVertex2f(1.8 * Time - 0.9 + 0.03, -(0.35 * sin(2 * Pi * Time)) - 0.55 - 0.03);
	glVertex2f(1.8 * Time - 0.9 - 0.03, -(0.35 * sin(2 * Pi * Time)) - 0.55 - 0.03);
	glVertex2f(1.8 * Time - 0.9 - 0.03, -(0.35 * sin(2 * Pi * Time)) - 0.55 + 0.03);
	glEnd();
	glutSwapBuffers();
	glFlush();
}

void DisplayB()
{
	if (DebugOn != 0)
	{
		fprintf(stderr, "Display\n");
	}


	// set which window we want to do the graphics into:

	glutSetWindow(SubWindowB);
	// erase the background:

	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);


	// specify shading to be flat:

	glShadeModel(GL_FLAT);


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (WhichProjection == ORTHO)
		glOrtho(-3., 3., -3., 3., 0.1, 1000.);
	else
		gluPerspective(90., 1., 0.1, 1000.);


	// place the objects into the scene:

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// set the eye position, look-at position, and up-vector:
	// IF DOING 2D, REMOVE THIS -- OTHERWISE ALL YOUR 2D WILL DISAPPEAR !
//	R*cos(2 * Pi * Time), 50, r * sin(2 * Pi / n * i) + R*sin(2 * Pi * Time)

	gluLookAt(R*cos(2 * Pi * Time), 70, R*sin(2 * Pi * Time), R / cos(Pi/6) * cos(Pi/4 + 2 * Pi * Time),68, R / cos(Pi/6) * sin(Pi/4 + 2 * Pi * Time), 0., 1., 0.);

	//gluLookAt(R*cos(2 * Pi * Time), 55, R*sin(2 * Pi * Time),0,0,0, 0., 1., 0.);
	// translate the objects in the scene:
	// note the minus sign on the z value
	// this is to make the appearance of the glui z translate
	// widget more intuitively match the translate behavior
	// DO NOT TRANSLATE IN Z IF YOU ARE DOING 2D !

	glTranslatef((GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2]);


	//// rotate the scene:
	//// DO NOT ROTATE (EXCEPT ABOUT Z) IF YOU ARE DOING 2D !

	//glRotatef((GLfloat)Yrot, 0., 1., 0.);
	//glRotatef((GLfloat)Xrot, 1., 0., 0.);
	//glMultMatrixf((const GLfloat *)RotMatrix);


	// uniformly scale the scene:

	//glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
	//GLfloat scale2 = 1. + Scale2;		// because glui translation starts at 0.
	//if (scale2 < MINSCALE)
	//	scale2 = MINSCALE;
	//glScalef((GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2);


	// set the fog parameters:
	// DON'T NEED THIS IF DOING 2D !

	if (DepthCueOn != 0)
	{
		glFogi(GL_FOG_MODE, FOGMODE);
		glFogfv(GL_FOG_COLOR, FOGCOLOR);
		glFogf(GL_FOG_DENSITY, FOGDENSITY);
		glFogf(GL_FOG_START, FOGSTART);
		glFogf(GL_FOG_END, FOGEND);
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}

	//TEXTURE MAPPING

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	switch (WhichTextureEnvironment)
	{
		case NoTexture:
			glDisable(GL_TEXTURE_2D);
			break;
		case GL_Rep:
			glEnable(GL_TEXTURE_2D);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			break;
		case GL_Mod:
			glEnable(GL_TEXTURE_2D);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	default:
		break;
	}

	switch (WhichTextureFilter)
	{
	case Nearest:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	case Linear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	}

	//Computing Per-triangle Lighting:
	if (WhichLighting == 0)
	{
		float v01[3], v02[3], norm[3], inten;
		glBindTexture(GL_TEXTURE_2D, subTerrainTex);
		glShadeModel(GL_FLAT);
		glBegin(GL_TRIANGLES);
		
		for (int z = 0; z < NUMLATS - 1; z++)
		{
			for (int x = 0; x < NUMLNGS - 1; x++)
			{
				//x,z; x,z+1; x+1, z+1

				//first triangle
				v01[0] = Points[z][x + 1].lng - Points[z][x].lng;
				v01[2] = Points[z][x + 1].lat - Points[z][x].lat;
				v01[1] = HeightExa * LHF * (Points[z][x + 1].hgt - Points[z][x].hgt);

				v02[0] = Points[z + 1][x].lng - Points[z][x].lng;
				v02[2] = Points[z + 1][x].lat - Points[z][x].lat;
				v02[1] = HeightExa * LHF * (Points[z + 1][x].hgt - Points[z][x].hgt);

				Cross(v01, v02, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z][x].s, Points[z][x].t);
				glVertex3f(Points[z][x].lng, Points[z][x].hgt * HeightExa, Points[z][x].lat);
				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt * HeightExa, Points[z + 1][x].lat);
				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt * HeightExa, Points[z][x + 1].lat);

				//second triangle
				v01[0] = Points[z + 1][x + 1].lng - Points[z][x + 1].lng;
				v01[2] = Points[z + 1][x + 1].lat - Points[z][x + 1].lat;
				v01[1] = HeightExa * LHF * (Points[z + 1][x + 1].hgt - Points[z][x + 1].hgt);


				v02[0] = Points[z + 1][x].lng - Points[z][x + 1].lng;
				v02[2] = Points[z + 1][x].lat - Points[z][x + 1].lat;
				v02[1] = HeightExa * LHF * (Points[z + 1][x].hgt - Points[z][x + 1].hgt);

				Cross(v01, v02, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);
				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt * HeightExa, Points[z + 1][x].lat);
				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt * HeightExa, Points[z][x + 1].lat);
				glTexCoord2f(Points[z + 1][x + 1].s, Points[z + 1][x + 1].t);
				glVertex3f(Points[z + 1][x + 1].lng, Points[z + 1][x + 1].hgt * HeightExa, Points[z + 1][x + 1].lat);


			}
		}
		glEnd();
	}
	
	//Computing Per-vertex Lighting
	else if (WhichLighting != 0)
	{
		float gradLongA[3], gradLatA[3], normA[3], intenA;
		float gradLongB[3], gradLatB[3], normB[3], intenB;
		float gradLongC[3], gradLatC[3], normC[3], intenC;
		float gradLongD[3], gradLatD[3], normD[3], intenD;

		glBindTexture(GL_TEXTURE_2D, subTerrainTex);
		glShadeModel(GL_SMOOTH);
		glBegin(GL_TRIANGLES);
	

		for (int z = 0; z < NUMLATS - 1; z++)
		{
			for (int x = 0; x < NUMLNGS - 1; x++)
			{
				//Z =0; X = 0;
				gradLongA[0] = Points[z][x + 1].lng - Points[z][x - 1].lng;
				gradLongA[1] = HeightExa*LHF*(Points[z][x + 1].hgt - Points[z][x - 1].hgt);
				gradLongA[2] = 0.;
				gradLatA[0] = 0.;
				gradLatA[1] = HeightExa*LHF*(Points[z + 1][x].hgt - Points[z - 1][x].hgt);
				gradLatA[2] = Points[z + 1][x].lat - Points[z - 1][x].lat;
				Cross(gradLongA, gradLatA, normA);
				Unit(normA, normA);
				intenA = fabs(normA[1]);

				//Z = 1; X =0
				gradLongB[0] = Points[z + 1][x + 1].lng - Points[z + 1][x - 1].lng;
				gradLongB[1] = HeightExa*LHF*(Points[z + 1][x + 1].hgt - Points[z + 1][x - 1].hgt);
				gradLongB[2] = 0.;
				gradLatB[0] = 0.;
				gradLatB[1] = HeightExa*LHF*(Points[z + 2][x].hgt - Points[z][x].hgt);
				gradLatB[2] = Points[z + 2][x].lat - Points[z][x].lat;
				Cross(gradLongB, gradLatB, normB);
				Unit(normB, normB);
				intenB = fabs(normB[1]);

				//Z = 0; X = 1
				gradLongC[0] = Points[z][x + 2].lng - Points[z][x].lng;
				gradLongC[1] = HeightExa*LHF*(Points[z][x + 2].hgt - Points[z][x].hgt);
				gradLongC[2] = 0.;
				gradLatC[0] = 0.;
				gradLatC[1] = HeightExa*LHF*(Points[z + 1][x + 1].hgt - Points[z - 1][x + 1].hgt);
				gradLatC[2] = Points[z + 1][x + 1].lat - Points[z - 1][x + 1].lat;
				Cross(gradLongC, gradLatC, normC);
				Unit(normC, normC);
				intenC = fabs(normC[1]);

				//Z =1; X = 1
				gradLongD[0] = Points[z + 1][x + 2].lng - Points[z + 1][x].lng;
				gradLongD[1] = HeightExa*LHF*(Points[z + 1][x + 2].hgt - Points[z + 1][x].hgt);
				gradLongD[2] = 0.;
				gradLatD[0] = 0.;
				gradLatD[1] = HeightExa*LHF*(Points[z + 2][x + 1].hgt - Points[z][x + 1].hgt);
				gradLatD[2] = Points[z + 2][x + 1].lat - Points[z][x + 1].lat;
				Cross(gradLongD, gradLatD, normD);
				Unit(normD, normD);
				intenD = fabs(normD[1]);
				
				//first triangle
				glColor3f(intenA, intenA, intenA);
				glTexCoord2f(Points[z][x].s, Points[z][x].t);
				glVertex3f(Points[z][x].lng, Points[z][x].hgt * HeightExa, Points[z][x].lat); //(0,0)
				
				glColor3f(intenB, intenB, intenB);
				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt * HeightExa, Points[z + 1][x].lat); //(0,1)
				
				glColor3f(intenC, intenC, intenC);
				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt * HeightExa, Points[z][x + 1].lat); //(1,0)
			
				//second triangle
				glColor3f(intenB, intenB, intenB);
				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt * HeightExa, Points[z + 1][x].lat); //(0,1)

				glColor3f(intenC, intenC, intenC);
				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt * HeightExa, Points[z][x + 1].lat); //(1,0)

				glColor3f(intenD, intenD, intenD);
				glTexCoord2f(Points[z + 1][x + 1].s, Points[z + 1][x + 1].t);
				glVertex3f(Points[z + 1][x + 1].lng, Points[z + 1][x + 1].hgt * HeightExa, Points[z + 1][x + 1].lat); //(1,1)
			}
		}
		glEnd();

	}
	
	glDisable(GL_TEXTURE_2D);

	//CLOUD ON

		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		DrawCircle();

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);

	
	


	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0., 100., 0., 100.);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1., 1., 1.);



	// swap the double-buffered framebuffers:

	glutSwapBuffers();


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();
}
// main program:

int
main(int argc, char *argv[])
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit(&argc, argv);


	// setup all the graphics stuff:

	InitGraphics();


	// create the display structures that will not change:

	InitLists();


	// init all the global variables used by Display( ):
	// this will also post a redisplay
	// it is important to call this before InitGlui( )
	// so that the variables that glui will control are correct
	// when each glui widget is created

	Reset();


	// setup all the user interface stuff:

	InitGlui();


	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutMainLoop();


	// this is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate()
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:
	// force a call to Display( ) next time it is convenient:

	glutSetWindow(MainWindow);
	glutPostRedisplay();

	glutSetWindow(SubWindowA);
	glutPostRedisplay();

	int ms = glutGet(GLUT_ELAPSED_TIME);
	
	ms = ms % PERIOD_MS;
	Time = (float)ms / (float)(PERIOD_MS - 1);

	glutSetWindow(MainWindow);
	glutPostRedisplay();

	glutSetWindow(SubWindowB);
	glutPostRedisplay();

	glutSetWindow(SubWindowA);
	glutPostRedisplay();
}



// glui buttons callback:

void
Buttons(int id)
{
	switch (id)
	{
	case RESET:
		Reset();
		Glui->sync_live();
		glutSetWindow(MainWindow);
		glutPostRedisplay();
		break;

	case QUIT:
		// gracefully close the glui window:
		// gracefully close out the graphics:
		// gracefully close the graphics window:
		// gracefully exit the program:

		Glui->close();
		glutSetWindow(MainWindow);
		glFinish();
		glutDestroyWindow(MainWindow);
		exit(0);
		break;

	default:
		fprintf(stderr, "Don't know what to do with Button ID %d\n", id);
	}

}


// draw the complete scene:

void
Display()
{
	if (DebugOn != 0)
	{
		fprintf(stderr, "Display\n");
	}


	// set which window we want to do the graphics into:

	glutSetWindow(MainWindow);




	// erase the background:

	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);


	// specify shading to be flat:

	glShadeModel(GL_FLAT);


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (WhichProjection == ORTHO)
		glOrtho(-3., 3., -3., 3., 0.1, 1000.);
	else
		gluPerspective(90., 1., 0.1, 1000.);


	// place the objects into the scene:

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// set the eye position, look-at position, and up-vector:
	// IF DOING 2D, REMOVE THIS -- OTHERWISE ALL YOUR 2D WILL DISAPPEAR !

	gluLookAt(-150., 300., 200., 0., 0., 0., 0., 1., 0.);


	// translate the objects in the scene:
	// note the minus sign on the z value
	// this is to make the appearance of the glui z translate
	// widget more intuitively match the translate behavior
	// DO NOT TRANSLATE IN Z IF YOU ARE DOING 2D !

	glTranslatef((GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2]);


	// rotate the scene:
	// DO NOT ROTATE (EXCEPT ABOUT Z) IF YOU ARE DOING 2D !

	glRotatef((GLfloat)Yrot, 0., 1., 0.);
	glRotatef((GLfloat)Xrot, 1., 0., 0.);
	glMultMatrixf((const GLfloat *)RotMatrix);


	// uniformly scale the scene:

	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
	GLfloat scale2 = 1. + Scale2;		// because glui translation starts at 0.
	if (scale2 < MINSCALE)
		scale2 = MINSCALE;
	glScalef((GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2);


	// set the fog parameters:
	// DON'T NEED THIS IF DOING 2D !

	if (DepthCueOn != 0)
	{
		glFogi(GL_FOG_MODE, FOGMODE);
		glFogfv(GL_FOG_COLOR, FOGCOLOR);
		glFogf(GL_FOG_DENSITY, FOGDENSITY);
		glFogf(GL_FOG_START, FOGSTART);
		glFogf(GL_FOG_END, FOGEND);
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}


	// possibly draw the axes:

	if (AxesOn != 0)
	{
		glColor3fv(&Colors[WhichColor][0]);
		glCallList(AxesList);
	}

	//TEXTURE MAPPING

	

	//project8

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	switch (WhichTextureEnvironment)
	{
		case NoTexture:
			glDisable(GL_TEXTURE_2D);
			break;
		case GL_Rep:
			glEnable(GL_TEXTURE_2D);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			break;
		case GL_Mod:
			glEnable(GL_TEXTURE_2D);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	default:
		break;
	}

	switch (WhichTextureFilter)
	{
	case Nearest:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	case Linear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	}

	//Computing Per-triangle Lighting:
	if (WhichLighting == 0)
	{
		float v01[3], v02[3], norm[3], inten;
		glBindTexture(GL_TEXTURE_2D, TerrainTex);
		glShadeModel(GL_FLAT);
		glBegin(GL_TRIANGLES);
		
		for (int z = 0; z < NUMLATS - 1; z++)
		{
			for (int x = 0; x < NUMLNGS - 1; x++)
			{
				//x,z; x,z+1; x+1, z+1

				//first triangle
				v01[0] = Points[z][x + 1].lng - Points[z][x].lng;
				v01[2] = Points[z][x + 1].lat - Points[z][x].lat;
				v01[1] = HeightExa * LHF * (Points[z][x + 1].hgt - Points[z][x].hgt);

				v02[0] = Points[z + 1][x].lng - Points[z][x].lng;
				v02[2] = Points[z + 1][x].lat - Points[z][x].lat;
				v02[1] = HeightExa * LHF * (Points[z + 1][x].hgt - Points[z][x].hgt);

				Cross(v01, v02, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z][x].s, Points[z][x].t);
				glVertex3f(Points[z][x].lng, Points[z][x].hgt * HeightExa, Points[z][x].lat);
				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt * HeightExa, Points[z + 1][x].lat);
				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt * HeightExa, Points[z][x + 1].lat);

				//second triangle
				v01[0] = Points[z + 1][x + 1].lng - Points[z][x + 1].lng;
				v01[2] = Points[z + 1][x + 1].lat - Points[z][x + 1].lat;
				v01[1] = HeightExa * LHF * (Points[z + 1][x + 1].hgt - Points[z][x + 1].hgt);


				v02[0] = Points[z + 1][x].lng - Points[z][x + 1].lng;
				v02[2] = Points[z + 1][x].lat - Points[z][x + 1].lat;
				v02[1] = HeightExa * LHF * (Points[z + 1][x].hgt - Points[z][x + 1].hgt);

				Cross(v01, v02, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);
				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt * HeightExa, Points[z + 1][x].lat);
				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt * HeightExa, Points[z][x + 1].lat);
				glTexCoord2f(Points[z + 1][x + 1].s, Points[z + 1][x + 1].t);
				glVertex3f(Points[z + 1][x + 1].lng, Points[z + 1][x + 1].hgt * HeightExa, Points[z + 1][x + 1].lat);


			}
		}
		glEnd();
	}
	
	//Computing Per-triangle Lighting:
	else if (WhichLighting != 0)
	{
		float gradLongA[3], gradLatA[3], normA[3], intenA;
		float gradLongB[3], gradLatB[3], normB[3], intenB;
		float gradLongC[3], gradLatC[3], normC[3], intenC;
		float gradLongD[3], gradLatD[3], normD[3], intenD;

		glBindTexture(GL_TEXTURE_2D, TerrainTex);
		glShadeModel(GL_SMOOTH);
		glBegin(GL_TRIANGLES);
	

		for (int z = 0; z < NUMLATS - 1; z++)
		{
			for (int x = 0; x < NUMLNGS - 1; x++)
			{
				//Z =0; X = 0;
				gradLongA[0] = Points[z][x + 1].lng - Points[z][x - 1].lng;
				gradLongA[1] = HeightExa*LHF*(Points[z][x + 1].hgt - Points[z][x - 1].hgt);
				gradLongA[2] = 0.;
				gradLatA[0] = 0.;
				gradLatA[1] = HeightExa*LHF*(Points[z + 1][x].hgt - Points[z - 1][x].hgt);
				gradLatA[2] = Points[z + 1][x].lat - Points[z - 1][x].lat;
				Cross(gradLongA, gradLatA, normA);
				Unit(normA, normA);
				intenA = fabs(normA[1]);

				//Z = 1; X =0
				gradLongB[0] = Points[z + 1][x + 1].lng - Points[z + 1][x - 1].lng;
				gradLongB[1] = HeightExa*LHF*(Points[z + 1][x + 1].hgt - Points[z + 1][x - 1].hgt);
				gradLongB[2] = 0.;
				gradLatB[0] = 0.;
				gradLatB[1] = HeightExa*LHF*(Points[z + 2][x].hgt - Points[z][x].hgt);
				gradLatB[2] = Points[z + 2][x].lat - Points[z][x].lat;
				Cross(gradLongB, gradLatB, normB);
				Unit(normB, normB);
				intenB = fabs(normB[1]);

				//Z = 0; X = 1
				gradLongC[0] = Points[z][x + 2].lng - Points[z][x].lng;
				gradLongC[1] = HeightExa*LHF*(Points[z][x + 2].hgt - Points[z][x].hgt);
				gradLongC[2] = 0.;
				gradLatC[0] = 0.;
				gradLatC[1] = HeightExa*LHF*(Points[z + 1][x + 1].hgt - Points[z - 1][x + 1].hgt);
				gradLatC[2] = Points[z + 1][x + 1].lat - Points[z - 1][x + 1].lat;
				Cross(gradLongC, gradLatC, normC);
				Unit(normC, normC);
				intenC = fabs(normC[1]);

				//Z =1; X = 1
				gradLongD[0] = Points[z + 1][x + 2].lng - Points[z + 1][x].lng;
				gradLongD[1] = HeightExa*LHF*(Points[z + 1][x + 2].hgt - Points[z + 1][x].hgt);
				gradLongD[2] = 0.;
				gradLatD[0] = 0.;
				gradLatD[1] = HeightExa*LHF*(Points[z + 2][x + 1].hgt - Points[z][x + 1].hgt);
				gradLatD[2] = Points[z + 2][x + 1].lat - Points[z][x + 1].lat;
				Cross(gradLongD, gradLatD, normD);
				Unit(normD, normD);
				intenD = fabs(normD[1]);
				
				//first triangle
				glColor3f(intenA, intenA, intenA);
				glTexCoord2f(Points[z][x].s, Points[z][x].t);
				glVertex3f(Points[z][x].lng, Points[z][x].hgt * HeightExa, Points[z][x].lat); //(0,0)
				
				glColor3f(intenB, intenB, intenB);
				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt * HeightExa, Points[z + 1][x].lat); //(0,1)
				
				glColor3f(intenC, intenC, intenC);
				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt * HeightExa, Points[z][x + 1].lat); //(1,0)
			
				//second triangle
				glColor3f(intenB, intenB, intenB);
				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt * HeightExa, Points[z + 1][x].lat); //(0,1)

				glColor3f(intenC, intenC, intenC);
				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt * HeightExa, Points[z][x + 1].lat); //(1,0)

				glColor3f(intenD, intenD, intenD);
				glTexCoord2f(Points[z + 1][x + 1].s, Points[z + 1][x + 1].t);
				glVertex3f(Points[z + 1][x + 1].lng, Points[z + 1][x + 1].hgt * HeightExa, Points[z + 1][x + 1].lat); //(1,1)
			}
		}
		glEnd();

	}
	
	glDisable(GL_TEXTURE_2D);

	//CLOUD ON
	if (CloudOn == 1)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		DrawCircle();

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
	// set the color of the object:

	glColor3fv(Colors[WhichColor]);


	// draw the current object:

	glCallList(BoxList);





	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0., 100., 0., 100.);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1., 1., 1.);



	// swap the double-buffered framebuffers:

	glutSwapBuffers();


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();
}


// use glut to display a string of characters using a raster font:

void
DoRasterString(float x, float y, float z, char *s)
{
	char c;			// one character to print

	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);
	for (; (c = *s) != '\0'; s++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString(float x, float y, float z, float ht, char *s)
{
	char c;			// one character to print

	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
	float sf = ht / (119.05 + 33.33);
	glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
	for (; (c = *s) != '\0'; s++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
	}
	glPopMatrix();
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds()
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:

	return (float)ms / 1000.;
}


// initialize the glui window:

void
InitGlui()
{

	GLUI_Panel *panel, *bigpanel;
	GLUI_RadioGroup *group;
	GLUI_Rotation *rot;
	GLUI_Translation *trans, *scale;
	GLUI_HSlider *slider;
	GLUI_Spinner *spinner;
	char str[256];


	// setup the glui window:

	glutInitWindowPosition(INIT_WINDOW_SIZE + 50, 0);
	Glui = GLUI_Master.create_glui((char *)GLUITITLE);


	Glui->add_statictext((char *)GLUITITLE);
	Glui->add_separator();

	Glui->add_checkbox("Axes", &AxesOn);
	Glui->add_checkbox("Perspective", &WhichProjection);

	Glui->add_checkbox("Intensity Depth Cue", &DepthCueOn);
	Glui->add_checkbox("Cloud", &CloudOn);
	panel = Glui->add_panel("Texture Filtering");
	group = Glui->add_radiogroup_to_panel(panel, &WhichTextureFilter, 0, (GLUI_Update_CB)TexFi);
	Glui->add_radiobutton_to_group(group, "GL_NEAREST");
	Glui->add_radiobutton_to_group(group, "GL_LINEAR");

	panel = Glui->add_panel("Texture Environment");
	group = Glui->add_radiogroup_to_panel(panel, &WhichTextureEnvironment, 0, (GLUI_Update_CB)TexEn);
	Glui->add_radiobutton_to_group(group, "No TEXTURE");
	Glui->add_radiobutton_to_group(group, "GL_REPLACE");
	Glui->add_radiobutton_to_group(group, "GL_MODULATE");

	Glui->add_checkbox("UseSmoothLighting", &WhichLighting);
	spinner = Glui->add_spinner("Height Exaggeration", GLUI_SPINNER_FLOAT, &HeightExa, 0);
	spinner->set_float_limits(2., 100.);
	spinner->set_speed(4.0);

	//careful the next lines
	spinner = Glui->add_spinner("Lighting Height Factor", GLUI_SPINNER_FLOAT, &LHF, 0);
	spinner->set_float_limits(2., 100.);
	spinner->set_speed(4.0);


	panel = Glui->add_panel("Object Transformation");

	rot = Glui->add_rotation_to_panel(panel, "Rotation", (float *)RotMatrix);



	// allow the object to be spun via the glui rotation widget:
	rot->set_spin(1.0);


	Glui->add_column_to_panel(panel, GLUIFALSE);
	scale = Glui->add_translation_to_panel(panel, "Scale", GLUI_TRANSLATION_Y, &Scale2);
	scale->set_speed(0.005f);

	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0]);
	trans->set_speed(0.05f);

	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Trans Z", GLUI_TRANSLATION_Z, &TransXYZ[2]);
	trans->set_speed(0.05f);

	Glui->add_checkbox("Debug", &DebugOn);


	panel = Glui->add_panel("", GLUIFALSE);

	Glui->add_button_to_panel(panel, "Reset", RESET, (GLUI_Update_CB)Buttons);

	Glui->add_column_to_panel(panel, GLUIFALSE);

	Glui->add_button_to_panel(panel, "Quit", QUIT, (GLUI_Update_CB)Buttons);

	// tell glui what graphics window it needs to post a redisplay to:

	Glui->set_main_gfx_window(MainWindow);
	
	Glui->set_main_gfx_window(SubWindowB);
	// set the graphics window's idle function if needed:
	GLUI_Master.set_glutIdleFunc(Animate);
}


// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics()
{
	// setup the display mode:
	// ( *must* be done before call to glutCreateWindow( ) )
	// ask for color, double-buffering, and z-buffering:

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);


	// set the initial window configuration:

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);


	// open the window and set its title:

	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);

	//open the sub window A and set its title:
	glutInitWindowPosition(0, 670);
	glutInitWindowSize(300, 300);

	SubWindowA = glutCreateWindow(SUBWINDOWTITLE);
	glutSetWindowTitle(SUBWINDOWTITLE);

	//open the sub window B and set its title :
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(330, 670);
	glutInitWindowSize(300, 300);

	SubWindowB = glutCreateWindow(SUBWINDOWTITLE);
	glutSetWindowTitle(SUBWINDOWTITLE);
	// setup the clear values:
	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);


	// setup the callback functions:

	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on



	glutSetWindow(MainWindow);
	
	glutDisplayFunc(Display);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(NULL);
	glutVisibilityFunc(Visibility);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStateFunc(NULL);
	glutTimerFunc(0, NULL, 0);



	FILE *fp = fopen("or.hgt", "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Can't open 'or.hgt'\n");
		
	}

	float lat, lng;
	float dlng = (abs(LNGMAX) + abs(LNGMIN))/NUMLNGS;
	float dlat = (abs(LATMAX) + abs(LATMIN))/NUMLATS;
	int	z, x;

	for (z = 0, lat = LATMAX; z < NUMLATS; z++, lat -= dlat)
	{
		for(x = 0, lng = LNGMIN; x < NUMLNGS; x++, lng += dlng)
		{
			Points[z][x].lng = lng;
			Points[z][x].lat = lat;
			fscanf(fp, "%f", &Points[z][x].hgt);
		}
	}


	for (z = 0, lat = LATMAX; z < NUMLATS; z++, lat -= dlat)
	{
		for (x = 0, lng = LNGMIN; x < NUMLNGS; x++, lng += dlng)
		{
			Points[z][x].s = (lng - LNGMIN) / (LNGMAX - LNGMIN);	// 0. -> 1.
			Points[z][x].t = (lat - LATMIN) / (LATMAX - LATMIN);	// 0. -> 1.
			Points[z][x].t = Points[z][x].t * 0.718;
		}
	}

	unsigned char *BmpToTexture(char *filename, int *widthp, int *heightp);
	unsigned char *Texture;
	int Width, Height;

	Texture = BmpToTexture("Oregon2048.bmp", &Width, &Height);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &TerrainTex);
	glBindTexture(GL_TEXTURE_2D, TerrainTex);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);

	glutSetWindow(SubWindowA);
	glutDisplayFunc(DisplayA);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);

	glutSetWindow(SubWindowB);
	glutDisplayFunc(DisplayB);
	glutReshapeFunc(Resize);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &subTerrainTex);
	glBindTexture(GL_TEXTURE_2D, subTerrainTex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists()
{
	
	//float dx = BOXSIZE / 2.;
	//float dy = BOXSIZE / 2.;
	//float dz = BOXSIZE / 2.;

	//// create the object:

	//BoxList = glGenLists(1);
	//glNewList(BoxList, GL_COMPILE);

	//glBegin(GL_QUADS);

	//glColor3f(0., 0., 1.);
	//glNormal3f(0., 0., 1.);
	//glVertex3f(-dx, -dy, dz);
	//glVertex3f(dx, -dy, dz);
	//glVertex3f(dx, dy, dz);
	//glVertex3f(-dx, dy, dz);

	//glNormal3f(0., 0., -1.);
	//glTexCoord2f(0., 0.);
	//glVertex3f(-dx, -dy, -dz);
	//glTexCoord2f(0., 1.);
	//glVertex3f(-dx, dy, -dz);
	//glTexCoord2f(1., 1.);
	//glVertex3f(dx, dy, -dz);
	//glTexCoord2f(1., 0.);
	//glVertex3f(dx, -dy, -dz);

	//glColor3f(1., 0., 0.);
	//glNormal3f(1., 0., 0.);
	//glVertex3f(dx, -dy, dz);
	//glVertex3f(dx, -dy, -dz);
	//glVertex3f(dx, dy, -dz);
	//glVertex3f(dx, dy, dz);

	//glNormal3f(-1., 0., 0.);
	//glVertex3f(-dx, -dy, dz);
	//glVertex3f(-dx, dy, dz);
	//glVertex3f(-dx, dy, -dz);
	//glVertex3f(-dx, -dy, -dz);

	//glColor3f(0., 1., 0.);
	//glNormal3f(0., 1., 0.);
	//glVertex3f(-dx, dy, dz);
	//glVertex3f(dx, dy, dz);
	//glVertex3f(dx, dy, -dz);
	//glVertex3f(-dx, dy, -dz);

	//glNormal3f(0., -1., 0.);
	//glVertex3f(-dx, -dy, dz);
	//glVertex3f(-dx, -dy, -dz);
	//glVertex3f(dx, -dy, -dz);
	//glVertex3f(dx, -dy, dz);

	//glEnd();

	//glEndList();


	// create the axes:

	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	Axes(1.5);
	glLineWidth(1.);
	glEndList();
}


// the keyboard callback:

void
Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
	case 'o':
	case 'O':
		WhichProjection = ORTHO;
		break;

	//case 'p':
	//case 'P':
	//	WhichProjection = PERSP;
	//	break;

	case 'q':
	case 'Q':
	case ESCAPE:
		Buttons(QUIT);	// will not return here
		break;			// happy compiler

	case 'r':
	case 'R':
		LeftButton = ROTATE;
		break;

	case 's':
	case 'S':
		LeftButton = SCALE;
		break;

	case 'p':
		Paused = !Paused;
		if (Paused)
			GLUI_Master.set_glutIdleFunc(NULL);
		else
			GLUI_Master.set_glutIdleFunc(Animate);
		break;

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}


	// synchronize the GLUI display with the variables:

	Glui->sync_live();


	// force a call to Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// called when the mouse button transitions down or up:

void
MouseButton(int button, int state, int x, int y)
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);


	// get the proper button bit mask:

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		b = LEFT;		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;		break;

	default:
		b = 0;
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}


	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}
}


// called when the mouse moves while a button is down:

void
MouseMotion(int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "MouseMotion: %d, %d\n", x, y);


	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		switch (LeftButton)
		{
		case ROTATE:
			Xrot += (ANGFACT*dy);
			Yrot += (ANGFACT*dx);
			break;

		case SCALE:
			Scale += SCLFACT * (float)(dx - dy);
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;
		}
	}


	if ((ActiveButton & MIDDLE) != 0)
	{
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset()
{
	ActiveButton = 0;
	AxesOn = GLUITRUE;
	DebugOn = GLUIFALSE;
	DepthCueOn = GLUIFALSE;
	LeftButton = ROTATE;
	Scale = 1.0;
	Scale2 = 0.0;		// because we add 1. to it in Display( )
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	TransXYZ[0] = TransXYZ[1] = TransXYZ[2] = 0.;

	RotMatrix[0][1] = RotMatrix[0][2] = RotMatrix[0][3] = 0.;
	RotMatrix[1][0] = RotMatrix[1][2] = RotMatrix[1][3] = 0.;
	RotMatrix[2][0] = RotMatrix[2][1] = RotMatrix[2][3] = 0.;
	RotMatrix[3][0] = RotMatrix[3][1] = RotMatrix[3][3] = 0.;
	RotMatrix[0][0] = RotMatrix[1][1] = RotMatrix[2][2] = RotMatrix[3][3] = 1.;
}


// called when user resizes the window:

void
Resize(int width, int height)
{
	if (DebugOn != 0)
		fprintf(stderr, "ReSize: %d, %d\n", width, height);

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// handle a change to the window's visibility:

void
Visibility(int state)
{
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}




///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////

// size of wings as fraction of length:

#define WINGS	0.10


// axes:

#define X	1
#define Y	2
#define Z	3


// x, y, z, axes:

static float axx[3] = { 1., 0., 0. };
static float ayy[3] = { 0., 1., 0. };
static float azz[3] = { 0., 0., 1. };


void
Arrow(float tail[3], float head[3])
{
	float u[3], v[3], w[3];		// arrow coordinate system

	// set w direction in u-v-w coordinate system:

	w[0] = head[0] - tail[0];
	w[1] = head[1] - tail[1];
	w[2] = head[2] - tail[2];


	// determine major direction:

	int axis = X;
	float mag = fabs(w[0]);
	if (fabs(w[1])  > mag)
	{
		axis = Y;
		mag = fabs(w[1]);
	}
	if (fabs(w[2])  > mag)
	{
		axis = Z;
		mag = fabs(w[2]);
	}


	// set size of wings and turn w into a Unit vector:

	float d = WINGS * Unit(w, w);


	// draw the shaft of the arrow:

	glBegin(GL_LINE_STRIP);
	glVertex3fv(tail);
	glVertex3fv(head);
	glEnd();

	// draw two sets of wings in the non-major directions:

	float x, y, z;

	if (axis != X)
	{
		Cross(w, axx, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}


	if (axis != Y)
	{
		Cross(w, ayy, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}



	if (axis != Z)
	{
		Cross(w, azz, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}
}



float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}



void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];

	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];

	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}



float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];

	if (dist > 0.0)
	{
		dist = sqrt(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}

	return dist;
}



// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {
	0.f, 1.f, 0.f, 1.f
};

static float xy[] = {
	-.5f, .5f, .5f, -.5f
};

static int xorder[] = {
	1, 2, -3, 4
};


static float yx[] = {
	0.f, 0.f, -.5f, .5f
};

static float yy[] = {
	0.f, .6f, 1.f, 1.f
};

static int yorder[] = {
	1, 2, 3, -2, 4
};


static float zx[] = {
	1.f, 0.f, 1.f, 0.f, .25f, .75f
};

static float zy[] = {
	.5f, .5f, -.5f, -.5f, 0.f, 0.f
};

static int zorder[] = {
	1, 2, 3, 4, -5, 6
};


// fraction of the length to use as height of the characters:

const float LENFRAC = 0.10f;


// fraction of length to use as start location of the characters:

const float BASEFRAC = 1.10f;


//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes(float length)
{
	glBegin(GL_LINE_STRIP);
	glVertex3f(length, 0., 0.);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., length, 0.);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., 0., length);
	glEnd();

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 4; i++)
	{
		int j = xorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(base + fact*xx[j], fact*xy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 5; i++)
	{
		int j = yorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(fact*yx[j], base + fact*yy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 6; i++)
	{
		int j = zorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(0.0, fact*zy[j], base + fact*zx[j]);
	}
	glEnd();

}

//FUNCTION OF BMP TO TEXTURE




/**
** read a BMP file into a Texture:
**/

unsigned char *
BmpToTexture(char *filename, int *width, int *height)
{

	int s, t, e;		// counters
	int numextra;		// # extra bytes each line in the file is padded with
	FILE *fp;
	unsigned char *texture;
	int nums, numt;
	unsigned char *tp;


	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open Bmp file '%s'\n", filename);
		return NULL;
	}

	FileHeader.bfType = ReadShort(fp);


	// if bfType is not 0x4d42, the file is not a bmp:

	if (FileHeader.bfType != 0x4d42)
	{
		fprintf(stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType);
		fclose(fp);
		return NULL;
	}


	FileHeader.bfSize = ReadInt(fp);
	FileHeader.bfReserved1 = ReadShort(fp);
	FileHeader.bfReserved2 = ReadShort(fp);
	FileHeader.bfOffBits = ReadInt(fp);


	InfoHeader.biSize = ReadInt(fp);
	InfoHeader.biWidth = ReadInt(fp);
	InfoHeader.biHeight = ReadInt(fp);

	nums = InfoHeader.biWidth;
	numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort(fp);
	InfoHeader.biBitCount = ReadShort(fp);
	InfoHeader.biCompression = ReadInt(fp);
	InfoHeader.biSizeImage = ReadInt(fp);
	InfoHeader.biXPelsPerMeter = ReadInt(fp);
	InfoHeader.biYPelsPerMeter = ReadInt(fp);
	InfoHeader.biClrUsed = ReadInt(fp);
	InfoHeader.biClrImportant = ReadInt(fp);


	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );


	texture = new unsigned char[3 * nums * numt];
	if (texture == NULL)
	{
		fprintf(stderr, "Cannot allocate the texture array!\b");
		return NULL;
	}


	// extra padding bytes:

	numextra = 4 * (((3 * InfoHeader.biWidth) + 3) / 4) - 3 * InfoHeader.biWidth;


	// we do not support compression:

	if (InfoHeader.biCompression != birgb)
	{
		fprintf(stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression);
		fclose(fp);
		return NULL;
	}



	rewind(fp);
	fseek(fp, 14 + 40, SEEK_SET);

	if (InfoHeader.biBitCount == 24)
	{
		for (t = 0, tp = texture; t < numt; t++)
		{
			for (s = 0; s < nums; s++, tp += 3)
			{
				*(tp + 2) = fgetc(fp);		// b
				*(tp + 1) = fgetc(fp);		// g
				*(tp + 0) = fgetc(fp);		// r
			}

			for (e = 0; e < numextra; e++)
			{
				fgetc(fp);
			}
		}
	}

	fclose(fp);

	*width = nums;
	*height = numt;
	return texture;
}



int
ReadInt(FILE *fp)
{
	unsigned char b3, b2, b1, b0;
	b0 = fgetc(fp);
	b1 = fgetc(fp);
	b2 = fgetc(fp);
	b3 = fgetc(fp);
	return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}


short
ReadShort(FILE *fp)
{
	unsigned char b1, b0;
	b0 = fgetc(fp);
	b1 = fgetc(fp);
	return (b1 << 8) | b0;
}

// draw circle
void DrawCircle()
{
	int i;
	
	glBegin(GL_POLYGON);
	glColor4f(255, 0, 255, .5);
	for (i = 0; i < n; ++i)
		glVertex3f(r * cos(2 * Pi / n * i) + R*cos(2 * Pi * Time), 40, r * sin(2 * Pi / n * i) + R*sin(2 * Pi * Time));
	glEnd();
	
}
// function to convert HSV to RGB
//
// Reference:  Foley, van Dam, Feiner, Hughes,
//		"Computer Graphics Principles and Practices,"

void
HsvRgb(float hsv[3], float rgb[3])
{
	float r, g, b;			// red, green, blue

	// guarantee valid input:

	float h = hsv[0] / 60.;
	while (h >= 6.)	h -= 6.;
	while (h <  0.) 	h += 6.;

	float s = hsv[1];
	if (s < 0.)
		s = 0.;
	if (s > 1.)
		s = 1.;

	float v = hsv[2];
	if (v < 0.)
		v = 0.;
	if (v > 1.)
		v = 1.;


	// if sat==0, then is a gray:

	if (s == 0.0)
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}



	// get an rgb from the hue itself:

	float i = floor(h);
	float f = h - i;
	float p = v * (1. - s);
	float q = v * (1. - s*f);
	float t = v * (1. - (s * (1. - f)));

	switch ((int)i)
	{
	case 0:
		r = v;	g = t;	b = p;
		break;

	case 1:
		r = q;	g = v;	b = p;
		break;

	case 2:
		r = p;	g = v;	b = t;
		break;

	case 3:
		r = p;	g = q;	b = v;
		break;

	case 4:
		r = t;	g = p;	b = v;
		break;

	case 5:
		r = v;	g = p;	b = q;
		break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}
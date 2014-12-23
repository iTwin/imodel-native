/****************************************************************************

Pointools Vortex API example application framework

(c) Copyright 2008-09 Pointools Ltd

This example uses GLUT and GLUI to set up a simple UI and demonstrate
the use of the Pointools Vortex API


CONFIDENTIAL 
--------------------------
This source code is provided as confidential material. License to use this
source code is provided for the purpose of evaluation only. 

This source code may not be passed onto any third party without prior consent 
from Pointools Ltd


LICENSE
--------------------------
License to use this source code is granted for the purpose of evaluation 
only provided that:

1. All marks and software identification including but not
   limited to logos, application title and text are not altered and no attempt
   is made to alter the apparant origin or author of the work.

2. The source code and complied binary application is treated as confidential

No transfer of ownership, intellectual property rights or software usage rights
whatsoever is implied by the provision of this source code.


SIMPLE APPLICATION FRAMEWORK FOR EXAMPLE APPLICATIONS

USE OF GLUI
--------------------------
GLUI is used under the LGPL license. The modified version of the source code 
of GLUI will be provided on request - contact support@pointools.com if you need
this.

****************************************************************************/
/* precompiled header */ 
#include "stdafx.h"

/* C headers */ 
#include <math.h>
#include <string.h>

/* GL */ 
#ifdef PT_BUILD_ENV
	#include <gl/glui.h>
#else
	#include "../include/glui.h"
#endif
#include <gl/glu.h>

/* C++ headers */ 
#include <vector>
#include <iostream>

/* resources */ 
#include "resource.h"

/* utility */ 
#include "../include/timer.h"

/* API header */ 
#include "../include/PointoolsVortexAPI_import.h"

/* license code -   Contact vortex@pointools.com if you do not have this*/ 
/*					or replace with one you have been provided			*/ 
#include "../lic/vortexLicense.c"

/* vortex initialization state */ 
bool	isVortexLoaded = false;

/* user interface */ 
#define PANEL_WIDTH 270
#define COPYRIGHT_NOTICE "   Copyright 2008-10. Pointools Ltd All Rights Reserved"

struct UI
{
	GLUI_Panel      *rolloutFile;
	GLUI_Panel		*rolloutShader;
	GLUI_Panel		*rolloutSelect;
	GLUI_Panel		*rolloutOptions;
	GLUI_Panel		*rolloutTests;
	GLUI_Panel		*rolloutTuning;

	GLUI_Rotation	*ballView;
	GLUI_Spinner    *light0_spinner;

	GLUI_Listbox	*rampList;
	GLUI_Listbox	*planeRampList;
	GLUI_Listbox	*planeAxis;

	GLUI			*panelSide;

	GLUI_RadioGroup *radio;

	GLUI_TextBox	*info;
	GLUI_TextBox	*output;
	GLUI_TextBox	*stats;
	GLUI_String		outputString;
	GLUI_String		infoString;
	GLUI_String		statsString;
	GLUI_String		loadString;

	GLUI_TextBox *mbSec;

	GLuint			logo;

	GLUI_Button *lyrButtons[ PT_EDIT_MAX_LAYERS ];
	GLUI_Button *lyrVis[ PT_EDIT_MAX_LAYERS ];
	GLUI_Button *lyrLock[ PT_EDIT_MAX_LAYERS ];

	int mainWindow;
	float xy_aspect;

	UI()
	{
		ballView = 0;
		panelSide = 0;
		mainWindow = 0;
	}
	void addStat(const std::string &stat)
	{
		if (!stats) return;
		statsString += "\n" + stat;
		stats->set_text( statsString.c_str() );
		stats->redraw();
	}
	void addOutput(const std::string &txt)
	{
		if (!output) return;
		outputString += "\n" + txt;
		output->set_text( outputString.c_str() );
		output->redraw();
	}
	void addTimeStat( pt::PerformanceTimer &t, const char* desc )
	{
		static char ms[128];
		sprintf_s( ms, "%s took %0.1fms", desc, t.millisecs() );
		addStat(ms);
	}

	const wchar_t *getSaveFilePath( const wchar_t * desc, const wchar_t *ext )
	{
		OPENFILENAME ofn;
		static wchar_t szFileName[MAX_PATH] = L"";

		ZeroMemory(&ofn, sizeof(ofn));

		wchar_t buffer[300];
		swprintf_s( buffer, L"%s (*.%s)\0*.%s\0\0", desc, ext, ext );

		ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = buffer;
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
		ofn.lpstrDefExt = ext;

		if(GetSaveFileName(&ofn))
		{
			return szFileName;
		}
		return 0;
	}

	const wchar_t *getLoadFilePath( const wchar_t * desc, const wchar_t *ext )
	{
		OPENFILENAME ofn;
		static wchar_t szFileName[MAX_PATH] = L"";

		ZeroMemory(&ofn, sizeof(ofn));

		wchar_t buffer[300];
		swprintf_s( buffer, L"%s (*.%s)\0*.%s\0\0", desc, ext, ext );

		ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = buffer;
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = ext;

		if(GetOpenFileName(&ofn))
		{
			return szFileName;
		}
		return 0;
	}
};
UI ui;


/* basic encapsulation of Vortex shader options */ 
struct VortexShaderOptions
{
	int	useRGB;
	int	useIntensity;
	int	useLighting;
	int useClassification;
	int usePlane;

	float	contrast;
	float	brightness;
	float	fps;

	float dist;
	float off;

	/* generic use */ 
	float mixer;

	int adaptivePointSize;
	int frontBias;

	std::vector< std::string > intensityRamps;
	std::vector< std::string > planeRamps;

	int intensityRamp;
	int planeRamp;
	int planeAxis;

	int selectMode;

	VortexShaderOptions()
	{
		useRGB = 1;
		useIntensity = 0;
		usePlane = 0;
		useLighting = 0;

		mixer = 0;
		selectMode = 0;

		contrast = 50.0f;
		brightness = 180.0f;

		adaptivePointSize = 1;
		frontBias = 0;

		intensityRamp = 2;
		planeRamp = 3;
		planeAxis = 2;

		fps = 15.0f;
		off = 0;
		dist = 10.0f;
	}
};
VortexShaderOptions shader;

/* Mouse operation */ 
enum MouseMode
{
	MouseNone = 0, 
	MouseOrbit = 1,
	MousePan = 2,
	MouseZoom = 3,
	MouseRect = 4,
	MousePolygon = 5,
	MouseBrush = 6,
	MouseMeasurePnt = 7,
	MouseMeasureDist = 8,
	MouseBiasPnt = 9,
	MouseRotate = 10,
};

/* mouse state */ 
struct  Mouse
{
	int eventStartX;
	int eventStartY;
	int lastX;
	int lastY;

	MouseMode mode;

	std::vector<int> polygon;

	Mouse()
	{
		mode = MouseNone;
		lastX = -1;
	}
};
Mouse mouse;

/* Viewport state */ 
struct View
{
	float rotateX;
	float rotateY;
	float rotateZ;

	float rotateXtmp;
	float rotateYtmp;
	float rotateZtmp;

	float posX;
	float posY;
	float posZ;

	float posXtmp;
	float posYtmp;
	float posZtmp;

	float centerX;
	float centerY;
	float centerZ;

	int iteration;

	bool dynamic;

	pt::PerformanceTimer timeSinceKbUpdate;

	PThandle displayQueryHandle;

	/* buffers for our own drawing code */ 
	PTfloat *pointsBuffer;
	PTubyte *rgbBuffer;
	PTshort *intenBuffer;
	PTubyte *classificationBuffer;

	PTuint	 validMeasurePnts;
	PTdouble measurePnt[6];
	PTdouble biasPnt[3];
	PTfloat  boxLower[3];
	PTfloat	 boxUpper[3];
	PTbool   boxValid;

	float	 boxColor[3];
	float	 boxPosition[3];
	float	 boxRotation[3];

	int		cacheSize;
	int		loadBias;
	int		ownDisplay;
	int		renderLimit;
	int		progress;
	bool	clearFrame;
	bool	freezeIdle;
	bool	freezeDraw;
	bool	isViewComplete;


	int		queryMax;

	PTdouble analysisPnt[3];

	View()
	{
		validMeasurePnts = 0;

		boxValid = false;

		rotateX = 0;
		rotateY = 0;
		rotateZ = 0;

		rotateXtmp = 0;
		rotateYtmp = 0;
		rotateZtmp = 0;

		posX = 0;
		posY = 0;
		posZ = -20;

		posXtmp = 0;
		posYtmp = 0;
		posZtmp = 0;

		centerX = 0;
		centerY = 0;
		centerZ = 0;

		boxColor[0] = 1;
		boxColor[1] = 1;
		boxColor[2] = 1;

		boxPosition[0] = 0;
		boxPosition[1] = 0;
		boxPosition[2] = 0;

		boxRotation[0] = 0;
		boxRotation[1] = 0;
		boxRotation[2] = 0;

		dynamic = false;

		displayQueryHandle = 0;
		pointsBuffer = 0;
		rgbBuffer = 0;
		classificationBuffer = 0;

		memset(measurePnt, 0, sizeof(measurePnt));
		memset(biasPnt, 0, sizeof(measurePnt));

		ownDisplay = 0;
		clearFrame = true;
		freezeIdle = false;
		freezeDraw = false;

		queryMax = -1;
		cacheSize = -1;
		loadBias = PT_LOADING_BIAS_SCREEN;

		isViewComplete = false;

		iteration =0 ;
		progress = 0;
	}
	void releaseQueryBuffers()
	{
		if (pointsBuffer) delete [] pointsBuffer;
		if (rgbBuffer) delete [] rgbBuffer;
		if (classificationBuffer) delete [] classificationBuffer;
		
		pointsBuffer = 0;
		rgbBuffer = 0;
		classificationBuffer = 0;
	}
};
View view;

/* Lighting constants - not used at this time */ 
GLfloat light0_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
GLfloat light0_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
GLfloat light0_position[] = {.5f, .5f, 1.0f, 0.0f};

GLfloat light1_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
GLfloat light1_diffuse[] =  {.9f, .6f, 0.0f, 1.0f};
GLfloat light1_position[] = {-1.0f, -1.0f, 1.0f, 0.0f};

GLfloat lights_rotation[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

/****************************************/
/*   draw logo in viewport				*/
/****************************************/
void drawLogo()
{
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );			

	glEnable( GL_BLEND );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D( tx, tx+tw, ty, ty+th);

	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, ui.logo );

	glDisable( GL_LIGHTING );

	glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glEnable( GL_COLOR_MATERIAL );
	glColor3f(1.0f,1.0f,1.0f);

	glBegin( GL_QUADS );

		glTexCoord2f(0,0);
		glVertex2i(10,10);

		glTexCoord2f(1,0);
		glVertex2i(266, 10);

		glTexCoord2f(1,0.15625);
		glVertex2i(266, 50);

		glTexCoord2f(0,0.15625);
		glVertex2i(10, 50);

	glEnd();

	glDisable( GL_TEXTURE_2D );

	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
	glDisable( GL_BLEND );
}
/****************************************/
/*   draw rectangle selector			*/
/****************************************/
void drawPntMarker( PTdouble *pnt, PTdouble size )
{
	size *= 0.5;
	glBegin(GL_LINES);
		glVertex3d(	pnt[0]-size, pnt[1], pnt[2] );
		glVertex3d(	pnt[0]+size, pnt[1], pnt[2] );

		glVertex3d(	pnt[0], pnt[1]-size, pnt[2] );
		glVertex3d(	pnt[0], pnt[1]+size, pnt[2] );

		glVertex3d(	pnt[0], pnt[1], pnt[2]-size );
		glVertex3d(	pnt[0], pnt[1], pnt[2]+size );
	glEnd();		

}

/****************************************/
/*   resize handler 					*/
/****************************************/
void glutResizeHandler( int x, int y )
{
  int tx, ty, tw, th;
  GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
  glViewport( tx, ty, tw, th );

  ui.xy_aspect = (float)tw / (float)th;

  glutPostRedisplay();
}

/****************************************/
/*   draw simple axis					*/
/****************************************/
void drawAxes( float scale )
{
  glDisable( GL_LIGHTING );
  glDisable ( GL_DEPTH_TEST );
  glPushMatrix();
  glScalef( scale, scale, scale );

  glBegin( GL_LINES );
 
  glColor3f( 1.0, 0.0, 0.0 );
  glVertex3f( .8f, 0.05f, 0.0 );  glVertex3f( 1.0, 0.25f, 0.0 ); /* Letter X */
  glVertex3f( 0.8f, .25f, 0.0 );  glVertex3f( 1.0, 0.05f, 0.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 1.0, 0.0, 0.0 ); /* X axis      */

  glColor3f( 0.0, 1.0, 0.0 );
  glVertex3f(0.05, 1.00, 0.0);
  glVertex3f(0.15, 0.85, 0.0);
  glVertex3f(0.25, 1.00, 0.0);
  glVertex3f(0.15, 0.85, 0.0);
  glVertex3f(0.15, 0.85, 0.0);
  glVertex3f(0.15, 0.65, 0.0);

  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 1.0, 0.0 ); /* Y axis      */

  glColor3f( 0.0, 0.0, 1.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 0.0, 1.0 ); /* Z axis    */
  glEnd();

  glPopMatrix();

  glEnable ( GL_DEPTH_TEST );
  glEnable( GL_LIGHTING );
}
/****************************************/
/*   Simple wc to ascii (do not reuse!)	*/
/****************************************/
char *wcToAscii( const wchar_t *wstr )
{
	static char asc[128];
	memset( asc, 0, 128 );
	int ln = (int)wcslen(wstr);	
	for (int i=0;i<ln;i++) asc[i] = (char)wstr[i];

	return asc;
}

/****************************************/
/*   Initialize Vortex API				*/
/****************************************/
bool initializeVortexAPI()
{
	extern bool LoadPointoolsDLL(const TCHAR*filepath);

	wchar_t folder[260];
	wchar_t apiFile[260];
	GetModuleFileNameW(NULL, folder, 260);
	::PathRemoveFileSpecW(folder);

#ifdef WIN64
	#ifdef PT_DEBUG	//this is for internal Pointools use only
		#ifdef _DEBUG
			swprintf_s(apiFile, L"%s//PointoolsVortexAPI64d.dll", folder);
		#else
			swprintf_s(apiFile, L"%s//PointoolsVortexAPI64pd.dll", folder);
		#endif
	#else
		swprintf_s(apiFile, L"%s//PointoolsVortexAPI64.dll", folder);
	#endif
#else
	#ifdef PT_DEBUG	//this is for internal Pointools use only
		#ifdef _DEBUG
			swprintf_s(apiFile, L"%s//PointoolsVortexAPId.dll", folder);
		#else
			swprintf_s(apiFile, L"%s//PointoolsVortexAPIpd.dll", folder);
		#endif
	#else
		swprintf_s(apiFile, L"%s//PointoolsVortexAPI.dll", folder);
	#endif
#endif
	if (LoadPointoolsDLL( apiFile ))
	{
		PTubyte version[4];
		ptGetVersionNum(version);

		std::cout << "Pointools Vortex " << (int)version[0] << "." << (int)version[1] << std::endl;
		if (ptInitialize( vortexLicCode ) == PT_FALSE)
		{
			std::cout << "Failed to initialize Vortex API" << std::endl;
			std::cout << wcToAscii(ptGetLastErrorString()) << std::endl;
		}
	
		ptSetWorkingFolder( folder );
		ptSetAutoBaseMethod( PT_AUTO_BASE_CENTER );
		ptFlipMouseYCoords();
		
		/* single viewport, no multiple viewport handling in this example */ 
		ptAddViewport( 0, L"viewport", PT_GL_VIEWPORT );

		PTubyte selCol [] = { 196, 128, 0 };
		ptSetSelectionDrawColor( selCol );
		isVortexLoaded = true;
	}
	else
	{
		std::cout << "Error loading PointoolsVortexAPI.dll" << std::endl;
		extern const char*DLLLoadErrorMessage();
		const char*err = DLLLoadErrorMessage();
		if (err)
			std::cout << err << std::endl;
	}
	return isVortexLoaded;
}
/****************************************/
/* Load logo for UI						*/
/****************************************/
void loadLogo()
{
	glGenTextures(1, &ui.logo );
	glBindTexture( GL_TEXTURE_2D, ui.logo );

	HBITMAP hBmp = NULL;

	hBmp = (HBITMAP) ::LoadImage( GetModuleHandle(0), 
		MAKEINTRESOURCE(IDB_LOGO), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	BITMAP BM;
	::GetObject(hBmp, sizeof(BM), &BM);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, 256, 256,  GL_BGR_EXT, GL_UNSIGNED_BYTE, BM.bmBits);

	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	DeleteObject((HGDIOBJ) hBmp);
	DeleteObject(&BM);	
}
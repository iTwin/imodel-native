/****************************************************************************

Pointools Vortex API example application framework

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

****************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Commdlg.h>
#include <shlwapi.h>

/* C headers */ 
#include <math.h>
#include <string.h>

/* GL */ 
#include <gl/glu.h>

/* C++ headers */ 
#include <vector>
#include <iostream>

/* resources */ 
#include "../res/resource.h"

/* app framework */
#include "../include/VortexExampleApp.h"

/* utility */ 
#include "../include/timer.h"

/* API headers */ 
#include <PointoolsVortexAPI_DLL/PTAPI/PointoolsVortexAPI_import.h>
#include <PointoolsVortexAPI_FeatureExtract_DLL/VortexFeatureExtract.h>

/* license code -   Contact vortex@pointools.com if you do not have this*/ 
/*					or replace with one you have been provided			*/ 
//#include "../lic/vortexLicense.c"

/* vortex initialization state */ 
bool	isVortexLoaded = false;

// UI Constants
#define COPYRIGHT_NOTICE	"   Copyright 2008-11. Pointools Ltd All Rights Reserved"

//-----------------------------------------------------------------------------
// UI Class Implementation
//-----------------------------------------------------------------------------
UI::UI()
//-----------------------------------------------------------------------------
{
	ballView = 0;
	panelSide = 0;
	mainWindow = 0;
	output=0;
}
//-----------------------------------------------------------------------------
void UI::addStat(const std::string &stat)
//-----------------------------------------------------------------------------
{
	if (!stats) return;
	statsString += "\n" + stat;
	stats->set_text( statsString.c_str() );
	stats->redraw();
}
//-----------------------------------------------------------------------------
void UI::addOutput(const std::string &txt)
//-----------------------------------------------------------------------------
{
	if (!output) return;
	outputString += "\n" + txt;
	output->set_text( outputString.c_str() );
	output->redraw();
}
//-----------------------------------------------------------------------------
void UI::addTimeStat( pt::PerformanceTimer &t, const char* desc )
//-----------------------------------------------------------------------------
{
	static char ms[128];
	sprintf_s( ms, "%s took %0.1fms", desc, t.millisecs() );
	addStat(ms);
}
//-----------------------------------------------------------------------------
const wchar_t *UI::getSaveFilePath( const wchar_t * desc, const wchar_t *ext )
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
const wchar_t *UI::getLoadFilePath( const wchar_t * desc, const wchar_t *ext )
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
bool	UI::getColor( COLORREF initial, COLORREF &chosen )
//-----------------------------------------------------------------------------
{
	CHOOSECOLOR cc;
	memset(&cc, 0, sizeof(cc));
	cc.lStructSize = sizeof(cc);

	COLORREF col_array[16];

	for (int i=0;i<16; i++)	col_array[ i ] = RGB(255,255,255);

	cc.lpCustColors = col_array;

	if (initial)
	{
		cc.Flags = CC_RGBINIT;
		cc.rgbResult = initial;
	}

	if (::ChooseColor( &cc ))
	{
		chosen = cc.rgbResult;
		return true;
	}
	return false;
}
//-----------------------------------------------------------------------------
Mouse::Mouse()
//-----------------------------------------------------------------------------
{
	eventStartX = -1;
	eventStartY = -1;
	lastX = -1;
	lastY = -1;
}
//-----------------------------------------------------------------------------
View::View()
//-----------------------------------------------------------------------------
{
	dynamic = false;
	enableDynamic = true; // when not enabled dynamic rendering is never used

	displayQueryHandle = 0;

	ownDisplay = 0;
	clearFrame = true;
	freezeIdle = false;
	freezeDraw = false;

	queryMax = -1;
	cacheSize = -1;
	loadBias = PT_LOADING_BIAS_SCREEN;

	isViewComplete = false;

	showLayerBounds = false;

	iteration =0 ;
	progress = 0;
}
//-----------------------------------------------------------------------------
// Utility
//-----------------------------------------------------------------------------
char *wcToAscii( const wchar_t *wstr )
{
	static char asc[128];
	memset( asc, 0, 128 );
	int ln = (int)wcslen(wstr);	
	for (int i=0;i<ln;i++) asc[i] = (char)wstr[i];

	return asc;
}
//-----------------------------------------------------------------------------
bool VortexExampleApp::setupGL(int argc, char* argv[])
//-----------------------------------------------------------------------------
{
	// glut
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowPosition( 50, 50 );
	glutInitWindowSize( 800, 600 );
	 
	m_ui.mainWindow = glutCreateWindow( "Pointools Vortex Example" );
	glutDisplayFunc( glutDisplay_stub );

	// gl options
	glEnable(GL_DEPTH_TEST);

	// setup lighting
	GLfloat light0_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
	GLfloat light0_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
	GLfloat light0_position[] = {.5f, .5f, 1.0f, 0.0f};

	GLfloat light1_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
	GLfloat light1_diffuse[] =  {.9f, .6f, 0.0f, 1.0f};
	GLfloat light1_position[] = {-1.0f, -1.0f, 1.0f, 0.0f};

	GLfloat lights_rotation[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

	glEnable(GL_LIGHTING);

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	return true;
}
//-----------------------------------------------------------------------------
namespace 
{
	VortexExampleApp* _app_instance_=0;
}
//-----------------------------------------------------------------------------
VortexExampleApp* VortexExampleApp::instance()
//-----------------------------------------------------------------------------
{
	return _app_instance_;
}
//-----------------------------------------------------------------------------
bool VortexExampleApp::initializeApplication(int argc, char* argv[], bool debugVortex)
//-----------------------------------------------------------------------------
{
	if (!initializeVortexAPI(debugVortex)) return false;		// load Vortex DLL and extract functions
	
	_app_instance_ = this;

	pt::PerformanceTimer::initialize();				// initialize utility timer

	if (!setupGL(argc,argv))	return false;

	m_ui.loadResources();								// load the Logo resource

	return true;
}
//-----------------------------------------------------------------------------
bool VortexExampleApp::runApplication()
//-----------------------------------------------------------------------------
{
	buildUserInterface();							// build the user interface, usually overridden

	GLUI_Master.set_glutIdleFunc( VortexExampleApp::glutIdleHandler_stub );
	glutMainLoop();

	ptRelease();

	return true;
}
//-----------------------------------------------------------------------------
bool VortexExampleApp::initializeVortexAPI(bool debugVortexDll)
//-----------------------------------------------------------------------------
{
	extern bool LoadPointoolsDLL(const TCHAR*filepath);

	wchar_t folder[260];
	wchar_t apiFile[260];

	GetModuleFileNameW(NULL, folder, 260);
	::PathRemoveFileSpecW(folder);

#ifdef WIN64
	if (debugVortexDll)
	{
		swprintf_s(apiFile, L"%s//PointoolsVortexAPI.dll", folder);
	}
	else
	{
		swprintf_s(apiFile, L"%s//PointoolsVortexAPI.dll", folder);
	}
#else
	if (debugVortexDll)
	{
		swprintf_s(apiFile, L"%s//PointoolsVortexAPI.dll", folder);
	}
	else
	{
		swprintf_s(apiFile, L"%s//PointoolsVortexAPI.dll", folder);
	}
#endif
	if (LoadPointoolsDLL( apiFile ))
	{
		PTubyte version[4];
		ptGetVersionNum(version);

		std::cout << "Pointools Vortex " << (int)version[0] << "." << (int)version[1] << std::endl;
		if (ptInitialize( NULL /*vortexLicCode*/ ) == PT_FALSE)
		{
			std::cout << "Failed to initialize Vortex API" << std::endl;
			std::cout << wcToAscii(ptGetLastErrorString()) << std::endl;
			return false;
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
//-----------------------------------------------------------------------------
Tool::Tool(int cmdStart, int cmdEnd) : cmdRangeStart(cmdStart), cmdRangeEnd(cmdEnd) 
//-----------------------------------------------------------------------------
{
}
//-----------------------------------------------------------------------------
void Tool::viewUpdate()
//-----------------------------------------------------------------------------
{
	VortexExampleApp::instance()->dirty();
}
//-----------------------------------------------------------------------------
void Tool::viewRedraw()
//-----------------------------------------------------------------------------
{
	View &view = VortexExampleApp::instance()->getView();
	
	view.clearFrame=true;
	VortexExampleApp::instance()->dirty();
}
//-----------------------------------------------------------------------------
void Tool::addStatisticMessage(const char *mess)
//-----------------------------------------------------------------------------
{
	//todo: add to UI
	std::cout << mess << std::endl;
}
//-----------------------------------------------------------------------------
void Tool::dispatchCmd( int cmdId )
//-----------------------------------------------------------------------------
{
	VortexExampleApp::instance()->dispatchUICmd_stub( cmdId );
}
//-----------------------------------------------------------------------------
void	Tool::startDynamicView()
//-----------------------------------------------------------------------------
{
	if (VortexExampleApp::instance()->getView().enableDynamic)
		VortexExampleApp::instance()->getView().dynamic = true;
}
//-----------------------------------------------------------------------------
void	Tool::endDynamicView()
//-----------------------------------------------------------------------------
{	
	VortexExampleApp::instance()->getView().dynamic = false;	
}
//-----------------------------------------------------------------------------
Tool::~Tool()
//-----------------------------------------------------------------------------
{
}
//-----------------------------------------------------------------------------
void VortexExampleApp::dispatchUICmd_stub( int cmdId )
//-----------------------------------------------------------------------------
{
	instance()->dispatchUICmd( cmdId );
}
//-----------------------------------------------------------------------------
void VortexExampleApp::dispatchUICmd( int cmdId )
//-----------------------------------------------------------------------------
{
	for (Tools::iterator i = m_tools.begin(); i!=m_tools.end(); i++)
	{
		if ((*i)->ownsCmd( cmdId ))
		{
			(*i)->command( cmdId );
		//	break; // commented out to allow other Tools to peek at called commands
		}
	}
}

//-----------------------------------------------------------------------------
void VortexExampleApp::addTool( Tool *tool )
//-----------------------------------------------------------------------------
{
	m_tools.push_back( tool );
}
//-----------------------------------------------------------------------------
View &VortexExampleApp::getView()
//-----------------------------------------------------------------------------
{
	return m_view;
}
//-----------------------------------------------------------------------------
UI &VortexExampleApp::getUI()
//-----------------------------------------------------------------------------
{
	return m_ui;
}
//-----------------------------------------------------------------------------
void	VortexExampleApp::buildUserInterface()
//-----------------------------------------------------------------------------
{
	GLUI_Master.set_glutReshapeFunc( glutResizeHandler_stub );  
	GLUI_Master.set_glutKeyboardFunc( glutKeyboardHandler_stub );
	GLUI_Master.set_glutSpecialFunc( NULL );
	GLUI_Master.set_glutMouseFunc( glutMouseButtonHandler_stub );
	glutMotionFunc( glutMouseDragHandler_stub );
	glutPassiveMotionFunc( glutMouseMoveHandler_stub );

	// Create the side panel
	m_ui.panelSide = GLUI_Master.create_glui_subwindow( m_ui.mainWindow, GLUI_SUBWINDOW_RIGHT );

	// Title
	// the following notices must appear in the UI as a condition of use 
	std::string versionStr( wcToAscii(ptGetVersionString()) );
	versionStr +=  " - ";
	versionStr += m_title;
	
	GLUI_StaticText *txt = new GLUI_StaticText( m_ui.panelSide, versionStr.c_str() );
//	txt->set_col( RGBc(255,255,255) );
	txt = new GLUI_StaticText( m_ui.panelSide, "   CONFIDENTIAL" );
//	txt->set_col( RGBc(128,128,128) );
	txt = new GLUI_StaticText( m_ui.panelSide, COPYRIGHT_NOTICE );
//	txt->set_col( RGBc(128,128,128) );
	new GLUI_StaticText( m_ui.panelSide, " " );

	// Add Tool UI
	Tools::iterator t_i = m_tools.begin();

	while (t_i != m_tools.end())
	{
		(*t_i)->buildUserInterface( m_ui.panelSide );
		++t_i;
	}
}

//-----------------------------------------------------------------------------
bool UI::loadResources()
//-----------------------------------------------------------------------------
{
	glGenTextures(1, &logo );
	glBindTexture( GL_TEXTURE_2D, logo );

	HBITMAP hBmp = NULL;

	hBmp = (HBITMAP) ::LoadImage( GetModuleHandle(0), 
		MAKEINTRESOURCE(IDB_LOGO), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	if (!hBmp) return false;

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

	return true;
}
//-----------------------------------------------------------------------------
void VortexExampleApp::drawBackground()
//-----------------------------------------------------------------------------
{
	if (m_renderer && (m_view.clearFrame || !m_view.ownDisplay ))
	{
		m_renderer->drawBackground();
	}
}
//-----------------------------------------------------------------------------
void VortexExampleApp::drawPointClouds()
//-----------------------------------------------------------------------------
{
	if (m_renderer)
	{
		m_renderer->drawPointClouds( m_view.dynamic, m_view.clearFrame );
	}
}
//-----------------------------------------------------------------------------
void VortexExampleApp::drawBoundingBox()
//-----------------------------------------------------------------------------
{
	glColor3f(0.7f,0.7f,0.7f);
	m_view.drawBox(m_bbLower, m_bbUpper, 0, 0);
}
//-----------------------------------------------------------------------------
void VortexExampleApp::drawLogo()
//-----------------------------------------------------------------------------
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
	glBindTexture( GL_TEXTURE_2D, m_ui.logo );

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

		glTexCoord2f(1,0.2578125);
		glVertex2i(266, 76);

		glTexCoord2f(0,0.2578125);
		glVertex2i(10, 76);

	glEnd();

	glDisable( GL_TEXTURE_2D );

	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
	glDisable( GL_BLEND );
}
//-----------------------------------------------------------------------------
void VortexExampleApp::drawAxes( float scale )
//-----------------------------------------------------------------------------
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
  glVertex3f(0.05f, 1.00f, 0.0f);
  glVertex3f(0.15f, 0.85f, 0.0f);
  glVertex3f(0.25f, 1.00f, 0.0f);
  glVertex3f(0.15f, 0.85f, 0.0f);
  glVertex3f(0.15f, 0.85f, 0.0f);
  glVertex3f(0.15f, 0.65f, 0.0f);

  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 1.0, 0.0 ); /* Y axis      */

  glColor3f( 0.0, 0.0, 1.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 0.0, 1.0 ); /* Z axis    */
  glEnd();

  glPopMatrix();

  glEnable ( GL_DEPTH_TEST );
  glEnable( GL_LIGHTING );
}
//-----------------------------------------------------------------------------
void View::drawPntMarker( PTdouble *pnt, PTdouble size )
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
void View::drawBox( double *lower, double *upper, double *position, double *rotation)
//-----------------------------------------------------------------------------
{
	glLineWidth( 1.0f );

	glMatrixMode(GL_MODELVIEW);

	if(position != NULL && rotation != NULL)
	{
		glPushMatrix();
		glTranslatef((float) position[0], (float) position[1], (float) position[2]);
		glRotatef((float) rotation[1], 0, 1, 0);
		glRotatef((float) rotation[0], 1, 0, 0);
		glTranslatef((float) -position[0], (float) -position[1], (float) -position[2]);
	}

	glBegin( GL_LINE_STRIP );
		glVertex3f( (float) lower[0], (float) lower[1], (float) lower[2] );
		glVertex3f( (float) upper[0], (float) lower[1], (float) lower[2] );
		glVertex3f( (float) upper[0], (float) upper[1], (float) lower[2] );
		glVertex3f( (float) lower[0], (float) upper[1], (float) lower[2] );
		glVertex3f( (float) lower[0], (float) lower[1], (float) lower[2] );
	glEnd();

	glBegin( GL_LINE_STRIP );
		glVertex3f( (float) lower[0], (float) lower[1], (float) upper[2] );
		glVertex3f( (float) upper[0], (float) lower[1], (float) upper[2] );
		glVertex3f( (float) upper[0], (float) upper[1], (float) upper[2] );
		glVertex3f( (float) lower[0], (float) upper[1], (float) upper[2] );
		glVertex3f( (float) lower[0], (float) lower[1], (float) upper[2] );
	glEnd();

	glBegin( GL_LINES );
		glVertex3f( (float) lower[0], (float) lower[1], (float) lower[2] );
		glVertex3f( (float) lower[0], (float) lower[1], (float) upper[2] );

		glVertex3f( (float) upper[0], (float) lower[1], (float) lower[2] );
		glVertex3f( (float) upper[0], (float) lower[1], (float) upper[2] );

		glVertex3f( (float) upper[0], (float) upper[1], (float) lower[2] );
		glVertex3f( (float) upper[0], (float) upper[1], (float) upper[2] );

		glVertex3f( (float) lower[0], (float) upper[1], (float) lower[2] );
		glVertex3f( (float) lower[0], (float) upper[1], (float) upper[2] );
	glEnd();

	glPopMatrix();
}

//-----------------------------------------------------------------------------
void View::drawBox( float *lower, float *upper, float *position, float *rotation)
//-----------------------------------------------------------------------------
{
	glLineWidth( 1.0f );

	glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

	if(position != NULL && rotation != NULL)
	{
		
		glTranslatef(position[0], position[1], position[2]);
		glRotatef(rotation[1], 0, 1, 0);
		glRotatef(rotation[0], 1, 0, 0);
		glTranslatef(-position[0], -position[1], -position[2]);
	}

	glBegin( GL_LINE_STRIP );
	glVertex3f( lower[0], lower[1], lower[2] );
	glVertex3f( upper[0], lower[1], lower[2] );
	glVertex3f( upper[0], upper[1], lower[2] );
	glVertex3f( lower[0], upper[1], lower[2] );
	glVertex3f( lower[0], lower[1], lower[2] );
	glEnd();

	glBegin( GL_LINE_STRIP );
	glVertex3f( lower[0], lower[1], upper[2] );
	glVertex3f( upper[0], lower[1], upper[2] );
	glVertex3f( upper[0], upper[1], upper[2] );
	glVertex3f( lower[0], upper[1], upper[2] );
	glVertex3f( lower[0], lower[1], upper[2] );
	glEnd();

	glBegin( GL_LINES );
	glVertex3f( lower[0], lower[1], lower[2] );
	glVertex3f( lower[0], lower[1], upper[2] );

	glVertex3f( upper[0], lower[1], lower[2] );
	glVertex3f( upper[0], lower[1], upper[2] );

	glVertex3f( upper[0], upper[1], lower[2] );
	glVertex3f( upper[0], upper[1], upper[2] );

	glVertex3f( lower[0], upper[1], lower[2] );
	glVertex3f( lower[0], upper[1], upper[2] );
	glEnd();

	glPopMatrix();
}
//-----------------------------------------------------------------------------
void	VortexExampleApp::updateBoundingBox()
//-----------------------------------------------------------------------------
{
	// get lastest BB, change last param to true for faster approx calc
	ptLayerBounds(0, m_bbLower, m_bbUpper, false );
}

//-----------------------------------------------------------------------------
void VortexExampleApp::glutKeyboardHandler_stub(unsigned char Key, int x, int y)
//-----------------------------------------------------------------------------
{
	instance()->glutKeyboardHandler(Key, x, y);
}
//-----------------------------------------------------------------------------
void VortexExampleApp::glutKeyboardHandler(unsigned char Key, int x, int y)
//-----------------------------------------------------------------------------
{
	switch(Key)
	{
	case 27: 

	case 'q':
		exit(0);
		break;
	}
}

//-----------------------------------------------------------------------------
void VortexExampleApp::glutResizeHandler_stub( int x, int y )
//-----------------------------------------------------------------------------
{
	instance()->glutResizeHandler(x,y);
}
//-----------------------------------------------------------------------------
void VortexExampleApp::glutResizeHandler( int x, int y )
//-----------------------------------------------------------------------------
{
  int tx, ty, tw, th;
  GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
  glViewport( tx, ty, tw, th );

  m_ui.xy_aspect = (float)tw / (float)th;

  m_dirty=true;
}
//-----------------------------------------------------------------------------
void VortexExampleApp::glutMouseButtonHandler_stub(int button, int button_state, int x, int y )
//-----------------------------------------------------------------------------
{
	instance()->glutMouseButtonHandler(button, button_state, x, y);
}
//-----------------------------------------------------------------------------
void VortexExampleApp::glutMouseButtonHandler(int button, int button_state, int x, int y )
//-----------------------------------------------------------------------------
{
	m_mouse.eventStartX = x;
	m_mouse.eventStartY = y;

	// MOUSE DOWN
	if (!button_state)
	{
		for (Tools::iterator i = m_tools.begin(); i!=m_tools.end(); i++)
		{
			if ((*i)->onMouseButtonDown(button, x, y))
			{
				break;
			}
		}
	}
	// MOUSE UP
	else
	{
		bool handled = false;
		for (Tools::iterator i = m_tools.begin(); i!=m_tools.end(); i++)
		{
			if ((*i)->onMouseButtonUp(button, x, y))
			{
				break;
			}
		}
	}
}
//-----------------------------------------------------------------------------
void VortexExampleApp::glutMouseMoveHandler_stub(int x, int y)
//-----------------------------------------------------------------------------
{
	instance()->glutMouseMoveHandler(x,y);
}
//-----------------------------------------------------------------------------
void VortexExampleApp::glutMouseMoveHandler(int x, int y)
//-----------------------------------------------------------------------------
{
	for (Tools::iterator i = m_tools.begin(); i!=m_tools.end(); i++)
	{
		if ((*i)->onMouseMove(x, y))
		{
			break;
		}
	}
}
//-----------------------------------------------------------------------------
void VortexExampleApp::glutMouseDragHandler_stub(int x, int y )
//-----------------------------------------------------------------------------
{
	instance()->glutMouseDragHandler(x,y);
}
//-----------------------------------------------------------------------------
void VortexExampleApp::glutMouseDragHandler(int x, int y )
//-----------------------------------------------------------------------------
{
	for (Tools::iterator i = m_tools.begin(); i!=m_tools.end(); i++)
	{
		if ((*i)->onMouseDrag(x, y, m_mouse.eventStartX, m_mouse.eventStartY))
		{
			break;
		}
	}
}
//-----------------------------------------------------------------------------
void	VortexExampleApp::glutIdleHandler_stub()
//-----------------------------------------------------------------------------
{
	instance()->glutIdleHandler();
}
//-----------------------------------------------------------------------------
void	VortexExampleApp::glutDisplay_stub()
//-----------------------------------------------------------------------------
{
	instance()->glutDisplay();
}
//-----------------------------------------------------------------------------
void	VortexExampleApp::setupFrustum()
//-----------------------------------------------------------------------------
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 60, m_ui.xy_aspect, 0.001, 5000);
}
//-----------------------------------------------------------------------------
void	VortexExampleApp::glutDisplay()
//-----------------------------------------------------------------------------
{
	if (m_view.freezeDraw) return;

	glDrawBuffer( GL_BACK );
	drawBackground();									// backdrop clear and draw

	setupFrustum();

	Tools::iterator i;
	for (i = m_tools.begin(); i!=m_tools.end(); i++)		// Tool pre draw
		(*i)->drawPreDisplay();	

	drawPointClouds();									// draw Point Clouds

	if (m_view.showLayerBounds)							// draw the bounding box of the layer
		drawBoundingBox();

	for (i = m_tools.begin(); i!=m_tools.end(); i++)		// Tool post draw
		(*i)->drawPostDisplay();
	
	drawAxes(2.0f);
	drawLogo();
	m_view.clearFrame = false;
	m_view.freezeIdle = false;

	glutSwapBuffers(); 
}
//-----------------------------------------------------------------------------
void	VortexExampleApp::glutIdleHandler()
//-----------------------------------------------------------------------------
{
	if (m_view.freezeIdle) return;

	Sleep(30); // Otherwise we'll get 100% cpu use

	if ( glutGetWindow() != m_ui.mainWindow ) 
	glutSetWindow( m_ui.mainWindow );  


	for (Tools::iterator i = m_tools.begin(); i!=m_tools.end(); i++)
	{
		(*i)->onIdle();
	}

	if (m_dirty)
	{
		glutPostRedisplay();
		m_dirty = false;
	}

}
//-----------------------------------------------------------------------------
void VortexExampleApp::setRenderer( Renderer *renderer )
//-----------------------------------------------------------------------------
{
	m_renderer = renderer;
}
//-----------------------------------------------------------------------------
Renderer				*VortexExampleApp::getRenderer()
//-----------------------------------------------------------------------------
{
	return m_renderer;
}
//-----------------------------------------------------------------------------
VortexExampleApp::VortexExampleApp( std::string &appTitle )
//-----------------------------------------------------------------------------
{
	m_title = appTitle;
	m_renderer = 0;
	m_dirty = false;

	m_bbLower[0] = 0;
	m_bbLower[1] = 0;
	m_bbLower[2] = 0;
	m_bbUpper[0] = 0;
	m_bbUpper[1] = 0;
	m_bbUpper[2] = 0;
}
//-----------------------------------------------------------------------------
void	VortexExampleApp::notifySceneUpdate( void )
//-----------------------------------------------------------------------------
{
	updateBoundingBox();

	for (Tools::iterator i = m_tools.begin(); i!=m_tools.end(); i++)
	{
		(*i)->onSceneUpdate();
	}
}

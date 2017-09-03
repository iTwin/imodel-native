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
#include <gl/glew.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <gl/freeglut_ext.h>

/* C++ headers */ 
#include <vector>
#include <iostream>

/* resources */ 
#include "../res/resource.h"

/* app framework */
#include "../include/ExampleApp.h"

/* utility */ 
#include "../include/timer.h"

/* vortex initialization state */ 
bool	isVortexLoaded = false;
GLint	g_uniformObjectID = 0;
GLint	g_uniformSpherical = 1;

// UI Constants
#define COPYRIGHT_NOTICE	"   Copyright 2017. Bentley Systems All Rights Reserved"

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

	wchar_t bufferTemp[300];
	wchar_t buffer[300];
	swprintf_s(bufferTemp, 300, L"%s (*.%s)\0", desc, ext);
	swprintf_s(buffer, 300, L"%s*.%s\0\0", bufferTemp, ext);

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

    wchar_t bufferTemp[300];
    wchar_t buffer[300];
    swprintf_s(bufferTemp, 300, L"%s (*.%s)\0", desc, ext);
    swprintf_s(buffer, 300, L"%s*.%s\0\0", bufferTemp, ext);

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
Multitouch::Multitouch()
//-----------------------------------------------------------------------------
{
	for (int i = 0; i < 4; i++)
	{
		active[i] = false;
		eventStartX[i] = -1;
		eventStartY[i] = -1;
		lastX[i] = -1;
		lastY[i] = -1;
		finger[i] = 0;
	}
}
//-----------------------------------------------------------------------------
View::View()
//-----------------------------------------------------------------------------
{
	dynamic = false;
	enableDynamic = true; // when not enabled dynamic rendering is never used

	ownDisplay = 0;
	clearFrame = true;
	freezeIdle = false;
	freezeDraw = false;

	queryMax = -1;
	cacheSize = -1;

	isViewComplete = false;

	showLayerBounds = false;

	iteration =0 ;
	progress = 0;

	backColor[0] = 0;
	backColor[1] = 0;
	backColor[2] = 0;
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
void enableMultisample(int msaa)
{
	if (msaa)
	{
		glEnable(GL_MULTISAMPLE);
		glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);

		// detect current settings
		GLint iMultiSample = 0;
		GLint iNumSamples = 0;
		glGetIntegerv(GL_SAMPLE_BUFFERS, &iMultiSample);
		glGetIntegerv(GL_SAMPLES, &iNumSamples);
		printf("MSAA on, GL_SAMPLE_BUFFERS = %d, GL_SAMPLES = %d\n", iMultiSample, iNumSamples);
	}
	else
	{
		glDisable(GL_MULTISAMPLE);
		printf("MSAA off\n");
	}
}


char*	ExampleApp::textFileRead(char *fn) 
{


	FILE *fp;
	char *content = NULL;

	int count = 0;

	if (fn != NULL) 
	{
		fp = fopen(fn, "rt");

		if (fp != NULL)
		{

			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0)
			{
				content = (char *)malloc(sizeof(char) * (count + 1));
				count = fread(content, sizeof(char), count, fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}

//jchen
bool ExampleApp::createShaders()
{
//Load, create, and compile the vertex shader.
	m_hdlShader_vert = glCreateShader(GL_VERTEX_SHADER);
	if (m_hdlShader_vert == 0)
	{
		std::cout << "Failed to create a vertex shader" << std::endl;
		return false;
	}
	char*  pShaderSourceCode = (textFileRead("domeprojection.vert"));
	if (pShaderSourceCode == NULL)
	{
		std::cout << "Failed to open and load the vertex shader source file: domeprojection.vert" << std::endl;
		return false;
	}
	glShaderSource(m_hdlShader_vert, 1, (&pShaderSourceCode), NULL);
	GLenum res = glGetError();
	if (res != GL_NO_ERROR)
	{
		std::cout << "Failed at glShaderSource(...); Error == " << res << std::endl;
		return false;
	}
	free(pShaderSourceCode);

	glCompileShader(m_hdlShader_vert);
	res = glGetError();
	if (res != GL_NO_ERROR)
	{
		std::cout << "Failed to compile the vertex shader cource code; Error == " << res << std::endl;
		return false;
	}
	GLint shaderCompileResult;
	glGetShaderiv(m_hdlShader_vert, GL_COMPILE_STATUS, &shaderCompileResult);
	if (shaderCompileResult==1)
	{
		std::cout << "Succeeded to compile the vertex shader;" << std::endl;
	}
	else
	{
		std::cout << "Failed to compile the vertex shader cource code. There exists syntax error in the vertex shader source code." << std::endl;
		return false;
	}

	//Load, create, and compile the fragment shader.
	m_hdlShader_frag = glCreateShader(GL_FRAGMENT_SHADER);
	if (m_hdlShader_vert == 0)
	{
		std::cout << "Failed to create a fragment shader" << std::endl;
		return false;
	}
	pShaderSourceCode = (textFileRead("domeprojection.frag"));
	if (pShaderSourceCode == NULL)
	{
		std::cout << "Failed to open and load the fragment shader source file: domeprojection.frag" << std::endl;
		return false;
	}
	glShaderSource(m_hdlShader_frag, 1, (&pShaderSourceCode), NULL);
	res = glGetError();
	if (res != GL_NO_ERROR)
	{
		std::cout << "Failed at glShaderSource(...); Error == " << res << std::endl;
		return false;
	}
	free(pShaderSourceCode);

	glCompileShader(m_hdlShader_frag);
	res = glGetError();
	if (res != GL_NO_ERROR)
	{
		std::cout << "Failed to compile the fragment shader cource code; Error == " << res << std::endl;
		return false;
	}
	glGetShaderiv(m_hdlShader_frag, GL_COMPILE_STATUS, &shaderCompileResult);
	if (shaderCompileResult == 1)
	{
		std::cout << "Succeeded to compile the fragment shader;" << std::endl;
	}
	else
	{
		std::cout << "Failed to compile the fragment shader cource code. There exists syntax error in the fragment shader source code." << std::endl;
		return false;
	}


	GLuint hdlGPUProgram = glCreateProgram();
	if (hdlGPUProgram == 0)
	{
		return false;
	}
	glAttachShader(hdlGPUProgram, m_hdlShader_vert);
	glAttachShader(hdlGPUProgram, m_hdlShader_frag);

	glLinkProgram(hdlGPUProgram);
	glUseProgram(hdlGPUProgram);
	res = glGetError();
	if (res != GL_NO_ERROR)
	{
		std::cout << "Failed to use the shaders; Error == " << res << std::endl;
		return false;
	}

	std::cout << "Succeeded to load and link the shaders. The shaders are ready for use." << std::endl;
	g_uniformObjectID = glGetUniformLocation(hdlGPUProgram, "objectID");
	g_uniformSpherical = glGetUniformLocation(hdlGPUProgram, "useSphericalProjection");

	return true;
}

//-----------------------------------------------------------------------------
bool ExampleApp::setupGL(int argc, char* argv[])
//-----------------------------------------------------------------------------
{
	// glut
	glutInit(&argc, argv);

	glutSetOption(GLUT_MULTISAMPLE, 4);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );// | GLUT_MULTISAMPLE );
	
	glutInitWindowPosition( 50, 50 );
	glutInitWindowSize( 800, 600 );
	
	m_ui.mainWindow = glutCreateWindow( "Bentley Reality Modeling | Viewer" );
	
	glutDisplayFunc( glutDisplay_stub );

	glutMultiButtonFunc(&ExampleApp::glutMultitouchButton_stub);
	glutMultiMotionFunc(&ExampleApp::glutMultitouchMotion_stub);
	glutMultiEntryFunc(&ExampleApp::glutMultitouchEntry_stub);

	GLenum err = glewInit();
	if (GLEW_OK != err)
		std::cout << "GLEW FAILED" << std::endl;
	if (!GLEW_VERSION_2_0)
		std::cout << " Wrong version " << std::endl;
	// gl options
	glEnable(GL_DEPTH_TEST);
	
	//enableMultisample(GL_TRUE);

	// setup lighting
	GLfloat light0_ambient[] =  {0.15f, 0.15f, 0.15f, 1.0f};
	GLfloat light0_diffuse[] =  {0.8f, 0.8f, 0.8f, 1.0f};
	GLfloat light0_spec[] =  {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat light0_position[] = {1.5f, 1.5f, -2.0f };

	glEnable(GL_LIGHTING);

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_spec);

	m_rc =0;
	m_dc =0 ;


	createShaders(); //jchen

	return true;
}
//-----------------------------------------------------------------------------
namespace 
{
	ExampleApp* _app_instance_=0;
}
//-----------------------------------------------------------------------------
ExampleApp* ExampleApp::instance()
//-----------------------------------------------------------------------------
{
	return _app_instance_;
}
//-----------------------------------------------------------------------------
bool ExampleApp::initializeApplication(int argc, char* argv[], bool debugVortex)
//-----------------------------------------------------------------------------
{	
	_app_instance_ = this;

	pt::PerformanceTimer::initialize();				// initialize utility timer

	if (!setupGL(argc,argv))	return false;

	m_ui.loadResources();								// load the Logo resource

	// read the appliaction folder
	::GetModuleFileNameA(NULL, m_appFolder, MAX_PATH);
	::PathRemoveFileSpecA(m_appFolder);
}
//-----------------------------------------------------------------------------
bool ExampleApp::initializeTools(int argc, char* argv[])
//-----------------------------------------------------------------------------
{
	for (int i = 0; i < m_tools.size(); i++)
	{
		m_tools[i]->parseCommandLine(argc, argv);
	}
	return true;
}
//-----------------------------------------------------------------------------
bool ExampleApp::runApplication()
//-----------------------------------------------------------------------------
{
	buildUserInterface();							// build the user interface, usually overridden

	GLUI_Master.set_glutIdleFunc( ExampleApp::glutIdleHandler_stub );
	glutMainLoop();

	return true;
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
	ExampleApp::instance()->dirty();
}
//-----------------------------------------------------------------------------
void Tool::viewRedraw()
//-----------------------------------------------------------------------------
{
	View &view = ExampleApp::instance()->getView();
	
	view.clearFrame=true;
	ExampleApp::instance()->dirty();
}
//-----------------------------------------------------------------------------
void Tool::addStatisticMessage(const char *mess)
//-----------------------------------------------------------------------------
{
	//todo: add to UI
	std::cout << mess << std::endl;
}
//-----------------------------------------------------------------------------
void Tool::getViewportSize(int & x, int & y, int & w, int & h) const
//-----------------------------------------------------------------------------
{
	GLUI_Master.get_viewport_area(&x, &y, &w, &h);
}
//-----------------------------------------------------------------------------
void Tool::dispatchCmd( int cmdId )
//-----------------------------------------------------------------------------
{
	ExampleApp::instance()->dispatchUICmd_stub( cmdId );
}
//-----------------------------------------------------------------------------
void	Tool::startDynamicView()
//-----------------------------------------------------------------------------
{
	if (ExampleApp::instance()->getView().enableDynamic)
		ExampleApp::instance()->getView().dynamic = true;
}
//-----------------------------------------------------------------------------
void	Tool::endDynamicView()
//-----------------------------------------------------------------------------
{	
	ExampleApp::instance()->getView().dynamic = false;	
}
//-----------------------------------------------------------------------------
Tool::~Tool()
//-----------------------------------------------------------------------------
{
}
//-----------------------------------------------------------------------------
void ExampleApp::dispatchUICmd_stub( int cmdId )
//-----------------------------------------------------------------------------
{
	instance()->dispatchUICmd( cmdId );
}
//-----------------------------------------------------------------------------
void ExampleApp::dispatchUICmd( int cmdId )
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
std::string ExampleApp::getFullResourcePath( const std::string &file ) const
//-----------------------------------------------------------------------------
{
	std::string path;

	path = m_appFolder;
	path += "\\" + file;

	//sprintf_s(path, MAX_PATH, "%s\\%s", m_appFolder, file);

	return path;
}
//-----------------------------------------------------------------------------
void ExampleApp::addTool( Tool *tool )
//-----------------------------------------------------------------------------
{
	m_tools.push_back( tool );
}
//-----------------------------------------------------------------------------
View &ExampleApp::getView()
//-----------------------------------------------------------------------------
{
	return m_view;
}
//-----------------------------------------------------------------------------
UI &ExampleApp::getUI()
//-----------------------------------------------------------------------------
{
	return m_ui;
}
//-----------------------------------------------------------------------------
void	ExampleApp::buildUserInterface()
//-----------------------------------------------------------------------------
{
	GLUI_Master.set_glutReshapeFunc( glutResizeHandler_stub );  
	GLUI_Master.set_glutKeyboardFunc( glutKeyboardHandler_stub );
	GLUI_Master.set_glutSpecialFunc( NULL );
	GLUI_Master.set_glutMouseFunc( glutMouseButtonHandler_stub );
	glutMotionFunc( glutMouseDragHandler_stub );
	glutPassiveMotionFunc( glutMouseMoveHandler_stub );

	return;	// for this example we want no UI, so return 

	// Create the side panel
	m_ui.panelSide = GLUI_Master.create_glui_subwindow( m_ui.mainWindow, GLUI_SUBWINDOW_RIGHT );

	// Title
	// the following notices must appear in the UI as a condition of use 
	std::string versionStr( "ScalableMesh Demo" );	// todo: add actual version 
	versionStr +=  " - ";
	versionStr += m_title;
	
	GLUI_StaticText *txt = new GLUI_StaticText( m_ui.panelSide, versionStr.c_str() );

	txt = new GLUI_StaticText( m_ui.panelSide, "   CONFIDENTIAL" );
	txt = new GLUI_StaticText( m_ui.panelSide, COPYRIGHT_NOTICE );
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
	// no longer used
	return true;
}
//-----------------------------------------------------------------------------
void ExampleApp::drawBackground()
//-----------------------------------------------------------------------------
{
	if (m_renderer && (m_view.clearFrame || !m_view.ownDisplay ))
	{
		m_renderer->drawBackground();
	}
}
//-----------------------------------------------------------------------------
GL_ScalableMesh *ExampleApp::addScalableMesh(const std::string &file)
//-----------------------------------------------------------------------------
{
	GL_ScalableMesh *sm = new GL_ScalableMesh(file.c_str());
	if (sm)
		m_meshes.push_back(sm);
	return sm;
} 
//-----------------------------------------------------------------------------
Camera & ExampleApp::getCamera()
//-----------------------------------------------------------------------------
{
	return m_camera;
}
//-----------------------------------------------------------------------------
void ExampleApp::drawPointClouds()
//-----------------------------------------------------------------------------
{
	if (m_renderer)
	{
		m_renderer->drawPointClouds( m_view.dynamic, m_view.clearFrame );
	}
}
//-----------------------------------------------------------------------------
void ExampleApp::drawMeshes()
//-----------------------------------------------------------------------------
{
	bool viewcomplete = true;
	 
	GL_Camera camera;

	camera.far_plane = m_camera.farPlane();
	camera.near_plane = m_camera.nearPlane();
	camera.fov = m_camera.fov();
	camera.position[0] = m_camera.position().x;
	camera.position[1] = m_camera.position().y;
	camera.position[2] = m_camera.position().z;

	camera.target[0] = m_camera.target().x;
	camera.target[1] = m_camera.target().y;
	camera.target[2] = m_camera.target().z;

	// start to render polygons
	// render Scalable Mesh
	for (int i = 0; i < m_meshes.size(); i++)
	{		
		// Make SM use a specific tex unit for the textures 
		//m_meshes[i]->setUniformTexID( nodef.getTexUnitUniformID() );
		viewcomplete &= m_meshes[i]->draw(camera);
	}

	if (!viewcomplete)
		glutPostRedisplay();
}
//-----------------------------------------------------------------------------
void ExampleApp::drawBoundingBox()
//-----------------------------------------------------------------------------
{
	glColor3f(0.7f,0.7f,0.7f);
	m_view.drawBox(m_bbLower, m_bbUpper, 0, 0);
}
//-----------------------------------------------------------------------------
void ExampleApp::drawLogo()
//-----------------------------------------------------------------------------
{
	return;

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
void ExampleApp::drawAxes( float scale )
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
void View::drawPntMarker( double *pnt, double size )
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
void	ExampleApp::updateBoundingBox()
//-----------------------------------------------------------------------------
{
	// get lastest BB, change last param to true for faster approx calc
	// not used
}

//-----------------------------------------------------------------------------
void ExampleApp::glutKeyboardHandler_stub(unsigned char Key, int x, int y)
//-----------------------------------------------------------------------------
{
	instance()->glutKeyboardHandler(Key, x, y);
}
//-----------------------------------------------------------------------------
void ExampleApp::glutKeyboardHandler(unsigned char Key, int x, int y)
//-----------------------------------------------------------------------------
{
	switch(Key)
	{
	case 27: 
	case 'q':
		exit(0);
		break;
	case 's':
	case 'S':
		if (m_bUseSphericalProjection == false)
		{
			m_bUseSphericalProjection = true;
			glUniform1i(g_uniformSpherical, 0);
		}
		else
		{
			m_bUseSphericalProjection = false;
			glUniform1i(g_uniformSpherical, 1);
		}
		break;
	default:
		break;
	}
}

//-----------------------------------------------------------------------------
void ExampleApp::glutResizeHandler_stub( int x, int y )
//-----------------------------------------------------------------------------
{
	instance()->glutResizeHandler(x,y);
}
//-----------------------------------------------------------------------------
void ExampleApp::glutResizeHandler( int x, int y )
//-----------------------------------------------------------------------------
{
  int tx, ty, tw, th;
  GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
  glViewport( tx, ty, tw, th );

  m_ui.xy_aspect = (float)tw / (float)th;

  m_dirty=true;
}
//-----------------------------------------------------------------------------
void ExampleApp::glutMultitouchEntry_stub(int finger_id, int touch_event)
//-----------------------------------------------------------------------------
{
	ExampleApp::instance()->glutMultitouchEntry(finger_id, touch_event);
}
//-----------------------------------------------------------------------------
void ExampleApp::glutMultitouchEntry(int finger_id, int touch_event)
//-----------------------------------------------------------------------------
{
	if (touch_event == 0)	//up
	{
		for (int f = 0; f < 4; f++)
		{
			if (m_multitouch.finger[f] == finger_id)
			{
				m_multitouch.active[f] = false;
				m_multitouch.lastX[f] = -1;
				m_multitouch.lastY[f] = -1;
				m_multitouch.finger[f] = 0;

				//std::cout << "MT Entry: finger " << f << " up" << std::endl;

				for (Tools::iterator i = m_tools.begin(); i != m_tools.end(); i++)
				{
					if ((*i)->onMultitouchExit(f))
					{
						break;
					}
				}
				break;
			}
		}
	}
	else
	{
		int f = 0;

		//down - find a spare slot
		for (f = 0; f < 4; f++) if (!m_multitouch.active[f]) break;

		if (f > 3) return;	// not supported

		m_multitouch.active[f] = true;
		m_multitouch.finger[f] = finger_id;
		m_multitouch.eventStartX[f] = -1;
		m_multitouch.eventStartY[f] = -1;

		//std::cout << "MT Entry: finger " << f << " down" << std::endl;

		for (Tools::iterator i = m_tools.begin(); i != m_tools.end(); i++)
		{
			if ((*i)->onMultitouchExit(f))
			{
				break;
			}
		}
	}
}
//-----------------------------------------------------------------------------
void ExampleApp::glutMultitouchButton_stub(int finger_id, int x, int y, int button, int touch_event)
//-----------------------------------------------------------------------------
{
	ExampleApp::instance()->glutMultitouchHandler(finger_id, x, y, touch_event);
}
//-----------------------------------------------------------------------------
void ExampleApp::glutMultitouchHandler(int finger_id, int x,int y, int touch_event)
//-----------------------------------------------------------------------------
{		
	if (touch_event == 1)	//up
	{
		//std::cout << "MT Button Up" << std::endl;

		for (Tools::iterator i = m_tools.begin(); i != m_tools.end(); i++)
		{
			if ((*i)->onMultitouchUp(0, x, y))
			{
				break;
			}
		}
	}
	else
	{
		//std::cout << "MT Button Down" << std::endl;

		for (Tools::iterator i = m_tools.begin(); i != m_tools.end(); i++)
		{
			if ((*i)->onMultitouchDown(0, x, y))
			{
				break;
			}
		}
	}
	
}
//-----------------------------------------------------------------------------
void ExampleApp::glutMultitouchMotion_stub(int finger_id, int x, int y)
//-----------------------------------------------------------------------------
{
	ExampleApp::instance()->glutMultitouchMotionHandler(finger_id, x, y);
}
//-----------------------------------------------------------------------------
void ExampleApp::glutMultitouchMotionHandler(int finger_id, int x, int y)
//-----------------------------------------------------------------------------
{
	// find finger
	int f;
	int f2;

	// count how many fingers active
	int fingers_down = 0;
	for (f = 0; f < 4; f++)
	{
		if (m_multitouch.active[f]) ++fingers_down;
	}

	// find this event active finger
	for (f = 0; f < 4; f++)
	{
		if (m_multitouch.finger[f] == finger_id) break;
	}

	if (f > 3) return;

	if (m_multitouch.eventStartX[f] == -1)
	{
		m_multitouch.eventStartX[f] = x;
		m_multitouch.eventStartY[f] = y;
	}
	
	// check for duplicate event
	if (m_multitouch.lastX[f] == x && m_multitouch.lastY[f] == y)
	{
		return;
	}
	m_multitouch.lastX[f] = x;
	m_multitouch.lastY[f] = y;

	//std::cout << "MT Motion: finger " << f << " - " << x << ", " << y << std::endl;

	// compute mode hint based on finger seperation 
	if (fingers_down == 2)
	{
		// find f2
		for (f2 = 0; f2 < 4; f2++)
		{
			if (m_multitouch.active[f2] && m_multitouch.finger[f2] != finger_id) break;
		}
		for (Tools::iterator i = m_tools.begin(); i != m_tools.end(); i++)
		{
			if ((*i)->onMultitouchDoubleDrag(m_multitouch.lastX[f], m_multitouch.lastY[f], 
				m_multitouch.lastX[f2], m_multitouch.lastY[f2],
				m_multitouch.eventStartX[f], m_multitouch.eventStartY[f], 
				m_multitouch.eventStartX[f2], m_multitouch.eventStartY[f2]))
			{
				break;
			}
		}
	}

	for (Tools::iterator i = m_tools.begin(); i != m_tools.end(); i++)
	{
		if ((*i)->onMultitouchMotion(finger_id, x, y))
		{
			break;
		}
	}
}
//-----------------------------------------------------------------------------
void ExampleApp::glutMouseButtonHandler_stub(int button, int button_state, int x, int y )
//-----------------------------------------------------------------------------
{
	instance()->glutMouseButtonHandler(button, button_state, x, y);
}
//-----------------------------------------------------------------------------
void ExampleApp::glutMouseButtonHandler(int button, int button_state, int x, int y )
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
void ExampleApp::glutMouseMoveHandler_stub(int x, int y)
//-----------------------------------------------------------------------------
{
	instance()->glutMouseMoveHandler(x,y);
}
//-----------------------------------------------------------------------------
void ExampleApp::glutMouseMoveHandler(int x, int y)
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
void ExampleApp::glutMouseDragHandler_stub(int x, int y )
//-----------------------------------------------------------------------------
{
	instance()->glutMouseDragHandler(x,y);
}
//-----------------------------------------------------------------------------
void ExampleApp::glutMouseDragHandler(int x, int y )
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
void	ExampleApp::glutIdleHandler_stub()
//-----------------------------------------------------------------------------
{
	instance()->glutIdleHandler();
}
//-----------------------------------------------------------------------------
void	ExampleApp::glutDisplay_stub()
//-----------------------------------------------------------------------------
{
	instance()->glutDisplay();
}
//-----------------------------------------------------------------------------
void	ExampleApp::setupFrustum()
//-----------------------------------------------------------------------------
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( m_camera.fov(), m_ui.xy_aspect, m_camera.nearPlane(), m_camera.farPlane() );
}
//-----------------------------------------------------------------------------
void	ExampleApp::glutDisplay()
//-----------------------------------------------------------------------------
{
	if (!m_rc) m_rc = wglGetCurrentContext();
	if (!m_dc) m_dc = wglGetCurrentDC();

	HWND win = WindowFromDC(m_dc);

	Tools::iterator i;

	for (i = m_tools.begin(); i != m_tools.end(); i++)		// Tool post draw
		(*i)->preDrawSetup();

	if (m_view.freezeDraw) return;

	glUniform1i(g_uniformObjectID, 1);
	glDrawBuffer( GL_BACK );
	glUniform1i(g_uniformObjectID, 2);
	drawBackground();									// backdrop clear and draw

	setupFrustum();
	glUniform1i(g_uniformObjectID, 3);
	for (i = m_tools.begin(); i!=m_tools.end(); i++)		// Tool pre draw
		(*i)->drawPreDisplay();	

	drawPointClouds();									// draw Point Clouds
	glUniform1i(g_uniformObjectID, 13);
	drawMeshes();

	glUniform1i(g_uniformObjectID, 4);
	if (m_view.showLayerBounds)							// draw the bounding box of the layer
		drawBoundingBox();

	glUniform1i(g_uniformObjectID, 5);
	for (i = m_tools.begin(); i!=m_tools.end(); i++)		// Tool post draw
		(*i)->drawPostDisplay();
	
	drawAxes(2.0f);
	drawLogo();
	m_view.clearFrame = false;
	m_view.freezeIdle = false;
	 
	glutSwapBuffers(); 

	for (i = m_tools.begin(); i!=m_tools.end(); i++)		// Tool post draw
		(*i)->onPostSwap();
}
//-----------------------------------------------------------------------------
void	ExampleApp::glutIdleHandler()
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
void ExampleApp::setRenderer( Renderer *renderer )
//-----------------------------------------------------------------------------
{
	m_renderer = renderer;
}
//-----------------------------------------------------------------------------
Renderer				*ExampleApp::getRenderer()
//-----------------------------------------------------------------------------
{
	return m_renderer;
}
//-----------------------------------------------------------------------------
ExampleApp::ExampleApp( std::string &appTitle )
//-----------------------------------------------------------------------------
{
	m_bUseSphericalProjection = false;
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
void	ExampleApp::notifySceneUpdate( void )
//-----------------------------------------------------------------------------
{
	updateBoundingBox();

	for (Tools::iterator i = m_tools.begin(); i!=m_tools.end(); i++)
	{
		(*i)->onSceneUpdate();
	}
}
//-----------------------------------------------------------------------------
HGLRC					ExampleApp::getRC()
//-----------------------------------------------------------------------------
{
	return m_rc;
}
//-----------------------------------------------------------------------------
HDC						ExampleApp::getDC()
//-----------------------------------------------------------------------------
{
	return m_dc;
}

Camera::Camera()
{
	m_fov = 60;
	m_nearPlane = 0.50;
	m_farPlane = 1800;

	m_target.zero();

	m_position.x = 30.0;
	m_position.y = 20.0;
	m_position.z = 50.0;

	m_up.x = 0;
	m_up.y = 0;
	m_up.z = 1.0;
}
const Vector3f & Camera::target() const
{
	return m_target;
}
Vector3f & Camera::target()
{
	return m_target;
}
const Vector3f & Camera::position() const
{
	return m_position;
}
Vector3f & Camera::position()
{
	return m_position;
}

Vector3f & Camera::up()
{
	return m_up;
}

double &Camera::farPlane()
{
	return m_farPlane;
}

double &Camera::nearPlane()
{
	return m_nearPlane;
}

double &Camera::fov()
{
	return m_fov;
}
const double &Camera::farPlane() const
{
	return m_farPlane;
}

const double &Camera::nearPlane() const
{
	return m_nearPlane;
}

const double &Camera::fov() const
{
	return m_fov;
}
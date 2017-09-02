/****************************************************************************

Pointools Vortex API example application framework

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

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
   is made to alter the apparent origin or author of the work.

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

#ifndef POINTOOLS_EXAMPLE_APP_FRAMEWORK_H_
#define POINTOOLS_EXAMPLE_APP_FRAMEWORK_H_

// OpenGL and GLUI 
#pragma warning(push)
#pragma warning(disable : 4244)

#include <gl/glew.h>
#include <gl/glui.h>

#pragma warning(pop)

#include <gl/glu.h>

// C++ headers 
#include <string>
#include <vector>
#include <iostream>

// utility
#include "../include/timer.h"

// API header 
#include "3smgl.h"

#include "Geom.h"

// UI constants
#define PANEL_WIDTH			270

//! UI
//! User Interface class
class UI
{
public:
	UI();
	void			addStat(const std::string &stat);
	void			addOutput(const std::string &txt);
	void			addTimeStat( pt::PerformanceTimer &t, const char* desc );
	const wchar_t	*getSaveFilePath( const wchar_t * desc, const wchar_t *ext );
	const wchar_t	*getLoadFilePath( const wchar_t * desc, const wchar_t *ext );
	
	bool			getColor( COLORREF initial, COLORREF &chosen );

	bool			loadResources();

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

	GLUI_TextBox	*mbSec;

	GLuint			logo;

	int				mainWindow;
	float			xy_aspect;
};

//! Current Mouse State
class  Mouse
{
public:
	Mouse();

	int eventStartX;
	int eventStartY;
	int lastX;
	int lastY;
};

//! Current Multitouch State
class  Multitouch
{
public:
	Multitouch();

	int finger[4];
	bool active[4];
	int eventStartX[4];
	int eventStartY[4];
	int lastX[4];
	int lastY[4];
};


enum MouseButton
{
	MouseLeftButton = 0,
	MouseMidButton = 1,
	MouseRightButton = 2
};
//! Renderer 
//! provides rendering services to application
//! Override for not standard rendering
class Renderer
{
public:
	virtual void			drawBackground()=0;
	virtual bool			drawPointClouds(bool dynamic, bool clearFrame )=0;
};

//! View 
//! Stores viewport state
//! and provides basic draw capabilities
class View
{
public:
	View();

	void	drawPntMarker( double *pnt, double size );
	void	drawBox( double *lower, double *upper, double *position, double *rotation);
	void	drawBox( float *lower, float *upper, float *position, float *rotation);

	pt::PerformanceTimer timeSinceKbUpdate;

	int		cacheSize;
	int		iteration;
	bool	dynamic;
	int		loadBias;
	int		ownDisplay;
	int		renderLimit;
	int		progress;
	bool	clearFrame;
	bool	freezeIdle;
	bool	freezeDraw;
	bool	isViewComplete;
	bool	enableDynamic;

	GLfloat backColor[3];

	bool	showLayerBounds;
	int		queryMax;
};

class Camera
{
public:
	Camera();

	Vector3f	&target();
	Vector3f	&position();
	Vector3f	&up();
	const Vector3f	&target() const;
	const Vector3f	&position() const;
	const Vector3f	&up() const;

	double		&farPlane();
	double		&nearPlane();
	double		&fov();
	const double &farPlane() const;
	const double &nearPlane() const;
	const double &fov() const;

private:
	Vector3f	m_target;
	Vector3f	m_position;
	Vector3f	m_up;

	double		m_farPlane;
	double		m_nearPlane;
	double		m_fov;
};

struct GLHelper
{
	static GLuint loadTexturePNG(const std::string &path, int &width, int &height, GLuint tex_type = GL_TEXTURE_2D);
	static GLuint loadTextureJPG(const std::string &path, int &width, int &height, GLuint tex_type = GL_TEXTURE_2D);
	static GLubyte *loadPNG(const std::string &path, int &width, int &height, bool &has_alpha);	// caller must delete []
	static void renderFullViewportQuad(bool texCoords = false);
	static void error(const char * s, ...);
};

//! Tool class (Model-Controller)
//! Used to add functionality to application
class Tool
{
public:
	Tool(int cmdStart=-1, int cmdEnd=-1);
	virtual ~Tool();

	virtual void parseCommandLine(int argc, char* argv[]) {};

	virtual void command( int cmdId )	{}
	virtual void buildUserInterface(GLUI_Node *parent)	{}

	virtual bool onMouseButtonDown( int button, int x, int y )			{ return false; }
	virtual bool onMouseButtonUp( int button, int x, int y )			{ return false; }
	virtual bool onMouseMove( int x, int y )							{ return false; }
	virtual bool onMouseDrag( int x, int y, int startX, int startY )	{ return false; }
	
	virtual bool onMultitouchEntry(int finger_id)						{ return false; }
	virtual bool onMultitouchExit(int finger_id)						{ return false; }
	virtual bool onMultitouchDown(int finger_id, int x, int y)			{ return false; }
	virtual bool onMultitouchUp(int finger_id, int x, int y)			{ return false; }
	virtual bool onMultitouchMotion(int finger_id, int x, int y )		{ return false; }
	virtual bool onMultitouchDoubleDrag(int x, int y, int x2, int y2, 
		int startx, int starty, int startx2, int starty2)				{ return false; }

	virtual void onSceneUpdate( void )									{};
	virtual void onIdle()												{};

	virtual void preDrawSetup()			{}
	virtual void drawPostDisplay()		{}
	virtual void drawPreDisplay()		{}
	virtual void onPostSwap()			{}

	bool	ownsCmd( int cmdId ) const 	
	{ 
		return (cmdId >= cmdRangeStart && cmdId <= cmdRangeEnd) ? true : false; 
	}

protected:
	enum
	{
		CmdRedrawView = 10000
	};
	void	getViewportSize(int &x, int &y, int &w, int &h) const;

	static	void dispatchCmd( int cmdId );
	void	startDynamicView();
	void	endDynamicView();

	void	addStatisticMessage(const char *mess);
	void	viewRedraw();
	void	viewUpdate();

private:
	int		cmdRangeStart;
	int		cmdRangeEnd;

};

//! ExampleApp
//! Application base class for example apps
class ExampleApp
{
public: //after change it to private. jchen
	GLuint		m_hdlShader_vert;
	GLuint		m_hdlShader_frag;
	bool		m_bUseSphericalProjection;
public:
	ExampleApp( std::string &appTitle );

	// initialization
	virtual bool			initializeApplication(int argc, char* argv[],bool debugVortexDll=false);
	virtual bool			initializeTools(int argc, char* argv[]);
	virtual bool			runApplication();
	virtual void			buildUserInterface();

	// singleton instance
	static ExampleApp	*instance();

	std::string				getFullResourcePath( const std::string &file) const;
	void					addTool( Tool* tool );
	
	void					setRenderer( Renderer *render );
	Renderer				*getRenderer();

	static void				dispatchUICmd_stub( int cmdId );
	void					dispatchUICmd( int cmdId );

	View & 					getView();
	UI &					getUI();

	virtual void			glutDisplay();

	void					dirty() { m_dirty = true; }

	void					updateBoundingBox();

	void					notifySceneUpdate( void );

	virtual void			setupFrustum();

	HGLRC					getRC();
	HDC						getDC();

	GL_ScalableMesh*		addScalableMesh(const std::string &file);
	
	Camera					&getCamera();

protected:
	
	void					drawLogo();
	void					drawAxes( float scale );

	virtual void			drawBackground();
	void					drawBoundingBox();
	virtual void			drawPointClouds();
	virtual void			drawMeshes();
	
	// glut handlers
	static void				glutIdleHandler_stub();
	virtual void			glutIdleHandler();

	static void				glutDisplay_stub();

	static void				glutResizeHandler_stub( int x, int y );
	virtual void			glutResizeHandler( int x, int y );

	static void				glutKeyboardHandler_stub(unsigned char Key, int x, int y);
	void					glutKeyboardHandler(unsigned char Key, int x, int y);
	
	static void				glutMouseButtonHandler_stub(int button, int button_state, int x, int y );
	void					glutMouseButtonHandler(int button, int button_state, int x, int y );

	static void				glutMouseMoveHandler_stub(int x, int y );
	void					glutMouseMoveHandler(int x, int y );

	static void				glutMouseDragHandler_stub(int x, int y );
	void					glutMouseDragHandler(int x, int y );

	static void				glutMultitouchEntry_stub(int finger_id, int touch_event);
	void					glutMultitouchEntry(int finger_id, int touch_event);

	static void				glutMultitouchButton_stub(int finger_id, int x, int y, int button, int touch_event);
	void					glutMultitouchHandler(int finger_id, int x, int y, int touch_event);

	static void				glutMultitouchMotion_stub(int finger_id, int x, int y);
	void					glutMultitouchMotionHandler(int finger_id, int x, int y);

	virtual					bool setupGL(int argc, char* argv[]);
	virtual					bool createShaders();
	virtual					char*	ExampleApp::textFileRead(char *);
	//components
	std::string				m_title;

	Mouse					m_mouse;
	Multitouch				m_multitouch;
	UI						m_ui;
	View					m_view;

	Camera					m_camera;

	HGLRC					m_rc;
	HDC						m_dc;

	char					m_appFolder[MAX_PATH];

private:

	Renderer				*m_renderer;	// for point clouds, not used in this example

	typedef					std::vector<Tool*>	Tools;
	Tools					m_tools;

	std::vector<GL_ScalableMesh*>	m_meshes;

	// viewport redraw flag
	bool					m_dirty;

	float					m_bbLower[3];
	float					m_bbUpper[3];
};

//! Wide char to Ascii char
//! Utility function needed for GLUI which doesn't support wchar
extern char *wcToAscii( const wchar_t *wstr );

#endif
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
#ifdef PT_BUILD_ENV
	#include <gl/glui.h>
#else
	//#include "../include/glui.h"
#include <gl/glui.h>
#endif

#include <gl/glu.h>

// C++ headers 
#include <string>
#include <vector>
#include <iostream>

// utility
#include "../include/timer.h"

// API header 
#include <PointoolsVortexAPI_DLL/PTAPI/PointoolsVortexAPI_import.h>

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

	void	drawPntMarker( PTdouble *pnt, PTdouble size );
	void	drawBox( PTdouble *lower, PTdouble *upper, PTdouble *position, PTdouble *rotation);
	void	drawBox( PTfloat *lower, PTfloat *upper, PTfloat *position, PTfloat *rotation);

	pt::PerformanceTimer timeSinceKbUpdate;

	PThandle displayQueryHandle;

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

	bool	showLayerBounds;
	int		queryMax;
};

//! Tool class (Model-Controller)
//! Used to add functionality to application
class Tool
{
public:
	Tool(int cmdStart=-1, int cmdEnd=-1);
	virtual ~Tool();

	virtual void command( int cmdId )	{}
	virtual void buildUserInterface(GLUI_Node *parent)	{}

	virtual bool onMouseButtonDown( int button, int x, int y )			{ return false; }
	virtual bool onMouseButtonUp( int button, int x, int y )			{ return false; }
	virtual bool onMouseMove( int x, int y )							{ return false; }
	virtual bool onMouseDrag( int x, int y, int startX, int startY )	{ return false; }
	virtual void onSceneUpdate( void )									{};
	virtual void onIdle()												{};

	virtual void drawPostDisplay()		{}
	virtual void drawPreDisplay()		{}

	bool	ownsCmd( int cmdId ) const 	
	{ 
		return (cmdId >= cmdRangeStart && cmdId <= cmdRangeEnd) ? true : false; 
	}

protected:
	enum
	{
		CmdRedrawView = 10000
	};
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

//! VortexExampleApp
//! Application base class for example apps
class VortexExampleApp
{
public:
	VortexExampleApp( std::string &appTitle );

	// initialization
	virtual bool			initializeApplication(int argc, char* argv[],bool debugVortexDll=false);
	virtual bool			runApplication();
	virtual void			buildUserInterface();

	// singleton instance
	static VortexExampleApp	*instance();

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

protected:
	
	void					drawLogo();
	void					drawAxes( float scale );

	virtual void			drawBackground();
	void					drawBoundingBox();
	virtual void			drawPointClouds();
	
	virtual void			setupFrustum();

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

	virtual					bool setupGL(int argc, char* argv[]);
	
	//components
	std::string				m_title;

	Mouse					m_mouse;
	UI						m_ui;
	View					m_view;

private:

	Renderer				*m_renderer;

	typedef					std::vector<Tool*>	Tools;
	Tools					m_tools;

	bool					initializeVortexAPI(bool debugVortexDll=false);
	void					loadRampsLists();

	// viewport redraw flag
	bool					m_dirty;

	PTfloat					m_bbLower[3];
	PTfloat					m_bbUpper[3];

};

//! Wide char to Ascii char
//! Utility function needed for GLUI which doesn't support wchar
extern char *wcToAscii( const wchar_t *wstr );

#endif
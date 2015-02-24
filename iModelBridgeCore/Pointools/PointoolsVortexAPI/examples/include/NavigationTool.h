/******************************************************************************

Pointools Vortex API Examples

NavigationTool.h

Provides basic camera navigation for example applications. Does not demonstrate
any Vortex features

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_NAVIGATION_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_NAVIGATION_TOOL_H_

#include "VortexExampleApp.h"

class CameraNavigation : public Tool
{
public:
	CameraNavigation();

	bool onMouseButtonDown( int button, int x, int y );
	bool onMouseButtonUp( int button, int x, int y );
	
	bool onMouseDrag( int x, int y, int startX, int startY );

	void drawPreDisplay();

	enum MouseMode
	{
		MouseNone = 0, 
		MouseOrbit = 1,
		MousePan = 2,
		MouseZoom = 3
	};

private:
	MouseMode	m_mode;
	float		m_rotateX;
	float		m_rotateY;
	float		m_rotateZ;

	float		m_rotateXtmp;
	float		m_rotateYtmp;
	float		m_rotateZtmp;

	float		m_posX;
	float		m_posY;
	float		m_posZ;

	float		m_posXtmp;
	float		m_posYtmp;
	float		m_posZtmp;
};

#endif


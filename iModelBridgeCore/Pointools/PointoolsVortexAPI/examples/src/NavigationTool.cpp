/******************************************************************************

Pointools Vortex API Examples

NavigationTool.cpp

Provides basic camera navigation for example applications. Does not demonstrate
any Vortex features

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#include "NavigationTool.h"

//-----------------------------------------------------------------------------
CameraNavigation::CameraNavigation()
//-----------------------------------------------------------------------------
{
	m_mode = MouseNone;

	m_rotateX = 0;
	m_rotateY = 0;
	m_rotateZ = 0;

	m_rotateXtmp = 0;
	m_rotateYtmp = 0;
	m_rotateZtmp = 0;

	m_posX = 0;
	m_posY = 0;
	m_posZ = -20;

	m_posXtmp = 0;
	m_posYtmp = 0;
	m_posZtmp = 0;
}
//-----------------------------------------------------------------------------
bool CameraNavigation::onMouseButtonDown( int button, int x, int y )
//-----------------------------------------------------------------------------
{
	m_mode = MouseNone;

	switch(button)
	{ 
		case 0: m_mode = MouseOrbit; break;
		case 1: m_mode = MousePan; break;
		case 2: m_mode = MouseZoom; break;
	}
	return true;
}
//-----------------------------------------------------------------------------
bool CameraNavigation::onMouseButtonUp( int button, int x, int y )
//-----------------------------------------------------------------------------
{
	/* end navigation operation */ 
	m_rotateX += m_rotateXtmp;
	m_rotateY += m_rotateYtmp;
	m_rotateZ += m_rotateZtmp;

	m_rotateXtmp = 0;
	m_rotateYtmp = 0;
	m_rotateZtmp = 0;

	m_posX += m_posXtmp;
	m_posY += m_posYtmp;
	m_posZ += m_posZtmp;
	
	m_posXtmp = 0;
	m_posYtmp = 0;
	m_posZtmp = 0;

	m_mode = MouseNone;
	
	endDynamicView();
	glutPostRedisplay();

	return true;
}
//-----------------------------------------------------------------------------
bool CameraNavigation::onMouseDrag( int x, int y, int startX, int startY )
//-----------------------------------------------------------------------------
{
	View &view = VortexExampleApp::instance()->getView();

	view.clearFrame = true;

	switch (m_mode)
	{
	case MouseOrbit:
		m_rotateZtmp = 0.5f * (x - startX);
		m_rotateXtmp = 0.5f * (y - startY);
		startDynamicView();
		break;
	case MousePan:
		m_posXtmp = 0.05f * (x - startX);
		m_posYtmp = -0.05f * (y - startY);
		startDynamicView();
		break;
	case MouseZoom:
		m_posZtmp = 0.05f * (y - startY); 
		startDynamicView();
		break;
	}
	if (view.dynamic)	glutPostRedisplay();
	return true;
}
//-----------------------------------------------------------------------------
void CameraNavigation::drawPreDisplay()
//-----------------------------------------------------------------------------
{
	glMatrixMode( GL_MODELVIEW );
	glTranslatef( 0.0, 0.0, -30 );

	glLoadIdentity(); 

	glTranslatef( m_posXtmp + m_posX, m_posYtmp + m_posY, m_posZtmp + m_posZ );

	glRotatef( m_rotateX + m_rotateXtmp, 1,0,0);
	glRotatef( m_rotateY + m_rotateYtmp, 0,1,0);
	glRotatef( m_rotateZ + m_rotateZtmp, 0,0,1);
}


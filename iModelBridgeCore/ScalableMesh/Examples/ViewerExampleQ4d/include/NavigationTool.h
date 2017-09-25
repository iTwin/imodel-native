/******************************************************************************

Pointools Vortex API Examples

NavigationTool.h

Provides basic camera navigation for example applications. Does not demonstrate
any Vortex features

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_NAVIGATION_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_NAVIGATION_TOOL_H_

#include "ExampleApp.h"

class CameraNavigation : public Tool
{
public:
	CameraNavigation();

	bool onMouseButtonDown( int button, int x, int y );
	bool onMouseButtonUp( int button, int x, int y );	
	bool onMouseDrag( int x, int y, int startX, int startY );

	bool onMultitouchDown(int finger_id, int x, int y);
	bool onMultitouchUp(int finger_id, int x, int y);
	
	bool onMultitouchEntry(int finger_id);
	bool onMultitouchExit(int finger_id);
	bool onMultitouchMotion(int finger_id, int x, int y);

	bool onMultitouchDoubleDrag(int x, int y, int x2, int y2,
		int startx, int starty, int startx2, int starty2);

	void drawPreDisplay();
	void drawPostDisplay();

	void getCameraPosition( float &x, float &y, float &z ) const;
	void setCameraPosition(const Vector3f &pos);
	void setCameraTarget(const Vector3f &tar);
	void setCameraNearPlane(double near_plane);
	void setCameraFarPlane(double far_plane);
	void setCameraFov(double fov);

	Vector3f getCameraPosition() const	{ return m_camera.position() + m_positionShift;  }
	Vector3f getCameraTarget() const	{ return m_camera.target() + m_targetShift; }
	Vector3f getCameraUp() const		{ return m_camera.up(); }
	double	getCameraNearPlane() const { return m_camera.nearPlane(); }
	double	getCameraFarPlane() const { return m_camera.farPlane(); }
	double  getCameraFOV() const { return m_camera.fov(); }
	

	void transitionTo(Vector3f new_position, Vector3f new_target);

	enum MouseMode
	{
		MouseNone = 0, 
		MouseOrbit = 1,
		MousePan = 2,
		MouseZoom = 3,
		MouseMultitouch = 4,
		MouseMultitouchPan = 5,
		MouseMultitouchZoom = 6
	};

	static CameraNavigation *instance();
	
private:

	MouseMode	m_mode;
	Camera		&m_camera;

	Vector3f	m_targetShift;
	Vector3f	m_positionShift;

	Vector3f	m_basisX;
	Vector3f	m_basisY;
	Vector3f	m_basisZ;

	// for event handling
	Vector3f	m_startEventBasisY;
	Vector3f	m_startEventBasisX;

	Matrix4f	m_projectionMatrix;
	Matrix4f	m_modelviewMatrix;
	Matrix4f	m_screenMatrix;

	int			m_eventStartX;
	int			m_eventStartY;

	int			m_multitouchLastX;
	int			m_multitouchLastY;

	bool		getSnapPoint(int x, int y, Vector3f &point);
	
	bool		doubleClickRetarget(int x, int y, bool touch);
	bool		longTouchRetarget(int x, int y);

	GLdouble	m_projection[16];
	GLdouble	m_modelview[16];
	GLint		m_viewport[4];

	bool		m_dotransition;
	float		m_transitionPoint;
	Vector3f	m_transitionPosition;
	Vector3f	m_transitionTarget;
	Vector3f	m_transitionFromPosition;
	Vector3f	m_transitionFromTarget;

	static		CameraNavigation*	s_instance;
};

#endif


/******************************************************************************

Pointools Vortex API Examples

NavigationTool.cpp

Provides basic camera navigation for example applications. Does not demonstrate
any Vortex features

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#include "NavigationTool.h"
#include "timer.h"

#include <time.h>
#define DEG2RAD 0.01745329251994329576923690768489

CameraNavigation* CameraNavigation::s_instance = 0;

//-----------------------------------------------------------------------------
CameraNavigation *CameraNavigation::instance()
//-----------------------------------------------------------------------------
{
	return s_instance;
}
//-----------------------------------------------------------------------------
bool CameraNavigation::getSnapPoint(int mx, int my, Vector3f & point)
//-----------------------------------------------------------------------------
{
	// not implemented 
	return false;
}

//-----------------------------------------------------------------------------
CameraNavigation::CameraNavigation()
	: m_camera( ExampleApp::instance()->getCamera() )
//-----------------------------------------------------------------------------
{
	m_mode = MouseNone;

	m_targetShift.zero();
	m_positionShift.zero();

	m_basisX.zero();
	m_basisY.zero();
	m_basisZ.zero();

	m_modelviewMatrix.identity();
	m_projectionMatrix.identity();
	m_screenMatrix.identity();

	m_transitionPoint = 0;
	m_dotransition = false;

	s_instance = this;
}
//-----------------------------------------------------------------------------
bool CameraNavigation::longTouchRetarget(int x, int y)		// this one zooms in
//-----------------------------------------------------------------------------
{
	static DWORD	timestamp = 0;
	static int		click_x = 0;
	static int		click_y = 0;

	DWORD t2 = GetTickCount();	
#ifdef _DEBUG
	std::cout << "MT Long Touch TEST" << std::endl;
#endif
	int interval = t2 - timestamp;

	if (interval > 500 && interval < 1200)
	{
		// check its the same place
		if (abs(click_x - x) < 10 && abs(click_y - y) < 10)
		{
#ifdef _DEBUG
			std::cout << "MT Long Touch" << std::endl;
#endif
			// find point under cursor and target this
			Vector3f new_target;
			bool found_point = getSnapPoint(x, y, new_target);

			if (found_point)
			{
				Vector3f offset = new_target - m_camera.target();
				Vector3f new_position = m_camera.position() + offset;

				Vector3f dir(new_position, new_target);
				dir.normalize();

				new_position = new_target - dir * 15.0f;	// hardcoded 15m, good for goodwood

				transitionTo(new_position, new_target);

				glutPostRedisplay();

				return true;
			}
		}
	}
	timestamp = t2;
	click_x = x;
	click_y = y;

	return false;
}
static std::vector<Vector3d> test_points;
//-----------------------------------------------------------------------------
bool CameraNavigation::doubleClickRetarget(int x, int y, bool touch)
//-----------------------------------------------------------------------------
{
	static DWORD	timestamp = 0;
	static int		click_x = 0;
	static int		click_y = 0;
	static bool		is_touch = false;	

	DWORD t2 = GetTickCount();

	// double click is re-center 
	if (t2 - timestamp < 500)
	{
		if (touch != is_touch)	// touch generates mouse events
		{
			// don't update state
			return false;
		}
		// check its the same place
		if ( abs(click_x - x) < 10 && abs(click_y-y) < 10)
		{
#ifdef _DEBUG
			std::cout << "MT Double Touch" << std::endl;
#endif
			// find point under cursor and target this
			Vector3f new_target;
			bool found_point = getSnapPoint(x, y, new_target);

			if (found_point)
			{
				test_points.push_back( Vector3d(new_target) );

				return true;

				Vector3f offset = m_camera.position() - m_camera.target();
				offset.normalize();
				offset *= 20.0f;
				Vector3f new_position = offset + new_target;

				transitionTo(new_position, new_target);

				glutPostRedisplay();

				return true;
			}
		}
	}
	timestamp = t2;
	click_x = x;
	click_y = y;
	is_touch = touch;

	return false;
}
//-----------------------------------------------------------------------------
bool CameraNavigation::onMouseButtonDown( int button, int x, int y )
//-----------------------------------------------------------------------------
{
#ifdef _DEBUG
	std::cout << "Mouse down" << std::endl;
#endif

	if (doubleClickRetarget(x, y, false))
		return true;

	m_mode = MouseNone;

	switch(button)
	{ 
		case 0: m_mode = MouseOrbit; break;
		case 1: m_mode = MousePan; break;
		case 2: m_mode = MouseZoom; break;
		case 3: m_mode = MouseMultitouch; break;
	}


	m_eventStartX = x;
	m_eventStartY = y;

	m_startEventBasisX = m_basisX;
	m_startEventBasisY = m_basisY;

	m_startEventBasisX.normalize();
	m_startEventBasisY.normalize();

	return true;
}
//-----------------------------------------------------------------------------
bool CameraNavigation::onMouseButtonUp( int button, int x, int y )
//-----------------------------------------------------------------------------
{
	m_mode = MouseNone;

	m_camera.position() += m_positionShift;
	m_camera.target() += m_targetShift;

	m_positionShift.zero();
	m_targetShift.zero();

	endDynamicView();
	glutPostRedisplay();

	return true;
}
static int fingers_down = 0;
//-----------------------------------------------------------------------------
bool CameraNavigation::onMouseDrag( int x, int y, int startX, int startY )
//-----------------------------------------------------------------------------
{
	if (fingers_down > 1) return true;
#ifdef _DEBUG
	std::cout << "Mouse Drag" << std::endl;
#endif
	// prevent immediate stop of a transition, but ig its at least half way through cancel it
	if (m_dotransition)
	{
		if (m_transitionPoint > 0.5)
			m_dotransition = false;
		else return true;
	}

	View &view = ExampleApp::instance()->getView();

	view.clearFrame = true;

	switch (m_mode)
	{
	case MouseOrbit:
	{
		double z_rot = 0.20f * (x - startX);
		double x_rot = 0.20f * (y - startY);

		Vector3f rel_pos = m_camera.position() - m_camera.target();
		Vector3f new_rel_pos;

		Matrix4f rot;

		rot.identity();
		rot.axisAngle(-x_rot, m_startEventBasisX);

		new_rel_pos = rel_pos * rot;


		rot.axisAngle(-z_rot, m_startEventBasisY);
		new_rel_pos *= rot;

		m_positionShift = new_rel_pos - rel_pos;

		startDynamicView();
	}
		break;

	case MousePan:
	{
		Vector3f px, py;

		// move camera in its coordinate frame
		// for this we need the basis vectors
		px = m_basisX;
		py = m_basisY;

		// should already be normalised
		px.normalize();
		py.normalize();

		float dist2tar = (m_camera.position() - m_camera.target()).length();

		double x_shift = -0.00091 * (x - startX)  * dist2tar;
		double y_shift = 0.00091 * (y - startY) * dist2tar;

		px *= x_shift;
		py *= y_shift;
		px += py;

		m_positionShift = px;
		m_targetShift = px;

		startDynamicView();
	}
		break;

	case MouseZoom:
	{
		// move camera towards target
		Vector3f dir = m_camera.target() - m_camera.position();
		double d = dir.length();
		double t = 0.1 * (y - startY);

		dir.normalize();
		dir *= t;

		m_positionShift = dir;

		//shift target forward if too close
		if (t > d)
		{
			dir.normalize();
			dir *= (t - d + 5);

			m_targetShift = dir;
		}

		startDynamicView();
	}
		break;
	}
	if (view.dynamic)	glutPostRedisplay();
	return true;
}


//-----------------------------------------------------------------------------
bool CameraNavigation::onMultitouchDown(int finger_id, int x, int y)
//-----------------------------------------------------------------------------
{
	fingers_down++;
	
	if (doubleClickRetarget(x, y, true))
		return true;

	longTouchRetarget(x, y);

	m_multitouchLastX = x;
	m_multitouchLastY = y;

	return false;
}
//-----------------------------------------------------------------------------
bool CameraNavigation::onMultitouchUp(int finger_id, int x, int y)
//-----------------------------------------------------------------------------
{
	return longTouchRetarget(x, y);
}
//-----------------------------------------------------------------------------
bool CameraNavigation::onMultitouchEntry(int finger_id)
//-----------------------------------------------------------------------------
{
	if (fingers_down == 2)
	{
		m_mode = MouseMultitouch;

		m_startEventBasisX = m_basisX;
		m_startEventBasisY = m_basisY;

		m_startEventBasisX.normalize();
		m_startEventBasisY.normalize();
	}
	return false;
}
//-----------------------------------------------------------------------------
bool CameraNavigation::onMultitouchExit(int finger_id)
//-----------------------------------------------------------------------------
{
	fingers_down--;

	if (fingers_down != 2)
	{
		onMouseButtonUp(0, 0, 0);
	}
	return false;
}
//-----------------------------------------------------------------------------
bool CameraNavigation::onMultitouchMotion(int finger_id, int x, int y)
//-----------------------------------------------------------------------------
{
	return false;
}
//-----------------------------------------------------------------------------
bool CameraNavigation::onMultitouchDoubleDrag(int x, int y, int x2, int y2, 
	int startx, int starty, int startx2, int starty2)
//-----------------------------------------------------------------------------
{
	// just be sure we have 2
	//if (fingers_down == 2)
	{
		View &view = ExampleApp::instance()->getView();
		view.clearFrame = true;

		// decide if to do pan or zoom
		Vector3f f1(x, y, 0);
		Vector3f f2(x2, y2, 0);
		Vector3f sf1(startx, starty, 0);
		Vector3f sf2(startx2, starty2, 0);

		Vector3f fv1 = f1 - sf1;
		Vector3f fv2 = f2 - sf2;
		
		Vector3f pan = (fv1 + fv2);
		float zoom_length = f1.dist(f2) - sf1.dist(sf2);

		float dist_to_target = m_camera.position().dist(m_camera.target());
		float width_view_at_depth = tan(m_camera.fov()*DEG2RAD) * dist_to_target / m_viewport[2];

		if (pan.length() < fabs(zoom_length) || m_mode == MouseMultitouchZoom)
		{
			m_mode = MouseMultitouchZoom;
#ifdef _DEBUG
			std::cout << "# ZOOM" << std::endl;
#endif
			// move camera towards target - actually should be towards center of pull
			Vector3f dir = m_camera.target() - m_camera.position();

			//double d = dist_to_target;
			//double t = 0.1 * (y - startY);

			float zoom_dist = dist_to_target * zoom_length / m_viewport[2];

			std::cout << "Zoom by " << zoom_dist << std::endl;

			//double t = zoom_length * 0.1;

			dir.normalize();
			dir *= zoom_dist;

			m_positionShift = dir;

			//shift target forward if too close
			if (zoom_dist > dist_to_target)
			{
				dir.normalize();
				dir *= (zoom_dist - dist_to_target + 5);

				m_targetShift = dir;
			}
		}
		else
		{
			pan *= width_view_at_depth * 0.75;

			m_mode = MouseMultitouchPan;
#ifdef _DEBUG
			std::cout << "# PAN vector = " << pan.x << ", " << pan.y << std::endl;
#endif
			Vector3f px, py;

			// move camera in its coordinate frame
			// for this we need the basis vectors
			px = m_basisX;
			py = m_basisY;

			// should already be normalised
			px.normalize();
			py.normalize();

			px *= -pan.x;
			py *= pan.y;
			px += py;

			m_positionShift = px;
			m_targetShift = px;
		}
		
		startDynamicView();
		
		if (view.dynamic)	glutPostRedisplay();

		return true;
	}
	return false;
}
//-----------------------------------------------------------------------------
void CameraNavigation::drawPreDisplay()
//-----------------------------------------------------------------------------
{
	glGetDoublev(GL_PROJECTION_MATRIX, m_projection);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Vector3f current_target = m_targetShift + m_camera.target();
	Vector3f current_position = m_positionShift + m_camera.position();

	gluLookAt(current_position.x, current_position.y, current_position.z,
		current_target.x, current_target.y, current_target.z,
		m_camera.up().x, m_camera.up().y, m_camera.up().z);

	// draw the target with pass through shader
	//NoDeferredRendering nodef;

	float target_scale = 0.5f;

	glColor3f(1.0f, 0, 1.0f);
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_LINES);
		glVertex3f(m_camera.target().x - target_scale, m_camera.target().y, m_camera.target().z);
		glVertex3f(m_camera.target().x + target_scale, m_camera.target().y, m_camera.target().z);

		glVertex3f(m_camera.target().x, m_camera.target().y - target_scale, m_camera.target().z);
		glVertex3f(m_camera.target().x, m_camera.target().y + target_scale, m_camera.target().z);

		glVertex3f(m_camera.target().x, m_camera.target().y, m_camera.target().z - target_scale);
		glVertex3f(m_camera.target().x, m_camera.target().y, m_camera.target().z + target_scale);
	glEnd();

	glGetDoublev(GL_MODELVIEW_MATRIX, m_modelview);
	glGetIntegerv(GL_VIEWPORT, m_viewport);

//	std::cout << "  CAM: " << m_camera.position().x << ", " << m_camera.position().y << ", " << m_camera.position().z;
//	std::cout << "  TAR: " << m_camera.target().x << ", " << m_camera.target().y << ", " << m_camera.target().z;
	Vector3f d(m_camera.target());
	d -= m_camera.position();
	d.normalize();
	d *= 8.0f;
	d += m_camera.position();
//	std::cout << std::endl;
//	std::cout << "  TAR_8: " << d.x << ", " << d.y << ", " <<d.z;
//	std::cout << std::endl;

	m_basisX.x = m_modelview[0];
	m_basisX.y = m_modelview[4];
	m_basisX.z = m_modelview[8];
	m_basisY.x = m_modelview[1];
	m_basisY.y = m_modelview[5];
	m_basisY.z = m_modelview[9];
	m_basisZ.x = m_modelview[2];
	m_basisZ.y = m_modelview[6];
	m_basisZ.z = m_modelview[10];
}
//-----------------------------------------------------------------------------
void CameraNavigation::drawPostDisplay()
//-----------------------------------------------------------------------------
{
	glColor3f(1.0f, 1.0f, 1.0f);

	View &v = ExampleApp::instance()->getView();

	//Vector3d wtp;
	//GL_ScalableMesh::getWorldOffset(wtp.x, wtp.y, wtp.z);

	//for (int i = 0; i < test_points.size(); i++)
	//{
	//	v.drawPntMarker(&test_points[i].x, 2.0);
	//}

	// do transition to new position if requested
	if (m_dotransition)
	{
		m_transitionPoint += 0.05f;
		if (m_transitionPoint > 1.0f)
		{
			endDynamicView();
			m_camera.position() = m_transitionPosition;
			m_camera.target() = m_transitionTarget;

			// do this a few times to ensure frame is fully updated
			if (m_transitionPoint > 1.2f)
			{
				m_transitionPoint = 0;
				m_dotransition = false;
			}
			else
			{
				// jiggle view
				Vector3f dir = m_transitionTarget - m_transitionPosition;
				dir.normalize();
				m_camera.position() = m_transitionPosition + dir * 0.05f;
				viewRedraw();
			}
		}
		else
		{
			m_camera.position() = m_transitionFromPosition + (m_transitionPosition - m_transitionFromPosition) * (1 - cos(m_transitionPoint*3.142))*0.5;
			m_camera.target() = m_transitionFromTarget + (m_transitionTarget - m_transitionFromTarget) * (1 - cos(m_transitionPoint*3.142))*0.5;

			viewRedraw();
		}
	}
}
//-----------------------------------------------------------------------------
void CameraNavigation::getCameraPosition( float &x, float &y, float &z ) const
//-----------------------------------------------------------------------------
{
	Vector3f current_position = m_positionShift + m_camera.position();

	x = current_position.x;
	y = current_position.y;
	z = current_position.z;
}
//-----------------------------------------------------------------------------
void CameraNavigation::setCameraPosition(const Vector3f &pos)
//-----------------------------------------------------------------------------
{
	m_camera.position() = pos;
}
//-----------------------------------------------------------------------------
void CameraNavigation::setCameraTarget(const Vector3f &pos)
//-----------------------------------------------------------------------------
{
	m_camera.target() = pos;
}
//-----------------------------------------------------------------------------
void CameraNavigation::setCameraNearPlane(double near_plane)
//-----------------------------------------------------------------------------
{
	m_camera.nearPlane() = near_plane;
}
//-----------------------------------------------------------------------------
void CameraNavigation::setCameraFarPlane(double far_plane)
//-----------------------------------------------------------------------------
{
	m_camera.farPlane() = far_plane;
}
//-----------------------------------------------------------------------------
void CameraNavigation::setCameraFov(double fov)
//-----------------------------------------------------------------------------
{
	m_camera.fov() = fov;
}
//-----------------------------------------------------------------------------
void CameraNavigation::transitionTo(Vector3f new_position, Vector3f new_target)
//-----------------------------------------------------------------------------
{
	m_transitionPosition = new_position;
	m_transitionTarget = new_target;
	m_transitionFromPosition = m_camera.position();
	m_transitionFromTarget = m_camera.target();
	m_transitionPoint = 0;
	m_dotransition = true;
}


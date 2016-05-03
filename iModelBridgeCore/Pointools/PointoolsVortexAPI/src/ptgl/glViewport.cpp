/*--------------------------------------------------------------------------*/
/*	Pointools Viewport class implmentation									*/
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/
/*																			*/
/*  Last Updated 12 Dec 2003 Faraz Ravi										*/
/*--------------------------------------------------------------------------*/
#include "PointoolsVortexAPIInternal.h"


#include <ptgl/glViewport.h>
#include <math/matrix_math.h>

#include <ptgl/gldraw.h>
#include <pt/project.h>

#define _VIEWPORT_VERSION 4

#ifndef _PI
#define _PI 3.1415926535897932384626433832795
#endif

using namespace pt;
using namespace ptgl;

#define COUTTRACER
#include <pt/trace.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Viewport::Viewport()
{
	/*intitialize members*/ 
	m_status = VPS_NORC;

	m_actualrate = 30.0f;
	m_framerate = 22.0f;

	m_camera.setFrustum(50.0, 1.0f, 1.0f, 70.0);
	m_camera.centerViewAt(vector3(0,0,3));
	m_camera.setOrtho(false);
	m_camera.setLight(&m_lights[0]);

	/*default control mapping*/ 
	m_op_map[VP_NONE] = 0;
	m_op_map[VP_PANNING] = VP_MOUSE_MIDDLE;
	m_op_map[VP_ZOOMING] = VP_MOUSE_RIGHT | VP_MOUSE_LEFT;
	m_op_map[VP_ROTATING] = VP_MOUSE_LEFT;

	m_LOD = VP_LOD_FULL;

	m_mouse_state = 0;

	m_plot			= false;
	m_hasFocus		= true;
	m_in2d			= false;
	m_override_lock = false;
	m_next = 0;
	m_paintcb = 0;
	m_notify = true;
	m_sync = true;
	m_up = 2;

	nav_free();

	m_backcolor.r = 0.05098f;
	m_backcolor.g = 0.05490f;
	m_backcolor.b = 0.07450f;

	m_backcolor2.r = 0.4275f;
	m_backcolor2.g = 0.4509f;
	m_backcolor2.b = 0.5215f;

	m_gridcolor.r = 0.5f;
	m_gridcolor.g = 0.5f;
	m_gridcolor.b = 0.5f;

	m_usegradient = true;
	
	m_lights[0].on();
	m_lights[1].off();
	m_lights[2].off();
	m_lights[3].off();
	m_colorPixels = 0;

	invalidateView();

	m_blitmode = VP_BLIT_NONE;
}

Viewport::~Viewport()
{
	if (m_colorPixels) delete [] m_colorPixels;
}
//---------------------------------------------------------------------
// Register painter (simple callback mechanism)
//---------------------------------------------------------------------
void Viewport::setDrawCallback(paintCB cb)
{
	m_paintcb = cb;
}
//---------------------------------------------------------------------
// public paint method
//---------------------------------------------------------------------
void Viewport::paint(UINT lod)
{
	m_LOD = lod;
	paint();
}
//---------------------------------------------------------------------
// Paint
//---------------------------------------------------------------------
void Viewport::paint()
{
	alignLayers();
	
	invalidateView();

	if (m_LOD != VP_LOD_REFRESH && m_camera.getNavigationState() == Camera::NONE)
		m_LOD = VP_LOD_FULL;

	else if (m_view_lock != VP_FULL_LOCK && m_paintcb)
	{
		m_paintcb(this, m_LOD);
	}

	m_viewState = 1;
	glFinish();
}
//---------------------------------------------------------------------
// setSize
//---------------------------------------------------------------------
void Viewport::setSize(const Recti &rWin)
{
	static bool init = false;
	int w = rWin.ux() *3;
	w = (w + 3) & ~3;
	int bitsize = w * rWin.uy();

	if (m_colorPixels) delete [] m_colorPixels;
	m_colorPixels = new GLubyte[bitsize];

	m_viewRect = rWin;

	m_camera.onResize(rWin.dx(), rWin.dy());
	m_camera.update();
	alignLayers();

	init = true;
}
//---------------------------------------------------------------------
// Setup 2d layer
//---------------------------------------------------------------------
void Viewport::setup2dview()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	Recti vp;
	getViewRect(vp);

	/*centre in window*/
	glTranslated((double)vp.dx()/2.0, (double)vp.dy()/2.0, 0);

	/*zoom scaling*/
//	glScaled(1.0 / m_v2d.dZoom, 1.0 / m_v2d.dZoom, 1.0f);

	/*pan translation*/
//	glTranslated(-m_v2d.dPan[0], -m_v2d.dPan[1], 0);
}
//---------------------------------------------------------------------
// Setup 2d proj
//---------------------------------------------------------------------
void Viewport::setup2dproj()
{
	glDisable(GL_DEPTH_TEST);

	int t = m_viewRect.dy();
	int b = 0;
	int l = 0;
	int r = m_viewRect.dx();

	//set up viewport clipping range
	glViewport(l, b, r, t);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluOrtho2D(l, r, b, t);
}
//---------------------------------------------------------------------
// Setup Pixel
//---------------------------------------------------------------------
void Viewport::setupPixel()
{
	glViewport(0, 0, m_viewRect.dx(), m_viewRect.dy());
	glDisable(GL_DEPTH_TEST);

	//setup otho2d
	//PROJECTION -------------------------------
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	int t = m_viewRect.dy();
	int b = 0;
	int l = 0;
	int r = m_viewRect.dx();

	gluOrtho2D(l, r, b, t);
	//-------------------------------------------
	// MODELVIEW
	//clear any transforms
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}
//---------------------------------------------------------------------
// Get View Rect - returns drawing area (exc frame);
//---------------------------------------------------------------------
void Viewport::getViewRect(Recti &r)
{
	r = m_viewRect;
}
//---- Set back Color ---------------------------------------------------
void Viewport::setBackColor(const ptgl::Color &col) { m_backcolor = col; }
//----- GetBackColor ----------------------------------------------------
void Viewport::getBackColor(ptgl::Color &p) const	 { p = m_backcolor; }
//---- Set back Color ---------------------------------------------------
void Viewport::setBackColor2(const ptgl::Color &col) { m_backcolor2 = col; }
//----- GetBackColor ----------------------------------------------------
void Viewport::getBackColor2(ptgl::Color &p) const   { p = m_backcolor2; }
//----- SetFrameRate ----------------------------------------------------
void Viewport::setFrameRate(int fr) { PTTRACEOUT << "SetFrameRate = " << fr; m_framerate = fr; }
//----- GetFrameRate ----------------------------------------------------
int Viewport::getFrameRate() const	{ return m_framerate; }
//----- setUseGradient --------------------------------------------------
void Viewport::setUseGradient(bool use) { m_usegradient = use; }
//----- getUseGradient --------------------------------------------------
bool Viewport::getUseGradient() const { return m_usegradient; }
//----- setGridColor ----------------------------------------------------
void Viewport::setGridColor(const ptgl::Color &col) { m_gridcolor = col; }
//----- getGridColor ----------------------------------------------------
void Viewport::getGridColor(ptgl::Color &col) const { col = m_gridcolor; }
//---------------------------------------------------------------------
// Draw Axis
//---------------------------------------------------------------------
void Viewport::drawAxis() const
{
	glPushAttrib(GL_LINE_BIT | GL_VIEWPORT_BIT | GL_TRANSFORM_BIT);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	//axes
	//model --------------------------------------
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslated(0.0, 0.0, -2.6);
	m_camera.pushGLrotation();

	//UCS *u = ViewAlgorithms::GetUCS();
	//u->buildGLrotate();

	//proj ----------------------------------------
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glOrtho(-1.6f, 1.6f, -1.6f, 1.6f, 0.1f, 5.0f);

	//view ----------------------------------------
	glViewport(0,0,65,65);
	glLineWidth(2.0f);
	glShadeModel(GL_FLAT);

	int up = m_camera.getConstraintAxis();
	bool con = m_camera.getUseConstraints();

	float col_1[3], col_2[3];
	double pos[3];
	int a, b;
	for (int i=0; i<3; i++)
	{
		if (!i)	{ a = 1; b = 2;	}
		else if (i == 1) {	a = 0; b = 2; }
		else	{ a = 0; b = 1;	}

		col_1[a] = 0.1f; col_1[b] = 0.1f; col_1[i] = (!con || (con && i == up)) ? 1.0f : 0.4f;
		col_2[a] = 0.4f; col_2[b] = 0.4f; col_2[i] = (!con || (con && i == up)) ? 1.0f : 0.75f;
		/*line*/
		glBegin(GL_LINES);
			glColor3f(col_1[0], col_1[1], col_1[2]);
			pos[a] = 0; pos[b] = 0; pos[i] = -0.01f;
			glVertex3dv(pos);
			pos[i] = 1.0f;
			glVertex3dv(pos);
		glEnd();
		/*arrow*/
		glBegin(GL_TRIANGLE_FAN);
			glColor3f(col_1[0], col_1[1], col_1[2]);
			pos[a] = 0; pos[b] = 0; pos[i] = 1.4;
			glVertex3dv(pos);

			glColor3f(col_2[0], col_2[1], col_2[2]);
			pos[a] = -0.2; pos[b] = -0.2; pos[i] = 1;
			glVertex3dv(pos);

			glColor3f(col_1[0], col_1[1], col_1[2]);
			pos[a] = -0.2; pos[b] = 0.2; pos[i] = 1;
			glVertex3dv(pos);

			glColor3f(col_2[0], col_2[1], col_2[2]);
			pos[a] = 0.2; pos[b] = 0.2; pos[i] = 1;
			glVertex3dv(pos);

			glColor3f(col_1[0], col_1[1], col_1[2]);
			pos[a] = 0.2; pos[b] = -0.2; pos[i] = 1;
			glVertex3dv(pos);

			glColor3f(col_2[0], col_2[1], col_2[2]);
			pos[a] = -0.2; pos[b] = -0.2; pos[i] = 1;
			glVertex3dv(pos);
		glEnd();
		glBegin(GL_TRIANGLE_FAN);
			glColor3f(col_2[0], col_2[1], col_2[2]);
			pos[a] = -0.2; pos[b] = -0.2; pos[i] = 0.975f;
			glVertex3dv(pos);
			pos[a] = 0.2; pos[b] = -0.2; pos[i] = 0.975f;
			glVertex3dv(pos);
			pos[a] = 0.2; pos[b] = 0.2; pos[i] = 0.975f;
			glVertex3dv(pos);
			pos[a] = -0.2; pos[b] = 0.2; pos[i] = 0.975f;
			glVertex3dv(pos);
		glEnd();
	}

	glPopMatrix(); //proj
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
	glLineWidth(1.0f);
	glDisable(GL_LINE_SMOOTH);
}
//---------------------------------------------------------------------
// Draw Backdrop
//---------------------------------------------------------------------
void Viewport::drawBackdrop()
{
	if (m_usegradient)
	{
		glClearColor(m_backcolor.r, m_backcolor.g, m_backcolor.b, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		gluOrtho2D(	0, 1.0, 0, 1.0);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glShadeModel(GL_SMOOTH);
			
			glBegin(GL_QUADS);
				glColor3f(m_backcolor.r, m_backcolor.g, m_backcolor.b);
				glVertex2i(0,1);
				glVertex2i(1,1);

				glColor3f(m_backcolor2.r, m_backcolor2.g, m_backcolor2.b);
				glVertex2i(1,0);
				glVertex2i(0,0);
			glEnd();

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glClearColor(m_backcolor.r, m_backcolor.g, m_backcolor.b, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}
//---------------------------------------------------------------------
// Draw Grid
//---------------------------------------------------------------------
void Viewport::drawGrid() const
{
	vector3 cen;
	float radius;
	mmatrix4d Mpw = pt::Project3D::project().registration().matrix();
	m_camera.getDataSphere(cen, radius);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslated(0,0,-Mpw(3,2));

	glEnable(GL_DEPTH_TEST);
	float spacer = 0.01f;

	//compute grid spacing
	if (radius == 0) radius = 20.0f;

	if (radius < 1.0f) 		radius = 1.0f;
	if (radius < 5.0f) 		radius = 5.0f;
	else if (radius < 10.0f) 	radius = 10.0f;
	else if (radius < 50.0f) 	radius = 50.0f;
	else if (radius < 100.0f)  	radius = 100.0f;
	else if (radius < 500.0f)  	radius = 500.0f;
	else if (radius < 1000.0f) 	radius = 1000.0f;
	else if (radius < 5000.0f) 	radius = 5000.0f;
	else if (radius < 10000.0f) 	radius = 10000.0f;
	else return;

	radius *= 0.5f;
	spacer = radius / 10.0f;

	glLineWidth(1.0f);
	glColor3f(m_gridcolor.r, m_gridcolor.g, m_gridcolor.b);

	glBegin(GL_LINES);
	float x, y;
	float ep = spacer * 0.01f;
	float stx = cen.x - fmod(cen.x, spacer);
	float sty = cen.y - fmod(cen.y, spacer);

	for (float i = -10; i< 11; i++)
	{
		x = stx + i*spacer;
		y = sty + i*spacer;

		float xw = x + Mpw(3,0);
		float yw = y + Mpw(3,1);

		if (fabs(xw) < ep)
		{
			//draw horiz
			glColor3f(0.4f,0.5f,0.4f);
			glVertex2f(x, sty - radius);
			//glVertex2f(x, -Mpw(3,1));
			//glVertex2f(x, spacer * 2 -Mpw(3,1));
			glVertex2f(x, sty + radius);
			glColor3f(m_gridcolor.r, m_gridcolor.g, m_gridcolor.b);
		}
		else
		{
			//draw horiz
			glVertex2f(x, sty - radius);
			glVertex2f(x, sty + radius);
		}
		if (fabs(yw) < ep)
		{
			//draw horiz
			glColor3f(0.5f,0.4f,0.4f);
			glVertex2f(stx - radius, y);
			//glVertex2f(-Mpw(3,0), y);
			//glVertex2f(spacer * 2-Mpw(3,0), y);
			glVertex2f(stx + radius, y);
			glColor3f(m_gridcolor.r, m_gridcolor.g, m_gridcolor.b);
		}
		else
		{
			//draw horiz
			glVertex2f(stx + radius, y);
			glVertex2f(stx - radius, y);
		}
	}
	glEnd();

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(1.5f);
	glBegin(GL_LINES);

	float offset = spacer * 0.005f;

	/* origin */ 
	glColor3f(0.3f,1.0f,0.3f);
	glVertex3f(-Mpw(3,0), -Mpw(3,1), offset);
	glVertex3f(-Mpw(3,0), spacer * 2 - Mpw(3,1), offset);

	glColor3f(1.0f,0.3f,0.3f);
	glVertex3f(-Mpw(3,0), -Mpw(3,1), offset);
	glVertex3f(spacer * 2-Mpw(3,0), -Mpw(3,1), offset);

	glColor3f(0.3f,0.3f,1.0f);
	glVertex3f(-Mpw(3,0), -Mpw(3,1), 0);
	glVertex3f(-Mpw(3,0), -Mpw(3,1), spacer * 2);

	glEnd();
	glPopMatrix();
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);
}
//---------------------------------------------------------------------
// color_gl , utility
//---------------------------------------------------------------------
void Viewport::color_gl(uint &col)
{
	glColor3ub(GetRValue(col), GetBValue(col), GetGValue(col));
}
//---------------------------------------------------------------------
// Draw Target
//---------------------------------------------------------------------
void Viewport::drawLightDirection() 
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 5.0f);

	vector3 angles = m_lights[0].directionAsEulers();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslated(0.0, 0.0, -3.0); 
	
	glPointSize(10.0f);
	glRotatef(angles.y, 0, 1.0f, 0);
	glRotatef(angles.z, 0, 0, 1.0f);

	glColor3f(1.0f,1.0f,0.5f);
	glBegin(GL_POINTS);
		glVertex3f(1.0f,0,0);
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(1.0f,0,0);
		glVertex3f(0,0,0);

		glVertex3f(0.1f,0.15f,0);
		glVertex3f(0,0,0);

		glVertex3f(0.1f,-0.15f,0);
		glVertex3f(0,0,0);
	glEnd();

	int size = m_viewRect.dx();
	if (m_viewRect.dy() < m_viewRect.dx())
		size = m_viewRect.dy();
	size -= 100;
	pushViewport();
	glViewport(	(m_viewRect.dx() - size) / 2.0,
				(m_viewRect.dy() - size) / 2.0,
				size, size);

	glLineWidth(1.0f);
	glShadeModel(GL_FLAT);

	Draw::lineSolid();
	Draw::fillNone();
	Draw::lineColor(vector3(1.0f,1.0f,1.0f));
	Draw::circle2(vector3(0,0,0), 1.0f, 1.0f);	

	glRotatef(90.0f,1,0,0);

	Draw::lineColor(vector3(1.0f,1.0f,1.0f));
	Draw::circle2(vector3(0,0,0), 1.0f,1.0f);	

	popViewport();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}
//---------------------------------------------------------------------
// Draw Target
//---------------------------------------------------------------------
void Viewport::drawTarget() const
{
	/*
	//must be in correct mode
	CRect r;
	getViewRect(r);

	CPoint c = r.CenterPoint();
	glPointSize(1.0f);
	glColor3f(1.0f,1.0f,1.0f);
	start2d();
		glBegin(GL_LINES);
			glVertex2i(c.x, c.y-5);
			glVertex2i(c.x, c.y+5);
			glVertex2i(c.x-4, c.y);
			glVertex2i(c.x+5, c.y);
		glEnd();
	end2d();

	m_camera.draw();*/
}
//---------------------------------------------------------------------
// Push Viewport - only 1 deep!!!
//---------------------------------------------------------------------
void Viewport::pushViewport()
{
	glPushAttrib( GL_TRANSFORM_BIT | GL_VIEWPORT_BIT );
}
//---------------------------------------------------------------------
// Pop Viewport
//---------------------------------------------------------------------
void Viewport::popViewport()
{
	glPopAttrib();
}
//---------------------------------------------------------------------
UINT Viewport::getLOD()
{
	if (m_op_state && m_mouse_state)
		return VP_LOD_NAV;
	else
		return VP_LOD_FULL;
}
//---------------------------------------------------------------------
void Viewport::setValid()
{
	m_plot = false;
}
//--------------------------------------------------------------------
// start 2d - must call end2d after done in this mode!!
//--------------------------------------------------------------------
void Viewport::start2d()
{
	if (!m_in2d)
	{
		glPushAttrib(GL_VIEWPORT_BIT);
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		glViewport(0, 0, m_viewRect.dx(), m_viewRect.dy());

		glDisable(GL_DEPTH_TEST);

		//setup otho2d
		//PROJECTION -------------------------------
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		int t = m_viewRect.dy();
		int b = 0;
		int l = 0;
		int r = m_viewRect.dx();

		gluOrtho2D(	l, r, b, t);
		//-------------------------------------------
		// MODELVIEW
		//clear any transforms
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
	}
	m_in2d = true;
}
//--------------------------------------------------------------------
// end 2d
//--------------------------------------------------------------------
void Viewport::end2d()
{
	if (m_in2d)
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		m_in2d = false;

		glPopAttrib();
	}
}
//---------------------------------------------------------------------
// Lock/unlock Viewport
//---------------------------------------------------------------------
void Viewport::lock_nav(UINT lock)
{
	m_view_lock = lock;
}
//---------------------------------------------------------------------
void Viewport::unlock_nav()
{
	m_view_lock = VP_NONE;
}
//---------------------------------------------------------------------
// navigation modes
//---------------------------------------------------------------------
void Viewport::nav_free()
{
	m_nav_mode = VP_FREE;
}
void Viewport::nav_rotate()
{
	m_nav_mode = VP_ROTATE;
}
void Viewport::nav_tumble()
{
	m_nav_mode = VP_TUMBLE;
}
void Viewport::nav_pan()
{
	m_nav_mode = VP_PAN;
}
void Viewport::nav_zoom()
{
	m_nav_mode = VP_ZOOM;
}
void Viewport::nav_end()
{
	m_nav_mode = VP_NONE;
}
void Viewport::setNavLock(int lock)
{
	if ((int)m_view_lock == lock) return;
	m_view_lock = lock;
	m_camera.resetControl();
	m_mouse_state = 0;
}
int Viewport::getNavLock() const
{
	return m_view_lock;
}

//---------------------------------------------------------------------
// Mouse Events
//---------------------------------------------------------------------
// MOUSE MOVE
//---------------------------------------------------------------------
bool Viewport::mouse_move(const Point &p, UINT flags)
{
	m_plot = false;
	m_op_state = VP_NONE;

	//locks
	if (m_view_lock == VP_FULL_LOCK) return false;
	if (m_view_lock == VP_HALF_LOCK && !(flags & VP_KEY_ALT)) return false;

	bool shift = flags & VP_KEY_SHIFT ? true : false;

	if (m_camera.getMode() == Camera::LIGHT)
	{
		m_camera.setLight(&m_lights[0]);
	}

	switch(m_nav_mode)
	{
	case VP_FREE:
		{
		bool nav = false;

		for (int i = 1; i<4; i++)
		{
			if (m_op_map[i] == m_mouse_state)
			{
				m_op_state = i;

				switch (i)
				{
				case VP_PANNING: m_camera.setNavigation(Camera::PAN); break;
				case VP_ZOOMING: m_camera.setNavigation(Camera::ZOOM); break;
				case VP_ROTATING: m_camera.setNavigation(Camera::TUMBLE); break;
				default:  m_camera.setNavigation(Camera::NONE); break;
				}
				
				nav = true;
				m_plot = true;
				break;
			}
		}
		if (!nav) m_camera.setNavigation(Camera::NONE);
		}
		break;

	case VP_PAN:
		if (m_mouse_state == VP_MOUSE_LEFT) {
			m_camera.setNavigation(Camera::PAN);
			m_plot = true;
		}
		break;

	case VP_ZOOM:
		if (m_mouse_state == VP_MOUSE_LEFT) {
			m_camera.setNavigation(Camera::ZOOM);
			m_plot = true;
		}
		break;

	case VP_TUMBLE:
		if (m_mouse_state == VP_MOUSE_LEFT) {
			m_camera.setNavigation(Camera::TUMBLE);
			m_plot = true;
		}
		break;

	case VP_ROTATE:
		if (m_mouse_state == VP_MOUSE_LEFT) {
			m_camera.setNavigation(Camera::TUMBLE);
			m_plot = true;
		}

		break;

	case VP_NONE:
		if (m_mouse_state == VP_MOUSE_MIDDLE)
		{
			m_camera.setNavigation(Camera::PAN);
			m_plot = true;
		}
		else
			m_camera.setNavigation(Camera::NONE);
		break;
	}

	if (m_camera.moveControl(p.x, p.y))
	{
		m_LOD = VP_LOD_NAV;
		alignLayers();
		if (m_plot) paint();
	}
	return m_plot;
}
//---------------------------------------------------------------------
// LEFT UP
//---------------------------------------------------------------------
bool Viewport::mouse_left_up(const Point &p, UINT flags)
{
	m_mouse_state &= ~VP_MOUSE_LEFT;
	release_mouse(VP_MOUSE_LEFT);

	return m_plot;
}
//---------------------------------------------------------------------
// LEFT DOWN
//---------------------------------------------------------------------
bool Viewport::mouse_left_down(const Point &p, UINT flags)
{

	//locks
	if (m_view_lock == VP_FULL_LOCK) return false;
	if (m_view_lock == VP_HALF_LOCK && !(flags & VP_KEY_ALT)) return false;

	m_mouse_state |= VP_MOUSE_LEFT;

	return m_plot;
}
//---------------------------------------------------------------------
// RIGHT UP
//---------------------------------------------------------------------
bool Viewport::mouse_right_up(const Point &p, UINT flags)
{
	m_mouse_state &= ~VP_MOUSE_RIGHT;
	release_mouse(VP_MOUSE_RIGHT);

	return m_plot;
}
//---------------------------------------------------------------------
// RIGHT DOWN
//---------------------------------------------------------------------
bool Viewport::mouse_right_down(const Point &p, UINT flags)
{
	//locks
	if (m_view_lock == VP_FULL_LOCK) return false;
	if (m_view_lock == VP_HALF_LOCK && !(flags & VP_KEY_ALT)) return false;

	m_mouse_state |= VP_MOUSE_RIGHT;

	return m_plot;
}
//---------------------------------------------------------------------
// MIDDLE UP
//---------------------------------------------------------------------
bool Viewport::mouse_mid_up(const Point &p, UINT flags)
{
	m_mouse_state &= ~VP_MOUSE_MIDDLE;
	release_mouse(VP_MOUSE_MIDDLE);

	return m_plot;
}
//---------------------------------------------------------------------
// MIDDLE DOWN
//---------------------------------------------------------------------
bool Viewport::mouse_mid_down(const Point &p, UINT flags)
{
	//locks
	if (m_view_lock == VP_FULL_LOCK) return false;
	if (m_view_lock == VP_HALF_LOCK && !(flags & VP_KEY_ALT)) return false;

	m_mouse_state |= VP_MOUSE_MIDDLE;
	return m_plot;
}
//---------------------------------------------------------------------
// ROLL UP
//---------------------------------------------------------------------
bool Viewport::mouse_roll_up(const Point &p, UINT flags)
{
	zoomIn();

	return true;
}
//---------------------------------------------------------------------
// ROLL DOWN
//---------------------------------------------------------------------
bool Viewport::mouse_roll_down(const Point &p, UINT flags)
{
	zoomOut();

	return true;
}
//---------------------------------------------------------------------
// LEFT DOUBLE CLICK
//---------------------------------------------------------------------
bool Viewport::mouse_left_dbl(const Point &p, UINT flags)
{
	return false;
}
//---------------------------------------------------------------------
// mouse release - controls ending of mouse up events
//---------------------------------------------------------------------
void Viewport::release_mouse(UINT release_op)
{
	switch(m_nav_mode)
	{

	case VP_FREE:
		{
		bool nav = false;

		for (int i = 1; i<4; i++)
		{
			if (m_op_map[i] == m_mouse_state)
			{
				nav = true;
				break;
			}
		}
		if (!nav) m_camera.setNavigation(Camera::NONE);
		}
		break;

	case VP_PAN:
		if (release_op == VP_MOUSE_LEFT)
			m_camera.setNavigation(Camera::NONE);
		break;
	case VP_ZOOM:
		if (release_op == VP_MOUSE_LEFT)
			m_camera.setNavigation(Camera::NONE);
		break;
	case VP_ROTATE:
		if (release_op == VP_MOUSE_LEFT)
			m_camera.setNavigation(Camera::NONE);
		break;

	case VP_NONE:
		/*check for pan*/
		if (m_camera.getNavigationState() == Camera::PAN)
		{
			m_camera.setNavigation(Camera::NONE);
			paint();
		}
		else
			m_camera.setNavigation(Camera::NONE);
		break;
	}
	if (m_nav_mode != VP_NONE && m_plot) paint();
	/*m_plot indicates some movement was done to qualify another plot
	 and is not negated by non-full lod plots*/
	m_plot = false;
}
//---------------------------------------------------------------------
// Projection
//---------------------------------------------------------------------
void Viewport::useOrtho()
{
	invalidateView();

	m_camera.setOrtho(true);

	m_camera.update();
	alignLayers();

	m_LOD = VP_LOD_FULL;
	paint();
}
//---------------------------------------------------------------------
void Viewport::usePerspective()
{
	invalidateView();

	m_camera.setOrtho(false);
	m_camera.update();
	alignLayers();

	m_LOD = VP_LOD_FULL;
	paint();
}
//---------------------------------------------------------------------
// ZOOM FUNCTIONS
//---------------------------------------------------------------------
void Viewport::zoomTo(const BoundingBox *bb, bool pnt)
{
	invalidateView();

	vector3 l(bb->lx(), bb->ly(), bb->lz());
	vector3 u(bb->ux(), bb->uy(), bb->uz());
	vector3 c;
	bb->getCenter(c);

	m_camera.zoomTo(bb, 15);
	if (pnt) paint();
}
//----------------------------------------------------------------------
void Viewport::zoomIn()
{
	invalidateView();

	m_camera.zoomStatic(-0.25f);
	alignLayers();
	m_LOD = VP_LOD_FULL;
	paint();
}
//----------------------------------------------------------------------
void Viewport::zoomOut()
{
	invalidateView();

	m_camera.zoomStatic(0.25f);
	alignLayers();
	m_LOD = VP_LOD_FULL;
	paint();
}
//----------------------------------------------------------------------
void Viewport::zoomWindow(const Recti &win, bool pnt)
{
	Rectf b;
	b.lx() = win.lx();
	b.ux() = win.ux();
	b.ly() = win.ly();
	b.uy() = win.uy();
	zoomWindow(b, pnt);
}
//----------------------------------------------------------------------
void Viewport::zoomWindow(const Rectf &win, bool pnt)
{
	invalidateView();

	m_camera.update();
	Rectd r = win.cast(double());

	m_camera.zoomWindow(r);

	m_plot = true;
	m_LOD = VP_LOD_FULL;
	if (pnt) paint();
}
//
// Left View
//
void Viewport::zoomLeft()
{
	invalidateView();

	double rot[3] = { 0,0,0 };;

	switch (m_camera.getConstraintAxis())
	{
	case 0:	break;
	case 1:	m_camera.rotationFromEulerZYX(rot[2], rot[1] + _PI/2.0, rot[0]);	break;
	case 2:	m_camera.rotationFromEulerZYX(3 * _PI/2.0 - rot[2], rot[1], rot[0] + _PI/2.0);	break;
	}

	m_LOD = VP_LOD_FULL;
	paint();
}
//
// Right View
//
void Viewport::zoomRight()
{
	double rot[3] = { 0,0,0 };

	switch (m_camera.getConstraintAxis())
	{
	case 0:	break;
	case 1:	m_camera.rotationFromEulerZYX(rot[2], rot[1] - _PI/2.0, rot[0]);	break;
	case 2:	m_camera.rotationFromEulerZYX(_PI/2.0 - rot[2], rot[1], rot[0] + _PI/2.0);	break;
	}

	m_LOD = VP_LOD_FULL;
	paint();
}
//
// Front View
//
void Viewport::zoomFront()
{
	double rot[3] = { 0,0,0 };

	switch (m_camera.getConstraintAxis())
	{
	case 0:	break;
	case 1:	m_camera.rotationFromEulerZYX(rot[2], rot[1], rot[0]);	break;
	case 2:	m_camera.rotationFromEulerZYX(-rot[2], rot[1], rot[0] + _PI/2.0);	break;
	}

	m_LOD = VP_LOD_FULL;
	paint();
}
//
// Back View
//
void Viewport::zoomBack()
{
	double rot[3] = { 0,0,0 };

	switch (m_camera.getConstraintAxis())
	{
	case 0:	break;
	case 1:	m_camera.rotationFromEulerZYX(rot[2], rot[1] + _PI, rot[0]);	break;
	case 2:	m_camera.rotationFromEulerZYX(-rot[2]-_PI, rot[1], rot[0]+_PI/2.0);	break;
	}

	m_LOD = VP_LOD_FULL;
	paint();
}
//
// Top View
//
void Viewport::zoomTop()
{
	double rot[3] = { 0,0,0 };

	switch (m_camera.getConstraintAxis())
	{
	case 0:	break;
	case 1:	m_camera.rotationFromEulerZYX(rot[2], rot[1], rot[0] - _PI/2.0); break;
	case 2:	m_camera.rotationFromEulerZYX(-rot[2], rot[1], rot[0]);		break;
	}

	m_LOD = VP_LOD_FULL;
	paint();
}
//
// Bottom View
//
void Viewport::zoomBottom()
{
	double rot[3] = { 0,0,0 };

	switch (m_camera.getConstraintAxis())
	{
	case 0:	break;
	case 1:	m_camera.rotationFromEulerZYX(rot[2], rot[1], rot[0] + _PI/2.0);	break;
	case 2:	m_camera.rotationFromEulerZYX(-rot[2], rot[1], rot[0] - _PI);	break;
	}

	m_LOD = VP_LOD_FULL;
	paint();
}
//
// Default View
//
void Viewport::zoomDefault()
{
	m_camera.rotationFromEulerZYX(0, 0, 0);

	m_LOD = VP_LOD_FULL;
	paint();
}
//
//void Viewport::SetUCSview(UINT view)
//{
/*	UCS *u = ViewAlgorithms::GetUCS();

	double deg = 180.0/_PI;
	double hpi = _PI/2.0;

	double za[3] = {0,0,1.0};
	double za2[3];

	u->inv_rotate(za);

	matrix4d rot_ax = matrix4d::identity();

	switch (view)
	{
	case VP_VIEW_BACK:
		rot_ax = matrix4d::rotation(0, -_PI, 0);
		break;
	case VP_VIEW_RIGHT:
		rot_ax = matrix4d::rotation(0, -hpi, 0);
		break;
	case VP_VIEW_LEFT:
		rot_ax = matrix4d::rotation(0, hpi, 0);
		break;
	}
	rot_ax.vec3_multiply_mat4(za, za2);

	vector3d z(za2[0], za2[1], za2[2]);

	matrix3d mat = matrix3d::lookat(z, 0);
	matrix4d M = matrix4d::identity();

	for (int i=0; i<3; i++)
		for (int j=0; j<3; j++)
			M(i,j) = mat(i,j);

	double tr[16];
	M.untransform(tr);

	//tweek the y
	switch (view)
	{
	case VP_VIEW_TOP:
		tr[U_ROTATEX] -= hpi;
		break;
	case VP_VIEW_BOTTOM:
		tr[U_ROTATEX] += hpi;
		break;
	}
	std::cout << "hpi = " << hpi << " deg = " << deg << std::endl;

	m_v3d.dAngle[0] = -tr[U_ROTATEX] * deg;
	m_v3d.dAngle[1] = -tr[U_ROTATEY] * deg;
	m_v3d.dAngle[2] = -tr[U_ROTATEZ] * deg;

	m_LOD = VP_LOD_FULL;
	paint();*/
//}
//
void Viewport::alignLayers()
{
	/*
	//align 2d layer to 3d layer
	m_v2d.dZoom = m_camera.getZoomFactor();

	if (!CViewportMan::updateViewsetup())
	{
		std::cout << "layer alignment failed" << std::endl;
		return;
	}
	CViewportMan::setup3d();

	//get screen projections
	DP3D_POINT an3d(0,0,0);
	DP3D_POINT sc3d;
	CViewportMan::project3d(an3d, sc3d);
	m_v2d.dPan[0] = -(sc3d[0] - (double)m_viewRect.CenterPoint().x) * m_v2d.dZoom;
	m_v2d.dPan[1] = -(sc3d[1] - (double)m_viewRect.CenterPoint().y) * m_v2d.dZoom;
	*/
}
void Viewport::rotationFromAxisAngle(float angle, float *v)
{
	m_camera.rotationFromAxisAngle(angle, v);
}
void Viewport::rotationFromMatix(const float *mat)
{
	m_camera.rotationFromMatrix(mat);
}
void Viewport::rotationFromLookAt(const float *loc, const float *up)
{
	m_camera.rotationFromLookAt(loc, up);
}
void Viewport::rotationFromEulerZYX(float z, float y, float x)
{
	m_camera.rotationFromEulerZYX(z, y, x);
}
bool Viewport::popClipFrustum()
{
	return m_camera.popCullingFrustum();
}
void Viewport::pushClipFrustum()
{
	m_camera.pushCullingFrustum();
}
bool Viewport::inFrustum(const BoundingBox *b) const
{
	return m_camera.cullFrustum().inFrustum(b);
}
bool Viewport::inFrustum(const float *v) const
{
	pt::vector3 vf(v[0], v[1], v[2]);
	return m_camera.cullFrustum().inFrustum(&vf);
}
void Viewport::setBlitMode(int blit)
{
	m_blitmode = blit;
}
void Viewport::storeViewPixels()
{
	std::cout << "storeViewPixels" << std::endl;
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

	glReadPixels(0,0,camera().getViewportRight(), camera().getViewportBottom(), GL_RGB, GL_UNSIGNED_BYTE, m_colorPixels);
}
void Viewport::restoreViewPixels()
{
	std::cout << "restoreViewPixels" << std::endl;

	glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_LOGIC_OP);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);

    glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
    glPixelTransferi(GL_RED_SCALE, 1);
    glPixelTransferi(GL_RED_BIAS, 0);
    glPixelTransferi(GL_GREEN_SCALE, 1);
    glPixelTransferi(GL_GREEN_BIAS, 0);
    glPixelTransferi(GL_BLUE_SCALE, 1);
    glPixelTransferi(GL_BLUE_BIAS, 0);
    glPixelTransferi(GL_ALPHA_SCALE, 1);
    glPixelTransferi(GL_ALPHA_BIAS, 0);

	glRasterPos2i(0,0);
	glDrawPixels(camera().getViewportRight(), camera().getViewportBottom(), GL_RGB, GL_COLOR, m_colorPixels);

	glEnable(GL_DEPTH_TEST);
}
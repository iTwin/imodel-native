/*--------------------------------------------------------------------------*/ 
/*	Pointools Camera class implmentation and implementation					*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#include "PointoolsVortexAPIInternal.h"


#include <GL/gl.h>
#include <GL/glu.h>

#include <wildmagic/math/Wm5Matrix3.h>
#include <wildmagic/math/Wm5Quaternion.h>
#include <math/matrix_math.h>
#include <ptgl/glCamera.h>

#include <math/geom.h>
#include <pt/Rect.h>
#include <pt/trace.h>

#include <pt/datatree.h>

using namespace pt;
using namespace ptgl;
 
#define MAX_FOV 85
#define DEFAULT_WIDTH 700
#define DEFAULT_HEIGHT 600

//----------------------------------------------------------
// Construction / Destruction
//----------------------------------------------------------
Camera::Camera()
{
	m_basefov = 60;
	m_fov = m_basefov;
	m_minfov = 1;
	m_zoomthreshold = 1.5f;

	/*target*/ 
	m_target.set(0,0,-5.0f);
	m_center.set(0,0,0);
	m_radius = 50.0f;

	m_dist2target = 1.0f;
	m_actlocation.x = 0;
	m_actlocation.y = 0;
	m_actlocation.z = -m_dist2target;

	m_zoom = 1.0;

	/*location*/ 
    m_frustumN = 0.5f;
    m_frustumF = 500.0f;

    m_frustumL = -25.0f;
    m_frustumR = 25.0f;
    m_frustumT = 25.0f;
    m_frustumB = -25.0f;

    m_viewportL = 0.0f;
    m_viewportR = 1.0f;
    m_viewportT = 1.0f;
    m_viewportB = 0.0f;

	m_model_needs_update = true;
	m_frustum_needs_update = true;
	m_vport_needs_update = true;

	m_ortho = false;

	m_mouseX = 0;
	m_mouseY = 0;
	m_lastX = 0;
	m_lastY = 0;

	m_angleX = PI;
	m_angleY = PI;
	m_angleZ = 0;

	m_nav_state = NONE;
	m_nav_mode = INSPECT;

	m_width = DEFAULT_WIDTH;
	m_height = DEFAULT_HEIGHT;

	m_actlocation.set(0,0,-10);
	setConstraintAxis(2);
	setUseConstraints(true);

	m_light = 0;

	m_from_vstore = false;

	rotationFromEulerZYX(m_angleZ, m_angleY, m_angleX);
}
void Camera::reset()
{
	m_fov = 60;
	m_basefov = 60;
	m_minfov = 1;
	m_zoomthreshold = 1.5f;

	/*target*/ 
	m_target.set(0,0,-5.0f);
	m_center.set(0,0,0);
	m_radius = 50.0f;

	m_dist2target = 1.0f;
	m_actlocation.x = 0;
	m_actlocation.y = 0;
	m_actlocation.z = -m_dist2target;

	m_zoom = 1.0;

	/*location*/ 
    m_frustumN = 0.5f;
    m_frustumF = 500.0f;

    m_frustumL = -25.0f;
    m_frustumR = 25.0f;
    m_frustumT = 25.0f;
    m_frustumB = -25.0f;

    m_viewportL = 0.0f;
    m_viewportR = 1.0f;
    m_viewportT = 1.0f;
    m_viewportB = 0.0f;

	m_model_needs_update = true;
	m_frustum_needs_update = true;
	m_vport_needs_update = true;

	m_mouseX = 0;
	m_mouseY = 0;
	m_lastX = 0;
	m_lastY = 0;

	m_angleX = PI;
	m_angleY = PI;
	m_angleZ = 0;

	m_nav_state = NONE;
	m_actlocation.set(0,0,-10);

	m_from_vstore = false;

	rotationFromEulerZYX(m_angleZ, m_angleY, m_angleX);
}
//
Camera::Camera(const Camera &c)
{
	(*this) = c;
}
void Camera::setFromViewstore(const Viewstore &v)
{
	m_vstore = v;
	m_from_vstore = true;
}
//-------------------------------------------------------------------
// construct from data tree branch
//-------------------------------------------------------------------
Camera::Camera(const pt::datatree::Branch *b)
{
	m_light = 0;

	b->getNode("angle_X", m_angleX);
	b->getNode("angle_Y", m_angleY);
	b->getNode("angle_Z", m_angleZ);

	b->getNode("frustum_N", m_frustumN);
	b->getNode("frustum_F", m_frustumF);
	b->getNode("frustum_L", m_frustumL);
	b->getNode("frustum_R", m_frustumR);
	b->getNode("frustum_T", m_frustumT);
	b->getNode("frustum_B", m_frustumB);

	b->getNode("min_FOV", m_minfov);
	b->getNode("base_FOV", m_basefov);
	b->getNode("current_FOV", m_fov);

	b->getNode("ortho", m_ortho);
	b->getNode("radius", m_radius);
	b->getNode("center", m_center);
	
	b->getNode("dist_2_target", m_dist2target);
	
	b->getNode("target", m_target);
	b->getNode("actual_location", m_actlocation);
	b->getNode("viewport_width", m_width);
	b->getNode("viewport_height", m_height);
	b->getNode("up vector", m_up);
	b->getNode("zoom", m_zoom);
	b->getNode("zoom_threshold", m_zoomthreshold);
	
	m_viewportL = 0.0f;
	m_viewportR = 1.0f;
	m_viewportT = 1.0f;
	m_viewportB = 0.0f;

	pt::datatree::Branch *arcb = b->getBranch("Arcball");
	m_arcball.readBranch(arcb);

	setConstraintAxis(2);
	m_constrained_up_axis = 2;
	m_constrained = true;

	rotationFromEulerZYX(m_angleZ, m_angleY, m_angleX);

	m_model_needs_update = true;
	m_frustum_needs_update = true;
	m_vport_needs_update = true;

	m_light = 0;

	m_nav_mode = INSPECT;
	m_nav_state = NONE;

	m_from_vstore = false;

	update(false);
}
//
Camera::~Camera()
{}
//
const Camera &Camera::operator = (const Camera &cam)
{
	m_fov = cam.m_fov;
	m_basefov = cam.m_basefov;
	m_minfov = cam.m_minfov;
	m_zoomthreshold = cam.m_zoomthreshold;
	m_up = cam.m_up;

	/*target*/ 
	m_target.x = cam.m_target.x;
	m_target.y = cam.m_target.y;
	m_target.z = cam.m_target.z;

	m_center = cam.m_target;
	m_radius = cam.m_radius;

	m_dist2target = cam.m_dist2target;
	m_actlocation.x = cam.m_actlocation.x;
	m_actlocation.y = cam.m_actlocation.y;
	m_actlocation.z = cam.m_actlocation.z;

	m_zoom = cam.m_zoom;

	/*location*/ 
    m_frustumN = cam.m_frustumN;
    m_frustumF = cam.m_frustumF;

    m_frustumL = cam.m_frustumL;
    m_frustumR = cam.m_frustumR;
    m_frustumT = cam.m_frustumT;
    m_frustumB = cam.m_frustumB;

    m_viewportL = cam.m_viewportL;
    m_viewportR = cam.m_viewportR;
    m_viewportT = cam.m_viewportT;
    m_viewportB = cam.m_viewportB;

	m_model_needs_update = true;
	m_frustum_needs_update = true;
	m_vport_needs_update = true;

	m_ortho = cam.m_ortho;

	m_mouseX = cam.m_mouseX;
	m_mouseY = cam.m_mouseY;
	m_lastX = cam.m_lastX;
	m_lastY = cam.m_lastY;

	setConstraintAxis(cam.getConstraintAxis());

	m_angleX = cam.m_angleX;
	m_angleY = cam.m_angleY;
	m_angleZ = cam.m_angleZ;

	m_nav_state = NONE;
	m_nav_mode = cam.m_nav_mode;

	m_constrained_up_axis = cam.m_constrained_up_axis;
	m_constrained = cam.m_constrained;

	if (cam.m_width != DEFAULT_WIDTH && cam.m_height != DEFAULT_HEIGHT)
	{
		m_width = cam.m_width;
		m_height = cam.m_height;
	}

	clearCullStack();
	m_arcball = cam.m_arcball;

	if (cam.m_light)
		m_light = cam.m_light;

	//update(false);
	m_from_vstore = false;

	return (*this);
}
/*used for animation of camera*/
const Camera &Camera::operator /= (const float &v)
{
	m_fov /= v;

	/*target*/ 
	m_target.x /= v;
	m_target.y /= v;
	m_target.z /= v;

	m_actlocation.x /= v;
	m_actlocation.y /= v;
	m_actlocation.z /= v;

	m_zoom /= v;

	/*location*/ 
    m_frustumN /= v;
    m_frustumF /= v;

    m_frustumL /= v;
    m_frustumR /= v;
    m_frustumT /= v;
    m_frustumB /= v;

	m_model_needs_update = true;
	m_frustum_needs_update = true;

	m_arcball /= v;
	resolveEulers();

	clearCullStack();
	m_from_vstore = false;

	update(false);

	return (*this);
}
const Camera &Camera::operator *= (const float &v)
{
	m_fov *= v;

	/*target*/ 
	m_target.x *= v;
	m_target.y *= v;
	m_target.z *= v;

	m_actlocation.x *= v;
	m_actlocation.y *= v;
	m_actlocation.z *= v;

	m_zoom *= v;

	/*location*/ 
    m_frustumN *= v;
    m_frustumF *= v;

    m_frustumL *= v;
    m_frustumR *= v;
    m_frustumT *= v;
    m_frustumB *= v;

	m_model_needs_update = true;
	m_frustum_needs_update = true;

	m_arcball *= v;
	resolveEulers();


	clearCullStack();
	m_from_vstore = false;

	update(false);
	return (*this);
}
const Camera &Camera::operator += (const Camera &cam)
{
	m_fov += cam.m_fov;

	/*target*/ 
	m_target.x += cam.m_target.x;
	m_target.y += cam.m_target.y;
	m_target.z += cam.m_target.z;

	m_actlocation.x += cam.m_actlocation.x;
	m_actlocation.y += cam.m_actlocation.y;
	m_actlocation.z += cam.m_actlocation.z;

	m_zoom += cam.m_zoom;

	/*location*/ 
    m_frustumN += cam.m_frustumN;
    m_frustumF += cam.m_frustumF;

    m_frustumL += cam.m_frustumL;
    m_frustumR += cam.m_frustumR;
    m_frustumT += cam.m_frustumT;
    m_frustumB += cam.m_frustumB;

	m_model_needs_update = true;
	m_frustum_needs_update = true;

	m_arcball += cam.m_arcball;
	resolveEulers();

	clearCullStack();
	m_from_vstore = false;

	update(false);
	return (*this);
}
const Camera &Camera::operator -= (const Camera &cam)
{
	m_fov -= cam.m_fov;
	m_zoom -= cam.m_zoom;

	/*target*/ 
	m_target.x -= cam.m_target.x;
	m_target.y -= cam.m_target.y;
	m_target.z -= cam.m_target.z;

	/*location*/ 
    m_frustumN -= cam.m_frustumN;
    m_frustumF -= cam.m_frustumF;
    m_frustumL -= cam.m_frustumL;
    m_frustumR -= cam.m_frustumR;
    m_frustumT -= cam.m_frustumT;
    m_frustumB -= cam.m_frustumB;

	m_arcball -= cam.m_arcball;
	resolveEulers();

	m_model_needs_update = true;
	m_frustum_needs_update = true;

	clearCullStack();
	m_from_vstore = false;

	update(false);
	return (*this);
}
// draws a camera widget
void Camera::draw()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	m_vstore.restore_modelview();
	glPopMatrix();

	glBegin(GL_LINE_STRIP);
		glVertex3f(0,0,0);
		glVertex3f(0.5f,0,0);
		glVertex3f(0.5f,0.8f,0);
		glVertex3f(0,0.5f,0);
		glVertex3f(0,0,0);
	glEnd();
	glBegin(GL_LINE_STRIP);
		glVertex3f(0,0,0.5f);
		glVertex3f(0.5f,0,0.5f);
		glVertex3f(0.5f,0.8f,0.5f);
		glVertex3f(0,0.5f,0.5f);
		glVertex3f(0,0,0.5f);
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 0.5f);

		glVertex3f(0.5f, 0, 0);
		glVertex3f(0.5f, 0, 0.5f);

		glVertex3f(0, 0.8f, 0);
		glVertex3f(0, 0.8f, 0.5f);

		glVertex3f(0, 0.8f, 0);
		glVertex3f(0, 0.8f, 0.5f);
	
		glVertex3f(0.25f, 0.8f, 0.25f);	
		glVertex3f(0.25f, 1.3f, 0.25f);	
	glEnd();
}
//----------------------------------------------------------
// Update GL matrix/viewport state
//----------------------------------------------------------
bool Camera::update(bool use_flag)
{
	if (m_from_vstore)
	{
		m_vstore.restore();
		m_from_vstore = false;
		return true;
	}
	bool update_cull = m_frustum_needs_update || m_model_needs_update;
	bool update = update_cull | m_vport_needs_update;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (m_light) m_light->setupGL();

	if (!updateViewport(use_flag)) m_vstore.restore_viewport();
	if (!updateFrustum(use_flag)) m_vstore.restore_projection();
	if (!updateTransform(use_flag)) m_vstore.restore_modelview();

	if (!use_flag || update_cull)
	{
		updateCullingFrustum();
	}
	m_vstore.store();
	/* get camera position*/ 
	mmatrix4d M;
	memcpy(&M, &m_vstore.model_mat, sizeof(double)*16);
	M.transpose();
	M.invert();

	vector4d pos(0,0,0,0);
	vector4d loc, tloc;
	vector4d tpos(0,0,-m_dist2target,0);

	M.vec3_multiply_mat4(pos, loc);
	M.vec3_multiply_mat4(tpos, tloc);
	
	m_actlocation.x = loc.x;
	m_actlocation.y = loc.y;
	m_actlocation.z = loc.z;

	if (m_nav_mode == WALK)
	{
		m_target.x = tloc.x;
		m_target.y = tloc.y;
		m_target.z = tloc.z;
	}
	//if (m_light) m_light->setupGL();

//	printf("camera at (%f, %f, %f)\n", loc[0], loc[1], loc[2]);
 	return update;
}
//----------------------------------------------------------
// Update GL matrix/viewport state
//----------------------------------------------------------
void Camera::updateCullingFrustum()
{
	clearCullStack();
	pushCullingFrustum();
}
//----------------------------------------------------------
// Update GL frustum
//----------------------------------------------------------
bool Camera::updateFrustum(bool use_flag)
{
	if (!use_flag || m_frustum_needs_update)
	{
		if (m_ortho)
		{
			/* set projection matrix*/ 
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(m_frustumL,m_frustumR,m_frustumB,m_frustumT,m_frustumN, m_frustumF);
			if ( m_height >= m_width )
				glScalef(1,(float)m_width/m_height, 1);
			else
				glScalef((float)m_height/m_width, 1, 1);
		}
		else
		{
			/* set projection matrix*/ 
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			/*check aspect and correct*/ 
			setFrustum(m_frustumN, m_frustumF, m_frustumL, m_frustumR, m_frustumT, m_frustumB);
			glFrustum(m_frustumL,m_frustumR,m_frustumB,m_frustumT,m_frustumN, m_frustumF);

			if ( m_height >= m_width )
				glScalef(1,(float)m_width/m_height, 1);
			else
				glScalef((float)m_height/m_width, 1, 1);

		}
		m_frustum_needs_update = false;
		return true;
	}
	return false;
}
//
double Camera::getZoomFactor() const
{
	if ( m_height >= m_width )
		return fabs(m_frustumR - m_frustumL) / m_width;
	else return fabs(m_frustumT - m_frustumB) / m_height;
}
void Camera::getRotation(float &x, float &y, float &z) const
{
	x = m_angleX;
	y = m_angleY;
	z = m_angleZ;
}
void Camera::pushGLrotation() const
{
	m_arcball.IssueGLrotation();
}
void Camera::getFrustum(float &n, float &f, float &l, float &r, float &t, float &b) const
{
	n = m_frustumN;
	f = m_frustumF;
	l = m_frustumL;
	r = m_frustumR;
	t = m_frustumT;
	b = m_frustumB;
}
void Camera::getFrustum(double &n, double &f, double &l, double &r, double &t, double &b) const
{
	n = m_frustumN;
	f = m_frustumF;
	l = m_frustumL;
	r = m_frustumR;
	t = m_frustumT;
	b = m_frustumB;
}
//----------------------------------------------------------
// Update GL model view matrix
//----------------------------------------------------------
bool Camera::updateTransform(bool use_flag, bool identity)
{
    if (!use_flag || m_model_needs_update)
	{
		/* set view matrix*/ 
		glMatrixMode(GL_MODELVIEW);
		if (identity) glLoadIdentity();

		gl_tmatrix m = m_arcball.GetMatrix();
		
		/*copy matrix*/ 
		mmatrix3 M;	for (int i=0; i<3; i++)	for (int j=0; j<3; j++)	M(i,j) = m(i,j);
		M.transpose();
		
		/*gl matrix*/ 
		if (m_nav_mode == WALK)
		{
			m_arcball.IssueGLrotation();
			glTranslatef(-m_actlocation.x, -m_actlocation.y, -m_actlocation.z);
		}
		else
		{
			glTranslatef(0,0, -m_dist2target);
			m_arcball.IssueGLrotation();
			glTranslatef(-m_target.x, -m_target.y, -m_target.z);
		}

		/*up vector		*/ 
		m_up = vector3(M.extractYVector().data());

		if (!m_constrained) resolveEulers();
		m_model_needs_update = false;
		return true;
	}
	return false;
}
//----------------------------------------------------------
// Update GL model view matrix
//----------------------------------------------------------
void Camera::resetArcball()
{
	return;
}
//----------------------------------------------------------
// checks if angles is on axis
//----------------------------------------------------------
bool Camera::onAxis()
{
	static double hpi = PI/2.0;

	float x = fabs(m_angleX) < 0.01f ? m_angleX : fmod(hpi, (double)m_angleX);
	float y = fabs(m_angleY) < 0.01f ? m_angleY : fmod(hpi, (double)m_angleY);
	float z = fabs(m_angleZ) < 0.01f ? m_angleZ : fmod(hpi, (double)m_angleZ);

	bool onaxis =   (x < 0.01f || fabs(x - hpi) < 0.01f) &&
					(y < 0.01f || fabs(y - hpi) < 0.01f) &&
					(z < 0.01f || fabs(z - hpi) < 0.01f) ? true : false;	

	return onaxis;
}	

//----------------------------------------------------------
// Update GL viewport
//----------------------------------------------------------
bool Camera::updateViewport(bool use_flag)
{
    if (!use_flag || m_vport_needs_update)
	{
		/* set view port*/
		GLint iX = m_viewportL*m_width;
		GLint iY = m_viewportB*m_height;
		GLsizei iW = (GLsizei)((m_viewportR - m_viewportL)*m_width);
		GLsizei iH = (GLsizei)((m_viewportT - m_viewportB)*m_height);
		glViewport(iX,iY,iW,iH);
		m_vport_needs_update = false;
		return true;
	}
	return false;
}
//----------------------------------------------------------
// rotate in radians
//----------------------------------------------------------
void Camera::resetRotation()
{
	m_arcball.ResetRotations();
	m_model_needs_update = true;
}
//----------------------------------------------------------
// clear frustum stack
//----------------------------------------------------------
void Camera::clearCullStack()
{
	while (m_cull_stack.size()) m_cull_stack.pop();
}
//----------------------------------------------------------
// pop culling frustum - can't pop the world frustum
//----------------------------------------------------------
bool Camera::popCullingFrustum()
{
	if (m_cull_stack.size() > 1)
	{
		m_cull_stack.pop();
		return true;
	}
	return false;
}
//----------------------------------------------------------
// pop culling frustum - can't pop the world frustum
//----------------------------------------------------------
void Camera::pushCullingFrustum()
{
	Frustum F;
	F.buildFrustum();
	m_cull_stack.push(F);
}
//----------------------------------------------------------
// viewport size
//----------------------------------------------------------
void Camera::onResize(int width, int height)
{
	m_height = height;
	m_width = width;

	m_arcball.ClientAreaResize(width,height);

	/*resizeing seems to shift rotations - this is a fix*/
	rotationFromEulerZYX(m_angleZ, m_angleY, m_angleX);

	m_vport_needs_update = true;
	m_frustum_needs_update = true;
}
//----------------------------------------------------------
// zoom by factor
//----------------------------------------------------------
void Camera::zoomStatic(float factor)
{
	if (fabs(factor) < 0.001) return;

	/*balance zoom by camera movement and frustum expansion*/ 
	/*to maximize viewable distance*/ 

	/*calculate data extents in tranform space*/ 
	vector3 cen;

	vector3 dcen = m_center - m_target;
	float distcentar = dcen.length();
	float rad = m_radius * 2.0f;

	m_zoom *= 1 + factor;
 
	if (m_ortho)
	{
		factor += 1.0f;

		m_frustumL *= factor;
		m_frustumR *= factor;
		m_frustumT *= factor;
		m_frustumB *= factor;

		/*set near and far to capture data*/ 
		m_frustumN = m_dist2target - rad;
		m_frustumF = m_dist2target + rad;
		if (m_frustumN < 0.1f) m_frustumN = 0.1f;
	}
	else
	{
		factor = -factor;

		if (m_fov < m_minfov) m_fov = m_minfov;
		if (m_fov > MAX_FOV) m_fov = MAX_FOV;

		if (m_dist2target > m_zoomthreshold)
		{
			/*dont want to clip out the target*/ 
			float z = (m_dist2target* (1 - factor)) / m_zoomthreshold;

			if (z >= 1.0)
			{
				/*set clipping planes*/ 
				computeNearFarPlanes();

				//m_frustumN = m_dist2target - rad;
				//m_frustumF = m_dist2target + rad;

				//if (m_frustumN < 0.1f)
				//{
				//	/*check dist to target*/ 
				//	if (m_dist2target < 1.0f)
				//		m_frustumN = 0.1f;
				//	else if (m_frustumN < (rad / 1000)) m_frustumN = (rad / 1000);
				//}
				
				m_dist2target *= 1 - factor;

				/*add bias*/ 
				setFrustum(m_fov, 0, m_frustumN, m_frustumF);
			}
			else
			{

				m_dist2target = m_zoomthreshold;
				zoomStatic(z * -factor);
			}
		}
		else if (m_fov > m_minfov && factor > 0)
		{

			computeNearFarPlanes();

			/*set near and far to capture data*/ 
			//m_frustumN = m_dist2target - rad;
			//m_frustumF = m_dist2target + rad;

			/*zoom in with fov*/ 
			if (m_dist2target < 2.0f) m_frustumN = 0.1f;

			factor = 1.0 - factor;

			factor *= m_fov;

			if (factor > m_minfov)	setFrustum(factor, 0, m_frustumN, m_frustumF);
			else m_fov = m_minfov;
		}
		else if (m_fov >= m_minfov && factor < 0)
		{

			/*set near and far to capture data*/ 
			//m_frustumN = m_dist2target - rad;
			//m_frustumF = m_dist2target + rad;
			computeNearFarPlanes();

			if (m_frustumN < 0.1f) m_frustumN = 0.1f;

			/*zoom in with fov*/ 

			if (m_fov < m_basefov)
			{
				/*zoom out from close in limit*/ 
				factor = 1.0 - factor;
				factor *= m_fov;

				setFrustum(factor, 0, m_frustumN, m_frustumF);
			}
			else
			{
				m_fov = m_basefov;
				m_dist2target *= 1 - factor;
			}
		}
		/*set near and far to capture data*/
		//m_frustumN = dist2cen - rad;
		//m_frustumF = dist2cen + rad;
		//if (m_frustumN < 0.1f) m_frustumN = 0.1f;
		//setFrustum(m_fov, 0, m_frustumN, m_frustumF);
	}
	//std::cout << "Frustum Near " << m_frustumN << "\tFar " << m_frustumF << std::endl;
	//std::cout << "Data radius " << rad << "\tDist2Target " << m_dist2target << std::endl;

	m_frustum_needs_update = true;
	m_model_needs_update = true;
}
//----------------------------------------------------------
// zoom to Window
//----------------------------------------------------------
void Camera::zoomWindow(const Rectd &from, const Rectd &to)
{
	float zoom = 1.0f;
	
	/*pan to centre*/ 
	int panx = to.mid(0) - from.mid(0);
	int pany = -(to.mid(1) - from.mid(1));

	panStatic(panx,pany);
	update();

	/*take greatest distance from centre and scale to half*/ 
	if (from.dx() >= from.dy())
		zoom = from.dx() / to.dy();
	else if (from.dy() > 0)
		zoom = from.dy() / to.dy();

	zoomStatic(zoom-1.0f);
}
void Camera::getViewport(Rectd &vp)
{
	vp.set(0, 0, m_width, m_height);
}
//----------------------------------------------------------
// zoom to Window
//----------------------------------------------------------
void Camera::zoomWindow(const Rectd &from)
{
	Rectd vp;
	getViewport(vp);
	zoomWindow(from, vp);
}
//----------------------------------------------------------
// set Frustum planes
//----------------------------------------------------------
void Camera::setFrustum(float fNear, float fFar, float fLeft, float fRight, float fTop, float fBottom)
{
	/*impose depth limit*/ 
	if (fFar / fNear > 2000)
	{
		fFar = fNear * 2000;
	}
	m_frustumN = fNear;
    m_frustumF = fFar;
    m_frustumL = fLeft;
    m_frustumR = fRight;
    m_frustumT = fTop;
    m_frustumB = fBottom;

	m_frustum_needs_update = true;
}
//----------------------------------------------------------
// set Frustum planes
//----------------------------------------------------------
void Camera::setFrustum(double fNear, double fFar, double fLeft, double fRight, double fTop, double fBottom)
{
	/*impose depth limit*/ 
	if (fFar / fNear > 2000)
	{
		fFar = fNear * 2000;
	}
	m_frustumN = fNear;
    m_frustumF = fFar;
    m_frustumL = fLeft;
    m_frustumR = fRight;
    m_frustumT = fTop;
    m_frustumB = fBottom;

	m_frustum_needs_update = true;
}
//----------------------------------------------------------
// set frustum FOV and near/far clipping planes
//----------------------------------------------------------
void Camera::setFrustum (float fUpFovDegrees, float fAspectRatio, float fNear, float fFar)
{
    float fHalfAngleRadians = 0.5f*fUpFovDegrees* DEG_TO_RAD;

	m_fov = fUpFovDegrees;

	fAspectRatio = 1.0f;//(float)m_width / m_height;

	/*aspect will be calculated later*/ 
    m_frustumT = fNear* tan(fHalfAngleRadians);
    m_frustumB = -m_frustumT;
    m_frustumR = fAspectRatio*m_frustumT;
    m_frustumL = -m_frustumR;
    m_frustumN = fNear;
    m_frustumF = fFar;
	m_frustum_needs_update = true;
}
//----------------------------------------------------------
// set frustum near clipping plane
//----------------------------------------------------------
void Camera::setFrustumNear (float fNear)
{
	m_frustumN = fNear;
	m_frustum_needs_update = true;
}
//----------------------------------------------------------
// set frustum far clipping plane
//----------------------------------------------------------
void Camera::setFrustumFar (float fFar)
{
	m_frustumF = fFar;
	m_frustum_needs_update = true;
}
//----------------------------------------------------------
// set frustum depth (based on target)
//----------------------------------------------------------
void Camera::setFrustumDepth(float depth)
{
	float d = distanceToTarget();
	m_frustumF = d + depth / 2;
	m_frustumN = d - depth / 2;

	if (m_frustumN < 0.3f)
	{
		m_frustumN = 0.3f;
	}
	m_frustum_needs_update = true;
}
//----------------------------------------------------------
// set viewport
//----------------------------------------------------------
void  Camera::setViewport (float fLeft, float fRight, float fTop, float fBottom)
{
    m_viewportL = fLeft;
    m_viewportR = fRight;
    m_viewportT = fTop;
    m_viewportB = fBottom;

	m_vport_needs_update = true;
}
//----------------------------------------------------------
// USER NAVIGATION CONTROL HANDLERS
//----------------------------------------------------------
void Camera::setNavigation(GLnavstate nav)
{
	switch (nav)
	{
		case TUMBLE:
			if (m_nav_state != TUMBLE && !m_constrained)
				m_arcball.MouseDown(m_mouseX, m_mouseY);
			break;
		case NONE:
			if (m_nav_state == TUMBLE && !m_constrained)
			{
				m_arcball.MouseUp(m_mouseX, m_mouseY);
				resetArcball();
			}
	}
	m_lastX = m_mouseX;
	m_lastY = m_mouseY;

	m_nav_state = nav;
}
//----------------------------------------------------------
// USER NAVIGATION CONTROL HANDLERS
//----------------------------------------------------------
void Camera::resetControl()
{
	m_lastX = m_mouseX;
	m_lastY = m_mouseY;
	
	m_nav_state = NONE;	
}
//----------------------------------------------------------
// MOVE - call on mouse move
//----------------------------------------------------------
bool Camera::moveControl(int x, int y)
{
	m_lastX = m_mouseX;
	m_lastY = m_mouseY;

	m_mouseX = x;
	m_mouseY = y;

	switch (m_nav_state)
	{
	case TUMBLE: rotate(); break;
	case PAN: pan(); break;
	case ZOOM: zoom(); break;
	}

	return m_nav_state != NONE;
}
//----------------------------------------------------------
// pan - called by moveControl
//----------------------------------------------------------
void Camera::pan()
{
	//find vector in realspace based at center of screen using z of target
	//move both location and target by this vector
	GLdouble sc[3];
	GLdouble a[3];
	GLdouble b[3];

	m_vstore.project3v(m_target, sc);
	GLdouble z = sc[2];

	/*vector start */ 
	sc[0] = m_mouseX;
	sc[1] = m_lastY;
	m_vstore.unproject3v(sc, a);

	sc[0] = m_lastX;
	sc[1] = m_mouseY;
	m_vstore.unproject3v(sc, b);

	b[0] -= a[0];
	b[1] -= a[1];
	b[2] -= a[2];

	if (m_nav_mode == WALK)
	{
		m_actlocation.x += b[0];
		m_actlocation.y += b[1];
		m_actlocation.z += b[2];		
	}
	else
	{
		m_target.x += b[0];
		m_target.y += b[1];
		m_target.z += b[2];
	}
	
	m_model_needs_update = true;

//	std::cout << "near plane = " << m_frustumN << " Far plane = " << m_frustumF << std::endl;
}
//----------------------------------------------------------
// rotate - this is called by moveControl
//----------------------------------------------------------
void Camera::rotate()
{
	if (m_nav_mode == LIGHT)
	{
		vector3 dir = m_light->directionAsEulers();
		dir.y = fmod((double)dir.y, 360);
		dir.z = fmod((double)dir.z, 360);
		dir.x = 0;

		dir.z -= (m_mouseY - m_lastY) / 3.0f;
		dir.y += (m_mouseX - m_lastX) / 3.0f;

		m_light->directionFromEulers(dir);
		return;
	}

	if (!m_constrained)
		m_arcball.MouseMove(m_mouseX,m_mouseY);
	else
	{
		m_angleX = fmod((double)m_angleX, 2*PI);
		m_angleY = fmod((double)m_angleY, 2*PI);
		m_angleZ = fmod((double)m_angleZ, 2*PI);
 
		switch (m_constrained_up_axis)
		{

		case 0:
			m_angleZ -= (m_mouseY - m_lastY) / 180.0f;
			m_angleX -= (m_mouseX - m_lastX) / 180.0f;
			break;
		case 1:
			m_angleY += (m_mouseX - m_lastX) / 180.0f;
			m_angleX -= (m_mouseY - m_lastY) / 180.0f;
			break;
		case 2:
			m_angleZ -= (m_mouseX - m_lastX) / 180.0f;
			m_angleX -= (m_mouseY - m_lastY) / 180.0f;
			break;
		}
		Wm5::Matrix3f m;

		float x = m_angleX;
		float y = m_angleY;
		float z = m_angleZ;

        m.MakeEulerZYX(m_angleZ, m_angleY, m_angleX);
		rotationFromMatrix(m);
	}
	m_model_needs_update = true;
}
//----------------------------------------------------------
// Use Constrainsts
//----------------------------------------------------------
void Camera::setUseConstraints(bool use)
{
	m_constrained = use;
	if (use) setConstraintAxis(m_constrained_up_axis);
}
//
#define CMPSIGN(x,y) ( (x < 0 && y < 0) || (x>0 && y>0) || (!x && !y))
//----------------------------------------------------------
// Use Constrainsts
//----------------------------------------------------------
void Camera::setConstraintAxis(int axis)
{
	if (m_constrained && m_constrained_up_axis == axis) return;

	m_constrained = true;
	m_constrained_up_axis = axis;

	/*zero out unwanted rotations*/ 
	switch (axis)
	{
		case 0: if (fmod((double)m_angleY, PI) > 0.001f) m_angleY = 0; break;
		case 1: if (fmod((double)m_angleZ, PI) > 0.001f) m_angleZ = PI; break;
		case 2: if (fmod((double)m_angleY, PI) > 0.001f) m_angleY = PI; break;
	}
	rotationFromEulerZYX(m_angleZ, m_angleY, m_angleX);	
	return;
}
//----------------------------------------------------------
// Zoom
//----------------------------------------------------------
void Camera::zoom()
{
	if (m_nav_mode == INSPECT)
		zoomStatic((float)(m_lastY - m_mouseY) / 100);
	else
	{
		/*we dont want vertical mouse to effect rotate*/ 
		//int y = m_mouseY;
		//m_mouseY = m_lastY;
		//rotate();

		//m_mouseY = y;
		dollyStatic(m_frustumF * (float)(m_lastY - m_mouseY) / 1500.0f);
	}
}
//----------------------------------------------------------
// Dolly
//----------------------------------------------------------
void Camera::dollyStatic(float distance)
{
	pt::vector3 v =  m_actlocation - m_target;
	v.normalize();
	m_actlocation += v * distance;
	m_target += v * distance;
	m_model_needs_update = true;
}
//----------------------------------------------------------
// set direction of target (not sure this is working correctly)
//----------------------------------------------------------
void Camera::setTargetDirection(float *target)
{
	pt::vector3 tar3(target);
	pt::vector3 v = tar3 - m_actlocation;
	v.normalize();
	v *= m_dist2target;
	v += m_actlocation;
	centerViewAt(v);	
}
//----------------------------------------------------------
// pan in screen coordinates
//----------------------------------------------------------
void Camera::panStatic(int dx, int dy)
{
	//find vector in realspace based at center of screen using z of target
	//move both location and target by this vector
	GLdouble sc[3];
	GLdouble a[3];
	GLdouble b[3];

	m_vstore.project3v(m_target, sc);
	GLdouble z = sc[2];

	/*vector start */ 
	sc[0] = m_width / 2;
	sc[1] = m_height / 2;
	m_vstore.unproject3v(sc, a);

	sc[0] -= dx;
	sc[1] += dy;
	m_vstore.unproject3v(sc, b);

	for (int i=0; i<3; i++)
	{
		b[i] -= a[i];
		m_target[i] += b[i];
	}
	m_model_needs_update = true;
}
//----------------------------------------------------------
// distance to target
//----------------------------------------------------------
float Camera::distanceToTarget() const
{
	return m_dist2target;
}
//----------------------------------------------------------
// set arcball quaternion from axis angle
//----------------------------------------------------------
void Camera::rotationFromAxisAngle(float angle, const vector3 &axis)
{
	Wm5::Quaternionf q;
	Wm5::Vector3f ax(axis[0], axis[1], axis[2]);
	q.FromAxisAngle(ax, angle);
	m_arcball.SetQuat(gl_unitquaternion(gl_quaternion(q.W(), q.X(), q.Y(), q.Z())));

	m_model_needs_update = true;
}
//----------------------------------------------------------
// set arcball quaternion from rotation matrix
//----------------------------------------------------------
void Camera::rotationFromMatrix(const float *mat)
{
	Wm5::Matrix3f M;
	memcpy(&M, mat, sizeof(Wm5::Matrix3f));
	
	Wm5::Quaternionf q;
	q.FromRotationMatrix(M);
	m_arcball.SetQuat(gl_unitquaternion(gl_quaternion(q.W(), q.X(), q.Y(), q.Z())));

	m_model_needs_update = true;
}
//----------------------------------------------------------
// set arcball quaternion from Euler angles
//----------------------------------------------------------
void Camera::rotationFromEulerZYX(float z, float y, float x)
{
	Wm5::Matrix3f m;

	m_angleZ = z;
	m_angleY = y;
	m_angleX = x;

    m.MakeEulerZYX(z, y, x);
	rotationFromMatrix(m);
}
//----------------------------------------------------------
// resolve Euler angles from arcball quaternion
//----------------------------------------------------------
void Camera::resolveEulers()
{
	gl_tmatrix m = m_arcball.GetMatrix();
	Wm5::Matrix3f M;	for (int i=0; i<3; i++)	for (int j=0; j<3; j++)	M[i][j] = m(i,j);
	M.Transpose();
    M.Inverse().ExtractEulerZYX(m_angleZ, m_angleY, m_angleX);
	m_angleX += PI;
	m_angleY += PI;
	m_angleZ += PI;
}
//----------------------------------------------------------
// set arcball quaternion from look at                      
// based on GLU manual                                      
//----------------------------------------------------------
void Camera::rotationFromLookAt(const vector3 &location, const vector3 &_up)
{
	if (location.length2() < 0.0001f) return;

	vector3 zv(location);
	zv.normalize();

	vector3 up(0,0,1);
	vector3 xv(zv.unit_cross(up));
	vector3 yv(xv.unit_cross(zv));

	Wm5::Matrix3f M;
	
	M[0][0] = -xv.x;
	M[1][0] = -xv.y;
	M[2][0] = -xv.z;

	M[0][1] = yv.x;
	M[1][1] = yv.y;
	M[2][1] = yv.z;

	M[0][2] = zv.x;
	M[1][2] = zv.y;
	M[2][2] = zv.z;

	rotationFromMatrix(M);
	update();
	resolveEulers();
}
//
// set Ortho - try to balance switch from perspective
//
void Camera::setOrtho(bool ortho)
{ 
	if (ortho != m_ortho)
	{
		/*data at target should stay roughly the same size*/ 
		BoundingBox bb(m_target.x + 5.0f, m_target.x - 5.0f,
			m_target.y + 5.0f, m_target.y - 5.0f, 
			m_target.z + 5.0f, m_target.z - 5.0f);
		Rectd r;
		
		projectedBoxExtents(&bb, r);

		m_frustum_needs_update = true; 
		m_ortho = ortho;
		update();

		double fov = m_fov;

		zoomTo(&bb, r, 15);

		/*check fov*/ 
		if (!ortho)
		{
			setFrustum(fov, 0, m_frustumN, m_frustumF);
			m_fov = fov;
		}
	}
	m_ortho = ortho; 
}
//
// move camera to view - set target
//
void Camera::centerViewAt(const float *v)
{
	/*maintain position of camera relative to target*/ 
	vector3 target(v);
 	m_dist2target = vector3(m_actlocation, target).length();
	
	m_target = target; 
	m_model_needs_update = true;

	if (m_nav_mode == WALK)
	{
		setMode(INSPECT);
		update();
		setMode(WALK);
	}
}	
//
// target
//
void Camera::setTarget(const float *v)
{
	/*maintain position of camera relative to target*/ 
	vector3 target(v);
 	m_dist2target = vector3(m_actlocation, target).length();
	m_target = target; 
	m_model_needs_update = true;

	setLocation(m_actlocation);
};
void	Camera::getTarget(float *v) const { memcpy(v, &m_target, sizeof(float)*3); };
const	float *Camera::getTarget() const { return (const float*)&m_target; };
//
// get actual camera location
//
void Camera::getLocation(float *v) const
{
	memcpy(v, &m_actlocation, sizeof(float)*3);
}
//
// set Location and Target at the same time
//
void Camera::setLocationAndTarget(const float *loc, const float *tar)
{
	GLnavmode mode = m_nav_mode;
	m_nav_mode = INSPECT;

	m_target.set(tar);
	setLocation(loc);

	m_nav_mode = mode;
}
//
// get up vector
//
void Camera::getUp(float *v) const
{
	memcpy(v, &m_up, sizeof(float)*3);
}
//
// get up vector
//
const float *Camera::getUp() const
{
	return (const float*)&m_up;
}
//
// set location of camera
//
void Camera::setLocation(const float *v)
{
	if (std::isnan(v[0])) return;

	vector3 loc(v);
	loc -= m_target;
	
	GLnavmode mode = m_nav_mode;
	m_nav_mode = INSPECT;

	m_dist2target = loc.length();
	rotationFromLookAt(loc, m_up);

	m_nav_mode = mode;
}
//
//
//
void Camera::translate(const float *v)
{
	vector3 loc(m_actlocation + vector3(v));
	setLocation(loc);
}
//
//
//
void Camera::translateTarget(const float *v)
{
	vector3 loc(m_target+ vector3(v));
	setTarget(loc);
}
//
// reset location
//
void Camera::resetLocation()
{
	vector3 l(0,0,-distanceToTarget());
	rotationFromLookAt(l, m_up);
}
//
//
//
void Camera::setUp(const float *v)
{
	rotationFromLookAt(m_actlocation, v);
}
//
//
//
void Camera::setDataBounds(const BoundingBox &box)
{
	m_databounds = box;
}
//
//
//
const BoundingBox &Camera::getDataBounds() const
{
	return m_databounds;
}
//
//
//
void Camera::setLight(Light *light)
{
	m_light = light; 
}
//
//
//
Light *Camera::getLight()
{
	return m_light; 
}
//
//
//
void Camera::setDataSphere(const float *center, float radius)
{
	if (radius < 0.001) return;

	m_center = center;
	m_radius = radius;
}
//
//
//
void Camera::getDataSphere(float *center, float &radius) const
{
	memcpy(center, &m_center, sizeof(float)*3);
	radius = m_radius;
}
//
// zoomTo
//
void Camera::zoomTo(const BoundingBox *bb, int tol)
{
	Rectd win;
	getViewport(win);
	zoomTo(bb, win, tol);
}
//
// zoomTo - bounding box --> area on screen
//
void Camera::zoomTo(const BoundingBox *bb, const Rectd &win, int tol)
{
	Rectd r;

	int it = 0;
	bool success = false;
	m_fov = m_basefov;

	GLnavmode nm = m_nav_mode;
	m_nav_mode = INSPECT;
	
	do
	{
		it++;

		/*set target to centre*/ 
		centerViewAt(bb->center());
		update();

		projectedBoxExtents(bb, r);

		/*check for success - containment within tolerance*/ 
		if(
			(	fabs(r.lx() - win.lx()) < tol
			&&	fabs(r.ux() - win.ux()) < tol
			&&	r.ly() >= win.ly()
			&&	r.uy() <= win.uy())
			||
			(	fabs(r.ly() - win.ly()) < tol
			&&	fabs(r.uy() - win.uy()) < tol
			&&	r.lx() >= win.lx()
			&&	r.ux() <= win.ux())	)
		{
			success = true;
			break;
		}

		/*now do zoom window*/ 
		zoomWindow(r, win);
		update();
	}
	while (it < 30 && !success);
	m_nav_mode = nm;
}
//
// projected Box
//
void Camera::projectedBoxExtents(const BoundingBox *bb, Rectd &rect) const
{
	GLdouble px[3];

	/*build 2d bounding box*/ 
	rect.makeEmpty();

	vector3 v;
	/*do projections*/ 
	for (int i=0; i<8; i++)
	{
		bb->getExtrema(i, v);
		project3v(v, px);
		rect.expand(px);
	}
}
//
// projection
//
bool Camera::project3v(const float *obj, GLfloat *win) const 
{
	GLdouble w[3]; 
	bool p = m_vstore.project3v(obj, w); 
	win[0] = w[0]; win[1] = w[1]; win[2] = w[2]; 
	return p; 
}
//
// Compute far and near planes
//
void Camera::computeNearFarPlanes()
{
	/*apply modelview to bounding box*/ 
	BoundingBox fsbb;

	mmatrix4d mat;
	memcpy(&mat, &m_vstore.model_mat, sizeof(double)*16);
	mat.transpose();

	vector3 v,vfs;

	/*do projections*/ 
	for (int i=0; i<8; i++)
	{
		m_databounds.getExtrema(i, v);
		mat.vec3_multiply_mat4f(v, vfs);
		fsbb.expand(vfs);
	}	

	float nz = -fsbb.uz();
	float fz = -fsbb.lz();

	if (fz > 0)
	{
		if (nz > 0)
		{
			m_frustumN = nz;
			m_frustumF = fz;
			
			/* check for excessive depth*/ 
			if (m_frustumF / m_frustumN > 2000)
				m_frustumN = m_frustumF / 2000;
			
			/* check for potential expansion*/ 
			else while (m_frustumF / m_frustumN < 600)
			{
				m_frustumN *= 0.75;
				m_frustumF *= 1.25;
			}
		}
		else
		{
			m_frustumN = fz / 1000;
			m_frustumF = fz;
		}
		//use these values to build projection keeping fov
		setFrustum(m_fov, 0, m_frustumN, m_frustumF);
		m_frustum_needs_update = true;
	}
	else
	{
		//std::cout << "Plane computation ERROR, far plane behind camera" << std::endl;
	}
}
//
//	write data block for persistance
//
//  calling method must allocate enough mem
void Camera::writeBranch(pt::datatree::Branch *b)
{
	b->addNode("angle_X", m_angleX);
	b->addNode("angle_Y", m_angleY);
	b->addNode("angle_Z", m_angleZ);

	b->addNode("frustum_N", m_frustumN);
	b->addNode("frustum_F", m_frustumF);
	b->addNode("frustum_L", m_frustumL);
	b->addNode("frustum_R", m_frustumR);
	b->addNode("frustum_T", m_frustumT);
	b->addNode("frustum_B", m_frustumB);

	b->addNode("min_FOV", m_minfov);
	b->addNode("base_FOV", m_basefov);
	b->addNode("current_FOV", m_fov);

	b->addNode("ortho", m_ortho);
	b->addNode("radius", m_radius);
	b->addNode("center", m_center);
	
	b->addNode("dist_2_target", m_dist2target);
	
	b->addNode("target", m_target);
	b->addNode("actual_location", m_actlocation);
	b->addNode("viewport_width", m_width);
	b->addNode("viewport_height", m_height);
	b->addNode("up vector", m_up);
	b->addNode("zoom", m_zoom);
	b->addNode("zoom_threshold", m_zoomthreshold);
	
	pt::datatree::Branch *arcb = b->addBranch("Arcball");
	m_arcball.writeBranch(arcb);
}

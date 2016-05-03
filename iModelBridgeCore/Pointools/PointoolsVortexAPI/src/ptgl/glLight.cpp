#include "PointoolsVortexAPIInternal.h"

#include <gl/gl.h>
#include <gl/glExt.h>

#include <math/matrix_math.h>

#include <ptgl/gllight.h>


using namespace ptgl;

#define DEG2RAD 0.017453292519943295769236907684886
// note that Fixed is not used internally, but is used by camera to determine 
// the correct time to setup the light
Light::Light() : m_position(10.0f,10.0f,10.0f), m_direction(1.0f,1.0f,1.0f), 
		m_ambient(0.2f,0.2f,0.2f), m_specular(1.0f,1.0f,1.0f),
		m_diffuse(0.7f,0.7f,0.7f), m_type(DIRECTIONAL), m_on(true)
{
	resetDirection();
};
void Light::setupGL() const
{
	GLfloat ambient[] = { m_ambient.x, m_ambient.y, m_ambient.z, 1.0f };
	GLfloat diffuse[] = { m_diffuse.x, m_diffuse.y, m_diffuse.z, 1.0f };
	GLfloat specular[] = { m_specular.x, m_specular.y, m_specular.z, 1.0f };

	GLfloat dir[] = { m_direction.x, m_direction.y, m_direction.z, 0 }; 

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	glLightfv(GL_LIGHT0, GL_POSITION, dir);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
}
#ifdef PTGL_USING_CG
void Light::setupCg(CgShader *shader) const
{
	shader->setLight(m_ambient, m_diffuse, m_specular, m_position, m_direction);
}
#endif
void Light::directionFromEulers(const pt::vector3 &dir)
{
	m_eulers = dir;
	mmatrix4 M = mmatrix4::rotation(m_eulers.x*DEG2RAD, m_eulers.y*DEG2RAD, m_eulers.z*DEG2RAD);
	M.invert();
	M.transpose();
	M.vec3_multiply_mat4(pt::vector3(1.0f,0,0), m_direction);
	m_direction.normalize();
}
void Light::resetDirection()
{
	directionFromEulers(pt::vector3(0,-120,24));
}

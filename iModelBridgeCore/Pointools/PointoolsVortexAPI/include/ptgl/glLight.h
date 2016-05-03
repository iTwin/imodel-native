#ifndef POINTOOLS_GL_LIGHT
#define POINTOOLS_GL_LIGHT

#include <ptgl/ptgl.h>
#include <pt/geomtypes.h>

namespace ptgl
{
class CgShader;
enum LightType { SPOT, DIRECTIONAL, POINT };

class PTGL_API Light
{
public:
	Light();
	virtual ~Light(){};

	const pt::vector3 &getPosition() const { return m_position; }
	void setPosition(const pt::vector3 &pos) { m_position = pos; }

	const pt::vector3 &getDirection() const { return m_direction; }
	void setDirection(const pt::vector3 &dir) { m_direction = dir; }

	const pt::vector3 &getAmbient() const { return m_ambient; }
	const pt::vector3 &getDiffuse() const { return m_diffuse; }
	const pt::vector3 &getSpecular() const { return m_specular; }

	void setAmbient(const pt::vector3 &v) { m_ambient = v; }
	void setDiffuse(const pt::vector3 &v) { m_diffuse = v; }
	void setSpecular(const pt::vector3 &v) { m_specular = v; }

	bool isOn() const { return m_on; }
	void on()	{ m_on = true; }
	void off()	{ m_on = false; }

	void setType(LightType t) { m_type = t; }
	LightType getType() const { return m_type; }

	void setupGL() const;
#ifdef PTGL_USING_CG
	void setupCg(CgShader *shader) const;
#endif

	pt::vector3 directionAsEulers() const { return m_eulers; }
	void directionFromEulers(const pt::vector3 &dir);

	void resetDirection();

private:
	pt::vector3 m_position;
	pt::vector3 m_direction;
	
	pt::vector3 m_ambient;
	pt::vector3 m_specular;
	pt::vector3 m_diffuse;

	pt::vector3 m_eulers;

	LightType	m_type;
	bool		m_on;
};
}

#endif
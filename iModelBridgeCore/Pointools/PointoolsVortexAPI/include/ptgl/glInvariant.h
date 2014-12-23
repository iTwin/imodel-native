#ifndef POINTOOLS_GL_INVARIANT
#define POINTOOLS_GL_INVARIANT

#define PT_NUMSTATES	6
#include <gl/gl.h>

namespace ptgl
{
	class Invariant
	{
	public:
		Invariant()
		{
			for (int i=0; i<PT_NUMSTATES; i++)
				_enabled[i] = glIsEnabled(states()[i]);
			glGetFloatv(GL_POINT_SIZE, &_pointSize);
		}
		~Invariant()
		{
			for (int i=0; i<PT_NUMSTATES; i++)
			{
				if (_enabled[i])	glEnable(states()[i]);
				else				glDisable(states()[i]);
			}
			glPointSize(_pointSize);
		}
		const GLenum *states() 
		{
			static GLenum _states[] = {
					GL_LIGHTING, 
					GL_BLEND, 
					GL_COLOR_MATERIAL,
					GL_NORMALIZE, 
					GL_TEXTURE_2D, 
					GL_TEXTURE_1D };

			return _states;
		}
		GLboolean	_enabled[PT_NUMSTATES];
		GLfloat		_pointSize;
	};
}
#endif
#include "PointoolsVortexAPIInternal.h"

#include <ptgl/glViewsetup.h>

#include <gl/gl.h>
#include <gl/glu.h>

using namespace ptgl;

PixelView::PixelView()
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	_l = viewport[0];
	_b = viewport[1];
	_r = viewport[2] + _l;
	_t = viewport[3] + _b;

	/*setup ortho*/  
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluOrtho2D(_l,_r, _b, _t);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();	
}
PixelView::~PixelView()
{
	glFlush();

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

XORView::XORView()
{
	glEnable(GL_COLOR_LOGIC_OP);
	
	glDrawBuffer(GL_FRONT);
	glLogicOp(GL_XOR);	
}
XORView::~XORView()
{
	glDisable(GL_COLOR_LOGIC_OP);
	glDrawBuffer(GL_BACK);
}
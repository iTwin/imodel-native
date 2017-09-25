#include "SimpleSky.h"
//#include "../deferred/NoDeferredRendering.h"
#include "NavigationTool.h"

#define DEG2RAD 0.01745329251994329576923690768489

SimpleSky::SimpleSky()
{
}

void SimpleSky::preDrawSetup()
{ 

}
 
void SimpleSky::drawPreDisplay()
{
	//NoDeferredRendering ndr;
	
	const GLfloat *horizon = ExampleApp::instance()->getView().backColor;

	glClear(GL_COLOR_BUFFER_BIT);//added by jchen
	glColor3fv(horizon);
	GLHelper::renderFullViewportQuad(false);	

	static GLubyte morning[] =
	{
		21,72,164,  22,74,168,  22,76,171,  27,83,171,  39,95,174,  50,108,179,  65,122,191,  87,140,200,  104,152,208,
		122,166,210,  140,184,215,  160,201,221,  184,218,225,  199,227,217,  218,235,208,  240,238,163
	};

	static GLubyte dusk[] =
	{
		16,32,56,  17,34,60,  20,37,65,  22,40,69,  24,45,75,  25,50,81,  31,55,89,  37,63,97,  45,70,109,  52,79,117,  62,92,129,
		78,108,141,  100,126,157,  128,146,147,  169,171,175,  216,188,147
	};

	GLubyte *colors = morning;
	 
	glDisable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);

	int skip = 6;
	int num_tiles = 360 / skip;
	float tmp;
	float w1, h1, w2, h2;
	float c, s;
	float t_x1, t_x2, t_y1, t_y2;

	float c1[] = { 0.6f, 0.6f, 0.9f };
	float c2[] = { 0.3f, 0.3f, 0.8f };

	int col = 0;

	float radius = 1000;

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	const Camera &cam = ExampleApp::instance()->getCamera();

	Vector3f pos = CameraNavigation::instance()->getCameraPosition();
	Vector3f tar = CameraNavigation::instance()->getCameraTarget();
	
	Vector3f dir = tar - pos;
	GLdouble cen[] = { dir.x, dir.y, dir.z };
	gluLookAt(0, 0, 0, cen[0], cen[1], cen[2], 0, 0, 1);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(cam.fov(), ExampleApp::instance()->getUI().xy_aspect, cam.nearPlane(), cam.farPlane());

	for (int i = 0; i <= 90; i += skip)
	{
		tmp = float(i) * DEG2RAD;

		w1 = sin(tmp);
		h1 = cos(tmp);

		tmp = float(i + skip) * DEG2RAD;
		w2 = sin(tmp);
		h2 = cos(tmp);

		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j <= 360; j += skip)
		{
			tmp = float(j + skip) * DEG2RAD;
			c = cos(tmp);
			s = sin(tmp);

			if (j + skip > 360)
			{
				c += 0.0005f;
				s += 0.0005f;
			}

			t_x1 = (float)(j / skip) / (float)num_tiles;
			t_x2 = (float)((j + skip) / skip) / (float)num_tiles;
			 
			t_y1 = 1.0f - ((h1 + 1.0f) / 2.0f); /* clamp to 0 to 1 range */
			t_y2 = 1.0f - ((h2 + 1.0f) / 2.0f);

			//gfx->TexCoord(t_x2, t_y2);
			if (col > 14)
				glColor3f(horizon[0], horizon[1], horizon[2]);
			else
				glColor3ub(colors[(col + 1) * 3], colors[(col + 1) * 3 + 1], colors[(col + 1) * 3 + 2]);

			glVertex3f(c * w2 * radius, s * w2 * radius, h2 * radius);

			if (col > 15)
				glColor3f(horizon[0], horizon[1], horizon[2]);
			else
				glColor3ub(colors[col * 3], colors[col * 3 + 1], colors[col * 3 + 2]);
			
			glVertex3f(c * w1 * radius, s * w1 * radius, h1 * radius);

		}
		glEnd();
		col++;
	}
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
}

void SimpleSky::drawPostDisplay()
{
}

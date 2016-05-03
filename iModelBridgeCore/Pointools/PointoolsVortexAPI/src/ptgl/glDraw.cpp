#include "PointoolsVortexAPIInternal.h"
#include <pt/geomtypes.h>

#include <gl/gl.h>
#include <ptgl/gldraw.h>

using namespace ptgl;
using namespace pt;

#define RAD2DEG 57.29577951308232087679815481410
#define DEG2RAD 0.017453292519943295769236907684
#define PI2		6.283185307179586476925286766559

namespace ptgl
{
	enum Style { Solid, Dash, None };
	
	static Style _linestyle = Solid;
	static Style _fillstyle = Solid;

	static vector3 _linecol = vector3(1.0f,1.0f,1.0f);
	static vector3 _backcol;
	static vector3 _fillcol;
	static vector3 _widgetcol = vector3(0.5f,1.0f,1.0f);

	static float _fillalpha = 1.0f;
	static float _linealpha = 1.0f;
	static float _linewidth = 1.0f;
	static float _depth = 0;

	static float _arrowlength = 15.0f;
	static float _arrowwidth = 5.0f;
};
void Draw::lineSolid()					{ _linestyle = Solid;	}
void Draw::lineDash()					{ _linestyle = Dash;	}
void Draw::lineNone()					{ _linestyle = None;	}
void Draw::lineWidth(float w)			{ _linewidth = w;		}
void Draw::lineColor(const float *col)	{ _linecol.set(col);	}
void Draw::lineOpacity(float alpha)		{ _linealpha = alpha;	}

void Draw::interactColor(const float *col)	{ _widgetcol.set(col);	}
void Draw::backColor(const float *col)	{ _backcol.set(col);	}
void Draw::fillColor(const float *col)	{ _fillcol.set(col);	}
void Draw::fillOpacity(float alpha)		{ _fillalpha = alpha;	}
void Draw::fillSolid()					{ _fillstyle = Solid;	}
void Draw::fillNone()					{ _fillstyle = None;	}

const float* Draw::fillColor() { glColor4f(_fillcol[0], _fillcol[1], _fillcol[2], _fillalpha); return _fillcol; }
const float* Draw::lineColor() { glColor4f(_linecol[0], _linecol[1], _linecol[2], _linealpha); return _linecol; }
const float* Draw::backColor() { glColor4f(_backcol[0], _backcol[1], _backcol[2], 1.0f); return _backcol; }
const float* Draw::interactColor() { glColor4f(_widgetcol[0], _widgetcol[1], _widgetcol[2], 1.0f); return _widgetcol; }

void Draw::fillColor(float r, float g, float b)		{ _fillcol.set(r, g, b);	}
void Draw::lineColor(float r, float g, float b)		{ _linecol.set(r, g, b);	}
void Draw::backColor(float r, float g, float b)		{ _backcol.set(r, g, b);	}
void Draw::interactColor(float r, float g, float b)		{ _widgetcol.set(r, g, b);	}

void Draw::depth(float d) { _depth = d; }
void Draw::arrowLength(float l) { _arrowlength = l; }
void Draw::arrowWidth(float w) { _arrowwidth = w; }

//----------------------------------------------------------------------------------------------
// marker
//----------------------------------------------------------------------------------------------
void Draw::marker2(const float *pos)
{
	glPointSize(5.0f);
	interactColor();

	glBegin(GL_POINTS);
		glVertex2fv(pos);
	glEnd();
}
//----------------------------------------------------------------------------------------------
// marker
//----------------------------------------------------------------------------------------------
void Draw::marker2(const double *pos)
{
	glPointSize(5.0f);
	interactColor();

	glBegin(GL_POINTS);
		glVertex2dv(pos);
	glEnd();
}
//----------------------------------------------------------------------------------------------
// marker
//----------------------------------------------------------------------------------------------
void Draw::marker3(const float *pos)
{
	glPointSize(5.0f);
	interactColor();

	glBegin(GL_POINTS);
		glVertex3fv(pos);
	glEnd();
}
//----------------------------------------------------------------------------------------------
// marker
//----------------------------------------------------------------------------------------------
void Draw::marker3(const double *pos)
{
	glPointSize(5.0f);
	interactColor();

	glBegin(GL_POINTS);
		glVertex3dv(pos);
	glEnd();
}
//----------------------------------------------------------------------------------------------
// circle
//----------------------------------------------------------------------------------------------
void Draw::circle2(const float *cen, float rx, float ry, float anglestep)
{
	anglestep *= (float)DEG2RAD;
	float t;

	glLineWidth(_linewidth);

	if (_fillstyle != None)
	{
		fillColor();
		glBegin(GL_POLYGON);
			for (t = 0; t<PI2+anglestep; t+= anglestep)
			{
				float x = sin(t) * rx;
				float y = cos(t) * ry;
				
				glVertex3f(x, y, _depth);
			}
		glEnd();
	}
	if (_linestyle != None)
	{
		lineColor();
		glBegin(GL_LINE_STRIP);
		arc2_vertices(cen, 0, 360, rx, ry, true, anglestep); 
		glEnd();
	}
}
//----------------------------------------------------------------------------------------------
// circle
//----------------------------------------------------------------------------------------------
void Draw::circle2(const double *cen, double rx, double ry, double anglestep)
{
	anglestep *= DEG2RAD;
	double t;	int i=0;

	glLineWidth(_linewidth);

	if (_fillstyle != None)
	{
		fillColor();
		glBegin(GL_POLYGON);
			for (t = 0; t<PI2+anglestep; t+= anglestep)
			{
				double x = sin(t) * rx;
				double y = cos(t) * ry;
				
				glVertex3f(x, y, _depth);
			}
		glEnd();
	}
	if (_linestyle != None)
	{
		lineColor();
		glBegin(GL_LINE_STRIP);
		arc2_vertices(cen, 0, 360, rx, ry, true, anglestep); 
		glEnd();
	}
}
//----------------------------------------------------------------------------------------------
// rounded Box
//----------------------------------------------------------------------------------------------
void Draw::roundedBox2(const float *pos, float w, float h, float r, unsigned char c)
{
	glLineWidth(_linewidth);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(pos[0], pos[1], _depth+0.01f);

	/*fill*/ 
	if (_fillstyle != None)
	{
		fillColor();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_POLYGON);
			if (c & LowerLeft)	arc2_vertices(vector3(r, r, -0.01f), 180, 270, r, r, false, 15);
				
			else				glVertex3f(0, 0, -0.01f);
			
			if (c & UpperLeft)	arc2_vertices(vector3(r, h-r, -0.01f), 270, 360, r, r, false, 15);
			else				glVertex3f(0, h, -0.01f);

			if (c & UpperRight) arc2_vertices(vector3(w-r, h-r, -0.01f), 0, 90, r, r, false, 15);
			else				glVertex3f(w, h, -0.01f);

			if (c & LowerRight) arc2_vertices(vector3(w-r, r, -0.01f), 90, 180, r, r, false, 15);
			else				glVertex3f(w, 0, -0.01f);

			if (c & LowerLeft)	glVertex3f(r, 0, -0.01f);
			else				glVertex3f(0, 0, -0.01f);
		glEnd();
	}
	/*outline*/ 
	if (_linestyle != None)
	{
		lineColor();
		glBegin(GL_LINE_STRIP);
			if (c & LowerLeft)	arc2_vertices(vector3(r, r, 0), 180, 270, r, r, false, 15);
			else				glVertex3f(0, 0, 0);

			if (c & UpperLeft)	arc2_vertices(vector3(r, h-r, 0), 270, 360, r, r, false, 15);
			else				glVertex3f(0, h, 0);

			if (c & UpperRight) arc2_vertices(vector3(w-r, h-r, 0), 0, 90, r, r, false, 15);
			else				glVertex3f(w, h, 0);

			if (c & LowerRight) arc2_vertices(vector3(w-r, r, 0), 90, 180, r, r, false, 15);
			else				glVertex3f(w, 0, 0);

			if (c & LowerLeft)	glVertex3f(r, 0, 0);
			else				glVertex3f(0, 0, 0);
		glEnd();		
	}
	glPopMatrix();
}

//----------------------------------------------------------------------------------------------
// rounded Box
//----------------------------------------------------------------------------------------------
void Draw::roundedBox2(const double *pos, double w, double h, double r, unsigned char c)
{
	glLineWidth(_linewidth);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslated(pos[0], pos[1], _depth+0.01f);

	int anglestep = 90 / r; 
	if (anglestep < 1) anglestep = 1;

	/*fill*/ 
	if (_fillstyle != None)
	{
		fillColor();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_POLYGON);
			if (c & LowerLeft)	arc2_vertices(vector3(r, r, -0.01f), 180, 270, r, r, false, anglestep);
				
			else				glVertex3d(0, 0, -0.01);
			
			if (c & UpperLeft)	arc2_vertices(vector3(r, h-r, -0.01f), 270, 360, r, r, false, anglestep);
			else				glVertex3d(0, h, -0.01);

			if (c & UpperRight) arc2_vertices(vector3(w-r, h-r, -0.01f), 0, 90, r, r, false, anglestep);
			else				glVertex3d(w, h, -0.01);

			if (c & LowerRight) arc2_vertices(vector3(w-r, r, -0.01f), 90, 180, r, r, false, anglestep);
			else				glVertex3d(w, 0, -0.01);

			if (c & LowerLeft)	glVertex3d(r, 0, -0.01);
			else				glVertex3d(0, 0, -0.01);
		glEnd();
	}
	/*outline*/ 
	if (_linestyle != None)
	{
		lineColor();
		glBegin(GL_LINE_STRIP);
			if (c & LowerLeft)	arc2_vertices(vector3(r, r, 0), 180, 270, r, r, false, anglestep);
			else				glVertex3d(0, 0, 0);

			if (c & UpperLeft)	arc2_vertices(vector3(r, h-r, 0), 270, 360, r, r, false, anglestep);
			else				glVertex3d(0, h, 0);

			if (c & UpperRight) arc2_vertices(vector3(w-r, h-r, 0), 0, 90, r, r, false, anglestep);
			else				glVertex3d(w, h, 0);

			if (c & LowerRight) arc2_vertices(vector3(w-r, r, 0), 90, 180, r, r, false, anglestep);
			else				glVertex3d(w, 0, 0);

			if (c & LowerLeft)	glVertex3d(r, 0, 0);
			else				glVertex3d(0, 0, 0);
		glEnd();		
	}
	glPopMatrix();
}
//----------------------------------------------------------------------------------------------
// arc 2
//----------------------------------------------------------------------------------------------
void Draw::arc2_vertices(const float *cen, float start, float end, float rx, float ry, bool line, float anglestep)
{
	float t,x,y; int i=0;
	
	for (t = start; t<=end; t+= anglestep)
	{
		if (line && _linestyle == Dash)
		{
			if (i++ % 2 == 0) backColor();
			else lineColor();
		}
		x = sin(t*DEG2RAD) * rx; y = cos(t*DEG2RAD) * ry;
		glVertex3f(x+cen[0], y+cen[1], cen[2]);
	}
}
//----------------------------------------------------------------------------------------------
// arc 2
//----------------------------------------------------------------------------------------------
void Draw::arc2_vertices(const double *cen, double start, double end, double rx, double ry, bool line, double anglestep)
{
	double t,x,y; int i=0;
	
	for (t = start; t<=end; t+= anglestep)
	{
		if (line && _linestyle == Dash)
		{
			if (i++ % 2 == 0) backColor();
			else lineColor();
		}
		x = sin(t*DEG2RAD) * rx; y = cos(t*DEG2RAD) * ry;
		glVertex3d(x+cen[0], y+cen[1], cen[2]);
	}
}
//----------------------------------------------------------------------------------------------
// Box
//----------------------------------------------------------------------------------------------
void Draw::box2(const float *pos, float width, float height)
{

}
void Draw::line2(const float *start, const float *end, bool start_arrow, bool end_arrow, bool line)
{
	/*calc vector*/ 
	pt::vector2 st, en;
	st.x = start[0]; st.y = start[1];
	en.x = end[0]; en.y = end[1];

	pt::vector2 v(st, en);// v.between(start, end);
	pt::vector2 vp;
	vp.x = -v.y;
	vp.y = v.x;

	v.normalize();
	vp.normalize();

	v.x *= _arrowlength;
	v.y *= _arrowlength;
	vp.x *= _arrowwidth;
	vp.y *= _arrowwidth;

	lineColor();
	glLineWidth(_linewidth);

	/*the line*/ 
	if (line)
	{
		glBegin(GL_LINES);
			glVertex2fv(start);
			glVertex2fv(end);
		glEnd();
	}
	fillColor();

	/*the arrow head*/ 
	if (end_arrow)
	{
		pt::vector2 ah(en), ah2(en);
		ah -= v;
		ah += vp;
		ah2 -= v;
		ah2 -= vp;

		glBegin(GL_TRIANGLES);
			glVertex2fv(end);			
			glVertex2fv(ah);
			glVertex2fv(ah2);
		glEnd();
	}
	if (start_arrow)
	{
		pt::vector2 ah(st), ah2(st);
		ah += v;
		ah += vp;
		ah2 += v;
		ah2 -= vp;

		glBegin(GL_TRIANGLES);
			glVertex2fv(st);			
			glVertex2fv(ah);
			glVertex2fv(ah2);
		glEnd();		
	}	
}

void Draw::line2h(const float *start, const float *end, bool start_arrow, bool end_arrow, bool line)
{
	/*calc vector*/ 
	pt::vector3 st, en;
	st.x = start[0]; st.y = start[1]; st.z = start[2];
	en.x = end[0]; en.y = end[1]; en.z = start[3];

	pt::vector3 v(st, en);// v.between(start, end);
	pt::vector3 vp;
	vp.x = -v.y;
	vp.y = v.x;
	vp.z = v.z;

	/* normalize x, y only */ 
	float w = sqrt(v.x * v.x + v.y * v.y);
	v.x /= w;
	v.y /= w;
	w = sqrt(vp.x * vp.x + vp.y * vp.y);
	vp.x /= w;
	vp.y /= w;

	v.x *= _arrowlength;
	v.y *= _arrowlength;
	vp.x *= _arrowwidth;
	vp.y *= _arrowwidth;

	lineColor();
	glLineWidth(_linewidth);

	/*the line*/ 
	if (line)
	{
		glBegin(GL_LINES);
			glVertex3fv(start);
			glVertex3fv(end);
		glEnd();
	}
	fillColor();

	/*the arrow head*/ 
	if (end_arrow)
	{
		pt::vector3 ah(en), ah2(en);
		ah -= v;
		ah += vp;
		ah2 -= v;
		ah2 -= vp;

		glBegin(GL_TRIANGLES);
			glVertex3fv(end);			
			glVertex3fv(ah);
			glVertex3fv(ah2);
		glEnd();
	}
	if (start_arrow)
	{
		pt::vector3 ah(st), ah2(st);
		ah += v;
		ah += vp;
		ah2 += v;
		ah2 -= vp;

		glBegin(GL_TRIANGLES);
			glVertex3fv(st);			
			glVertex3fv(ah);
			glVertex3fv(ah2);
		glEnd();		
	}	
}
void Draw::line2(const double *start, const double *end, bool start_arrow, bool end_arrow, bool line)
{
	/*calc vector*/ 
	pt::vector2 st, en;
	st.x = start[0]; st.y = start[1];
	en.x = end[0]; en.y = end[1];

	pt::vector2 v(st, en);// v.between(start, end);
	pt::vector2 vp;
	vp.x = -v.y;
	vp.y = v.x;

	v.normalize();
	vp.normalize();

	v.x *= _arrowlength;
	v.y *= _arrowlength;
	vp.x *= _arrowwidth;
	vp.y *= _arrowwidth;

	lineColor();
	glLineWidth(_linewidth);

	/*the line*/ 
	if (line)
	{
		glBegin(GL_LINES);
			glVertex2dv(start);
			glVertex2dv(end);
		glEnd();
	}

	fillColor();

	/*the arrow head*/ 
	if (end_arrow)
	{
		pt::vector2 ah(en), ah2(en);
		ah -= v;
		ah += vp;
		ah2 -= v;
		ah2 -= vp;

		glBegin(GL_TRIANGLES);
			glVertex2dv(end);			
			glVertex2fv(ah);
			glVertex2fv(ah2);
		glEnd();
	}
	if (start_arrow)
	{
		pt::vector2 ah(st), ah2(st);
		ah += v;
		ah += vp;
		ah2 += v;
		ah2 -= vp;

		glBegin(GL_TRIANGLES);
			glVertex2fv(st);			
			glVertex2fv(ah);
			glVertex2fv(ah2);
		glEnd();		
	}	
}

void Draw::line2h(const double *start, const double *end, bool start_arrow, bool end_arrow, bool line)
{
	/*calc vector*/ 
	pt::vector3 st, en;
	st.x = start[0]; st.y = start[1]; st.z = start[2];
	en.x = end[0]; en.y = end[1]; en.z = start[3];

	pt::vector3 v(st, en);// v.between(start, end);
	pt::vector3 vp;
	vp.x = -v.y;
	vp.y = v.x;
	vp.z = v.z;

	/* normalize x, y only */ 
	float w = sqrt(v.x * v.x + v.y * v.y);
	v.x /= w;
	v.y /= w;
	w = sqrt(vp.x * vp.x + vp.y * vp.y);
	vp.x /= w;
	vp.y /= w;

	v.x *= _arrowlength;
	v.y *= _arrowlength;
	vp.x *= _arrowwidth;
	vp.y *= _arrowwidth;

	lineColor();
	glLineWidth(_linewidth);

	/*the line*/ 
	if (line)
	{
		glBegin(GL_LINES);
			glVertex3dv(start);
			glVertex3dv(end);
		glEnd();
	}

	fillColor();

	/*the arrow head*/ 
	if (end_arrow)
	{
		pt::vector3 ah(en), ah2(en);
		ah -= v;
		ah += vp;
		ah2 -= v;
		ah2 -= vp;

		glBegin(GL_TRIANGLES);
			glVertex2dv(end);			
			glVertex2fv(ah);
			glVertex2fv(ah2);
		glEnd();
	}
	if (start_arrow)
	{
		pt::vector3 ah(st), ah2(st);
		ah += v;
		ah += vp;
		ah2 += v;
		ah2 -= vp;

		glBegin(GL_TRIANGLES);
			glVertex2fv(st);			
			glVertex2fv(ah);
			glVertex2fv(ah2);
		glEnd();		
	}	
}
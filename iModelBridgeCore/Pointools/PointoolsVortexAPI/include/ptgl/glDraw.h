#ifndef POINTOOLS_GL_DRAW_ROUTINES
#define POINTOOLS_GL_DRAW_ROUTINES

#include <ptgl/ptgl.h>

namespace ptgl
{
struct PTGL_API Draw
{
	enum Corners { UpperLeft = 1, UpperRight = 2, LowerLeft = 4, LowerRight = 8,
		Left = 5, Right = 10, Top = 3, Bottom = 12, All = 15 };

	static void lineSolid();
	static void lineDash();
	static void lineNone();
	static void lineWidth(float w);
	static void lineOpacity(float a);

	static void fillOpacity(float a);
	static void fillSolid();
	static void fillNone();

	static void lineColor(const float *col);
	static void backColor(const float *col);
	static void fillColor(const float *col);
	static void interactColor(const float *col);

	static void lineColor(float r, float g, float b);
	static void backColor(float r, float g, float b);
	static void fillColor(float r, float g, float b);
	static void interactColor(float r, float g, float b);

	static const float* fillColor();
	static const float* lineColor();
	static const float* backColor();
	static const float* interactColor();

	static void depth(float d);
	static void arrowLength(float l);
	static void arrowWidth(float w);

	static void marker3(const float *pos);
	static void marker3(const double *pos);

	/*2d graphics*/ 
	static void circle2(const float*cen, float rx, float ry, float anglestep=5.0f);
	static void roundedBox2(const float *pos, float width, float height, float radius, unsigned char corners);
	static void box2(const float *pos, float width, float height);
	static void marker2(const float *pos);

	static void line2(const float *start, const float *end, bool start_arrow, bool end_arrow, bool line=true);
	static void line2h(const float *start, const float *end, bool start_arrow, bool end_arrow, bool line=true);

	static void arc2_vertices(const float *cen, float start, float end, float rx, float ry, bool line = true, float anglestep=5.0f);

	/* double versions */ 
	static void circle2(const double*cen, double rx, double ry, double anglestep=5.0f);
	static void roundedBox2(const double *pos, double width, double height, double radius, unsigned char corners);
	static void box2(const double *pos, double width, double height);
	static void marker2(const double *pos);

	static void line2(const double *start, const double *end, bool start_arrow, bool end_arrow, bool line=true);
	static void line2h(const double *start, const double *end, bool start_arrow, bool end_arrow, bool line=true);

	static void arc2_vertices(const double *cen, double start, double end, double rx, double ry, bool line = true, double anglestep=5.0f);
};
}
#endif
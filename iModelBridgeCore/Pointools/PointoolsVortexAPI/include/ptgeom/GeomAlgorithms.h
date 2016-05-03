// GeomAlgorithms.h: interface for the GeomAlgorithms class.
//
//////////////////////////////////////////////////////////////////////

#ifndef PT_GEOMETRY_ALGORITHMS
#define PT_GEOMETRY_ALGORITHMS
#include <pt/geomTypes.h>

struct gaPoint2d
{
	float x;
	float y;
};

class GeomAlgorithms
{
public: 
	GeomAlgorithms();
	virtual ~GeomAlgorithms();
	static double dist(float x1, float y1, float x2, float y2)
	{
		return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
	}
	static double dist(double x1, double y1, double x2, double y2)
	{
		return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
	}
	static int dist(int x1, int y1,int x2,int y2)
	{
		return sqrt((double)((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)));
	}
//------------------------------------------------------------------------------------
	static float TriArea2(	float x1, float x2, float x3,
					float y1, float y2, float y3);
	static float TriArea2(const pt::vector2i &a, const pt::vector2i &b, const pt::vector2i &c);
 	static float TriArea3(const float *a, const float *b, const float *c);
	
//------------------------------------------------------------------------------------
	static float area3D_Polygon( int n, pt::vector3* V, pt::vector3 N );

//------------------------------------------------------------------------------------
	static inline float isLeft( gaPoint2d P0, gaPoint2d P1, gaPoint2d P2 )
	{
		return (P1.x - P0.x)*(P2.y - P0.y) - (P2.x - P0.x)*(P1.y - P0.y);
	};
	static inline float isLeft(double P0[2], double P1[2], double P2[2] )
	{
		return (P1[0] - P0[0])*(P2[1] - P0[1]) - (P2[0] - P0[0])*(P1[1] - P0[1]);
	};
	static inline float isLeft(double x0, double y0, double x1, double y1, double xp, double yp)
	{
		return (x1 - x0)*(yp - y0) - (xp - x0)*(y1 - y0);
	};

//------------------------------------------------------------------------------------
	static int nearHull_2D( gaPoint2d* P, int n, int k, gaPoint2d* H );

// intersection
	template <class T> static inline bool intersect_2D_seg_ray(T ax0, T ay0, T ax1, T ay1, T bx0, T by0, T bx1, T by1,T &ix, T &iy)
	{
		if (intersect_2D_ray_ray(ax0, ay0, ax1, ay1, bx0, by0, bx1, by1, ix, iy)
			&& (((ix >= ax0 && ix <= ax1) || (ix <= ax0 && ix >= ax1)) 
			&& ((iy >= ay0 && iy <= ay1) || (iy <= ay0 && iy >= ay1)))
			) return true;
		return false;
	};
	template <class T> static inline bool intersect_2D_seg_ray(const T &a0, const T &a1, const T &b0, const T &b1, T &i)
	{
		if (intersect_2D_ray_ray(a0,a1,a2,i)
			&& (((i.x >= a.x && i.x <= a1.x) || (i.x <= a0.x && i.x >= a1.x)) 
			&& ((i.y >= a.y && i.y <= a1.y) || (i.y <= a0.y && i.y >= a1.y)))
			) return true;
		return false;
	};
	template <class T> static inline bool intersect_2D_seg_seg(T ax0, T ay0, T ax1, T ay1, T bx0, T by0, T bx1, T by1,T &ix, T &iy)
	{
		if (intersect_2D_ray_ray(ax0, ay0, ax1, ay1, bx0, by0, bx1, by1, ix, iy)
			&&
			(((ix >= ax0 && ix <= ax1) || (ix <= ax0 && ix >= ax1)) &&
			((iy >= ay0 && iy <= ay1) || (iy <= ay0 && iy >= ay1)))	&&	
			(((ix >= bx0 && ix <= bx1) || (ix <= bx0 && ix >= bx1))	&&
			((iy >= by0 && iy <= by1) || (iy <= by0 && iy >= by1)))
			) return true;
		return false;
	};
	template <class T> static inline bool intersect_2D_seg_seg(const T &a0, const T &a1, const T &b0, const T &b1, T &i)
	{
		if (intersect_2D_ray_ray(a0, a1, a2, i)
			&&
			(((i.xx >= a0.x && i.x <= a1.x) || (i.x <= a0.x && i.x >= a1.x)) &&
			((i.y >= a0.y && i.y <= a1.y) || (i.y <= a0.y && i.y >= a1.y)))	&&	
			(((i.x >= b0.x && i.x <= b1.x) || (i.x <= b0.x && i.x >= b1.x))	&&
			((i.y >= b0.x && i.y <= b1.y) || (i.y <= b0.y && i.y >= b1.y)))
			) return true;
		return false;
	};
	template <class T> static inline bool intersect_2D_ray_ray(T ax0, T ay0, T ax1, T ay1, T bx0, T by0, T bx1, T by1,T &ix, T &iy)
	{
		T r = ((ay0-by0)*(bx1-bx0) - (ax0-bx0)*(by1-by0)) / ((ax1-ax0)*(by1-by0)-(ay1-ay0)*(bx1-bx0));
		ix = ax0+r*(ax1-ax0);
		iy = ay0+r*(ay1-ay0);		

		/*check if parrallel*/ 
		return true;
	};
	template <class T> static inline bool intersect_2D_ray_ray(const T &a0, const T &a1, const T &b0, const T &b1, T &i)
	{
		float r = ((a0.y-b0.y)*(b1.x-b0.x) - (a0.x-b0.x)*(b1.y-b0.y)) / ((a1.x-a0.x)*(b1.y-b0.y)-(a1.y-a0.y)*(b1.x-b0.x));
		i.x = a0.x+r*(a1.x-a0.x);
		i.y = a0.y+r*(a1.y-a0.y);		

		return true;
	};
	//int
	static inline bool intersect_2D(const int &ax0, const int &ay0, const int &ax1, const int &ay1,
									const int &bx0, const int &by0, const int &bx1, const int &by1,
									int &ix, int &iy)
	{
		double r = (double)((ay0-by0)*(bx1-bx0) - (ax0-bx0)*(by1-by0)) / (double)((ax1-ax0)*(by1-by0)-(ay1-ay0)*(bx1-bx0));
		ix = ax0+r*(ax1-ax0);
		iy = ay0+r*(ay1-ay0);
		//check for segment intersection
		if ((((ix >= ax0 && ix < ax1) || (ix <= ax0 && ix > ax1)) 
			&&
			((iy >= ay0 && iy < ay1) || (iy <= ay0 && iy > ay1)))
			&&	
			(((ix >= bx0 && ix < bx1) || (ix <= bx0 && ix > bx1)) 
			&&
			((iy >= by0 && iy < by1) || (iy <= by0 && iy > by1)))
			) return true;
		return false;
	};
	//int
	static inline bool intersect_2D(  int ax0,   int ay0,   int ax1,   int ay1,
									  int bx0,   int by0,   int bx1,   int by1, int &ix, int &iy)
	{
		double r = (double)((ay0-by0)*(bx1-bx0) - (ax0-bx0)*(by1-by0)) / (double)((ax1-ax0)*(by1-by0)-(ay1-ay0)*(bx1-bx0));
		ix = ax0+r*(ax1-ax0);
		iy = ay0+r*(ay1-ay0);
		//check for segment intersection
		if ((((ix >= ax0 && ix <= ax1) || (ix <= ax0 && ix >= ax1)) 
			&&
			((iy >= ay0 && iy <= ay1) || (iy <= ay0 && iy >= ay1)))
			&&	
			(((ix >= bx0 && ix <= bx1) || (ix <= bx0 && ix >= bx1)) 
			&&
			((iy >= by0 && iy <= by1) || (iy <= by0 && iy >= by1)))
			) return true;
		return false;
	};

//------------------------------------------------------------------------------------
/*
	  Purpose:

		ENORMSQ0_2D computes the square of the Euclidean norm of (P1-P0) in 2D.

	  Modified:

		18 April 1999

	  Author:

		John Burkardt

	  Parameters:

		Input, float X0, Y0, X1, Y1, the coordinates of the points 
		P0 and P1.

		Output, float ENORMSQ0_2D, the square of the Euclidean norm of (P1-P0).
	*/
//------------------------------------------------------------------------------------
	static inline float enormsq0_2d ( float x0, float y0, float x1, float y1 )
	{
	  float value;

	  value = 
		( x1 - x0 ) * ( x1 - x0 ) + 
		( y1 - y0 ) * ( y1 - y0 );
 
	  return value;
	}
//------------------------------------------------------------------------------------
/*
  Purpose:

    ENORM0_2D computes the Euclidean norm of (P1-P0) in 2D.

  Modified:

    18 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X0, Y0, X1, Y1, the coordinates of the points P0 and P1.

    Output, float ENORM0_2D, the Euclidean norm of (P1-P0).
*/
	static inline float enorm0_2d ( float x0, float y0, float x1, float y1 )
	{
		float value;

		value = sqrt (
			( x1 - x0 ) * ( x1 - x0 ) + 
			( y1 - y0 ) * ( y1 - y0 ) );
 
		return value;
	}
//------------------------------------------------------------------------------------
/*
  Purpose:

    LINE_EXP_POINT_DIST_2D: distance ( explicit line, point ) in 2D.

  Formula:

    The explicit form of a line in 2D is:

      (X1,Y1), (X2,Y2).

  Modified:

    22 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2.  (X1,Y1) and (X2,Y2) are two points on
    the line.

    Input, float X, Y, the point whose distance from the line is
    to be measured.

    Output, float LINE_EXP_DIST_2D, the distance from the point to the line.
*/
//------------------------------------------------------------------------------------
	static float line_exp_point_dist_2d ( float x1, float y1, float x2, float y2, float x, float y )
	{
	  float bot, dist, dot, t, xn, yn;
	  bot = enormsq0_2d ( x1, y1, x2, y2 );

	  if ( bot == 0.0 ) {

		xn = x1;
		yn = y1;
	  }

	 // (P-P1) dot (P2-P1) = Norm(P-P1) * Norm(P2-P1) * Cos(Theta).
	 //
	 // (P-P1) dot (P2-P1) / Norm(P-P1)**2 = normalized coordinate T
	 // of the projection of (P-P1) onto (P2-P1).
	
	  else {

		dot =
			( x - x1 ) * ( x2 - x1 )
		  + ( y - y1 ) * ( y2 - y1 );

		t = dot / bot;

		xn = x1 + t * ( x2 - x1 );
		yn = y1 + t * ( y2 - y1 );

	  }

	  dist = enorm0_2d ( xn, yn, x, y );

	  return dist;
	};
//
	template <class T> static inline bool Point_on_Segment_2D(T x_1, T y_1, T x_2, T y_2, 
		T p_x, T p_y, T dist, T tolerance)
	{
		float ux, lx, uy, ly;
		if (x_1 >= x_2) { ux = x_1; lx = x_2; }
		else			{ ux = x_2; lx = x_1; }
		if (y_1 >= y_2)	{ uy = y_1; ly = y_2; }
		else			{ uy = y_2; ly = y_1; }

		lx -= tolerance;
		ux += tolerance;
		ly -= tolerance;
		uy += tolerance;

		if ((p_x >= lx && p_x <= ux) && (p_y >= ly && p_y <= uy))
		{
			dist = line_exp_point_dist_2d(x_1, y_1, x_2, y_2, p_x, p_y);
			
			if (dist < tolerance) return true;
		}
		return false;
	};
//
	static inline bool Point_on_Segment_2D(int x_1, int y_1, int x_2, int y_2, 
		int p_x, int p_y, float &dist, const float tolerance)
	{
		int ux, lx, uy, ly;
		if (x_1 >= x_2) { ux = x_1; lx = x_2; }
		else			{ ux = x_2; lx = x_1; }
		if (y_1 >= y_2)	{ uy = y_1; ly = y_2; }
		else			{ uy = y_2; ly = y_1; }

		lx -= tolerance;
		ux += tolerance;
		ly -= tolerance;
		uy += tolerance;

		if ((p_x >= lx && p_x <= ux) && (p_y >= ly && p_y <= uy))
		{
			dist = line_exp_point_dist_2d((float)x_1, (float)y_1, (float)x_2, (float)y_2, (float)p_x, (float)p_y);
			
			if (dist < tolerance) return true;
		}
		return false;
	};
//
	static double polyArea2d(gaPoint2d *polygon,int N);
};

#endif // !defined(AFX_GEOMALGORITHMS_H__08CC8CAA_8C8D_4BA5_8466_A36FB8A0923E__INCLUDED_)

/*  geometryc.c  22 January 2001 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mathimf.h>
#include <sys/types.h>
#include <time.h>

#include <ptgeom/geom.h>

/**********************************************************************/

float angle_deg_2d ( float x1, float y1, float x2, float y2, 
  float x3, float y3 )

/**********************************************************************/

/*
  Purpose:

    ANGLE_DEG_2D returns the angle in degrees swept out between two rays in 2D.

  Discussion:

    Except for the zero angle case, it should be 1 that

    ANGLE_DEG_2D(X1,Y1,X2,Y2,X3,Y3)
    + ANGLE_DEG_2D(X3,Y3,X2,Y2,X1,Y1) = 360.0

  Modified:

    19 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, X3, Y3, define the rays
    ( X1-X2, Y1-Y2 ) and ( X3-X2, Y3-Y2 ) which in turn define the
    angle, counterclockwise from ( X1-X2, Y1-Y2 ).

    Output, float ANGLE_DEG_2D, the angle swept out by the rays, measured
    in degrees.  0 <= ANGLE_DEG_2D < 360.  If either ray has zero length,
    then ANGLE_DEG_2D is set to 0.
*/
{
  float angle_rad_2d;
  float value;
  float x;
  float y;

  x = ( x1 - x2 ) * ( x3 - x2 ) + ( y1 - y2 ) * ( y3 - y2 );
  y = ( x1 - x2 ) * ( y3 - y2 ) - ( y1 - y2 ) * ( x3 - x2 );

  if ( x == 0.0 && y == 0.0 ) {
    value = 0.0;
  }
  else {

    angle_rad_2d = atan2 ( y, x );

    if ( angle_rad_2d < 0.0 ) {
      angle_rad_2d = angle_rad_2d + 2.0 * PI;
    }

    value = angle_rad_2d * RAD_TO_DEG;

  }

  return value;
}
/**********************************************************************/

float angle_rad_2d ( float x1, float y1, float x2, float y2, 
  float x3, float y3 )

/**********************************************************************/

/*
  Purpose:

    ANGLE_RAD_2D returns the angle in radians swept out between two rays in 2D.

  Discussion:

    Except for the zero angle case, it should be 1 that

    ANGLE_RAD_2D(X1,Y1,X2,Y2,X3,Y3)
    + ANGLE_RAD_2D(X3,Y3,X2,Y2,X1,Y1) = 2 * PI

  Modified:

    19 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, X3, Y3, define the rays
    ( X1-X2, Y1-Y2 ) and ( X3-X2, Y3-Y2 ) which in turn define the
    angle, counterclockwise from ( X1-X2, Y1-Y2 ).

    Output, float ANGLE_RAD_2D, the angle swept out by the rays, measured
    in radians.  0 <= ANGLE_DEG_2D < 2 PI.  If either ray has zero length,
    then ANGLE_RAD_2D is set to 0.
*/
{
  float value;
  float x;
  float y;

  x = ( x1 - x2 ) * ( x3 - x2 ) + ( y1 - y2 ) * ( y3 - y2 );
  y = ( x1 - x2 ) * ( y3 - y2 ) - ( y1 - y2 ) * ( x3 - x2 );

  if ( x == 0.0 && y == 0.0 ) {
    value = 0.0;
  }
  else {

    value = atan2 ( y, x );

    if ( value < 0.0 ) {
      value = value + 2.0 * PI;
    }

  }

  return value;
}
/**********************************************************************/

float angle_rad_3d ( float x1, float y1, float z1, float x2, float y2, 
  float z2, float x3, float y3, float z3 )

/**********************************************************************/

/*
  Purpose:

    ANGLE_RAD_3D returns the angle between two vectors in 3D.

  Discussion:

    The routine always computes the SMALLER of the two angles between
    two vectors.  Thus, if the vectors make an (exterior) angle of 200 
    degrees, the (interior) angle of 160 is reported.

    Thanks to Laszlo Molnar, molnarl@index.hu, for pointing out a 
    typographical error.

  Formula:

    X dot Y = Norm(X) * Norm(Y) * Cos ( Angle(X,Y) )

  Modified:

    22 January 2001

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, are three points
    which define the vectors.  The vectors are:
    ( X1-X2, Y1-Y2, Z1-Z2 ) and ( X3-X2, Y3-Y2, Z3-Z2 ).

    Output, float ANGLE_RAD_3D, the angle between the two vectors, in radians.
    This value will always be between 0 and Pi.  If either vector has
    zero length, then the angle is returned as zero.
*/
{
  float dot;
  float v1norm;
  float v2norm;
  float value;

  dot = dot0_3d ( x2, y2, z2, x1, y1, z1, x3, y3, z3 );

  v1norm = enorm0_3d ( x1, y1, z1, x2, y2, z2 );

  v2norm = enorm0_3d ( x3, y3, z3, x2, y2, z2 );

  if ( v1norm == 0.0 || v2norm == 0.0 ) {
    value = 0.0;
  }
  else {
    value = acos ( dot / ( v1norm * v2norm ) );
  }

  return value;
}
/**********************************************************************/

float angle_rad_nd ( int n, float vec1[], float vec2[] )

/**********************************************************************/

/*
  Purpose:

    ANGLE_RAD_ND returns the angle between two vectors in ND.

  Discussion:

    ANGLE_RAD_ND always computes the SMALLER of the two angles between
    two vectors.  Thus, if the vectors make an (exterior) angle of 
    1.5 PI radians, then the (interior) angle of 0.5 radians is returned.

  Formula:

    X dot Y = Norm(X) * Norm(Y) * Cos( Angle(X,Y) )

  Modified:

   19 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vectors.

    Input, float VEC1[N], VEC2[N], the two vectors to be considered.

    Output, float ANG_RAD_ND, the angle between the vectors, in radians.
    This value will always be between 0 and PI.
*/
{
  float dot;
  float v1norm;
  float v2norm;
  float value;

  dot = dot_nd ( n, vec1, vec2 );

  v1norm = enorm_nd ( n, vec1 );

  v2norm = enorm_nd ( n, vec2 );

  if ( v1norm == 0.0 || v2norm == 0.0 ) {

    value = 0.0;

  }
  else {

    value = acos ( dot / ( v1norm * v2norm ) );

  }

  return value;
}
/**********************************************************************/

float anglei_deg_2d ( float x1, float y1, float x2, float y2, float x3, 
  float y3 )

/**********************************************************************/

/*
  Purpose:

    ANGLEI_DEG_2D returns the interior angle in degrees between two rays in 2D.

  Modified:

    19 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, X3, Y3, define the rays
    (X1-X2,Y1-Y2) and (X3-X2,Y3-Y2) which in turn define the angle.

    Output, float ANGLEI_DEG_2D, the angle swept out by the rays, measured
    in degrees.  This value satisfies 0 <= ANGLEI_DEG_2D < 180.0.  If either
    ray is of zero length, then ANGLEI_deg_2D is returned as 0.
*/
{
  float value;
  float x;
  float y;

  x = ( x1 - x2 ) * ( x3 - x2 ) + ( y1 - y2 ) * ( y3 - y2 );
  y = ( x1 - x2 ) * ( y3 - y2 ) - ( y1 - y2 ) * ( x3 - x2 );

  if ( x == 0.0 && y == 0.0 ) {
    value = 0.0;
  }
  else {

    value = atan2 ( y, x );

    if ( value < 0.0 ) {
      value = value + 2.0 * PI;
    }

    value = value * RAD_TO_DEG;

    if ( value > 180.0 ) {
      value = 360.0 - value;
    }

  }
  return value;
}
/**********************************************************************/

float anglei_rad_2d ( float x1, float y1, float x2, float y2, float x3, 
  float y3 )

/**********************************************************************/

/*
  Purpose:

    ANGLEI_RAD_2D returns the interior angle in radians between two rays in 2D.

  Modified:

    19 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, X3, Y3, define the rays
    (X1-X2,Y1-Y2) and (X3-X2,Y3-Y2) which in turn define the angle.

    Output, float ANGLEI_RAD_2D, the angle swept out by the rays, measured
    in degrees.  This value satisfies 0 <= ANGLEI_RAD_2D < PI.  If either
    ray is of zero length, then ANGLEI_RAD_2D is returned as 0.
*/
{
  float value;
  float x;
  float y;

  x = ( x1 - x2 ) * ( x3 - x2 ) + ( y1 - y2 ) * ( y3 - y2 );
  y = ( x1 - x2 ) * ( y3 - y2 ) - ( y1 - y2 ) * ( x3 - x2 );

  if ( x == 0.0 && y == 0.0 ) {
    value = 0.0;
  }
  else {

    value = atan2 ( y, x );

    if ( value < 0.0 ) {
      value = value + 2.0 * PI;
    }

    if ( value > PI ) {
      value = 2.0 * PI - value;
    }

  }
  return value;
}
/**********************************************************************/

int box_contains_point_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float x3, float y3, float z3, float x4, float y4, 
  float z4, float x, float y, float z )

/**********************************************************************/

/*
  Purpose:

    BOX_CONTAINS_POINT_3D determines if a point is inside a parallelepiped in 3D.

  Definition:

    A parallelepiped is a "slanted box", that is, opposite
    sides are parallel planes.

  Diagram:

               *------------------*
              / \                / \
             /   \              /   \
            /     \            /     \
      (X4,Y4,Z4)--------------*       \
            \        .         \       \
             \        .         \       \
              \        .         \       \
               \   (X2,Y2,Z2).....\-------\
                \     /            \     /
                 \   /              \   /
                  \ /                \ /
            (X1,Y1,Z1)-----------(X3,Y3,Z3)

  Modified:

    20 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, X4, Y4, Z4, the
    coordinates of four corners of the parallelepiped, which we will
    call P1, P2, P3 and P4.  It is assumed that P2, P3 and P4 are
    immediate neighbors of P1.

    Input, float X, Y, Z, the point to be checked.

    Output, int BOX_CONTAINS_POINT_3D, is 1 if (X,Y,Z) is inside the
    parallelepiped, or on its boundary, and 0 otherwise.
*/
{
  float dot;
  float normsq;
 
  dot = dot0_3d ( x1, y1, z1, x2, y2, z2, x, y, z );

  if ( dot < 0.0 ) {
    return 0;
  }

  normsq = enormsq0_3d ( x1, y1, z1, x2, y2, z2 );

  if ( normsq < dot ) {
    return 0;
  }

  dot = dot0_3d ( x1, y1, z1, x3, y3, z3, x, y, z );

  if ( dot < 0.0 ) {
    return 0;
  }

  normsq = enormsq0_3d ( x1, y1, z1, x3, y3, z3 );

  if ( normsq < dot ) {
    return 0;
  }

  dot = dot0_3d ( x1, y1, z1, x4, y4, z4, x, y, z );

  if ( dot < 0.0 ) {
    return 0;
  }

  normsq = enormsq0_3d ( x1, y1, z1, x4, y4, z4 );

  if ( normsq < dot ) {
    return 0;
  }

  return 1;
}
/**********************************************************************/

float box_point_dist_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float x3, float y3, float z3, float x4, float y4, 
  float z4, float x, float y, float z )

/**********************************************************************/

/*
  Purpose:

    BOX_POINT_DIST_3D: distance ( parallelepiped, point ) in 3D.

  Definition:

    A parallelepiped is a "slanted box", that is, opposite
    sides are parallel planes.

  Diagram:

       7----8
      /|   /|
     / 3--/-5
    4----6 /
    |/   |/
    1----2

  Modified:

    20 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, X4, Y4, Z4, half of
    the corners of the box, from which the other corners can be
    deduced.  The corners should be chosen so that the first corner
    is directly connected to the other three.  The locations of
    corners 5, 6, 7 and 8 will be computed by the parallelogram
    relation.

    Input, float X, Y, Z, the point which is to be checked.

    Output, float BOX_POINT_DIST_3D, the distance from the point to the box.  
    The distance is zero if the point lies exactly on the box.
*/
{
  float dis;
  float dist;
  float x5;
  float x6;
  float x7;
  float x8;
  float y5;
  float y6;
  float y7;
  float y8;
  float z5;
  float z6;
  float z7;
  float z8;
/*
  Fill in the other corners
*/
  x5 = x2 + x3 - x1;
  y5 = y2 + y3 - y1;
  z5 = z2 + z3 - z1;

  x6 = x2 + x4 - x1;
  y6 = y2 + y4 - y1;
  z6 = z2 + z4 - z1;

  x7 = x3 + x4 - x1;
  y7 = y3 + y4 - y1;
  z7 = z3 + z4 - z1;

  x8 = x2 + x3 + x4 - 2.0 * x1;
  y8 = y2 + y3 + y4 - 2.0 * y1;
  z8 = z2 + z3 + z4 - 2.0 * z1;
/*
  Compute the distance from the point ( X, Y, Z ) to each of the six
  paralleogram faces.
*/
  dis = para_point_dist_3d ( x1, y1, z1, x2, y2, z2, x3, y3, z3, x, y, z );

  dist = dis;

  dis = para_point_dist_3d ( x1, y1, z1, x2, y2, z2, x4, y4, z4, x, y, z );

  if ( dis < dist ) {
    dist = dis;
  }

  dis = para_point_dist_3d ( x1, y1, z1, x3, y3, z3, x4, y4, z4, x, y, z );

  if ( dis < dist ) {
    dist = dis;
  }

  dis = para_point_dist_3d ( x8, y8, z8, x5, y5, z5, x6, y6, z6, x, y, z );

  if ( dis < dist ) {
    dist = dis;
  }

  dis = para_point_dist_3d ( x8, y8, z8, x5, y5, z5, x7, y7, z7, x, y, z );

  if ( dis < dist ) {
    dist = dis;
  }

  dis = para_point_dist_3d ( x8, y8, z8, x6, y6, z6, x7, y7, z7, x, y, z );

  if ( dis < dist ) {
    dist = dis;
  }

  return dist;
}
/**********************************************************************/

void circle_dia2imp_2d ( float x1, float y1, float x2, float y2, float *r, 
  float *xc, float *yc )

/**********************************************************************/

/*
  Purpose:

    CIRCLE_DIA2IMP_2D converts a diameter to an implicit circle in 2D.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, are the X and Y coordinates
    of two points which form a diameter of the circle.

    Output, float *R, the computed radius of the circle.

    Output, float *XC, *YC, the computed center of the circle.
*/
{
  *r = 0.5 * enorm0_2d ( x1, y1, x2, y2 );

  *xc = 0.5 * ( x1 + x2 );
  *yc = 0.5 * ( y1 + y2 );

  return;
}
/**********************************************************************/

int circle_exp_contains_point_2d ( float x1, float y1, float x2, float y2, 
  float x3, float y3, float x, float y )

/**********************************************************************/

/*
  Purpose:

    CIRCLE_EXP_CONTAINS_POINT_2D determines if an explicit circle contains a point in 2D.

  Formula:

    The explicit form of a circle in 2D is:

  (X1,Y1), (X2,Y2), (X3,Y3).

  Modified:

    21 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, X3, Y3, the (X,Y) coordinates of three
    points that lie on a circle.

    Input, float X, Y, the (X,Y) coordinates of a point, whose position
    relative to the circle is desired.

    Output, int CIRCLE_EXP_CONTAINS_POINT_2D:

   -1, the three points are distinct and noncolinear,
    and (x,y) lies inside the circle.
    0, the three points are distinct and noncolinear,
    and (x,y) lies on the circle.
    1, the three points are distinct and noncolinear,
    and (x,y) lies outside the circle.

    2, the three points are distinct and colinear,
    and (x,y) lies on the line.
    3, the three points are distinct and colinear,
    and (x,y) does not lie on the line.

    4, two points are distinct, and (x,y) lies on the line.
    5, two points are distinct, and (x,y) does not lie on the line.

    6, all three points are equal, and (x,y) is equal to them,
    7, all three points are equal, and (x,y) is not equal to them.
*/
{
  float a[4][4];
  float det;
  int inside;
/*
  P1 = P2?
*/
  if ( x1 == x2 && y1 == y2 ) {

    if ( x1 == x3 && y1 == y3 ) {

      if ( x1 == x && y1 == y ) {
        inside = 6;
      }
      else {
        inside = 7;
      }

    }
    else {

      det = ( x1 - x3 ) * ( y - y3 ) - ( x - x3 ) * ( y1 - y3 );

      if ( det == 0.0 ) {
        inside = 4;
      }
      else {
        inside = 5;
      }
    }

    return inside;

  }
/*
  P1 does not equal P2.  Does P1 = P3?
*/
  if ( x1 == x3 && y1 == y3 ) {

    det = ( x1 - x2 ) * ( y - y2 ) - ( x - x2 ) * ( y1 - y2 );

    if ( det == 0.0 ) {
      inside = 4;
    }
    else {
      inside = 5;
    }

    return inside;

  }
/*
  The points are distinct.  Are they colinear?
*/
  det = ( x1 - x2 ) * ( y3 - y2 ) - ( x3 - x2 ) * ( y1 - y2 );

  if ( det == 0.0 ) {

    det = ( x1 - x2 ) * ( y - y2 ) - ( x - x2 ) * ( y1 - y2 );

    if ( det == 0.0 ) {
      inside = 2;
    }
    else {
      inside = 3;
    }

    return inside;

  }
/*
  The points are distinct and non-colinear.

  Compute the determinant
*/
  a[0][0] = x1;
  a[0][1] = y1;
  a[0][2] = x1 * x1 + y1 * y1;
  a[0][3] = 1.0;

  a[1][0] = x2;
  a[1][1] = y2;
  a[1][2] = x2 * x2 + y2 * y2;
  a[1][3] = 1.0;

  a[2][0] = x3;
  a[2][1] = y3;
  a[2][2] = x3 * x3 + y3 * y3;
  a[2][3] = 1.0;

  a[3][0] = x;
  a[3][1] = y;
  a[3][2] = x * x + y * y;
  a[3][3] = 1.0;

  det = rmat4_det ( a );

  if ( det < 0.0 ) {
    inside = 1;
  }
  else if ( det == 0.0 ) {
    inside = 0;
  }
  else {
    inside = -1;
  }

  return inside;
}

/**********************************************************************/

int circle_imp_contains_point_2d ( float radius, float xc, float yc,
  float x, float y )

/**********************************************************************/

/*

  Purpose:

    CIRCLE_IMP_CONTAINS_POINT_2D determines if an implicit circle contains a point in 2D.

  Formula:

    An implicit circle in 2D satisfies the equation:

  ( X - XC )**2 + ( Y - YC )**2 = R**2

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float RADIUS, the radius of the circle.

    Input, float XC, YC, the coordinates of the center of the circle.

    Input, float X, Y, the point to be checked.

    Output, logical CIRCLE_IMP_CONTAINS_POINT_2D, is 1 if the point 
    is inside or on the circle, 0 otherwise.
*/
{
  if ( enorm0_2d ( x, y, xc, yc ) <= radius ) {
    return 1;
  }
  else {
    return 0;
  }
}
/**********************************************************************/

void circle_imp_line_par_int_2d ( float radius, float xc, float yc,
  float x0, float y0, float f, float g, int *nint, float x[], float y[] )

/**********************************************************************/

/*
  Purpose:

    CIRCLE_IMP_LINE_PAR_INT_2D: ( implicit circle, parametric line ) intersection in 2D.

  Formula:

    An implicit circle in 2D satisfies the equation:

      ( X - XC )**2 + ( Y - YC )**2 = R**2

    The parametric form of a line in 2D is:

      X = X0 + F * T
      Y = Y0 + G * T

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float RADIUS, the radius of the circle.

    Input, float XC, YC, the coordinates of the center of the circle.

    Input, float F, G, X0, Y0, the parametric parameters of the line.

    Output, int *NINT, the number of intersecting points found.
    NINT will be 0, 1 or 2.

    Output, float X[2], Y[2], contains the X and Y coordinates of
    the intersecting points.
*/
{
  float root;
  float t;

  root = radius * radius * ( f * f + g * g )
    - ( f * ( yc - y0 ) - g * ( xc - x0 ) ) 
    * ( f * ( yc - y0 ) - g * ( xc - x0 ) );

  if ( root < 0.0 ) {

    *nint = 0;

  }
  else if ( root == 0.0 ) {

    *nint = 1;

    t = ( f * ( xc - x0 ) + g * ( yc - y0 ) ) / ( f * f + g * g );
    x[0] = x0 + f * t;
    y[0] = y0 + g * t;

  }
  else if ( root > 0.0 ) {

    *nint = 2;

    t = ( ( f * ( xc - x0 ) + g * ( yc - y0 ) ) - sqrt ( root ) )
      / ( f * f + g * g );

    x[0] = x0 + f * t;
    y[0] = y0 + g * t;

    t = ( ( f * ( xc - x0 ) + g * ( yc - y0 ) ) + sqrt ( root ) )
      / ( f * f + g * g );

    x[1] = x0 + f * t;
    y[1] = y0 + g * t;

  }

  return;
}

/**********************************************************************/

void circle_points_2d ( int n, float x[], float y[] )

/**********************************************************************/

/*
  Purpose:

    CIRCLE_POINTS_2D returns N equally spaced points on the circle in 2D.

  Note:

    The first point is always (1,0) and subsequent points proceed
    counterclockwise around the circle.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of points desired.  N must be at least 1.

    Output, float X[N], Y[N], the X and Y coordinates of the
    N equally spaced points.
*/
{
  float angle;
  int i;

  if ( n <= 0 ) {
    printf ( "\n" );
    printf ( "CIRCLE_POINTS_2D - Fatal error\n" );
    printf ( "  The input value of N is %d\n", n );
    printf ( "  N must be at least 1.\n" );
    exit ( EXIT_FAILURE );
  }

  for ( i = 0; i < n; i++ ) {
    angle = ( 2.0 * PI * ( float ) i ) / ( float ) n ;
    x[i] = cos ( angle );
    y[i] = sin ( angle );
  }

  return;
}
/**********************************************************************/

float cone_area_3d ( float h, float r ) {

/**********************************************************************/

/*
  Purpose:

    CONE_AREA_3D computes the surface area of a right circular cone in 3D.

  Modified:

    22 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float H, R, the height of the cone, and the radius of the
    circle that forms the base of the cone.

    Output, float CONE_AREA_3D, the surface area of the cone.
*/
  float area;

  area = PI * r * sqrt ( h * h + r * r );

  return area;
}
/**********************************************************************/

float cone_volume_3d ( float h, float r )

/**********************************************************************/

/*
  Purpose:

    CONE_VOLUME_3D computes the volume of a right circular cone in 3D.

  Modified:

    22 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float H, R, the height of the cone, and the radius of the
    circle that forms the base of the cone.

    Output, float CONE_VOLUME_3D, the volume of the cone.
*/
{
  float volume;

  volume = PI * r * r * h / 3.0;

  return volume;
}
/**********************************************************************/

float cot_rad ( float angle )

/**********************************************************************/

/*
  Purpose:

    COT_RAD returns the cotangent of an angle.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float ANGLE, the angle, in radians.

    Output, float COT_RAD, the cotangent of the angle.
*/
{
  float value;

  value = cos ( angle ) / sin ( angle );

  return value;
}
/**********************************************************************/

float cot_deg ( float angle )

/**********************************************************************/

/*
  Purpose:

    COT_DEG returns the cotangent of an angle given in degrees.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float ANGLE, the angle, in degrees.

    Output, float COT_DEG, the cotangent of the angle.
*/
{
  float angle_rad;
  float value;

  angle_rad = angle * DEG_TO_RAD;

  value  = cos ( angle_rad ) / sin ( angle_rad );

  return value;
}
/***********************************************************************/
 
float cross_2d ( float x1, float y1, float x2, float y2 )

/***********************************************************************/

/*
  Purpose:
 
    CROSS_2D finds the cross product of a pair of vectors in 2D.

  Discussion:

    Strictly speaking, the vectors lie in the (X,Y) plane, and
    the cross product here is a vector in the Z direction.

  Modified:

    19 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, the X and Y coordinates of the vectors.

    Output, float CROSS_2D, the Z component of the cross product
    of (X1,Y1,0) and (X2,Y2,0).
*/
{
  float value;

  value = x1 * y2 - y1 * x2;

  return value;
}
/**********************************************************************/

void cross_3d ( float x1, float y1, float z1, float x2, float y2, float z2, 
  float *x3, float *y3, float *z3 )

/**********************************************************************/

/*
  Purpose:

    CROSS_3D computes the cross product of two vectors in 3D.

  Modified:

    19 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, the coordinates of the vectors.

    Output, float *X3, *Y3, *Z3, the cross product vector.
*/
{
  *x3 = y1 * z2 - z1 * y2;
  *y3 = z1 * x2 - x1 * z2;
  *z3 = x1 * y2 - y1 * x2;

  return;
}
/***********************************************************************/
 
float cross0_2d ( float x0, float y0, float x1, float y1, float x2, 
  float y2 )

/***********************************************************************/

/*
  Purpose:
 
    CROSS0_2D finds the cross product of a pair of vectors in 2D.

  Discussion:

    Strictly speaking, the vectors lie in the (X,Y) plane, and
    the cross product here is a vector in the Z direction.

    The vectors are specified with respect to a basis point P0.
    We are computing the normal to the triangle (P0,P1,P2).

  Modified:

    19 April 1999

  Author:

    John Burkardt

  Parameters:

    Output, float X3, Y3, Z3, the cross product (P1-P0) x (P2-P0), or
    (X1-X0,Y1-Y0,Z1-Z0) x (X2-X0,Y2-Y0,Z2-Z0).

    Input, float X0, Y0, X1, Y1, X2, Y2, the coordinates of the three
    points.  The basis point P0 is (X0,Y0).

    Output, float CROSS0_2D, the Z component of the cross product
    (X1-X0,Y1-Y0,0) x (X2-X0,Y2-Y0,0).
*/
{
  float value;
 
  value =
      ( x1 - x0 ) * ( y2 - y0 )
    - ( y1 - y0 ) * ( x2 - x0 );

  return value;
}
/***********************************************************************/

void cross0_3d ( float x0, float y0, float z0, float x1, float y1, float z1,
  float x2, float y2, float z2, float *x3, float *y3, float *z3 )
 
/***********************************************************************/

/*
  Purpose:
 
    CROSS0_3D computes the cross product of two vectors in 3D.

  Discussion:

    The vectors are specified with respect to a basis point P0.
    We are computing the normal to the triangle (P0,P1,P2).

  Modified:

    19 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X0, Y0, Z0, X1, Y1, Z1, X2, Y2, Z2, the coordinates of
    three points.  The basis point is (X0,Y0,Z0).

    Output, float *X3, *Y3, *Z3, the cross product (P1-P0) x (P2-P0), or
    (X1-X0,Y1-Y0,Z1-Z0) x (X2-X0,Y2-Y0,Z2-Z0).
*/
{
  *x3 =
      ( y1 - y0 ) * ( z2 - z0 )
    - ( z1 - z0 ) * ( y2 - y0 );
 
  *y3 =
      ( z1 - z0 ) * ( x2 - x0 )
    - ( x1 - x0 ) * ( z2 - z0 );
 
  *z3 =
      ( x1 - x0 ) * ( y2 - y0 )
    - ( y1 - y0 ) * ( x2 - x0 );

  return;
}
/**********************************************************************/

void direction_pert_3d ( float sigma, int *iseed, float vbase[3], 
  float vran[3] )

/**********************************************************************/

/*
  Purpose:

    DIRECTION_PERT_3D randomly perturbs a direction vector in 3D.

  Modified:

    17 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float SIGMA, determines the strength of the perturbation.
    SIGMA <= 0 results in a completely random direction.
    1 <= SIGMA results in VBASE.
    0 < SIGMA < 1 results in a perturbation from VBASE, which is
    large when SIGMA is near 0, and small when SIGMA is near 1.

    Input/output, int *ISEED, a seed for the random number generator.

    Input, float VBASE[3], the base direction vector, which should have
    unit norm.

    Output, float VRAN[3], the perturbed vector, which will have unit norm.
*/
{
  float dphi;
  int i;
  float phi;
  float psi;
  float theta;
  float vdot;
  float x;
  float x2;
  float y;
  float y2;
  float z;
  float z2;
/*
  SIGMA >= 0, just use the base vector.
*/
  if ( sigma >= 1.0 ) {

    for ( i = 0; i < 3; i++ ) {
      vran[i] = vbase[i];
    }

  }
  else if ( sigma <= 0.0 ) {

    vdot = 2.0 * uniform_01_sample ( iseed ) - 1.0;
    phi = acos ( vdot );
    theta = 2.0 * PI * uniform_01_sample ( iseed );

    vran[0] = cos ( theta ) * sin ( phi );
    vran[1] = sin ( theta ) * sin ( phi );
    vran[2] = cos ( phi );

  }
  else {

    phi = acos ( vbase[2] );
    theta = atan2 ( vbase[1], vbase[0] );
/*
  Pick VDOT, which must be between -1 and 1.  This represents
  the dot product of the perturbed vector with the base vector.

  UNIFORM_01_SAMPLE returns a uniformly random value between 0 and 1.
  The operations we perform on this quantity tend to bias it
  out towards 1, as SIGMA grows from 0 to 1.

  VDOT, in turn, is a value between -1 and 1, which, for large
  SIGMA, we want biased towards 1.
*/
    x = uniform_01_sample ( iseed );
    x = exp ( ( 1.0 - sigma ) * log ( x ) );
    vdot = 2.0 * x - 1.0;
    dphi = acos ( vdot );
/*
  Now we know enough to write down a vector that is rotated DPHI
  from the base vector.
*/
    x = cos ( theta ) * sin ( phi + dphi );
    y = sin ( theta ) * sin ( phi + dphi );
    z = cos ( phi + dphi );
/*
  Pick a uniformly random rotation between 0 and 2 Pi around the
  axis of the base vector.
*/
    psi = 2.0 * PI * uniform_01_sample ( iseed );

    vector_rotate_3d ( x, y, z, &x2, &y2, &z2, vbase[0], vbase[1], vbase[2], psi );

    vran[0] = x2;
    vran[1] = y2;
    vran[2] = z2;
  }

  return;
}
/**********************************************************************/

void direction_random_3d ( int *iseed, float vran[3] )

/**********************************************************************/

/*
  Purpose:

    DIRECTION_RANDOM_3D picks a random direction vector in 3D.

  Modified:

    17 April 1999

  Author:

    John Burkardt

  Parameters:

    Input/output, int *ISEED, a seed for the random number generator.

    Output, float VRAN[3], the random direction vector, with unit norm.
*/
{
  float phi;
  float theta;
  float vdot;
/*
  Pick a uniformly random VDOT, which must be between -1 and 1.
  This represents the dot product of the random vector with the Z unit vector.

  Note: this works because the surface area of the sphere between
  Z and Z + dZ is independent of Z.  So choosing Z uniformly chooses
  a patch of area uniformly.
*/
  vdot = 2.0 * uniform_01_sample ( iseed ) - 1.0;
  phi = acos ( vdot );
/*
  Pick a uniformly random rotation between 0 and 2 Pi around the
  axis of the Z vector.
*/
  theta = 2.0 * PI * uniform_01_sample ( iseed );

  vran[0] = cos ( theta ) * sin ( phi );
  vran[1] = sin ( theta ) * sin ( phi );
  vran[2] = cos ( phi );

  return;
}
/**********************************************************************/

void direction_random_nd ( int n, int *iseed, float w[] )

/**********************************************************************/

/*
  Purpose:

    DIRECTION_RANDOM_ND generates a random direction vector in ND.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the dimension of the space.

    Input/output, int *ISEED, a seed for the random number generator.

    Output, float W[N], a random direction vector, with unit norm.
*/
{
  int i;
  float norm;
/*
  Get N values from a standard normal distribution.
*/
  for ( i = 0; i < n; i++ ) {
    w[i] = normal_01_sample ( iseed );
  }
/*
  Compute the length of the vector.
*/
  norm = 0.0;
  for ( i = 0; i < n; i++ ) {
    norm = norm + w[i] * w[i];
  }
  norm = sqrt ( norm );
/*
  Normalize the vector.
*/
  for ( i = 0; i < n; i++ ) {
    w[i] = w[i] / norm;
  }

  return;
}
/**********************************************************************/

float dot_2d ( float x1, float y1, float x2, float y2 )

/**********************************************************************/

/*
  Purpose:

    DOT_2D computes the dot product of a pair of vectors in 2D.

  Modified:

    19 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, the coordinates of the vectors.

    Output, float DOT_2D, the dot product.
*/
{
  float value;

  value = x1 * x2 + y1 * y2;
 
  return value;
}
/**********************************************************************/

float dot_3d ( float x1, float y1, float z1, float x2, float y2, float z2 )

/**********************************************************************/

/*
  Purpose:

    DOT_3D computes the dot product of a pair of vectors in 3D.

  Modified:

    19 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, the coordinates of the vectors.

    Output, float DOT_3D, the dot product.
*/
{
  float value;

  value = 
    x1 * x2 + 
    y1 * y2 + 
    z1 * z2;
 
  return value;
}
/**********************************************************************/

float dot_nd ( int n, float vec1[], float vec2[] )

/**********************************************************************/

/*
  Purpose:

    DOT_ND computes the dot product of a pair of vectors in ND.

  Modified:

   19 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vectors.

    Input, float VEC1[N], VEC2[N], the two vectors to be considered.

    Output, float DOT_ND, the dot product of the vectors.
*/
{
  int i;
  float value;

  value = 0.0;
  for ( i = 0; i < n; i++ ) {
    value = value + vec1[i] * vec2[i];
  }

  return value;
}
/**********************************************************************/

float dot0_2d ( float x0, float y0, float x1, float y1, float x2, 
  float y2 )

/**********************************************************************/

/*
  Purpose:

    DOT0_2D computes the dot product of (P1-P0) and (P2-P0) in 2D.

  Modified:

    17 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X0, Y0, X1, Y1, X2, Y2, the coordinates of 
    the points P0, P1 and P1.

    Output, float DOT0_2D, the dot product of (P1-P0) and (P2-P0).
*/
{
  float value;

  value = 
    ( x1 - x0 ) * ( x2 - x0 ) + 
    ( y1 - y0 ) * ( y2 - y0 );
 
  return value;
}
/**********************************************************************/

float dot0_3d ( float x0, float y0, float z0, float x1, float y1, float z1, 
  float x2, float y2, float z2 )

/**********************************************************************/

/*
  Purpose:

    DOT0_3D computes the dot product of (P1-P0) and (P2-P0) in 3D.

  Modified:

    20 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X0, Y0, Z0, the coordinates of the point P0.

    Input, float X1, Y1, Z1, the coordinates of the point P1.

    Input, float X2, Y2, Z2, the coordinates of the point P2.

    Output, float DOT0_3D, the dot product of (P1-P0) and (P2-P0).
*/
{
  float value;

  value = 
    ( x1 - x0 ) * ( x2 - x0 ) + 
    ( y1 - y0 ) * ( y2 - y0 ) + 
    ( z1 - z0 ) * ( z2 - z0 );
 
  return value;
}
/**********************************************************************/

float enorm_2d ( float x1, float y1 )

/**********************************************************************/

/*
  Purpose:

    ENORM_2D computes the Euclidean norm of a vector in 2D.

  Modified:

    18 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, the coordinates of the vector.

    Output, float ENORM_2D, the Euclidean norm of the vector.
*/
{
  float value;

  value = sqrt ( x1 * x1 + y1 * y1 );
 
  return value;
}
/**********************************************************************/

float enorm_3d ( float x1, float y1, float z1 )

/**********************************************************************/

/*
  Purpose:

    ENORM_3D computes the Euclidean norm of a vector in 3D.

  Modified:

    18 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, the coordinates of the vector.

    Output, float ENORM_3D, the Euclidean norm of the vector.
*/
{
  float value;

  value = sqrt ( 
    x1 * x1 + 
    y1 * y1 + 
    z1 * z1 );
 
  return value;
}
/**********************************************************************/

float enorm_nd ( int n, float x[] )

/**********************************************************************/

/*
  Purpose:

    ENORM_ND computes the Euclidean norm of a vector in N space.

  Modified:

    18 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the dimension of the space.

    Input, float X(N), the coordinates of the vector.

    Output, float ENORM_ND, the Euclidean norm of the vector.
*/
{
  int i;
  float value;

  value = 0.0;

  for ( i = 0; i < n; i++ ) {
    value = value + x[i] * x[i];
  }

  value = sqrt ( value );
 
  return value;
}
/**********************************************************************/

float enorm0_2d ( float x0, float y0, float x1, float y1 )

/**********************************************************************/

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
{
  float value;

  value = sqrt (
    ( x1 - x0 ) * ( x1 - x0 ) + 
    ( y1 - y0 ) * ( y1 - y0 ) );
 
  return value;
}
/**********************************************************************/

float enorm0_3d ( float x0, float y0, float z0, float x1, float y1, float z1 )

/**********************************************************************/

/*
  Purpose:

    ENORM0_3D computes the Euclidean norm of (P1-P0) in 3D.

  Modified:

    18 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X0, Y0, Z0, X1, Y1, Z1, the coordinates of the points 
    P0 and P1.

    Output, float ENORM0_3D, the Euclidean norm of (P1-P0).
*/
{
  float value;

  value = sqrt (
    ( x1 - x0 ) * ( x1 - x0 ) + 
    ( y1 - y0 ) * ( y1 - y0 ) + 
    ( z1 - z0 ) * ( z1 - z0 ) );
 
  return value;
}
/**********************************************************************/

float enorm0_nd ( int n, float x[], float y[] )

/**********************************************************************/

/*
  Purpose:

    ENORM0_ND computes the Euclidean norm of a (X-Y) in N space.

  Modified:

    18 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the dimension of the space.

    Input, float X(N), Y(N), the coordinates of the vectors.

    Output, float ENORM0_ND, the Euclidean norm of the vector.
*/
{
  int i;
  float value;

  value = 0.0;

  for ( i = 0; i < n; i++ ) {
    value = value + ( x[i] - y[i] ) * ( x[i] - y[i] );
  }

  value = sqrt ( value );
 
  return value;
}
/**********************************************************************/

float enormsq0_2d ( float x0, float y0, float x1, float y1 )

/**********************************************************************/

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
{
  float value;

  value = 
    ( x1 - x0 ) * ( x1 - x0 ) + 
    ( y1 - y0 ) * ( y1 - y0 );
 
  return value;
}
/**********************************************************************/

float enormsq0_3d ( float x0, float y0, float z0, float x1, float y1, 
  float z1 )

/**********************************************************************/

/*
  Purpose:

    ENORMSQ0_3D computes the square of the Euclidean norm of (P1-P0) in 3D.

  Modified:

    18 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X0, Y0, Z0, X1, Y1, Z1, the coordinates of the points 
    P0 and P1.

    Output, float ENORMSQ0_3D, the square of the Euclidean norm of (P1-P0).
*/
{
  float value;

  value = 
    ( x1 - x0 ) * ( x1 - x0 ) + 
    ( y1 - y0 ) * ( y1 - y0 ) + 
    ( z1 - z0 ) * ( z1 - z0 );
 
  return value;
}
/***********************************************************************/
 
float enormsq0_nd ( int n, float x0[], float x1[] )

/***********************************************************************/

/*
  Purpose:
 
    ENORMSQ0_ND computes the squared Euclidean norm of (P1-P0) in ND.

  Modified:

    18 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the dimension of the space.

    Input, float X0[N], the coordinates of the base vector.

    Input, float X1[N], the coordinates of the displacement vector.

    Output, float ENORMSQ0_ND, the squared Euclidean norm of the vector
    ( X1 - X0 ).
*/
{
  float value;
  int i;

  value = 0.0;

  for ( i = 0; i < n; i++ ) {
    value = value + ( x1[i] - x0[i] ) * ( x1[i] - x0[i] );
  }

  return value;
}
/**********************************************************************/

int get_seed ( void )

/**********************************************************************/

/*
  Purpose:

    GET_SEED returns a random seed for the random number generator.

  Modified:

    04 May 1999

  Author:

    John Burkardt

  Parameters:

    Output, int GET_SEED, a random seed value.
*/
{
  time_t clock;
  int i;
  int ihour;
  int imin;
  int isec;
  int iseed;
  static int iseed_internal = 0;
  struct tm *lt;
  time_t tloc;
/*
  If the internal seed is 0, generate a value based on the time.
*/
  if ( iseed_internal == 0 ) {

    clock = time ( &tloc );
    lt = localtime ( &clock );
/*
  Hours is 1, 2, ..., 12.
*/
    ihour = lt->tm_hour;

    if ( ihour > 12 ) {
      ihour = ihour - 12;
    }
/*
  Move Hours to 0, 1, ..., 11
*/
    ihour = ihour - 1;

    imin = lt->tm_min;

    isec = lt->tm_sec;

    iseed_internal = isec + 60 * ( imin + 60 * ihour );
/*
  We want values in [1,43200], not [0,43199].
*/
    iseed_internal = iseed_internal + 1;
/*
  Remap ISEED from [1,43200] to [1,IMAX].
*/
    iseed_internal = ( int ) (
      ( float ) iseed_internal * ( float ) I_MAX / ( 60.0 * 60.0 * 12.0 ) 
    );

  }
/*
  Never use a seed of 0.
*/
  if ( iseed_internal == 0 ) {
    iseed_internal = 1;
  }

  if ( iseed_internal == I_MAX ) {
    iseed_internal = I_MAX - 1;
  }
/*
  Call UNIFORM_01_SAMPLE a few times to mix things up.
*/
  for ( i = 0; i < 10; i++ ) {
    uniform_01_sample ( &iseed_internal );
  }

  iseed = iseed_internal;

  return iseed;
}
/**********************************************************************/

int halfspace_imp_triangle_int_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float x3, float y3, float z3, float a, float b, 
  float c, float d, float x[], float y[], float z[] )

/**********************************************************************/

/*
  Purpose:

    HALFSPACE_IMP_TRIANGLE_INT_3D: intersection ( implicit halfspace, triangle ) in 3D.

  Definition:

    The implicit form of a half-space in 3D may be described as the set
    of points (X,Y,Z) on or "above" an implicit plane:

      A * X + B * Y + C * Z + D >= 0

    The triangle is specified by listing its three vertices.

  Discussion:

    The intersection may be described by the number of vertices of the
    triangle that are included in the halfspace, and by the location of
    points between vertices that separate a side of the triangle into
    an included part and an unincluded part.

    0 vertices, 0 separators    (no intersection)
    1 vertex, 0 separators  (point intersection)
    2 vertices, 0 separators    (line intersection)
    3 vertices, 0 separators    (triangle intersection)

    1 vertex, 2 separators,     (intersection is a triangle)
    2 vertices, 2 separators,   (intersection is a quadrilateral).

  Modified:

    03 June 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, the coordinates
    of the vertices of the triangle.

    Input, float A, B, C, D, the parameters that define the implicit plane,
    which in turn define the implicit halfspace.

    Output, float X[4], Y[4], Z[4], the coordinates of the 
    intersection points.  The points will lie in sequence on the triangle.
    Some points will be vertices, and some may be separators.

    Output, int HALFSPACE_IMP_TRIANGLE_INT_3D, the number of intersection 
    points returned, which will always be between 0 and 4.
*/
{
  float dist1;
  float dist2;
  float dist3;
  int num_int;
/*
  Compute the signed distances between the vertices and the plane.
*/
  dist1 = a * x1 + b * y1 + c * z1 + d;
  dist2 = a * x2 + b * y2 + c * z2 + d;
  dist3 = a * x3 + b * y3 + c * z3 + d;
/*
  Now we can find the intersections.
*/
  num_int = halfspace_triangle_int_3d ( x1, y1, z1, x2, y2,
    z2, x3, y3, z3, dist1, dist2, dist3, x, y, z );

  return num_int;
}
/**********************************************************************/

int halfspace_norm_triangle_int_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float x3, float y3, float z3, float xp, float yp, 
  float zp, float xn, float yn, float zn, float x[], float y[], float z[] )

/**********************************************************************/

/*
  Purpose:

    HALFSPACE_NORM_TRIANGLE_INT_3D: intersection ( normal halfspace, triangle ) in 3D.

  Definition:

    The normal form of a halfspace in 3D may be described as the set
    of points (X,Y,Z) on or "above" a plane described in normal form:

      ( Xp, Yp, Zp ) is a point on the plane,
      ( Xn, Yn, Zn ) is the unit normal vector, pointing "out" of the halfspace

    The triangle is specified by listing its three vertices.

  Discussion:

    The intersection may be described by the number of vertices of the
    triangle that are included in the halfspace, and by the location of
    points between vertices that separate a side of the triangle into
    an included part and an unincluded part.

    0 vertices, 0 separators    (no intersection)
    1 vertex, 0 separators  (point intersection)
    2 vertices, 0 separators    (line intersection)
    3 vertices, 0 separators    (triangle intersection)

    1 vertex, 2 separators,     (intersection is a triangle)
    2 vertices, 2 separators,   (intersection is a quadrilateral).

  Modified:

    03 June 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, the coordinates
    of the vertices of the triangle.

    Input, float XP, YP, ZP, a point on the bounding plane that defines
    the halfspace.

    Input, float XN, YN, ZN, the components of the normal vector to the
    bounding plane that defines the halfspace.  By convention, the
    normal vector points "outwards" from the halfspace.

    Output, float X[4], Y[4], Z[4], the coordinates of the 
    intersection points.  The points will lie in sequence on the triangle.
    Some points will be vertices, and some may be separators.

    Output, int HALFSPACE_NORM_TRIANGLE_INT_3D, the number of intersection 
    points returned, which will always be between 0 and 4.
*/
{
  float d;
  float dist1;
  float dist2;
  float dist3;
  int num_int;
/*
  Compute the signed distances between the vertices and the plane.
*/
  d = - xn * xp - yn * yp - zn * zp;

  dist1 = xn * x1 + yn * y1 + zn * z1 + d;
  dist2 = xn * x2 + yn * y2 + zn * z2 + d;
  dist3 = xn * x3 + yn * y3 + zn * z3 + d;
/*
  Now we can find the intersections.
*/
  num_int = halfspace_triangle_int_3d ( x1, y1, z1, x2, y2,
    z2, x3, y3, z3, dist1, dist2, dist3, x, y, z );

  return num_int;
}
/**********************************************************************/

int halfspace_triangle_int_3d ( float x1, float y1, float z1, 
  float x2, float y2, float z2, float x3, float y3, float z3, float dist1, 
  float dist2, float dist3, float x[], float y[], float z[] )

/**********************************************************************/

/*
  Purpose:

    HALFSPACE_TRIANGLE_INT_3D: intersection ( halfspace, triangle ) in 3D.

  Definition:

    The triangle is specified by listing its three vertices.

  Discussion:

    The halfspace is not described in the input data.  Rather, the
    distances from the triangle vertices to the halfspace are given.

    The intersection may be described by the number of vertices of the
    triangle that are included in the halfspace, and by the location of
    points between vertices that separate a side of the triangle into
    an included part and an unincluded part.

    0 vertices, 0 separators    (no intersection)
    1 vertex, 0 separators  (point intersection)
    2 vertices, 0 separators    (line intersection)
    3 vertices, 0 separators    (triangle intersection)

    1 vertex, 2 separators,     (intersection is a triangle)
    2 vertices, 2 separators,   (intersection is a quadrilateral).

  Modified:

    03 June 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, the coordinates
    of the vertices of the triangle.

    Input, float DIST1, DIST2, DIST3, the distances from each of the
    three vertices of the triangle to the halfspace.  The distance is
    zero if a vertex lies within the halfspace, or on the plane that
    defines the boundary of the halfspace.  Otherwise, it is the
    distance from that vertex to the bounding plane.

    Output, float X[4], Y[4], Z[4], the coordinates of the 
    intersection points.  The points will lie in sequence on the triangle.
    Some points will be vertices, and some may be separators.

    Output, int HALFSPACE_TRIANGLE_INT_3D, the number of intersection points 
    returned, which will always be between 0 and 4.
*/
{
  int num_int;
/*
  Walk around the triangle, looking for vertices that are included,
  and points of separation.
*/
  num_int = 0;

  if ( dist1 <= 0.0 ) {

    x[num_int] = x1;
    y[num_int] = y1;
    z[num_int] = z1;
    num_int = num_int + 1;

  }

  if ( dist1 * dist2 < 0.0 ) {

    x[num_int] = ( dist1 * x2 - dist2 * x1 ) / ( dist1 - dist2 );
    y[num_int] = ( dist1 * y2 - dist2 * y1 ) / ( dist1 - dist2 );
    z[num_int] = ( dist1 * z2 - dist2 * z1 ) / ( dist1 - dist2 );
    num_int = num_int + 1;

  }

  if ( dist2 <= 0.0 ) {

    x[num_int] = x2;
    y[num_int] = y2;
    z[num_int] = z2;
    num_int = num_int + 1;

  }

  if ( dist2 * dist3 < 0.0 ) {

    x[num_int] = ( dist2 * x3 - dist3 * x2 ) / ( dist2 - dist3 );
    y[num_int] = ( dist2 * y3 - dist3 * y2 ) / ( dist2 - dist3 );
    z[num_int] = ( dist2 * z3 - dist3 * z2 ) / ( dist2 - dist3 );
    num_int = num_int + 1;

  }

  if ( dist3 <= 0.0 ) {

    x[num_int] = x3;
    y[num_int] = y3;
    z[num_int] = z3;
    num_int = num_int + 1;

  }

  if ( dist3 * dist1 < 0.0 ) {

    x[num_int] = ( dist3 * x1 - dist1 * x3 ) / ( dist3 - dist1 );
    y[num_int] = ( dist3 * y1 - dist1 * y3 ) / ( dist3 - dist1 );
    z[num_int] = ( dist3 * z1 - dist1 * z3 ) / ( dist3 - dist1 );
    num_int = num_int + 1;

  }

  return num_int;
}
/**********************************************************************/

int i_random ( int ilo, int ihi, int *iseed )

/**********************************************************************/

/*
  Purpose:

    I_RANDOM returns a random int in a given range.

  Modified:

    19 May 1999

  Author:

    John Burkardt

  Parameters:

    Output, int I, the randomly chosen integer.

    Input, int ILO, IHI, the minimum and maximum values acceptable
    for I.

    Input/output, int ISEED, a seed for the random number generator.
*/
{
  int i;
  float r;
  float rhi;
  float rlo;
/*
  Pick a random number in (0,1).
*/
  r = uniform_01_sample ( iseed );
/*
  Set an interval [RLO,RHI] which contains the integers [ILO,IHI],
  each with a "neighborhood" of width 1.
*/
  rlo = ( ( float ) ilo ) - 0.5;
  rhi = ( ( float ) ihi ) + 0.5;
/*
  Set I to the integer that is nearest the scaled value of R.
*/
  r = ( ( 1.0 - r ) * rlo + r * rhi );

  if ( r < 0.0 ) {
    r = r - 0.5;
  }
  else {
    r = r + 0.5;
  }

  i = ( int ) r;
/*
  In case of oddball events at the boundary, enforce the limits.
*/
  if ( i < ilo ) {
    i = ilo;
  }

  if ( i > ihi ) {
    i = ihi;
  }

  return i;
}
/**********************************************************************/

float line_exp_point_dist_2d ( float x1, float y1, float x2, float y2, 
  float x, float y )

/**********************************************************************/

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
{
  float bot;
  float dist;
  float dot;
  float t;
  float xn;
  float yn;

  bot = enormsq0_2d ( x1, y1, x2, y2 );

  if ( bot == 0.0 ) {

    xn = x1;
    yn = y1;
  }
/*
  (P-P1) dot (P2-P1) = Norm(P-P1) * Norm(P2-P1) * Cos(Theta).

  (P-P1) dot (P2-P1) / Norm(P-P1)**2 = normalized coordinate T
  of the projection of (P-P1) onto (P2-P1).
*/
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
}
/**********************************************************************/

float line_exp_point_dist_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float x, float y, float z )

/**********************************************************************/

/*
  Purpose:

    LINE_EXP_POINT_DIST_3D: distance ( explicit line, point ) in 3D.

  Formula:

    The explicit form of a line in 2D is:

      (X1,Y1,Z1), (X2,Y2,Z2).

  Modified:

    22 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, (X1,Y1,Z1) and (X2,Y2,Z2) are
    two points on the line.  If the points are identical, then
    the line will be treated as a single point.

    Input, float X, Y, Z, the point whose distance from the line is
    to be measured.

    Output, float LINE_EXP_POINT_DIST_3D, the distance from the point 
    to the line.
*/
{
  float bot;
  float dist;
  float t;
  float xn;
  float yn;
  float zn;

  bot = enormsq0_3d ( x1, y1, z1, x2, y2, z2 );

  if ( bot == 0.0 ) {

    xn = x1;
    yn = y1;
    zn = z1;

  }
/*
  (P-P1) dot (P2-P1) = Norm(P-P1) * Norm(P2-P1) * Cos(Theta).

  (P-P1) dot (P2-P1) / Norm(P-P1)**2 = normalized coordinate T
  of the projection of (P-P1) onto (P2-P1).
*/
  else {

    t = (
      ( x - x1 ) * ( x2 - x1 ) +
      ( y - y1 ) * ( y2 - y1 ) +
      ( z - z1 ) * ( z2 - z1 ) ) / bot;

    xn = x1 + t * ( x2 - x1 );
    yn = y1 + t * ( y2 - y1 );
    zn = z1 + t * ( z2 - z1 );

  }
/*
  Now compute the distance between the projection point and P.
*/
  dist = enorm0_3d ( x, y, z, xn, yn, zn );

  return dist;
}
/**********************************************************************/

float line_exp_point_dist_signed_2d ( float x1, float y1, float x2, 
  float y2, float x, float y )

/**********************************************************************/

/*
  Purpose:

    LINE_EXP_POINT_DIST_SIGNED_2D: signed distance ( explicit line, point ) in 2D.

  Discussion:

    The signed distance has two interesting properties:

    *  The absolute value of the signed distance is the
       usual (Euclidean) distance.

    *  Points with signed distance 0 lie on the line,
       points with a negative signed distance lie on one side
         of the line,
       points with a positive signed distance lie on the
         other side of the line.

    Assuming that C is nonnegative, then if a point is a positive
    distance away from the line, it is on the same side of the
    line as the point (0,0), and if it is a negative distance
    from the line, it is on the opposite side from (0,0).

  Modified:

    21 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, define the two points
    (X1,Y1) and (X2,Y2) that determine the line.

    Input, float X, Y, the point (X,Y) whose signed distance is desired.

    Output, float LINE_EXP_DIST_SIGNED_2D, the signed distance from the 
    point to the line.
*/
{
  float a;
  float b;
  float c;
  float dist_signed;
/*
  Convert the line to A*x+B*y+C form.
*/
  line_exp2imp_2d ( x1, y1, x2, y2, &a, &b, &c );
/*
  Compute the signed distance from the point to the line.
*/
  dist_signed = ( a * x + b * y + c ) / sqrt ( a * a + b * b );
 
  return dist_signed;
}
/**********************************************************************/

void line_exp2imp_2d ( float x1, float y1, float x2, float y2, float *a, 
  float *b, float *c )

/**********************************************************************/

/*
  Purpose:

    LINE_EXP2IMP_2D converts an explicit line to implicit form in 2D.

  Formula:

    The explicit form of a line in 2D is:

      (X1,Y1), (X2,Y2).

    The implicit form of a line in 2D is:

      A * X + B * Y + C = 0

  Modified:

    22 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2.  (X1,Y1) and (X2,Y2) are
    two points on the line. (X1,Y1) must be different
    from (X2,Y2).

    Output, float *A, *B, *C, three coefficients which describe
    the line that passes through (X1,Y1) and (X2,Y2).
*/
{
/*
  Take care of degenerate cases.
*/
  if ( x1 == x2 && y1 == y2 ) {
    printf ( "\n" );
    printf ( "LINE_EXP2IMP_2D - Fatal error!\n" );
    printf ( "  (X1,Y1) = (X2,Y2)\n" );
    printf ( "  (X1,Y1) = %f %f\n", x1, y1 );
    printf ( "  (X2,Y2) = %f %f\n", x2, y2 );
    exit ( EXIT_FAILURE );
  }

  *a = y2 - y1;
  *b = x1 - x2;
  *c = x2 * y1 - x1 * y2;

  return;
}
/**********************************************************************/

void line_exp2par_3d ( float x1, float y1, float z1, float x2, float y2,  
  float z2, float *f, float *g, float *h, float *x0, float *y0, float *z0 )

/**********************************************************************/

/*
  Purpose:

    LINE_EXP2PAR_3D converts a line into direction form in 3D.

  Definition:

    The direction form of a line is:

      (x,y,z)(t) = (x0,y0,z0) + t * ( f, g, h )

  Modified:

    17 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, two points on the line. 

    Output, float *F, *G, *H, the components of the direction vector.

    Output, float *X0, *Y0, *Z0, the base vector.
*/
{
  float norm;
/*
  Take care of degenerate cases.
*/
  if ( x1 == x2 && y1 == y2 && z1 == z2 ) {
    *f = 0.0;
    *g = 0.0;
    *h = 0.0;
    *x0 = x1;
    *y0 = y1;
    *z0 = z1;
  }
  else {
    norm = sqrt ( 
        ( x2 - x1 ) * ( x2 - x1 ) 
      + ( y2 - y1 ) * ( y2 - y1 )
      + ( z2 - z1 ) * ( z2 - z1 ) );
    *f = ( x2 - x1 ) / norm;
    *g = ( y2 - y1 ) / norm;
    *h = ( z2 - z1 ) / norm;
    *x0 = x1;
    *y0 = y1;
    *z0 = z1;
  }

  return;
}
/**********************************************************************/

float line_seg_point_dist_3d ( float x1, float y1, float z1, float x2,   
  float y2, float z2, float x, float y, float z )

/**********************************************************************/

/*
  Purpose:

    LINE_SEG_POINT_DIST_3D computes the distance from a point to a line segment in 3D.

  Modified:

    17 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, the endpoints of the line 
    segment.

    Input, float X, Y, Z, the point whose nearest neighbor on the line 
    segment is to be determined.

    Output, float LINE_SEG_POINT_DIST_3D, the distance from the point to the line segment.
*/
{
  float bot;
  float dist;
  float t;
  float xn;
  float yn;
  float zn;
/*
  If the line segment is actually a point, then the answer is easy.
*/
  if ( x1 == x2 && y1 == y2 && z1 == z2 ) {
    xn = x1;
    yn = y1;
    zn = z1;
  }
  else {
 
    bot = 
        ( x1 - x2 ) * ( x1 - x2 )
      + ( y1 - y2 ) * ( y1 - y2 )
      + ( z1 - z2 ) * ( z1 - z2 );

    t = ( 
        ( x1 - x ) * ( x1 - x2 ) 
      + ( y1 - y ) * ( y1 - y2 ) 
      + ( z1 - z ) * ( z1 - z2 ) ) / bot;
 
    t = MAX ( t, 0.0 );
    t = MIN ( t, 1.0 );

    xn = x1 + t * ( x2 - x1 );
    yn = y1 + t * ( y2 - y1 );
    zn = z1 + t * ( z2 - z1 );

  }
 
  dist = sqrt (
      ( xn - x ) * ( xn - x ) 
    + ( yn - y ) * ( yn - y )
    + ( zn - z ) * ( zn - z ) );

  return dist;
}
/**********************************************************************/

void line_seg_point_near_3d ( float x1, float y1, float z1, 
  float x2, float y2, float z2, float x, float y, float z,
  float *xn, float *yn, float *zn, float *dist, float *t )

/**********************************************************************/

/*
  Purpose:

    LINE_SEG_POINT_NEAR_3D finds the point on a line segment nearest a point in 3D.

  Modified:

    17 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, the two endpoints of the line segment.

    (X1,Y1,Z1) should generally be different from (X2,Y2,Z2), but
    if they are equal, the program will still compute a meaningful
    result.

    Input, float X, Y, Z, the point whose nearest neighbor
    on the line segment is to be determined.

    Output, float *XN, *YN, *ZN, the point on the line segment which is
    nearest the point (X,Y,Z).
 
    Output, float *DIST, the distance from the point to the nearest point
    on the line segment.

    Output, float *T, the relative position of the nearest point
    (XN,YN,ZN) to the defining points (X1,Y1,Z1) and (X2,Y2,Z2).

      (XN,YN,ZN) = (1-T)*(X1,Y1,Z1) + T*(X2,Y2,Z2).

    T will always be between 0 and 1.

*/
{
  float bot;

  if ( x1 == x2 && y1 == y2 && z1 == z2 ) {
    *t = 0.0;
    *xn = x1;
    *yn = y1;
    *zn = z1;
  }
  else {

    bot = 
        ( x1 - x2 ) * ( x1 - x2 )
      + ( y1 - y2 ) * ( y1 - y2 )
      + ( z1 - z2 ) * ( z1 - z2 );

    *t = (
      + ( x1 - x ) * ( x1 - x2 )
      + ( y1 - y ) * ( y1 - y2 )
      + ( z1 - z ) * ( z1 - z2 ) ) / bot;

    if ( *t < 0.0 ) {
      *t = 0.0;
      *xn = x1;
      *yn = y1;
      *zn = z1;
    }
    else if ( *t > 1.0 ) {
      *t = 1.0;
      *xn = x2;
      *yn = y2;
      *zn = z2;
    }
    else {
      *xn = x1 + *t * ( x2 - x1 );
      *yn = y1 + *t * ( y2 - y1 );
      *zn = z1 + *t * ( z2 - z1 );
    }
  }
  *dist = sqrt ( 
      ( *xn - x ) * ( *xn - x ) 
    + ( *yn - y ) * ( *yn - y ) 
    + ( *zn - z ) * ( *zn - z ) );

  return;
}
/**********************************************************************/

float lines_imp_angle_2d ( float a1, float b1, float c1, 
  float a2, float b2, float c2 )

/**********************************************************************/

/*
  Purpose:

    LINES_IMP_ANGLE_2D finds the angle between two implicit lines in 2D.

  Formula:

    The implicit form of a line in 2D is:

      A * X + B * Y + C = 0

  Reference:

    Adrian Bowyer and John Woodwark,
    A Programmer's Geometry,
    Butterworths, 1983.

  Modified:

    27 June 1999

  Author:

    John Burkardt

  Parameters:

    Input, float A1, B1, C1, the implicit parameters of the first line.

    Input, float A2, B2, C2, the implicit parameters of the second line.

    Output, float LINES_IMP_ANGLE_2D, the angle between the two lines.
*/
{
  float ctheta;
  float pdotq;
  float pnorm;
  float qnorm;
  float theta;

  pdotq = a1 * a2 + b1 * b2;
  pnorm = sqrt ( a1 * a1 + b1 * b1 );
  qnorm = sqrt ( a2 * a2 + b2 * b2 );

  ctheta = pdotq / ( pnorm * qnorm );

  theta = acos ( ctheta );

  return theta;
}
/**********************************************************************/

void lines_imp_int_2d ( float a1, float b1, float c1, float a2, float b2, 
  float c2, int *ival, float *x, float *y )

/**********************************************************************/

/*
  Purpose:

    LINES_IMP_INT_2D determines where two implicit lines intersect in 2D.

  Formula:

    The implicit form of a line in 2D is:

      A * X + B * Y + C = 0

  Modified:

    27 June 1999

  Author:

    John Burkardt

  Parameters:

    Input, float A1, B1, C1, define the first line.
    At least one of A1 and B1 must be nonzero.

    Input, float A2, B2, C2, define the second line.
    At least one of A2 and B2 must be nonzero.

    Output, int *IVAL, reports on the intersection.

    -1, both A1 and B1 were zero.
    -2, both A2 and B2 were zero.
     0, no intersection, the lines are parallel.
     1, one intersection point, returned in X, Y.
     2, infinitely many intersections, the lines are identical.

    Output, float *X, *Y, if IVAL = 1, then X, Y contains
    the intersection point.  Otherwise, X = 0, Y = 0.
*/
{
  float a[2][2];
  float b[2][2];
  float det;

  *x = 0.0;
  *y = 0.0;
/*
  Refuse to handle degenerate lines.
*/
  if ( a1 == 0.0 && b1 == 0.0 ) {
    *ival = - 1;
    return;
  }
  else if ( a2 == 0.0 && b2 == 0.0 ) {
    *ival = - 2;
    return;
  }
/*
  Set up a linear system, and compute its inverse.
*/
  a[0][0] = a1;
  a[0][1] = b1;
  a[1][0] = a2;
  a[1][1] = b2;

  det = rmat2_inverse ( a, b );
/*
  If the inverse exists, then the lines intersect.
  Multiply the inverse times -C to get the intersection point.
*/
  if ( det != 0.0 ) {

    *ival = 1;
    *x = - b[0][0] * c1 - b[0][1] * c2;
    *y = - b[1][0] * c1 - b[1][1] * c2;
  }
/*
  If the inverse does not exist, then the lines are parallel
  or coincident.  Check for parallelism by seeing if the
  C entries are in the same ratio as the A or B entries.
*/
  else {

    *ival = 0;

    if ( a1 == 0.0 ) {
      if ( b2 * c1 == c2 * b1 ) {
        *ival = 2;
      }
    }
    else {
      if ( a2 * c1 == c2 * a1 ) {
        *ival = 2;
      }
    }

  }

  return;
}
/**********************************************************************/

float lines_seg_dist_3d ( float x1, float y1, float z1, float x2, float y2,  
  float z2, float x3, float y3, float z3, float x4, float y4, float z4 )

/**********************************************************************/

/*
  Purpose:

    LINES_SEG_DIST_3D computes the distance between two line segments in 3D.

  Modified:

    03 November 1998

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, the endpoints of the first segment.

    Input, float X3, Y3, Z3, X4, Y4, Z4, the endpoints of the second segment.
 
    Output, float LINES_SEG_DIST_3D, the distance between the line segments.
*/
{
  float d1;
  float d2;
  float dist;
  float dl;
  float dm;
  float dr;
  float t1;
  float t2;
  float tl;
  float tm;
  float tmin;
  float tr;
  float xn1;
  float xn2;
  float yn1;
  float yn2;
  float zn1;
  float zn2;
/*
  Find the nearest points on line 2 to the endpoints of line 1.
*/
  line_seg_point_near_3d ( x3, y3, z3, x4, y4, z4, x1, y1, z1,
    &xn1, &yn1, &zn1, &d1, &t1 );

  line_seg_point_near_3d ( x3, y3, z3, x4, y4, z4, x2, y2, z2,
    &xn2, &yn2, &zn2, &d2, &t2 );

  if ( t1 == t2 ) {
    dist = line_seg_point_dist_3d ( x1, y1, z1, x2, y2, z2, xn1, yn1, zn1 );
    return dist;
  }
/*
  On line 2, over the interval between the points nearest to line 1, 
  the square of the distance of any point to line 1 is a quadratic function.  
  Evaluate it at three points, and seek its local minimum.
*/
  dl = line_seg_point_dist_3d ( x1, y1, z1, x2, y2, z2, xn1, yn1, zn1 );
  dm = line_seg_point_dist_3d ( x1, y1, z1, x2, y2, z2, 0.5*(xn1+xn2), 0.5*(yn1+yn2), 0.5*(zn1+zn2) );
  dr = line_seg_point_dist_3d ( x1, y1, z1, x2, y2, z2, xn2, yn2, zn2 );

  tl = 0.0;
  tm = 0.5;
  tr = 1.0;

  dl = dl * dl;
  dm = dm * dm;
  dr = dr * dr;

  minquad ( tl, dl, tm, dm, tr, dr, &tmin, &dist );

  dist = sqrt ( dist );

  return dist;
}
/**********************************************************************/

int minquad ( float x1, float y1, float x2, float y2, float x3, float y3, 
  float *xmin, float *ymin )

/**********************************************************************/

/*
  Purpose:

    MINQUAD finds a local minimum of F(X) = A * X**2 + B * X + C.

  Note:

    MINQUAD is primarily intended as a utility routine for use by
    DISLSLS3.  The square of the distance function between a point
    and a line segment has the form of F(X).  Hence, we can seek
    the line on the second segment which minimizes the square of
    the distance to the other line segment.

  Modified:

    02 November 1998

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, X3, Y3, are three sets of data
    of the form ( X, F(X) ).  The three X values must be distinct.

    Output, float *XMIN, *YMIN.  XMIN is a point within the interval
    spanned by X1, X2 and X3, at which F takes its local minimum
    value YMIN.

    Output, int MINQUAD, 
    SUCCESS if no error, 
    ERROR if error because X values are not distinct.
*/
{
  int ierror;
  float x;
  float xleft;
  float xrite;
  float y;

  *xmin = 0.0;
  *ymin = 0.0;
/*
  Refuse to deal with coincident data.
*/
  if ( x1 == x2 || x2 == x3 || x3 == x1 ) {
    return ERROR;
  }
/*
  Find the interval endpoints.
*/
  xleft = x1;
  if ( x2 < xleft ) {
    xleft = x2;
  }
  if ( x3 < xleft ) {
    xleft = x3;
  }
  xrite = x1;
  if ( x2 > xrite ) {
    xrite = x2;
  }
  if ( x3 > xrite ) {
    xrite = x3;
  }
/*
  Find the minimizer and its function value over the three input points.
*/
  if ( y1 <= y2 && y1 <= y3 ) {
    *xmin = x1;
    *ymin = y1;
  }
  else if ( y2 <= y1 && y2 <= y3 ) {
    *xmin = x2;
    *ymin = y2;
  }
  else if ( y3 <= y1 && y3 <= y2 ) {
    *xmin = x3;
    *ymin = y3;
  }
/*
  Find the minimizer and its function value over the real line.
*/
  ierror = parabola_ex ( x1, y1, x2, y2, x3, y3, &x, &y );

  if ( ierror != 2 && y < *ymin && xleft < x && x < xrite ) {
    *xmin = x;
    *ymin = y;
  }

  return SUCCESS;
}
/**********************************************************************/

float normal_01_sample ( int *iseed )

/**********************************************************************/

/*
  NORMAL_01_SAMPLE returns random values from the standard 0,1 normal distribution.

  Definition:

    The standard 0,1 normal distribution has mean 0 and standard deviation 1.

  Method:

    The Box-Muller method is used.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input/output, int *ISEED, a seed for the random number generator.

    Output, float NORMAL_01_SAMPLE, a normally distributed random value.
*/
{
  static int need_new = 1;
  static float x1 = 0.0;
  static float x2 = 0.0;
  float y;

  if ( need_new == 1 ) {

    x1 = uniform_01_sample ( iseed );
    if ( x1 < 0.0000001 ) {
      x1 = 0.0000001;
    }

    x2 = uniform_01_sample ( iseed );

    y = sqrt ( - 2.0 * log ( x1 ) ) * cos ( 2.0 * PI * x2 );

    need_new = 0;

  }
  else {

    y = sqrt ( - 2.0 * log ( x1 ) ) * sin ( 2.0 * PI * x2 );

    need_new = 1;

  }

  return y;
}
/**********************************************************************/

int para_contains_point_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float x3, float y3, float z3, float x, float y, 
  float z )

/**********************************************************************/

/*
  Purpose:

    PARA_CONTAINS_POINT_3D determines if a point is inside a parallelogram in 3D.

  Diagram:

     (X2,Y2,Z2).............
        /                 .
       /                 .
      /                 .
    (X1,Y1,Z1)--------->(X3,Y3,Z3)

  Modified:

    20 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, the coordinates of
    the corners of the parallelogram.

    Input, float X, Y, Z, the point to be checked.

    Output, int PARA_CONTAINS_POINT_3D, is 1 if (X,Y,Z) is inside the
    parallelogram, or on its boundary, and 0 otherwise.
    A slight amount of leeway is allowed for error, since a three
    dimensional point may lie exactly in the plane of the parallelogram,
    and yet be computationally slightly outside it.
*/
{
#define TOL 0.00001

  float dot;
  float dotb;
  float dott;
  float v;
  float xn12;
  float xn23;
  float xn31;
  float yn12;
  float yn23;
  float yn31;
  float zn12;
  float zn23;
  float zn31;
/*
  Compute V3, the vector normal to V1 = P2-P1 and V2 = P3-P1.
*/
  cross0_3d ( x1, y1, z1, x2, y2, z2, x3, y3, z3, &xn12, &yn12, &zn12 );
/*
  If the component of V = P-P1 in the V3 direction is too large,
  then it does not lie in the parallelogram.
*/
  dot = ( x - x1 ) * xn12 + ( y - y1 ) * yn12 +  ( z - z1 ) * zn12;

  v = enorm0_3d ( x, y, z, x2, y2, z2 );

  if ( fabs ( dot ) > TOL * ( 1.0 + v ) ) {
    return 0;
  }
/*
  Compute V23, the vector normal to V2 and V3.
*/
  cross_3d ( x3-x1, y3-y1, z3-z1, xn12, yn12, zn12, &xn23, &yn23, &zn23 );
/*
  Compute ALPHA = ( V dot V23 ) / ( V1 dot V23 )
*/
  dott = ( x - x1 ) * xn23 + ( y - y1 ) * yn23 + ( z - z1 ) * zn23;

  dotb =
    ( x2 - x1 ) * xn23 +
    ( y2 - y1 ) * yn23 +
    ( z2 - z1 ) * zn23;

  if ( dotb < 0.0 ) {
    dott = - dott;
    dotb = - dotb;
  }

  if ( dott < 0.0 || dott > dotb ) {
    return 0;
  }
/*
  Compute V31, the vector normal to V3 and V1.
*/
  cross_3d ( xn12, yn12, zn12, x2-x1, y2-y1, z2-z1, &xn31, &yn31, &zn31 );
/*
  Compute BETA = ( V dot V31 ) / ( V2 dot V31 )
*/
  dott = ( x - x1 ) * xn31 + ( y - y1 ) * yn31 + ( z - z1 ) * zn31;

  dotb =
    ( x3 - x1 ) * xn31 +
    ( y3 - y1 ) * yn31 +
    ( z3 - z1 ) * zn31;

  if ( dotb < 0.0 ) {
    dott = - dott;
    dotb = - dotb;
  }

  if ( dott < 0.0 || dott > dotb ) {
    return 0;
  }
/*
  V = ALPHA * V1 + BETA * V2, where both ALPHA and BETA are between
  0 and 1.
*/
  return 1;
#undef TOL
}
/**********************************************************************/

float para_point_dist_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float x3, float y3, float z3, float x, float y, 
  float z )

/**********************************************************************/

/*
  Purpose:

    PARA_POINT_DIST_3D: distance ( parallelogram, point ) in 3D.

  Diagram:

     (X2,Y2,Z2).............
        /                 .
       /                 .
      /                 .
    (X1,Y1,Z1)--------->(X3,Y3,Z3)

  Modified:

    20 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, determine the
    parallelogram, generated by the vectors from (X1,Y1) to (X2,Y2)
    and from (X1,Y1) to (X3,Y3).

    Input, float X, Y, Z, the point which is to be checked.

    Output, float PARA_POINT_DIST_3D, the distance from the point to the
    parallelogram.  DIST is zero if the point lies exactly on the
    parallelogram.
*/
{
  float dis13;
  float dis21;
  float dis34;
  float dis42;
  float dist;
  int inside;
  float t;
  float temp;
  float x4;
  float xn;
  float xp;
  float y4;
  float yn;
  float yp;
  float z4;
  float zn;
  float zp;
/*
  Compute P, the unit normal to X2-X1 and X3-X1:
*/
  cross0_3d ( x1, y1, z1, x2, y2, z2, x3, y3, z3, &xp, &yp, &zp );

  temp = sqrt ( xp * xp + yp * yp + zp * zp );

  if ( temp == 0.0 ) {
    printf ( "\n" );
    printf ( "PARA_POINT_DIST_3D - Fatal error!\n" );
    printf ( "  The normal vector is zero.\n" );
    exit ( EXIT_FAILURE );
  }

  xp = xp / temp;
  yp = yp / temp;
  zp = zp / temp;
/*
  Find ( XN, YN, ZN), the nearest point to ( X, Y, Z ) in the plane.
*/
  t = xp * ( x - x1 ) + yp * ( y - y1 ) + zp * ( z - z1 );

  xn = x - xp * t;
  yn = y - yp * t;
  zn = z - zp * t;
/*
  if ( XN, YN, ZN ) lies WITHIN the parallelogram, we're done.
*/
  inside = para_contains_point_3d ( x1, y1, z1, x2, y2, z2,
    x3, y3, z3, x, y, z );

  if ( inside == 1 ) {
    dist = enorm0_3d ( x, y, z, xn, yn, zn );
    return dist;
  }
/*
  Otherwise, find the distance between ( X, Y, Z ) and each of the
  four line segments that make up the boundary of the parallelogram.
*/
  x4 = x2 + x3 - x1;
  y4 = y2 + y3 - y1;
  z4 = z2 + z3 - z1;

  dis13 = line_seg_point_dist_3d ( x1, y1, z1, x3, y3, z3, x, y, z );

  dist = dis13;

  dis34 = line_seg_point_dist_3d ( x3, y3, z3, x4, y4, z4, x, y, z );

  if ( dis34 < dist ) {
    dist = dis34;
  }

  dis42 = line_seg_point_dist_3d ( x4, y4, z4, x2, y2, z2, x, y, z );

  if ( dis42 < dist ) {
    dist = dis42;
  }

  dis21 = line_seg_point_dist_3d ( x2, y2, z2, x1, y1, z1, x, y, z );

  if ( dis21 < dist ) {
    dist = dis21;
  }

  return dist;
}

/**********************************************************************/

int parabola_ex ( float x1, float y1, float x2, float y2, float x3, float y3, 
  float *x, float *y )

/**********************************************************************/

/*
  Purpose:

    PARABOLA_EX finds the extremal point of a parabola determined by three points.

  Modified:

    17 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, X3, Y3, the coordinates of three points
    on the parabola.  X1, X2 and X3 must be distinct.

    Output, float *X, *Y, the X coordinate of the extremal point of the
    parabola, and the value of the parabola at that point.

    Output, int PARABOLA_EX, error flag.
    0, no error.
    1, two of the X values are equal.
    2, the data lies on a straight line; there is no finite extremal
    point.
    3, the data lies on a horizontal line; every point is "extremal".
*/
{
  float bot;

  *x = 0.0;
  *y = 0.0;

  if ( x1 == x2 || x2 == x3 || x3 == x1 ) {
    return 1;
  }

  if ( y1 == y2 && y2 == y3 && y3 == y1 ) {
    *x = x1;
    *y = y1;
    return 3;
  }

  bot = ( x2 - x3 ) * y1 - ( x1 - x3 ) * y2 + ( x1 - x2 ) * y3;

  if ( bot == 0.0 ) {
    return 2;
  }

  *x = 0.5 * ( 
      x1 * x1 * ( y3 - y2 )
    + x2 * x2 * ( y1 - y3 )
    + x3 * x3 * ( y2 - y1 ) ) / bot;

  *y =  (
      ( *x - x2 ) * ( *x - x3 ) * ( x2 - x3 ) * y1
    - ( *x - x1 ) * ( *x - x3 ) * ( x1 - x3 ) * y2
    + ( *x - x1 ) * ( *x - x2 ) * ( x1 - x2 ) * y3 ) /
    ( ( x1 - x2 ) * ( x2 - x3 ) * ( x1 - x3 ) );

  return 0;
}
/**********************************************************************/

int parabola_ex2 ( float x1, float y1, float x2, float y2, float x3, float y3, 
  float *x, float *y, float *a, float *b, float *c )

/**********************************************************************/

/*
  Purpose:

    PARABOLA_EX2 finds the extremal point of a parabola determined by three points.

  Modified:

    17 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, X3, Y3, the coordinates of three points
    on the parabola.  X1, X2 and X3 must be distinct.

    Output, float *X, *Y, the X coordinate of the extremal point of the
    parabola, and the value of the parabola at that point.

    Output, float *A, *B, *C, the coefficients that define the parabola:
    P(X) = A * X**2 + B * X + C.

    Output, int PARABOLA_EX2, error flag.
    0, no error.
    1, two of the X values are equal.
    2, the data lies on a straight line; there is no finite extremal
    point.
    3, the data lies on a horizontal line; any point is an "extremal point".
*/
{
  float v[3][3];
  float w[3][3];

  *a = 0.0;
  *b = 0.0;
  *c = 0.0;
  *x = 0.0;
  *y = 0.0;

  if ( x1 == x2 || x2 == x3 || x3 == x1 ) {
    return 1;
  }

  if ( y1 == y2 && y2 == y3 && y3 == y1 ) {
    *x = x1;
    *y = y1;
    return 3;
  }
/*
  Set up the Vandermonde matrix.
*/
  v[0][0] = 1.0;
  v[0][1] = x1;
  v[0][2] = x1 * x1;

  v[1][0] = 1.0;
  v[1][1] = x2;
  v[1][2] = x2 * x2;

  v[2][0] = 1.0;
  v[2][1] = x3;
  v[2][2] = x3 * x3;
/*
  Get the inverse.
*/
  rmat3_inverse ( v, w );
/*
  Compute the parabolic coefficients.
*/
  *c = w[0][0] * y1 + w[0][1] * y2 + w[0][2] * y3;
  *b = w[1][0] * y1 + w[1][1] * y2 + w[1][2] * y3;
  *a = w[2][0] * y1 + w[2][1] * y2 + w[2][2] * y3;
/*
  Determine the extremal point.
*/
  if ( *a == 0.0 ) {
    return 2;
  }

  *x = - *b / ( 2.0 * *a );
  *y = *a * *x * *x + *b * *x + *c;

  return 0;
}
/**********************************************************************/

void plane_exp2imp_3d ( float x1, float y1, float z1, float x2, float y2, 
  float z2, float x3, float y3, float z3, float *a, float *b, float *c, 
  float *d )

/**********************************************************************/

/*
  Purpose:

    PLANE_EXP2IMP_3D converts an explicit plane to implicit form in 3D.

  Definition:

    The explicit form of a plane in 3D is

      (X1,Y1,Z1), (X2,Y2,Z2), (X3,Y3,Z3).

    The implicit form of a plane in 3D is

      A * X + B * Y + C * Z + D = 0

  Reference:

    Adrian Bowyer and John Woodwark,
    A Programmer's Geometry,
    Butterworths, 1983.

  Modified:

    22 June 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, X2, X3, Y3, Z3, are three points
    on the plane, which must be distinct, and not collinear.

    Output, float *A, *B, *C, *D, coefficients which describe the plane.
*/
{
  *a = ( y2 - y1 ) * ( z3 - z1 ) - ( z2 - z1 ) * ( y3 - y1 );
  *b = ( z2 - z1 ) * ( x3 - x1 ) - ( x2 - x1 ) * ( z3 - z1 );
  *c = ( x2 - x1 ) * ( y3 - y1 ) - ( y2 - y1 ) * ( x3 - x1 );
  *d = - x2 * *a - y2 * *b - z2 * *c;

  return;
}
/**********************************************************************/

void plane_exp2norm_3d ( float x1, float y1, float z1, float x2, float y2, 
  float z2, float x3, float y3, float z3,
  float *xp, float *yp, float *zp, float *xn, float *yn, float *zn )

/**********************************************************************/

/*
  Purpose:

    PLANE_EXP2NORM_3D converts an explicit plane to normal form in 3D.

  Definition:

    The explicit form of a plane in 3D is

      (X1,Y1,Z1), (X2,Y2,Z2), (X3,Y3,Z3).

    The normal form of a plane in 3D is

      (Xp,Yp,Zp), a point on the plane, and
      (Xn,Yn,Zn), the unit normal to the plane.

  Modified:

    22 June 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, X2, X3, Y3, Z3, are three points
    on the plane, which must be distinct, and not collinear.

    Output, float *XP, *YP, *ZP, a point on the plane.

    Output, float *XN, *YN, *ZN, the unit normal vector to the plane.
*/
{
  float norm;

  *xp = x1;
  *yp = y1;
  *zp = z1;

  *xn = ( y2 - y1 ) * ( z3 - z1 ) - ( z2 - z1 ) * ( y3 - y1 );
  *yn = ( z2 - z1 ) * ( x3 - x1 ) - ( x2 - x1 ) * ( z3 - z1 );
  *zn = ( x2 - x1 ) * ( y3 - y1 ) - ( y2 - y1 ) * ( x3 - x1 );

  norm = sqrt ( *xn * *xn + *yn * *yn + *zn * *zn );

  if ( norm == 0.0 ) {
    printf ( "\n" );
    printf ( "PLANE_EXP2NORM_3D - Fatal error!\n" );
    printf ( "  The normal vector is null.\n" );
    printf ( "  Two points coincide, or nearly so.\n" );
    exit ( EXIT_FAILURE );
  }

  *xn = *xn / norm;
  *yn = *yn / norm;
  *zn = *zn / norm;

  return;
}
/**********************************************************************/

void plane_exp_normal_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float x3, float y3, float z3, float *xn, 
  float *yn, float *zn )

/**********************************************************************/

/*
  Purpose:

    PLANE_EXP_NORMAL_3D finds the normal to an explicit plane in 3D.

  Definition:

    The explicit form of a plane in 3D is

      (X1,Y1,Z1), (X2,Y2,Z2), (X3,Y3,Z3).

  Modified:

    27 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, the coordinates of
    three points that constitute a line.  These points should not
    be identical, nor colinear.

    Output, float *XN, *YN, *ZN, the coordinates of the unit normal 
    vector to the plane containing the three points.
*/
{
  float temp;
/*
  The cross product (P2-P1) x (P3-P1) is a vector normal to
  (P2-P1) and (P3-P1).
*/
  cross0_3d ( x1, y1, z1, x2, y2, z2, x3, y3, z3, xn, yn, zn );

  temp = sqrt ( *xn * *xn + *yn * *yn + *zn * *zn );

  if ( temp == 0.0 ) {
    printf ( "\n" );
    printf ( "PLANE_EXP_NORMAL_3D - Fatal error!\n" );
    printf ( "  The plane is poorly defined.\n" );
    exit ( EXIT_FAILURE );
  }
  else {
    *xn = *xn / temp;
    *yn = *yn / temp;
    *zn = *zn / temp;
  }

  return;
}
/**********************************************************************/

void plane_imp2norm_3d ( float a, float b, float c, float d, 
  float *xp, float *yp, float *zp, float *xn, float *yn, float *zn )

/**********************************************************************/

/*
  Purpose:

    PLANE_IMP2NORM_3D converts an implicit plane to normal form in 3D.

  Definition:

    The implicit form of a plane in 3D is

      A * X + B * Y + C * Z + D = 0.

    The normal form of a plane in 3D is

      (Xp,Yp,Zp), a point on the plane, and
      (Xn,Yn,Zn), the unit normal to the plane.

  Modified:

    22 June 1999

  Author:

    John Burkardt

  Parameters:

    Input, float A, B, C, D, parameters that define the implicit plane.

    Output, float *XP, *YP, *ZP, a point on the plane.

    Output, float *XN, *YN, *ZN, the unit normal vector to the plane.
*/
{
  float norm;

  norm = sqrt ( a * a + b * b + c * c );

  if ( norm == 0.0 ) {
    printf ( "\n" );
    printf ( "PLANE_IMP2NORM_3D - Fatal error!\n" );
    printf ( "  The normal vector is null.\n" );
    printf ( "  Two points coincide, or nearly so.\n" );
    exit ( EXIT_FAILURE );
  }

  *xn = a / norm;
  *yn = b / norm;
  *zn = c / norm;

  if ( a != 0.0 ) {
    *xp = - d / a;
    *yp = 0.0;
    *zp = 0.0;
  }
  else if ( b != 0.0 ) {
    *xp = 0.0;
    *yp = - d / b;
    *zp = 0.0;
  }
  else if ( c != 0.0 ) {
    *xp = 0.0;
    *yp = 0.0;
    *zp = - d / c;
  }
  else {
    printf ( "\n" );
    printf ( "PLANE_IMP2NORM_3D - Fatal error!\n" );
    printf ( "  The (A,B,C) vector is null.\n" );
    exit ( EXIT_FAILURE );
  }

  return;
}
/*********************************************************************/
 
void plane_imp_line_seg_near_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float a, float b, float c, float d, float *dist, 
  float *xp, float *yp, float *zp, float *xls, float *yls, float *zls )

/***********************************************************************/

/*
  Purpose:
 
    PLANE_IMP_LINE_SEG_NEAR_3D: nearest ( implicit plane, line segment ) in 3D

  Modified:

    18 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, the endpoints of the line
    segment.

    Input, float A, B, C, D, the parameters that define the implicit
    plane.

    Output, float *DIST, the distance between the line segment and
    the plane.

    Output, float *XP, *YP, *ZP, the nearest point on the plane.

    Output, float *XLS, *YLS, *ZLS, the nearest point on the line segment
    to the plane.  If DIST is zero, (XLS,YLS,ZLS) is a point of
    intersection of the plane and the line segment.
*/
{
  float alpha;
  float an;
  float bn;
  float cn;
  float dn;
  float idiocy;
  float norm;
  float p1;
  float p2;

  *xls = 0.0;
  *yls = 0.0;
  *zls = 0.0;
  *xp = 0.0;
  *yp = 0.0;
  *zp = 0.0;

  norm = sqrt ( a*a + b*b + c*c );

  if ( norm == 0.0 ) {
    printf ( "\n" );
    printf ( "PLANE_IMP_LINE_SEG_NEAR_3D - Fatal error!\n" );
    printf ( "  Plane normal vector is null.\n" );
    exit ( EXIT_FAILURE );
  }

/*
  The normalized coefficients allow us to compute the (signed) distance.
*/
  an = a / norm;
  bn = b / norm;
  cn = c / norm;
  dn = d / norm;
/*
  If the line segment is actually a point, then the answer is easy.
*/
  if ( x1 == x2 && y1 == y2 && z1 == z2 ) {

    p1 = an * x1 + bn * y1 + cn * z1 + dn;
    *dist = fabs ( p1 );
    *xls = x1;
    *yls = y1;
    *zls = z1;
    *xp = x1 - an * p1;
    *yp = y1 - bn * p1;
    *zp = z1 - cn * p1;
    return;

  }
/*
  Compute the projections of the two points onto the normal vector.
*/
  p1 = an * x1 + bn * y1 + cn * z1 + dn;
  p2 = an * x2 + bn * y2 + cn * z2 + dn;
/*
  If these have the same sign, then the line segment does not
  cross the plane, and one endpoint is the nearest point.
*/
  idiocy = p1 * p2;
  if ( idiocy > 0.0 ) {
 
    p1 = fabs ( p1 );
    p2 = fabs ( p2 );
 
    if ( p1 < p2 ) {
      *xls = x1;
      *yls = y1;
      *zls = z1;
      *xp = x1 - an * p1;
      *yp = y1 - bn * p1;
      *zp = z1 - cn * p1;
      *dist = p1;
    }
    else {
      *xls = x2;
      *yls = y2;
      *zls = z2;
      *dist = p2;
      *xp = x2 - an * p2;
      *yp = y2 - bn * p2;
      *zp = z2 - cn * p2;
    }
/*
  If the projections differ in sign, the line segment crosses the plane.
*/
  }
  else {

    if ( p1 == 0.0 ) {
      alpha = 0.0;
    }
    else if ( p2 == 0.0 ) {
      alpha = 1.0;
    }
    else {
      alpha = p2 / ( p2 - p1 );
    }

    *xls = alpha * x1 + ( 1.0 - alpha ) * x2;
    *yls = alpha * y1 + ( 1.0 - alpha ) * y2;
    *zls = alpha * z1 + ( 1.0 - alpha ) * z2;
    *xp = (*xls);
    *yp = (*yls);
    *zp = (*zls);

    *dist = 0.0;

  }

  return;
}
/**********************************************************************/

float plane_imp_point_dist_3d ( float a, float b, float c, float d, 
  float x, float y, float z )

/**********************************************************************/

/*
  Purpose:

    PLANE_IMP_POINT_DIST_3D: distance ( point, implicit plane ) in 3D.

  Definition:

    The implicit form of a plane in 3D is:

      A * X + B * Y + C * Z + D = 0

  Reference:

    Adrian Bowyer and John Woodwark,
    A Programmer's Geometry,
    Butterworths, 1983.

  Modified:

    23 June 1999

  Author:

    John Burkardt

  Parameters:

    Input, float A, B, C, D, coefficients that define the plane as
    the set of points for which A*X+B*Y+C*Z+D = 0.

    Input, float X, Y, Z, the coordinates of the point.

    Output, float PLANE_IMP_POINT_DIST_3D, the distance from the point to 
    the plane.
*/
{
  float dist;

  dist = 
    fabs ( a * x + b * y + c * z + d ) /
    sqrt ( a * a + b * b + c * c );

  return dist;
}
/**********************************************************************/

float plane_imp_point_dist_signed_3d ( float a, float b, float c, float d, 
  float x, float y, float z )

/**********************************************************************/

/*
  Purpose:

    PLANE_IMP_POINT_DIST_SIGNED_3D: signed distance ( implicit plane, point) in 3

  Definition:

    The implicit form of a plane in 3D is:

      A * X + B * Y + C * Z + D = 0

  Modified:

    22 June 1999

  Author:

    John Burkardt

  Parameters:

    Input, float A, B, C, D, determine the equation of the
    plane, which is:

      A*X + B*Y + C*Z + D = 0.

    Input, float X, Y, Z, the coordinates of the point.

    Output, float PLANE_IMP_POINT_DIST_SIGNED_3D, the signed distance from 
    the point to the plane.
*/
{
  float dist;

  dist = - ( a * x + b * y + c * z + d ) / sqrt ( a * a + b * b + c * c );

  if ( d < 0.0 ) {
    dist = - dist;
  }

  return dist;
}
/**********************************************************************/

void plane_imp_triangle_int_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float x3, float y3, float z3, float a, float b, 
  float c, float d, int *num_int, float x[], float y[], float z[] )

/**********************************************************************/

/*
  Purpose:

    PLANE_IMP_TRIANGLE_INT_3D: intersection ( implicit plane, triangle ) in 3D.

  Definition:

    An implicit plane in 3D is the set of points (X,Y,Z) satisfying
      A * X + B * Y + C * Z + D = 0,
    for a given set of parameters A, B, C, D.  At least one of 
    A, B and C must be nonzero.

  Discussion:

    There may be 0, 1, 2 or 3 points of intersection return;ed.

    If two intersection points are return;ed, then the entire line 
    between them comprises points of intersection.

    If three intersection points are return;ed, then all points of 
    the triangle intersect the plane.
 
  Modified:

    19 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, the coordinates
    of the vertices of the triangle.

    Input, float A, B, C, D, the parameters that define the implicit plane.

    Output, float DIST, the distance between the triangle and the plane.

    Output, int *NUM_INT, the number of intersection points return;ed.

    Output, float X[3], Y[3], Z[3], the coordinates of the intersection points.
*/
{
  float dist1;
  float dist2;
  float dist3;

  *num_int = 0;
/*
  Compute the signed distances between the vertices and the plane.
*/
  dist1 = a * x1 + b * y1 + c * z1 + d;
  dist2 = a * x2 + b * y2 + c * z2 + d;
  dist3 = a * x3 + b * y3 + c * z3 + d;
/*
  Consider any zero distances.
*/
  if ( dist1 == 0.0 ) {

    x[*num_int] = x1;
    y[*num_int] = y1;
    z[*num_int] = z1;
    *num_int = *num_int + 1;

  }

  if ( dist2 == 0.0 ) {

    x[*num_int] = x2;
    y[*num_int] = y2;
    z[*num_int] = z2;
    *num_int = *num_int + 1;

  }

  if ( dist3 == 0.0 ) {

    x[*num_int] = x3;
    y[*num_int] = y3;
    z[*num_int] = z3;
    *num_int = *num_int + 1;

  }
/*
  If 2 or 3 of the nodes intersect, we're already done.
*/
  if ( *num_int >= 2 ) {
    return;
  }
/*
  If one node intersects, then we're done unless the other two
  are of opposite signs.
*/
  if ( *num_int == 1 ) {

    if ( dist1 == 0.0 ) {

      plane_imp_triangle_int_add_3d ( x2, y2, z2, x3, y3, z3, 
        dist2, dist3, num_int, x, y, z );

    }
    else if ( dist2 == 0.0 ) {

      plane_imp_triangle_int_add_3d ( x1, y1, z1, x3, y3, z3, 
        dist1, dist3, num_int, x, y, z );

    }
    else if ( dist3 == 0.0 ) {

      plane_imp_triangle_int_add_3d ( x1, y1, z1, x2, y2, z2, 
        dist1, dist2, num_int, x, y, z );

    }

    return;

  }
/*
  All nodal distances are nonzero, and there is at least one
  positive and one negative.
*/
  if ( dist1 * dist2 < 0.0 && dist1 * dist3 < 0.0 ) {

    plane_imp_triangle_int_add_3d ( x1, y1, z1, x2, y2, z2, 
      dist1, dist2, num_int, x, y, z );

    plane_imp_triangle_int_add_3d ( x1, y1, z1, x3, y3, z3, 
      dist1, dist3, num_int, x, y, z );

  }
  else if ( dist2 * dist1 < 0.0 && dist2 * dist3 < 0.0 ) {

    plane_imp_triangle_int_add_3d ( x2, y2, z2, x1, y1, z1, 
      dist2, dist1, num_int, x, y, z );

    plane_imp_triangle_int_add_3d ( x2, y2, z2, x3, y3, z3, 
      dist2, dist3, num_int, x, y, z );

  }
  else if ( dist3 * dist1 < 0.0 && dist3 * dist2 < 0.0 ) {

    plane_imp_triangle_int_add_3d ( x3, y3, z3, x1, y1, z1, 
      dist3, dist1, num_int, x, y, z );

    plane_imp_triangle_int_add_3d ( x3, y3, z3, x2, y2, z2, 
      dist3, dist2, num_int, x, y, z );

  }

  return;
}
/**********************************************************************/

void plane_imp_triangle_int_add_3d ( float x1, float y1, float z1, 
  float x2, float y2, float z2, float dist1, float dist2, 
  int *num_int, float x[], float y[], float z[] )

/**********************************************************************/

/*
  Purpose:

    PLANE_IMP_TRIANGLE_INT_ADD_3D is a utility for PLANE_IMP_TRIANGLE_INT_3D.

  Discussion:

    This routine is called to consider the value of the signed distance
    from a plane of two nodes of a triangle.  If the two values
    have opposite signs, then there is a point of intersection between
    them.  The routine computes this point and adds it to the list.

  Modified:

    19 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, the coordinates
    of two vertices of a triangle.

    Input, float DIST1, DIST2, the signed distances of the two vertices
    from a plane.  

    Input/output, int *NUM_INT, the number of intersection points.

    Input/output, float X[], Y[], Z[], the coordinates
    of the intersection points.
*/
{
  float alpha;

  if ( dist1 == 0.0 ) {
    x[*num_int] = x1;
    y[*num_int] = y1;
    z[*num_int] = z1;
    *num_int = *num_int + 1;
  }
  else if ( dist2 == 0.0 ) {
    x[*num_int] = x2;
    y[*num_int] = y2;
    z[*num_int] = z2;
    *num_int = *num_int + 1;
  }
  else if ( dist1 * dist2 < 0.0 ) {
    alpha = dist2 / ( dist2 - dist1 );
    x[*num_int] = alpha * x1 + ( 1.0 - alpha ) * x2;
    y[*num_int] = alpha * y1 + ( 1.0 - alpha ) * y2;
    z[*num_int] = alpha * z1 + ( 1.0 - alpha ) * z2;
    *num_int = *num_int + 1;
  }

  return;
}
/**********************************************************************/

void plane_norm_basis_3d ( float xp, float yp, float zp, float xn, 
  float yn, float zn, float *xq, float *yq, float *zq, float *xr, 
  float *yr, float *zr )

/**********************************************************************/

/*
  Purpose:

    PLANE_NORM_BASIS_3D finds two perpendicular vectors in a plane in 3D.

  Discussion:

    Given a plane in point, normal form P = (XP,YP,ZP) and N = (XN,YN,ZN),
    any point in that plane can be described in terms of the point P
    plus a weighted sum of two vectors Q = (XQ,YQ,ZQ) and R = (XR,YR,ZR):

      (X,Y,Z) = (XP,YP,ZP) + a * (XQ,YQ,ZQ) + b * (XR,YR,ZR).

    The vector Q has unit length, and is perpendicular to P and R;
    the vector R has unit length, and is perpendicular to P and Q.

  Modified:

    25 June 1999

  Author:

    John Burkardt

  Parameters:

    Input, float XP, YP, ZP, a point on the plane.  (Actually, we never
    need to know these values to do the calculation!)

    Input, float XN, YN, ZN, a normal vector N to the plane.  The
    vector must not have zero length, but it is not necessary for N
    to have unit length.

    Output, float *XQ, *YQ, *ZQ, a vector of unit length, perpendicular
    to the vector N and the vector R.

    Output, float *XR, *YR, *ZR, a vector of unit length, perpendicular
    to the vector N and the vector Q.
*/
{
  float dot;
  float min_com;
  float norm_n;
  float norm_q;
/*
  Compute the length of N = (XN,YN,ZN).
*/
  norm_n = sqrt ( xn * xn + yn * yn + zn * zn );

  if ( norm_n == 0.0 ) {
    printf ( "\n" );
    printf ( "PLANE_NORM_BASIS_3D - Fatal error!\n" );
    printf ( "  The normal vector is 0.\n" );
    exit ( EXIT_FAILURE );
  }
/*
  To find a vector distinct from N, find the minimum component
  of N, and set the corresponding component of Q to 1, the
  rest to zero.
*/
  *xq = 0.0;
  *yq = 0.0;
  *zq = 0.0;

  min_com = fabs ( xn );

  if ( fabs ( yn ) < min_com ) {
    min_com = fabs ( yn );
  }

  if ( fabs ( zn ) < min_com ) {
    min_com = fabs ( zn );
  }

  if ( min_com == fabs ( xn ) ) {
    *xq = 1.0;
  }
  else if ( min_com == fabs ( yn ) ) {
    *yq = 1.0;
  }
  else if ( min_com == fabs ( zn ) ) {
    *zq = 1.0;
  }
/*
  Now subtract off the component of Q in the direction of N,
  computing Q = Q - Q dot N / || N ||,
  and then normalize.
*/
  dot = ( *xq * xn + *yq * yn + *zq * zn ) / norm_n;

  *xq = *xq - dot * xn / norm_n;
  *yq = *yq - dot * yn / norm_n;
  *zq = *zq - dot * zn / norm_n;

  norm_q = sqrt ( *xq * *xq + *yq * *yq + *zq * *zq );

  *xq = *xq / norm_q;
  *yq = *yq / norm_q;
  *zq = *zq / norm_q;
/*
  Now just take the cross product N x Q to get the R vector.
  Plus, if we did things right, R will already have unit length.
*/
  *xr = ( yn * *zq - zn * *yq ) / norm_n;
  *yr = ( zn * *xq - xn * *zq ) / norm_n;
  *zr = ( xn * *yq - yn * *xq ) / norm_n;

  return;
}

/**********************************************************************/

void plane_norm2imp_3d ( float xp, float yp, float zp, float xn, 
  float yn, float zn, float *a, float *b, float *c, float *d )

/**********************************************************************/

/*
  Purpose:

    PLANE_NORM2IMP_3D converts a normal form plane to implicit form in 3D.

  Definition:

    The normal form of a plane in 3D is

      (Xp,Yp,Zp), a point on the plane, and
      (Xn,Yn,Zn), the unit normal to the plane.

    The implicit form of a plane in 3D is

      A * X + B * Y + C * Z + D = 0.

  Modified:

    22 June 1999

  Author:

    John Burkardt

  Parameters:

    Input, float XP, YP, ZP, a point on the plane.

    Input, float XN, YN, ZN, the unit normal vector to the plane.

    Output, float *A, *B, *C, *D, parameters that define the implicit plane.
*/
{
  *a = xn;
  *b = yn;
  *c = zn;
  *d = - xn * xp - yn * yp - zn * zp;

  return;
}

/**********************************************************************/

float points_colin_2d ( float x1, float y1, float x2, float y2, 
  float x3, float y3 )

/**********************************************************************/

/*
  Purpose:

    POINTS_COLIN_2D estimates the colinearity of 3 points in 2D.

  Modified:

    23 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, X3, Y3, the coordinates of the points.

    Output, float POINTS_COLIN_2D, an estimate of colinearity, namely, the 
    ratio of the area of the triangle spanned by the points to the area
    of the equilateral triangle with the same perimeter.

    This is 1.0 if the points are maximally noncolinear, 0.0 if the
    points are exactly colinear, and otherwise is closer to 1 or 0 depending
    on whether the points are far or close to colinearity.
*/
{
  float area;
  float area2;
  float colin;
  float perim;
  float side;

  area = triangle_area_2d ( x1, y1, x2, y2, x3, y3 );

  if ( area == 0.0 ) {

    colin = 0.0;
  }
  else {

    perim = enorm0_2d ( x1, y1, x2, y2 )
          + enorm0_2d ( x2, y2, x3, y3 )
          + enorm0_2d ( x3, y3, x1, y1 );

    side = perim / 3.0;

    area2 = 0.25 * sqrt ( 3.0 ) * side * side;

    colin = area / area2;

  }
 
  return colin;
}
/**********************************************************************/

float points_colin_3d ( float x1, float y1, float z1, float x2, float y2, 
  float z2, float x3, float y3, float z3 )

/**********************************************************************/

/*
  Purpose:

    POINTS_COLIN_3D estimates the colinearity of 3 points in 3D.

  Modified:

    23 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, the coordinates of 
    the points.

    Output, float POINTS_COLIN_3D, an estimate of colinearity, namely, the ratio
    of the area of the triangle spanned by the points to the area
    of the equilateral triangle with the same perimeter.
    This is 1.0 if the points are maximally noncolinear, 0.0 if the
    points are exactly colinear, and otherwise is closer to 1 or 0 depending
    on whether the points are far or close to colinearity.
*/
{
  float area;
  float area2;
  float colin;
  float perim;
  float side;

  area = triangle_area_3d ( x1, y1, z1, x2, y2, z2, x3, y3, z3 );

  area = fabs ( area );

  if ( area == 0.0 ) {

    colin = 0.0;

  }
  else {

    perim = 
        enorm0_3d ( x1, y1, z1, x2, y2, z2 )
      + enorm0_3d ( x2, y2, z2, x3, y3, z3 )
      + enorm0_3d ( x3, y3, z3, x1, y1, z1 );

    side = perim / 3.0;

    area2 = 0.25 * sqrt ( 3.0 ) * side * side;

    colin = area / area2;

  }
 
  return colin;
}
/**********************************************************************/

float polygon_1_2d ( int n, float x[], float y[] )

/**********************************************************************/

/*
  Purpose:

    POLYGON_1_2D integrates the function 1 over a polygon in 2D.

  Formula:

    INTEGRAL = 0.5 * SUM (I=1 to N) (X(I)+X(I-1)) * (Y(I)-Y(I-1))

    where X[N] and Y[N] should be replaced by X[0] and Y[0].

  Note:

    The integral of 1 over a polygon is the area of the polygon.

  Reference:

    S F Bockman,
    Generalizing the Formula for Areas of Polygons to Moments,
    American Mathematical Society Monthly,
    1989, pages 131-132.

  Modified:

    27 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of vertices of the polygon.
    N should be at least 3 for a nonzero result.

    Input, float X[N], Y[N], the coordinates of the vertices
    of the polygon.  These vertices should be given in
    counter-clockwise order.

    Output, float POLYGON_1_2D, the value of the integral.
*/
{
  int i;
  int im1;
  float result;

  result = 0.0;
 
  if ( n < 3 ) {
    printf ( "\n" );
    printf ( "POLYGON_1_2D - Warning!\n" );
    printf ( "  The number of vertices must be at least 3.\n" );
    printf ( "  The input value of N = %d\n", n );
    return result;
  }
 
  for ( i = 0; i < n; i++ ) {
 
    if ( i == 0 ) {
      im1 = n;
    }
    else {
      im1 = i - 1;
    }
 
    result = result + 0.5 * ( x[i] + x[im1] ) * ( y[i] - y[im1] );
 
  }
 
  return result;
}
/**********************************************************************/

float polygon_area_2d ( int n, float x[], float y[] )

/**********************************************************************/

/*
  Purpose:

    POLYGON_AREA_2D computes the area of a polygon in 2D.

  Formula:

    AREA = ABS ( 0.5 * SUM ( I = 1 to N) X(I) * ( Y(I+1)-Y(I-1) ) )
    where Y[N] should be replaced by Y[0], and Y[N+1] by Y[1].

  Modified:

    27 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of vertices of the polygon.

    Input, float X[N], Y[N], the coordinates of the vertices.

    Output, float POLYGON_AREA_2D, the absolute area of the polygon.
*/
{
  float area;
  int i;
  int im1;
  int ip1;

  area = 0.0;
 
  for ( i = 0; i < n; i++ ) {
 
    if ( i == 0 ) {
      im1 = n;
    }
    else {
      im1 = i - 1;
    }
 
    if ( i < n-1 ) {
      ip1 = i + 1;
    }
    else {
      ip1 = 1;
    }
 
    area = area + x[i] * ( y[ip1] - y[im1] );
 
  }
 
  area = 0.5 * fabs ( area );
 
  return area;
}
/**********************************************************************/

float polygon_area_2_2d ( int n, float x[], float y[] )

/**********************************************************************/

/*
  Purpose:

    POLYGON_AREA_2_2D computes the area of a polygon in 2D.

  Formula:

    The area is the sum of the areas of the triangles formed by
    node N with consecutive pairs of nodes.

  Reference:

    Adrian Bowyer and John Woodwark,
    A Programmer's Geometry,
    Butterworths, 1983.

  Modified:

    27 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of vertices of the polygon.

    Input, float X[N], Y[N], the coordinates of the vertices.

    Output, float POLYGON_AREA_2_2D, the absolute area of the polygon.
*/
{
  float area;
  float areat;
  int i;

  area = 0.0;
 
  for ( i = 0; i < n - 2; i++ ) {
 
    areat = triangle_area_2d ( x[i], y[i], x[i+1], y[i+1], x[n], y[n] );

    area = area + areat;
 
  }
  
  return area;
}
/**********************************************************************/

float polygon_area_2_3d ( int n, float x[], float y[], float z[] )

/**********************************************************************/

/*
  Purpose:

    POLYGON_AREA_2_3D computes the area of a polygon in 3D.

  Formula:

    The area is the sum of the areas of the triangles formed by
    node N with consecutive pairs of nodes.

  Reference:

    Adrian Bowyer and John Woodwark,
    A Programmer's Geometry,
    Butterworths, 1983.

  Modified:

    27 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of vertices of the polygon.

    Input, float X[N], Y[N], Z[N], the coordinates of the vertices.

    Output, float POLYGON_AREA_2_3D, the absolute area of the polygon.
*/
{
  float area;
  float areat;
  int i;

  area = 0.0;
 
  for ( i = 0; i < n - 2; i++ ) {

    areat = triangle_area_3d ( x[i], y[i], z[i], x[i+1], 
      y[i+1], z[i+1], x[n-1], y[n-1], z[n-1] );

    area = area + areat;
 
  }
  
  return area;
}
/**********************************************************************/

float polygon_area_3d ( int n, float x[], float y[], float z[], 
  float normal[] )

/**********************************************************************/

/*
  Purpose:

    POLYGON_AREA_3D computes the area of a polygon in 3D.

  Restriction:

    The computation is not valid unless the vertices floatly do lie
    in a plane, so that the polygon that is defined is "flat".
    The polygon does not have to be "regular", that is, neither its 
    sides nor its angles need to be equal.

  Reference:

    Allen Van Gelder,
    Efficient Computation of Polygon Area and Polyhedron Volume,
    Graphics Gems V, edited by Alan Paeth,
    AP Professional, 1995.

  Modified:

    27 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of vertices.

    Input, float X[N], Y[N], Z[N], the coordinates of the vertices.
    The vertices should be listed in neighboring order.

    Output, float NORMAL[3], the unit normal vector to the polygon. 

    Output, float POLYGON_AREA_3D, the area of the polygon.
*/
{
  float area;
  int i;
  int ip1;
  float x1;
  float x2;
  float x3;
  float y1;
  float y2;
  float y3;
  float z1;
  float z2;
  float z3;

  normal[0] = 0.0;
  normal[1] = 0.0;
  normal[2] = 0.0;

  for ( i = 0; i < n; i++ ) {

    x1 = x[i];
    y1 = y[i];
    z1 = z[i];

    if ( i < n - 1 ) {
      ip1 = i + 1;
    }
    else {
      ip1 = 1;
    }

    x2 = x[ip1];
    y2 = y[ip1];
    z2 = z[ip1];

    cross_3d ( x1, y1, z1, x2, y2, z2, &x3, &y3, &z3 );

    normal[0] = normal[0] + x3;
    normal[1] = normal[1] + y3;
    normal[2] = normal[2] + z3;

  }

  area = sqrt ( 
      normal[0] * normal[0]
    + normal[1] * normal[1]
    + normal[2] * normal[2] );

  if ( area != 0.0 ) {
    normal[0] = normal[0] / area;
    normal[1] = normal[1] / area;
    normal[2] = normal[2] / area;
  }
  else {
    normal[0] = 1.0;
    normal[1] = 0.0;
    normal[2] = 0.0;
  }

  area = 0.5 * area;

  return area;
}
/**********************************************************************/

void polygon_centroid_2d ( int n, float x[], float y[], 
  float *cx, float *cy )

/**********************************************************************/

/*
  Purpose:

    POLYGON_CENTROID_2D computes the centroid of a polygon in 2D.

  Formula:

    Denoting the centroid coordinates by (CX,CY), then

      CX = Integral ( Polygon interior ) x dx dy / Area ( Polygon )
      CY = Integral ( Polygon interior ) y dx dy / Area ( Polygon ).

    Green's theorem states that

      Integral ( Polygon boundary ) ( M dx + N dy ) =
      Integral ( Polygon interior ) ( dN/dx - dM/dy ) dx dy.

    Using M = 0 and N = x**2/2, we get:

      CX = 0.5 * Integral ( Polygon boundary ) x**2 dy,

    which becomes

      CX = 1/6 SUM ( I = 1 to N ) 
        ( X[I+1] + X[I] ) * ( X[I] * Y[I+1] - X[I+1] * Y[I] )

    where, when I = N, the index "I+1" is replaced by 1.

    A similar calculation gives us a formula for CY.

  Reference:

    Gerard Bashein and Paul Detmer,
    Centroid of a Polygon,
    Graphics Gems IV, edited by Paul Heckbert,
    AP Professional, 1994.

  Modified:

    27 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of sides of the polygonal shape.

    Input, float X[N], Y[N], the coordinates of the vertices of the shape.

    Output, float *CX, *CY, the coordinates of the centroid of the shape.
*/
{
  float area;
  int i;
  int ip1;
  float temp;

  area = 0.0;
  *cx = 0.0;
  *cy = 0.0;

  for ( i = 0; i < n; i++ ) {

    if ( i < n - 1 ) {
      ip1 = i + 1;
    }
    else {
      ip1 = 0;
    }

    temp = ( x[i] * y[ip1] - x[ip1] * y[i] );

    area = area + temp;
    *cx = *cx + ( x[ip1] + x[i] ) * temp;
    *cy = *cy + ( y[ip1] + y[i] ) * temp;

  }

  area = area / 2.0;

  *cx = *cx / ( 6.0 * area );
  *cy = *cy / ( 6.0 * area );

  return;
}
/**********************************************************************/

void polygon_centroid_2_2d ( int n, float x[], float y[], float *cx, 
  float *cy )

/**********************************************************************/

/*
  Purpose:

    POLYGON_CENTROID_2_2D computes the centroid of a polygon in 2D.

  Method:

    The centroid is the area-weighted sum of the centroids of
    disjoint triangles that make up the polygon.
 
  Reference:

    Adrian Bowyer and John Woodwark,
    A Programmer's Geometry,
    Butterworths, 1983.

  Modified:

    27 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of vertices of the polygon.

    Input, float X[N], Y[N], the coordinates of the vertices. 

    Output, float *CX, *CY, the coordinates of the centroid.
*/
{
  float area;
  float areat;
  int i;

  area = 0.0;
  *cx = 0.0;
  *cy = 0.0;
 
  for ( i = 0; i < n-2; i ++ ) {
 
    areat = triangle_area_2d ( x[i], y[i], x[i+1], y[i+1], x[n-1], y[n-1] );

    area = area + areat;
    *cx = *cx + areat * ( x[i] + x[i+1] + x[n-1] ) / 3.0;
    *cy = *cy + areat * ( y[i] + y[i+1] + y[n-1] ) / 3.0;
 
  }

  *cx = *cx / area;
  *cy = *cy / area;
  
  return;
}
/**********************************************************************/

void polygon_centroid_3d ( int n, float x[], float y[], float z[], 
  float *cx, float *cy, float *cz )

/**********************************************************************/

/*
  Purpose:

    POLYGON_CENTROID_3D computes the centroid of a polygon in 3D.

  Method:

    The centroid is the area-weighted sum of the centroids of
    disjoint triangles that make up the polygon.
 
  Reference:

    Adrian Bowyer and John Woodwark,
    A Programmer's Geometry,
    Butterworths, 1983.

  Modified:

    27 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of vertices of the polygon.

    Input, float X[N], Y[N], Z[N], the coordinates of the vertices.

    Output, float CX, CY, CZ, the coordinates of the centroid.
*/
{
  float area;
  float areat;
  int i;

  area = 0.0;
  *cx = 0.0;
  *cy = 0.0;
  *cz = 0.0;
 
  for ( i = 0; i < n - 2; i++ ) {
 
    areat = triangle_area_3d ( x[i], y[i], z[i], x[i+1], 
      y[i+1], z[i+1], x[n-1], y[n-1], z[n-1] );

    area = area + areat;
    *cx = *cx + areat * ( x[i] + x[i+1] + x[n-1] ) / 3.0;
    *cy = *cy + areat * ( y[i] + y[i+1] + y[n-1] ) / 3.0;
    *cz = *cz + areat * ( z[i] + z[i+1] + z[n-1] ) / 3.0;
 
  }

  *cx = *cx / area;
  *cy = *cy / area;
  *cz = *cz / area;
  
  return;
}

/**********************************************************************/

float polygon_x_2d ( int n, float x[], float y[] )

/**********************************************************************/

/*
  Purpose:

    POLYGON_X_2D integrates the function X over a polygon in 2D.

  Formula:

    INTEGRAL = (1/6) * SUM ( I = 1 to N )
      ( X[I]**2 + X[I] * X[I-1] + X[I-1]**2 ) * ( Y[I] - Y[I-1] )

    where X[N] and Y[N] should be replaced by X[0] and Y[0].

  Modified:

    27 May 1999

  Author:

    John Burkardt

  Reference:

    S F Bockman,
    Generalizing the Formula for Areas of Polygons to Moments,
    American Mathematical Society Monthly,
    1989, pages 131-132.

  Parameters:

    Input, int N, the number of vertices of the polygon.
    N should be at least 3 for a nonzero result.

    Input, float X[N], Y[N], the coordinates of the vertices
    of the polygon.  These vertices should be given in
    counter-clockwise order.

    Output, float POLYGON_X_2D, the value of the integral.
*/
{
  int i;
  int im1;
  float result;

  result = 0.0;
 
  if ( n < 3 ) {
    printf ( "\n" );
    printf ( "POLYGON_X_2D - Warning!\n" );
    printf ( "  The number of vertices must be at least 3.\n" );
    printf ( "  The input value of N = %d\n", n );
    return result;
  }
 
  for ( i = 0; i < n; i++ ) {
 
    if ( i == 0 ) {
      im1 = n;
    }
    else {
      im1 = i - 1;
    }
 
    result = result + ( x[i] * x[i] + x[i] * x[im1] + x[im1] * x[im1] )
      * ( y[i] - y[im1] );
 
  }
 
  result = result / 6.0;
 
  return result;
}
/**********************************************************************/

float polygon_y_2d ( int n, float x[], float y[] )

/**********************************************************************/

/*
  Purpose:

    POLYGON_Y_2D integrates the function Y over a polygon in 2D.

  Formula:

    INTEGRAL = (1/6) * SUM ( I = 1 to N )
      - ( Y[I]**2 + Y[I] * Y[I-1] + Y[I-1]**2 ) * ( X[I] - X[I-1] )

    where X[N] and Y[N] should be replaced by X[0] and Y[0].

  Reference:

    S F Bockman,
    Generalizing the Formula for Areas of Polygons to Moments,
    American Mathematical Society Monthly,
    1989, pages 131-132.

  Modified:

    27 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of vertices of the polygon.
    N should be at least 3 for a nonzero result.

    Input, float X[N], Y[N], the coordinates of the vertices
    of the polygon.  These vertices should be given in
    counter-clockwise order.

    Output, float POLYGON_Y_2D, the value of the integral.
*/
{
  int i;
  int im1;
  float result;

  result = 0.0;
 
  if ( n < 3 ) {
    printf ( "\n" );
    printf ( "POLYGON_Y_2D - Warning!\n" );
    printf ( "  The number of vertices must be at least 3.\n" );
    printf ( "  The input value of N = %d\n", n );
    return result;
  }
 
  for ( i = 0; i < n; i++ ) {
 
    if ( i == 0 ) {
      im1 = n;
    }
    else {
      im1 = i - 1;
    }
 
    result = result - ( y[i] * y[i] + y[i] * y[im1] + y[im1] * y[im1] )
      * ( x[i] - x[im1] );
 
  }
 
  result = result / 6.0;
 
  return result;
}
/**********************************************************************/

float polygon_xx_2d ( int n, float x[], float y[] )

/**********************************************************************/

/*
  Purpose:

    POLYGON_XX_2D integrates the function X*X over a polygon in 2D.

  Formula:

    INTEGRAL = (1/12) * SUM ( I = 1 to N )
      ( X[I]**3 + X[I]**2 * X[I-1] + X[I] * X[I-1]**2 + X[I-1]**3 )
      * ( Y[I] - Y[I-1] )

    where X[N] and Y[N] should be replaced by X[0] and Y[0].

  Reference:

    S F Bockman,
    Generalizing the Formula for Areas of Polygons to Moments,
    American Mathematical Society Monthly,
    1989, pages 131-132.

  Modified:

    27 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of vertices of the polygon.
    N should be at least 3 for a nonzero result.

    Input, float X[N], Y[N], the coordinates of the vertices
    of the polygon.  These vertices should be given in
    counter-clockwise order.

    Output, float POLYGON_XX_2D, the value of the integral.
*/
{
  int i;
  int im1;
  float result;

  result = 0.0;
 
  if ( n < 3 ) {
    printf ( "\n" );
    printf ( "POLYGON_XX_2D - Warning!\n" );
    printf ( "  The number of vertices must be at least 3.\n" );
    printf ( "  The input value of N = %d\n", n );
    return result;
  }
 
  for ( i = 0; i < n; i++ ) {
 
    if ( i == 0 ) {
      im1 = n;
    }
    else {
      im1 = i - 1;
    }
 
    result = result + ( 
        x[i]   * x[i]   * x[i] 
      + x[i]   * x[i]   * x[im1] 
      + x[i]   * x[im1] * x[im1] 
      + x[im1] * x[im1] * x[im1] ) * ( y[i] - y[im1] );
 
  }
 
  result = result / 12.0;
 
  return result;
}
/**********************************************************************/

float polygon_xy_2d ( int n, float x[], float y[] )

/**********************************************************************/

/*
  Purpose:

    POLYGON_XY_2D integrates the function X*Y over a polygon in 2D.

  Formula:

    INTEGRAL = (1/24) * SUM (I=1 to N)
      ( Y[I] * 
        ( 3 * X[I]**2 + 2 * X[I] * X[I-1] + X[I-1]**2 )
      + Y[I-1] *
        ( X[I]**2 + 2 * X[I] * X[I-1] + 3 * X[I-1]**2 ) 
      ) * ( Y[I] - Y[I-1] )

    where X[N] and Y[N] should be replaced by X[0] and Y[0].

  Reference:

    S F Bockman,
    Generalizing the Formula for Areas of Polygons to Moments,
    American Mathematical Society Monthly,
    1989, pages 131-132.

  Modified:

    27 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of vertices of the polygon.
    N should be at least 3 for a nonzero result.

    Input, float X[N], Y[N], the coordinates of the vertices
    of the polygon.  These vertices should be given in
    counter-clockwise order.

    Output, float POLYGON_XY_2D, the value of the integral.
*/
{
  int i;
  int im1;
  float result;

  result = 0.0;
 
  if ( n < 3 ) {
    printf ( "\n" );
    printf ( "POLYGON_XY_2D - Warning!\n" );
    printf ( "  The number of vertices must be at least 3.\n" );
    printf ( "  The input value of N = %d\n", n );
    return result;
  }
 
  for ( i = 0; i < n; i++ ) {
 
    if ( i == 0 ) {
      im1 = n;
    }
    else {
      im1 = i - 1;
    }
 
    result = result + (
      y[i] * ( 
        3.0 * x[i] * x[i] + 2.0 * x[i] * x[im1] +       x[im1] * x[im1] )
      + y[im1] * (
              x[i] * x[i] + 2.0 * x[i] * x[im1] + 3.0 * x[im1] * x[im1] )
      ) * ( y[i] - y[im1] );
 
  }
 
  result = result / 24.0;
 
  return result;
}
/**********************************************************************/

float polygon_yy_2d ( int n, float x[], float y[] )

/**********************************************************************/

/*
  Purpose:

    POLYGON_YY_2D integrates the function Y*Y over a polygon in 2D.

  Formula:

    INTEGRAL = (1/12) * SUM ( I = 1 to N )
      - ( Y[I]**3 + Y[I]**2 * Y[I-1] + Y[I] * Y[I-1]**2 + Y[I-1]**3 )
      * ( X[I] - X[I-1] )

    where X[N] and Y[N] should be replaced by X[0] and Y[0].

  Reference:

    S F Bockman,
    Generalizing the Formula for Areas of Polygons to Moments,
    American Mathematical Society Monthly,
    1989, pages 131-132.

  Modified:

    27 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of vertices of the polygon.
    N should be at least 3 for a nonzero result.

    Input, float X[N], Y[N], the coordinates of the vertices
    of the polygon.  These vertices should be given in
    counter-clockwise order.

    Output, float POLYGON_YY_2D, the value of the integral.

*/
{
  int i;
  int im1;
  float result;

  result = 0.0;
 
  if ( n < 3 ) {
    printf ( "\n" );
    printf ( "POLYGON_YY_2D - Warning!\n" );
    printf ( "  The number of vertices must be at least 3.\n" );
    printf ( "  The input value of N = %d\n", n );
    return result;
  }
 
  for ( i = 0; i < n; i++ ) {
 
    if ( i == 0 ) {
      im1 = n;
    }
    else {
      im1 = i - 1;
    }
 
    result = result -
      ( y[i]   * y[i]   * y[i]
      + y[i]   * y[i]   * y[im1] 
      + y[i]   * y[im1] * y[im1]
      + y[im1] * y[im1] * y[im1] ) 
      * ( x[i] - x[im1] );
 
  }
 
  result = result / 12.0;
 
  return result;
}
/**********************************************************************/

float rmat2_det ( float a[2][2] )

/**********************************************************************/

/*
  Purpose:

    RMAT2_DET computes the determinant of a 2 by 2 matrix.

  Formula:

    The determinant of a 2 by 2 matrix is

      a11 * a22 - a12 * a21.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float A[2][2], the matrix whose determinant is desired.

    Output, float RMAT2_DET, the determinant of the matrix.
*/
{
  float det;

  det = a[0][0] * a[1][1] - a[0][1] * a[1][0];

  return det;
}
/**********************************************************************/

float rmat2_inverse ( float a[2][2], float b[2][2] )

/**********************************************************************/

/*
  Purpose:

    RMAT2_INVERSE inverts a 2 by 2 float matrix using Cramer's rule.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float A[2][2], the matrix to be inverted.

    Output, float B[2][2], the inverse of the matrix A.

    Output, float RMAT2_INVERSE, the determinant of the matrix A.

    If the determinant is zero, then A is singular, and does not have an
    inverse.  In that case, B is simply set to zero, and a
    message is printed.

    If the determinant is nonzero, then its value is roughly an estimate
    of how nonsingular the matrix A is.
*/
{
  float det;
  int i;
  int j;
/*
  Compute the determinant of A.
*/
  det = a[0][0] * a[1][1] - a[0][1] * a[1][0];
/*
  If the determinant is zero, bail out.
*/
  if ( det == 0.0 ) {

    for ( i = 0; i < 2; i++ ) {
      for ( j = 0; j < 2; j++ ) {
        b[i][j] = 0.0;
      }
    }

    return det;
  }
/*
  Compute the entries of the inverse matrix using an explicit formula.
*/
  b[0][0] = + a[1][1] / det;
  b[0][1] = - a[0][1] / det;
  b[1][0] = - a[1][0] / det;
  b[1][1] = + a[0][0] / det;

  return det;
}
/**********************************************************************/

float rmat3_det ( float a[3][3] )

/**********************************************************************/

/*
  Purpose:

    RMAT3_DET computes the determinant of a 3 by 3 matrix.

  Formula:

    The determinant of a 3 by 3 matrix is

        a11 * a22 * a33 - a11 * a23 * a32
      + a12 * a23 * a31 - a12 * a21 * a33
      + a13 * a21 * a32 - a13 * a22 * a31

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float A[3][3], the matrix whose determinant is desired.

    Output, float RMAT3_DET, the determinant of the matrix.
*/
{
  float det;

  det =
      a[0][0] * ( a[1][1] * a[2][2] - a[1][2] * a[2][1] )
    + a[0][1] * ( a[1][2] * a[2][0] - a[1][0] * a[2][2] )
    + a[0][2] * ( a[1][0] * a[2][1] - a[1][1] * a[2][0] );

  return det;
}
/**********************************************************************/

float rmat3_inverse ( float a[3][3], float b[3][3] )

/**********************************************************************/

/*
  Purpose:

    RMAT3_INVERSE inverts a 3 by 3 matrix using Cramer's rule.

  Modified:

    14 December 1998

  Author:

    John Burkardt

  Parameters:

    Input, float A[3][3], the matrix to be inverted.

    Output, float B[3][3], the inverse of the matrix A.

    Output, float RMAT3_INVERSE, the determinant of the matrix A.

    If the determinant is zero, A is singular, and does not have an
    inverse.  In that case, B is simply set to zero.

    If the determinant is nonzero, its value is an estimate
    of how nonsingular the matrix A is.
*/
{
  float det;
  int i;
  int j;
/*
  Compute the determinant of A.
*/
  det =
     a[0][0] * ( a[1][1] * a[2][2] - a[1][2] * a[2][1] )
   + a[0][1] * ( a[1][2] * a[2][0] - a[1][0] * a[2][2] )
   + a[0][2] * ( a[1][0] * a[2][1] - a[1][1] * a[2][0] );

  if ( det == 0.0 ) {
    for ( i = 0; i < 3; i++ ) {
      for ( j = 0; j < 3; j++ ) {
        b[i][j] = 0.0;
      }
    }
  }
  else {
    b[0][0] =   ( a[1][1] * a[2][2] - a[1][2] * a[2][1] ) / det;
    b[0][1] = - ( a[0][1] * a[2][2] - a[0][2] * a[2][1] ) / det;
    b[0][2] =   ( a[0][1] * a[1][2] - a[0][2] * a[1][1] ) / det;

    b[1][0] = - ( a[1][0] * a[2][2] - a[1][2] * a[2][0] ) / det;
    b[1][1] =   ( a[0][0] * a[2][2] - a[0][2] * a[2][0] ) / det;
    b[1][2] = - ( a[0][0] * a[1][2] - a[0][2] * a[1][0] ) / det;

    b[2][0] =   ( a[1][0] * a[2][1] - a[1][1] * a[2][0] ) / det;
    b[2][1] = - ( a[0][0] * a[2][1] - a[0][1] * a[2][0] ) / det;
    b[2][2] =   ( a[0][0] * a[1][1] - a[0][1] * a[1][0] ) / det;

  }

  return det;
}
/**********************************************************************/

float rmat4_det ( float a[4][4] )

/**********************************************************************/

/*
  Purpose:

    RMAT4_DET computes the determinant of a 4 by 4 matrix.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float A[4][4], the matrix whose determinant is desired.

    Output, float RMAT4_DET, the determinant of the matrix.
*/
{
  float det;

  det =
      a[0][0] * (
          a[1][1] * ( a[2][2] * a[3][3] - a[2][3] * a[3][2] )
        - a[1][2] * ( a[2][1] * a[3][3] - a[2][3] * a[3][1] )
        + a[1][3] * ( a[2][1] * a[3][2] - a[2][2] * a[3][1] ) )
    - a[0][1] * (
          a[1][0] * ( a[2][2] * a[3][3] - a[2][3] * a[3][2] )
        - a[1][2] * ( a[2][0] * a[3][3] - a[2][3] * a[3][0] )
        + a[1][3] * ( a[2][0] * a[3][2] - a[2][2] * a[3][0] ) )
    + a[0][2] * (
          a[1][0] * ( a[2][1] * a[3][3] - a[2][3] * a[3][1] )
        - a[1][1] * ( a[2][0] * a[3][3] - a[2][3] * a[3][0] )
        + a[1][3] * ( a[2][0] * a[3][1] - a[2][1] * a[3][0] ) )
    - a[0][3] * (
          a[1][0] * ( a[2][1] * a[3][2] - a[2][2] * a[3][1] )
        - a[1][1] * ( a[2][0] * a[3][2] - a[2][2] * a[3][0] )
        + a[1][2] * ( a[2][0] * a[3][1] - a[2][1] * a[3][0] ) );

  return det;
}
/**********************************************************************/

float rmat5_det ( float a[5][5] )

/**********************************************************************/

/*
  Purpose:

    RMAT5_DET computes the determinant of a 5 by 5 matrix.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float A[5][5], the matrix whose determinant is desired.

    Output, float RMAT5_DET, the determinant of the matrix.
*/
{
  float b[4][4];
  float det;
  int i;
  int inc;
  int j;
  int k;
  float sign;
/*
  Expand the determinant into the sum of the determinants of the
  five 4 by 4 matrices created by dropping row 1, and column k.
*/
  det = 0.0;
  sign = 1.0;

  for ( k = 0; k < 5; k++ ) {

    for ( i = 0; i < 4; i++ ) {
      for ( j = 0; j < 4; j++ ) {

        if ( j < k ) {
          inc = 0;
        }
        else {
          inc = 1;
        }

        b[i][j] = a[i+1][j+inc];

      }
    }

    det = det + sign * a[0][k] * rmat4_det ( b );

    sign = - sign;

  }

  return det;
}
/**********************************************************************/

float sphere_imp_volume_3d ( float r )

/**********************************************************************/

/*
  Purpose:

    SPHERE_IMP_VOLUME_3D computes the volume of an implicit sphere in 3D.

  Formula:

    An implicit sphere in 3D satisfies the equation:

      ( X - XC )**2 + ( Y - YC )**2 + ( Z - ZC )**2 = R**2

  Modified:

    22 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float R, the radius of the sphere.

    Output, float SPHERE_IMP_VOLUME_3D, the volume of the sphere.
*/
{
  float volume;

  volume = 4.0 * PI * r * r * r / 3.0;

  return volume;
}
/**********************************************************************/

float tan_deg ( float angle )

/**********************************************************************/

/*
  Purpose:

    TAN_DEG returns the tangent of an angle given in degrees.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float ANGLE, the angle, in degrees.

    Output, float TAN_DEG, the tangent of the angle.
*/
{
  float angle_rad;
  float value;

  angle_rad = angle * DEG_TO_RAD;
  value  = sin ( angle_rad ) / cos ( angle_rad );

  return value;
}
/**********************************************************************/

void tetra_centroid_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float x3, float y3, float z3, float x4, 
  float y4, float z4, float *xc, float *yc, float *zc )

/**********************************************************************/

/*
  Purpose:

    TETRA_CENTROID_3D computes the centroid of a tetrahedron in 3D.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, X4, Y4, Z4, the 
    coordinates of the vertices.

    Output, float *XC, *YC, *ZC, the coordinates of the centroid.
*/
{
  *xc = 0.25 * ( x1 + x2 + x3 + x4 );
  *yc = 0.25 * ( y1 + y2 + y3 + y4 );
  *zc = 0.25 * ( z1 + z2 + z3 + z4 );

  return;
}
/**********************************************************************/

float tetra_volume_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float x3, float y3, float z3, float x4, 
  float y4, float z4 )

/**********************************************************************/

/*
  Purpose:

    TETRA_VOLUME_3D computes the volume of a tetrahedron in 3D.

  Modified:

    22 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, X4, Y4, Z4, the 
    coordinates of the corners of the tetrahedron.

    Output, float TETRA_VOLUME_3D, the volume of the tetrahedron.
*/
{
  float a[4][4];
  float volume;

  a[0][0] = x1;
  a[1][0] = x2;
  a[2][0] = x3;
  a[3][0] = x4;

  a[0][1] = y1;
  a[1][1] = y2;
  a[2][1] = y3;
  a[3][1] = y4;

  a[0][2] = z1;
  a[1][2] = z2;
  a[2][2] = z3;
  a[3][2] = z4;

  a[0][3] = 1.0;
  a[1][3] = 1.0;
  a[2][3] = 1.0;
  a[3][3] = 1.0;

  volume = fabs ( rmat4_det ( a ) ) / 6.0;

  return volume;
}
/*********************************************************************/

void tmat_init ( float a[4][4] )

/*********************************************************************/

/*
  Purpose:

    TMAT_INIT initializes the geometric transformation matrix.

  Definition:

    The geometric transformation matrix can be thought of as a 4 by 4
    matrix "A" having components:

      r11 r12 r13 t1
      r21 r22 r23 t2
      r31 r32 r33 t3
        0   0   0  1

    This matrix encodes the rotations, scalings and translations that
    are applied to graphical objects.

    A point P = (x,y,z) is rewritten in "homogeneous coordinates" as 
    PH = (x,y,z,1).  Then to apply the transformations encoded in A to 
    the point P, we simply compute A * PH.

    Individual transformations, such as a scaling, can be represented
    by simple versions of the transformation matrix.  If the matrix
    A represents the current set of transformations, and we wish to 
    apply a new transformation B, { the original points are
    transformed twice:  B * ( A * PH ).  The new transformation B can
    be combined with the original one A, to give a single matrix C that
    encodes both transformations: C = B * A.

  Modified:

    19 October 1998

  Author:

    John Burkardt

  Reference:

    Foley, van Dam, Feiner, Hughes,
    Computer Graphics, Principles and Practice,
    Addison Wesley, Second Edition, 1990.

  Parameters:

    Input, float A[4][4], the geometric transformation matrix.
*/
{
  int i;
  int j;

  for ( i = 0; i < 4; i++ ) {
    for ( j = 0; j < 4; j++ ) {
      if ( i == j ) {
        a[i][j] = 1.0;
      }
      else {
        a[i][j] = 0.0;
      }
    }
  }
  return;
}
/*********************************************************************/

void tmat_mxm ( float a[4][4], float b[4][4], float c[4][4] )

/*********************************************************************/

/*
  Purpose:

    TMAT_MXM multiplies two geometric transformation matrices.

  Note:

    The product is accumulated in a temporary array, and { assigned
    to the result.  Therefore, it is legal for any two, or all three,
    of the arguments to share memory.

  Modified:

    19 October 1998

  Author:

    John Burkardt

  Reference:

    Foley, van Dam, Feiner, Hughes,
    Computer Graphics, Principles and Practice,
    Addison Wesley, Second Edition, 1990.

  Parameters:

    Input, float A[4][4], the first geometric transformation matrix.

    Input, float B[4][4], the second geometric transformation matrix.

    Output, float C[4][4], the product A * B.
*/
{
  float d[4][4];
  int i;
  int j;
  int k;

  for ( i = 0; i < 4; i++ ) {
    for ( k = 0; k < 4; k++ ) {
      d[i][k] = 0.0;
      for ( j = 0; j < 4; j++ ) {
        d[i][k] = d[i][k] + a[i][j] * b[j][k];
      }
    }
  }

  for ( i = 0; i < 4; i++ ) {
    for ( j = 0; j < 4; j++ ) {
      c[i][j] = d[i][j];
    }
  }
  return;
}
/*********************************************************************/

void tmat_mxp ( float a[4][4], float x[4], float y[4] )

/*********************************************************************/

/*
  Purpose:

    TMAT_MXP multiplies a geometric transformation matrix times a point.

  Modified:

    19 October 1998

  Author:

    John Burkardt

  Reference:

    Foley, van Dam, Feiner, Hughes,
    Computer Graphics, Principles and Practice,
    Addison Wesley, Second Edition, 1990.

  Parameters:

    Input, float A[4][4], the geometric transformation matrix.

    Input, float X[4], the point to be multiplied.  The fourth component
    of X is implicitly assigned the value of 1.

    Output, float Y[4], the result of A*X.  The product is accumulated in 
    a temporary vector, and { assigned to the result.  Therefore, it 
    is legal for X and Y to share memory.
*/
{
  int i;
  int j;
  float z[4];

  for ( i = 0; i < 3; i++ ) {
    z[i] = a[i][3];
    for ( j = 0; j < 3; j++ ) {
      z[i] = z[i] + a[i][j] * x[j];
    }
  }

  for ( i = 0; i < 3; i++ ) {
    y[i] = z[i];
  }
  return;
}
/*********************************************************************/

void tmat_mxp2 ( float a[4][4], float x[][3], float y[][3], int n )

/*********************************************************************/

/*
  Purpose:

    TMAT_MXP2 multiplies a geometric transformation matrix times N points.

  Modified:

    20 October 1998

  Author:

    John Burkardt

  Reference:

    Foley, van Dam, Feiner, Hughes,
    Computer Graphics, Principles and Practice,
    Addison Wesley, Second Edition, 1990.

  Parameters:

    Input, float A[4][4], the geometric transformation matrix.

    Input, float X[N][3], the points to be multiplied.  

    Output, float Y[N][3], the transformed points.  Each product is 
    accumulated in a temporary vector, and { assigned to the
    result.  Therefore, it is legal for X and Y to share memory.
*/
{
  int i;
  int j;
  int k;
  float z[4];

  for ( k = 0; k < n; k++ ) {

    for ( i = 0; i < 3; i++ ) {
      z[i] = a[i][3];
      for ( j = 0; j < 3; j++ ) {
        z[i] = z[i] + a[i][j] * x[k][j];
      }
    }

    for ( i = 0; i < 3; i++ ) {
      y[k][i] = z[i];
    }

  }
  return;
}
/*********************************************************************/

void tmat_mxv ( float a[4][4], float x[4], float y[4] )

/*********************************************************************/

/*
  Purpose:

    TMAT_MXV multiplies a geometric transformation matrix times a vector.

  Modified:

    12 August 1999

  Author:

    John Burkardt

  Reference:

    Foley, van Dam, Feiner, Hughes,
    Computer Graphics, Principles and Practice,
    Addison Wesley, Second Edition, 1990.

  Parameters:

    Input, float A[4][4], the geometric transformation matrix.

    Input, float X[4], the vector to be multiplied.  The fourth component
    of X is implicitly assigned the value of 1.

    Output, float Y[4], the result of A*X.  The product is accumulated in 
    a temporary vector, and assigned to the result.  Therefore, it 
    is legal for X and Y to share memory.
*/
{
  int i;
  int j;
  float z[4];

  for ( i = 0; i < 3; i++ ) {
    z[i] = 0.0;
    for ( j = 0; j < 3; j++ ) {
      z[i] = z[i] + a[i][j] * x[j];
    }
    z[i] = z[i] + a[i][3];
  }

  for ( i = 0; i < 3; i++ ) {
    y[i] = z[i];
  }
  return;
}
/*********************************************************************/

void tmat_rot_axis ( float a[4][4], float b[4][4], float angle, 
  char axis )

/*********************************************************************/

/*
  Purpose:

    TMAT_ROT_AXIS applies an axis rotation to the geometric transformation matrix.

  Modified:

    19 April 1999

  Author:

    John Burkardt

  Reference:

    Foley, van Dam, Feiner, Hughes,
    Computer Graphics, Principles and Practice,
    Addison Wesley, Second Edition, 1990.

  Parameters:

    Input, float A[4][4], the current geometric transformation matrix.

    Output, float B[4][4], the modified geometric transformation matrix.
    A and B may share the same memory.

    Input, float ANGLE, the angle, in degrees, of the rotation.

    Input, character AXIS, is 'X', 'Y' or 'Z', specifying the coordinate
    axis about which the rotation occurs.
*/
{
  float c[4][4];
  float d[4][4];
  int i;
  int j;
  float theta;

  theta = angle * DEG_TO_RAD;

  tmat_init ( c );

  if ( axis == 'X' || axis == 'x' ) {
    c[1][1] =   cos ( theta );
    c[1][2] = - sin ( theta );
    c[2][1] =   sin ( theta );
    c[2][2] =   cos ( theta );
  }
  else if ( axis == 'Y' || axis == 'y' ) {
    c[0][0] =   cos ( theta );
    c[0][2] =   sin ( theta );
    c[2][0] = - sin ( theta );
    c[2][2] =   cos ( theta );
  }
  else if ( axis == 'Z' || axis == 'z' ) {
    c[0][0] =   cos ( theta );
    c[0][1] = - sin ( theta );
    c[1][0] =   sin ( theta );
    c[1][1] =   cos ( theta );
  }
  else {
    printf ( "\n" );
    printf ( "TMAT_ROT_AXIS - Fatal error!\n" );
    printf ( "  Illegal rotation axis: %c.\n", axis );
    printf ( "  Legal choices are 'X', 'Y', or 'Z'.\n" );
    return;
  }

  tmat_mxm ( c, a, d );

  for ( i = 0; i < 4; i++ ) {
    for ( j = 0; j < 4; j++ ) {
      b[i][j] = d[i][j];
    }
  }
  return;
}
/*********************************************************************/

void tmat_rot_vector ( float a[4][4], float b[4][4], float angle, 
  float v1, float v2, float v3 )

/*********************************************************************/

/*
  Purpose:

    TMAT_ROT_VECTOR applies a rotation about a vector to the geometric transformation matrix.

  Modified:

    27 July 1999

  Author:

    John Burkardt

  Reference:

    Foley, van Dam, Feiner, Hughes,
    Computer Graphics, Principles and Practice,
    Addison Wesley, Second Edition, 1990.

  Parameters:

    Input, float A[4][4], the current geometric transformation matrix.

    Output, float B[4][4], the modified geometric transformation matrix.
    A and B may share the same memory.

    Input, float ANGLE, the angle, in degrees, of the rotation.

    Input, float V1, V2, V3, the X, Y and Z coordinates of a (nonzero)
    point defining a vector from the origin.  The rotation will occur
    about this axis.
*/
{
  float c[4][4];
  float ca;
  float d[4][4];
  int i;
  int j;
  float sa;
  float theta;

  if ( v1 * v1 + v2 * v2 + v3 * v3 == 0.0 ) {
    return;
  }

  theta = angle * DEG_TO_RAD;

  tmat_init ( c );

  ca = cos ( theta );
  sa = sin ( theta );

  c[0][0] =                v1 * v1 + ca * ( 1.0 - v1 * v1 );
  c[0][1] = ( 1.0 - ca ) * v1 * v2 - sa * v3;
  c[0][2] = ( 1.0 - ca ) * v1 * v3 + sa * v2;

  c[1][0] = ( 1.0 - ca ) * v2 * v1 + sa * v3;
  c[1][1] =                v2 * v2 + ca * ( 1.0 - v2 * v2 );
  c[1][2] = ( 1.0 - ca ) * v2 * v3 - sa * v1;

  c[2][0] = ( 1.0 - ca ) * v3 * v1 - sa * v2;
  c[2][1] = ( 1.0 - ca ) * v3 * v2 + sa * v1;
  c[2][2] =                v3 * v3 + ca * ( 1.0 - v3 * v3 );

  tmat_mxm ( c, a, d );

  for ( i = 0; i < 4; i++ ) {
    for ( j = 0; j < 4; j++ ) {
      b[i][j] = d[i][j];
    }
  }
  return;
}
/*********************************************************************/

void tmat_scale ( float a[4][4], float b[4][4], float sx, float sy, 
  float sz )

/*********************************************************************/

/*
  Purpose:

    TMAT_SCALE applies a scaling to the geometric transformation matrix.

  Modified:

    19 October 1998

  Author:

    John Burkardt

  Reference:

    Foley, van Dam, Feiner, Hughes,
    Computer Graphics, Principles and Practice,
    Addison Wesley, Second Edition, 1990.

  Parameters:

    Input, float A[4][4], the current geometric transformation matrix.

    Output, float B[4][4], the modified geometric transformation matrix.
    A and B may share the same memory.

    Input, float SX, SY, SZ, the scalings to be applied to the X, Y and
    Z coordinates.
*/
{
  float c[4][4];
  float d[4][4];
  int i;
  int j;

  tmat_init ( c );

  c[0][0] = sx;
  c[1][1] = sy;
  c[2][2] = sz;

  tmat_mxm ( c, a, d );

  for ( i = 0; i < 4; i++ ) {
    for ( j = 0; j < 4; j++ ) {
      b[i][j] = d[i][j];
    }
  }
  return;
}
/*********************************************************************/

void tmat_shear ( float a[4][4], float b[4][4], char *axis, float s )

/*********************************************************************/

/*
  Purpose:

    TMAT_SHEAR applies a shear to the geometric transformation matrix.

  Modified:

    19 October 1998

  Author:

    John Burkardt

  Reference:

    Foley, van Dam, Feiner, Hughes,
    Computer Graphics, Principles and Practice,
    Addison Wesley, Second Edition, 1990.

  Parameters:

    Input, float A[4][4], the current geometric transformation matrix.

    Output, float B[4][4], the modified geometric transformation matrix.
    A and B may share the same memory.

    Input, character*3 AXIS, is 'XY', 'XZ', 'YX', 'YZ', 'ZX' or 'ZY',
    specifying the shear equation:

      XY:  x' = x + s * y;
      XZ:  x' = x + s * z;
      YX:  y' = y + s * x;
      YZ:  y' = y + s * z;
      ZX:  z' = z + s * x;
      ZY:  z' = z + s * y.

    Input, float S, the shear coefficient.
*/
{
  float c[4][4];
  float d[4][4];
  int i;
  int j;

  tmat_init ( c );

  if ( strcmp ( axis, "XY" ) == 0 || strcmp ( axis, "xy" ) == 0 ) {
    c[0][1] = s;
  }
  else if ( strcmp ( axis, "XZ" ) == 0 || strcmp ( axis, "xz" ) == 0 ) {
    c[0][2] = s;
  }
  else if ( strcmp ( axis, "YX" ) == 0 || strcmp ( axis, "yx" ) == 0 ) {
    c[1][0] = s;
  }
  else if ( strcmp ( axis, "YZ" ) == 0 || strcmp ( axis, "yz" ) == 0 ) {
    c[1][2] = s;
  }
  else if ( strcmp ( axis, "ZX" ) == 0 || strcmp ( axis, "zx" ) == 0 ) {
    c[2][0] = s;
  }
  else if ( strcmp ( axis, "ZY" ) == 0 || strcmp ( axis, "zy" ) == 0 ) {
    c[2][1] = s;
  }
  else {
    printf ( "\n" );
    printf ( "TMAT_SHEAR - Fatal error!\n" );
    printf ( "  Illegal shear axis: %s.\n", axis );
    printf ( "  Legal choices are XY, XZ, YX, YZ, ZX, or ZY.\n" );
    return;
  }

  tmat_mxm ( c, a, d );

  for ( i = 0; i < 4; i++ ) {
    for ( j = 0; j < 4; j++ ) {
      b[i][j] = d[i][j];
    }
  }
  return;
}
/*********************************************************************/

void tmat_trans ( float a[4][4], float b[4][4], float x, float y, 
  float z )

/*********************************************************************/

/*
  Purpose:

    TMAT_TRANS applies a translation to the geometric transformation matrix.

  Modified:

    19 October 1998

  Author:

    John Burkardt

  Reference:

    Foley, van Dam, Feiner, Hughes,
    Computer Graphics, Principles and Practice,
    Addison Wesley, Second Edition, 1990.

  Parameters:

    Input, float A[4][4], the current geometric transformation matrix.

    Output, float B[4][4], the modified transformation matrix.
    A and B may share the same memory.

    Input, float X, Y, Z, the translation.  This may be thought of as the
    point that the origin moves to under the translation.
*/
{
  int i;
  int j;

  for ( i = 0; i < 4; i++ ) {
    for ( j = 0; j < 4; j++ ) {
      b[i][j] = a[i][j];
    }
  }
  b[0][3] = b[0][3] + x;
  b[1][3] = b[1][3] + y;
  b[2][3] = b[2][3] + z;

  return;
}
/**********************************************************************/

float torus_volume_3d ( float r1, float r2 )

/**********************************************************************/

/*
  Purpose:

    TORUS_VOLUME_3D computes the volume of a torus in 3D.

  Definition:

    A torus with radii R1 and R2 is the set of points (X,Y,Z) satisfying:

      ( sqrt ( X**2 + Y**2 ) - R1 )**2 + Z**2 <= R2**2

  Modified:

    22 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float R1, R2, the "inner" and "outer" radii of the torus.

    Output, float TORUS_VOLUME_3D, the volume of the torus.
*/
{
  float volume;

  volume = 2.0 * PI * PI * r1 * r2 * r2;

  return volume;
}
/**********************************************************************/

float triangle_area_2d ( float x1, float y1, float x2, float y2, 
  float x3, float y3 )

/**********************************************************************/

/*
  Purpose:

    TRIANGLE_AREA_2D computes the area of a triangle in 2D.

  Modified:

    22 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, X3, Y3, the X and Y coordinates
    of the corners of the triangle.

    Output, float TRIANGLE_AREA_2D, the area of the triangle.  AREA will 
    be nonnegative.
*/
{
  float area;

  area = fabs ( 0.5 * ( 
    x1 * ( y3 - y2 ) + 
    x2 * ( y1 - y3 ) + 
    x3 * ( y2 - y1 ) ) );
 
  return area;
}
/**********************************************************************/

float triangle_area_signed_2d ( float x1, float y1, float x2, float y2, 
  float x3, float y3 )

/**********************************************************************/

/*
  Purpose:

    TRIANGLE_AREA_SIGNED_2D computes the signed area of a triangle in 2D.

  Modified:

    22 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, X3, Y3, the X and Y coordinates
    of the corners of the triangle.

    Output, float TRIANGLE_AREA_SIGNED, the signed area of the triangle.  
    The size of this number is the usual area, but the sign can be 
    negative or positive, depending on the order in which the corners 
    of the triangle were listed.
*/
{
  float area;

  area = 0.5 * ( 
    x1 * ( y3 - y2 ) + 
    x2 * ( y1 - y3 ) + 
    x3 * ( y2 - y1 ) );
 
  return area;
}
/**********************************************************************/

float triangle_area_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float x3, float y3, float z3 )

/**********************************************************************/

/*
  Purpose:

    TRIANGLE_AREA_3D computes the area of a triangle in 3D.

  Modified:

    22 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, the (X,Y,Z) 
    coordinates of the corners of the triangle.

    Output, float TRIANGLE_AREA_3D, the area of the triangle.
*/
{
  float a;
  float alpha;
  float area;
  float b;
  float base;
  float c;
  float dot;
  float height;
/*
  Find the projection of (P3-P1) onto (P2-P1).
*/
  dot = 
    ( x2 - x1 ) * ( x3 - x1 ) +
    ( y2 - y1 ) * ( y3 - y1 ) +
    ( z2 - z1 ) * ( z3 - z1 );

  base = enorm0_3d ( x1, y1, z1, x2, y2, z2 );
/*
  The height of the triangle is the length of (P3-P1) after its
  projection onto (P2-P1) has been subtracted.
*/
  if ( base == 0.0 ) {

    height = 0.0;

  }
  else {

    alpha = dot / ( base * base );
      
    a = x3 - x1 - alpha * ( x2 - x1 );
    b = y3 - y1 - alpha * ( y2 - y1 );
    c = z3 - z1 - alpha * ( z2 - z1 );

    height = sqrt ( a * a + b * b + c * c );

  }

  area = 0.5 * base * height;
 
  return area;
}
/**********************************************************************/

void triangle_centroid_2d ( float x1, float y1, float x2, float y2, 
  float x3, float y3, float *x, float *y )

/**********************************************************************/

/*
  Purpose:

    TRIANGLE_CENTROID_2D computes the centroid of a triangle in 2D.

  Discussion:

    The centroid of a triangle can also be considered the center
    of gravity, assuming that the triangle is made of a thin uniform
    sheet of massy material.

  Reference:

    Adrian Bowyer and John Woodwark,
    A Programmer's Geometry,
    Butterworths, 1983.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, X3, Y3, the X and Y coordinates
    of the corners of the triangle.

    Output, float *X, *Y, the coordinates of the centroid of the triangle.
*/
{
  *x = ( x1 + x2 + x3 ) / 3.0;
  *y = ( y1 + y2 + y3 ) / 3.0;
 
  return;
}
/**********************************************************************/

void triangle_centroid_3d ( float x1, float y1, float z1, float x2, 
  float y2, float z2, float x3, float y3, float z3, float *x, 
  float *y, float *z )

/**********************************************************************/

/*
  Purpose:

    TRIANGLE_CENTROID_3D computes the centroid of a triangle in 3D.

  Discussion:

    The centroid of a triangle can also be considered the center
    of gravity, assuming that the triangle is made of a thin uniform
    sheet of massy material.

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, the coordinates
    of the corners of the triangle.

    Output, float *X, *Y, *Z, the coordinates of the centroid.
*/
{
  *x = ( x1 + x2 + x3 ) / 3.0;
  *y = ( y1 + y2 + y3 ) / 3.0;
  *z = ( z1 + z2 + z3 ) / 3.0;

  return;
}
/**********************************************************************/

void triangle_incircle_2d ( float x1, float y1, float x2, float y2, 
  float x3, float y3, float *xc, float *yc, float *r )

/**********************************************************************/

/*
  Purpose:

    TRIANGLE_INCIRCLE_2D computes the inscribed circle of a triangle in 2D.

  Discussion:

    The inscribed circle of a triangle is the largest circle that can
    be drawn inside the triangle.  It is tangent to all three sides,
    and the lines from its center to the vertices bisect the angles
    made by each vertex.

  Reference:

    Adrian Bowyer and John Woodwark,
    A Programmer's Geometry,
    Butterworths, 1983.

  Modified:

    21 May 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, X2, Y2, X3, Y3, the X and Y coordinates
    of the corners of the triangle.

    Output, float *XC, *YC, *R, the coordinates of the center of the
    inscribed circle, and its radius.
*/
{
  float perim;
  float s12;
  float s23;
  float s31;

  s12 = enorm0_2d ( x1, y1, x2, y2 );
  s23 = enorm0_2d ( x2, y2, x3, y3 );
  s31 = enorm0_2d ( x3, y3, x1, y1 );
  perim = s12 + s23 + s31;

  *xc = ( s23 * x1 + s31 * x2 + s12 * x3 ) / perim;
  *yc = ( s23 * y1 + s31 * y2 + s12 * y3 ) / perim;

  *r = 0.5 * sqrt (
      ( - s12 + s23 + s31 )
    * ( + s12 - s23 + s31 )
    * ( + s12 + s23 - s31 ) / perim );

  return;
}

/**********************************************************************/

float uniform_01_sample ( int *iseed )

/**********************************************************************/

/*
  Purpose:

    UNIFORM_01_SAMPLE is a random number generator.

  Formula:

    ISEED = ISEED * (7**5) mod (2**31 - 1)
    UNIFORM_01_SAMPLE = ISEED * / ( 2**31 - 1 )

  Parameters:

    Input/output, int *ISEED, the integer "seed" used to generate
    the output random number, and updated in preparation for the
    next one.  *ISEED should not be zero.

    Output, float UNIFORM_01_SAMPLE, a random value between 0 and 1.
*/
{
/*
  IA = 7**5
  IB = 2**15
  IB16 = 2**16
  IP = 2**31 - 1
*/
  static int  ia = 16807;
  static int  ib15 = 32768;
  static int  ib16 = 65536;
  static int  ip = 2147483647;
  int         iprhi;
  int         ixhi;
  int         k;
  int         leftlo;
  int         loxa;
  float       temp;
/*
  Don't let ISEED be 0.
*/
  if ( *iseed == 0 ) {
    *iseed = ip;
  }
/*
  Get the 15 high order bits of ISEED.
*/
  ixhi = *iseed / ib16;
/*
  Get the 16 low bits of ISEED and form the low product.
*/
  loxa = ( *iseed - ixhi * ib16 ) * ia;
/*
  Get the 15 high order bits of the low product.
*/
  leftlo = loxa / ib16;
/*
  Form the 31 highest bits of the full product.
*/
  iprhi = ixhi * ia + leftlo;
/*
  Get overflow past the 31st bit of full product.
*/
  k = iprhi / ib15;
/*
  Assemble all the parts and presubtract IP.  The parentheses are essential.
*/
  *iseed = ( ( ( loxa - leftlo * ib16 ) - ip ) +
    ( iprhi - k * ib15 ) * ib16 ) + k;
/*
  Add IP back in if necessary.
*/
  if ( *iseed < 0 ) {
    *iseed = *iseed + ip;
  }
/*
  Multiply by 1 / (2**31-1).
*/
  temp = ( ( float ) *iseed ) * 4.656612875e-10;

  return ( temp );
}
/**********************************************************************/

void vector_rotate_2d ( float x1, float y1, float angle, float *x2, 
  float *y2 )

/**********************************************************************/

/*
  Purpose:

    VECTOR_ROTATE_2D rotates a vector around the origin in 2D.

  Discussion:

    To see why this formula is so, consider that the original point
    has the form ( R cos Theta, R sin Theta ), and the rotated point
    has the form ( R cos ( Theta + Angle ), R sin ( Theta + Angle ) ).
    Now use the addition formulas for cosine and sine to relate
    the new point to the old one:

      ( X2 ) = ( cos Angle  - sin Angle ) * ( X1 )
      ( Y2 )   ( sin Angle    cos Angle )   ( Y1 )

  Modified:

    21 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, the components of the vector to be rotated.

    Input, float ANGLE, the angle, in radians, of the rotation to be
    carried out.  A positive angle rotates the vector in the
    counterclockwise direction.

    Output, float *X2, *Y2, the rotated vector.
*/
{
  *x2 = cos ( angle ) * x1 - sin ( angle ) * y1;
  *y2 = sin ( angle ) * x1 + cos ( angle ) * y1;

  return;
}
/*********************************************************************/

void vector_rotate_3d ( float x1, float y1, float z1, float *x2, float *y2, 
  float *z2, float xa, float ya, float za, float angle )

/**********************************************************************/

/*
  VECTOR_ROTATE_3D rotates a vector around an axis vector in 3D.

  Modified:

    17 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, float X1, Y1, Z1, the components of the vector to be rotated.

    Output, float *X2, *Y2, *Z2, the rotated vector.

    Input, float XA, YA, ZA, the vector about which the rotation is to
    be carried out.

    Input, float ANGLE, the angle, in radians, of the rotation to be
    carried out.
*/
{
  float dot;
  float norma;
  float normn;
  float xn;
  float xn2;
  float xp;
  float xr;
  float yn;
  float yn2;
  float yp;
  float yr;
  float zn;
  float zn2;
  float zp;
  float zr;
/*
  Compute the length of the rotation axis.
*/
  norma = sqrt ( xa * xa + ya * ya + za * za );

  if ( norma == 0.0 ) {
    *x2 = x1;
    *y2 = y1;
    *z2 = z1;
    return;
  }
/*
  Compute the dot product of the vector and the rotation axis.
*/
  dot = x1 * xa + y1 * ya + z1 * za;
/*
  Compute the parallel component of the vector.
*/
  xp = dot * xa / norma;
  yp = dot * ya / norma;
  zp = dot * za / norma;
/*
  Compute the normal component of the vector.
*/
  xn = x1 - xp;
  yn = y1 - yp;
  zn = z1 - zp;

  normn = sqrt ( xn * xn + yn * yn + zn * zn );

  vector_unit_3d ( &xn, &yn, &zn );
/*
  Compute a second vector, lying in the plane, perpendicular
  to (X1,Y1,Z1), and forming a right-handed system...
*/
  cross_3d ( xa, ya, za, xn, yn, zn, &xn2, &yn2, &zn2 );

  vector_unit_3d ( &xn2, &yn2, &zn2 );
/*
  Rotate the normal component by the angle.
*/
  xr = normn * ( cos ( angle ) * xn + sin ( angle ) * xn2 );
  yr = normn * ( cos ( angle ) * yn + sin ( angle ) * yn2 );
  zr = normn * ( cos ( angle ) * zn + sin ( angle ) * zn2 );
/*
  The rotated vector is the parallel component plus the rotated
  component.
*/
  *x2 = xp + xr;
  *y2 = yp + yr;
  *z2 = zp + zr;

  return;
}
/**********************************************************************/

void vector_unit_2d ( float *x1, float *y1 )

/**********************************************************************/

/*
  VECTOR_UNIT_2D normalizes a vector in 2D.

  Modified:

    20 April 1999

  Author:

    John Burkardt

  Parameters:

    Input/output, float *X1, *Y1, the components of a vector to be
    normalized.  On output, the vector should have unit Euclidean norm.
    However, if the input vector has zero Euclidean norm, it is
    not altered.
*/
{
  float temp;

  temp = sqrt ( (*x1) * (*x1) + (*y1) * (*y1) );

  if ( temp != 0.0 ) {
    *x1 = *x1 / temp;
    *y1 = *y1 / temp;
  }

  return;
}
/**********************************************************************/

void vector_unit_3d ( float *x1, float *y1, float *z1 )

/**********************************************************************/

/*
  VECTOR_UNIT_3D normalizes a vector in 3D.

  Modified:

    17 April 1999

  Author:

    John Burkardt

  Parameters:

    Input/output, float *X1, *Y1, *Z1, the components of a vector to be
    normalized.  On output, the vector should have unit Euclidean norm.
    However, if the input vector has zero Euclidean norm, it is
    not altered.
*/
{
  float temp;

  temp = sqrt ( (*x1) * (*x1) + (*y1) * (*y1) + (*z1) * (*z1) );

  if ( temp != 0.0 ) {
    *x1 = *x1 / temp;
    *y1 = *y1 / temp;
    *z1 = *z1 / temp;
  }

  return;
}
/**********************************************************************/

void vector_unit_nd ( int n, float x[] )

/**********************************************************************/

/*
  VECTOR_UNIT_ND normalizes a vector in ND.

  Modified:

    20 April 1999

  Author:

    John Burkardt

  Parameters:

    Input, int N, the dimension of the vector.

    Input/output, float X[N}, the vector to be normalized.  On output, 
    the vector should have unit Euclidean norm.
    However, if the input vector has zero Euclidean norm, it is
    not altered.
*/
{
  int i;
  float temp;

  temp = enorm_nd ( n, x );

  if ( temp != 0.0 ) {
    for ( i = 0; i < n; i++ ) {
      x[i] = x[i] / temp;
    }
  }

  return;
}


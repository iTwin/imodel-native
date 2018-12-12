/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/libgeom.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*====================================================================
**
**  geometry library header file
**
**  This file contains definitions required by and to use the libgeom.a
**  cartographic geometry package.
**
**  This file uses the definition contained in the geometry.h header file
**  but does not use any of the geometry.a library functions.
**
**  This library header contains definition for the following :
**
**      DCOORD_2D
**      DCOORD_3D
**      DCOORD
**      LCOORD_2D
**      LCOORD_3D
**      XCOORD_2D
**      XCOORD_3D
**      DMAT_2D
**      DMAT_3D
**      DMAT
**      DLINE_EQUATION
**      DEXTENT_2D
**      DRECTANGLE_2D
**      DRECTANGLE_3D
**      LRECTANGLE_2D
**      LRECTANGLE_3D
**
**  cylindric 3d arcs
**        CYLARC
**  cylindric 3d circles
**        CYLCIR
**  3D segments
**        SEGMENT
**  3D triangle
**        TRIANGLE
**  3D boxes
**        BOX
**  3D quadrilater
**        QUADRILATER
**  3D lines
**        CARTO_LINE
**  3D plane
**        PLANE
**  3D dradient vector
**        GRADIENT
**
**  The file contains also the prototypes for 2D and 2 1/2 D functions
**  related the presented data types. It also contains the 3D point
**  functions required based on 3D points defined in geometry.h
**
**  HMR FUNCTIONS
**         angle
**         simple_angle
**  END HMR FUNCTIONS
*
**====================================================================*/
#ifndef __LIBGEOM_H__
#define __LIBGEOM_H__

#include "oldhtypes.h"

/*====================================================================
**
**  Constant definitions
**
**  PI : ratio of circle circumference to diameter
**
**  GEOM_COUNTER_CLOCKWISE : to indicate counter clockwise rotation
**
**  GEOM_CLOCKWISE : to indicate clockwise rotation
**
**  ONE_DEGREE : value in radians of one degree
**
**====================================================================*/

#ifndef PI
#define PI                      3.141592653589793
#endif

#define GEOM_COUNTER_CLOCKWISE       0
#define GEOM_CLOCKWISE               1

#define ONE_DEGREE                   (PI / 180)

/*====================================================================
**
**  Constant definitions used for text box processing
**
**  JU_MIN : minimum value for text justification
**
**  JU_MAX : maximum value for text box justification
**
**  JU_BOTTOM_LEFT : bottom left justification
**
**  JU_HALF_LEFT : half height to the left justification
**
**  JU_TOP_LEFT : top left justification
**
**  JU_BOTTOM_CENTER : bottom center of text justification
**
**  JU_HALF_CENTER : half height center of text justification
**
**  JU_TOP_CENTER : top and center of text justification
**
**  JU_BOTTOM_RIGHT : bottom right justification
**
**  JU_HALF_RIGHT : half height right justification
**
**  JU_TOP_RIGHT : top right justification
**
**  JU_UNDERLINE_RIGHT_MARGIN : 1/2 char height under the bottom
**                          and 1/2 char width to the right of text
**
**  JU_HALF_RIGHT_MARGIN : at half text, 1/2 char width to the right
**                         of text
**
**  JU_OVERLINE_RIGHT_MARGIN : 1/2 char height over the top
**                          and 1/2 char width to the right of text
**
**  JU_UNDERLINE_CENTER  : at center of text, 1/2 char height under bottom
**
**  JU_HALF_CENTER_TOO : identical to JU_HALF_CENTER in application but
**                      with a different value
**
**  JU_OVERLINE_CENTER : at center, 1/2 char height over the top
**
**  JU_UNDERLINE_LEFT_MARGIN : 1/2 char height under bottom, and
**                             1/2 char width to the left of text.
**
**  JU_HALF_LEFT_MARGIN : at half, 1/2 char width to the left of text
**
**  JU_OVERLINE_LEFT_MARGIN : 1/2 char height over the top and 1/2 char
**                            width to the left of text.
**
**  JU_NUMBER_BY_COLUMN : number of different justification settings
**                        by column
**
**  JU_NUMBER_BY_LINE   : number of different justification settings
**                        by line
**
**  JU_USE_DEFAULT      : indicator to use default values
**
**
**====================================================================*/
#define JU_MIN              1
#define JU_MAX              19
#define JU_MAX_FIRST_SET    9
#define JU_MIN_FIRST_SET    1
#define JU_MAX_SECOND_SET   19
#define JU_MIN_SECOND_SET   11

#define JU_BOTTOM_LEFT             1
#define JU_HALF_LEFT               2
#define JU_TOP_LEFT                3
#define JU_BOTTOM_CENTER           4
#define JU_HALF_CENTER             5
#define JU_TOP_CENTER              6
#define JU_BOTTOM_RIGHT            7
#define JU_HALF_RIGHT              8
#define JU_TOP_RIGHT               9
#define JU_UNDERLINE_RIGHT_MARGIN  11
#define JU_HALF_RIGHT_MARGIN       12
#define JU_OVERLINE_RIGHT_MARGIN   13
#define JU_UNDERLINE_CENTER        14
#define JU_HALF_CENTER_TOO         15
#define JU_OVERLINE_CENTER         16
#define JU_UNDERLINE_LEFT_MARGIN   17
#define JU_HALF_LEFT_MARGIN        18
#define JU_OVERLINE_LEFT_MARGIN    19

#define JU_NUMBER_BY_COLUMN 3
#define JU_NUMBER_BY_LINE   3

#define JU_USE_DEFAULT      0

/*====================================================================
**
**  Macros :
**
**  in geometry.h are defined the macros degree_to_radian()
**  and radian_to_degree()
**
**  This module further defines :
**
**    gradian_to_degree()
**    degree_to_gradian()
**
**
**    min() : of two arguments returns the smallest
**    max() : of two arguments returns the biggest
**
**    degree_to_radian(a)
**    radian_to_degree(r)
**
**====================================================================*/

#ifndef degree_to_gradian
#   define degree_to_gradian(a)   (((a) * 10) / 9)
#endif

#ifndef gradian_to_degree
#   define gradian_to_degree(a)   ((a) * 0.9)
#endif


#ifndef min
#   define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#   define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif

#ifndef degree_to_radian
#   define degree_to_radian(a)     (((a) / 180.0) * PI)
#endif

#ifndef radian_to_degree
#   define radian_to_degree(r)     (((r) * 180.0) / PI)
#endif

#ifndef iround
#   define iround(a)    ((int) ((a) >= 0 ? ((a) + 0.5) : ((a) - 0.5)))
#endif

#ifndef lround
#   define lround(a)    ((int32_t) ((a) >= 0L ? ((a) + 0.5) : ((a) - 0.5)))
#endif

/*====================================================================
**
**  DCOORD_2D
**
**  This structure record strictly bi-dimensional coordinates in x and y
**  rectangular axis data.
**
**====================================================================*/
typedef struct tagDCOORD_2D
{
    double x;
    double y;
} DCOORD_2D;

typedef DCOORD_2D * pDCOORD_2D;

/*====================================================================
**
**  DCOORD_3D
**
**  This structure records information concerning 3D rectangular
**  coordinates
**
**====================================================================*/
#if defined (__HMR_MDL)
typedef DPoint3d  DCOORD_3D;
#else
typedef struct tagDCOORD_3D
{
    double x;
    double y;
    double z;
} DCOORD_3D;
#endif
typedef DCOORD_3D * pDCOORD_3D;

/*====================================================================
**
**  DCOORD
**
**  This structure records information concerning coordinates in 3D
**  rectangular.
**
**====================================================================*/

typedef DCOORD_3D DCOORD;

typedef DCOORD * pDCOORD;


/*====================================================================
**
**  LCOORD_2D
**
**  This structure record strictly bi-dimensional coordinates in x and y
**  rectangular axis data.
**
**====================================================================*/
typedef struct tagLCOORD_2D
{
    int32_t x;
    int32_t y;
} LCOORD_2D;

typedef LCOORD_2D * pLCOORD_2D;

/*====================================================================
**
**  LCOORD_3D
**
**  This structure records information concerning 3D rectangular
**  coordinates
**
**====================================================================*/
typedef struct tagLCOORD_3D
{
    int32_t x;
    int32_t y;
    int32_t z;
} LCOORD_3D;

typedef LCOORD_3D * pLCOORD_3D;


/*====================================================================
**
**  XCOORD_2D
**
**  This structure record strictly bi-dimensional coordinates in x and y
**  rectangular axis data. (double and Int32)
**
**====================================================================*/
typedef union tagXCOORD_2D
{
    DCOORD_2D dCoord;
    LCOORD_2D lCoord;

} XCOORD_2D;

typedef XCOORD_2D * pXCOORD_2D;

/*====================================================================
**
**  XCOORD_3D
**
**  This structure records information concerning 3D rectangular
**  coordinates (double and Int32)
**
**====================================================================*/
typedef union tagXCOORD_3D
{
    DCOORD_3D dCoord;
    LCOORD_3D lCoord;

} XCOORD_3D;

typedef XCOORD_3D * pXCOORD_3D;


/*====================================================================
**
**  DMAT_2D
**
**  This information records information concerning 2D pre-expanded
**  orientation matrix.
**
**====================================================================*/

typedef double DMAT_2D[2][2];

typedef DMAT_2D * pDMAT_2D;

/*====================================================================
**
**  DMAT_3D
**
**  This structure records information concerning 3D pre-expanded
**  orientation matrix.
**
**====================================================================*/
typedef double DMAT_3D[3][3];

typedef DMAT_3D * pDMAT_3D;

/*====================================================================
**
**  DMAT
**
**  Equivalent to DMAT_3D
**
**====================================================================*/
typedef DMAT_3D DMAT;

typedef DMAT * pDMAT;

/*====================================================================
**
**  DLINE_EQUATION
**
**  y = Mx + B
**
**  where:
**
**      M is called the slope
**      B is called the intercept
**
**      NOTE: If line is vertical, the slope field is unused and the
**      vertical flag is TRUE.
**
**====================================================================*/
typedef struct tagDLINE_EQUATION
{
    int8_t  vertical;             /* True only if vertical line      */
    double  slope;                /* Slope used if not vertical line */
    double  intercept;            /* Point where line crosses Y axis */
                                  /* or X value if infinite_slope    */
}DLINE_EQUATION;

typedef DLINE_EQUATION * pDLINE_EQUATION;


/*====================================================================
**
**  BINARY_COORD
**
**  This structure records any data requiring pairs of coordinates
**  accessible by indexing. It is a local typedef used to declared
**  the following exportable structured types.
**
**   pt - DCOORD [2] - pair of coordinates (3D).
**
**====================================================================*/

typedef struct tagBINARY_COORD {
  DCOORD pt[2];
}BINARY_COORD;



/*====================================================================
**
**  TRINARY_COORD
**
**  This structure records any data requiring a triplet of coordinates
**  accessible by indexing. It is a local typedef used to declared
**  the following exportable structured types.
**
**   pt - DCOORD [3] - triplet of coordinates (3D).
**
**====================================================================*/

typedef struct tagTRINARY_COORD {
  DCOORD pt[3];
}TRINARY_COORD;




/*====================================================================
**
**  CYLARC
**
**  This structure records the information specific to a cylindric type
**  arc. This is an arc in 3 dimension of which the projection on the
**  x-y plane still looks like an arc, not like a portion of ellipse.
**  The arc can still be inclinated in 3D, but this corresponds only to
**  certain types of 3D ellipses. The name of cylindric arc comes from
**  the fact that those arcs appear to be generated from the 3D crossing
**  of a vertical cylinder with a plane.
**
**  begin_pt         - 3D coordinate representing the beginning of the arc
**                     if generated in a clockwise fashion as seen from
**                     above.
**
**  center           - 3D coordinate, of the center of the cylinder
**                     in x-y with the specific elevation at which this
**                     center intersected with plane.
**
**  end_pt           - 3D coordinate of the end of the arc if generated
**                     in a clockwise fashion as seen from above.
**
**      Note that the three coordinates describe completely the
**      cylinder and the plane from which could have originated the
**      arc. And also the cylindric circle of which it is a portion
**      of
**
**====================================================================*/

typedef struct tagCYLARC {
  DCOORD begin_pt;
  DCOORD center;
  DCOORD end_pt;
}CYLARC;

typedef CYLARC * pCYLARC;


/*====================================================================
**
**  CYLCIR
**
**  This structure records the information specific to a cylindric type
**  circle. This is a circle in 3 dimension of which the projection on the
**  x-y plane still looks like a circle, not like an ellipse.
**  The circle can still be inclinated in 3D, but this corresponds only to
**  certain types of 3D ellipses. The name of cylindric circle comes from
**  the fact that those circles appear to be generated from the 3D crossing
**  of a vertical cylinder with a plane.
**
**
**  radius - double : radius of circle
**
**  center - DCOORD : center of circle
**
**  grad_pt - DCOORD : destination of the gradient vector origination
**            from the center point.
**
**====================================================================*/

typedef struct tagCYLCIR {
  DCOORD center;
  double radius;
  DCOORD grad_pt;
}CYLCIR;

typedef CYLCIR * pCYLCIR;


/*====================================================================
**
**  LINE
**
**  This structure records the information specific to a 3 D line. This
**  line is fully described by two 3D points. The structure of LINES
**  is identical to the structure SEGMENT defined below. The difference
**  lies in the target process for each one.
**
**  The points being identical one to another, there are kept in an array
**  inside the structure. The points can then be refered to by an index,
**  either constant or variable.
**
**  pt - DCOORD [] : list of two points defining the 3D line.
**
**====================================================================*/


typedef BINARY_COORD LINE;

typedef LINE * pLINE;

/*====================================================================
**
**  SEGMENT
**
**  This structure records the inmformation specific to a 3D segment.
**  This segment is fully described by two 3D points. The structure is
**  identical to the LINE structure defined above. The end points of the
**  segment can or cannot be part of the segment (inclusive or exclusive),
**  depending on the implementation of the functions.
**
**  The points being identical one to another, they are kept in an array
**  inside the structure. The points can then be refered to by an index,
**  either constant or variable.
**
**  pt - DCOORD [] : list of points defining the 3D segment.
**
**====================================================================*/


typedef BINARY_COORD SEGMENT;


typedef SEGMENT * pSEGMENT;

/*====================================================================
**
**  BOX
**
**  This structure records the information specific to a 2D box. This box
**  is defined as a 2 dimensionnal rectangle of which two sides are
**  parallel to the x axis , and the other two to the y axis. A box is fully
**  described by two 2D coordinates, which are kept in 4 different fields
**
**  xmin - double : x coordinate of lower box
**
**  xmax - double : x coordinate of upper box
**
**  ymin - double : y coordinate of left boundary of box
**
**  ymax - double : y coordinate of right boundary of box
**
**====================================================================*/

typedef struct tagBOX {
  double xmin;
  double xmax;
  double ymin;
  double ymax;
}BOX;

typedef BOX * pBOX;


/*====================================================================
**
**  DEXTENT_2D
**
**  This structure records an extent rectangle as a 2D box which coordinates
**  are double values.
**
**  PUBLIC:
**
**    --none--
**
**  PRIVATE:
**
**    empty   -  A booelan flag that indicates whether the extent is empty
**               or not
**
**    xmin    -  The X coordinate of the lower left corner of the extent
**
**    ymin    -  The Y coordinate of the lower left corner of the extent
**
**    xmax    -  The X coordinate of the upper right corner of the extent
**
**    ymax    -  The Y coordinate of the upper right corner of the extent
**
**====================================================================*/
typedef struct tagDEXTENT_2D
{
    int8_t  empty;
    double  xmin;
    double  xmax;
    double  ymin;
    double  ymax;
} DEXTENT_2D;

typedef DEXTENT_2D * pDEXTENT_2D;


/*====================================================================
**
**  TRIANGLE
**
**  This structure records the information specific to a 3D triangle. This
**  triangle is composed of three 3D points. The triangle can be viewed
**  either as a fully 3D vector triangle, or neglecting the elevation of
**  the points, as a 2D triangle, depending on implementation of the functions.
**
**  All 3 points being equivalent one to another, they are kept in an array
**  inside the structure. The points can then be refered to by an index
**  either constant or variable.
**
**  pt - DCOORD [3] : list of points constituting the corners of triangle
**
**====================================================================*/

typedef TRINARY_COORD TRIANGLE;


typedef TRIANGLE * pTRIANGLE;

/*====================================================================
**
**  QUADRILATER
**
**  This structure records information specific to a 3D quadrilater. The
**  3D vector type quadrilater is defined by use of 4 3D points. The
**  quadrilater can be viewed either as a 3D vector quadrilater, or
**  neglecting the elevation of the points, as a 2D quadrilater, depending
**  on the implementation of the functions using it.
**
**  All four points being identical to each other, they are kept in an array
**  inside the structure. The points can then be refered to by an index.
**
**  pt - DCOORD [4] : list of points constituting the quadrilater.
**
**====================================================================*/

typedef struct tagQUADRILATER {
  DCOORD pt[4];
}QUADRILATER;

typedef QUADRILATER * pQUADRILATER;


/*====================================================================
**
**  PLANE
**
**  This structure records information specific to a 3D plane.
**  A plane is fully defined by 3 3D points.
**
**  All 3 points being identical to each other, they are kept in an array
**  inside the structure. The points can then be refered to by index.
**
**  pt - DCOORD [3] : list of points canstituting the plane.
**
**====================================================================*/


typedef TRINARY_COORD PLANE;

typedef PLANE * pPLANE;

/*====================================================================
**
**  GRADIENT
**
**  This structure records information specific to a gradient vector.
**  This vector represents the inclinaison of a 3D surface. In the case
**  a plane, the gradient vector represents the inclinaison at every point,
**  but for any other surface, the gradient is defined ponctual, at
**  the first point defining it.
**
**  The gradient vector is fully defined by two points; the first being the
**  location of the vector, and the second point, representing the direction.
**  The vector size is defined by the difference in elevation of second to
**  first point.
**
**  pt - DCOORD [2] : where [0] is origin of vector, and [1] is destination
**
**
**
**    grad.pt[0] = (x0,y0,z0)
**
**    grad.pt[1] = (x0 + A, y0 + B, z0 + C)
**
**              where A is the partial derivate of surface at [0] in x
**              B in y and C in z.
**
**    if surface is vertical at studied point, the [1] = [0];
**
**
**====================================================================*/

typedef BINARY_COORD GRADIENT;

typedef GRADIENT * pGRADIENT;




/*====================================================================
**
**  DRECTANGLE_2D
**
**  This structure records an extent rectangle as a 2D box which coordinates
**  are double values.
**
**  PUBLIC:
**
**    --none--
**
**  PRIVATE:
**
**    origin  -  The coordinate of the lower left corner.
**    corner  -  The coordinate of the upper right corner.
**
**====================================================================*/
typedef struct tagDRECTANGLE_2D
{
    DCOORD_2D origin;
    DCOORD_2D corner;
} DRECTANGLE_2D;

typedef DRECTANGLE_2D *pDRECTANGLE_2D;


/*====================================================================
**
**  DRECTANGLE_3D
**
**  This structure records an extent rectangle as a 3D box which coordinates
**  are double values.
**
**  PUBLIC:
**
**    --none--
**
**  PRIVATE:
**
**    origin  -  The coordinate of the lower left corner.
**    corner  -  The coordinate of the upper right corner.
**
**====================================================================*/
typedef struct tagDRECTANGLE_3D
{
    DCOORD_3D origin;
    DCOORD_3D corner;
} DRECTANGLE_3D;

typedef DRECTANGLE_3D *pDRECTANGLE_3D;



/*====================================================================
**
**  LRECTANGLE_2D
**
**  This structure records an extent rectangle as a 2D box which coordinates
**  are Int32 values.
**
**  PUBLIC:
**
**    --none--
**
**  PRIVATE:
**
**    origin  -  The coordinate of the lower left corner.
**    corner  -  The coordinate of the upper right corner.
**
**====================================================================*/
typedef struct tagLRECTANGLE_2D
{
    LCOORD_2D origin;
    LCOORD_2D corner;
} LRECTANGLE_2D;

typedef LRECTANGLE_2D *pLRECTANGLE_2D;


/*====================================================================
**
**  LRECTANGLE_3D
**
**  This structure records an extent rectangle as a 3D box which coordinates
**  are Int32 values.
**
**  PUBLIC:
**
**    --none--
**
**  PRIVATE:
**
**    origin  -  The coordinate of the lower left corner.
**    corner  -  The coordinate of the upper right corner.
**
**====================================================================*/
typedef struct tagLRECTANGLE_3D
{
    LCOORD_3D origin;
    LCOORD_3D corner;
} LRECTANGLE_3D;

typedef LRECTANGLE_3D *pLRECTANGLE_3D;

/*====================================================================
**
**  List of functions of library
**
**====================================================================*/
#if defined (__cplusplus)
extern "C" {
#endif

double angle                     (const DCOORD    *pt1,
                                              const DCOORD    *pt2);

double simple_angle            (double        angl);

#if defined (__cplusplus)
}
#endif

#endif /* __LIBGEOM_H__ */





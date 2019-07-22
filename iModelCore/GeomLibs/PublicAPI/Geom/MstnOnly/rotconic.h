/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

/*----------------------------------------------------------------------+
|                                                                       |
|                                                                       |
| Standard Conic Surfaces                                               |
| Cylinder - (x,y,z) = (cos(theta), sin(theta), alpha)                  |
|       in a given coordinate system.                                   |
| Sphere - (x,y,z) = (cos(theta) cos(phi), sin(theta) cos(phi), sin(phi))|
|       in given coordinate system.                                     |
|                                                                       |
| General rotation formulation                                          |
|                                                                       |
| Let F be an n-component 'parameterization' vector for a curve. For    |
| a line, F is two components [(1-phi) phi], where phi is a parameter   |
| along the line.   For an ellipse, F is three components               |
| [cos(phi) sin(phi) 1] where phi is an angular parameter moving around |
| the ellipse.                                                          |
| Let B be a 4xn matrix that carries F into a homogeneous space, i.e    |
| defines points Y = B * F.                                             |
| Let K be a 4x4 matrix representing rotation in the xy plane by angle  |
| theta, i.e.                                                           |
|           K = [ c s 0 0]                                              |
|               [-s c 0 0]                                              |
|               [ 0 0 1 0]                                              |
|               [ 0 0 0 1]                                              |
| where c=cos(theta), s=sin(theta)                                      |
| Let C be a 4x4 homogeneous matrix.                                    |
|                                                                       |
| Define a curve X(theta,phi) = C * K * B * F.                          |
| That is, Y is a line or ellipse in the local coordinate frame defined |
| by C.  K rotates Y around the zw axes of the C coordinate system.     |
| C places the curve into output coordinates.                           |
|                                                                       |
| For implementation, use a DEllipse4d for B*F.   In ellipse case, this  |
| is a direct representation.  For line case, user the vector0 and      |
| vector90 entries as the support vectors in the linear interpolation.  |
|                                                                       |
| A point on the rotated surface is unambiguously represented by the    |
| parameter pair [theta,phi].  Numerically, it is preferable to avoid   |
| working with angles; it is better to carry theta as the pair of       |
| trig values (cos(theta),sin(theta)).                                  |
|                                                                       |
| A "theta formated" point is a DPoint3d with                           |
|   x=cos(theta), y=sin(theta), z=1.                                    |
| A "phi formated point" is a point 'ready to be applied to the B       |
| matrix embedded in the DEllispe4d, with the x,y,z components multiplying|
| vector0, vector90, and center.  Hence for a swung ellipse, the        |
| entries are                                                           |
|   x=cos(phi), y=sin(phi), z=1.                                        |
| while for a swung line they are                                       |
|   x=1-phi, y=phi, z=0                                                 |
+----------------------------------------------------------------------*/
#if defined(__omdl) || !defined(mdl)
typedef enum
    {
    RC_NullSurface,
    RC_RotatedLine,             /* Line defined as conic.center + alpha * conic.vector0 */
    RC_RotatedEllipse,          /* Ellipse defined by conic.center + cos(phi) * conic.vector0 + sin(phi) * conic.vector90 */
    RC_Cylinder,                /* Standard Form Cylinder  x^2 + y^2 = 1 */
    RC_Cone,                    /* Standard Form Cone      x^2^ + y^2 = z^2 */
    RC_Sphere,                  /* Standard Form Sphere */
    RC_Disk,                    /* Polar disk on plane */
    RC_Plane,                   /* Rectangular patch of plane */
    RC_Line,                    /* Z axis */
    RC_Point,                   /* single point */
    RC_Torus
    } RotatedConicType;

/*
    This is the list of all possible canonical diagonalized quadric surfaces.
    A canonical diagonalized surface is described by the sign of its four diagonal entries.
    This is enumerated by listing 4 characters, each of which can be PLUS, ZERO, or MINUS.
    PLUSes must precede ZEROS, which must precede MINUSes, and there must be at least as many
    PLUS as MINUS.
*/
typedef enum
    {
    QSC_NullSurface,     /* 0000     No solutions */
    QSC_AllPoints,       /* ++++     Every point is a solution */
    QSC_OnePlane,        /* +000     Just the plane x=0 */
    QSC_OneLine,         /* ++00     Just the positive z axis*/
    QSC_OnePoint,        /* +++0     Isolated point at origin */
    QSC_TwoPlanes,       /* +00-     Pair of planes, x=+-1    */
    QSC_Cylinder,        /* ++0-     Cylinder along z axis */
    QSC_Sphere,          /* +++-     Unit sphere */
    QSC_Hyperbolic       /* ++--     Hyperbolic Parabolid */
    } QuadricSurfaceClass;
#else

typedef int RotatedConicType;

#define    RC_NullSurface       0
#define    RC_RotatedLine       1
#define    RC_RotatedEllipse    2
#define    RC_Cylinder          3
#define    RC_Cone              4
#define    RC_Sphere            5
#define    RC_Disk              6
#define    RC_Plane             7
#define    RC_Line              8
#define    RC_Point             9
#define    RC_Torus            10

typedef int QuadricSurfaceClass;
typedef int RotatedConicType;
#endif

typedef struct   _range1d
    {
    double     minValue;
    double     maxValue;
    } Range1d;

struct _smallsetrange1d
    {
    Range1d     interval[MSGEOM_SMALL_SET_SIZE];
    int         n;
    };

struct _dEllipse4d
    {
    DPoint4d            center;
    DPoint4d            vector0;
    DPoint4d            vector90;
    SmallSetRange1d     sectors;
    };

typedef struct rotatedConic
    {
    HMap                rotationMap;
    DEllipse4d          conic;
    RotatedConicType    type;
    double              sweep;
    double              tolerance;
    DRange3d            parameterRange;     /* Param range in standard coordinates */
    double              hoopRadius;         /* Minor radius of torus, as fraction of major radius = 1 */
    } RotatedConic;

/* Bit masks for curve flags */
#define RC_PeriodicFunctionMask   (0x00000001)
#define RC_FullParamterRangeMask  (0x00000002)
#define RC_CurvedMask             (0x00000004)

#define RC_EvaluateBoundaryMask     (0x0001000)
#define RC_IntersectLineMask        (0x0002000)
#define RC_IntersectPlaneMask       (0x0004000)
#define RC_SilhouetteMask           (0x0008000)


#define STATUS_ROTATEDCONIC_NONCONIC_SILHOUETTE -1001

#define RC_COORDSYS_world 0
#define RC_COORDSYS_local 1
#define RC_COORDSYS_parameter 2
#define RC_COORDSYS_parameter01 3

#define RC_CURVEMASK_SMOOTH 0x00000001
#define RC_CURVEMASK_CLOSED 0x00000002

typedef struct HConic
    {
    DEllipse4d        coordinates;
    int             type;
    } HConic;

typedef StatusInt (*SilhouetteArrayHandler)

(
HConic      *pConic,        /* => single conic */
DPoint3dP pPointArray,   /* => polyline */
int         numPoint,
unsigned int  curveMask,      /* => bitmask for curve properties */
const RotatedConic  *pSurface,
void        *userDataP
);

/*------------------------------------------------------+
|                                                       |
| Small sets of planes and surfaces used as clippers.   |
|                                                       |
+------------------------------------------------------*/
#define RC_MAX_TREEPLANE      6
#define RC_MAX_TREESURF       4
#define RC_MAX_TREEOP        12
#define RC_TREEOP_NOOP        0
#define RC_TREEOP_PUSH_PLANE  1
#define RC_TREEOP_PUSH_SURF   2
#define RC_TREEOP_AND         3
#define RC_TREEOP_OR          4
#define RC_TREEOP_PUSH_1      5
#define RC_TREEOP_PUSH_0      6

typedef struct
    {
    int op;
    int index;
    } RotatedConic_TreeOp;

typedef struct
    {
    RotatedConic_TreeOp opcode[RC_MAX_TREEOP];
    int numOpcode;
    RotatedConic rcSurface[RC_MAX_TREESURF];
    int numSurf;
    DPoint4d hPlane[RC_MAX_TREEPLANE];
    int numPlane;
    DPoint4d inPoint;       /* A point known to be IN */
    bool    inverted;       /* true if the space is to be inverted */
    } RotatedConic_Tree;



/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/********************************************************/
/* AABB-triangle overlap test code                      */
/* by Tomas Akenine-Möller                              */
/* Function: int triBoxOverlap(FloatType boxcenter[3],      */
/*          FloatType boxhalfsize[3],FloatType triverts[3][3]); */
/* History:                                             */
/*   2001-03-05: released the code in its first version */
/*   2001-06-18: changed the order of the tests, faster */
/*                                                      */
/* Acknowledgement: Many thanks to Pierre Terdiman for  */
/* suggestions and discussions on how to optimize code. */
/* Thanks to David Hunt for finding a ">="-bug!         */
/********************************************************/
/* Earlin Lutz port for Bentley:
* * change from float to double
*   * typedef FloatType
*   * #define FloatZero        (instead of 0.0f)
*   * #define FloatAbs fabs    (instead of fabsf)
* * add backslash where missing in middle of each AXISTEST_abc macro
*/
typedef double FloatType;
#define FloatZero 0.0
#define FloatAbs fabs

#define X 0
#define Y 1
#define Z 2
#define CROSS(dest,v1,v2) \
dest[0] = v1[1] * v2[2] - v1[2] * v2[1]; \
dest[1] = v1[2] * v2[0] - v1[0] * v2[2]; \
dest[2] = v1[0] * v2[1] - v1[1] * v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2) \
dest[0] = v1[0] - v2[0]; \
dest[1] = v1[1] - v2[1]; \
dest[2] = v1[2] - v2[2];

#define FINDMINMAX(x0,x1,x2,min,max) \
min = max = x0;   \
if (x1 < min) min = x1; \
if (x1 > max) max = x1; \
if (x2 < min) min = x2; \
if (x2 > max) max = x2;


int planeBoxOverlap(FloatType normal[3], FloatType vert[3], FloatType maxbox[3])	// -NJMP-
    {
    int q;
    FloatType vmin[3], vmax[3], v;
    for (q = X; q <= Z; q++)
        {
        v = vert[q];					// -NJMP-
        if (normal[q] > FloatZero)
            {
            vmin[q] = -maxbox[q] - v;	// -NJMP-
            vmax[q] = maxbox[q] - v;	// -NJMP-
            }
        else
            {
            vmin[q] = maxbox[q] - v;	// -NJMP-
            vmax[q] = -maxbox[q] - v;	// -NJMP-
            }
        }
    if (DOT(normal, vmin) > FloatZero) return 0;	// -NJMP-
    if (DOT(normal, vmax) >= FloatZero) return 1;	// -NJMP-
    return 0;
    }

/*======================== X-tests ========================*/
// a,b are y and z edge components of edge 01 or 12
// fa,fb are their absolute values.
// cross product of x with the vector is in the y z plane.
// 
#define AXISTEST_X01(a, b, fa, fb)			   \
p0 = a * v0[Y] - b * v0[Z];			       	   \
p2 = a * v2[Y] - b * v2[Z];			       	   \
if (p0 < p2) { min = p0; max = p2; } \
else { min = p2; max = p0; } \
rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
if (min > rad || max < -rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)			   \
p0 = a * v0[Y] - b * v0[Z];			           \
p1 = a * v1[Y] - b * v1[Z];			       	   \
if (p0 < p1) { min = p0; max = p1; } \
else { min = p1; max = p0; } \
rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
if (min > rad || max < -rad) return 0;

/*======================== Y-tests ========================*/

#define AXISTEST_Y02(a, b, fa, fb)			   \
p0 = -a * v0[X] + b * v0[Z];		      	   \
p2 = -a * v2[X] + b * v2[Z];	       	       	   \
if (p0 < p2) { min = p0; max = p2; } \
else { min = p2; max = p0; } \
rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
if (min > rad || max < -rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)			   \
p0 = -a * v0[X] + b * v0[Z];		      	   \
p1 = -a * v1[X] + b * v1[Z];	     	       	   \
if (p0 < p1) { min = p0; max = p1; } \
else { min = p1; max = p0; } \
rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
if (min > rad || max < -rad) return 0;

/*======================== Z-tests ========================*/
#define AXISTEST_Z12(a, b, fa, fb)			   \
p1 = a * v1[X] - b * v1[Y];			           \
p2 = a * v2[X] - b * v2[Y];			       	   \
if (p2 < p1) { min = p2; max = p1; }\
else { min = p1; max = p2; } \
rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
if (min > rad || max < -rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)			   \
p0 = a * v0[X] - b * v0[Y];				   \
p1 = a * v1[X] - b * v1[Y];			           \
if (p0 < p1) { min = p0; max = p1; }\
else { min = p1; max = p0; } \
rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
if (min > rad || max < -rad) return 0;


int triBoxOverlap(FloatType boxcenter[3], FloatType boxhalfsize[3], FloatType triverts[3][3])
    {
    /*    use separating axis theorem to test overlap between triangle and box */
    /*    need to test for overlap in these directions: */
    /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
    /*       we do not even need to test these) */
    /*    2) normal of the triangle */
    /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
    /*       this gives 3x3=9 more tests */
    FloatType v0[3], v1[3], v2[3];
    //   FloatType axis[3];
    FloatType min, max, p0, p1, p2, rad, fex, fey, fez;		// -NJMP- "d" local variable removed
    FloatType normal[3], e0[3], e1[3], e2[3];
    /* This is the fastest branch on Sun */
    /* move everything so that the boxcenter is in (0,0,0) */
    SUB(v0, triverts[0], boxcenter);
    SUB(v1, triverts[1], boxcenter);
    SUB(v2, triverts[2], boxcenter);

    /* compute triangle edges */
    SUB(e0, v1, v0);      /* tri edge 0 */
    SUB(e1, v2, v1);      /* tri edge 1 */
    SUB(e2, v0, v2);      /* tri edge 2 */

    /* Bullet 3:  */
    /*  test the 9 tests first (this was faster) */
    fex = FloatAbs(e0[X]);
    fey = FloatAbs(e0[Y]);
    fez = FloatAbs(e0[Z]);

    AXISTEST_X01(e0[Z], e0[Y], fez, fey);
    AXISTEST_Y02(e0[Z], e0[X], fez, fex);
    AXISTEST_Z12(e0[Y], e0[X], fey, fex);
    fex = FloatAbs(e1[X]);
    fey = FloatAbs(e1[Y]);
    fez = FloatAbs(e1[Z]);
    AXISTEST_X01(e1[Z], e1[Y], fez, fey);
    AXISTEST_Y02(e1[Z], e1[X], fez, fex);
    AXISTEST_Z0(e1[Y], e1[X], fey, fex);

    fex = FloatAbs(e2[X]);
    fey = FloatAbs(e2[Y]);
    fez = FloatAbs(e2[Z]);
    AXISTEST_X2(e2[Z], e2[Y], fez, fey);
    AXISTEST_Y1(e2[Z], e2[X], fez, fex);
    AXISTEST_Z12(e2[Y], e2[X], fey, fex);

    /* Bullet 1: */
    /*  first test overlap in the {x,y,z}-directions */
    /*  find min, max of the triangle each direction, and test for overlap in */
    /*  that direction -- this is equivalent to testing a minimal AABB around */
    /*  the triangle against the AABB */

    /* test in X-direction */
    FINDMINMAX(v0[X], v1[X], v2[X], min, max);
    if (min > boxhalfsize[X] || max < -boxhalfsize[X]) return 0;
    /* test in Y-direction */
    FINDMINMAX(v0[Y], v1[Y], v2[Y], min, max);
    if (min > boxhalfsize[Y] || max < -boxhalfsize[Y]) return 0;
    /* test in Z-direction */
    FINDMINMAX(v0[Z], v1[Z], v2[Z], min, max);
    if (min > boxhalfsize[Z] || max < -boxhalfsize[Z]) return 0;

    /* Bullet 2: */
    /*  test if the box intersects the plane of the triangle */
    /*  compute plane equation of triangle: normal*x+d=0 */
    CROSS(normal, e0, e1);
    // -NJMP- (line removed here)
    if (!planeBoxOverlap(normal, v0, boxhalfsize)) return 0;	// -NJMP-
    return 1;   /* box and triangle overlaps */
    }
/**
* Wrapper to convert Bentley DRange3d and DPoint3d args to triBoxOverlap array style and call triBoxOverlap
* return 0 if all out
* return 1 if any part of the triangle is "in"
*/
bool PolygonOps::TriangleIntersectsRangeBySeparatorPlanes(DRange3d const & range, DPoint3d const &point0, DPoint3d const &point1, DPoint3d const &point2)
    {
    FloatType triverts[3][3] = {
        {point0.x, point0.y, point0.z},
        {point1.x, point1.y, point1.z},
        {point2.x, point2.y, point2.z}
        };
    FloatType boxCenter[3], boxHalfSize[3];
    boxCenter[0] = 0.5 * (range.low.x + range.high.x);
    boxCenter[1] = 0.5 * (range.low.y + range.high.y);
    boxCenter[2] = 0.5 * (range.low.z + range.high.z);
    boxHalfSize[0] = range.high.x - boxCenter[0];
    boxHalfSize[1] = range.high.y - boxCenter[1];
    boxHalfSize[2] = range.high.z - boxCenter[2];
    return triBoxOverlap(boxCenter, boxHalfSize, triverts) != 0 ? true : false;
    }

/*
* struct PointWithAltitudes contains:
* * xyz = a point
* * altitudes[i] = its altitude (positive is OUT) from plane i.
* * outBit = bit map, with bits addressed by the 6 static constants BitLowX, BitHighX, BitLowY, BitHighY, BitLowZ, BitHighZ
*
* The Init method
* * saves xyz
* * sets all 6 altitudes
* * sets the 6 bits of outBit
*/
struct PointWithAltitudes {
    static const uint32_t BitLowX = 0x01;
    static const uint32_t BitHighX = 0x02;
    static const uint32_t BitLowY = 0x04;
    static const uint32_t BitHighY = 0x08;
    static const uint32_t BitLowZ = 0x10;
    static const uint32_t BitHighZ = 0x20;

    DPoint3d xyz;
    // altitude (out positive) from low.x, high.x, low.y. high.y, low.z, high.z
    double altitude[6];
    uint32_t outBit;
    void Init(DPoint3d const &point, DRange3d const & range) {
        xyz = point;
        outBit = 0;
        altitude[0] = range.low.x - point.x;
        if (altitude[0] > 0.0)
            outBit |= BitLowX;
        altitude[1] = point.x - range.high.x;
        if (altitude[1] > 0.0)
            outBit |= BitHighX;

        altitude[2] = range.low.y - point.y;
        if (altitude[2] > 0.0)
            outBit |= BitLowY;
        altitude[3] = point.y - range.high.y;
        if (altitude[3] > 0.0)
            outBit |= BitHighY;

        altitude[4] = range.low.z - point.z;
        if (altitude[4] > 0.0)
            outBit |= BitLowZ;
        altitude[5] = point.z - range.high.z;
        if (altitude[5] > 0.0)
            outBit |= BitHighZ;
        }
    };
/** Array with the 6 plane bits */
static const uint32_t s_planeBits[] =
    {
        PointWithAltitudes::BitLowX,
        PointWithAltitudes::BitHighX,
        PointWithAltitudes::BitLowY,
        PointWithAltitudes::BitHighY,
        PointWithAltitudes::BitLowZ,
        PointWithAltitudes::BitHighZ
    };
 
/**
 * * return true if any part of the triangular area is within the range.
 * * return false if the triangle is entirely outside.
 */
bool PolygonOps::TriangleIntersectsRangeByPlaneClip
(
    DRange3d const & range,
    DPoint3d const &point0,
    DPoint3d const &point1,
    DPoint3d const &point2
)
    {
    PointWithAltitudes points[2][11];	// clip may produce more points. also allow for a wraparound.
    points[0][0].Init(point0, range);
    if (points[0][0].outBit == 0)
        return 1;
    points[0][1].Init(point1, range);
    if (points[0][1].outBit == 0)
        return 1;
    points[0][2].Init(point2, range);
    if (points[0][2].outBit == 0)
        return 1;
    // any surviving 1 in the AND of all bits indicates completely outside in that direction . . .
    uint32_t andBits = points[0][0].outBit & points[0][1].outBit & points[0][2].outBit;
    uint32_t orBits = points[0][0].outBit | points[0][1].outBit | points[0][2].outBit;
    if (andBits != 0)
        return 0;
    // Simple tests are ambiguous ... do proper clip
    uint32_t numOld = 3;
    uint32_t numNew;
    uint32_t oldPointsIndex = 0;
    uint32_t newPointsIndex;
    uint32_t i;
    double altitude0, altitude1, fraction;
    for (auto planeSelector = 0; planeSelector < 6; planeSelector++)
        {
        // If the original points did not have both IN and OUT for this plane, don't bother clipping it.
        if ((andBits & s_planeBits[planeSelector]) != (orBits & s_planeBits[planeSelector]))
            {
            newPointsIndex = 1 - oldPointsIndex;
            points[oldPointsIndex][numOld] = points[oldPointsIndex][0];
            altitude0 = points[oldPointsIndex][0].altitude[planeSelector];
            numNew = 0;
            for (i = 1; i <= numOld; i++)
                {
                altitude1 = points[oldPointsIndex][i].altitude[planeSelector];
                if (altitude0 <= 0)
                    {
                    points[newPointsIndex][numNew++] = points[oldPointsIndex][i - 1];
                    }
                if (altitude0 * altitude1 < 0.0)
                    {
                    // the edge is split.
                    // division for the fraction is safe because the altitudes are different enough to have negative product.
                    fraction = altitude0 / (altitude0 - altitude1);
                    points[newPointsIndex][numNew].Init(DPoint3d::FromInterpolate(
                        points[oldPointsIndex][i - 1].xyz,
                        fraction,
                        points[oldPointsIndex][i].xyz), range);
                    if (points[newPointsIndex][numNew].outBit == 0)
                        return true;
                    numNew++;
                    }
                altitude0 = altitude1;
                }
            if (numNew == 0)
                return false;
            oldPointsIndex = newPointsIndex;
            numOld = numNew;
            }
        }
    // and this should never happen.  Call it an intersection case
    return true;
    }
/* Altitude functions return positive OUTSIDE */
typedef double(*AltitudeFunction)(DRange3d const &range, DPoint3d const &xyz);
static double AltidueFunction_LowX(DRange3d const &range, DPoint3d const &xyz) { return range.low.x - xyz.x; }
static double AltidueFunction_HighX(DRange3d const &range, DPoint3d const &xyz) { return xyz.x - range.high.x; }

static double AltidueFunction_LowY(DRange3d const &range, DPoint3d const &xyz) { return range.low.y - xyz.y; }
static double AltidueFunction_HighY(DRange3d const &range, DPoint3d const &xyz) { return xyz.y - range.high.y; }

static double AltidueFunction_LowZ(DRange3d const &range, DPoint3d const &xyz) { return range.low.z - xyz.z; }
static double AltidueFunction_HighZ(DRange3d const &range, DPoint3d const &xyz) { return xyz.z - range.high.z; }
static AltitudeFunction s_rangePlaneAltitudeFunction[] =
    {
    AltidueFunction_LowX,
    AltidueFunction_HighX,
    AltidueFunction_LowY,
    AltidueFunction_HighY,
    AltidueFunction_LowZ,
    AltidueFunction_HighZ
    };
/**
* * Clip a convex polygon to a range.
* * Return the clipped polygon in the same array.
* * Return empty points array anytime points.size() is 2 or less.
*/
void PolygonOps::ClipConvexPolygonToRange
(
DRange3d const &range,
bvector <DPoint3d> &points,
bvector<DPoint3d> &workPoints
)
    {
    for (AltitudeFunction F : s_rangePlaneAltitudeFunction)
        {
        size_t n = points.size();
        if (n < 3)
            {
            points.clear();
            return;
            }
        workPoints.clear();
        DPoint3d point0 = points[0];
        double altitude0 = F(range, point0);
        DPoint3d point1;
        double altitude1;
        for (size_t i = 1; i <= n; i++, altitude0 = altitude1, point0 = point1)
            {
            point1 = (i < n) ? points[i] : points[0];
            altitude1 = F(range, point1);
            if (altitude0 <= 0.0)
                {
                workPoints.push_back(point0);
                }
            if (altitude0 * altitude1 < 0.0)    // STRICTLY less than -- division will be safe.
                {
                double fraction = altitude0 / (altitude0 - altitude1);
                workPoints.push_back(DPoint3d::FromInterpolate(point0, fraction, point1));
                }
            }
        points.swap(workPoints);
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE

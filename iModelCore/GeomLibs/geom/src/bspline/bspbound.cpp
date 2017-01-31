/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspbound.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define FLAT_TOL            10
#define MINIMUM_BoundBox    fc_epsilon


/*----------------------------------------------------------------------+
|                                                                       |
|    Bounding volume Routines                                           |
|                                                                       |
|    - "box" is an oriented parallelapiped whose axes are defined       |
|       by the rectangular mesh of points                               |
|    - "sphere" is 3d sphere                                            |
|    - "range" is an orthogonal box aligned with axes                   |
|    - "cyl" is an oriented cylinder whose axis is defined by           |
|       first & last points                                             |
|    - "rectangle" is a 3D rectangle bounding a set of 3D points        |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|    Bounding 3D Routines                                               |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* pure 3d
* @bsimethod                                                    Brian.Peters    01/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bound_subtractDpoints
(
DPoint3d        *diff,
DPoint3d        *pt1,
DPoint3d        *pt2
)
    {
    diff->x = pt1->x - pt2->x;
    diff->y = pt1->y - pt2->y;
    diff->z = pt1->z - pt2->z;
    }

/*---------------------------------------------------------------------------------**//**
* pure 3d
* @bsimethod                                                    rbb             10/85
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bound_rotatePoint
(
DPoint3d        *point,
RotMatrix       *rmatrix
)
    {
    DPoint3d    oldPoint;

    oldPoint = *point;

    point->x =  oldPoint.x * rmatrix->form3d[0][0] +
                oldPoint.y * rmatrix->form3d[0][1] +
                oldPoint.z * rmatrix->form3d[0][2];
    point->y =  oldPoint.x * rmatrix->form3d[1][0] +
                oldPoint.y * rmatrix->form3d[1][1] +
                oldPoint.z * rmatrix->form3d[1][2];
    point->z =  oldPoint.x * rmatrix->form3d[2][0] +
                oldPoint.y * rmatrix->form3d[2][1] +
                oldPoint.z * rmatrix->form3d[2][2];
    }

/*---------------------------------------------------------------------------------**//**
* pure 3d
* @bsimethod                                                    rbb             10/85
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bound_unrotatePoint
(
DPoint3d        *point,
RotMatrix       *rmatrix
)
    {
    DPoint3d    oldPoint;

    oldPoint = *point;

    point->x =  oldPoint.x * rmatrix->form3d[0][0] +
                oldPoint.y * rmatrix->form3d[1][0] +
                oldPoint.z * rmatrix->form3d[2][0];
    point->y =  oldPoint.x * rmatrix->form3d[0][1] +
                oldPoint.y * rmatrix->form3d[1][1] +
                oldPoint.z * rmatrix->form3d[2][1];
    point->z =  oldPoint.x * rmatrix->form3d[0][2] +
                oldPoint.y * rmatrix->form3d[1][2] +
                oldPoint.z * rmatrix->form3d[2][2];
    }

/*----------------------------------------------------------------------+
|                                                                       |
|    Bounding Rectangle Routines                                        |
|                                                                       |
+----------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    intervalOverlap
(
double          orgA,
double          endA,
double          orgB,
double          endB
)
    {
    double      v0, v1, v2, v3;

    v0 = orgA - orgB;
    v1 = orgA - endB;
    v2 = endA - orgB;
    v3 = endA - endB;

    if (v0*v2 <= 0.0        /* orgB falls between orgA & endA */
     || v1*v3 <= 0.0        /* endB               orgA & endA */
     || v0*v1 <= 0.0        /* orgA               orgB & endB */
     || v2*v3 <= 0.0)       /* endA               orgB & endB */
        return (true);
    else
        return (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    intervalIntersect
(
double          *min,
double          *max,
double          orgA,
double          endA,
double          orgB,
double          endB
)
    {
    *min = orgA < orgB ? orgB : orgA;
    *max = endA > endB ? endB : endA;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    12/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_rectanglesIntersect
(
DRange2d      *rect1,
DRange2d      *rect2
)
    {
    return (intervalOverlap (rect1->low.x, rect1->high.x,
                             rect2->low.x, rect2->high.x) &&
            intervalOverlap (rect1->low.y, rect1->high.y,
                             rect2->low.y, rect2->high.y));
    }

/*----------------------------------------------------------------------+
|                                                                       |
|    Oriented Bounding Box Routines                                     |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void     bound_boxCompute
(
BoundBox        *box,
DPoint3d        *points,
int             numU,
int             numV
)
    {
    int         i, j;
    double      mag;
    DPoint3d    *pP, temp, *sysP;

#if defined (debug_boxCompute)
    {
    int           showPoints=false;
    ElementUnion  u;

    if (showPoints)
        {
        create_lineStringD (&u, NULL, points, NULL, numU*numV, LINE_STRING_ELM, 0);
        mdlElement_display (&u, 0);
        }
    }
#endif

    sysP = (DPoint3d *) &box->system;

    box->origin = points[0];
    if ((box->extent.x = bsiDPoint3d_computeNormal (sysP, points+numU-1, points)) < fc_epsilon)
       box->extent.x = bsiDPoint3d_computeNormal (sysP, points+1, points);

    bound_subtractDpoints (sysP+1, points+(numV-1)*numU, points);
    if (bsiDPoint3d_magnitude (sysP+1) < fc_epsilon)
        bound_subtractDpoints (sysP+1, points+numU, points);

    bsiDPoint3d_crossProduct (sysP+2, sysP, sysP+1);
    mag = bsiDPoint3d_normalizeInPlace (sysP+2);
    if (mag < 1.0e-15)
        {
        sysP->x = sysP->y = 0.0; sysP->z = 1.0; box->extent.x = MINIMUM_BoundBox;
        }

    bsiDPoint3d_crossProduct (sysP+1, sysP+2, sysP);

    box->extent.y = box->extent.z = MINIMUM_BoundBox;

    for (j=0, pP=points; j < numV; j++)
        for (i=0; i < numU; i++, pP++)
            {
            bound_subtractDpoints (&temp, pP, &box->origin);
            bound_rotatePoint (&temp, (RotMatrix *) box->system);

            if (temp.x > box->extent.x)
                box->extent.x = temp.x;
            else if (temp.x < 0.0)
                {
                box->extent.x -= temp.x;
                bsiDPoint3d_addScaledDPoint3d (&box->origin, &box->origin, sysP, temp.x);
                }

            if (temp.y > box->extent.y)
                box->extent.y = temp.y;
            else if (temp.y < 0.0)
                {
                box->extent.y -= temp.y;
                bsiDPoint3d_addScaledDPoint3d (&box->origin, &box->origin, sysP+1, temp.y);
                }

            if (temp.z > box->extent.z)
                box->extent.z = temp.z;
            else if (temp.z < 0.0)
                {
                box->extent.z -= temp.z;
                bsiDPoint3d_addScaledDPoint3d (&box->origin, &box->origin, sysP+2, temp.z);
                }
            }
    }

/*---------------------------------------------------------------------------------**//**
* Loosely based on - A Linear-Time Simple Bounding Volume Algorithum Xiolain Wu (Page 301, Graphics Gems III)
* @bsimethod                                                    Ray.Bentley     05/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void bound_optimizedBoxCompute
(
BoundBox    *boxP,
DPoint3d    *pointP,
int         nPoints
)
    {
    int         nIterations;
    double      c[3][3], eigenValues[3], eigenSystem[3][3];
    DPoint3d    *pP, *endP, test, centroid;

    /* Compute covariance matrix and z limits */
    memset (c, 0, sizeof(c));
    centroid.x = centroid.y = centroid.z = 0.0;
    for (pP=pointP, endP=pointP+nPoints; pP<endP; pP++)
        {
        bsiDPoint3d_addDPoint3dDPoint3d (&centroid, &centroid, pP);

        c[0][0] += pP->x * pP->x;
        c[1][1] += pP->y * pP->y;
        c[2][2] += pP->z + pP->z;

        c[0][1] += pP->x * pP->y;
        c[0][2] += pP->x * pP->z;
        c[1][2] += pP->y * pP->z;
        }
    c[0][0] -= centroid.x * centroid.x / (double) nPoints;
    c[1][1] -= centroid.y * centroid.y / (double) nPoints;
    c[2][2] -= centroid.z + centroid.z / (double) nPoints;

    c[0][1] -= centroid.x * centroid.y / (double) nPoints;
    c[0][2] -= centroid.x * centroid.z / (double) nPoints;
    c[1][2] -= centroid.y * centroid.z / (double) nPoints;

    c[1][0] = c[0][1];
    c[2][0] = c[0][2];
    c[2][1] = c[1][2];

    bsiGeom_jacobi3X3 (eigenValues, eigenSystem, &nIterations, c);
    bsiRotMatrix_transpose ((RotMatrix*)boxP->system, (RotMatrix*)eigenSystem);
    boxP->origin.x = boxP->origin.y = boxP->origin.z = fc_hugeVal;
    boxP->extent.x = boxP->extent.y = boxP->extent.z = -fc_hugeVal;
    for (pP=pointP, endP=pointP+nPoints; pP<endP; pP++)
        {
        test = *pP;
        bound_rotatePoint (&test, (RotMatrix *) boxP->system);
        if (test.x < boxP->origin.x)        boxP->origin.x = test.x;
        if (test.y < boxP->origin.y)        boxP->origin.y = test.y;
        if (test.z < boxP->origin.z)        boxP->origin.z = test.z;
        if (test.x > boxP->extent.x)        boxP->extent.x = test.x;
        if (test.y > boxP->extent.y)        boxP->extent.y = test.y;
        if (test.z > boxP->extent.z)        boxP->extent.z = test.z;
        }

    bound_subtractDpoints (&boxP->extent, &boxP->extent, &boxP->origin);
    bound_unrotatePoint (&boxP->origin, (RotMatrix *) boxP->system);
    }

/*---------------------------------------------------------------------------------**//**
* pts[0] = box->origin 6---------7 pts[1] = x direction /| /| pts[2] = y direction / | / | pts[3] = z direction 3---------5 | . | | | | . |
* 2------|--4 . | / | / as indicated. |/ |/ 0---------1
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void     bound_getBoxCorners
(
DPoint3d        *pts,                  /* <= corners in world coordinates */
BoundBox        *box
)
    {
    double      *extP;
    DPoint3d    *sysP;

    extP = (double *) &box->extent;
    sysP = (DPoint3d *) &box->system;

    pts[0] = box->origin;
    bsiDPoint3d_addScaledDPoint3d (pts+1, pts,   sysP,   extP[0]);
    bsiDPoint3d_addScaledDPoint3d (pts+2, pts,   sysP+1, extP[1]);
    bsiDPoint3d_addScaledDPoint3d (pts+3, pts,   sysP+2, extP[2]);
    bsiDPoint3d_addScaledDPoint3d (pts+4, pts+1, sysP+1, extP[1]);
    bsiDPoint3d_addScaledDPoint3d (pts+5, pts+1, sysP+2, extP[2]);
    bsiDPoint3d_addScaledDPoint3d (pts+6, pts+2, sysP+2, extP[2]);
    bsiDPoint3d_addScaledDPoint3d (pts+7, pts+4, sysP+2, extP[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bound_pointsInRange
(
DPoint3d        *points,
int             numPoints,
DPoint3d        *extent
)
    {
    DPoint3d    *pP, *endP;

    for (pP=endP=points, endP += numPoints; pP < endP; pP++)
        {
        if ((-fc_epsilon <= pP->x && pP->x <= extent->x + fc_epsilon) &&
            (-fc_epsilon <= pP->y && pP->y <= extent->y + fc_epsilon) &&
            (-fc_epsilon <= pP->z && pP->z <= extent->z + fc_epsilon))
           return (true);
        }

    return (false);
    }

/*---------------------------------------------------------------------------------**//**
* segment is in rotated coordinate system of box, so the normals of the box are simply unit vectors in x,y,z
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool    bound_segmentBoxIntersect
(
DSegment3d       *segment,
BoundBox        *box
)
    {
    int         i;
    double      *dirP, *segP, *extP, param;
    DPoint3d    direction, intPt;

    bound_subtractDpoints (&direction, &segment->point[0], &segment->point[1]);

    dirP = (double *) &direction;
    segP = (double *) &segment->point[0];
    extP = (double *) &box->extent;
    for (i=0; i < 3; i++, dirP++, segP++, extP++)
        {
        if (fabs (*dirP) > fc_epsilon)
            {
            param = *segP / *dirP;
            if (0.0 <= param && param <= 1.0)
                {
                bsiDPoint3d_addScaledDPoint3d (&intPt, &segment->point[0], &direction, -param);
                if (bound_pointsInRange (&intPt, 1, &box->extent))
                    return (true);
                }

            param = (*segP - *extP) / *dirP;
            if (0.0 <= param && param <= 1.0)
                {
                bsiDPoint3d_addScaledDPoint3d (&intPt, &segment->point[0], &direction, -param);
                if (bound_pointsInRange (&intPt, 1, &box->extent))
                    {
                    return (true);
                    }
                }
            }
        }
    return (false);
    }

#define OFFLEFT     0x0001
#define OFFRIGHT    0x0002
#define OFFTOP      0x0004
#define OFFBTM      0x0008
#define OFFFRONT    0x0010
#define OFFBACK     0x0020
#define OFFALL      0x003f

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bound_box1in2
(
BoundBox        *b1,
BoundBox        *b2
)
    {
    int         currentTest, accumulateTest;
    DPoint3d    *pP, pts[8], *endP;
    DSegment3d   *eP, edges[12], *endE;

    bound_getBoxCorners (pts, b1);
    for (pP=endP=pts, endP += 8; pP < endP; pP++)
        {
        bound_subtractDpoints (pP, pP, &b2->origin);
        bound_rotatePoint (pP, (RotMatrix *) b2->system);
        }

    /* If all points outside of any face then edge intersection not posibble */
    accumulateTest = OFFALL;
    for (pP=endP=pts, endP += 8; pP < endP; pP++)
        {
        currentTest = 0;
        if (pP->x < -fc_epsilon)                currentTest |= OFFLEFT;
        if (pP->y < -fc_epsilon)                currentTest |= OFFBTM;
        if (pP->z < -fc_epsilon)                currentTest |= OFFBACK;
        if (pP->x > b2->extent.x + fc_epsilon)  currentTest |= OFFRIGHT;
        if (pP->y > b2->extent.y + fc_epsilon)  currentTest |= OFFTOP;
        if (pP->z > b2->extent.z + fc_epsilon)  currentTest |= OFFFRONT;

        /* If all tests pass, points is in range */
        if (! currentTest)
            return true;
        else
            accumulateTest &= currentTest;
        }
    if (accumulateTest)     return false;

    /* If any edge crosses any face then intersection */
    edges[0].point[0] = edges[4].point[0] = edges[8].point[0]  = pts[0];
    edges[0].point[1] = edges[5].point[0] = edges[9].point[0]  = pts[1];
    edges[1].point[0] = edges[4].point[1] = edges[11].point[0] = pts[2];
    edges[3].point[0] = edges[7].point[0] = edges[8].point[1]  = pts[3];
    edges[1].point[1] = edges[5].point[1] = edges[10].point[0] = pts[4];
    edges[3].point[1] = edges[6].point[0] = edges[9].point[1]  = pts[5];
    edges[2].point[0] = edges[7].point[1] = edges[11].point[1] = pts[6];
    edges[2].point[1] = edges[6].point[1] = edges[10].point[1] = pts[7];

    for (eP=endE=edges, endE += 12; eP < endE; eP++)
        if (bound_segmentBoxIntersect (eP, b2))
            return (true);

    return (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_boxesIntersect
(
BoundBox        *b1,
BoundBox        *b2
)
    {
    if (bound_box1in2 (b1, b2) || bound_box1in2 (b2, b1))
        return (true);
    else
        return (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_boxFlat
(
BoundBox        *box,
double          tolerance
)
    {
    double      tmp;

    if (box->extent.z <= tolerance ||
        box->extent.y <= tolerance ||
        box->extent.x <= tolerance)
        return (true);

    tmp = box->extent.z * FLAT_TOL;
    if (tmp < box->extent.x && tmp < box->extent.y)
        return (true);
    tmp = box->extent.y * FLAT_TOL;
    if (tmp < box->extent.x && tmp < box->extent.z)
        return (true);
    tmp = box->extent.x * FLAT_TOL;
    if (tmp < box->extent.y && tmp < box->extent.z)
        return (true);
    return (false);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void     bound_boxFromSurface
(
BoundBox            *box,
MSBsplineSurface    *surface
)
    {
    int         total;

    if (surface->rational)
        {
        total = surface->uParams.numPoles * surface->vParams.numPoles;
        if (total > MAX_ORDER*MAX_ORDER)
            {
            bsputil_unWeightPoles (surface->poles, surface->poles, surface->weights, total);
            bound_boxCompute (box, surface->poles, surface->uParams.numPoles,
                              surface->vParams.numPoles);
            bsputil_weightPoles (surface->poles, surface->poles, surface->weights, total);

            }
        else
            {
            DPoint3d    local[MAX_ORDER*MAX_ORDER];

            bsputil_unWeightPoles (local, surface->poles, surface->weights, total);
            bound_boxCompute (box, local, surface->uParams.numPoles,
                              surface->vParams.numPoles);
            }
        }
    else
        {
        bound_boxCompute (box, surface->poles, surface->uParams.numPoles,
                          surface->vParams.numPoles);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public  int bound_optimizedBoxFromSurface
(
BoundBox            *boxP,
MSBsplineSurface    *surfaceP
)
    {
    if (surfaceP->uParams.order == 2 && surfaceP->vParams.order == 2)
        {
        bound_boxFromSurface (boxP, surfaceP);
        return SUCCESS;
        }
    else
        {
        int             totalPoles = surfaceP->uParams.numPoles * surfaceP->vParams.numPoles;
        DPoint3d        *pointP;
        if (surfaceP->rational)
            {
            pointP = (DPoint3d*)_alloca (totalPoles * sizeof(DPoint3d));
            if (!pointP)
                return ERROR;
            bsputil_unWeightPoles (pointP, surfaceP->poles, surfaceP->weights, totalPoles);
            }
        else
            {
            pointP = surfaceP->poles;
            }

        bound_optimizedBoxCompute (boxP, pointP, totalPoles);


        return SUCCESS;
        }
    }


#define compile_bound_boxFromCyl
#ifdef compile_bound_boxFromCyl

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    11/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void     bound_boxFromCyl
(
BoundBox        *box,
BoundCyl        *cyl,
double          expansion
)
    {
    double      expandedRadius;

    expandedRadius = cyl->radius + expansion;

    if (true /*mdlActiveModel_is3d()*/)
        rotMatrix_orthogonalFromZRow ((RotMatrix *) box->system, (DVec3d*)&cyl->dir);
    else
        {
        box->system[0][0] = box->system[0][1] = 0.0;
        box->system[0][2] = 1.0;
        bsiDPoint3d_crossProduct ((DPoint3d*)box->system[1], &cyl->dir, (DPoint3d*)box->system[0]);
        memcpy (box->system[2], &cyl->dir, sizeof(DPoint3d));
        }

    bsiDPoint3d_addScaledDPoint3d (&box->origin, &cyl->org,    (DPoint3d*)box->system[0], -expandedRadius);
    bsiDPoint3d_addScaledDPoint3d (&box->origin, &box->origin, (DPoint3d*)box->system[1], -expandedRadius);
    box->extent.x = box->extent.y = 2.0 * expandedRadius;
    box->extent.z = cyl->length + expansion;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    12/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void     bound_boxFromCurve
(
BoundBox        *box,
MSBsplineCurve  *curve
)
    {
    BoundCyl    cyl;

    bound_cylFromCurve (&cyl, curve);
    bound_boxFromCyl (box, &cyl, 0.0);
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    09/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    pntutil_crossFrom3Pts
(
DPoint3d        *cross,
DPoint3d        *origin,
DPoint3d        *p1,
DPoint3d        *p2
)
    {
    DPoint3d    diff0, diff1;

    bsiDPoint3d_computeNormal(&diff0, p1, origin);
    bsiDPoint3d_computeNormal (&diff1, p2, origin);
    bsiDPoint3d_crossProduct (cross, &diff0, &diff1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    09/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void     bound_boxFromEval
(
BoundBox        *box,
Evaluator       *eval
)
    {
    if (eval->offset)
        {
        int                 uNum, vNum, total;
        DPoint3d            norm, cross;
        MSBsplineSurface    *surf=eval->surf;

        memset (&norm, 0, sizeof(norm));
        uNum = surf->uParams.numPoles;
        vNum = surf->vParams.numPoles;
        total = uNum * vNum;

        bound_boxFromSurface (box, surf);

        if (surf->rational)
            bsputil_unWeightPoles (surf->poles, surf->poles, surf->weights, total);
        // normal vector at 00
        pntutil_crossFrom3Pts (&norm, surf->poles, surf->poles+1, surf->poles+uNum);
        // normal vector at 10
        pntutil_crossFrom3Pts (&cross,
                surf->poles+uNum-1, surf->poles+2*uNum-1, surf->poles+uNum-2);
        bsiDPoint3d_addDPoint3dDPoint3d (&norm, &norm, &cross);
        // normal vector at 11
        pntutil_crossFrom3Pts (&cross,
                surf->poles+total-1, surf->poles+total-2, surf->poles+total-uNum-1);
        bsiDPoint3d_addDPoint3dDPoint3d (&norm, &norm, &cross);
        // normal vector at 01
        pntutil_crossFrom3Pts (&cross,
                surf->poles+total-uNum, surf->poles+total-2*uNum, surf->poles+total-uNum+1);
        bsiDPoint3d_addDPoint3dDPoint3d (&norm, &norm, &cross);

        bsiDPoint3d_scale (&norm, &norm, 0.25);
        // Shift by average normal.  This is really suspicious scaling -- the normal is computed by
        // cross product of two normalized vectors.   This is NOT a unit vector !!!!
        // So it looks like this wants to be moving by eval->distance in the average normal direction,
        // but it really doesn't.
        bsiDPoint3d_addScaledDPoint3d (&box->origin, &box->origin, &norm, eval->distance);

        if (surf->rational)
            bsputil_weightPoles (surf->poles, surf->poles, surf->weights, total);
        }
    else
        bound_boxFromSurface (box, eval->surf);
    }



/*----------------------------------------------------------------------+
|                                                                       |
|    Bounding Sphere Routines                                           |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* from Graphics Gems page 301
* @bsimethod                                                    BFP             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public  double   bound_sphereCompute
(
DPoint3d        *center,
double          *radius,
DPoint3d        *points,
int             numPoints
)
    {
    int         i;
    double      ySpan, zSpan, maxSpan, radSquared, distSquared, dist,
                deltaRad;
    DPoint3d    *pP, minx, miny, minz, maxx, maxy, maxz, diff;
    DRange3d   diameter;

    minx.x = miny.y = minz.z = fc_hugeVal;
    maxx.x = maxy.y = maxz.z = -fc_hugeVal;
    for (i=0, pP=points; i < numPoints; i++, pP++)
        {
        if (pP->x < minx.x)     minx = *pP;
        if (pP->y < miny.y)     miny = *pP;
        if (pP->z < minz.z)     minz = *pP;

        if (pP->x > maxx.x)     maxx = *pP;
        if (pP->y > maxy.y)     maxy = *pP;
        if (pP->z > maxz.z)     maxz = *pP;
        }

    maxSpan = bsiDPoint3d_distance (&minx, &maxx);
    ySpan = bsiDPoint3d_distance (&miny, &maxy);
    zSpan = bsiDPoint3d_distance (&minz, &maxz);

    diameter.low = minx;
    diameter.high = maxx;

    if (ySpan > maxSpan)
        {
        maxSpan = ySpan;
        diameter.low = miny;
        diameter.high = maxy;
        }
    if (zSpan > maxSpan)
        {
        maxSpan = zSpan;
        diameter.low = minz;
        diameter.high = maxz;
        }

    center->x = (diameter.low.x + diameter.high.x) / 2.0;
    center->y = (diameter.low.y + diameter.high.y) / 2.0;
    center->z = (diameter.low.z + diameter.high.z) / 2.0;

    *radius = bsiDPoint3d_distance (center, &diameter.low);
    radSquared = *radius * *radius;

    for (i=0, pP=points; i < numPoints; i++, pP++)
        {
        bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, pP, center);
        distSquared = diff.x*diff.x + diff.y*diff.y + diff.z*diff.z;
        if (distSquared > radSquared)
            {
            dist = sqrt (distSquared);
            *radius = (*radius + dist) / 2.0;
            radSquared = *radius * *radius;
            deltaRad = dist - *radius;

            center->x = (*radius * center->x + deltaRad * pP->x) / dist;
            center->y = (*radius * center->y + deltaRad * pP->y) / dist;
            center->z = (*radius * center->z + deltaRad * pP->z) / dist;
            }
        }
    return (radSquared);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             01/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_sphereMinMax
(
double          *min,
double          *max,
DPoint3d        *testPt,
DPoint3d        *center,
double          radius
)
    {
    bool        inside = false;
    double      dist = bsiDPoint3d_distance (testPt, center);

    *min = dist - radius;
    if (*min < 0.0)
        inside = true;

    *max = dist + radius;

    return (inside);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             01/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_spherePtInside
(
DPoint3d        *testPt,
DPoint3d        *center,
double          radius
)
    {
    double      mag = bsiDPoint3d_distance (center, testPt);

    return (mag < radius ? true : false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_spherePairDist
(
double          *minDist,
double          *maxDist,
DPoint3d        *center0,
double          radius0,
DPoint3d        *center1,
double          radius1
)
    {
    bool        intersect = false;
    double      dist = bsiDPoint3d_distance (center0, center1);

    *minDist = dist - radius0 - radius1;
    if (*minDist < 0.0)
        intersect = true;

    *maxDist = dist + radius0 + radius1;

    return (intersect);
    }

/*---------------------------------------------------------------------------------**//**
* from Graphic Gems page 388
* @bsimethod                                                    BFP             01/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_sphereLineIntersect
(
DPoint3d        *point,                /* <= intersect point, or NULL */
DRay3d          *ray,                  /* => origin & unit vector */
DPoint3d        *center,
double          radius
)
    {
    double      dot, mag, disc, radSq;
    DPoint3d    diff;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, center, &ray->origin);
    dot = bsiDPoint3d_dotProduct (&ray->direction, &diff);
    mag = bsiDPoint3d_dotProduct (&diff, &diff);

    radSq = radius*radius;
    disc = radSq - (mag - dot*dot);

    if (disc < 0.0)
        return (false);
    else if (dot < 0.0 && mag > radSq)    /* modification */
        return (false);
    else
        {
        if (point)
            {
            disc = sqrt (disc);
            mag = dot - disc;
            bsiDPoint3d_addScaledDPoint3d (point, &ray->origin, &ray->direction, mag);
            }
        return (true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* from Graphic Gems page 388, but modified for ray projecting in only one direction
* @bsimethod                                                    BFP             01/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_sphereRayIntersect
(
DPoint3d        *point,                /* <= intersect point, or NULL */
DRay3d          *ray,                  /* => origin & unit vector */
DPoint3d        *center,
double          radius
)
    {
    double      dot, mag, disc, radSq;
    DPoint3d    diff;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, center, &ray->origin);
    dot = bsiDPoint3d_dotProduct (&ray->direction, &diff);
    mag = bsiDPoint3d_dotProduct (&diff, &diff);

    radSq = radius*radius;
    disc = radSq - (mag - dot*dot);

    if (disc < 0.0)
        return (false);
    else if (dot < 0.0 && mag > radSq)    /* modification */
        return (false);
    else
        {
        if (point)
            {
            disc = sqrt (disc);
            mag = dot - disc;
            bsiDPoint3d_addScaledDPoint3d (point, &ray->origin, &ray->direction, mag);
            }
        return (true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* returns true if segment lies entirely within sphere
* @bsimethod                                                    BFP             01/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_sphereIntersectSegment
(
DPoint3d        *point,                /* <= intersect point, or NULL */
DSegment3d       *segment,              /* => endpoints of segment */
DPoint3d        *center,
double          radius
)
    {
    double      dot, mag, disc, length, radSqd;
    DPoint3d    diff, norm;

    length = bsiDPoint3d_computeNormal (&norm, &segment->point[1], &segment->point[0]);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, center, &segment->point[0]);
    dot = bsiDPoint3d_dotProduct (&diff, &norm);
    mag = bsiDPoint3d_dotProduct (&diff, &diff);

    radSqd = radius * radius;
    disc = radSqd - (mag - dot*dot);

    /* return true if segment lies inside sphere */
    if (mag < radSqd)   return (true);

    if (disc < 0.0)
        {
        return (false);
        }
    else
        {
        disc = sqrt (disc);
        mag = dot - disc;
        if (mag > length || dot < 0.0)
            return (false);

        if (point)
            bsiDPoint3d_addScaledDPoint3d (point, &segment->point[0], &norm, mag);
        return (true);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public  double   bound_sphereFromCurve
(
DPoint3d        *center,
double          *radius,
MSBsplineCurve  *curve
)
    {
    double      mag;

    if (curve->rational)
        {
        if (curve->params.numPoles > MAX_ORDER)
            {
            bsputil_unWeightPoles (curve->poles, curve->poles, curve->weights,
                                   curve->params.numPoles);
            mag = bound_sphereCompute (center, radius, curve->poles, curve->params.numPoles);
            bsputil_weightPoles (curve->poles, curve->poles, curve->weights,
                                 curve->params.numPoles);
            return (mag);
            }
        else
            {
            DPoint3d    local[MAX_ORDER];

            bsputil_unWeightPoles (local, curve->poles, curve->weights,
                                   curve->params.numPoles);
            return bound_sphereCompute (center, radius, local, curve->params.numPoles);
            }
        }
    else
        return bound_sphereCompute (center, radius, curve->poles, curve->params.numPoles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public  double   bound_sphereFromSurface
(
DPoint3d            *center,
double              *radius,
MSBsplineSurface    *surface
)
    {
    if (surface->rational)
        {
        int             total;

        total = surface->uParams.numPoles * surface->vParams.numPoles;
        if (total > MAX_ORDER*MAX_ORDER)
            {
            double      mag;

            bsputil_unWeightPoles (surface->poles, surface->poles, surface->weights, total);
            mag = bound_sphereCompute (center, radius, surface->poles, total);
            bsputil_weightPoles (surface->poles, surface->poles, surface->weights, total);
            return (mag);
            }
        else
            {
            DPoint3d    local[MAX_ORDER*MAX_ORDER];

            bsputil_unWeightPoles (local, surface->poles, surface->weights, total);
            return bound_sphereCompute (center, radius, local, total);
            }
        }
    else
        {
        return bound_sphereCompute (center, radius, surface->poles,
                                    surface->uParams.numPoles * surface->vParams.numPoles);
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
|    Range Box Routines                                                 |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* NOTE: degenerate dimensions are normalized to have no effect.
* @bsimethod                                                    Brian.Peters    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
static double  bound_rangeComputeVolume
(
DRange3d *range
)
    {
    double      tmp;
    DPoint3d    delta;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&delta, &range->high, &range->low);

    if ((tmp = fabs (delta.x)) < fc_epsilon)
        delta.x = 1.0;
    else
        delta.x = tmp;

    if ((tmp = fabs (delta.y)) < fc_epsilon)
        delta.y = 1.0;
    else
        delta.y = tmp;

    if ((tmp = fabs (delta.z)) < fc_epsilon)
        delta.z = 1.0;
    else
        delta.z = tmp;

    return (delta.x * delta.y * delta.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
static double  bound_rangeCompute
(
DRange3d       *bound,
DPoint3d        *points,
int             numPoints
)
    {
    DPoint3d    *pP, *endP;

    /* Calculate a min/max bounding box for the points */
    bound->low.x = bound->low.y = bound->low.z = fc_hugeVal;
    bound->high.x = bound->high.y = bound->high.z = -fc_hugeVal;

    for (pP=endP=points, endP += numPoints; pP < endP; pP++)
        {
        bound->low.x = pP->x < bound->low.x ? pP->x : bound->low.x;
        bound->low.y = pP->y < bound->low.y ? pP->y : bound->low.y;
        bound->low.z = pP->z < bound->low.z ? pP->z : bound->low.z;

        bound->high.x = pP->x > bound->high.x ? pP->x : bound->high.x;
        bound->high.y = pP->y > bound->high.y ? pP->y : bound->high.y;
        bound->high.z = pP->z > bound->high.z ? pP->z : bound->high.z;
        }
    return bsiDPoint3d_distance (&bound->low, &bound->high);
    }
#ifdef compile_bound_rangePairDist
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bound_rangePairDist
(
double          *minDist,
double          *maxDist,
DRange3d       *box0,
DRange3d       *box1
)
    {
    int         tags[4];
    double      dist[4];

    dist[0] = bsiDPoint3d_distance (&box0->low, &box1->low);
    dist[1] = bsiDPoint3d_distance (&box0->low, &box1->high);
    dist[2] = bsiDPoint3d_distance (&box0->high, &box1->low);
    dist[3] = bsiDPoint3d_distance (&box0->high, &box1->high);

    util_tagSort (tags, dist, 4);
    *minDist = dist[tags[0]];
    *maxDist = dist[tags[3]];
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_rangesIntersect
(
DRange3d       *range0,
DRange3d       *range1
)
    {
    return (intervalOverlap (range0->low.x, range0->high.x,
                             range1->low.x, range1->high.x) &&
            intervalOverlap (range0->low.y, range0->high.y,
                             range1->low.y, range1->high.y) &&
            intervalOverlap (range0->low.z, range0->high.z,
                             range1->low.z, range1->high.z));
    }
#ifdef compile_bound_rangerayIntersect
/*---------------------------------------------------------------------------------**//**
* from Graphics Gem page 395
* @bsimethod                                                    BFP             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bound_rangerayIntersect
(
DPoint3d        *coord,                /* <= coordinates of hit */
DRange3d        *box,                  /* => bounding box */
DRay3d          *ray                   /* => ray origin and direction */
)
    {
    int         max;
    double      *dist;
    Point3d     tst;        /* 0 if ray org below, 1 if above, -1 if inside */
    DPoint3d    origin, direction;
    DPoint3d    distance,    /* distance of closest plane to origin along ray */
                candidate;    /* closest of pair of bounding planes */

    origin    = ray->origin;
    direction = ray->direction;

    tst.x = (origin.x < box->low.x ? 0 : (origin.x > box->high.x ? 1 : -1));
    tst.y = (origin.y < box->low.y ? 0 : (origin.y > box->high.y ? 1 : -1));
    tst.z = (origin.z < box->low.z ? 0 : (origin.z > box->high.z ? 1 : -1));

    /* Check if origin of ray is inside box */
    if (tst.x == -1 && tst.y == -1 && tst.z == -1)
        return (true);

    candidate.x = tst.x > -1 ? ((DPoint3d *) box)[tst.x].x : 0.0;
    candidate.y = tst.y > -1 ? ((DPoint3d *) box)[tst.y].y : 0.0;
    candidate.z = tst.z > -1 ? ((DPoint3d *) box)[tst.z].z : 0.0;
    // Is direction a unit vector?  This division test is pretty safe for a unit vector,
    //    knot sure otherwise.
    distance.x = (tst.x == -1 || MSBsplineCurve::SameWeight (direction.x, 0.0)) ?
                 -1.0  : (candidate.x - origin.x) / direction.x;
    distance.y = (tst.y == -1 || MSBsplineCurve::SameWeight (direction.y, 0.0)) ?
                 -1.0  : (candidate.y - origin.y) / direction.y;
    distance.z = (tst.z == -1 || MSBsplineCurve::SameWeight (direction.z, 0.0)) ?
                 -1.0  : (candidate.z - origin.z) / direction.z;

    dist = (double *) &distance;
    max = 0;
    max = dist[max] < dist[1] ? 1 : max;
    max = dist[max] < dist[2] ? 2 : max;

    if (dist[max] < 0.0)
        return (false);

    coord->x = max == 0 ? candidate.x : origin.x + dist[max] * direction.x;
    coord->y = max == 0 ? candidate.y : origin.y + dist[max] * direction.y;
    coord->z = max == 0 ? candidate.z : origin.z + dist[max] * direction.z;

    /* test left to do */
    if ((tst.x == 0 && coord->x > box->high.x) ||
        (tst.x == 1 && coord->x < box->low.x) ||
        (tst.y == 0 && coord->y > box->high.y) ||
        (tst.y == 1 && coord->y < box->low.y) ||
        (tst.z == 0 && coord->z > box->high.z) ||
        (tst.z == 1 && coord->z < box->low.z))
        return (false);

    return (true);
    }
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    07/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public  double   bound_rangeFromBox
(
DRange3d       *range,
BoundBox        *box
)
    {
    DPoint3d    corners[8];

    bound_getBoxCorners (corners, box);
    return bound_rangeCompute (range, corners, 8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    12/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_range1EntirelyIn2
(
DRange3d       *r1,                   /* => input range */
DRange3d       *r2                    /* => input range */
)
    {
    return (r1->low.x >= r2->low.x &&
            r1->low.y >= r2->low.y &&
            r1->low.z >= r2->low.z &&

            r1->high.x <= r2->high.x &&
            r1->high.y <= r2->high.y &&
            r1->high.z <= r2->high.z);
    }

/*---------------------------------------------------------------------------------**//**
* out = the common portion of the two input ranges
* @bsimethod                                                    Brian.Peters    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bound_rangeComputeIntersect  /* <= false if they do not intersect */
(
DRange3d       *out,                  /* <= common range */
DRange3d       *in1,                  /* => input range */
DRange3d       *in2                   /* => input range */
)
    {
    DRange3d   r1, r2;

    if (!bound_rangesIntersect (in1, in2))
        return false;

    r1 = *in1;
    r2 = *in2;

    intervalIntersect (&out->low.x, &out->high.x, r1.low.x, r1.high.x, r2.low.x, r2.high.x);
    intervalIntersect (&out->low.y, &out->high.y, r1.low.y, r1.high.y, r2.low.y, r2.high.y);
    intervalIntersect (&out->low.z, &out->high.z, r1.low.z, r1.high.z, r2.low.z, r2.high.z);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void bound_rangeComputeUnion
(
DRange3d   *unionP,
DRange3d   *rect1P,
DRange3d   *rect2P
)
    {
    unionP->low.x = rect1P->low.x < rect2P->low.x ? rect1P->low.x : rect2P->low.x;
    unionP->low.y = rect1P->low.y < rect2P->low.y ? rect1P->low.y : rect2P->low.y;
    unionP->low.z = rect1P->low.z < rect2P->low.z ? rect1P->low.z : rect2P->low.z;
    unionP->high.x = rect1P->high.x > rect2P->high.x ? rect1P->high.x : rect2P->high.x;
    unionP->high.y = rect1P->high.y > rect2P->high.y ? rect1P->high.y : rect2P->high.y;
    unionP->high.z = rect1P->high.z > rect2P->high.z ? rect1P->high.z : rect2P->high.z;
    }

/*---------------------------------------------------------------------------------**//**
* returns (volume 1 that is inside 2 / volume 1)
* @bsimethod                                                    Brian.Peters    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public  double   bound_rangeAmount1In2 /* <= percent contained */
(
DRange3d       *r1,                   /* => input range */
DRange3d       *r2                    /* => input range */
)
    {
    double      r1Volume, commonVolume;
    DRange3d common;

    if (! bound_rangeComputeIntersect (&common, r1, r2))
        return 0.0;

    r1Volume = bound_rangeComputeVolume (r1);
    commonVolume = bound_rangeComputeVolume (&common);

    return (commonVolume / r1Volume);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_rangeContainsPoints
(
int             *index,                /* <= index of first point in range, or NULL */
int             *numOut,               /* <= number of points outside, or NULL */
DPoint3d        *points,
int             numPoints,
DRange3d        *range
)
    {
    DPoint3d    *pP, *highP;

    if (index)      *index = -1;
    if (numOut)     *numOut = 0;

    for (pP = highP = points, highP += numPoints; pP < highP; pP++)
        {
        if ((range->low.x <= pP->x && pP->x <= range->high.x) &&
            (range->low.y <= pP->y && pP->y <= range->high.y) &&
            (range->low.z <= pP->z && pP->z <= range->high.z))
            break;
        }

    if (pP < highP)
        {
#ifdef TODO_EliminateAddressArithmetic
#endif
        if (index)
            *index = (int)(pP - points);

        if (numOut)
            {
            for (pP++; pP < highP; pP++)
                {
                if ((range->low.x > pP->x || pP->x > range->high.x) ||
                    (range->low.y > pP->y || pP->y > range->high.y) ||
                    (range->low.z > pP->z || pP->z > range->high.z))
                    *numOut += 1;
                }
            }

        return true;
        }
    else
        {
        if (numOut)
            *numOut = numPoints;
        return false;
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_rangeContainsCurvePolygon
(
int             *index,
int             *numOut,
MSBsplineCurve  *curve,
DRange3d        *range
)
    {
    if (curve->rational)
        {
        if (curve->params.numPoles > MAX_ORDER)
            {
            bool        code;

            bsputil_unWeightPoles (curve->poles, curve->poles, curve->weights,
                                   curve->params.numPoles);
            code = bound_rangeContainsPoints (index, numOut, curve->poles,
                                              curve->params.numPoles, range);
            bsputil_weightPoles (curve->poles, curve->poles, curve->weights,
                                 curve->params.numPoles);
            return code;
            }
        else
            {
            DPoint3d    local[MAX_ORDER];

            bsputil_unWeightPoles (local, curve->poles, curve->weights,
                                   curve->params.numPoles);
            return bound_rangeContainsPoints (index, numOut, local, curve->params.numPoles,
                                              range);
            }
        }
    else
        return bound_rangeContainsPoints (index, numOut, curve->poles,
                                          curve->params.numPoles, range);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    07/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_cylMinMax
(
double          *min,
double          *max,
DPoint3d        *testPt,
BoundCyl        *cyl
)
    {
    double      x, y, tmp0, tmp1;
    DPoint3d    diff, cross;

    /* Express testPt in coordinate system of cylinder's axis */
    bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, testPt, &cyl->org);
    x = bsiDPoint3d_dotProduct (&cyl->dir, &diff);
    bsiDPoint3d_crossProduct (&cross, &cyl->dir, &diff);
    y = bsiDPoint3d_magnitude (&cross);

    if (x < 0.0)
        {
        tmp0 = cyl->length - x;
        tmp1 = cyl->radius + y;
        *max = sqrt (tmp0*tmp0 + tmp1*tmp1);
        if (y <= cyl->radius)
            *min = -x;
        else
            {
            tmp1 = y - cyl->radius;
            *min = sqrt (x*x + tmp1*tmp1);
            }
        }
    else if (x > cyl->length)
        {
        tmp0 = x - cyl->length;
        tmp1 = cyl->radius + y;
        *max = sqrt (x*x + tmp1*tmp1);
        if (y <= cyl->radius)
            *min = tmp0;
        else
            {
            tmp1 = y - cyl->radius;
            *min = sqrt (tmp0*tmp0 + tmp1*tmp1);
            }
        }
    else
        {
        if (y <= cyl->radius)
            *min = *max = 0.0;
        else
            {
            *min = y - cyl->radius;
            tmp0 = cyl->length - x;
            tmp0 = tmp0 > x ? tmp0 : x;
            tmp1 = cyl->radius + y;
            *max = sqrt (tmp0*tmp0 + tmp1*tmp1);
            }
        return (true);
        }

    return (false);
    }

/*---------------------------------------------------------------------------------**//**
* Calculates the 4 distances resulting from offsetting the 2 given points by their radius in directions perpendicular to the supplied
* directions.
* @bsimethod                                                    Brian.Peters    07/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bound_calculateDistances
(
double          *distance,
DPoint3d        *p0,
DPoint3d        *dir0,
double          rad0,
DPoint3d        *p1,
DPoint3d        *dir1,
double          rad1
)
    {
    double      mag;
    DPoint3d    pt0, pt1, cross, diff, perp;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, p1, p0);
    pt1.x = bsiDPoint3d_dotProduct (dir0, &diff);
    bsiDPoint3d_crossProduct (&cross, dir0, &diff);
    pt1.y = bsiDPoint3d_magnitude (&cross);
    pt1.z = pt0.x = pt0.z = 0.0;

    bsiDPoint3d_crossProduct (&perp, dir1, &cross);
    if ((mag = bsiDPoint3d_normalizeInPlace (&perp)) < fc_epsilon)
        bsiDPoint3d_computeNormal (&perp, &pt1, &pt0);

    pt0.y = rad0;
    bsiDPoint3d_addScaledDPoint3d (&pt1, &pt1, &perp, rad1);
    distance[0] = bsiDPoint3d_distance (&pt0, &pt1);
    pt0.y = -rad0;
    distance[1] = bsiDPoint3d_distance (&pt0, &pt1);

    bsiDPoint3d_addScaledDPoint3d (&pt1, &pt1, &perp, -2.0*rad1);
    distance[2] = bsiDPoint3d_distance (&pt0, &pt1);
    pt0.y = rad0;
    distance[3] = bsiDPoint3d_distance (&pt0, &pt1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    07/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_cylPairDist
(
double          *minDist,
double          *maxDist,
BoundCyl        *cyl0,
BoundCyl        *cyl1
)
    {
    double      s, t, mag, magmag, dist = 0.0, rSum, endDist[16], searchMax, searchMin, *dP, *endP;
    DPoint3d    diff, tmp, mutualPerp, end0, end1;

    bsiDPoint3d_crossProduct (&mutualPerp, &cyl0->dir, &cyl1->dir);
    mag = bsiDPoint3d_magnitude (&mutualPerp);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, &cyl1->org, &cyl0->org);

    rSum = cyl0->radius + cyl1->radius;

    /* Check for parallel directions */
    if (mag < fc_epsilon)
        {
        dist = bsiDPoint3d_magnitude (&diff);
        *minDist = dist - rSum;
        *maxDist = dist + rSum;
        return (*minDist <= 0.0 ? true : false);
        }

    /* Calculate parameter along axes of closest approach */
    magmag = mag*mag;
    bsiDPoint3d_crossProduct (&tmp, &diff, &mutualPerp);
    t = -bsiDPoint3d_dotProduct (&cyl1->dir, &tmp) / magmag;
    s = -bsiDPoint3d_dotProduct (&cyl0->dir, &tmp) / magmag;

    /* Check if closest approach is from within both cylinders */
    if (t <= cyl0->length && s <= cyl1->length)
        dist = bsiDPoint3d_dotProduct (&diff, &mutualPerp) / mag - rSum;

    /* Check distances between ends of cylinders */
    bsiDPoint3d_addScaledDPoint3d (&end0, &cyl0->org, &cyl0->dir, cyl0->length);
    bsiDPoint3d_addScaledDPoint3d (&end1, &cyl1->org, &cyl1->dir, cyl1->length);
    bound_calculateDistances (endDist,
                              &cyl0->org, &cyl0->dir, cyl0->radius,
                              &cyl1->org, &cyl1->dir, cyl1->radius);
    bound_calculateDistances (endDist+4,
                              &end0, &cyl0->dir, cyl0->radius,
                              &cyl1->org, &cyl1->dir, cyl1->radius);
    bound_calculateDistances (endDist+8,
                              &cyl0->org, &cyl0->dir, cyl0->radius,
                              &end1, &cyl1->dir, cyl1->radius);
    bound_calculateDistances (endDist+12,
                              &end0, &cyl0->dir, cyl0->radius,
                              &end1, &cyl1->dir, cyl1->radius);

    searchMin = fc_hugeVal;
    searchMax = -1.0 ;

    for (dP=endP=endDist, endP+=16; dP < endP; dP++)
        {
        if (*dP < searchMin)    searchMin = *dP;
        if (*dP > searchMax)    searchMax = *dP;
        }

    *minDist = dist < searchMin ? dist : searchMin;
    *maxDist = searchMax;

    return (*minDist <= 0.0 ? true : false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    07/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_cylIntersectSegment
(
DPoint3d        *point,                /* <= intersect point, or NULL */
DSegment3d      *segment,              /* => endpoints of segment */
BoundCyl        *cyl
)
    {
    int         h0inRange, h1inRange;
    double      a, b, c, det, s0, s1, h0, h1, denom, dot, sdot;
    DRange2d   xSeg;
    DPoint3d    seg, seg_dir, diff, diff_dir, cross, end, intPt;

    /* transform segment into coordinate system of cylinder's axis */
    bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, &segment->point[0], &cyl->org);
    bsiDPoint3d_crossProduct (&cross, &diff, &cyl->dir);
    xSeg.low.x = bsiDPoint3d_dotProduct (&diff, &cyl->dir);
    xSeg.low.y = bsiDPoint3d_magnitude (&cross);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, &segment->point[1], &cyl->org);
    bsiDPoint3d_crossProduct (&cross, &diff, &cyl->dir);
    xSeg.high.x = bsiDPoint3d_dotProduct (&diff, &cyl->dir);
    xSeg.high.y = bsiDPoint3d_magnitude (&cross);

    /* Check if either end of segment is inside the cylinder */
    if ((xSeg.low.y <= cyl->radius && 0.0 <= xSeg.low.x && xSeg.low.x <= cyl->length) ||
        (xSeg.high.y <= cyl->radius && 0.0 <= xSeg.high.x && xSeg.high.x <= cyl->length))
        return (true);

    /* Check for intersection with cylinder walls. s is the parameter of the line
        defined by segment where it intersects the cylinder. h is the height of the
        intersection along the cylinder. */
    bsiDPoint3d_subtractDPoint3dDPoint3d (&seg, &segment->point[1], &segment->point[0]);
    bsiDPoint3d_crossProduct (&seg_dir, &seg, &cyl->dir);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, &segment->point[0], &cyl->org);
    bsiDPoint3d_crossProduct (&diff_dir, &diff, &cyl->dir);

    /* solve quadratic formula for s */
    a = bsiDPoint3d_dotProduct (&seg_dir, &seg_dir);
    b = 2.0 * bsiDPoint3d_dotProduct (&diff_dir, &seg_dir);
    c = bsiDPoint3d_dotProduct (&diff_dir, &diff_dir);
    dot = b / (2.0 * sqrt (a) * sqrt (c));
    sdot = 0.0;         // EDL Argh.  This was uninitizlized.  Should it be zero?    
    if (fabs (fabs (dot) - 1) < fc_nearZero)
        sdot = 1.0;

    h0inRange = h1inRange = false;
    det = fc_4 * a * (c * (sdot - 1.0) + cyl->radius * cyl->radius);
    denom = -2.0 * a;

    /* If det < 0.0 then imaginary roots so no solution.
       If denom == 0.0 then segment is parallel to cylinder's axis so no solution. */
    if (det >= 0.0 && fabs (denom) > fc_epsilon)
        {
        det = sqrt (det);

        s0 = (b+det)/denom;
        bsiDPoint3d_addScaledDPoint3d (&intPt, &segment->point[0], &seg, s0);
        bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, &intPt, &cyl->org);
        h0 = bsiDPoint3d_dotProduct (&cyl->dir, &diff);
        h0inRange = 0.0 <= h0 && h0 <= cyl->length;

        if (0.0 <= s0 && s0 <= 1.0 && h0inRange)
            {
            if (point)     *point = intPt;
            return (true);
            }

        s1 = (b-det)/denom;
        bsiDPoint3d_addScaledDPoint3d (&intPt, &segment->point[0], &seg, s1);
        bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, &intPt, &cyl->org);
        h1 = bsiDPoint3d_dotProduct (&cyl->dir, &diff);
        h1inRange = 0.0 <= h1 && h1 <= cyl->length;
        if (0.0 <= s1 && s1 <= 1.0 && h1inRange)
            {
            if (point)     *point = intPt;
            return (true);
            }

        /* Check for segment entirely within cylinder */
        if ((s0 * s1 < 0.0) && h0inRange && h1inRange)
            {
            if (point)     *point = segment->point[0];
            return (true);
            }
        }

    /* Check for intersection with cylinder end caps */
    denom = bsiDPoint3d_dotProduct (&cyl->dir, &seg);

    if (fabs (denom) > fc_epsilon)
        {
        bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, &cyl->org, &segment->point[0]);
        s0 = bsiDPoint3d_dotProduct (&cyl->dir, &diff) / denom;
        bsiDPoint3d_addScaledDPoint3d (&intPt, &segment->point[0], &seg, s0);
        if (0.0 <= s0 && s0 <= 1.0 &&
            bsiDPoint3d_distance (&intPt, &cyl->org) <= cyl->radius)
            {
            if (point)     *point = intPt;
            return (true);
            }

        bsiDPoint3d_addScaledDPoint3d (&end, &cyl->org, &cyl->dir, cyl->length);
        bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, &end, &segment->point[0]);
        s1 = bsiDPoint3d_dotProduct (&cyl->dir, &diff) / denom;
        bsiDPoint3d_addScaledDPoint3d (&intPt, &segment->point[0], &seg, s1);
        if (0.0 <= s1 && s1 <= 1.0 &&
            bsiDPoint3d_distance (&intPt, &end) <= cyl->radius)
            {
            if (point)     *point = intPt;
            return (true);
            }
        }

    return (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    07/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_cylIntersectRay
(
DPoint3d        *point,                /* <= intersect point, or NULL */
DPoint3d        *origin,               /* => origin of ray */
DPoint3d        *direction,            /* => endpoints of segment */
BoundCyl        *cyl,                  /* => cylinder */
bool            capped                 /* => cylinder is capped */
)
    {
    int         h0inRange, h1inRange;
    double      a, b, c, det, s0, s1, h0, h1, denom;
    DPoint3d    seg_dir, diff, diff_dir, end, intPt, originDiff;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&originDiff, origin, &cyl->org);

#if defined (POINT_INSIDE_CYLINDER_OPTIMIZATION)
    /* This test is valid for determining if a ray intersection is present,
       but it does not determine the intersection point or set *point correctly. */

    if (capped)
        {
        DPoint2d check;
        DPoint3d cross, seg;

        /* transform point into coordinate system of cylinder's axis */
        bsiDPoint3d_crossProduct (&cross, &originDiff, &cyl->dir);
        check.x = bsiDPoint3d_dotProduct (&originDiff, &cyl->dir);
        check.y = bsiDPoint3d_magnitude (&cross);
        /* Check if point is inside the cylinder */
        if (check.y <= cyl->radius && 0.0 <= check.x && check.x <= cyl->length)
            return true;
        }
#endif

    /* Check for intersection with cylinder walls. s is the distance along the ray
        defined by segment where it intersects the cylinder. h is the height of the
        intersection along the cylinder. */
    bsiDPoint3d_crossProduct (&seg_dir, direction, &cyl->dir);
    bsiDPoint3d_crossProduct (&diff_dir, &originDiff, &cyl->dir);

    /* solve quadratic formula for s */
    a = bsiDPoint3d_dotProduct (&seg_dir, &seg_dir);
    b = 2.0 * bsiDPoint3d_dotProduct (&diff_dir, &seg_dir);
    c = bsiDPoint3d_dotProduct (&diff_dir, &diff_dir) - cyl->radius * cyl->radius;

    det = b*b - fc_4*a*c;
    denom = -2.0 * a;

    /* If det < 0.0 then imaginary roots so no solution.
       If denom == 0.0 then segment is parallel to cylinder's axis so no solution. */
    if (det >= 0.0 && fabs (denom) > fc_epsilon)
        {
        det = sqrt (det);

        s0 = (b+det)/denom;
        bsiDPoint3d_addScaledDPoint3d (&intPt, origin, direction, s0);
        bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, &intPt, &cyl->org);
        h0 = bsiDPoint3d_dotProduct (&cyl->dir, &diff);
        h0inRange = 0.0 <= h0 && h0 <= cyl->length;

        if (0.0 <= s0 && h0inRange)
            {
            if (point)     *point = intPt;

            return (true);
            }

        s1 = (b-det)/denom;
        bsiDPoint3d_addScaledDPoint3d (&intPt, origin, direction, s1);
        bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, &intPt, &cyl->org);
        h1 = bsiDPoint3d_dotProduct (&cyl->dir, &diff);
        h1inRange = 0.0 <= h1 && h1 <= cyl->length;
        if (0.0 <= s1 && h1inRange)
            {
            if (point)     *point = intPt;
            return (true);
            }

        }

    if (capped)
        {
        /* Check for intersection with cylinder end caps */
        denom = bsiDPoint3d_dotProduct (&cyl->dir, direction);

        if (fabs (denom) > fc_epsilon)
            {
            s0 = - bsiDPoint3d_dotProduct (&cyl->dir, &originDiff) / denom;
            bsiDPoint3d_addScaledDPoint3d (&intPt, origin, direction, s0);
            if (0.0 <= s0 && bsiDPoint3d_distance (&intPt, &cyl->org) <= cyl->radius)
                {
                if (point)     *point = intPt;
                return (true);
                }

            bsiDPoint3d_addScaledDPoint3d (&end, &cyl->org, &cyl->dir, cyl->length);
            bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, &end, origin);
            s1 = bsiDPoint3d_dotProduct (&cyl->dir, &diff) / denom;
            bsiDPoint3d_addScaledDPoint3d (&intPt, origin, direction, s1);
            if (0.0 <= s1 && bsiDPoint3d_distance (&intPt, &end) <= cyl->radius)
                {
                if (point)     *point = intPt;
                return (true);
                }
            }
        }

    return (false);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    08/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bound_cylFlat
(
BoundCyl        *cyl,
double          tolerance
)
    {
    if (cyl->radius <= tolerance)
        return (true);

    if (cyl->radius < cyl->length / FLAT_TOL / FLAT_TOL)
        return (true);
    return (false);
    }

END_BENTLEY_GEOMETRY_NAMESPACE

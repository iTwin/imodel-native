/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/regions/rg_geom.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "Regions/rg_intern.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
*
* @param pLineParam <= 0, 1, or 2 intersection parameters on the line.
* @param pIntersectionPoint <= 0, 1, or 2 intersection points, computed from the
*               line start, end, and parameter.
* @param bounded => if true, only intersections in the parameter interval [0,1]
*                   are returned.  Note that the parameter interval is closed but
*                   not toleranced.
* @return number of intersections
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public  int      jmdlRG_intersectCircleSegmentXY
(
EmbeddedDoubleArray     *pParamArray,
EmbeddedDPoint3dArray   *pPointArray,
DPoint3d    *pStart,
DPoint3d    *pEnd,
DPoint3d    *pCenter,
double      r,
bool        bounded
)
    {
    DPoint3d vector0, vector1;
    DPoint3d point;
    double a0, a1, a2;
    double uu[2], vv[2];
    double rr = r * r;
    int i;
    int numRoot;
    int numOut = 0;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector0, pStart, pCenter);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector1, pEnd  , pCenter);

    a0 = bsiDPoint3d_dotProductXY (&vector0, &vector0) - rr;
    a1 = bsiDPoint3d_dotProductXY (&vector0, &vector1) - rr;
    a2 = bsiDPoint3d_dotProductXY (&vector1, &vector1) - rr;

    numRoot = bsiMath_solveConvexQuadratic (vv, uu, a0, 2.0 * a1, a2);
    if (pParamArray)
        jmdlEmbeddedDoubleArray_empty (pParamArray);

    if (pPointArray)
        jmdlEmbeddedDPoint3dArray_empty (pPointArray);

    for (i = 0; i < numRoot; i++)
        {
        if (!bounded || (uu[i] <= 1.0 && vv[i] <= 1.0))
            {
            numOut++;
            if (pParamArray)
                jmdlEmbeddedDoubleArray_addDouble (pParamArray, uu[i]);
            if (pPointArray)
                {
                bsiDPoint3d_add2ScaledDPoint3d (&point, NULL,
                                pStart, vv[i], pEnd, uu[i]);
                jmdlEmbeddedDPoint3dArray_addDPoint3d (pPointArray, &point);
                }
            }
        }
    return numOut;
    }
END_BENTLEY_GEOMETRY_NAMESPACE

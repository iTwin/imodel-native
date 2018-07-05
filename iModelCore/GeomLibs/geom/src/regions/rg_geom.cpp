/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/regions/rg_geom.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
bvector<double>         *pParamArray,
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

    vector0.DifferenceOf (*pStart, *pCenter);
    vector1.DifferenceOf (*pEnd, *pCenter);

    a0 = vector0.DotProductXY (vector0) - rr;
    a1 = vector0.DotProductXY (vector1) - rr;
    a2 = vector1.DotProductXY (vector1) - rr;

    numRoot = bsiMath_solveConvexQuadratic (vv, uu, a0, 2.0 * a1, a2);
    if (pParamArray)
        pParamArray->clear();

    if (pPointArray)
        jmdlEmbeddedDPoint3dArray_empty (pPointArray);

    for (i = 0; i < numRoot; i++)
        {
        if (!bounded || (uu[i] <= 1.0 && vv[i] <= 1.0))
            {
            numOut++;
            if (pParamArray)
                pParamArray->push_back (uu[i]);
            if (pPointArray)
                {
                point.SumOf(*pStart, vv[i], *pEnd, uu[i]);
                jmdlEmbeddedDPoint3dArray_addDPoint3d (pPointArray, &point);
                }
            }
        }
    return numOut;
    }
END_BENTLEY_GEOMETRY_NAMESPACE

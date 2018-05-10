/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/SolidPrimitive/sp_CurveIntersection.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "BoxFaces.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
@description Return the (weighted) control points of quadratic beziers which
   combine to represent the full conic section.

 @param pEllipse => ellipse to query
 @param pPoleArray <= array of poles of composite quadratic Bezier curve.  Array count depends on the angular extent.
 @param pCirclePoleArray <= array of corresponding poles which
            map the bezier polynomials back to the unit circle points (x,y,w)
            where x^2 + y^2 = w^2, but beware that w is not identically 1!  (Use this to map from bezier parameter back to angles.)
 @param pNumPole <= number of poles returned
 @param pNumSpan <= number of spans returned.  The pole indices for the first span are (always) 0,1,2; for the second span (if present),
                    2,3,4, and so on.
 @param maxPole => maximum number of poles desired.  maxPole must be at least
                5.  The circle is split into (maxPole - 1) / 2 spans.
                Beware that for 5 poles the circle is split into at most
                two spans, and there may be zero weights.   For 7 or more poles
                all weights can be positive.  The function may return fewer
                poles.
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
void     DEllipse3d_getQuadricBezierPoles

(
DEllipse3dCP pEllipse,
DPoint4dP pPoleArray,
DPoint3dP pCirclePoleArray,
int       *pNumPole,
int       *pNumSpan,
int       maxPole
)
    {
    DConic4d conic;
    bsiDConic4d_initFromDEllipse3d (&conic, pEllipse);
    bsiDConic4d_getQuadricBezierPoles (&conic, pPoleArray, pCirclePoleArray, pNumPole, pNumSpan, maxPole);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void bsiBezierDPoint4d_weightedInnerProduct(
double *productBezier,
int     &productOrder,
DPoint4dCR diagonalCoffs,
DPoint4dCP curveA,
DPoint4dCP curveB,
int curveOrder
)
    {
    productOrder = 2 * curveOrder - 1 ;
    for (int i = 0; i < productOrder; i++)
        productBezier[i] = 0.0;
    double termCoffs[MAX_BEZIER_ORDER];
    double scaleFactor[4];
    scaleFactor[0] = diagonalCoffs.x;
    scaleFactor[1] = diagonalCoffs.y;
    scaleFactor[2] = diagonalCoffs.z;
    scaleFactor[3] = diagonalCoffs.w;
    for (int axisIndex = 0; axisIndex < 4; axisIndex++)
        {
        bsiBezier_univariateProduct (termCoffs, 0, 1,
                (double*)curveA, curveOrder, axisIndex, 4,
                (double*)curveB, curveOrder, axisIndex, 4);
        for (int i =0; i < productOrder; i++)
            {
            productBezier[i] += scaleFactor[axisIndex] * termCoffs[i];
            }
        }
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool FractionalizeInCircle(double x, double y, double r, double &u, double &v)
    {
    double a = 2.0 * r;
    bool uStat = DoubleOps::SafeDivide (u, x + r, a, 0.0);
    bool vStat = DoubleOps::SafeDivide (v, y + r, a, 0.0);
    return uStat && vStat;
    }
    

/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct QuadraticArc
{
DEllipse3d m_arc;
DPoint4d m_arcBezCoffs[5];       //012 and 234 are quadratic bezier for arc segments.
DPoint3d m_unitCircleCoffs[5];    // (x/z,y/z) are u,v of point on unit circle.
int m_numPoles, m_numSpan;
QuadraticArc (DEllipse3dCR arc, double x0, double y0, double z0)
    {
    m_arc = arc;
    m_arc.center.x -= x0;
    m_arc.center.y -= y0;
    m_arc.center.z -= z0;
    m_arc.QuadricBezierPoles (m_arcBezCoffs, m_unitCircleCoffs, &m_numPoles, &m_numSpan, 5);
    }

// Form quartic as weighted inner product (x*x*diagonals.x + ...)
// Solve
// Convert back to angles.
int SolveDiagonalQuadric (DPoint4d diagonals, double *angleArray, int maxOut)
    {
    int numOut = 0;
    int curveOrder = 3;
    double bezCoffs[10];
    for (int i0 = 0; i0 + 2 < m_numPoles; i0 += curveOrder - 1)
        {
        int productOrder;
        bsiBezierDPoint4d_weightedInnerProduct (bezCoffs, productOrder,
                    diagonals,
                    &m_arcBezCoffs[i0],
                    &m_arcBezCoffs[i0],
                    curveOrder);
        double bezRoots[5];
        int numRoot = 0;
        if (bsiBezier_univariateRoots (bezRoots, &numRoot, bezCoffs, productOrder)
            && numRoot > 0)
            {
            DPoint3d xyzRoot[5];
            bsiBezierDPoint3d_evaluateArray (xyzRoot, NULL,
                    &m_unitCircleCoffs[i0],
                    curveOrder, bezRoots, numRoot);
            for (int i = 0; i < numRoot; i++)
                {
                if (numOut < maxOut)
                    angleArray[numOut++] = atan2 (xyzRoot[i].y, xyzRoot[i].x);
                }
            }        
        }
    return numOut;
    }
};


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool SolidLocationDetail::cb_compareLT_parameter
(
SolidLocationDetail const &dataA,
SolidLocationDetail const &dataB
)
    {
    return dataA.m_parameter < dataB.m_parameter;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void SortTail(bvector<SolidLocationDetail> &data, size_t i0)
    {
    std::sort (data.begin () + i0, data.end (), 
            SolidLocationDetail::cb_compareLT_parameter);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void FilterMinParameter(bvector<SolidLocationDetail> &pickData, size_t i0, double minParameter)
    {
    size_t newSize = i0;
    size_t oldSize = pickData.size ();
    for (size_t i = i0; i < oldSize; i++)
        {
        SolidLocationDetailR candidate = pickData.at(i);
        if (candidate.GetPickParameter () >= minParameter)
            {
            if (i > newSize)
                pickData[newSize] = candidate;
            newSize++;
            }
        }
    if (newSize < oldSize)
        pickData.resize (newSize);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void FilterMaxParameter(bvector<SolidLocationDetail> &pickData, size_t i0, double maxParameter)
    {
    size_t newSize = i0;
    size_t oldSize = pickData.size ();
    for (size_t i = i0; i < oldSize; i++)
        {
        SolidLocationDetailR candidate = pickData.at(i);
        if (candidate.GetPickParameter () <= maxParameter)
            {
            if (i > newSize)
                pickData[newSize] = candidate;
            newSize++;
            }
        }
    if (newSize < oldSize)
        pickData.resize (newSize);
    }



END_BENTLEY_GEOMETRY_NAMESPACE

#include "sp_rayIntersection.h"
#include "sp_CurvePrimitiveIntersection.h"



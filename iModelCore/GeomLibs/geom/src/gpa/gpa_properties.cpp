/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gpa_properties.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <stdlib.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct GPACurveProcessor
{
protected:
    GraphicsPointArrayCR m_sourceGPA;
    size_t m_readIndex;
    size_t m_subIndex;
    size_t m_subIntervalCount;
public:
GPACurveProcessor (GraphicsPointArrayCR sourceGPA);

bool GEOMDLLIMPEXP ProcessPrimitives ();
bool ProcessAt (size_t index);

GEOMAPI_VIRTUAL bool ProcessDSegment4d(DSegment4dCR segment);
GEOMAPI_VIRTUAL bool ProcessDConic4d(DConic4dCR conic);
GEOMAPI_VIRTUAL bool ProcessBezierDPoint4d(DPoint4dCP bezierPoles, int order, double knot0, double knot1);
GEOMAPI_VIRTUAL bool ProcessBsplineCurve();
GEOMAPI_VIRTUAL bool AnnounceMajorBreak();


};


// Curve Processor virtual defaults

GPACurveProcessor::GPACurveProcessor (GraphicsPointArrayCR sourceGPA)
    : m_sourceGPA(sourceGPA)
    {
    m_readIndex = m_subIndex = m_subIntervalCount = 0;
    }

// Default bspline processor breaks to beziers !!!
bool GPACurveProcessor::ProcessBsplineCurve ()
    {
    DPoint4d poleArray[MAX_BEZIER_ORDER];
    int      numPole;
    double knot0, knot1;
    bool isNullInterval;

    double knotA, knotB;
    size_t poleIndex0, poleIndex1;
    if (!m_sourceGPA.ParseBsplineCurveKnotDomain (m_readIndex, poleIndex0, poleIndex1, knotA, knotB))
        return false;
    m_subIntervalCount = poleIndex1 - poleIndex0 - 1;
    for (size_t interval = 0;
        m_sourceGPA.GetBezierSpanFromBsplineCurve (m_readIndex, interval, poleArray, numPole, MAX_BEZIER_ORDER,
                isNullInterval, knot0, knot1);
        interval++)
        {
        m_subIndex = interval;
        if (!isNullInterval)
            {
            m_subIndex = interval;
            if (!ProcessBezierDPoint4d (poleArray, numPole, knot0, knot1))
                return false;
            }
        }
    return true;
    }

bool GPACurveProcessor::ProcessDSegment4d (DSegment4dCR segment) {return true;}
bool GPACurveProcessor::ProcessDConic4d (DConic4dCR conic) {return true;}
bool GPACurveProcessor::ProcessBezierDPoint4d (DPoint4dCP bezierPoles, int order, double knot0, double knot1) {return true;}
bool GPACurveProcessor::AnnounceMajorBreak () {return true;}

bool GPACurveProcessor::ProcessAt (size_t index0)
    {
    GraphicsPoint gp;
    if (!m_sourceGPA.GetGraphicsPoint (index0, gp))
        return false;
    
    m_readIndex = index0;
    m_subIndex = 0;
    m_subIntervalCount = 0;
    int readIndex = (int) index0;
    int curveType = gp.GetCurveType ();
    DPoint4d poleArray[MAX_BEZIER_ORDER];
    int order;

    if (curveType == 0)
        {
        DSegment4d segment;
        if (jmdlGraphicsPointArray_getDSegment4d (&m_sourceGPA, &readIndex, &segment))
            {
            return ProcessDSegment4d (segment);
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
        {
        DConic4d conic;
        if (jmdlGraphicsPointArray_getDConic4d (&m_sourceGPA, &readIndex, &conic,
                                                    NULL, NULL, NULL, NULL))
            {
            return ProcessDConic4d (conic);
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER)
        {
        if (jmdlGraphicsPointArray_getBezier (&m_sourceGPA, &readIndex, poleArray, &order,
                                    MAX_BEZIER_ORDER))
            {
            return ProcessBezierDPoint4d (poleArray, order, 0.0, 1.0);
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
        {
        return ProcessBsplineCurve ();
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Process primitives from one GPA.
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
bool GPACurveProcessor::ProcessPrimitives ()
    {
    int index0, index1;
    int curveType;
    for (index0 = index1 = -1;
        jmdlGraphicsPointArray_parsePrimitiveAfter (&m_sourceGPA, &index0, &index1, NULL, NULL, &curveType, index1);
        )
        {
        if (!ProcessAt (index0))
            return false;
        if (jmdlGraphicsPointArray_isMajorBreak (&m_sourceGPA, index1))
            if (!AnnounceMajorBreak ())
                return false;
        }
    return true;
    }


// GPACurveProcessor for length accumulation.
struct GPA_LengthAccumulator : GPACurveProcessor
{
private:
double m_length;
public:
GPA_LengthAccumulator (GraphicsPointArrayCR source)
    : GPACurveProcessor (source)
    {
    m_length = 0.0;
    }

double GetCurveLength (){ return m_length;}

bool ProcessDSegment4d (DSegment4dCR segment) override
    {
    m_length += segment.point[0].RealDistance (segment.point[1]);
    return true;
    }
bool ProcessDConic4d (DConic4dCR conic) override
    {
    m_length += bsiDConic4d_arcLength (&conic);
    return true;
    }
bool ProcessBezierDPoint4d (DPoint4dCP bezierPoles, int order, double knot0, double knot1) override
    {
    m_length += bsiBezierDPoint4d_arcLength (bezierPoles, order, 0.0, 1.0);
    return true;
    }

};

// GPACurveProcessor for length accumulation.
struct GPA_FindHighestBezierOrder : GPACurveProcessor
{
private:
int m_maxOrder;
void TestOrder (int order)
    {
    if (order > m_maxOrder)
        m_maxOrder = order;
    }
public:
GPA_FindHighestBezierOrder (GraphicsPointArrayCR source)
    : GPACurveProcessor (source)
    {
    m_maxOrder = 0;
    }

int GetMaxOrder (){ return m_maxOrder;}

bool ProcessDSegment4d (DSegment4dCR segment) override
    {TestOrder (2); return true;}
bool ProcessDConic4d (DConic4dCR conic) override
    {TestOrder (3); return true;}
bool ProcessBezierDPoint4d (DPoint4dCP bezierPoles, int order, double knot0, double knot1) override
    {TestOrder(order); return true;}
bool ProcessBsplineCurve () override
    {
    GraphicsPoint gp;
    if (m_sourceGPA.GetGraphicsPoint (m_readIndex, gp))
        {
        TestOrder (gp.GetOrder ());
        return true;
        }
    return false;
    }

};

// GPACurveProcessor for length accumulation.
struct GPA_EvaluateAtFraction : GPACurveProcessor
{
private:
double m_fraction;
size_t m_numTermsRequested;
size_t m_numTermsGot;
DPoint4d m_xyzw[5];

DPoint3d m_xyz;
DVec3d   m_xyzDerivative[4];

bool NormalizeDerivatives (size_t numTermsGot)
    {
    m_numTermsGot = 0;
    if (numTermsGot == 1)
        {
        if (m_xyzw[0].GetProjectedXYZ (m_xyz))
            {
            m_numTermsGot = 1;
            return true;
            }
        }
    else if (numTermsGot == 2)
        {
        if (bsiDPoint4d_normalizePointAndDerivatives (
                &m_xyz,
                &m_xyzDerivative[0],
                NULL,
                &m_xyzw[0], &m_xyzw[1], NULL))
            {
            m_numTermsGot = 2;
            return true;
            }
        }
    else if (numTermsGot == 3)
        {
        if (bsiDPoint4d_normalizePointAndDerivatives (
                &m_xyz,
                &m_xyzDerivative[0],
                &m_xyzDerivative[1],
                &m_xyzw[0], &m_xyzw[1], &m_xyzw[2]))
            {
            m_numTermsGot = 3;
            return true;
            }
        }
    return false;
    }

bool NormalizeDerivatives (size_t numTermsGot, double a0, double a1)
    {
    if (NormalizeDerivatives (numTermsGot))
        {
        double intervalLength = a1 - a0;
        if (intervalLength == 1.0)
            {}
        else if (intervalLength == 0.0)
            {
            return false;
            }
        else
            {
            double a = 1.0 / intervalLength;
            double b = a;
            for (size_t i = 1; i < numTermsGot; i++)
                {
                m_xyzDerivative[i-1].Scale (b);
                b *= a;
                }
            }
        return true;
        }
    return false;
    }

public:
GPA_EvaluateAtFraction (GraphicsPointArrayCR source, int numTermsRequested, double fraction)
    : GPACurveProcessor (source)
    {
    m_fraction = fraction;
    m_numTermsRequested = numTermsRequested;
    if (numTermsRequested > 5)
        m_numTermsRequested = 5;
    if (numTermsRequested < 0)
        m_numTermsRequested = 1;
    m_numTermsGot = 0;
    }

void SetFraction (double fraction)
    {
    m_fraction = fraction;
    m_numTermsGot = 0;
    }
bool GetPoint (DPoint3dR xyz)
    {
    if (m_numTermsGot > 0)
        {
        xyz = m_xyz;
        return true;
        }
    xyz.Zero ();
    return false;
    }

bool GetDerivative1 (DVec3dR derivative1)
    {
    if (m_numTermsGot > 1)
        {
        derivative1 = m_xyzDerivative[0];
        return true;
        }
    derivative1.Zero ();
    return false;
    }

bool GetDerivative2 (DVec3dR derivative1)
    {
    if (m_numTermsGot > 2)
        {
        derivative1 = m_xyzDerivative[1];
        return true;
        }
    derivative1.Zero ();
    return false;
    }

bool GetSeriesTerm (DPoint4dR xyzw, size_t i)
    {
    if (i < m_numTermsGot)
        {
        xyzw = m_xyzw[i];
        return true;
        }
    xyzw.Zero ();
    return false;
    }


bool ProcessDSegment4d (DSegment4dCR segment) override
    {
    m_xyzw[1].DifferenceOf (segment.point[1], segment.point[0]);
    m_xyzw[0].SumOf (segment.point[0], m_xyzw[1], m_fraction);
    m_numTermsGot = 2;
    for (size_t i = 2; i < m_numTermsRequested; i++)
        {
        m_xyzw[i].Zero ();
        }
    return NormalizeDerivatives (m_numTermsRequested);
    }

bool ProcessDConic4d (DConic4dCR conic) override
    {
    bsiDConic4d_fractionParameterToDPoint4dDerivatives
            (
            &conic,
            &m_xyzw[0],
            m_numTermsRequested > 1 ? &m_xyzw[1] : NULL,
            m_numTermsRequested > 2 ? &m_xyzw[2] : NULL,
            m_fraction
            );
    return NormalizeDerivatives (m_numTermsRequested <= 3 ? m_numTermsRequested : 3);
    }

bool ProcessBezierDPoint4d (DPoint4dCP bezierPoles, int order, double knot0, double knot1) override
    {
    bsiBezierDPoint4d_evaluateDPoint4dArrayExt
                    (
                    &m_xyzw[0],
                    m_numTermsRequested > 1 ? &m_xyzw[1] : NULL,
                    m_numTermsRequested > 2 ? &m_xyzw[2] : NULL,
                    bezierPoles,
                    order,
                    &m_fraction, 1);
    return NormalizeDerivatives (m_numTermsRequested <= 3 ? m_numTermsRequested : 3, knot0, knot1);
    }

bool ProcessBsplineCurve () override
    {
    size_t i0, i1;
    double knot0, knot1;
    size_t bezierSelector;
    double bezierFraction;
    if (m_sourceGPA.ParseBsplineCurveKnotDomain (m_readIndex, i0, i1, knot0, knot1))
        {
        double knot = knot0 + m_fraction * (knot1 - knot0);
        DPoint4d bezierPoles[MAX_BEZIER_CURVE_ORDER];
        int order;
        bool isNullInterval;
        double knot0, knot1;
        if (m_sourceGPA.SearchBsplineInterval
                (m_readIndex, knot, true, bezierSelector, bezierFraction)
            && m_sourceGPA.GetBezierSpanFromBsplineCurve (m_readIndex, bezierSelector,
                    bezierPoles, order, MAX_BEZIER_CURVE_ORDER, isNullInterval,
                    knot0, knot1)
            )
            {
            bsiBezierDPoint4d_evaluateDPoint4dArrayExt
                    (
                    &m_xyzw[0],
                    m_numTermsRequested > 1 ? &m_xyzw[1] : NULL,
                    m_numTermsRequested > 2 ? &m_xyzw[2] : NULL,
                    bezierPoles,
                    order,
                    &bezierFraction, 1);
            return NormalizeDerivatives (m_numTermsRequested <= 3 ? m_numTermsRequested : 3);
            }
        }
    return false;
    }
};

GEOMDLLIMPEXP bool GraphicsPointArray::IsConnectedAndClosed(double absTol, double relTol) const
    {
    return jmdlGraphicsPointArray_isClosed (this, absTol, relTol) ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* Compute total length of the curves in the array.
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool GraphicsPointArray::PrimitiveFractionToDPoint3d (size_t primitiveIndex, double fraction, DPoint3dR xyz) const
    {
    GPA_EvaluateAtFraction accumulator (*this, 1, fraction);
    accumulator.ProcessAt (primitiveIndex);
    return accumulator.GetPoint (xyz);
    }

/*---------------------------------------------------------------------------------**//**
* Local frame evaluation
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool GraphicsPointArray::PrimitiveFractionToFrenetFrame
(
size_t           primitiveIndex,
double           fraction,
TransformR       transform,
DVec3dCP        defaultZ
) const
    {
    DVec3d unitX, unitY, unitZ;
    DPoint3d xyz;
    if (jmdlGraphicsPointArray_primitiveFractionToFrenetFrame (this, &xyz, &unitX, &unitY, &unitZ, NULL, (int)primitiveIndex, fraction, defaultZ))
        {
        transform.InitFromOriginAndVectors (xyz, unitX, unitY, unitZ);
        return true;
        }
    transform.InitIdentity ();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Local frame evaluation (at start point)
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool GraphicsPointArray::GetPlane
    (
    TransformR   transform,
    DRange3dP    localRange,
    double       *maxDeviation,
    DVec3dCP     defaultZ
    ) const
    {
    return jmdlGraphicsPointArray_getPlaneAsTransformExt2 (this, &transform, maxDeviation, defaultZ, localRange) ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* Multiple point evaluation
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool GraphicsPointArray::PrimitiveFractionArrayToDPoint3dDerivatives
(
size_t primitiveIndex,
double *pFractions,
size_t numFraction,
DPoint3dP xyz,
DVec3dP   dXYZ,
DVec3dP   ddXYZ
) const
    {
    int numTerm = 1;
    if (dXYZ != NULL)
        numTerm = 2;
    if (ddXYZ != NULL)
        numTerm = 3;

    GPA_EvaluateAtFraction accumulator (*this, numTerm, 0.0);
    for (size_t i = 0; i < numFraction; i++)
        {
        accumulator.SetFraction (pFractions[i]);
        if (accumulator.ProcessAt (primitiveIndex))
            {
            if (xyz != NULL)
                accumulator.GetPoint (xyz[i]);
            if (dXYZ != NULL)
                accumulator.GetDerivative1 (dXYZ[i]);
            if (ddXYZ != NULL)
                accumulator.GetDerivative2 (ddXYZ[i]);
            }
        else
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Multiple point evaluation
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool GraphicsPointArray::PrimitiveFractionArrayToDPoint4dDerivatives
(
size_t primitiveIndex,
double *pFractions,
size_t numFraction,
DPoint4dP   xyzw,
DPoint4dP   dXYZW,
DPoint4dP   ddXYZW
) const
    {
    int numTerm = 1;
    if (dXYZW != NULL)
        numTerm = 2;
    if (ddXYZW != NULL)
        numTerm = 3;

    GPA_EvaluateAtFraction accumulator (*this, numTerm, 0.0);
    for (size_t i = 0; i < numFraction; i++)
        {
        accumulator.SetFraction (pFractions[i]);
        if (accumulator.ProcessAt (primitiveIndex))
            {
            if (xyzw != NULL)
                accumulator.GetSeriesTerm (xyzw[i], 0);
            if (dXYZW != NULL)
                accumulator.GetSeriesTerm (dXYZW[i], 1);
            if (ddXYZW != NULL)
                accumulator.GetSeriesTerm (ddXYZW[i], 2);
            }
        else
            return false;
        }
    return true;
    }



GEOMDLLIMPEXP bool GraphicsPointArray::PrimitiveFractionToDPoint3d
(
size_t primitiveIndex,
double fraction,
DPoint3dR xyz,
DVec3dR   dXYZ
) const
    {
    GPA_EvaluateAtFraction accumulator (*this, 2, fraction);
    return accumulator.ProcessAt (primitiveIndex)
        && accumulator.GetPoint (xyz)
        && accumulator.GetDerivative1 (dXYZ);
    }

GEOMDLLIMPEXP bool GraphicsPointArray::PrimitiveFractionToDPoint3d
(
size_t primitiveIndex,
double fraction,
DPoint3dR xyz,
DVec3dR   dXYZ,
DVec3dR   ddXYZ
) const
    {
    GPA_EvaluateAtFraction accumulator (*this, 3, fraction);
    return accumulator.ProcessAt (primitiveIndex)
        && accumulator.GetPoint (xyz)
        && accumulator.GetDerivative1 (dXYZ)
        && accumulator.GetDerivative1 (ddXYZ);
    }

/*---------------------------------------------------------------------------------**//**
* Search for max curve order
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP int   GraphicsPointArray::HighestBezierOrder () const
    {
    GPA_FindHighestBezierOrder accumulator (*this);
    accumulator.ProcessPrimitives ();
    return accumulator.GetMaxOrder ();
    }


/*---------------------------------------------------------------------------------**//**
* Compute total length of a particular primitive in the array.
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP double   GraphicsPointArray::PrimitiveLength (size_t index) const
    {
    GPA_LengthAccumulator accumulator (*this);
    accumulator.ProcessAt (index);
    return accumulator.GetCurveLength ();
    }



/*---------------------------------------------------------------------------------**//**
* Compute total length of the curves in the array.
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP double   GraphicsPointArray::CurveLength () const
    {
    GPA_LengthAccumulator accumulator (*this);
    accumulator.ProcessPrimitives ();
    return accumulator.GetCurveLength ();
    }


static bool squaredDistanceBetween (GraphicsPointCR gp, DPoint3d xyz, bool xyOnly,
            double &squaredDistance)
    {
    return xyOnly
            ? gp.point.RealDistanceSquaredXY (&squaredDistance, xyz)
            : gp.point.RealDistanceSquared   (&squaredDistance, xyz);
    }

/*---------------------------------------------------------------------------------**//**
* Compute total length of the curves in the array.
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool GraphicsPointArray::ClosestVertex (DPoint3dCR spacePoint, bool xyOnly,
                GraphicsPointR gp, size_t &index) const
    {
    double currDistanceSquared, minDistanceSquared;
    size_t n = vbArray_hdr.size ();
    index = 0;
    gp.Zero ();
    if (n == 0)
        return false;
    size_t numPointsFound = 0;
    minDistanceSquared = DBL_MAX;
    for (size_t i = 0; i < n; i++)
        {
        GraphicsPoint gpi = vbArray_hdr[i];
        if (squaredDistanceBetween (gpi.point, spacePoint, xyOnly, currDistanceSquared)
            && (numPointsFound == 0 || currDistanceSquared < minDistanceSquared))
            {
            index = i;
            gp = gpi;
            minDistanceSquared = currDistanceSquared;
            numPointsFound++;
            }
        }

    return numPointsFound > 0;
    }

void GEOMDLLIMPEXP GraphicsPointArray::CopyFrom (GraphicsPointArrayCR source)
    {
    arrayMask = source.arrayMask;
    vbArray_hdr = source.vbArray_hdr;
    }

void GEOMDLLIMPEXP GraphicsPointArray::GetRange (DRange3dR range) const
    {
    range.Init ();
    jmdlGraphicsPointArray_extendDRange3d (this, &range);
    }
bool GEOMDLLIMPEXP GraphicsPointArray::IsEmpty () const {return vbArray_hdr.size () == 0;}

void GEOMDLLIMPEXP GraphicsPointArray::AppendFrom (GraphicsPointArrayCR source)
    {
    vbArray_hdr.insert (vbArray_hdr.end (), source.vbArray_hdr.begin (), source.vbArray_hdr.end ());
    }

void GEOMDLLIMPEXP GraphicsPointArray::AppendFrom (GraphicsPointArrayCR source, size_t i0, size_t i1)
    {
    if (i1 >= i0)
        {
        for (size_t i = i0; i <= i1; i++)
            vbArray_hdr.push_back (source.vbArray_hdr[i]);
        }
    else
        {
        for (size_t i = i0; i >= i1; i--)
            vbArray_hdr.push_back (source.vbArray_hdr[i]);
        }
    }


void GEOMDLLIMPEXP GraphicsPointArray::CopyReversedFrom (GraphicsPointArrayCR source)
    {
    // NEEDS WORK: COPY MAJOR BREAKS
    arrayMask = source.arrayMask;
    Clear ();
    GraphicsPointArray::Parser parser (&source);
    parser.ResetAtEnd ();
    while (parser.MoveToPreviousFragment ())
        {
        int curveType = parser.GetCurveType ();
        if (curveType == 0)
            {
            for (size_t i = parser.m_i1; i >= parser.m_i0; i--)
                {
                GraphicsPoint gp;
                source.GetGraphicsPoint (i, gp);
                gp.SetCurveBreak (i == parser.m_i0);
                vbArray_hdr.push_back (gp);
                if (i == 0)
                    break;
                }
            }
        else
            {
            // punt !!  Label data gets lost !!!
            jmdlGraphicsPointArray_appendInterval (this, &source,
                    (int)parser.m_i0, 1.0, (int)parser.m_i0, 0.0);
            }
        }
    }




typedef bool    (*BezierCurveSearchFunc)
    (
            double      *pParamArray,
            DPoint4d    *pPointArray,
            int         *pNumOut,
            int         maxOut,
    const   DPoint4d    *pPoleArray,
            int         order,
    const   DPoint4d    *pFixedPoint,
            int         workDimension,
            bool        extend
    );

typedef void (*VertexSearchFunc)
    (
            ProximityData *pProximityData,
    const   DPoint4d    *pVertex,
            int         primitiveIndex,
            double      primitiveFraction,
    const   DPoint4d    *pFixedPoint,
            int         workDimension
    );

static void cb_vertexProximityTest
(
        ProximityData *pProx,
const   DPoint4d    *pVertex,
        int         primitiveIndex,
        double      primitiveFraction,
const   DPoint4d    *pFixedPoint,
        int         workDimension
)
    {
    if (workDimension == 2)
        bsiProximityData_testXY (pProx, pVertex, primitiveFraction, primitiveIndex);
    else
        bsiProximityData_test   (pProx, pVertex, primitiveFraction, primitiveIndex);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
static     void    addRoots
(
        GraphicsPointArray  *pDest,
        int                     primitiveIndex,
const   DPoint4d                *pPoleArray,
        int                     order,
const   DPoint4d                *pRefPoint,
        int                     workDimension,
const   DConic4d                *pConic,
const   DPoint3d                *pTrigPoleArray,
        BezierCurveSearchFunc   bezierSearchFunc,
        bool                    extend,
        VertexSearchFunc        vertexSearchFunc,   /* Optionally pass start, end vertices
                                                        to this func */
        ProximityData           *pVertexData,
        bool                    testStart,
        bool                    testEnd,
        double                  a0 = 0.0,
        double                  a1 = 1.0
)
    {
    double *paramArray = (double*)BSIBaseGeom::Malloc (MAX_BEZIER_ORDER*sizeof (double));
    DPoint4d *pointArray = (DPoint4d*)BSIBaseGeom::Malloc (MAX_BEZIER_ORDER*sizeof (DPoint4d));
    DPoint3d *trigArray = (DPoint3d*)BSIBaseGeom::Malloc (MAX_BEZIER_ORDER*sizeof (DPoint3d));
    int numPerp = 0;
    int i;
    if (bezierSearchFunc
        && bezierSearchFunc (paramArray, pointArray, &numPerp, MAX_BEZIER_ORDER,
                    pPoleArray, order, pRefPoint, workDimension, extend))
        {

        if (pTrigPoleArray)
            bsiBezierDPoint3d_evaluateArray
                                (
                                trigArray, NULL,
                                pTrigPoleArray, order,
                                paramArray, numPerp
                                );

        for (i = 0; i < numPerp; i++)
            {
            double s, theta, fraction;
            DPoint4d finalPoint;
            s = paramArray[i];
            if (!pTrigPoleArray)
                {
                fraction = s;
                finalPoint = pointArray[i];
                }
            else
                {
                /* The point came from an conic.  Convert the bezier parameter
                        to angle, angle to fraction of conic,
                        and reevaluate the ellipse point (because the weighting
                        is different)
                */
                theta = atan2 (trigArray[i].y, trigArray[i].x);
                fraction = bsiDConic4d_angleParameterToFraction (pConic, theta);
                bsiDConic4d_angleParameterToDPoint4d (pConic, &finalPoint, theta);
                }

            jmdlGraphicsPointArray_addComplete (pDest,
                                finalPoint.x, finalPoint.y, finalPoint.z, finalPoint.w,
                                bsiTrig_interpolate (a0, fraction, a1),
                                0,primitiveIndex);
            jmdlGraphicsPointArray_markPoint (pDest);
            }
        }

    if (vertexSearchFunc)
        {
        if (testStart)
            vertexSearchFunc (pVertexData, &pPoleArray[0], primitiveIndex, a0, pRefPoint, workDimension);
        if (testEnd)
            vertexSearchFunc (pVertexData, &pPoleArray[order - 1], primitiveIndex, a1, pRefPoint, workDimension);
        }

    BSIBaseGeom::Free (trigArray);
    BSIBaseGeom::Free (pointArray);
    BSIBaseGeom::Free (paramArray);
    }

/*---------------------------------------------------------------------------------**//**
* Drop perpendiculars to all curves in the array.
* @param pDest <= array to receive perpendicular points (with primitive and parameter in
*                       source array).
* @param pSource => array of path geoemtry.
* @param pPoint => space point to be projected.  Note that a point with weight 0 is valid --
*           the points computed are the extrema of the geometry projected onto the direction
*           in the xyz part of the point.
* @param workDimension => 2 for xyw parts only, 3 for xyzw.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
static  void    jmdlGraphicsPointArray_addRootsPerPrimitive
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const   DPoint4d                *pPoint,
        int                     workDimension,
        BezierCurveSearchFunc   bezierSearchFunc,
        bool                    extend,
        VertexSearchFunc        vertexSearchFunc,
        ProximityData            *pVertexData
)
    {
    int curr0, curr1;
    int readIndex;
    int curveType;


    for (curr1 = -1;
        jmdlGraphicsPointArray_parsePrimitiveAfter (pSource, &curr0, &curr1, NULL, NULL, &curveType, curr1);
        )
        {
        if (curveType == 0)
            {
            DSegment4d segment;
            DPoint4d poleArray[2];
            if (readIndex = curr0,
                jmdlGraphicsPointArray_getDSegment4d (pSource, &readIndex, &segment))
                {
                poleArray[0] = segment.point[0];
                poleArray[1] = segment.point[1];
                if (bezierSearchFunc)
                    addRoots
                        (pDest, curr0, poleArray, 2, pPoint, workDimension,
                        NULL, NULL, bezierSearchFunc, extend,
                        vertexSearchFunc, pVertexData, true, true);
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            DConic4d conic, extendedConic;
            if (readIndex = curr0,
                jmdlGraphicsPointArray_getDConic4d (pSource, &readIndex, &conic,
                                                        NULL, NULL, NULL, NULL))
                {
                DPoint4d conicPoleArray[5];
                DPoint3d circlePoleArray[5];
                bool    testStart, testEnd;
                int numPole, numSpan;
                int i;
                extendedConic = conic;

            /* If extended geometry is requested, generate the bezier approximation
                over the whole circle.   However, pass only the original (partial)
                conic to addRoots, so it computes angular fractions relative to the
                trimmed range. */
                if (extend)
                    bsiDConic4d_makeFullSweep (&extendedConic);
                bsiDConic4d_getQuadricBezierPoles (&extendedConic,
                                    conicPoleArray,
                                    circlePoleArray,
                                    &numPole, &numSpan, 5);
                for (i = 0; i < numSpan; i++)
                    {
                    int k = 2 * i;
                    testStart = (i == 0);
                    testEnd   = (i == numSpan - 1);
                    addRoots (pDest, curr0, conicPoleArray + k, 3, pPoint, workDimension,
                                        &conic, circlePoleArray + k, bezierSearchFunc, false,
                                        vertexSearchFunc, pVertexData, testStart, testEnd
                              );
                    }
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
            {
            DPoint4d poleArray[MAX_BEZIER_ORDER];
            int order;
            if (readIndex = curr0,
                jmdlGraphicsPointArray_getBezier (pSource, &readIndex, poleArray, &order,
                                        MAX_BEZIER_ORDER))
                {
                addRoots (pDest, curr0, poleArray, order, pPoint, workDimension,
                                            NULL, NULL, bezierSearchFunc, false,
                                            vertexSearchFunc, pVertexData, true, true);
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
            DPoint4d poleArray[MAX_BEZIER_ORDER];
            int order;
            bool isNullInterval;
            double knot0, knot1;
            for (size_t spanIndex = 0;
                pSource->GetBezierSpanFromBsplineCurve (curr0, spanIndex, poleArray, order,
                        MAX_BEZIER_ORDER, isNullInterval, knot0, knot1); spanIndex++)
                {
                if (!isNullInterval)
                    addRoots (pDest, curr0, poleArray, order, pPoint, workDimension,
                                            NULL, NULL, bezierSearchFunc, false,
                                            vertexSearchFunc, pVertexData, true, true, knot0, knot1);
                }
            }

        }
    }



/*---------------------------------------------------------------------------------**//**
* Drop perpendiculars to all curves in the array.
* @param pDest <= array to receive perpendicular points (with primitive and parameter in
*                       source array).
* @param pSource => array of path geoemtry.
* @param pPoint => space point to be projected.  Note that a point with weight 0 is valid --
*           the points computed are the extrema of the geometry projected onto the direction
*           in the xyz part of the point.
* @param workDimension => 2 for xyw parts only, 3 for xyzw.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addPerpendicularsFromDPoint4d
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const   DPoint4d                *pPoint,
        int                     workDimension
)
    {
    jmdlGraphicsPointArray_addRootsPerPrimitive (pDest, pSource, pPoint, workDimension,
                        bsiBezierDPoint4d_allPerpendicularsFromDPoint4dExt, false,
                        NULL, NULL);
    }





// GPACurveProcessor for length accumulation.
struct GPA_RootSearch : GPACurveProcessor
{
private:
GraphicsPointArrayP m_rootArray;
DPoint4d m_spacePoint;
bool     m_xyOnly;
ProximityData m_rootProximity;
ProximityData m_endPointProximity;
bool     m_doEndpoints;
bool     m_extend;

BezierCurveSearchFunc   m_bezierSearchFunc;
VertexSearchFunc        m_vertexSearchFunc;


public:
GPA_RootSearch (
    GraphicsPointArrayCR source,
    GraphicsPointArrayP  rootArray,  // MUST be non null !!!
    DPoint3dCR spacePoint, bool doEndpoints,
    bool extend,
    BezierCurveSearchFunc   bezierSearchFunc,
    VertexSearchFunc        vertexSearchFunc
    )
    : GPACurveProcessor (source)
    {
    m_rootArray = rootArray;
    m_doEndpoints = doEndpoints;
    m_spacePoint.Init (spacePoint, 1.0);
    bsiProximityData_init (&m_rootProximity, &spacePoint, -1, 0.0);
    bsiProximityData_init (&m_endPointProximity, &spacePoint, -1, 0.0);
    m_extend      = extend;
    m_bezierSearchFunc = bezierSearchFunc;
    m_vertexSearchFunc = vertexSearchFunc;
    }

// Test proximity to each root in the destination array.
// Accumulate in m_rootProximity.
void TestRootProximity ()
    {
    GraphicsPoint gp;
    for (size_t i = 0; m_rootArray->GetGraphicsPoint (i, gp); i++)
        {
        if (m_xyOnly)
            bsiProximityData_testXY (&m_rootProximity, &gp.point, gp.a, gp.userData);
        else
            bsiProximityData_test (&m_rootProximity, &gp.point, gp.a, gp.userData);
        }
    }

ProximityData GetRootProximity () { return m_rootProximity;}
ProximityData GetEndPointProximity () { return m_endPointProximity;}

bool ProcessDSegment4d (DSegment4dCR segment) override
    {
    addRoots
        (m_rootArray, (int)m_readIndex, segment.point, 2, &m_spacePoint, m_xyOnly ? 2 : 3,
                    NULL, NULL, m_bezierSearchFunc, m_extend ? true : false,
                    m_vertexSearchFunc, &m_endPointProximity, true, true);
    return true;
    }

bool ProcessDConic4d (DConic4dCR conic) override
    {
    DPoint4d conicPoleArray[5];
    DPoint3d circlePoleArray[5];
    DConic4d extendedConic;
    bool    testStart, testEnd;
    int numPole, numSpan;
    int i;
    extendedConic = conic;

    /* If extended geometry is requested, generate the bezier approximation
        over the whole circle.   However, pass only the original (partial)
        conic to addRoots, so it computes angular fractions relative to the
        trimmed range. */
    if (m_extend)
        bsiDConic4d_makeFullSweep (&extendedConic);
    bsiDConic4d_getQuadricBezierPoles (&extendedConic,
                        conicPoleArray,
                        circlePoleArray,
                        &numPole, &numSpan, 5);
    for (i = 0; i < numSpan; i++)
        {
        int k = 2 * i;
        testStart = (i == 0);
        testEnd   = (i == numSpan - 1);
        addRoots 
            (m_rootArray, (int)m_readIndex, conicPoleArray + k, 3, &m_spacePoint, m_xyOnly ? 2 : 3,
                            &conic, circlePoleArray + k, m_bezierSearchFunc, m_extend ? true : false,
                            m_vertexSearchFunc, &m_endPointProximity, testStart, testEnd);
        }
    return true;
    }

bool ProcessBezierDPoint4d (DPoint4dCP bezierPoles, int order, double knot0, double knot1) override
    {
    addRoots
        (m_rootArray, (int)m_readIndex, bezierPoles, order, &m_spacePoint, m_xyOnly ? 2 : 3,
                    NULL, NULL, m_bezierSearchFunc, m_extend ? true : false,
                    m_vertexSearchFunc, &m_endPointProximity, m_subIndex == 0, m_subIndex == m_subIntervalCount - 1,
                    knot0, knot1);
    return true;
    }
};



//! @param [out] perpendicularPointData Data for closest true perpendicular.
//! @param [out] endPointData Optional endpoint test data.
//! @param [in] spacePoint base point of distance measurement.
//! @param [in] xyOnly true to do distance calculations with only xy.
//! @param [in] extend true to extend lines, arcs.
bool GEOMDLLIMPEXP GraphicsPointArray::ClosestPointOnCurve (
                ProximityDataR          perpendicularPointData,
                ProximityDataP          endPointData,
                DPoint3dCR              point,
                bool                    xyOnly,
                bool                    extend
                )
    {
    GraphicsPointArray   *pWorkArray = jmdlGraphicsPointArray_grab ();

    GPA_RootSearch searcher (*this, pWorkArray, point, xyOnly, extend, bsiBezierDPoint4d_allPerpendicularsFromDPoint4dExt,
            endPointData != NULL ? cb_vertexProximityTest : NULL
            );
    static bool sUseNew = false;
    bool newStat = (sUseNew ? searcher.ProcessPrimitives () : false);
    bool stat = false;
    if (newStat)
        {
        searcher.TestRootProximity ();
        perpendicularPointData = searcher.GetRootProximity ();
        if (endPointData != NULL && endPointData != &perpendicularPointData)
            *endPointData = searcher.GetEndPointProximity ();
        stat = (0 != perpendicularPointData.dataValid)
             || ((NULL != endPointData) && (0 != endPointData->dataValid));
        }
    else
        {
        if (jmdlGraphicsPointArray_closestPointExt (this, &perpendicularPointData, endPointData, &point,
                    xyOnly ? 2 : 3, extend ? true : false))
            {
            stat = true;
            }
        }
    jmdlGraphicsPointArray_drop (pWorkArray);
    return stat;
    }



/*---------------------------------------------------------------------------------**//**
* Drop perpendiculars to all curves in the array.  Search for the closest.
* @param pSource => array of path geoemtry.
* @param pProx <= proximity data for closest point.
*               Contains point, primitive index, and paramter of closest point.
* @param pPoint => space point to be projected.
* @param workDimension => 2 for xyw parts only, 3 for xyzw.
* @param extend => true to test on unbounded geometry
* @param testEndPoints => true to also test at endpoints.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_closestPointExt
(
GraphicsPointArrayCP pSource,
        ProximityData           *pProx,
        ProximityData           *pEndPointProx,
const   DPoint3d                *pPoint,
        int                     workDimension,
        bool                    extend
)
    {
    GraphicsPointArray   *pDest = jmdlGraphicsPointArray_grab ();
    int i;
    GraphicsPoint gp;
    //bool    boolstat = false;
    DPoint4d hPoint;

    bsiDPoint4d_initFromDPoint3dAndWeight (&hPoint, pPoint, 1.0);

    if (pProx)
        bsiProximityData_init (pProx, pPoint, -1, 0.0);

    if (pEndPointProx)
        bsiProximityData_init (pEndPointProx, pPoint, -1, 0.0);

    jmdlGraphicsPointArray_addRootsPerPrimitive (pDest, pSource, &hPoint, workDimension,
                        bsiBezierDPoint4d_allPerpendicularsFromDPoint4dExt,  extend,
                        pEndPointProx ? cb_vertexProximityTest : NULL, pEndPointProx);

    for (i = 0; jmdlGraphicsPointArray_getGraphicsPoint (pDest, &gp, i); i++)
        {
        if (workDimension == 2)
            bsiProximityData_testXY (pProx, &gp.point, gp.a, gp.userData);
        else
            bsiProximityData_test (pProx, &gp.point, gp.a, gp.userData);
        }

    jmdlGraphicsPointArray_drop (pDest);
    return (pProx && pProx->dataValid) || (pEndPointProx && pEndPointProx->dataValid);
    }




/*---------------------------------------------------------------------------------**//**
* Drop perpendiculars to all curves in the array.  Search for the closest.
* @param pSource => array of path geoemtry.
* @param pProx <= proximity data for closest point.
*               Contains point, primitive index, and paramter of closest point.
* @param pPoint => space point to be projected.
* @param workDimension => 2 for xyw parts only, 3 for xyzw.
* @param extend => true to test on unbounded geometry
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_closestPoint
(
GraphicsPointArrayCP pSource,
        ProximityData           *pProx,
const   DPoint3d                *pPoint,
        int                     workDimension,
        bool                    extend
)
    {
    return jmdlGraphicsPointArray_closestPointExt (pSource, pProx, NULL, pPoint, workDimension, extend);
    }






/*---------------------------------------------------------------------------------**//**
* Drop tangents to all curves in the array.
* @param pDest <= array to receive perpendicular points (with primitive and parameter in
*                       source array).
* @param pSource => array of path geoemtry.
* @param pPoint => space point to be projected.  Note that a point with weight 0 is valid --
*           the points computed are the extrema of the geometry projected onto the direction
*           in the xyz part of the point.
* @param workDimension => 2 for xyw parts only, 3 for xyzw.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addTangentsFromDPoint4d
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const   DPoint4d                *pPoint,
        int                     workDimension
)
    {
    jmdlGraphicsPointArray_addRootsPerPrimitive (pDest, pSource, pPoint, workDimension,
                        bsiBezierDPoint4d_allTangentsFromDPoint4dExt, false,
                        NULL, NULL);

    }


/*---------------------------------------------------------------------------------**//**
* Drop perpendiculars to all curves in the array.
* @param pDest <= array to receive perpendicular points (with primitive and parameter in
*                       source array).
* @param pSource => array of path geoemtry.
* @param pPoint => space point to be projected.  Note that a point with weight 0 is valid --
*           the points computed are the extrema of the geometry projected onto the direction
*           in the xyz part of the point.
* @param workDimension => 2 for xyw parts only, 3 for xyzw.
* @param extend => true to use unbounded (extended) geometry when possible.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addPerpendicularsFromDPoint4dExt
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const   DPoint4d                *pPoint,
        int                     workDimension,
        bool                    extend
)
    {
    jmdlGraphicsPointArray_addRootsPerPrimitive (pDest, pSource, pPoint, workDimension,
                        bsiBezierDPoint4d_allPerpendicularsFromDPoint4dExt, extend,
                        NULL, NULL);
    }


/*---------------------------------------------------------------------------------**//**
* Compute points of simple intersection with a plane.  Accumulate points
* in destination.
* @param pDest <= array to receive intersection points (with primitive and parameter in
*                       source array).
* @param pSource => array of path geoemtry.
* @param pPlaneCoffs => homogeneous coefficients of plane.
* @param extend => true to use unbounded (extended) geometry when possible.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDPlane4dIntersectionPoints
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const   DPoint4d                *pPlaneCoffs,
        bool                    extend
)
    {
    jmdlGraphicsPointArray_addRootsPerPrimitive (pDest, pSource, pPlaneCoffs,
                        3,
                        bsiBezierDPoint4d_allDPlane4dIntersections, extend, NULL, NULL);
    }


/*---------------------------------------------------------------------------------**//**
* Compute points of simple intersection with a plane.  Accumulate points
* in destination.
* @param pDest <= array to receive intersection points (with primitive and parameter in
*                       source array).
* @param pSource => array of path geoemtry.
* @param pPlane => point-origin form of plane.
* @param extend => true to use unbounded (extended) geometry when possible.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDPlane3dIntersectionPoints
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const   DPlane3d                *pPlane,
        bool                    extend
)
    {
    DPoint4d planeCoffs;
    bsiDPlane3d_getDPoint4d (pPlane, &planeCoffs);
    jmdlGraphicsPointArray_addRootsPerPrimitive (pDest, pSource, &planeCoffs,
                        3,
                        bsiBezierDPoint4d_allDPlane4dIntersections, extend, NULL, NULL);
    }


/*---------------------------------------------------------------------------------**//**
* Drop tangents to all curves in the array.
* @param pDest <= array to receive perpendicular points (with primitive and parameter in
*                       source array).
* @param pSource => array of path geoemtry.
* @param pPoint => space point to be projected.  Note that a point with weight 0 is valid --
*           the points computed are the extrema of the geometry projected onto the direction
*           in the xyz part of the point.
* @param workDimension => 2 for xyw parts only, 3 for xyzw.
* @param extend => true to use unbounded (extended) geometry when possible.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addTangentsFromDPoint4dExt
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const   DPoint4d                *pPoint,
        int                     workDimension,
        bool                    extend
)
    {
    jmdlGraphicsPointArray_addRootsPerPrimitive (pDest, pSource, pPoint, workDimension,
                        bsiBezierDPoint4d_allTangentsFromDPoint4dExt, extend, NULL, NULL);
    }


GEOMDLLIMPEXP void GraphicsPointArray::AddSpacedPoints
(
GraphicsPointArrayCR path,
bvector<double>&distanceArray,
bool        storeTangent
)
    {
    jmdlGraphicsPointArray_addSpacedPoints (this, &path,
                                &distanceArray[0], (int)distanceArray.size (), 0.0, 0.0, storeTangent);
    }

//! @description
//! @param [out] pointsOnA intersection points on curves in curveA
//! @param [out] pointsOnB intersection points on curves in curveB
//! @param [in] curveA first curve set
//! @param [in] curveB second curve set
//! @param [in] extendLines true to extend lines
//! @param [in] extendConics true to extend conics
void GEOMDLLIMPEXP GraphicsPointArray::XYIntersections
(
GraphicsPointArrayR pointsOnA,
GraphicsPointArrayR pointsOnB,
GraphicsPointArrayCR curveA,
GraphicsPointArrayCR curveB,
bool extendLines,
bool extendConics
)
    {
    jmdlGraphicsPointArray_xyIntersectionPointsExt (&pointsOnA, &pointsOnB, &curveA, &curveB, extendLines ? true : false, extendConics ? true : false);
    }


//! @description Find points where a discontinuity point (end point or interior sharp angle) of curveA are within tolerance of curveB.
//! @param [out] pointsOnA intersection points on curves in curveA
//! @param [out] pointsOnB intersection points on curves in curveB
//! @param [in] curveA curves - discontinuities
//! @param [in] curveB second curve set
//! @param [in] extendLines true to extend lines
//! @param [in] extendConics true to extend conics
void GEOMDLLIMPEXP GraphicsPointArray::AddNearContactAtDiscontinuities
(
GraphicsPointArrayR pointsOnA,
GraphicsPointArrayR pointsOnB,
GraphicsPointArrayCR curveA,
GraphicsPointArrayCR curveB,
double tolerance
)
    {
    jmdlGraphicsPointArray_addDiscontinuityContacts (&pointsOnA, &pointsOnB, &curveA, &curveB, tolerance);
    }

///! Compute the centroid of the curves as a path (thin wire) -- not as an area.
///! @param [out] centroid computed centroid.
///! @param [out] arcLength computed arc lenght
///! @param [in] computeAtFixedZ true to enable xy-plane calculations.
///! @param [in] fixedZ z value for computeAtFixedZ
bool GEOMDLLIMPEXP GraphicsPointArray::GetPathCentroid
(
DPoint3dR centroid,
double &arcLength,
bool   computeAtFixedZ,
double fixedZ
) const
    {
    return jmdlGraphicsPointArray_getPathCentroidExt (this, &centroid, &arcLength, computeAtFixedZ, fixedZ) ? true : false;
    }


//! @description Test if two arrays are have identical curve type and point coordinates.
//! @param [in] other second array of comparison.
//! @param [in] xyzAbsTol absolute tolerance for comparisons.
//! @param [in] relTol relative tolerance.
GEOMDLLIMPEXP bool GraphicsPointArray::IsSameGeometryPointByPoint
(
GraphicsPointArrayCR other,
double xyzAbsTol,
double relTol
) const
    {
    return jmdlGraphicsPointArray_sameGeometryPointByPoint (this, &other, xyzAbsTol, relTol) ? true : false;
    }

//! @description Compute a point at specified distance from start, measuring along the true curve.
//! @param [in] target distance distance along curve
//! @param [out] primitiveIndex returned curve primitive index
//! @param [out] fraction fractional parameter along the primitive
//! @param [out] point point on curve.
GEOMDLLIMPEXP bool GraphicsPointArray::GetPointAtDistanceFromStart
(
double distance,
size_t &primitiveIndex,
double &fraction,
DPoint3dR point
) const
    {
    int prim;
    bool stat = jmdlGraphicsPointArray_primitiveFractionAtDistance (
                const_cast <GraphicsPointArrayP> (this),
                &prim, &fraction, &point, distance
                ) ? true : false;
    primitiveIndex = prim;
    return stat;
    }

//! @description Generate an offset of the lines and curves.
//! @param [in] arcAngle => If the turning angle at a vertex exceeds this angle (radians),
//!                       an arc is created.   Passing a negative angle                   means no arcs.
//! @param [in] chamferAngle => If the turning angle at a vertex is smaller than
//!                       arc angle but larger than chamfer angle, a chamfer is
//!                       created.   This angle is always forced to .99pi or less.
//! @param [in] offsetDistance the offset distance (uniform).  Positive is to
//!                           left of curve relative to the normal.
//! @param [in] normal optional normal to plane of offset measurement.  Default is Z vector.
GEOMDLLIMPEXP void     GraphicsPointArray::AddOffsetCurves
(
GraphicsPointArrayCR source,
double offsetDistance,
DVec3dCP planeNormal,
double arcAngle,
double chamferAngle
)
    {
    jmdlGraphicsPointArray_collectOffset (this, &source, arcAngle, chamferAngle, offsetDistance, planeNormal);
    }


//! @description compute simple (point) intersections with a plane.
//! @param [in] source candidate intersection geometry.
//! @param [in] plane 
//! @param [in] extend true to extend lines and arcs
GEOMDLLIMPEXP void GraphicsPointArray::AddPlaneIntersectionPoints
(
GraphicsPointArrayCR    source,
DPlane3dCR              plane,
bool                    extend
)
    {
    jmdlGraphicsPointArray_addDPlane3dIntersectionPoints (this, &source, &plane, extend);
    }

//! @description compute line segments of intersections with a plane, using parity rules to determine in and out.
//!    Intersection edges are added to the instance array in start-end pairs.
//! @param [in] boundary boundary geometry
//! @param [in] plane 
GEOMDLLIMPEXP void GraphicsPointArray::AddPlaneIntersectionEdges
(
GraphicsPointArrayCR    boundary,
DPlane3dCR              plane
)
    {
    jmdlGraphicsPointArray_intersectDPlane3d (this, &boundary, &plane);
    }

END_BENTLEY_GEOMETRY_NAMESPACE

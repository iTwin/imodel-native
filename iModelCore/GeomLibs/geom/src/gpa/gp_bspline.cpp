/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_bspline.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
#include "ArrayWrapper.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//static const int s_allCurveBits = HPOINT_MASK_CURVETYPE_BITS | HPOINT_MASK_POINTTYPE_BITS;
static const int s_bsplineEndMask = HPOINT_MASK_CURVETYPE_BSPLINE | HPOINT_MASK_BSPLINE_STARTEND;
static const int s_bsplinePoleMask = HPOINT_MASK_CURVETYPE_BSPLINE | HPOINT_MASK_BSPLINE_POLE;
static const int s_bsplineExtraPoleMask = HPOINT_MASK_CURVETYPE_BSPLINE | HPOINT_MASK_BSPLINE_EXTRA_POLE;

#define IS_BSPLINE_ENDPOINT_MASK(_mask_)    \
            (((_mask_) & s_allCurveBits) == s_bezierEndMask)

#define IS_BSPLINE_POLE_MASK(_mask_)    \
            (((_mask_) & s_allCurveBits) == s_bezierPoleMask)

#define IS_BSPLINE_EXTRA_POLE_MASK(_mask_)    \
            (((_mask_) & s_allCurveBits) == s_bezierExtraPoleMask)

GEOMDLLIMPEXP GraphicsPointArray::GraphicsPointArray () { }
size_t GEOMDLLIMPEXP GraphicsPointArray::GetGraphicsPointCount () const
    {
    return vbArray_hdr.size ();
    }

size_t GEOMDLLIMPEXP GraphicsPointArray::GetLastIndex () const
    {
    return vbArray_hdr.size () - 1; // hmmm.. count on wraparound SIZE_MAX if 0?
    }

bool GEOMDLLIMPEXP GraphicsPointArray::IsValidIndex (size_t i) const
    {
    return i < vbArray_hdr.size ();
    }


bool GEOMDLLIMPEXP GraphicsPointArray::GetGraphicsPoint (size_t i, GraphicsPointR gp) const
    {
    if (i >= vbArray_hdr.size ())
        {
        memset (&gp, 0, sizeof (gp));
        return false;
        }
    gp = vbArray_hdr[i];
    return true;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::GetDPoint4d (size_t i, DPoint4dR xyzw) const
    {
    if (i >= vbArray_hdr.size ())
        {
        memset (&xyzw, 0, sizeof (xyzw));
        return false;
        }
    xyzw = vbArray_hdr[i].point;
    return true;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::GetNormalizedPoint (size_t i, DPoint3dR xyz) const
    {
    if (i >= vbArray_hdr.size ())
        {
        xyz.Zero ();
        return false;
        }
    return vbArray_hdr[i].point.GetProjectedXYZ (xyz);
    }


static size_t reversedIndex (size_t i, size_t n)
    {
    return n - i - 1;
    }


bool GEOMDLLIMPEXP GraphicsPointArray::AddBsplineCurve
(
MSBsplineCurveCR curve, bool splinesAsBezier
)
    {
    if (splinesAsBezier)
        {
        jmdlGraphicsPointArray_addBsplineCurve (this, &curve);
        return true;
        }

    return AddAsCompleteBsplineCurve (curve);
    }

bool GEOMDLLIMPEXP GraphicsPointArray::AddAsCompleteBsplineCurve
(
MSBsplineCurveCR curve
)
    {

    int order = curve.GetIntOrder ();
    if (curve.IsClosed ())
        {
        MSBsplineCurve openCurve;
        openCurve.CopyOpen (curve, curve.GetKnot (0));
        bool stat = AddAsCompleteBsplineCurve (openCurve);
        openCurve.ReleaseMem ();
        return stat;
        }
    DPoint3d startXYZ, endXYZ;
    curve.ExtractEndPoints (startXYZ, endXYZ);
    size_t numKnots = curve.NumberAllocatedKnots ();

    size_t knotIndex = 0;
    
    int numExtraPoles = curve.GetIntOrder () - 2;

    GraphicsPoint startGP (startXYZ, 1.0, curve.GetKnot ((int)knotIndex), 0, s_bsplineEndMask,
                        curve.GetKnot ((int)knotIndex), 
                        reversedIndex(knotIndex, numKnots));
    startGP.SetOrder (order);
    knotIndex++;
    jmdlGraphicsPointArray_addGraphicsPoint (this, &startGP);
    for (int i = 0; i < numExtraPoles; i++)
        {
        GraphicsPoint extraPoleGP (startXYZ, 1.0, curve.GetKnot ((int)knotIndex), 0, s_bsplineExtraPoleMask,
                    curve.GetKnot ((int)knotIndex),
                    reversedIndex(knotIndex, numKnots));
        extraPoleGP.SetOrder (order);
        jmdlGraphicsPointArray_addGraphicsPoint (this, &extraPoleGP);
        knotIndex++;
        }

    int numPoles = curve.NumberAllocatedPoles ();
    for (int i = 0; i < numPoles; i++)
        {
        GraphicsPoint poleGP (curve.GetPoleDPoint4d(i), curve.GetKnot ((int)knotIndex), 0, s_bsplinePoleMask,
                    curve.GetKnot ((int)knotIndex), 
                    reversedIndex(knotIndex, numKnots));
        poleGP.SetOrder (order);
        jmdlGraphicsPointArray_addGraphicsPoint (this, &poleGP);
        knotIndex++;
        }

    GraphicsPoint endGP   (endXYZ,   1.0, curve.GetKnot ((int)knotIndex), 0, s_bsplineEndMask,
                    curve.GetKnot ((int)knotIndex), 
                    reversedIndex(knotIndex, numKnots));
    endGP.SetOrder (order);
    knotIndex++;
    jmdlGraphicsPointArray_addGraphicsPoint (this, &endGP);
    jmdlGraphicsPointArray_markBreak (this);
    return true;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::GetBezierSpanFromBsplineCurve
(
size_t bsplineBaseIndex,
size_t bezierSelect,
DPoint4dP poles,
int &order,
int maxOrder,
bool &isNullInterval,
double &knot0,
double &knot1
) const
    {
    GraphicsPoint gp;
    isNullInterval = false;
    size_t arraySize = vbArray_hdr.size ();
    if (bsplineBaseIndex >= arraySize)
        return false;
    gp = vbArray_hdr[bsplineBaseIndex];
    order = gp.GetOrder ();
    int degree = order - 1;
    if (    order < 2
        ||  order > MAX_BEZIER_CURVE_ORDER
        ||  order > maxOrder
       )
        return false;
    size_t poleBaseIndex = bsplineBaseIndex + order - 1;
    size_t firstPoleIndex = poleBaseIndex + bezierSelect;
    size_t lastPoleIndex  = firstPoleIndex + order;
    if (lastPoleIndex >= arraySize)
        return false;

    size_t firstKnotIndex = bsplineBaseIndex + bezierSelect + 1;
    size_t lastKnotIndex  = firstKnotIndex + 2 * order - 3;
    if (lastKnotIndex >= arraySize)
        return false;

    double knots[2 * MAX_BEZIER_CURVE_ORDER];
    for (size_t i = 0; i < (size_t)order; i++)
        {
        size_t ii = firstPoleIndex + i;
        GraphicsPoint gp = vbArray_hdr[ii];
        if (!gp.CheckCurveAndPointType (HPOINT_MASK_CURVETYPE_BSPLINE, HPOINT_MASK_BSPLINE_POLE))
            return false;
        poles[i] = gp.point;
        }

    // This knot array carries (only!!) {degree} knots at left and right ....
    size_t numKnot = 2 * order - 2;
    for (size_t i = 0; i < numKnot; i++)
        {
        size_t ii = firstKnotIndex + i;
        GraphicsPoint gp = vbArray_hdr[ii];
        knots[i] = gp.b;
        }
    knot0 = knots[degree - 1];
    knot1 = knots[degree];
    bsiBezier_saturateKnotsInInterval ((double*)poles, 4, knots, order, isNullInterval);
    return true;
    }

int GEOMDLLIMPEXP GraphicsPointArray::CopyFromWithBsplinesExpandedToBeziers (GraphicsPointArrayCR &source)
    {
    size_t num0 = source.GetGraphicsPointCount ();
    size_t i1;
    vbArray_hdr.clear ();
    int numBspline = 0;
    MSBsplineCurve curve;
    for (size_t i0 = 0; i0 < num0; i0++)
        {
        if (   source.IsBsplineCurve (i0, i1)
            && source.GetBsplineCurve (i0, curve)
            )
            {
            numBspline++;
            AddBsplineCurve (curve, true);
            curve.ReleaseMem ();
            i0 = i1 + 1;
            }
        else
            {
            GraphicsPoint gp;
            source.GetGraphicsPoint (i0, gp);
            Add (gp);
            }
        }
    return numBspline;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::GetBsplineCurve (size_t i0, MSBsplineCurve &curve) const
    {
    curve.Zero ();
    size_t i1;
    if (IsBsplineCurve (i0, i1))
        {
        GraphicsPoint gp;
        GetGraphicsPoint (i0, gp);
        int order = gp.GetOrder ();
        size_t firstKnotIndex = i0;
        size_t firstPoleIndex = i0 + order - 1;
        size_t numKnots = gp.index + 1;
        size_t numPoles = numKnots - order; // Classic convention.
        bool rational = false;
        for (size_t poleOffset = 0; poleOffset < numPoles; poleOffset++)
            if (!MSBsplineCurve::AreSameWeights(vbArray_hdr[firstPoleIndex + poleOffset].point.w,1.0))
                {
                rational = true;
                break;
                }
                
        if (SUCCESS == curve.Allocate ((int)numPoles, (int)order, false, rational))
            {
            GraphicsPoint gp;
            for (size_t poleOffset = 0; poleOffset < numPoles; poleOffset++)
                {
                GetGraphicsPoint (firstPoleIndex + poleOffset, gp);
                curve.poles[poleOffset].Init (gp.point.x, gp.point.y, gp.point.z);
                if (curve.weights != NULL)
                    curve.weights[poleOffset] = gp.point.w;
                }

            for (size_t knotOffset = 0; knotOffset < numKnots; knotOffset++)
                {
                GetGraphicsPoint (firstKnotIndex + knotOffset, gp);
                curve.knots[knotOffset] = gp.b;
                }
            return true;
            }
        }
    return false;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::GetBezier (size_t readIndex, int maxOrder, size_t &nextReadIndex, DPoint4dP poles, int &order, double &a0, double &a1) const
    {
    bvector<DPoint4d> myPoles;
    if (!GetBezier (readIndex, nextReadIndex, myPoles, a0, a1)
        || (int)myPoles.size () > maxOrder)
        {
        order = 0;
        return false;
        }
    order = (int)myPoles.size ();
    for (size_t i = 0; i < myPoles.size (); i++)
        poles[i] = myPoles[i];
    return true;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::GetBezier (size_t readIndex, size_t &nextReadIndex, bvector<DPoint4d> &poles, double &a0, double &a1) const
    {
    nextReadIndex = readIndex;
    size_t n = vbArray_hdr.size ();
    poles.clear ();
    GraphicsPoint gp;

    if (readIndex < n)
        {
        gp = vbArray_hdr[readIndex++];
        if (!gp.CheckCurveAndPointType (HPOINT_MASK_CURVETYPE_BEZIER, HPOINT_MASK_BEZIER_STARTEND))
            return false;
        poles.push_back (gp.point);
        a0 = gp.a;
        for (;readIndex < n;)
            {
            gp = vbArray_hdr[readIndex++];
            poles.push_back (gp.point);
            if (gp.CheckCurveAndPointType (HPOINT_MASK_CURVETYPE_BEZIER, HPOINT_MASK_BEZIER_POLE))
                {
                // pole as expected
                }
            else if (gp.CheckCurveAndPointType (HPOINT_MASK_CURVETYPE_BEZIER, HPOINT_MASK_BEZIER_STARTEND))
                {
                a1 = gp.a;
                nextReadIndex = readIndex;
                return true;
                }
            else
                {
                break;
                }
            }
        }
    // All failures fall through ...
    return false;
    }



bool GEOMDLLIMPEXP GraphicsPointArray::ParseBsplineCurveKnotDomain
(
size_t readIndex,
size_t &i0,
size_t &i1,
double &knot0,
double &knot1,
int    &order,
size_t &numBezier
) const
    {
    GraphicsPoint gp0, gp1;
    i1 = i0 = readIndex;
    if (!GetGraphicsPoint (readIndex, gp0))
        return false;
    if (gp0.index <= 0)
        return false;
    size_t lastIndex = readIndex + gp0.index;
    if (!GetGraphicsPoint (lastIndex, gp1))
        return false;
    order = gp0.GetOrder ();
    if (   gp0.GetCurveType () != HPOINT_MASK_CURVETYPE_BSPLINE
        || gp0.GetPointType () != HPOINT_MASK_BSPLINE_STARTEND
        || gp1.GetCurveType () != HPOINT_MASK_CURVETYPE_BSPLINE
        || gp1.GetPointType () != HPOINT_MASK_BSPLINE_STARTEND
        || gp1.index != 0
        || !gp1.IsCurveBreak ())
        return false;
    i0 = readIndex + order - 1;
    i1 = lastIndex - order + 1;
    GetGraphicsPoint (i0, gp0);
    GetGraphicsPoint (i1, gp1);
    knot0 = gp0.b;
    knot1 = gp1.b;
    numBezier = i1 - i0;
    return true;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::ParseBsplineCurveKnotDomain
(
size_t readIndex,
size_t &i0,
size_t &i1,
double &knot0,
double &knot1
) const
    {
    int order;
    size_t numBezier;
    return ParseBsplineCurveKnotDomain (readIndex, i0, i1, knot0, knot1,
                order, numBezier);
    }
bool GEOMDLLIMPEXP GraphicsPointArray::ParseBsplineCurvePoleIndices
(
size_t readIndex,
size_t &poleIndex0,
size_t &poleIndex1
) const
    {
    GraphicsPoint gp0, gp1;
    poleIndex1 = poleIndex0 =readIndex;
    if (!GetGraphicsPoint (readIndex, gp0))
        return false;
    if (gp0.index <= 0)
        return false;
    size_t lastIndex = readIndex + gp0.index;
    if (!GetGraphicsPoint (lastIndex, gp1))
        return false;
    int order = gp0.GetOrder ();
    if (   gp0.GetCurveType () != HPOINT_MASK_CURVETYPE_BSPLINE
        || gp0.GetPointType () != HPOINT_MASK_BSPLINE_STARTEND
        || gp1.GetCurveType () != HPOINT_MASK_CURVETYPE_BSPLINE
        || gp1.GetPointType () != HPOINT_MASK_BSPLINE_STARTEND
        || gp1.index != 0
        || !gp1.IsCurveBreak ())
        return false;
    poleIndex0 = readIndex + order - 1;
    poleIndex1 = lastIndex - 1;
    return true;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::SearchBsplineInterval
(
size_t readIndex,
double knotValue,
bool chooseIntervalToRightIfExactHit,
size_t &bezierSelect,
double &bezierFraction
) const
    {
    size_t i0, i1;
    double knot0, knot1;
    if (!ParseBsplineCurveKnotDomain (readIndex, i0, i1, knot0, knot1))
        {
        bezierSelect = 0;
        bezierFraction = 0.0;
        return false;
        }
    size_t numBezierInterval = i1 - i0 + 1;
    // NEEDS_WORK Do binary search ...
    if (knotValue >= knot1)
        {
        bezierSelect = i1 - 1 - i0;
        double b0 = vbArray_hdr[i0 + bezierSelect].b;
        bezierFraction = (knotValue - b0) / (knot1 - b0);
        return true;
        }
    if (chooseIntervalToRightIfExactHit)
        {
        double b0 = vbArray_hdr[i0].b;
        double b1;
        for (size_t i = 1; i <= numBezierInterval; i++, b0 = b1)
            {
            b1 = vbArray_hdr[i0 + i].b;
            if (b1 > knotValue) // um what if extra dups at i0?
                {
                bezierSelect = i - 1;
                bezierFraction = (knotValue - b0) / (b1 - b0);
                return true;
                }
            }
        }
    else
        {
        double b0 = vbArray_hdr[i0].b;
        double b1;
        for (size_t i = 1; i <= numBezierInterval; i++, b0 = b1)
            {
            b1 = vbArray_hdr[i0 + i].b;
            if (b1 >= knotValue) // um what if extra dups at i0?
                {
                bezierSelect = i - 1;
                bezierFraction = (knotValue - b0) / (b1 - b0);
                return true;
                }
            }
        }
    return false;
    }



void GEOMDLLIMPEXP GraphicsPointArray::Add (GraphicsPointCR gp)
    {
    vbArray_hdr.push_back (gp);
    }

void GEOMDLLIMPEXP GraphicsPointArray::Add (DSegment3dCR segment, bool markBreak)
    {
    Add (GraphicsPoint (segment.point[0]));
    Add (GraphicsPoint (segment.point[1]));
    if (markBreak)
        MarkBreak ();
    }

void GEOMDLLIMPEXP GraphicsPointArray::AddMasked (DPoint3dCR point, int mask, bool markBreak)
    {
    GraphicsPoint gp (point);
    gp.mask = mask;
    Add (gp);
    if (markBreak)
        MarkBreak ();
    }

void GEOMDLLIMPEXP GraphicsPointArray::AddMasked (DPoint4dCR point, int mask, bool markBreak)
    {
    GraphicsPoint gp (point);
    gp.mask = mask;
    Add (gp);
    if (markBreak)
        MarkBreak ();
    }


void GEOMDLLIMPEXP GraphicsPointArray::AddBezier (DPoint3dCP points, size_t order)
    {
    if (order < 2 || order > MAX_BEZIER_CURVE_ORDER)
        return;
    AddMasked (points[0],
        HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_STARTEND, false);
    for (size_t i = 1; i < order - 1; i++)
        AddMasked (points[i],
            HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_POLE, false);
    AddMasked (points[order - 1],
        HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_STARTEND, true);
    }

void GEOMDLLIMPEXP GraphicsPointArray::AddBezier (DPoint4dCP points, size_t order)
    {
    if (order < 2 || order > MAX_BEZIER_CURVE_ORDER)
        return;
    AddMasked (points[0],
        HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_STARTEND, false);
    for (size_t i = 1; i < order - 1; i++)
        AddMasked (points[i],
            HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_POLE, false);
    AddMasked (points[order - 1],
        HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_STARTEND, true);
    }


void GEOMDLLIMPEXP GraphicsPointArray::AddBezier (bvector<DPoint3d> &points)
    {
    size_t order = points.size ();
    if (order < 2 || order > MAX_BEZIER_CURVE_ORDER)
        return;
    AddMasked (points[0],
        HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_STARTEND, false);
    for (size_t i = 1; i < order - 1; i++)
        AddMasked (points[i],
            HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_POLE, false);
    AddMasked (points[order - 1],
        HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_STARTEND, true);
    }


void GEOMDLLIMPEXP GraphicsPointArray::Add (DPoint3dCR point, bool markBreak)
    {
    Add (GraphicsPoint (point));
    if (markBreak)
        MarkBreak ();
    }

void GEOMDLLIMPEXP GraphicsPointArray::AddArray (DPoint3dCP points, size_t numPoints, bool markBreak)
    {
    for (size_t i = 0; i < numPoints; i++)
        if (points[i].IsDisconnect ())
            MarkBreak ();
        else
            Add (GraphicsPoint (points[i]));
    if (markBreak)
        MarkBreak ();
    }

void GEOMDLLIMPEXP GraphicsPointArray::AddArray (DPoint2dCP points, size_t numPoints, bool markBreak)
    {
    for (size_t i = 0; i < numPoints; i++)
        if (points[i].IsDisconnect ())
            MarkBreak ();
        else
            Add (GraphicsPoint (points[i].x, points[i].y, 0.0));
    if (markBreak)
        MarkBreak ();
    }

void GEOMDLLIMPEXP GraphicsPointArray::Add (bvector<DPoint3d> &points, bool markBreak)
    {
    size_t numPoints = points.size ();
    for (size_t i = 0; i < numPoints; i++)
        Add (GraphicsPoint (points[i]));
    if (markBreak)
        MarkBreak ();
    }

void GEOMDLLIMPEXP GraphicsPointArray::Add (DEllipse3dCR ellipse)
    {
    DEllipse3d myEllipse = ellipse;
    if (myEllipse.sweep < 0.0)
        {
        myEllipse.start *= -1.0;
        myEllipse.sweep *= -1.0;
        myEllipse.vector90.Negate ();
        }

    DPoint3d point0, point1;

    myEllipse.FractionParameterToPoint (point0, 0.0);
    myEllipse.FractionParameterToPoint (point1, 1.0);
    double theta0 = myEllipse.FractionToAngle (0.0);
    double theta1 = myEllipse.FractionToAngle (1.0);

    double a = 0.0;
    int userData = 0;
    size_t index = 0;

    Add (GraphicsPoint (point0, 1.0, a, userData,
                            HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_STARTEND, theta0, index));
    Add (GraphicsPoint (myEllipse.vector0, 0.0, a, userData,
                            HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_VECTOR, 0.0, index));
    Add (GraphicsPoint (myEllipse.center, 1.0, a, userData,
                            HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_CENTER, myEllipse.sweep, index));
    Add (GraphicsPoint (myEllipse.vector90, 0.0, a, userData,
                            HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_VECTOR, msGeomConst_piOver2, index));
    Add (GraphicsPoint (point1, 1.0, a, userData,
                            HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_STARTEND, theta1, index));

    SetArrayMask (HPOINT_ARRAYMASK_CURVES);
    }
    

void GEOMDLLIMPEXP GraphicsPointArray::SetArrayMask (int mask) {arrayMask |= mask;}
int  GEOMDLLIMPEXP GraphicsPointArray::GetArrayMask (int mask) const {return mask & arrayMask;}

bool GEOMDLLIMPEXP GraphicsPointArray::GetDSegment4d (size_t index, size_t &nextReadIndex, DSegment4dR segment) const
    {
    GraphicsPoint gp0, gp1, gp2;
    nextReadIndex = index;
    if (   GetCheckedGraphicsPoint (index, gp0, 0, 0)
        && GetCheckedGraphicsPoint (index + 1, gp1, 0, 0)
        && !gp0.IsCurveBreak ()
        )
        {
        segment.point[0] = gp0.point;
        segment.point[1] = gp1.point;
        if (!gp1.IsCurveBreak () && GetCheckedGraphicsPoint (index + 2, gp2, 0, 0))
            nextReadIndex = index + 2;
        else
            nextReadIndex = index + 1;
        return true;
        }
    return false;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::GetDConic4d (size_t index, size_t &nextReadIndex, DConic4dR conic) const
    {
    GraphicsPoint gp0, gp1, gp2, gp3, gp4, gp5;
    nextReadIndex = index;
    if (   GetCheckedGraphicsPoint (index++,   gp0, HPOINT_MASK_CURVETYPE_ELLIPSE, HPOINT_MASK_ELLIPSE_STARTEND)
        && GetCheckedGraphicsPoint (index++, gp1, HPOINT_MASK_CURVETYPE_ELLIPSE, HPOINT_MASK_ELLIPSE_VECTOR)
        && GetCheckedGraphicsPoint (index++, gp2, HPOINT_MASK_CURVETYPE_ELLIPSE, HPOINT_MASK_ELLIPSE_CENTER)
        && GetCheckedGraphicsPoint (index++, gp3, HPOINT_MASK_CURVETYPE_ELLIPSE, HPOINT_MASK_ELLIPSE_VECTOR)
        && GetCheckedGraphicsPoint (index++, gp4, HPOINT_MASK_CURVETYPE_ELLIPSE, HPOINT_MASK_ELLIPSE_STARTEND)
        )
        {
        conic.center = gp2.point;
        conic.vector0 = gp1.point;
        conic.vector90 = gp3.point;
        conic.start = gp0.b;
        conic.sweep = gp4.b - gp0.b;
        nextReadIndex = index;
        return true;
        }
    return false;
    }

bool GEOMDLLIMPEXP GraphicsPointArray::GetDConic4d (size_t index, size_t &nextReadIndex, DConic4dR conic, DEllipse3dR ellipse, bool &isSimpleEllipse) const
    {
    if (!GetDConic4d (index, nextReadIndex, conic))
        return false;
    isSimpleEllipse = bsiDEllipse3d_initFromDConic4d (&ellipse, &conic) ? true : false;
    return true;
    }


bool GEOMDLLIMPEXP GraphicsPointArray::GetDSegment3d (size_t index, size_t &nextReadIndex, DSegment3dR segment) const
    {
    DSegment4d segment4d;
    return GetDSegment4d (index, nextReadIndex, segment4d)
            && segment4d.point[0].GetProjectedXYZ (segment.point[0])
            && segment4d.point[1].GetProjectedXYZ (segment.point[1]);
    }

bool GEOMDLLIMPEXP GraphicsPointArray::GetDPoint3dArray (size_t index, DPoint3dP buffer, size_t &numGot, size_t max) const
    {
    size_t n = vbArray_hdr.size ();
    numGot = 0;
    for (size_t i = index;
            i < n && numGot < max && GetNormalizedPoint (i, buffer[numGot]); i++)
        {
        numGot++;
        }
    return numGot == max;
    }


bool GEOMDLLIMPEXP GraphicsPointArray::GetDEllipse3d (size_t index, size_t &nextReadIndex, DEllipse3dR ellipse) const
    {
    DConic4d conic;
    return    GetDConic4d (index, nextReadIndex, conic)
           && bsiDEllipse3d_initFromDConic4d (&ellipse, &conic);
    }

bool GEOMDLLIMPEXP GraphicsPointArray::IsLastPrimitive (size_t index) const
    {
    int i0, i1;
    return jmdlGraphicsPointArray_parsePrimitiveAt (this, &i0, &i1, NULL, NULL, NULL, (int)index)
       && !jmdlGraphicsPointArray_parsePrimitiveAfter (this, &i0, &i1, NULL, NULL, NULL,  i1);
    }

void GEOMDLLIMPEXP GraphicsPointArray::Clear ()
    {
    vbArray_hdr.clear ();
    arrayMask = 0;
    }

void GEOMDLLIMPEXP GraphicsPointArray::Multiply (DMatrix4dCR transform)
    {
    transform.Multiply (&vbArray_hdr[0], &vbArray_hdr[0], vbArray_hdr.size ());
    }

void GEOMDLLIMPEXP GraphicsPointArray::Multiply (TransformCR transform)
    {
    // The transform.Multiply function need to change to pointer args....
    size_t n = vbArray_hdr.size ();
    for (size_t i = 0; i < n; i++)
        transform.Multiply (&vbArray_hdr[i].point, &vbArray_hdr[i].point, 1);
    }

END_BENTLEY_GEOMETRY_NAMESPACE

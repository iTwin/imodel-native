/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/GParray.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <Mtg/MtgApi.h>

GPArrayP        GPArray::Grab () {return (GPArrayP) jmdlGraphicsPointArray_grab();}

void            GPArray::Drop()                                  {jmdlGraphicsPointArray_drop (this);}
void            GPArray::Empty()                                 {GraphicsPointArray::Clear ();}
void            GPArray::EmptyAll()                              {GraphicsPointArray::Clear ();}

double          GPArray::Length() const                          {return GraphicsPointArray::CurveLength ();}
double          GPArray::ApproximateLength() const               { return jmdlGraphicsPointArray_getQuickLength (this); }
void            GPArray::Add (DPoint3dCP pt)                     {GraphicsPointArray::Add (*pt);}
void            GPArray::Add (DPoint3dCP pt, int num)            {GraphicsPointArray::AddArray (pt, (size_t)num);}
void            GPArray::Add (DPoint2d const* pt, int num)       {GraphicsPointArray::AddArray (pt, (size_t)num);}
void            GPArray::Add (GraphicsPoint const* pt)           {GraphicsPointArray::Add (*pt);}
void            GPArray::Add (DEllipse3d const& el)              {GraphicsPointArray::Add (el);}
void            GPArray::Add (double x, double y, double z, double w, double a, int mask, int userData){GraphicsPointArray::Add( GraphicsPoint (x, y, z, w, a, userData, mask));}
void            GPArray::MarkBreak()                             {GraphicsPointArray::MarkBreak ();}
void            GPArray::MarkMajorBreak()                        {GraphicsPointArray::MarkMajorBreak ();}
void            GPArray::SetArrayMask (int mask)                 {GraphicsPointArray::SetArrayMask (mask);}
int             GPArray::GetArrayMask (int mask) const           {return GraphicsPointArray::GetArrayMask (mask);}

GraphicsPoint const*   GPArray::GetConstPtr (int index) const    {return (GraphicsPoint const*) jmdlGraphicsPointArray_getConstPtr(this, index);}
GraphicsPoint*  GPArray::GetPtr (int index)                      {return (GraphicsPoint *) jmdlGraphicsPointArray_getPtr(this, index);}

bool            GPArray::Get (GraphicsPointR gp, int index) const{return GraphicsPointArray::GetGraphicsPoint (ResolveIndex (index), gp);}
                        
int             GPArray::GetCount() const                        {return (int)GetGraphicsPointCount ();}
bool            GPArray::GetPrimitiveFractionPoint(DPoint3dP point, int i, double f) const
    {
    return TO_BOOL(this->PrimitiveFractionToDPoint3d ((size_t) i, f, *point));
    }
bool            GPArray::GetPrimitiveFractionPointAndTangent (DPoint3dR point, DVec3dR tangent, int i, double f) const
    {
    return TO_BOOL(this->PrimitiveFractionToDPoint3d ((size_t) i, f, point, tangent));
    }
bool            GPArray::GetPlane (DPlane3dR plane) const        { return TO_BOOL(jmdlGraphicsPointArray_getDPlane3d (this, &plane)); }
void            GPArray::Stroke (double tolerance)               {jmdlGraphicsPointArray_dropToStrokes (this, tolerance); }
bool            GPArray::IsMajorBreak (int i) const              {return GraphicsPointArray::IsMajorBreak (ResolveIndex (i));}
bool            GPArray::CopyContentsOf (GPArrayCP source)        {this->CopyFrom (*source); return true;}
bool            GPArray::Append (GPArrayCP source)                {this->AppendFrom (*source); return true;}
int             GPArray::CountToEndOfLinestring (int startIndex) const
    {
    size_t endIndex;
    if (this->IsLineString ((size_t)startIndex, endIndex))
        {
        return (int)(endIndex - startIndex) + 1;
        }
    else
        return 0;
    }
bool            GPArray::GetDPoint3dArray (DPoint3d* point, int* nGot, int i0, int nreq) const {return TO_BOOL(jmdlGraphicsPointArray_getDPoint3dArray (this, point, nGot, i0, nreq));}
void            GPArray::SortByA ()                              { jmdlGraphicsPointArray_sortByA (this); }
bool            GPArray::IsPointInsideXY (DPoint3dR point) const { return false != jmdlGraphicsPointArray_isDPoint3dXYInOrOn (const_cast <GPArray*> (this), &point); }

void            GPArray::GetXYIntersections (GPArrayP thisIntersects, GPArrayP otherIntersects, GPArray const& otherGPA) const
                        { jmdlGraphicsPointArray_xyIntersectionPoints (thisIntersects, otherIntersects, const_cast <GPArray*> (this), const_cast <GPArray*> (&otherGPA)); }

void            GPArray::GetPlaneIntersections (GPArrayP intersections, DPlane3dCR plane, bool extend) const
                        { jmdlGraphicsPointArray_addDPlane3dIntersectionPoints (intersections, const_cast <GPArray*> (this), &plane, extend); }

bool            GPArray::CopyPortionOf (GPArrayCP gpa, GPArrayParamR start, GPArrayParamR end)
                        { return TO_BOOL(jmdlGraphicsPointArray_appendInterval (this, gpa, start.m_index, start.m_param, end.m_index, end.m_param)); }

void            GPArray::GetClosestApproachPoints (GPArrayR thisPoints, GPArrayR otherPoints, GPArrayCR  otherCurve,
                            double maxDistance, bool xyOnly, bool extend, double duplicatePointTol) const
                        { return jmdlGraphicsPointArray_collectProjectedApproachPointsExt (&thisPoints, &otherPoints,
                                    const_cast <GPArrayP> (this), const_cast <GPArrayP> (&otherCurve),
                                    extend ? 1 : 0,
                                    maxDistance, xyOnly ? 2 : 3, duplicatePointTol); }

double          GPArray::GetTolerance (GPArrayCP gpa1, GPArrayCP gpa2, double absTol, double relTol)
                        { return jmdlGraphicsPointArray_getTolerance (gpa1, gpa2, absTol, relTol, 0.0);}

bool            GPArray::GetNormalized (DPoint3dR xyz, int index) const
    {
    GraphicsPoint gp;
    return Get (gp, index) && gp.GetNormalized (xyz);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GPArray::Equals (GPArray const& rhs) const
    {
    return TO_BOOL( jmdlGraphicsPointArray_sameGeometryPointByPoint (this, &rhs, GetTolerance(this,&rhs), 0) ); 
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
GPArrayInterval::GPArrayInterval (GPArrayParamCR start, GPArrayParamCR end, GPArrayCP gpa) : m_start (start), m_end (end)
    {
    int                 i1, curveType;
    static double       s_nearOne = 1.0 - 1.0E-10;

    // Make sure it doesn't start at end of a segment...
    if (m_start.m_param >= s_nearOne &&
        NULL != gpa &&
        !gpa->IsLastPrimitive (m_start.m_index) &&
        jmdlGraphicsPointArray_parseFragment (gpa, &i1,  NULL, NULL,  &curveType, m_start.m_index))
        {
        m_start.m_param = 0.0;
        if (curveType == 0 && (int) (m_start.m_index + 1) <  i1)
            m_start.m_index++;
        else
            m_start.m_index =  i1 + 1;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool GPArrayInterval::Compare (GPArrayIntervalCR interval0, GPArrayIntervalCR interval1)
    {
    if (interval0.m_start.GetValue() == interval1.m_end.GetValue())
        return interval0.m_end.GetValue()  < interval1.m_end.GetValue();
    else
        return interval0.m_start.GetValue() < interval1.m_start.GetValue();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GPArray::GetClosestPoint (DPoint3dP closePoint, GPArrayParamP closeParam, DPoint3dCR testPoint, bool xyOnly) const
    {
    ProximityData           proximityData;

    if (!jmdlGraphicsPointArray_closestPoint (this, &proximityData, &testPoint, xyOnly ? 2 : 3, false))
        return false;

    if (NULL != closePoint)
        proximityData.closePoint.getXYZ (closePoint);

    if (NULL != closeParam)
        {
        closeParam->m_index = proximityData.closeIndex;
        closeParam->m_param = proximityData.closeParam;
        }

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3dP      GPArray::GetRange (DRange3dR range) const
    {
    GraphicsPointArray::GetRange (range);
    return &range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            GPArray::Draw (IDrawGeomR drawGeom, bool closed, bool filled) const
    {
    if (0 == GetCount ())
        return;

    CurveVectorPtr  curves = ToCurveVector ();

    if (!curves.IsValid ())
        return;
    
    if (!closed && curves->IsAnyRegionType ()) // Caller just wants outline display...
        {
        if (curves->IsUnionRegion () || curves->IsParityRegion ())
            {
            for (ICurvePrimitivePtr curve: *curves)
                {
                if (curve.IsNull () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                    continue;

                CurveVector* childCurves = const_cast <CurveVector*> (curve->GetChildCurveVectorCP ());

                childCurves->SetBoundaryType (CurveVector::BOUNDARY_TYPE_Open);
                drawGeom.DrawCurveVector (*childCurves, false);
                }
            }
        else
            {
            curves->SetBoundaryType (CurveVector::BOUNDARY_TYPE_Open);
            drawGeom.DrawCurveVector (*curves, false);
            }

        return;
        }

    BeAssert (closed == curves->IsAnyRegionType () && L"Malformed GPA - Open/Closed is determined by presence of major breaks.");
    drawGeom.DrawCurveVector (*curves, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void            GPArray::Transform (TransformCP transform)
    {
    if (NULL != transform && !transform->isIdentity())
        this->Multiply (*transform);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void            GPArray::Transform (DMatrix4dCP  matrix)
    {
    if (NULL != matrix)
        jmdlGraphicsPointArray_multiplyByDMatrix4d (this, matrix); 
    }

/*=================================================================================**//**
Convert the GPArray internal curve type to the GPCurveType ...
* @bsimethod                                                Earlin.Lutz     08/08
+===============+===============+===============+===============+===============+======*/
GPCurveType GPArray::SimplifyCurveType (int detailType)
    {
    switch  (detailType)
        {
        case HPOINT_MASK_CURVETYPE_ELLIPSE:
            return GPCurveType::Ellipse;

        case HPOINT_MASK_CURVETYPE_BEZIER:
            return GPCurveType::Bezier;

        case HPOINT_MASK_CURVETYPE_BSPLINE:
            return GPCurveType::BSpline;
        default:
            return GPCurveType::LineString;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     1/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GPArray::ParseAt (int* i0, int* i1, DPoint4d* point0, DPoint4d  *point1, GPCurveType* curveType, int prev) const
    {
    int         type;
    bool        status = TO_BOOL(jmdlGraphicsPointArray_parsePrimitiveAt (this, i0, i1, point0, point1, &type, prev));

    if (NULL != curveType)
        *curveType = GPArray::SimplifyCurveType (type);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     1/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GPArray::ParseBefore(int* i0, int* i1, DPoint4d* point0, DPoint4d  *point1, GPCurveType* curveType, int prev) const
    {
    int         type;
    bool        status = TO_BOOL(jmdlGraphicsPointArray_parsePrimitiveBefore (this, i0, i1, point0, point1, &type, prev));

    if (NULL != curveType)
        *curveType = GPArray::SimplifyCurveType (type);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     1/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GPArray::ParseAfter (int* i0, int* i1, DPoint4d* point0, DPoint4d  *point1, GPCurveType* curveType, int prev) const
    {
    int         type;
    bool        status = TO_BOOL(jmdlGraphicsPointArray_parsePrimitiveAfter (this, i0, i1, point0, point1, &type, prev));

    if (NULL != curveType)
        *curveType = GPArray::SimplifyCurveType (type);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2007
+---------------+---------------+---------------+---------------+---------------+------*/
GPCurveType     GPArray::GetCurveType (int index) const
    {
    return GPArray::SimplifyCurveType (jmdlGraphicsPointArray_getCurveType (this, index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2007
+---------------+---------------+---------------+---------------+---------------+------*/                              
GPArrayParam    GPArray::GetEndParam() const
    {
    size_t          lastIndex;

    if (!GetLastPrimitiveIndex (lastIndex))
        lastIndex = 0;

    return GPArrayParam ((int) lastIndex, 1.0); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GPArray::GetLineString (int *index, bvector<DPoint3d>& points) const
    {
    if (NULL == index)
        return ERROR;
    
    size_t i0 = (size_t)*index;
    size_t i1;

    if (!IsLineString (i0, i1))
        return ERROR;

    for (; (size_t)*index <= i1; (*index)++)
        {
        DPoint3d  pt;

        this->GetNormalized (pt, (size_t)*index);
        points.push_back (pt);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GPArray::GetEllipse (int* index, DEllipse3dP ellipse) const
    {
    return GetEllipse (index, ellipse, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 12/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GPArray::GetConicOrEllipse (int* index, DConic4dP conic, DEllipse3dP ellipse, bool &isEllipse, bool flatten) const
    {
    size_t nextReadIndex;
    if (!GetDConic4d ((size_t)*index, nextReadIndex, *conic, *ellipse, isEllipse))
        return ERROR;
    *index = (int)nextReadIndex;
        
    if (isEllipse)
        return SUCCESS;

    DConic4d    axes;
    RotMatrix   basis;
    int         curveType;

    if (flatten)
        conic->vector0.z = conic->vector90.z = conic->center.z = 0.0;

    if (bsiDConic4d_getCommonAxes (conic, &axes, &basis, &curveType) && curveType == 1)
        {
        axes.center.getXYZ (&ellipse->center);
        axes.vector0.getXYZ (&ellipse->vector0);
        axes.vector90.getXYZ (&ellipse->vector90);
        ellipse->start = axes.start;
        ellipse->sweep = axes.sweep;
        isEllipse = true;

        return SUCCESS;
        }

    // You'll have to live with the conic ...
    isEllipse = false;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 12/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GPArray::GetEllipse (int* index, DEllipse3dP ellipse, bool flatten) const
    {
    DConic4d    conic;
    bool isSimpleEllipse;
    size_t nextReadIndex;
    if (!GetDConic4d ((size_t)*index, nextReadIndex, conic, *ellipse, isSimpleEllipse))
        return ERROR;
    *index = (int)nextReadIndex;

    if (isSimpleEllipse)
        return SUCCESS;

    DConic4d    axes;
    RotMatrix   basis;
    int         curveType;

    if (flatten)
        conic.vector0.z = conic.vector90.z = conic.center.z = 0.0;

    if (bsiDConic4d_getCommonAxes (&conic, &axes, &basis, &curveType) && curveType == 1)
        {
        axes.center.getXYZ (&ellipse->center);
        axes.vector0.getXYZ (&ellipse->vector0);
        axes.vector90.getXYZ (&ellipse->vector90);
        ellipse->start = axes.start;
        ellipse->sweep = axes.sweep;

        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GPArray::IsSingleDEllipse3d (DEllipse3dP ellipse) const
    {
    size_t nextReadIndex;
    return GetDEllipse3d (0, nextReadIndex, *ellipse) && IsLastPrimitive (0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GPArray::IsSingleDSegment3d (DSegment3dP segment) const
    {
    size_t nextReadIndex;
    return GetDSegment3d (0, nextReadIndex, *segment) && IsLastPrimitive (0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GPArray::IsSingleCurvePath () const
    {
    /* NOTE: We can only create a valid association to a single curve/linestring.
             If the path gpa represents a complex shape/chain we *could* represent 
             it as bspline curve and allow the association. This currently isn't 
             needed for section associations which I *believe* are always single 
             edges so this can be deferred until it's actually needed. */
    size_t      tailIndex;

    if (!IsLastPrimitive (IsLineString (0, tailIndex) ? tailIndex-1 : 0))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void        GPArray::Dump (WCharCP label) const
    {
    GraphicsPoint gp;
    printf ("%ls - %d Points\n", label, GetCount());
    for (int i=0; Get (gp, i); i++)
        printf ("%ls - Point: %d = (%f,%f,%f) \t - Mask: %x\n", label, i, gp.point.x, gp.point.y, gp.point.z, gp.mask);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dP GPArray::GetPoint (DPoint3dR point, int index) const
    {
    return  (index >= 0 && GetNormalized (point, index)) ?  &point : NULL;
    }
// Return the real distance between poles, 
// and true if this and the weight difference are within tolerance.
bool MatchedPoles (DPoint4dCR poleA, DPoint4dCR poleB, double xyzTol, double &d)
    {
    static double s_weightTol = 1.0e-8;
    d = bsiDPoint4d_realDistance (&poleA, &poleB);
    if (fabs (poleA.w - poleB.w) > s_weightTol)
        return false;
    return  d <= xyzTol ? true : false;
    }


// A <-- A + fraction * (A - B)
void FractionalExtrapolate (double fraction, DPoint4d &A, DPoint4d const &B)
    {
    A.x += fraction * (A.x - B.x);
    A.y += fraction * (A.y - B.y);
    A.z += fraction * (A.z - B.z);
    A.w += fraction * (A.w - B.w);
    }

static DPoint4d AddWeightedXYZ (DPoint4dCR xyzw, DPoint3dCR xyz, double a)
    {
    DPoint4d result = xyzw;
    result.x += a * xyz.x;
    result.y += a * xyz.y;
    result.z += a * xyz.z;
    return result;
    }

// Context for building up bspline poles and knots from bezier fragments.
//=======================================================================================
// @bsiclass       
//=======================================================================================
struct KnotRemovalContext
{
public:
bvector<DPoint4d> poles;
bvector<double>   knots;
int               order;
double m_abstol;
double m_reltol;

DPoint4d acceptedBezierPoles[MAX_BEZIER_ORDER];
DPoint4d workPoles[MAX_BEZIER_ORDER];
double   lambda[MAX_BEZIER_ORDER];
double   acceptedNewKnot;
size_t   numAccept;
public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod       
+---------------+---------------+---------------+---------------+---------------+------*/
KnotRemovalContext (double abstol = 1.0e-14, double reltol = 1.0e-10)
    {
    m_abstol = abstol;
    m_reltol = reltol;
    order = 0;
    }
private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod       
+---------------+---------------+---------------+---------------+---------------+------*/
void AddKnot (double a, int multiplicity)
    {
    for (size_t i = 0; i < (size_t)multiplicity; i++)
        knots.push_back (a);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod       
+---------------+---------------+---------------+---------------+---------------+------*/
// return numberRemovable = number of knot removal steps that terminate with a match.
// Matching the endpoint is 1
// Matching (parametric) derivative is 2.
// @param [in,out] bezierPoles On input, a bezier span to be appended at right of an existing bspline.
//    On ouptut, these are revised so that the first numberRemovable of these can be used IN PLACE OF 
//     the final knots of the prior spline.
// @param [in] poles poles of spline.  These are NOT changed.
// @param [in] knots.  knots of spline.  These are NOT changed.
// @param [in] order order of bezier and bspline.
// @param [in] newKnot  The right knot after appending the new bezier.
int TestKnotRemoval
(
DPoint4dCP bezierPoles,
double newKnot,
size_t maxAccept,
double &dMax
)
    {
    static int s_preferTarget = 1;
    static int s_zeroOrigin = 1;
    dMax = 0.0;
    ptrdiff_t lastInteriorKnotIndex = (ptrdiff_t)knots.size () - order;
    DRange3d range;
    range.Init ();
    range.Extend (bezierPoles, order);
    double tolerance = m_abstol
            + m_reltol * (range.low.maxAbs () + range.high.maxAbs ()) * order * order;

    double lastKnot = knots[lastInteriorKnotIndex + 1];
    size_t lastPoleIndex = poles.size() - 1;
    DPoint3d localOrigin;
    poles[0].getProjectedXYZ (&localOrigin);
    if (s_zeroOrigin)
        localOrigin.zero ();
    for (int i = 0; i < order; i++)
        {
        double interiorKnot = knots[lastInteriorKnotIndex - i];
        lambda[i] = (interiorKnot - lastKnot) / (lastKnot - newKnot);
        workPoles[i] = acceptedBezierPoles[i] = AddWeightedXYZ (bezierPoles[i], localOrigin, -bezierPoles[i].w);
        }
    acceptedNewKnot = newKnot;

    numAccept = 0;
    if (maxAccept > (size_t)order)
        maxAccept = (size_t)order;
    for (size_t k = 0; k < maxAccept; k++)
        {
        // backwards extrapolation of k "new" polygon edges
        for (size_t j = 0; j < k; j++)
            {
            FractionalExtrapolate (lambda[j+1], workPoles[k - j - 1], workPoles[k - j]);
            }
        DPoint4d target = AddWeightedXYZ (poles[lastPoleIndex - k], localOrigin, -poles[lastPoleIndex - k].w);
        // After extrapolation (from inside back to 0), the 0 pole should match a pole inside the bspline
        double d;
        if (!MatchedPoles (workPoles[0], target, tolerance, d))
            break;
        if (s_preferTarget)
            workPoles[0] = target;
        if (d > dMax)
            dMax = d;
        numAccept++;
        for (size_t j = 0; j < k; j++)
            acceptedBezierPoles[j] = workPoles[j];
        }

    for (size_t i = 0; i < (size_t)order; i++)
        acceptedBezierPoles[i] = AddWeightedXYZ (acceptedBezierPoles[i], localOrigin, acceptedBezierPoles[i].w);

    return (int) numAccept;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod       
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplyAcceptedBezier ()
    {
    assert (poles.size () >= numAccept);
    size_t n = poles.size ();
    for (size_t i = 0; i < numAccept; i++)
        poles[n - numAccept + i] = acceptedBezierPoles[i];
    for (size_t i = numAccept; i < (size_t)order; i++)
        poles.push_back (acceptedBezierPoles[i]);
    knots.resize (knots.size () - numAccept);
    AddKnot (acceptedNewKnot, order);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod       
+---------------+---------------+---------------+---------------+---------------+------*/
public:
bool AppendBezier (DPoint4dCP newBezierPoles, double knot0, double knot1, int newOrder, int minMatch, int maxMatch)
    {
    static double s_knotAppendTol = 1.0e-10;    // Tighter than usual knot tol, but we really expect good things here.
    if (poles.size () == 0)
        {
        for (size_t i = 0; i < (size_t)newOrder; i++)
            poles.push_back (newBezierPoles[i]);
        order = newOrder;
        AddKnot (knot0, order);
        AddKnot (knot1, order);
        return true;
        }
    else if (order == newOrder && knot1 > knots.back () && fabs (knots.back () - knot0) < s_knotAppendTol)
        {
        double dMax = 0.0;
        int num = TestKnotRemoval (newBezierPoles, knot1, maxMatch, dMax);
        if (num >= minMatch && num <= maxMatch)
            {
            ApplyAcceptedBezier ();
            return true;
            }
        }
    return false;
    }
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod       
+---------------+---------------+---------------+---------------+---------------+------*/
// It is observed ...
// A nonrational bspline (that's the easy kind, expect unit weights everywhere)
// arrives with beziers whose weights deviate from 1 by various tiny amounts (e.g. 1e-15)
// When these go through knot removal, the extrapolations gradually expand
// deviations, so by round 15 or so they are 1e-8.
// So we will say that if everything looks like 1 to start with it shall become exactly one.
void CorrectUnitWeights (DPoint4dP poles, int order)
    {
    static double s_unitTol = 1.0e-14;
    for (int i = 0; i < order; i++)
        if (fabs (1.0 - poles[i].w) >  s_unitTol)
            return;

    // everything is near 1.  Force it exact.
    for (int i = 0; i < order; i++)
        poles[i].w = 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod       
+---------------+---------------+---------------+---------------+---------------+------*/
// Pull beziers from gpa.
// Assemble to saturated bspline style.
// First knot appears order times.
// Intermediate knots appear order-1 times.
// Final knot appears order times.
// Shared pole at interior knots is not repeated.
bool AppendBezier
(
KnotRemovalContext &krc,
GraphicsPointArrayCR source,
int readIndex0,
int &nextReadIndex
)
    {
    int myReadIndex = readIndex0;
    nextReadIndex = readIndex0;
    DPoint4d newPoles[MAX_BEZIER_CURVE_ORDER];
    int newOrder;
//    static double s_knotTol = 1.0e-8; unused var removed in graphite
//    static double s_relTol  = 1.0e-10; unused var removed in graphite
    if (jmdlGraphicsPointArray_getBezier (&source, &myReadIndex, newPoles, &newOrder, MAX_BEZIER_CURVE_ORDER))
        {
        DRange3d bezierRange;
        bezierRange.Init ();
        bezierRange.Extend (newPoles, newOrder);
        GraphicsPoint gp;
        double knot0, knot1;
        jmdlGraphicsPointArray_getGraphicsPoint (&source, &gp, readIndex0);
        knot0 = gp.a;
        jmdlGraphicsPointArray_getGraphicsPoint (&source, &gp, readIndex0 + newOrder - 1);
        knot1 = gp.a;
        if (knot1 <= knot0)
            return false;
        CorrectUnitWeights (newPoles, newOrder);

        if (krc.AppendBezier (newPoles, knot0, knot1, newOrder, 1, newOrder - 1))
            {
            nextReadIndex = myReadIndex;    
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2007
+---------------+---------------+---------------+---------------+---------------+------*/                              
static StatusInt   bCurveFromDPoint4d
(
MSBsplineCurveP     curve,
bvector<DPoint4d>&  poles,
bvector<double>&    knots,
int                 order
)
    {
    int numPoles = (int) poles.size ();
    int numKnots = (int) knots.size ();
    if (numPoles + order != numKnots)
        return ERROR;

    memset (curve, 0, sizeof (MSBsplineCurve));
    
    curve->rational = !bsiBezierDPoint4d_isUnitWeight (&poles[0], numPoles, -1.0);
    curve->display.curveDisplay = TRUE;
    curve->params.order = order;
    curve->params.numPoles = numPoles;
    curve->params.numKnots = numKnots;

    if (SUCCESS != curve->Allocate ())
        return ERROR;
    
    for (int i = 0; i < numPoles; i++)
        {
        DPoint4d xyzw = poles[i];
        curve->poles[i].x = xyzw.x;
        curve->poles[i].y = xyzw.y;
        curve->poles[i].z = xyzw.z;

        if (curve->rational)
            curve->weights[i] = xyzw.w;
        }

    for (int i = 0; i < numKnots; i++)
        curve->knots[i] = knots[i];

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod       
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt GetFastBCurve2
(
GraphicsPointArrayCR source,
int &readIndex,
MSBsplineCurve* curve
)
    {
    static double s_absTol = 1.0e-12;
    static double s_relTol = 1.0e-7;
    KnotRemovalContext krc (s_absTol, s_relTol);
    int readIndexA = readIndex;
    int numBezier = 0;

    while (AppendBezier (krc, source, readIndexA, readIndexA))
        numBezier++;    
    memset (curve, 0, sizeof (MSBsplineCurve));
    if (numBezier == 0)
        return ERROR;

    if (SUCCESS == bCurveFromDPoint4d (curve, krc.poles, krc.knots, krc.order))
        {
        readIndex = readIndexA;
        return SUCCESS;
        }
    return ERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod       
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus  GetBCurveFromBezier
(
GraphicsPointArrayCR source,
int *index,
MSBsplineCurveR curve
)
    {
    if (SUCCESS == GetFastBCurve2 (source, *index, &curve))
        return SUCCESS;

    DPoint4d        poleArray[MAX_BEZIER_CURVE_ORDER];
    int             numPole;
    double a0, a1;
    size_t readIndex = (size_t)*index;
    size_t nextReadIndex;
    if (source.GetBezier (readIndex, MAX_BEZIER_CURVE_ORDER, nextReadIndex, poleArray, numPole, a0, a1)
        && SUCCESS == curve.InitFromDPoint4dArray (poleArray, numPole, numPole)
        )
        {
        *index = (int)nextReadIndex;
        return SUCCESS;
        }
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod       
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GPArray::GetBCurve (int *index, MSBsplineCurve* curve) const
    {
    GraphicsPoint gp;
    size_t i0 = (size_t)*index;
    size_t i1;
    if (IsBsplineCurve (i0, i1)
        && GraphicsPointArray::GetBsplineCurve (i0, *curve))
        {        
        *index = (int)(1 + i1);
        return SUCCESS;
        }
    return GetBCurveFromBezier (*this, index, *curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GPArray::GetAsBCurve (int* i, MSBsplineCurve* curve) const
    {
    BentleyStatus   status = ERROR;

    switch (GetCurveType (*i))
        {
        case GPCurveType::Ellipse:
            {
            DEllipse3d  ellipse;

            if (SUCCESS == (status = GetEllipse (i, &ellipse)))
                status = (BentleyStatus) curve->InitFromDEllipse3d (ellipse);
            break;
            }

        case GPCurveType::Bezier:
        case GPCurveType::BSpline:
            {
            status = GetBCurve (i, curve);
            break;
            }

        case GPCurveType::LineString:
            {
            bvector<DPoint3d> points;

            if (SUCCESS == (status = GetLineString (i, points)))
                status = (BentleyStatus) curve->InitFromPoints (&points.front (), (int) points.size ());
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GPArray::ToBCurve (MSBsplineCurve* curve) const
    {
    int         count = GetCount();

    if (!count)
        return ERROR;

    BentleyStatus   status = SUCCESS;
    double          segmentParam = 0.0;
    curve->Zero ();

    for (int i=0; i < count && SUCCESS == status; )
        {
        MSBsplineCurve      segment;
        segment.Zero ();
        if (SUCCESS == (status = GetAsBCurve (&i, &segment)))
            {
            if (0 == curve->params.numPoles)
                {
                *curve = segment;
                }
            else
                {
                curve->MapKnots (0.0, segmentParam += 1.0);
                status = (BentleyStatus) curve->AppendCurves (*curve, segment, false, false);
                }
            }
        }

    if (SUCCESS != status)
        curve->ReleaseMem ();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2007
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GPArray::ContainsCurves () const
    {
    Parser parser (this);
    parser.Reset ();
    while (parser.MoveToNextFragment ())
        if (parser.GetCurveType () != 0)
            return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GPArray::GetTranslationTo (DVec3dR translation, GPArrayCR other, double tolerance) const
    {
    if (other.GetCount() != GetCount())
        return ERROR;

    double      length = ApproximateLength(), otherLength = other.ApproximateLength();

    if (fabs (length - otherLength) > tolerance)
        return ERROR;

    
    DPoint3d        point0, point1;
    DVec3d          testTranslation;

    GetPrimitiveFractionPoint (&point0, 0, 0.0);
    other.GetPrimitiveFractionPoint (&point1, 0, 0.0);

    translation.differenceOf (&point1, &point0);
    for (int i=0, iEnd; i<GetCount(); i = iEnd + 1)
        {
        GPCurveType curveType = GetCurveType (i);
        jmdlGraphicsPointArray_parseFragment (this, &iEnd, NULL, NULL, NULL, i);

        if (GPCurveType::LineString == curveType)
            {
            for (int iTest = i; iTest<=iEnd; iTest++)
                {
                GetConstPtr (iTest)->GetNormalized (point0);
                other.GetConstPtr(iTest)->GetNormalized (point1);

                testTranslation.DifferenceOf (point1, point0);
                if (testTranslation.Distance (translation) > tolerance)
                    return ERROR;

                }    
            }
        else
            {
            GetPrimitiveFractionPoint (&point0, i, .5);
            other.GetPrimitiveFractionPoint (&point1, i, .5);

            testTranslation.DifferenceOf (point1, point0);
            if (testTranslation.Distance (translation) > tolerance)
                return ERROR;
            }
        }
    return SUCCESS;
    }

struct GPACompatibiltyEnforcer
{
private:

GPArrayP *mpProfile;          // ARRAY of GPA pointers.
int      mNumProfile;
double   mDistanceTol;
bool     mbBreakAtIntermediatePhysicalClosure;
bool     mbForceFinalClosure;

/*---------------------------------------------------------------------------------**//**
Load start and end rails from a fragment in multiple GPA's.
Record the rails in graphics points:
   point = coordinates from curve
   userData = curveType
@param readIndex OUT as returned from _parseFragment.
Return ERROR if any mismatch in point counts.
* @bsimethod                                                    Earlin.Lutz 09/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool LoadRails
(
GraphicsPoint *pRailA,
GraphicsPoint *pRailB,
int readIndex,
int &readIndex1
)
    {

    for (int i = 0; i < mNumProfile; i++)
        {
        DPoint4d pointA, pointB;
        int detailType;
        int iEnd;
        jmdlGraphicsPointArray_parseFragment (mpProfile[i], &iEnd, &pointA, &pointB, &detailType, readIndex);
        int curveType = (int)GPArray::SimplifyCurveType (detailType);
        bsiGraphicsPoint_initFromDPoint4d (&pRailA[i], &pointA, 0, 0, curveType);
        bsiGraphicsPoint_initFromDPoint4d (&pRailB[i], &pointB, 0, 0, curveType);
        if (i == 0)
            {
            readIndex1 = iEnd;
            }
        else
            {
            if (iEnd != readIndex1)
                return false;
            }

        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
Copy rail data and read index from C to A
* @bsimethod                                                    Earlin.Lutz  09/08
+---------------+---------------+---------------+---------------+---------------+------*/
void CopyRail
(
GraphicsPoint *pRailA,
int &readIndexA,
GraphicsPoint *pRailC,
int readIndexC
)
    {
    memcpy (pRailA, pRailC, mNumProfile * sizeof (GraphicsPoint));
    readIndexA = readIndexC;
    }

/*---------------------------------------------------------------------------------**//**
Construct gap edge from B to C.
* @bsimethod                                                    Earlin.Lutz  09/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool EnforceContinuityByInsertingEdge
(
GPArrayP pProfile,
int readIndexC,
DPoint4d &pointB,
DPoint4d &pointC
)
    {
    GraphicsPoint gpB, gpC;
    bsiGraphicsPoint_initFromDPoint4d (&gpB, &pointB, 0, 0, HPOINT_NORMAL);
    bsiGraphicsPoint_initFromDPoint4d (&gpC, &pointC, 0, 0, HPOINT_MASK_BREAK);
    jmdlGraphicsPointArray_insertGraphicsPoint (pProfile, &gpB, readIndexC + 1);
    jmdlGraphicsPointArray_insertGraphicsPoint (pProfile, &gpC, readIndexC + 2);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
Force coordinates from one point to another.  Lines can move, bspline ends can move, ellipse ends cannot.
* @bsimethod                                                    Earlin.Lutz  09/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool EnforceContinuityByMovingEndPoint
(
GPArrayP pProfile,
int readIndexB,
DPoint4dR pointB,
int curveTypeB,
int readIndexC,
DPoint4dR pointC,
int curveTypeC
)
    {
    GraphicsPointP gpB = pProfile->GetPtr (readIndexB);
    GraphicsPointP gpC = pProfile->GetPtr (readIndexC);
    bool bstat = true;
    if (static_cast<GPCurveType>(curveTypeB) == GPCurveType::LineString)
        {
        gpB->SetPointPreserveWeight (pointC);
        }
    else if (static_cast<GPCurveType>(curveTypeC) == GPCurveType::LineString)
        {
        gpC->SetPointPreserveWeight (pointB);
        }
    else if (static_cast<GPCurveType>(curveTypeB) == GPCurveType::Bezier)
        {
        gpB->SetPointPreserveWeight (pointC);
        }
    else if (static_cast<GPCurveType>(curveTypeC) == GPCurveType::Bezier)
        {
        gpC->SetPointPreserveWeight (pointB);
        }
    else
        {
        // alas, ellipse ellispe is hard.
        bstat = false;
        }
    return bstat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz  09/08
+---------------+---------------+---------------+---------------+---------------+------*/
double MaxGapBetweenRails
(
GraphicsPoint *pRailB,
GraphicsPoint *pRailC
)
    {
    double dd, ddMax = 0.0;
    for (int i = 0; i < mNumProfile; i++)
        {
        bsiDPoint4d_realDistanceSquared (&pRailB[i].point, &dd, &pRailC[i].point);
        if (dd > ddMax)
            ddMax = dd;
        }
    return ddMax;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz  09/08
+---------------+---------------+---------------+---------------+---------------+------*/
double MinGapBetweenRails
(
GraphicsPoint *pRailA,
GraphicsPoint *pRailD
)
    {
    double dd, ddMin = DBL_MAX;
    for (int i = 0; i < mNumProfile; i++)
        {
        bsiDPoint4d_realDistanceSquared (&pRailA[i].point, &dd, &pRailD[i].point);
        if (dd < ddMin)
            ddMin = dd;
        }
    return sqrt (ddMin);
    }

/*---------------------------------------------------------------------------------**//**
Force continuity between rails B and C.
If all gaps are small, force them to zero.
If any gap is large, insert join segments in all profiles.
* @bsimethod                                                    Earlin.Lutz  09/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool TestAndFillGaps
(
int readIndexB,
GraphicsPoint *pRailB,
int readIndexC,
GraphicsPoint *pRailC,
int &numAdd
)
    {
    double d = MaxGapBetweenRails (pRailB, pRailC);
    numAdd = 0;
    if (d <= mDistanceTol)
        {
        for (int i = 0; i < mNumProfile; i++)
            EnforceContinuityByMovingEndPoint (mpProfile[i], readIndexB, pRailB[i].point, pRailB[i].userData, readIndexC, pRailC[i].point, pRailC[i].userData);
        return true;
        }
    else
        {
        for (int i = 0; i < mNumProfile; i++)
            EnforceContinuityByInsertingEdge (mpProfile[i], readIndexB, pRailB[i].point, pRailC[i].point);

        numAdd = 2;
        return true;
        }

    // Unreachable code
    // return false;
    }

/*---------------------------------------------------------------------------------**//**
If any profile is closed, enforce closure on all sections.
* @bsimethod                                                    Earlin.Lutz  09/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool TestAndEnforceRailClosure
(
int readIndexA,
GraphicsPoint *pRailA,
int readIndexD,
GraphicsPoint *pRailD,
int &numAdd
)
    {
    static double sClosureFactor = 1.0;
    numAdd = 0;
    if (MinGapBetweenRails (pRailA, pRailD) <= sClosureFactor * mDistanceTol)
        {
        for (int i = 0; i < mNumProfile; i++)
            jmdlGraphicsPointArray_markMajorBreakAt (mpProfile[i], readIndexD);

        TestAndFillGaps (readIndexD, pRailA, readIndexA, pRailA, numAdd);
        return true;
        }
    return false;
    }

public:

GPACompatibiltyEnforcer
(
GPArrayP *pProfile,
int     numProfile,
double  distanceTol,
bool    bBreakAtIntermediatePhysicalClosure,
bool    bForceFinalClosure
)
    {
    mpProfile = pProfile;
    mNumProfile = numProfile;
    mbBreakAtIntermediatePhysicalClosure = bBreakAtIntermediatePhysicalClosure;
    mbForceFinalClosure = false;
    mDistanceTol = distanceTol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   Go ()
    {
    GraphicsPoint *pRailA = (GraphicsPoint*)_alloca (mNumProfile * sizeof (GraphicsPoint));
    GraphicsPoint *pRailB = (GraphicsPoint*)_alloca (mNumProfile * sizeof (GraphicsPoint));
    GraphicsPoint *pRailC = (GraphicsPoint*)_alloca (mNumProfile * sizeof (GraphicsPoint));
    GraphicsPoint *pRailD = (GraphicsPoint*)_alloca (mNumProfile * sizeof (GraphicsPoint));

    // (A "rail" is a chain of points at a fixed index in all profiles.)
    // pRailA is saved from first rail of the current loop.
    // pRailB is saved from the end rail of the previous fragment.
    // pRailC is the first rail of the current fragment
    // pRailD is the end rail of the current fragment.

    int readIndexA, readIndexB = 0;
    int readIndexC, readIndexD;
    int numAdd = 2;
    for (readIndexC = readIndexA = 0;
            readIndexC < jmdlGraphicsPointArray_getCount (mpProfile[0]);
            readIndexC = readIndexD + 1)
        {
        if (!LoadRails (pRailC, pRailD, readIndexC, readIndexD))
            return ERROR;

        if (readIndexC == readIndexA)
            {
            // Save for reference as first curve in section
            CopyRail (pRailA, readIndexA, pRailC, readIndexC);
            }

        if (readIndexC > readIndexA && readIndexC == readIndexB + 1)
            {
            TestAndFillGaps (readIndexB, pRailB, readIndexC, pRailC, numAdd);
            readIndexC += numAdd;
            readIndexD += numAdd;
            }

        if (mbBreakAtIntermediatePhysicalClosure
            && TestAndEnforceRailClosure (readIndexA, pRailA, readIndexD, pRailD, numAdd))
            {
            // Reset at second loop.
            readIndexD += numAdd;
            readIndexA = readIndexD + 1;
            }
        else if (mpProfile[0]->IsMajorBreak (readIndexD))
            {
            readIndexA = readIndexD + 1;
            }
        else
            {
            // Continue moving through current loop.  Save D as B for comparison to next fragment ...
            CopyRail (pRailB, readIndexB, pRailD, readIndexD);
            }
        if (mbForceFinalClosure
            && readIndexD + 1 == readIndexC < jmdlGraphicsPointArray_getCount (mpProfile[0]))
            {
            TestAndFillGaps (readIndexD, pRailD, readIndexA, pRailA, numAdd);
            break;
            }
        }

    return SUCCESS;
    }

}; // GPACompatibiltyEnforcer

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GPArray::EnforceProfileContinuity
(
GPArrayP*       pProfile,
int             numProfile,
double          distanceTol,
bool            bBreakAtIntermediatePhysicalClosure,
bool            bForceFinalClosure
)
    {
    GPACompatibiltyEnforcer context (pProfile, numProfile, distanceTol, bBreakAtIntermediatePhysicalClosure, bForceFinalClosure);
    return context.Go ();
    }

/*---------------------------------------------------------------------------------**//**
*   BSpline Curve/Surface to GPA Conversion Routines
+---------------+---------------+---------------+---------------+---------------+------*/
#define MAX_POLE_BUFFER 1000



/*---------------------------------------------------------------------------------**//**
* Load B-Spline curve into a graphics point array -- only handles easy
* special cases.
* @bsimethod                                                    RayBentley      06/01
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   addBsplineCurveSpecialCases (GraphicsPointArrayP pGPA, MSBsplineCurveCP pCurve)
    {
    GraphicsPoint   gp;
    int             order = pCurve->params.order;

    // linear B-spline curves are stored as linestrings
    if (order == 2)
        {
        if (pCurve->params.closed)
            {
            if (pCurve->params.numKnots == pCurve->params.numPoles - 1)
                {
                /* In addition to the indicated interior knot count,
                    the knot array is sandwiched by one start/end knot and one
                    wraparound knot at each end. */
                double   const  *pKnotBuffer    = pCurve->GetKnotCP () + order - 1;
                DPoint3d const  *pPoleBuffer    = pCurve->GetPoleCP ();
                double   const  *pWeightBuffer  = pCurve->GetWeightCP ();
                int         i;
                for (i = 0; i < pCurve->params.numPoles; i++)
                    {
                    gp.point.x  = pPoleBuffer[i].x;
                    gp.point.y  = pPoleBuffer[i].y;
                    gp.point.z  = pPoleBuffer[i].z;
                    gp.point.w  = pCurve->rational ? pWeightBuffer[i] : 1.0;
                    gp.mask     = 0;
                    gp.userData = 0;
                    gp.a        = pKnotBuffer[i];
                    pGPA->Add (gp);
                    }

                /* last pole is first */
                gp.point.x  = pPoleBuffer[0].x;
                gp.point.y  = pPoleBuffer[0].y;
                gp.point.z  = pPoleBuffer[0].z;
                gp.point.w  = pCurve->rational ? pWeightBuffer[0] : 1.0;
                gp.mask     = 0;
                gp.userData = 0;
                gp.a        = pKnotBuffer[pCurve->params.numPoles];
                pGPA->Add (gp);
                pGPA->MarkBreak ();
                return SUCCESS;
                }
            }
        else /* open */
            {
            if (pCurve->params.numKnots == pCurve->params.numPoles - order)
                {
                /* In addition to the indicated interior knot count,
                    the knot array contains double knots at start and end. */
                double   const  *pKnotBuffer    = pCurve->GetKnotCP () + order - 1;
                DPoint3d const  *pPoleBuffer    = pCurve->GetPoleCP ();
                double   const  *pWeightBuffer  = pCurve->GetWeightCP ();
                int         i;
                for (i = 0; i < pCurve->params.numPoles; i++)
                    {
                    gp.point.x  = pPoleBuffer[i].x;
                    gp.point.y  = pPoleBuffer[i].y;
                    gp.point.z  = pPoleBuffer[i].z;
                    gp.point.w  = pCurve->rational ? pWeightBuffer[i] : 1.0;
                    gp.mask     = 0;
                    gp.userData = 0;
                    gp.a        = pKnotBuffer[i];
                    pGPA->Add (gp);
                    }

                pGPA->MarkBreak ();
                return SUCCESS;
                }
            }
        }
    return ERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void            GPArray::Add (MSBsplineCurve const& curve)
    {
    if (SUCCESS == addBsplineCurveSpecialCases (this, &curve))
        return;

    AddAsCompleteBsplineCurve (curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr GPArray::GetCurvePrimitive (int& index) const
    {
    ICurvePrimitivePtr  primitive;

    switch (GetCurveType (index))
        {
        case GPCurveType::LineString:
            {
            bvector<DPoint3d> points;

            if (SUCCESS != GetLineString (&index, points))
                break;

            primitive = ICurvePrimitive::CreateLineString (points);
            break;
            }

        case GPCurveType::Ellipse:
            {
            DEllipse3d  ellipse;

            if (SUCCESS != GetEllipse (&index, &ellipse))
                break;

            primitive = ICurvePrimitive::CreateArc (ellipse);
            break;
            }

        case GPCurveType::Bezier:
        case GPCurveType::BSpline:
            {
            MSBsplineCurve  curve;

            if (SUCCESS != GetBCurve (&index, &curve))
                break;

            primitive = ICurvePrimitive::CreateBsplineCurve (curve);
            curve.ReleaseMem ();
            break;
            }
        }

    return primitive;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GPArray::Add (CurveVectorCR curves, bool splinesAsBezier) {return AddCurves (curves, splinesAsBezier);}
CurveVectorPtr  GPArray::ToCurveVector () const {return CreateCurveVector ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GPArraySegmentLengths::GPArraySegmentLengths (GPArrayCR gpa)
    {
    int             i0, i1, curveType;
    double          length = 0.0;


    for (i0 = 0; jmdlGraphicsPointArray_parseFragment (&gpa, &i1,  NULL, NULL,  &curveType, i0); i0 = i1 + 1)
        {
        int iRead = i0;

        insert (T_SegmentLengthPair (i0, length));

        switch (curveType)
            {
            case 0:
                {
                const GraphicsPoint *pGP = jmdlGraphicsPointArray_getConstPtr (&gpa, 0);
                DPoint3d basePoint, currPoint;

                /* look for the first real point: */
                while (iRead <= i1 && !bsiDPoint4d_normalize (&pGP[iRead].point, &basePoint))
                    iRead++;

                while (++iRead <= i1)
                    {
                    if (bsiDPoint4d_normalize (&pGP[iRead].point, &currPoint))
                        {
                        length += bsiDPoint3d_distance (&basePoint, &currPoint);
                        insert (T_SegmentLengthPair (iRead, length));

                        basePoint = currPoint;
                        }
                    }
                break;
                }

            default:
                length += gpa.PrimitiveLength (i0);
                break;
           }
        }
    insert (T_SegmentLengthPair (gpa.GetCount() - 1, length));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
double      GPArraySegmentLengths::LengthFromParam (GPArrayParamCR param) const
    {
    std::map<int,double>::const_iterator  thisLength = find (param.m_index);

    if (thisLength == end())
        {
        BeAssert (false);
        return 0.0;
        }
    std::map <int,double>::const_iterator  nextLength = thisLength;
    nextLength++;

    if (nextLength == end())
        return thisLength->second;

    return thisLength->second + param.m_param * (nextLength->second - thisLength->second);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GPArrayParam interpolateSubCurveParam (GPArrayParamCR inParam, GPArrayCR subCurve, GPArrayIntervalCR subCurveInterval)
    {
    double          segmentStart = 0.0, segmentEnd = 1.0;
    static double   s_nearZero = 1.0E-12, s_nearOne = 1.0 - s_nearZero;

    if (0 == inParam.m_index && inParam.m_param < s_nearOne)
        segmentStart = subCurveInterval.m_start.m_param;

    size_t      lastPrimitiveIndex;
    if (subCurve.GetLastPrimitiveIndex(lastPrimitiveIndex) && inParam.m_index == lastPrimitiveIndex && inParam.m_param > s_nearZero)
        segmentEnd = subCurveInterval.m_end.m_param;
 
    GPArrayParam    outParam;
    
    outParam.m_param = segmentStart + inParam.m_param * (segmentEnd - segmentStart);
    outParam.m_index = inParam.m_index + subCurveInterval.m_start.m_index;

    return outParam;
    }   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GPArrayInterval GPArrayInterval::InterpolateSubInterval (GPArrayIntervalCR interval, GPArrayIntervalCR subCurveInterval, GPArrayCR subCurve)
    {
    return GPArrayInterval (interpolateSubCurveParam (interval.m_start, subCurve, subCurveInterval), interpolateSubCurveParam (interval.m_end, subCurve, subCurveInterval), NULL);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GPArrayParam::GPArrayParam (double value)
    {
    m_index = (uint32_t) value; 
    m_param = fmod (value, 1.0); 

    if (m_index > 0 && 0.0 == m_param)
        {
        m_index--;
        m_param = 1.0; 
        }
    }
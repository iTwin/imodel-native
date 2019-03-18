/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/ICurvePrimitive.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <../SolidPrimitive/BoxFaces.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitive::CurvePrimitiveType ICurvePrimitive::_GetCurvePrimitiveType () const    { return CURVE_PRIMITIVE_TYPE_Invalid; }
void ICurvePrimitive::SetCurvePrimitiveInfo (ICurvePrimitiveInfoPtr data)               { m_info = data; }
ICurvePrimitiveInfoPtr const& ICurvePrimitive::GetCurvePrimitiveInfo () const           { return m_info; }
ICurvePrimitiveInfoPtr& ICurvePrimitive::GetCurvePrimitiveInfoW ()                      { return m_info; }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2012
+--------------------------------------------------------------------------------------*/
FacetEdgeLocationDetailVectorPtr ICurvePrimitive::GetFacetEdgeLocationDetailVectorPtr () const
    {
    if (m_info.IsValid ())
        return dynamic_cast <FacetEdgeLocationDetailVector*> (m_info.get ());
    return NULL;        
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2012
+--------------------------------------------------------------------------------------*/
CurvePrimitiveIdCP ICurvePrimitive::GetId() const                          { return m_id.get(); }
void ICurvePrimitive::SetId(CurvePrimitiveIdP id) const                    { m_id = id; }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DSegment3dCP ICurvePrimitive::_GetLineCP () const                {return NULL;}
bvector<DPoint3d> const*            ICurvePrimitive::_GetLineStringCP () const          {return NULL;}
bvector<DPoint3d> *                 ICurvePrimitive::_GetLineStringP ()                 {return NULL;}

DEllipse3dCP ICurvePrimitive::_GetArcCP () const                                        {return NULL;}
MSBsplineCurveCP ICurvePrimitive::_GetBsplineCurveCP () const                           {return NULL;}
MSBsplineCurvePtr ICurvePrimitive::_GetBsplineCurvePtr () const                         {return NULL;}
MSBsplineCurveCP ICurvePrimitive::_GetProxyBsplineCurveCP () const                      {return NULL;}
MSBsplineCurvePtr ICurvePrimitive::_GetProxyBsplineCurvePtr () const                    {return NULL;}
 MSInterpolationCurveCP ICurvePrimitive::_GetInterpolationCurveCP () const              {return NULL;}
bvector<DPoint3d> const*            ICurvePrimitive::_GetAkimaCurveCP () const          {return NULL;}
bvector<DPoint3d> const*            ICurvePrimitive::_GetPointStringCP () const         {return NULL;}
CurveVector const*                  ICurvePrimitive::_GetChildCurveVectorCP () const    {return NULL;}
CurveVectorPtr                      ICurvePrimitive::_GetChildCurveVectorP ()           {return NULL;}
bool                      ICurvePrimitive::_TryGetCatenary (DCatenary3dPlacementR data)  const {return false;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DSpiral2dPlacementCP ICurvePrimitive::_GetSpiralPlacementCP () const     {return NULL;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PartialCurveDetailCP ICurvePrimitive::_GetPartialCurveDetailCP () const  {return NULL;}

bool ICurvePrimitive::_TryGetPartialCurveData (double &fractionA, double &fractionB, ICurvePrimitivePtr &parent, int64_t &tag) const
    {
    fractionA = fractionB = 0.0;  parent = NULL; tag = 0; return false;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitive::ICurvePrimitive() : m_markerBits (0) {}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ICurvePrimitive::FinishClone (ICurvePrimitiveCR parent)
    {
    m_markerBits            = parent.m_markerBits;
    m_info                  = parent.m_info;
    m_id                    = parent.m_id;
    m_tag                   = parent.m_tag;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::Clone () const
    {
    ICurvePrimitivePtr result = _Clone ();
    if (result.IsValid ())
        result->FinishClone (*this);
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CloneComponent (ptrdiff_t componentIndex) const
    {
    ICurvePrimitivePtr result = _CloneComponent (componentIndex);
    if (result.IsValid ())
        result->FinishClone (*this);
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
int64_t ICurvePrimitive::GetTag () const  {return m_tag;}
int ICurvePrimitive::GetIntTag () const  {return (int)m_tag;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ICurvePrimitive::SetTag (int64_t tag)  {m_tag = tag;}
void ICurvePrimitive::SetTag (int tag) {m_tag = (int64_t) tag;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::GetMarkerBit (ICurvePrimitive::CurvePrimitiveMarkerBit select) const
    {
    return 0 != (select & m_markerBits);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ICurvePrimitive::SetMarkerBit (ICurvePrimitive::CurvePrimitiveMarkerBit select, bool value)
    {
    if (value)
        {
        m_markerBits |= select;
        }
    else
        {
        m_markerBits &= ~select;
        }
    }

// base implemetation of partial clone applies shared clamp logic.
// Derived classes can assume fractions are pre-clamped and not equal.
ICurvePrimitivePtr ICurvePrimitive::CloneBetweenFractions (double fractionA, double fractionB, bool allowExtension) const
    {
    double df = fractionB - fractionA;
    double period;
    if (df == 0.0)
        return NULL;
    if (allowExtension && IsPeriodicFractionSpace (period))
        {
        // leave fractions alone.
        }
    else if (!allowExtension || !IsExtensibleFractionSpace ())
        {
        DoubleOps::ClampDirectedFractionInterval (fractionA, fractionB);
        }
    if (DoubleOps::AlmostEqual (fractionA, fractionB))
        return NULL;
    bool isFullClone = DoubleOps::AlmostEqual (fractionA, 0.0) && DoubleOps::AlmostEqual (fractionB, 1.0);
       
    ICurvePrimitivePtr result = isFullClone ? _Clone () : _CloneBetweenFractions (fractionA, fractionB, allowExtension);
    if (result.IsValid ())
    result->FinishClone (*this);
    return result;
    }


// public/private transition stubs ....
bool ICurvePrimitive::FractionToPoint (double fraction, DPoint3dR point) const {return _FractionToPoint (fraction, point);}
bool ICurvePrimitive::FractionToPoint (double fraction, CurveLocationDetailR data) const {return _FractionToPoint (fraction, data);}

bool ICurvePrimitive::FractionToPointWithTwoSidedDerivative (double fraction, DPoint3dR point, DVec3dR tangentA, DVec3dR tangentB) const {return _FractionToPointWithTwoSidedDerivative (fraction, point, tangentA, tangentB);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::FractionToPoint (double fraction, DPoint3dR point, DVec3dR tangent) const {return _FractionToPoint (fraction, point, tangent);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/2015
+--------------------------------------------------------------------------------------*/
ValidatedDRay3d ICurvePrimitive::FractionToPointAndUnitTangent (double fraction) const
    {
    DRay3d ray;
    ray.origin.Zero ();
    ray.direction.Zero ();
    if (_FractionToPoint (fraction, ray.origin, ray.direction))
        {
        return ray.ValidatedNormalize ();
        }
    return ValidatedDRay3d (ray, false);
    }


// public/private transition stubs ....
bool ICurvePrimitive::ComponentFractionToPoint (ptrdiff_t componentIndex, double fraction, DPoint3dR point) const {return _ComponentFractionToPoint (componentIndex, fraction, point);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::ComponentFractionToPoint (ptrdiff_t componentIndex, double fraction, DPoint3dR point, DVec3dR tangent) const {return _ComponentFractionToPoint (componentIndex, fraction, point, tangent);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::FractionToPoint (double fraction, DRay3dR ray) const {return _FractionToPoint (fraction, ray.origin, ray.direction);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::FractionToPoint (double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const {return _FractionToPoint (fraction, point, tangent, derivative2);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::FractionToPoint (double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const {return _FractionToPoint (fraction, point, tangent, derivative2, derivative3);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::FractionToFrenetFrame (double f, TransformR frame) const {return _FractionToFrenetFrame (f, frame);}
bool ICurvePrimitive::FractionToFrenetFrame (double f, TransformR frame, double &curvature, double &torsion) const {return _FractionToFrenetFrame (f, frame, curvature, torsion);}

ValidatedTransform ICurvePrimitive::FractionToFrenetFrame (double f) const
    {
    Transform frame;
    if (_FractionToFrenetFrame (f, frame))
        return ValidatedTransform (frame, true);
    return ValidatedTransform (Transform::FromIdentity (), false);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::Length(double &length) const {return _Length (length);}
bool ICurvePrimitive::Length(RotMatrixCP worldToLocal, double &length) const {return _Length (worldToLocal, length);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2013
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::FastLength(double &length) const {return _FastLength (length);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::GetRange (DRange3dR range) const {return _GetRange (range);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::GetRange (DRange3dR range, TransformCR transform) const {return _GetRange (range, transform);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::GetMSBsplineCurve (MSBsplineCurveR curve, double fractionA, double fractionB) const
    {return _GetMSBsplineCurve (curve, fractionA, fractionB);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/2014
+--------------------------------------------------------------------------------------*/
MSBsplineCurvePtr ICurvePrimitive::GetMSBsplineCurvePtr (double fractionA, double fractionB) const
    {
    MSBsplineCurve bcurve;
    if (_GetMSBsplineCurve (bcurve, fractionA, fractionB))
        {
        return bcurve.CreateCapture ();
        }
    return NULL;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double ICurvePrimitive::FastMaxAbs () const {return _FastMaxAbs ();}
size_t ICurvePrimitive::NumComponent () const {return _NumComponent ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::IsSameStructure (ICurvePrimitiveCR other) const 
        {
        CurvePrimitiveType myType = GetCurvePrimitiveType ();
        if (myType != other.GetCurvePrimitiveType ())
            return false;
        if (myType == CURVE_PRIMITIVE_TYPE_CurveVector)
            {
            // Recurse to both curve vectors...
            CurveVectorCP myChild = GetChildCurveVectorCP ();
            return myChild->IsSameStructure (*other.GetChildCurveVectorCP ());
            }
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool CurveVector::IsSameStructure (CurveVectorCR other) const
    {
    size_t n0 = size ();
    size_t n1 = other.size ();
    if (n0 != n1)
        return false;
    for (size_t i = 0; i < n0; i++)
        {
        if (!at(i)->IsSameStructure (*other.at(i)))
            return false;
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const 
        {
        // Enforce hard structure requirement directly.
        // Can the virtuals assume "other" matches? Maybe.
        CurvePrimitiveType myType = GetCurvePrimitiveType ();
        if (myType != other.GetCurvePrimitiveType ())
            return false;
        return _IsSameStructureAndGeometry (other, tolerance);
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool CurveVector::IsSameStructureAndGeometry (CurveVectorCR other, double tolerance) const
    {
    size_t n0 = size ();
    size_t n1 = other.size ();
    if (GetBoundaryType () != other.GetBoundaryType ())
        return false;
    if (n0 != n1)
        return false;
    for (size_t i = 0; i < n0; i++)
        {
        if (!at(i)->IsSameStructureAndGeometry (*other.at(i), tolerance))
            return false;
        }
    return true;
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2012
+--------------------------------------------------------------------------------------*/
DRange1d ICurvePrimitive::ProjectedParameterRange (DRay3dCR ray) const {return _ProjectedParameterRange (ray);}
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2012
+--------------------------------------------------------------------------------------*/
DRange1d ICurvePrimitive::ProjectedParameterRange (DRay3dCR ray, double fraction0, double fraction1) const {return _ProjectedParameterRange (ray, fraction0, fraction1);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::GetBreakFraction (size_t breakFractionIndex, double &fraction) const {return _GetBreakFraction (breakFractionIndex, fraction);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::AdjustFractionToBreakFraction (double fraction, Rounding::RoundingMode mode, size_t &breakIndex, double &adjustedFraction) const {return _AdjustFractionToBreakFraction (fraction, mode, breakIndex, adjustedFraction);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::SignedDistanceBetweenFractions (double startFraction, double endFraction, double &signedDistance) const {return _SignedDistanceBetweenFractions (startFraction, endFraction, signedDistance);}
bool ICurvePrimitive::SignedDistanceBetweenFractions (RotMatrixCP worldToLocal, double startFraction, double endFraction, double &signedDistance) const {return _SignedDistanceBetweenFractions (worldToLocal, startFraction, endFraction, signedDistance);}

//!
bool ICurvePrimitive::PointAtSignedDistanceFromFraction_go
(
RotMatrixCP worldToLocal,
double startFraction,
double signedDistance,
bool allowExtension,
CurveLocationDetailR location
) const
        {
        if (!allowExtension)
            {
            if (startFraction < 0.0)
                startFraction = 0.0;
            if (startFraction > 1.0)
                startFraction = 1.0;
            }
        bool stat =
                worldToLocal == nullptr 
                    ? _PointAtSignedDistanceFromFraction (startFraction, signedDistance, allowExtension, location)
                    : _PointAtSignedDistanceFromFraction (worldToLocal, startFraction, signedDistance, allowExtension, location);

        if (stat)
            {
            if (!allowExtension && (location.fraction > 1.0 || location.fraction < 0.0))
                {
                // Force it fraction and distance back to the endpoint.
                if (location.fraction >= 0.5)
                    location.fraction = location.componentFraction = 1.0;
                else
                    location.fraction = location.componentFraction = 0.0;
                if (nullptr == worldToLocal)
                    _SignedDistanceBetweenFractions (startFraction, location.fraction, location.a);
                else
                    _SignedDistanceBetweenFractions (worldToLocal, startFraction, location.fraction, location.a);
                _FractionToPoint (location.fraction, location.point);
                }
            }
        return stat;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::PointAtSignedDistanceFromFraction (RotMatrixCP worldToLocal, double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const
    {
    return PointAtSignedDistanceFromFraction_go (worldToLocal, startFraction, signedDistance, allowExtension, location);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::PointAtSignedDistanceFromFraction (double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const
    {
    return PointAtSignedDistanceFromFraction_go (nullptr, startFraction, signedDistance, allowExtension, location);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::ClosestPointBounded (DPoint3dCR spacePoint, double &fraction, DPoint3dR curvePoint) const
                {return _ClosestPointBounded (spacePoint, fraction, curvePoint, false, false);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::ClosestPointBounded (DPoint3dCR spacePoint, CurveLocationDetailR location) const {return _ClosestPointBounded (spacePoint, location, false, false);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::ClosestPointBounded (DPoint3dCR spacePoint, CurveLocationDetailR location, bool extend0, bool extend1) const {return _ClosestPointBounded (spacePoint, location, extend0, extend1);}

void ICurvePrimitive::_AnnounceKeyPoints
(
DPoint3dCR spacePoint,                       //!< [in] point to project
CurveKeyPointCollector &collector,           //!< [in] object to receive keypoint announcements
bool extend0,
bool extend1
) const
    {
    auto child = (const_cast<ICurvePrimitive*>(this))->GetChildCurveVectorP ();
    if (child.IsValid ())
        child->AnnounceKeyPoints (spacePoint, collector, extend0, extend1);
    //BeAssert (false);   // unimplemented _AnnounceKeyPoints
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ICurvePrimitive::AnnounceKeyPoints
(
DPoint3dCR spacePoint,                       //!< [in] point to project
CurveKeyPointCollector &collector,           //!< [in] object to receive keypoint announcements
bool extend0,
bool extend1
) const
    {
    CurveLocationDetail detail;
    if (collector.NeedKeyPointType (CurveKeyPointCollector::KeyPointType::EndPoint))
        {
        FractionToPoint (0.0, detail);
        collector.AnnouncePoint (detail, CurveKeyPointCollector::KeyPointType::EndPoint);
        FractionToPoint (1.0, detail);
        collector.AnnouncePoint (detail, CurveKeyPointCollector::KeyPointType::EndPoint);
        }
    _AnnounceKeyPoints (spacePoint, collector, extend0, extend1);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CloneAsSingleOffsetPrimitiveXY (CurveOffsetOptionsCR options) const {return _CloneAsSingleOffsetPrimitiveXY (options);}


ICurvePrimitive::CurvePrimitiveType   ICurvePrimitive::GetCurvePrimitiveType () const   {return _GetCurvePrimitiveType ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DSegment3dCP ICurvePrimitive::GetLineCP () const                       {return _GetLineCP ();}
bvector<DPoint3d> const*  ICurvePrimitive::GetLineStringCP ()  const                {return _GetLineStringCP ();}
bvector<DPoint3d> *       ICurvePrimitive::GetLineStringP ()                        {return _GetLineStringP ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3dCP ICurvePrimitive::GetArcCP () const                        {return _GetArcCP ();}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::TryGetLine (DSegment3dR dest) const
    {
    DSegment3dCP sourceP = GetLineCP ();
    if (NULL == sourceP)
        return false;
    dest = *sourceP;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::TryGetCatenary (DCatenary3dPlacementR data) const
    {
    return _TryGetCatenary (data);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::TryGetPoint (DPoint3dR xyz) const
    {
    DSegment3d segment;
    if (TryGetLine (segment))
        {
        if (segment.IsSinglePoint ())
            {
            xyz = segment.point[0];
            return true;
            }
        }
    xyz.Zero ();
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::TryAddLineStringPoint (DPoint3dCR xyz)
    {
    bvector<DPoint3d> *linestring = _GetLineStringP ();
    if (NULL != linestring)
        {
        linestring->push_back (xyz);
        return false;
        }
    return true;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::TryGetSegmentInLineString (DSegment3dR segment, size_t index0) const
    {
    size_t index1 = index0 + 1;
    bvector<DPoint3d> const*linestring = _GetLineStringCP ();
    if (NULL != linestring
        && index1 < linestring->size ())
        {
        segment.Init (linestring->at (index0), linestring->at (index1));
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::TryGetArc  (DEllipse3dR dest) const
    {
    DEllipse3dCP sourceP = GetArcCP ();
    if (NULL == sourceP)
        return false;
    dest = *sourceP;
    return true;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
MSBsplineCurveCP ICurvePrimitive::GetBsplineCurveCP () const               {return _GetBsplineCurveCP ();}
MSBsplineCurvePtr ICurvePrimitive::GetBsplineCurvePtr () const               {return _GetBsplineCurvePtr ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
MSBsplineCurveCP ICurvePrimitive::GetProxyBsplineCurveCP  () const          {return _GetProxyBsplineCurveCP ();}
MSBsplineCurvePtr ICurvePrimitive::GetProxyBsplineCurvePtr () const          {return _GetProxyBsplineCurvePtr ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
MSInterpolationCurveCP ICurvePrimitive::GetInterpolationCurveCP () const    {return _GetInterpolationCurveCP ();}
bvector<DPoint3d> const*  ICurvePrimitive::GetAkimaCurveCP () const         {return _GetAkimaCurveCP ();}
bvector<DPoint3d> const*  ICurvePrimitive::GetPointStringCP () const        {return _GetPointStringCP ();}
CurveVectorCP ICurvePrimitive::GetChildCurveVectorCP () const               {return _GetChildCurveVectorCP ();}
CurveVectorPtr ICurvePrimitive::GetChildCurveVectorP ()                     {return _GetChildCurveVectorP ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DSpiral2dPlacementCP ICurvePrimitive::GetSpiralPlacementCP () const            {return _GetSpiralPlacementCP ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PartialCurveDetailCP ICurvePrimitive::GetPartialCurveDetailCP () const         {return _GetPartialCurveDetailCP ();}
bool ICurvePrimitive::TryGetPartialCurveData (double &fractionA, double &fractionB, ICurvePrimitivePtr &parent, int64_t &tag) const
            {return _TryGetPartialCurveData (fractionA, fractionB, parent, tag);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::AddStrokes (bvector <DPoint3d> &points, IFacetOptionsCR options,
                bool includeStartPoint,
                double startFraction,
                double endFraction
                ) const
    {
    return _AddStrokes (points, options, includeStartPoint, startFraction, endFraction);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::AddStrokes (DPoint3dDoubleUVCurveArrays &points, IFacetOptionsCR options,
                double startFraction,
                double endFraction
                ) const
    {
    return _AddStrokes (points, options, startFraction, endFraction);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::AddStrokes (bvector <PathLocationDetail> &points, IFacetOptionsCR options,
                double startFraction,
                double endFraction
                ) const
    {
    return _AddStrokes (points, options, startFraction, endFraction);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t ICurvePrimitive::GetStrokeCount (IFacetOptionsCR options, double startFraction, double endFraction) const
    {
    return _GetStrokeCount (options, startFraction, endFraction);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_TransformInPlace (TransformCR transform)
    {
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ICurvePrimitive::_AppendCurvePlaneIntersections (DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tolerance) const
    {
    // No intersections we know about here !!!
    }

static void TryCurveRangeIntersectionInterval
(
ICurvePrimitiveP curve,
double f0,
double f1,
LocalRangeCR range,
bvector<PartialCurveDetail> &intersections
)
    {
    DPoint3d xyz;
    if (DoubleOps::ClearlyIncreasingFraction (f0, f1)
        && curve->FractionToPoint (0.5*(f0+f1), xyz))
        {
        range.m_worldToLocal.Multiply (xyz);
        if (range.m_localRange.IsContained (xyz))
            intersections.push_back (PartialCurveDetail (curve, f0, f1, 0));
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2015
+--------------------------------------------------------------------------------------*/
void ICurvePrimitive::_AppendCurveRangeIntersections
(
LocalRangeCR range,                         //!< [in] transformed range.
bvector<PartialCurveDetail> &intersections     //!< intersections. 
) const
    {
    // Default implementation ... make planes .....
    BoxFaces faces;
    faces.Load (range.m_localRange);
    faces.MultiplyCorners (range.m_localToWorld);
    // ASSUME ... positie determinant !!!!
    bvector<CurveLocationDetailPair> planePairs;
    bvector<double> parameters;
    for (int i = 0; i < 6; i++)
        {
        auto plane = faces.GetFacePlane (i);
        if (plane.IsValid ())
            { 
            AppendCurvePlaneIntersections (plane, planePairs);
            }
        }
    // Those intersections may have "partially on" parts.  flatten the details so they can get sorted without regard to that ...
    bvector<CurveLocationDetail> planeCuts;
    for (auto &pair : planePairs)
        {
        planeCuts.push_back (pair.detailA);
        if (!pair.SameCurveAndFraction ())
            planeCuts.push_back (pair.detailB);
        }
    CurveLocationDetail::SortByCurveAndFraction (planeCuts);
    for (size_t i1 = 0, i0 = 0; i0 < planeCuts.size (); i0 = i1)
        {
        // delimit the block with the same curve primitive ...
        for (i1 = i0 + 1; i1 < planeCuts.size () && planeCuts[i0].curve == planeCuts[i1].curve;)
            {
            i1++;
            }
        // The planeCuts array does NOT have all 0,1 points ... need to force both end intervals ..
        double f1, f0 = 0.0;
        ICurvePrimitiveP curve = const_cast<ICurvePrimitiveP>(planeCuts[i0].curve);
        for (size_t i = i0; i < i1; i++)
            {
            f1 = planeCuts[i].fraction;
            TryCurveRangeIntersectionInterval (curve, f0, f1, range, intersections);
            f0 = f1;
            }
        f1 = 1.0;
        TryCurveRangeIntersectionInterval (curve, f0, f1, range, intersections);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2015
+--------------------------------------------------------------------------------------*/
void ICurvePrimitive::_AppendCurveBilinearPatchIntersections
(
DBilinearPatch3dCR patch,                         //!< [in] patch to intesect
bvector<CurveAndSolidLocationDetail> &intersections     //!< [in,out] intersections. 
) const
    {
    // Triangle on near part of patch ...
    DPoint3dDVec3dDVec3d triangleA = DPoint3dDVec3dDVec3d::FromOriginAndTargets (patch.point[0][0], patch.point[1][0], patch.point[0][1]);
    AppendCurvePlaneIntersections (triangleA, UVBoundarySelect (UVBoundarySelect::Triangle), intersections);
    size_t numA = intersections.size ();
    DPoint3dDVec3dDVec3d triangleB = DPoint3dDVec3dDVec3d::FromOriginAndTargets (patch.point[1][1], patch.point[0][1], patch.point[1][0]);
    // Triangle on far part of patch ... uv has to be reampped ...
    AppendCurvePlaneIntersections (triangleB, UVBoundarySelect (UVBoundarySelect::Triangle), intersections);
    for (size_t i = numA; i < intersections.size (); i++)
        {
        DPoint2d uv0 = intersections[i].m_solidDetail.GetUV ();
        DVec3d uDirection = intersections[i].m_solidDetail.GetUDirection ();
        uDirection.Negate ();
        DVec3d vDirection = intersections[i].m_solidDetail.GetVDirection ();
        vDirection.Negate ();
        // mirror the uv coordinates ...
        DPoint2d uv1 = DPoint2d::From (1.0 - uv0.x, 1.0 - uv0.y);
        intersections[i].m_solidDetail.SetUV (uv1.x, uv1.y, uDirection, vDirection);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2015
+--------------------------------------------------------------------------------------*/
void ICurvePrimitive::_AppendCurvePlaneIntersections
(
DPoint3dDVec3dDVec3dCR plane,                         //!< [in] patch to intesect
UVBoundarySelect bounded,                    //!< [in] Unbounded, Triangle, Square
bvector<CurveAndSolidLocationDetail> &intersections     //!< [in,out] intersections. 
) const
    {
    Transform localToWorld, worldToLocal;
    bvector <CurveLocationDetailPair> unboundedPlaneIntersections;
    if (plane.GetTransformsUnitZ (localToWorld, worldToLocal))
        {
        DPlane3d zPlane;
        zPlane.origin = DVec3d::FromTranslation (localToWorld);
        zPlane.normal = DVec3d::FromMatrixColumn (localToWorld, 2);
        _AppendCurvePlaneIntersections (zPlane, unboundedPlaneIntersections, 0.0);
        for (auto &planeCut : unboundedPlaneIntersections)
            {
            DPoint3d localPoint;
            CurveLocationDetail curveDetail = planeCut.detailA;
            // ugh ... our output does not tolerance "on" sections ...
            if (planeCut.SameCurveAndFraction ())
                {
                worldToLocal.Multiply (localPoint, curveDetail.point);
                if (bounded.IsInOrOn (localPoint.x, localPoint.y))
                    {
                    intersections.push_back (CurveAndSolidLocationDetail (                        
                        curveDetail,
                        SolidLocationDetail (0, curveDetail.a, curveDetail.point, localPoint.x, localPoint.y, plane.vectorU, plane.vectorV)
                        ));
                    }
                }
            }
        }
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_ClosestPointBoundedXY (DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location, bool extend0, bool extend1) const
    {
    location = CurveLocationDetail ();
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_ClosestPointBounded (DPoint3dCR spacePoint, double &fraction, DPoint3dR point, bool extend0, bool extend1) const
    {
    fraction = 0.0;
    point.Zero ();
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_GetStartEnd (DPoint3dR pointA, DPoint3dR pointB) const
    {
    return     _FractionToPoint (0.0, pointA)
            && _FractionToPoint (1.0, pointB);
    }

bool ICurvePrimitive::_FractionToPointWithTwoSidedDerivative (double f, DPoint3dR point, DVec3dR tangentA, DVec3dR tangentB) const
    {
    bool stat = _FractionToPoint (f, point, tangentA);
    tangentB = tangentA;
    return stat;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_TrySetStart (DPoint3dCR xyz)
    {
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_TrySetEnd (DPoint3dCR xyz)
    {
    return false;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_GetStartEnd (DPoint3dR pointA, DPoint3dR pointB, DVec3dR unitTangentA, DVec3dR unitTangentB) const
    {
    DVec3d tangentA, tangentB;
    if (  _FractionToPoint (0.0, pointA, tangentA)
       && _FractionToPoint (1.0, pointB, tangentB))
        {
        unitTangentA.Normalize (tangentA);
        unitTangentB.Normalize (tangentB);
        return true;
        }
    pointA.Zero ();
    pointB.Zero ();
    unitTangentA.Zero ();
    unitTangentB.Zero ();
    return false;
    }

// DEFAULT IPMLEMENTATIONS (FAILURES)
bool ICurvePrimitive::_FractionToPoint (double f, DPoint3dR point) const
    {
    point.Zero ();
    return false;
    }
// Default implementation appropriate for all "single component" curves ...
bool ICurvePrimitive::_FractionToPoint (double f, CurveLocationDetail &detail) const
    {
    DPoint3d xyz;
    if (FractionToPoint (f, xyz))
        {
        detail = CurveLocationDetail (this, f, xyz);
        return true;
        }
    detail = CurveLocationDetail (this);
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_FractionToPoint (double f, DPoint3dR point, DVec3dR tangent) const
    {
    point.Zero ();
    tangent.Zero ();
    return false;
    }

// DEFAULT IPMLEMENTATIONS (ACT AS IF SINGLE COMPONENT)
bool ICurvePrimitive::_ComponentFractionToPoint (ptrdiff_t componentIndex, double f, DPoint3dR point) const {return _FractionToPoint (f, point);}
bool ICurvePrimitive::_ComponentFractionToPoint (ptrdiff_t componentIndex, double f, DPoint3dR point, DVec3dR tangent) const { return _FractionToPoint (f, point, tangent);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_FractionToPoint (double f, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const
    {
    point.Zero ();
    tangent.Zero ();
    derivative2.Zero ();
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_FractionToPoint (double f, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const
    {
    point.Zero ();
    tangent.Zero ();
    derivative2.Zero ();
    derivative3.Zero ();
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_FractionToFrenetFrame (double f, TransformR frame) const
    {
    DPoint3d    point;
    DVec3d      deriv1, deriv2;
    if (_FractionToPoint (f, point, deriv1, deriv2))
        {
        RotMatrix rMatrix1, rMatrix2;
        rMatrix1.InitFrom2Vectors(deriv1, deriv2);
        if (rMatrix2.SquareAndNormalizeColumns(rMatrix1, 0, 1))
            {
            frame.InitFrom(rMatrix2, point);
            return true;
            }
        // The derivatives are zero or parallel.  Take the first nonzero derivative as xVec ...
        DVec3d xVec, yVec, zVec;
        if (deriv1.GetNormalizedTriad (yVec, zVec, xVec)
            || deriv2.GetNormalizedTriad(yVec, zVec, xVec))
            {
            frame = Transform::FromOriginAndVectors (point, xVec, yVec, zVec);
            return true;
            }
        // Capture the origin ..
        frame = Transform::From (point);
        return false;
        }
    frame.FromIdentity ();
    return false;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
 see for instance https://en.wikipedia.org/wiki/Frenet%E2%80%93Serret_formulas
 http://www.cs.indiana.edu/pub/techreports/TR407.pdf

 curvature = mag(X' cross X'')/(mag X')^3
 torsion   = triple (X', X'', X''')/ (mag (X' cross X'')^2
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_FractionToFrenetFrame (double f, TransformR frame, double &curvature, double &torsion) const
    {
    DPoint3d    point;
    DVec3d      deriv1, deriv2, deriv3;
    curvature = torsion = 0.0;
    if (!_FractionToPoint (f, point, deriv1, deriv2, deriv3))
        {
        frame.FromIdentity ();
        return false;
        }
    
    RotMatrix rMatrix1, rMatrix2;
    rMatrix1.InitFrom2Vectors (deriv1, deriv2);
    rMatrix2.SquareAndNormalizeColumns (rMatrix1, 0, 1);
    frame.InitFrom (rMatrix2, point);
    double a = deriv1.Magnitude ();
    double a3 = a * a * a;
    DVec3d cross12 = DVec3d::FromCrossProduct (deriv1, deriv2);
    double tripleProduct = cross12.DotProduct (deriv3);
    double crossmag2 = cross12.MagnitudeSquared ();
    double crossmag  = sqrt (crossmag2);
    DoubleOps::SafeDivide (curvature, crossmag, a3, 0.0);
    DoubleOps::SafeDivide (torsion, tripleProduct, crossmag2, 0.0);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_Length (double &length) const
    {
    length = 0.0;
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/15
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_Length(RotMatrixCP worldToLocal, double &length) const 
    {
    length = 0.0;
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2013
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_FastLength (double &length) const
    {
    // Primitives can just allow fast length to be the usual _Length.
    // Definitely ok for lineSegment and LineString.
    // BsplineCurve should definitley override and do polygon length.
    // Ellipse ??? Hmmm.. Hard call.   If (a) circles are special cased and (b) true ellipses
    //    go to the NumericalRecipes mystical iteration, there's not much to be gained.
    return _Length (length);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_SignedDistanceBetweenFractions (double startFraction, double endFraction, double &signedDistance) const
    {
    signedDistance = 0.0;
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_SignedDistanceBetweenFractions (RotMatrixCP worldToLocal, double startFraction, double endFraction, double &signedDistance) const
    {
    signedDistance = 0.0;
    return false;
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::_CloneAsSingleOffsetPrimitiveXY (CurveOffsetOptionsCR options) const
    {
    MSBsplineCurveCP bcurve = GetProxyBsplineCurveCP ();
    if (nullptr != bcurve)
        {
        double offsetDistance = options.GetOffsetDistance ();
        MSBsplineCurvePtr offsetBCurve = bcurve->CreateCopyOffsetXY (offsetDistance, offsetDistance, options);
        if (offsetBCurve.IsValid ())
            return ICurvePrimitive::CreateBsplineCurve (offsetBCurve);
        }
    return nullptr;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/13
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::_CloneComponent (ptrdiff_t componentIndex) const
    {
    return _Clone ();       // This has to go to the virtual, else FinishClone gets called twice.  (Harmless if just copying simple data.)
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_PointAtSignedDistanceFromFraction (double startFraction, double signedLength, bool allowExtension, CurveLocationDetailR location) const
    {
    location = CurveLocationDetail ();
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_PointAtSignedDistanceFromFraction (RotMatrixCP worldToLocal, double startFraction, double signedLength, bool allowExtension, CurveLocationDetailR location) const
    {
    location = CurveLocationDetail ();
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_GetRange (DRange3dR range) const 
    {
    range.Init ();
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_GetRange (DRange3dR range, TransformCR transform) const 
    {
    range.Init ();
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double ICurvePrimitive::_FastMaxAbs () const 
    {
    DRange3d range;
    if (GetRange (range))
        return range.LargestCoordinate ();
    return 0.0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2012
+--------------------------------------------------------------------------------------*/
DRange1d ICurvePrimitive::_ProjectedParameterRange (DRay3dCR ray) const 
    {
    MSBsplineCurveCP bcurve = GetProxyBsplineCurveCP ();
    if (NULL != bcurve)
        return bcurve->GetRangeOfProjectionOnRay (ray, 0.0, 1.0);
    return DRange1d::NullRange ();
    }
     
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2012
+--------------------------------------------------------------------------------------*/
DRange1d ICurvePrimitive::_ProjectedParameterRange (DRay3dCR ray, double fractionA, double fractionB) const 
    {
    MSBsplineCurveCP bcurve = GetProxyBsplineCurveCP ();
    if (NULL != bcurve)
        return bcurve->GetRangeOfProjectionOnRay (ray, fractionA, fractionB);
    return DRange1d::NullRange ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_GetBreakFraction (size_t breakFractionIndex, double &fraction) const 
    {
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_AdjustFractionToBreakFraction (double fraction, Rounding::RoundingMode mode, size_t &breakIndex, double &adjustedFraction) const
    {
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_GetMSBsplineCurve (MSBsplineCurveR curve, double fractionA, double fractionB) const
    {
    auto cp = _GetProxyBsplineCurveCP ();
    if (cp != nullptr)
        {
        return SUCCESS == curve.CopySegment (*cp,
            cp->FractionToKnot (fractionA), cp->FractionToKnot (fractionB));
        }
    curve.Zero ();
    return false;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_IsExtensibleFractionSpace () const { return false;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_IsPeriodicFractionSpace (double &period) const {period = 0.0; return false;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_IsMappableFractionSpace () const {return false;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_IsFractionSpace () const {return false;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_WireCentroid (double &length, DPoint3dR centroid, double fraction0, double fraction1) const
    {return false;}



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::ClosestPointBoundedXY (DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location) const
        {return _ClosestPointBoundedXY (spacePoint, worldToLocal, location, false, false);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::ClosestPointBoundedXY (DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location, bool extend0, bool extend1) const
        {return _ClosestPointBoundedXY (spacePoint, worldToLocal, location, extend0, extend1);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::GetStartEnd (DPoint3dR pointA, DPoint3dR pointB) const  {return _GetStartEnd (pointA, pointB);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::TrySetStart (DPoint3dCR xyz) {return _TrySetStart (xyz);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::TrySetEnd   (DPoint3dCR xyz) {return _TrySetEnd (xyz);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::GetStartEnd (DPoint3dR pointA, DPoint3dR pointB, DVec3dR unitTangentA, DVec3dR unitTangentB) const
        {return _GetStartEnd (pointA, pointB, unitTangentA, unitTangentB);}

// Implementation of GetStartPoint at ICurvePrimitive level
// 1) recurse if needed.
// 2) otherwise evaluate at fraction 0.
// Note that we do not bother having GetStartPoint as a virtual implemented by line/linestring/ellipse/bspline.
// A virtual class would give a very minor performance benefit (e.g. line can copy its start point rather than evaluate with fraction 0)
//  but it just isn't forcing the code to all the classes.
bool ICurvePrimitive::GetStartPoint (DPoint3dR point) const
    {
    CurveVectorCP child = GetChildCurveVectorCP ();
    if (NULL != child)
        return child->GetStartPoint (point);
    return FractionToPoint (0.0, point);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::IsExtensibleFractionSpace () const  {return _IsExtensibleFractionSpace ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::IsMappableFractionSpace   () const  {return _IsMappableFractionSpace ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::IsFractionSpace () const  {return _IsFractionSpace ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::IsPeriodicFractionSpace (double &period) const  {return _IsPeriodicFractionSpace (period);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::TransformInPlace (TransformCR transform) {return _TransformInPlace (transform);}
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::ReverseCurvesInPlace () {return _ReverseCurvesInPlace ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ICurvePrimitive::AppendCurvePlaneIntersections (DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tolerance) const
        {_AppendCurvePlaneIntersections (plane, intersections, tolerance);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2015
+--------------------------------------------------------------------------------------*/
void ICurvePrimitive::AppendCurveRangeIntersections (LocalRangeCR range, bvector<PartialCurveDetail> &intersections) const
    {
    _AppendCurveRangeIntersections (range, intersections);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2015
+--------------------------------------------------------------------------------------*/
void ICurvePrimitive::AppendCurveBilinearPatchIntersections (DBilinearPatch3dCR patch, bvector<CurveAndSolidLocationDetail> &intersections) const
    {
    _AppendCurveBilinearPatchIntersections (patch, intersections);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2015
+--------------------------------------------------------------------------------------*/
void ICurvePrimitive::AppendCurvePlaneIntersections (DPoint3dDVec3dDVec3dCR plane, UVBoundarySelect bounded, bvector<CurveAndSolidLocationDetail> &intersections) const
    {
    _AppendCurvePlaneIntersections (plane, bounded, intersections);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_ClosestPointBounded (DPoint3dCR spacePoint, CurveLocationDetailR location, bool extend0, bool extend1) const
    {
    double fraction;
    DPoint3d curvePoint;
    location = CurveLocationDetail (this, 1);
    if (_ClosestPointBounded (spacePoint, fraction, curvePoint, extend0, extend1))
        {
        location = CurveLocationDetail (const_cast <ICurvePrimitive*>(this), fraction, curvePoint, 0, 1, fraction);
        location.SetDistanceFrom (spacePoint);
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::WireCentroid (double &length, DPoint3dR centroid, double fraction0, double fraction1) const
    {return _WireCentroid (length, centroid, fraction0, fraction1);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_AddStrokes (bvector <DPoint3d> &points, IFacetOptionsCR options,
                bool includeStartPoint,
                double startFraction,
                double endFraction
                ) const
    {
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_AddStrokes (DPoint3dDoubleUVCurveArrays &points, IFacetOptionsCR options,
                double startFraction,
                double endFraction
                ) const
    {
    size_t numStrokes = _GetStrokeCount (options, startFraction, endFraction);
    if (numStrokes < 1)
        return false;
    double df = 1.0 / numStrokes;
    DPoint3d xyz;
    DVec3d dxyz;
    for (size_t i = 0; i <= numStrokes; i++)
        {
        double f = i * df;
        FractionToPoint (f, xyz, dxyz);
        points.Add (xyz, f, dxyz, const_cast <ICurvePrimitiveP>(this));
        }
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::_AddStrokes (bvector <PathLocationDetail> &points, IFacetOptionsCR options,
                double startFraction,
                double endFraction
                ) const
    {
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t ICurvePrimitive::_GetStrokeCount (IFacetOptionsCR options, double startFraction, double endFraction) const
    {
    return 0;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ICurvePrimitive::Process (ICurvePrimitiveProcessor & processor) const
    {
    _Process (processor, NULL);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/15
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CloneAsBspline (double fraction0, double fraction1) const
    {
    MSBsplineCurve bcurve;
    if (GetMSBsplineCurve (bcurve))
        return ICurvePrimitive::CreateBsplineCurveSwapFromSource (bcurve);
    return nullptr;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/15
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::Clone (TransformCR transform) const
    {
    auto clone = Clone ();
    if (clone.IsValid ())
        clone->TransformInPlace (transform);
    return clone;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/15
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::Clone (DMatrix4dCR transform4d) const
    {
    // convert and return immediately if possible . .
    switch (GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3d segment;
            if (TryGetLine (segment))
                {
                transform4d.MultiplyAndRenormalize (segment.point, segment.point, 2);
                return ICurvePrimitive::CreateLine (segment);
                }
            }
            break;
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            DEllipse3d arcA, arcC;
            if (TryGetArc (arcA))
                {
                DConic4d conicA, conicB, conicC;
                bsiDConic4d_initFromDEllipse3d (&conicA, &arcA);
                bsiDConic4d_applyDMatrix4d (&conicB, &transform4d, &conicA);
                int curveType;
                RotMatrix basis;
                if (bsiDConic4d_getCommonAxes (&conicB, &conicC, &basis, &curveType) && curveType == 1)
                    {
                    conicC.center.GetXYZ (arcC.center);
                    conicC.vector0.GetXYZ (arcC.vector0);
                    conicC.vector90.GetXYZ (arcC.vector90);
                    arcC.start = conicC.start;
                    arcC.sweep = conicC.sweep;
                    return ICurvePrimitive::CreateArc (arcC);
                    }
                }
            break;
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d>const *pointA = GetLineStringCP ();
            if (pointA != nullptr && pointA->size () > 0)
                {
                bvector<DPoint3d> pointB = *pointA;
                transform4d.MultiplyAndRenormalize (&pointB[0], &pointB[0], (int)pointB.size ());
                return ICurvePrimitive::CreateLineString (pointB);
                }
            }
            break;
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            bvector<DPoint3d>const *pointA = GetPointStringCP ();
            if (pointA != nullptr && pointA->size () > 0)
                {
                bvector<DPoint3d> pointB = *pointA;
                transform4d.MultiplyAndRenormalize (&pointB[0], &pointB[0], (int)pointB.size ());
                return ICurvePrimitive::CreatePointString (pointB);
                }
            }
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
            {
            auto childA = GetChildCurveVectorCP ();
            if (childA != nullptr)
                {
                auto childB = childA->Clone (transform4d);
                return CreateChildCurveVector (childB);
                }
            return nullptr;
            }
        }
    // convert all others to bspline ..
    auto bcurvePrimitive = CloneAsBspline ();
    if (bcurvePrimitive.IsValid ())
        {
        auto bcurve = bcurvePrimitive->GetBsplineCurvePtr ();
        if (bcurve.IsValid ())
            {
            bcurve->TransformCurve4d (transform4d);
            return bcurvePrimitive;
            }
        }

    return nullptr;
    }


END_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#include "cpstructs.h"

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurvePrimitiveLine::CurvePrimitiveLine(DSegment3dCR segment) {m_segment = segment;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr CurvePrimitiveLine::_Clone() const  {return Create (m_segment);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr CurvePrimitiveLine::_CloneBetweenFractions
(
double fractionA,
double fractionB,
bool allowExtrapolation
) const 
    {
    DSegment3d segment1;
    m_segment.FractionParameterToPoint (segment1.point[0], fractionA);
    m_segment.FractionParameterToPoint (segment1.point[1], fractionB);
    return Create (segment1);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr CurvePrimitiveLine::_CloneAsSingleOffsetPrimitiveXY (CurveOffsetOptionsCR options) const
    {
    DVec3d tangent = DVec3d::FromStartEnd (m_segment.point[0], m_segment.point[1]);
    DVec3d perp;
    if (perp.UnitPerpendicularXY (tangent))
        {
        double d = options.GetOffsetDistance ();
        DSegment3d offsetSegment = m_segment;
        offsetSegment.point[0] = DPoint3d::FromSumOf (m_segment.point[0], perp, -d);
        offsetSegment.point[1] = DPoint3d::FromSumOf (m_segment.point[1], perp, -d);
        return Create (offsetSegment);
        }
    return NULL;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitive::CurvePrimitiveType CurvePrimitiveLine::_GetCurvePrimitiveType() const  {return CURVE_PRIMITIVE_TYPE_Line;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DSegment3dCP CurvePrimitiveLine::_GetLineCP() const  {return &m_segment;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurvePrimitiveLine::_Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const
    {
    processor._ProcessLine (*this, m_segment, interval);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_IsExtensibleFractionSpace() const {return true;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_IsMappableFractionSpace() const {return true;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_IsFractionSpace() const {return true;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_FractionToPoint(double fraction, DPoint3dR point) const 
        {
        m_segment.FractionParameterToPoint (point, fraction);
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_TrySetStart (DPoint3dCR xyz) 
        {
        m_segment.point[0] = xyz;
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_TrySetEnd (DPoint3dCR xyz) 
        {
        m_segment.point[1] = xyz;
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent) const 
        {
        m_segment.FractionParameterToTangent (point, tangent, fraction);
        return true;
        }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const 
        {
        m_segment.FractionParameterToTangent (point, tangent, fraction);
        derivative2.Zero ();
        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const 
        {
        m_segment.FractionParameterToTangent (point, tangent, fraction);
        derivative2.Zero ();
        derivative3.Zero ();
        return true;
        }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_FractionToFrenetFrame(double f, TransformR frame) const 
        {
        DPoint3d point;
        DVec3d tangent, xDir, yDir, zDir;

        if (!_FractionToPoint (f, point, tangent))
            return false;

        if (!tangent.GetNormalizedTriad (yDir, zDir, xDir))
            frame.InitFrom (point);
        else
            frame.InitFromOriginAndVectors (point, xDir, yDir, zDir);

        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_Length(double &length) const 
        {
        length = m_segment.Length();
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/15
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_Length(RotMatrixCP worldToLocal, double &length) const 
        {
        DVec3d vector = DVec3d::FromStartEnd (m_segment.point[0], m_segment.point[1]);
        worldToLocal->Multiply (vector);
        length = vector.Magnitude ();
        return true;
        }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_GetRange(DRange3dR range) const 
        {
        range.Init ();
        range.Extend (m_segment.point, 2);
        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_GetRange(DRange3dR range, TransformCR transform) const 
        {
        range.Init ();
        range.Extend (transform, m_segment.point, 2);
        return true;
        }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double CurvePrimitiveLine::_FastMaxAbs() const 
        {
        return DoubleOps::Max (m_segment.point[0].MaxAbs (), m_segment.point[1].MaxAbs ());
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const
    {
    DSegment3d otherSegment;
    if (!other.TryGetLine (otherSegment))
        return false;
    return m_segment.IsAlmostEqual (otherSegment, tolerance);
    }
        
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2013
+--------------------------------------------------------------------------------------*/
size_t CurvePrimitiveLine::_NumComponent () const  {return 1;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DRange1d CurvePrimitiveLine::_ProjectedParameterRange(DRay3dCR ray) const 
    {
    return m_segment.ProjectedParameterRange (ray);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DRange1d CurvePrimitiveLine::_ProjectedParameterRange(DRay3dCR ray, double fractionA, double fractionB) const 
    {
    return DSegment3d::FromFractionInterval (m_segment, fractionA, fractionB).ProjectedParameterRange (ray);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_GetBreakFraction(size_t breakFractionIndex, double &fraction) const 
        {
        if (breakFractionIndex == 0)
            {
            fraction = 0.0;
            return true;
            }
        else if (breakFractionIndex == 1)
            {
            fraction = 1.0;
            return true;
            }
        else
            return false;
        }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_AdjustFractionToBreakFraction(double fraction, Rounding::RoundingMode mode, size_t &breakIndex, double &adjustedFraction) const 
        {
        adjustedFraction = Rounding::Round (fraction, mode, 0.0, 1.0);
        if (DoubleOps::AlmostEqual (adjustedFraction, 0.0))
            breakIndex = 0;
        else
            breakIndex = 1;

        return true;
        }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_GetMSBsplineCurve(MSBsplineCurveR curve, double fractionA, double fractionB) const 
        {
        if (DoubleOps::IsExact01 (fractionA, fractionB))
            {
            return SUCCESS == curve.CreateFromPointsAndOrder (m_segment.point, 2, 2);
            }
        else
            {
            DPoint3d newPoints[2];
            m_segment.FractionParameterToPoint (newPoints[0], fractionA);
            m_segment.FractionParameterToPoint (newPoints[1], fractionB);
            return SUCCESS == curve.CreateFromPointsAndOrder (newPoints, 2, 2);
            }
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_AddStrokes(bvector <DPoint3d> &points, IFacetOptionsCR options,
                bool includeStartPoint,
                double startFraction,
                double endFraction
                ) const 
        {
        DSegment3d localSegment = DSegment3d::FromFractionInterval (m_segment, startFraction, endFraction);

        size_t edgeCount = options.SegmentStrokeCount (localSegment);
        PolylineOps::AddContinuationStartPoint (points, localSegment.point[0], includeStartPoint);

        if (edgeCount > 2)
            {
            double df = 1.0 / (double) (edgeCount - 1);
            DVec3d extent = DVec3d::FromStartEnd (localSegment.point[0], localSegment.point[1]);
            for (size_t i = 1; i < edgeCount - 1; i++)
                {
                DPoint3d xyz;
                xyz.SumOf (localSegment.point[0], extent, i * df);
                points.push_back (xyz);
                }
            }
        points.push_back (localSegment.point[1]);
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_AddStrokes(bvector <PathLocationDetail> &points, IFacetOptionsCR options,
                double startFraction,
                double endFraction
                ) const 
        {
        DSegment3d localSegment = DSegment3d::FromFractionInterval (m_segment, startFraction, endFraction);
        
        size_t edgeCount = options.SegmentStrokeCount (localSegment);
        if (edgeCount < 1)
            edgeCount = 1;
        double df = 1.0 / (double) (edgeCount);
        double distance0 = points.size () == 0 ? 0.0 : points.back ().DistanceFromPathStart ();
        DVec3d extent = DVec3d::FromStartEnd (localSegment.point[0], localSegment.point[1]);
        double activeLength = extent.Magnitude ();
        CurveLocationDetail cd;
        FractionToPoint (startFraction, cd);
        points.push_back (PathLocationDetail (cd, 0, distance0));
        for (size_t i = 1; i < edgeCount; i++)
            {
            double g = i *df;   // fraction within localSegment
            double f = DoubleOps::Interpolate (startFraction, g, endFraction);  // fraction within total segment.
            FractionToPoint (f, cd);
            points.push_back (PathLocationDetail (cd, 0, distance0 + g * activeLength));
            }
        FractionToPoint (endFraction, cd);
        points.push_back (PathLocationDetail (cd, 0, distance0 + activeLength));

        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t CurvePrimitiveLine::_GetStrokeCount(IFacetOptionsCR options, double startFraction, double endFraction) const 
    {
    DSegment3d localSegment = DSegment3d::FromFractionInterval (m_segment, startFraction, endFraction);
    return options.SegmentStrokeCount (localSegment);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_SignedDistanceBetweenFractions(double startFraction, double endFraction, double &length) const 
        {
        length = (endFraction - startFraction) * m_segment.Length();
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_SignedDistanceBetweenFractions(RotMatrixCP worldToLocal, double startFraction, double endFraction, double &length) const 
        {
        Length (worldToLocal, length);
        length *= (endFraction - startFraction);
        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_PointAtSignedDistanceFromFraction(double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const 
        {
        location = CurveLocationDetail (this, 1);
        double a = m_segment.Length ();
        double deltaFraction;
        if (DoubleOps::SafeDivideParameter (deltaFraction, signedDistance, a, 0.0))
            {
            double endFraction = startFraction + deltaFraction;
            double actualDistance = signedDistance;
            location.SetSingleComponentFractionAndA (endFraction, actualDistance);
            }
        m_segment.FractionParameterToPoint (location.point, location.fraction);
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_PointAtSignedDistanceFromFraction(RotMatrixCP worldToLocal, double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const 
        {
        location = CurveLocationDetail (this, 1);
        double a;
        _Length (worldToLocal, a);
        double deltaFraction;
        bool stat = DoubleOps::SafeDivideParameter (deltaFraction, signedDistance, a, 0.0);
        if (stat)
            {
            double endFraction = startFraction + deltaFraction;
            double actualDistance = signedDistance;
            location.SetSingleComponentFractionAndA (endFraction, actualDistance);
            }
        m_segment.FractionParameterToPoint (location.point, location.fraction);
        return stat;
        }


//    using ICurvePrimitive::_ClosestPointBounded;    // suppresses C4266

bool CurvePrimitiveLine::_ClosestPointBounded (DPoint3dCR spacePoint, double &fraction, DPoint3dR curvePoint, bool extend0, bool extend1) const 
    {
    return m_segment.ProjectPointBounded (curvePoint, fraction, spacePoint, extend0, extend1);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_ClosestPointBoundedXY(DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location, bool extend0, bool extend1) const 
        {
        location = CurveLocationDetail (this, 1);
        m_segment.ClosestPointBoundedXY (location.point, location.fraction, location.a, spacePoint, worldToLocal, extend0, extend1);
        location.SetSingleComponentData ();
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/15
+--------------------------------------------------------------------------------------*/
void CurvePrimitiveLine::_AnnounceKeyPoints
(
DPoint3dCR spacePoint,                       //!< [in] point to project
CurveKeyPointCollector &collector,           //!< [in] object to receive keypoint announcements
bool extend0,                               //!< true to extend at start of primitives.
bool extend1                                //!< true to extend at end of primitives.
) const
    {
    DMatrix4d worldToLocal;
    bool xy = collector.GetWorldToLocal (worldToLocal);
    CurveLocationDetail detail;
    if (collector.NeedKeyPointType (CurveKeyPointCollector::KeyPointType::EndPoint))
        {
        FractionToPoint (0.0, detail);
        collector.AnnouncePoint (detail, CurveKeyPointCollector::KeyPointType::EndPoint);
        FractionToPoint (1.0, detail);
        collector.AnnouncePoint (detail, CurveKeyPointCollector::KeyPointType::EndPoint);
        }
    if (collector.NeedKeyPointType (CurveKeyPointCollector::KeyPointType::Perpendicular))
        {
        if (xy)
            {
            }
        else
            {
            DPoint3d xyz;
            double f;
            if (m_segment.ProjectPoint (xyz, f, spacePoint))
                {
                if (DoubleOps::IsIn01OrExtension (f, extend0, extend1))
                    {
                    FractionToPoint (f, detail);
                    collector.AnnouncePoint (detail, CurveKeyPointCollector::KeyPointType::Perpendicular);
                    }
                }
            }
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_TransformInPlace (TransformCR transform) 
        {
        transform.Multiply (m_segment.point[0]);
        transform.Multiply (m_segment.point[1]);
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_ReverseCurvesInPlace () 
        {
        std::swap (m_segment.point[0], m_segment.point[1]);
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurvePrimitiveLine::_AppendCurvePlaneIntersections(DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tol) const
        {AppendTolerancedPlaneIntersections (plane, this, m_segment, intersections, tol);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
void CurvePrimitiveLine::_AppendCurvePlaneIntersections
(
DPoint3dDVec3dDVec3dCR plane,  //!< [in] plane to intersect
UVBoundarySelect   bounded,         //!< [in] selects Unbounded, Triangle, or Parallelogram boundaries.
bvector<CurveAndSolidLocationDetail> &intersections   //!< [out] intersections 
) const
    {
    DPoint2d uv;
    double f;
    if (plane.TransverseIntersection (m_segment, uv, f))
        {
        DPoint3dDVec3dDVec3dCR planePoint = plane.EvaluateTangents (uv.x, uv.y);
        if (bounded.IsInOrOn (uv))
            {
            CurveLocationDetail curveDetail;
            FractionToPoint (f, curveDetail);
            intersections.push_back (CurveAndSolidLocationDetail (curveDetail,
                    SolidLocationDetail (0, f,
                    planePoint.origin,
                    uv.x, uv.y,
                    planePoint.vectorU, planePoint.vectorV)
                    ));
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
void CurvePrimitiveLine::_AppendCurveRangeIntersections (LocalRangeCR range, bvector<PartialCurveDetail> &intersections) const
        {
        DSegment3d localSegment, localClip;
        range.m_worldToLocal.Multiply (localSegment, m_segment);
        double f0, f1;
        if (range.m_localRange.IntersectBounded (f0, f1, localClip, localSegment))
            intersections.push_back (PartialCurveDetail (
                    (ICurvePrimitiveP)this,
                    f0, f1, 0));
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLine::_WireCentroid(double &length, DPoint3dR centroid, double fraction0, double fraction1) const
        {
        m_segment.WireCentroid (length, centroid, fraction0, fraction1);
        return true;
        }



ICurvePrimitive* CurvePrimitiveLine::Create (DSegment3dCR segment) {return new CurvePrimitiveLine (segment);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateLine (DSegment3dCR segment)
    {return CurvePrimitiveLine::Create (segment);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateLine (DPoint3dCR point0, DPoint3dCR point1)
    {return CurvePrimitiveLine::Create (DSegment3d::From (point0, point1));}

END_BENTLEY_GEOMETRY_NAMESPACE

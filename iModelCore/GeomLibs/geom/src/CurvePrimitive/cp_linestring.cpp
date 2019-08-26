/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#include "cpstructs.h"

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurvePrimitiveLineString::CurvePrimitiveLineString (DPoint3dCP points, size_t nPoints)
        {
        if (nPoints > 0 && NULL != points)
            {
            m_points.resize (nPoints);
            memcpy (&m_points[0], points, nPoints * sizeof (*points));
            }
        else
            {
            // hmm.. just let it be empty.
            }
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurvePrimitiveLineString::CurvePrimitiveLineString (bvector<DPoint2d> const &points, double z)
    {
    if (points.size () > 0)
        {
        for (auto &xy : points)
            m_points.push_back (DPoint3d::From (xy.x, xy.y, z));
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurvePrimitiveLineString::CurvePrimitiveLineString (bvector<DPoint3d> const &points) {m_points = points;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurvePrimitiveLineString::CurvePrimitiveLineString(){}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr CurvePrimitiveLineString::_Clone() const {return new CurvePrimitiveLineString (m_points);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr CurvePrimitiveLineString::_CloneComponent (ptrdiff_t componentIndex) const
    {
    if (componentIndex < 0 || componentIndex + 1 >= (ptrdiff_t)m_points.size ())
        return _Clone ();
    else
        {
        DSegment3d segment = DSegment3d::From (m_points[componentIndex], m_points[componentIndex + 1]);
        return ICurvePrimitive::CreateLine (segment);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr CurvePrimitiveLineString::_CloneBetweenFractions
(
double fractionA,
double fractionB,
bool allowExtrapolation
) const
    {
    CurvePrimitiveLineString *linestring = new CurvePrimitiveLineString ();
    PolylineOps::CopyBetweenFractions (m_points, linestring->m_points, fractionA, fractionB);
    return linestring;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitive::CurvePrimitiveType CurvePrimitiveLineString::_GetCurvePrimitiveType() const {return CURVE_PRIMITIVE_TYPE_LineString;}
bvector<DPoint3d>*    CurvePrimitiveLineString::_GetLineStringP () {return &m_points;}
bvector<DPoint3d> const*    CurvePrimitiveLineString::_GetLineStringCP () const {return &m_points;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurvePrimitiveLineString::_Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const
    {processor._ProcessLineString (*this, m_points, interval);}



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_IsExtensibleFractionSpace() const {return true;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_IsMappableFractionSpace() const {return false;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_IsFractionSpace() const {return true;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_FractionToPoint(double fraction, DPoint3dR point) const
    {
    if (m_points.size () == 0)
        return false;        
    DVec3d tangentA, tangentB;
    if (bsiDPoint3d_interpolatePolyline (point, tangentA, tangentB, &m_points[0], (int)m_points.size (), fraction))
        return true;
    return false;
    }

bool CurvePrimitiveLineString::_FractionToPointWithTwoSidedDerivative (double fraction, DPoint3dR point, DVec3dR tangentA, DVec3dR tangentB) const
    {
    if (m_points.size () == 0)
        return false;        
    if (bsiDPoint3d_interpolatePolyline (point, tangentA, tangentB, &m_points[0], (int)m_points.size (), fraction))
        return true;
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/15
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_FractionToPoint (double f, CurveLocationDetail &detail) const
    {
    DPoint3d xyz;
    if (FractionToPoint (f, xyz))
        {
        detail = CurveLocationDetail (this, f, xyz);
        detail.SetComponentFractionFromFraction (f, m_points.size () > 0 ? m_points.size () - 1 : 1);
        return true;
        }
    detail = CurveLocationDetail (this);
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent) const
    {
    if (m_points.size () == 0)
        return false;        
    if (bsiDPoint3d_interpolatePolyline (&point, &tangent, &m_points[0], (int)m_points.size (), fraction))
        return true;
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_ComponentFractionToPoint(ptrdiff_t componentIndex, double fraction, DPoint3dR point) const
    {
    if (componentIndex < 0)
        return FractionToPoint (fraction, point);
    if (componentIndex + 1 > (ptrdiff_t)m_points.size ())
        return false;
    point.Interpolate (m_points[componentIndex], fraction, m_points[componentIndex + 1]);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_ComponentFractionToPoint(ptrdiff_t componentIndex, double fraction, DPoint3dR point, DVec3dR tangent) const
    {
    if (componentIndex < 0)
        return FractionToPoint (fraction, point, tangent);
    if (componentIndex + 1 > (ptrdiff_t)m_points.size ())
        return false;
    point.Interpolate (m_points[componentIndex], fraction, m_points[componentIndex + 1]);
    tangent.DifferenceOf (m_points[componentIndex + 1], m_points[componentIndex]);
    return true;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const
    {
    if (m_points.size () == 0)
        return false;        
    if (bsiDPoint3d_interpolatePolyline (&point, &tangent, &m_points[0], (int)m_points.size (), fraction))
        {
        derivative2.Zero ();
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const
    {
    if (_FractionToPoint(fraction, point, tangent))
        {
        derivative2.Zero ();
        derivative3.Zero ();
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_FractionToFrenetFrame(double f, TransformR frame) const
        {
        if (m_points.size () == 0)
            return false;
        
        return PolylineOps::FractionToFrenetFrame (m_points, f, frame);
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_Length(double &length) const
    {
    length = PolylineOps::Length (m_points);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_Length(RotMatrixCP worldToLocal, double &length) const
    {
    length = PolylineOps::Length (worldToLocal, m_points);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_GetRange(DRange3dR range) const
    {
    range.Init ();
    range.Extend (&m_points[0], (int)m_points.size ());
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_GetRange(DRange3dR range, TransformCR transform) const
    {
    range.Init ();
    range.Extend (transform, m_points);
    return true;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double CurvePrimitiveLineString::_FastMaxAbs() const
    {
    return DPoint3dOps::LargestCoordinate (m_points);
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const
    {
    bvector<DPoint3d> const *otherPoints = other.GetLineStringCP ();
    return nullptr != otherPoints
          && DPoint3d::AlmostEqual (m_points, *otherPoints, tolerance);
    }
            
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2013
+--------------------------------------------------------------------------------------*/
size_t CurvePrimitiveLineString::_NumComponent () const {return m_points.size () < 1 ? 0 : m_points.size () - 1;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DRange1d CurvePrimitiveLineString::_ProjectedParameterRange(DRay3dCR ray) const
    {
    return DPoint3dOps::ProjectedParameterRange (m_points, ray);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DRange1d CurvePrimitiveLineString::_ProjectedParameterRange(DRay3dCR ray, double fractionA, double fractionB) const
    {
    return PolylineOps::ProjectedParameterRange (m_points, ray, fractionA, fractionB);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_GetBreakFraction(size_t breakFractionIndex, double &fraction) const
        {
        if (breakFractionIndex < m_points.size ())
            {
            fraction = double (breakFractionIndex)/(m_points.size () - 1);
            return true;
            }
        else
            return false;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_AdjustFractionToBreakFraction(double fraction, Rounding::RoundingMode mode, size_t &breakIndex, double &adjustedFraction) const
        {
        double lowerBound, upperBound;

        if (DoubleOps::BoundingValues (m_points.size (), fraction, breakIndex, lowerBound, upperBound))
            {
            adjustedFraction = Rounding::Round (fraction, mode, lowerBound, upperBound);
            if (adjustedFraction > lowerBound)
                breakIndex ++;

            return true;
            }
        else
            return false;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_GetMSBsplineCurve(MSBsplineCurveR curve, double fractionA, double fractionB) const
        {
        if (DoubleOps::IsExact01 (fractionA, fractionB))
            {
            return SUCCESS == curve.CreateFromPointsAndOrder (&m_points[0], (int)m_points.size (), 2);
            }
        else
            {
            bvector<DPoint3d> newPoints;
            PolylineOps::CopyBetweenFractions (m_points, newPoints, fractionA, fractionB);
            return SUCCESS == curve.CreateFromPointsAndOrder (&newPoints[0], (int)newPoints.size (), 2);
            }
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_AddStrokes(bvector <DPoint3d> &points, IFacetOptionsCR options,
                bool includeStartPoint,
                double startFraction,
                double endFraction
                ) const
        {
        return PolylineOps::AddStrokes (m_points, points, options, includeStartPoint, startFraction, endFraction);
        }

bool CurvePrimitiveLineString::_AddStrokes (DPoint3dDoubleUVCurveArrays &points, IFacetOptionsCR options,
                double startFraction,
                double endFraction
                )  const
        {
        return PolylineOps::AddStrokes (m_points, points, options, startFraction, endFraction);
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_AddStrokes(bvector <PathLocationDetail> &points, IFacetOptionsCR options,
                double startFraction,
                double endFraction
                ) const
        {
        bvector<CurveLocationDetail> vertexStrokes;
        PolylineOps::CopyBetweenFractions (m_points, vertexStrokes, startFraction, endFraction);
        for (CurveLocationDetail &cd : vertexStrokes)
            cd.curve = this;
        double distance0 = points.size () == 0 ? 0.0 : points.back ().DistanceFromPathStart ();
        for (size_t vs = 0; vs + 1 < vertexStrokes.size (); vs++)
            {
            DSegment3d segment = DSegment3d::From (vertexStrokes[vs].point, vertexStrokes[vs+1].point);
            DVec3d extent = DVec3d::FromStartEnd (segment.point[0], segment.point[1]);
            size_t edgeCount = options.SegmentStrokeCount (segment);
            double dg = 1.0 / (double) (edgeCount);
            double activeLength = extent.Magnitude ();

            if (vs == 0)
                points.push_back (PathLocationDetail (vertexStrokes[vs], 0, distance0));
            for (size_t i = 1; i < edgeCount; i++)
                {
                double g = i *dg;   // fraction within segment
                points.push_back (PathLocationDetail (
                            vertexStrokes[vs].Interpolate (g, vertexStrokes[vs+1]),
                            0, distance0 + g * activeLength));
                }
            distance0 += activeLength;
            points.push_back (PathLocationDetail (vertexStrokes[vs+1], 0, distance0));

            }
        return true;
        }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t CurvePrimitiveLineString::_GetStrokeCount(IFacetOptionsCR options, double startFraction, double endFraction) const
    {
    return PolylineOps::GetStrokeCount (m_points, options, startFraction, endFraction);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_SignedDistanceBetweenFractions(double startFraction, double endFraction, double &signedDistance) const
    {
    signedDistance = PolylineOps::SignedDistanceBetweenFractions (m_points, startFraction, endFraction);
    return true;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_SignedDistanceBetweenFractions(RotMatrixCP worldToLocal, double startFraction, double endFraction, double &signedDistance) const
    {
    signedDistance = PolylineOps::SignedDistanceBetweenFractions (worldToLocal, m_points, startFraction, endFraction);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_PointAtSignedDistanceFromFraction(double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const
    {
    double endFraction;
    double actualDistance;
    size_t segmentIndex;
    double segmentFraction;
    if (PolylineOps::FractionAtSignedDistanceFromFraction (m_points, startFraction, signedDistance, endFraction,
                segmentIndex, segmentFraction,
                actualDistance))
        {
        DPoint3d point = PolylineOps::SegmentFractionToPoint (m_points, segmentIndex, segmentFraction); 
        location = CurveLocationDetail (this, endFraction,
                        point,
                        segmentIndex, m_points.size () - 1, segmentFraction);
        location.a = actualDistance;
        return true;
        }
    location = CurveLocationDetail (this, 1);
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_PointAtSignedDistanceFromFraction(RotMatrixCP worldToLocal, double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const
    {
    double endFraction;
    double actualDistance;
    size_t segmentIndex;
    double segmentFraction;
    if (PolylineOps::FractionAtSignedDistanceFromFraction (worldToLocal, m_points, startFraction, signedDistance, endFraction,
                segmentIndex, segmentFraction,
                actualDistance))
        {
        DPoint3d point = PolylineOps::SegmentFractionToPoint (m_points, segmentIndex, segmentFraction); 
        location = CurveLocationDetail (this, endFraction,
                        point,
                        segmentIndex, m_points.size () - 1, segmentFraction);
        location.a = actualDistance;
        return true;
        }
    location = CurveLocationDetail (this, 1);
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_ClosestPointBounded(DPoint3dCR spacePoint, double &fraction, DPoint3dR curvePoint, bool extend0, bool extend1) const
    {
    size_t componentIndex, numComponent;
    double componentFraction;
    return PolylineOps::ClosestPoint (m_points, false, spacePoint, fraction, curvePoint,
                componentIndex, numComponent, componentFraction,
                extend0, extend1);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_ClosestPointBounded(DPoint3dCR spacePoint, CurveLocationDetailR detail, bool extend0, bool extend1) const
    {
    detail          = CurveLocationDetail (this, 1);
    PolylineOps::ClosestPoint (m_points, false, spacePoint,
                detail.fraction, detail.point,
                detail.componentIndex, detail.numComponent, detail.componentFraction,
                extend0, extend1);
    detail.SetDistanceFrom (spacePoint);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_ClosestPointBoundedXY
(
DPoint3dCR spacePoint,
DMatrix4dCP worldToLocal,
CurveLocationDetailR detail,
bool extend0,
bool extend1
) const
    {
    detail          = CurveLocationDetail (this, 1);
    PolylineOps::ClosestPointXY (m_points, false, spacePoint, worldToLocal,
                detail.fraction, detail.point, detail.componentIndex, detail.numComponent, detail.componentFraction,
                detail.a, extend0, extend1);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/15
+--------------------------------------------------------------------------------------*/
void CurvePrimitiveLineString::_AnnounceKeyPoints
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
    size_t numPoints = m_points.size ();
    ptrdiff_t numEdges = numPoints - 1;
    if (collector.NeedKeyPointType (CurveKeyPointCollector::KeyPointType::BreakPoint)
        && numPoints > 2)
        {
        // announce all strict INTERIOR breaks -- ends are done by pre-virtual.
        double df = 1.0 / (double)(numEdges);
        for (size_t i = 1; i + 1 < numPoints; i++)
            {
            collector.AnnouncePoint (
                CurveLocationDetail (this, i * df, m_points[i], i, numEdges, 0.0),
                CurveKeyPointCollector::KeyPointType::BreakPoint
                );
            }
        }

    if (collector.NeedKeyPointType (CurveKeyPointCollector::KeyPointType::Perpendicular)
        && numPoints > 1)
        {
        double df = 1.0 / (double)(numEdges);
        if (xy)
            {
            }
        else
            {
            DPoint3d xyz;
            double segmentFraction;
            DSegment3d segment;
            for (size_t i = 0; i < (size_t)numEdges; i++)
                {
                segment.point[0] = m_points[i];
                segment.point[1] = m_points[i+1];
                if (segment.ProjectPoint (xyz, segmentFraction, spacePoint)
                    && DoubleOps::IsIn01OrExtension (segmentFraction, extend0 && (i == 0), extend1 && (i + 1 == numEdges))
                    )
                    {
                    collector.AnnouncePoint (
                        CurveLocationDetail (this, (i + segmentFraction )* df, xyz, i, numEdges, segmentFraction),
                        CurveKeyPointCollector::KeyPointType::Perpendicular
                        );
                    }
                }
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_TransformInPlace (TransformCR transform)
    {
    DPoint3dOps::Multiply (&m_points, transform);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_ReverseCurvesInPlace ()
    {
    DPoint3dOps::Reverse (m_points);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_WireCentroid(double &length, DPoint3dR centroid, double fraction0, double fraction1) const
    {
    PolylineOps::WireCentroid (m_points, length, centroid, fraction0, fraction1);
    return true;
    }


ICurvePrimitive* CurvePrimitiveLineString::Create (bvector<DPoint2d> const &points, double z) {return new CurvePrimitiveLineString (points, z);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitive* CurvePrimitiveLineString::Create (DPoint3dCP points, size_t nPoints) {return new CurvePrimitiveLineString (points, nPoints);}

struct RangePlaneBits
{
// Bits indicating OUT for various directions ..
uint32_t m_bits;
static const uint32_t xNegativeBit = 0x01;
static const uint32_t xPositiveBit = 0x02;

static const uint32_t yNegativeBit = 0x04;
static const uint32_t yPositiveBit = 0x08;

static const uint32_t zNegativeBit = 0x10;
static const uint32_t zPositiveBit = 0x20;
RangePlaneBits () : m_bits(0){}
RangePlaneBits (DRange3dCR range, DPoint3d point)
    {
    m_bits = 0;
    if (point.x < range.low.x)
        m_bits |= xNegativeBit;
    if (point.x > range.high.x)
        m_bits |= xPositiveBit;

    if (point.y < range.low.y)
        m_bits |= yNegativeBit;
    if (point.y > range.high.y)
        m_bits |= yPositiveBit;

    if (point.z < range.low.z)
        m_bits |= zNegativeBit;
    if (point.z > range.high.z)
        m_bits |= zPositiveBit;
    }

// return true if all classifications are IN:
bool AllIn () const {return m_bits == 0;}
// return true if both classifications for any single plane are OUT
bool SimpleOutSegment (RangePlaneBits const &other) const
    {
    return 0 != (m_bits & other.m_bits);
    }
// return true if both classifications for any single plane are OUT
bool SimpleInSegment (RangePlaneBits const &other) const
    {
    return m_bits == 0 && other.m_bits == 0;
    }

};
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurvePrimitiveLineString::_AppendCurvePlaneIntersections(DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tol) const
    {AppendTolerancedPlaneIntersections (plane, this, m_points, intersections, tol);}

void CurvePrimitiveLineString::_AppendCurveRangeIntersections (LocalRangeCR range, bvector<PartialCurveDetail> &intersections) const
        {
        if (m_points.empty ())
            return;
        DPoint3d localA, localB;
        range.m_worldToLocal.Multiply (localA, m_points[0]);
        RangePlaneBits bitA (range.m_localRange, localA);
        if (m_points.size () == 1)
            {
            intersections.push_back (PartialCurveDetail (
                    (ICurvePrimitiveP)this,
                    0.0, 1.0, 0));
            return;
            }
        double fA = 0.0;
        double df = 1.0 / (double)(m_points.size () - 1);
        double fB, g0, g1;
        RangePlaneBits bitB;
        bool canExtendPriorPartialCurve = false;
        for (size_t i = 1; i < m_points.size (); i++, fA = fB, localA = localB, bitA = bitB)
            {
            range.m_worldToLocal.Multiply (localB, m_points[i]);
            bitB = RangePlaneBits (range.m_localRange, m_points[i]);
            fB = i * df;
            if (bitA.SimpleInSegment (bitB))
                {
                if (canExtendPriorPartialCurve)
                    intersections.back ().UpdateFraction1AndUserData (fB, i);
                else
                    intersections.push_back (PartialCurveDetail (
                            (ICurvePrimitiveP)this,
                            fA, fB, i));
                canExtendPriorPartialCurve = true;
                }
            else if (bitA.SimpleOutSegment (bitB))
                {
                canExtendPriorPartialCurve = false;
                }
            else
                {
                DSegment3d localSegment = DSegment3d::From (localA, localB);
                DSegment3d localClip;
                if (range.m_localRange.IntersectBounded (g0, g1, localClip, localSegment))
                    {
                    double f0 = DoubleOps::Interpolate (fA, g0, fB);
                    double f1 = DoubleOps::Interpolate (fA, g1, fB);
                    if (canExtendPriorPartialCurve && bitA.AllIn ())
                        {
                        intersections.back ().UpdateFraction1AndUserData (f1, i);
                        }
                    else
                        {
                        intersections.push_back (PartialCurveDetail (
                                (ICurvePrimitiveP)this,
                                f0, f1, i));
                        }
                    canExtendPriorPartialCurve = bitB.AllIn ();
                    }
                else
                    canExtendPriorPartialCurve = false;
                }
            }
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_TrySetStart (DPoint3dCR xyz)
    {
    if (m_points.size () > 0)
        m_points.front () = xyz;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveLineString::_TrySetEnd (DPoint3dCR xyz)
    {
    if (m_points.size () > 0)
        m_points.back () = xyz;
    return true;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateLineString (DPoint3dCP points, size_t nPoints)
    {return CurvePrimitiveLineString::Create (points, nPoints);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateLineString (bvector<DPoint3d> const &points)
    {return CurvePrimitiveLineString::Create (&points[0], points.size ());}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/2018
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateLineString (bvector<DPoint2d> const &points, double z)
    {return CurvePrimitiveLineString::Create (points, z);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurvePrimitivePointString::CurvePrimitivePointString(DPoint3dCP points, size_t nPoints) 
    : CurvePrimitiveLineString (points, nPoints) {}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool CurvePrimitivePointString::_IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const
    {
    bvector<DPoint3d> const *otherPoints = other.GetPointStringCP ();
    size_t n = m_points.size ();
    if (NULL == otherPoints || otherPoints->size () != n)
        return false;
    for (size_t i = 0; i < n; i ++)
        {
        if (!DPoint3dOps::AlmostEqual (m_points[i], otherPoints->at(i)))
            return false;
        }
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurvePrimitivePointString::CurvePrimitivePointString(bvector<DPoint3d> points) 
    : CurvePrimitiveLineString (points) {}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitive::CurvePrimitiveType CurvePrimitivePointString::_GetCurvePrimitiveType() const {return CURVE_PRIMITIVE_TYPE_PointString;}
bvector<DPoint3d> const*    CurvePrimitivePointString::_GetPointStringCP () const {return CurvePrimitiveLineString::_GetLineStringCP ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitivePointString::_GetMSBsplineCurve(MSBsplineCurveR curve, double fractionA, double fractionB) const
        {
        return false;
        }


bvector<DPoint3d> const* CurvePrimitivePointString::_GetLineStringCP () const {return NULL;}
bvector<DPoint3d> * CurvePrimitivePointString::_GetLineStringP () {return NULL;}

ICurvePrimitive* CurvePrimitivePointString::Create (DPoint3dCP points, size_t nPoints) {return new CurvePrimitivePointString (points, nPoints);}
ICurvePrimitive* CurvePrimitivePointString::Create (bvector<DPoint3d> &points) {return new CurvePrimitivePointString (points);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr CurvePrimitivePointString::_Clone() const     {return CreatePointString (&m_points[0], m_points.size ());}

//using ICurvePrimitive::_ClosestPointBounded;    // suppresses C4266

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2013
+--------------------------------------------------------------------------------------*/
bool CurvePrimitivePointString::_ClosestPointBounded(DPoint3dCR spacePoint, CurveLocationDetailR detail, bool extend0, bool extend1) const
    {
    detail          = CurveLocationDetail (this, 1);
    size_t index;
    double minDist;
    if (DPoint3dOps::ClosestPoint (m_points, spacePoint, index, minDist))
        {
        detail.componentFraction = 0.0;
        detail.point = m_points [index];
        detail.componentIndex = index;
        detail.numComponent = m_points.size ();
        detail.SetDistanceFrom (spacePoint);
        return true;
        }
    return false;   // no points?
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitivePointString::_ClosestPointBoundedXY(DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR detail, bool extend0, bool extend1) const
    {
    detail          = CurveLocationDetail (this, 1);
    size_t index;
    double minDist;
    if (DPoint3dOps::ClosestPointXY (m_points, worldToLocal, spacePoint, index, minDist))
        {
        detail.componentFraction = 0.0;
        detail.point = m_points [index];
        detail.componentIndex = index;
        detail.numComponent = m_points.size ();
        detail.a = minDist;
        return true;
        }
    return false;   // no points?
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreatePointString (DPoint3dCP points, size_t nPoints)
    {return CurvePrimitivePointString::Create (points, nPoints);}
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2013
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreatePointString (bvector<DPoint3d> &points)
    {return CurvePrimitivePointString::Create (points);}




END_BENTLEY_GEOMETRY_NAMESPACE

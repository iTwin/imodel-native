/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/cp_arc.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#include "cpstructs.h"

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurvePrimitiveArc::CurvePrimitiveArc(DEllipse3dCR ellipse) {m_ellipse = ellipse;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr CurvePrimitiveArc::_Clone() const {return new CurvePrimitiveArc (m_ellipse);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_IsExtensibleFractionSpace() const {return true;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_IsMappableFractionSpace() const {return true;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_IsFractionSpace() const {return true;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_IsPeriodicFractionSpace(double &period) const
    {
    // hmmm.. What to return on zero sweep?
    return DoubleOps::SafeDivideParameter (period, msGeomConst_2pi, m_ellipse.sweep, 0.0);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr CurvePrimitiveArc::_CloneBetweenFractions
(
double fractionA,
double fractionB,
bool allowExtrapolation
) const
    {
    DEllipse3d newArc = m_ellipse;
    newArc.start = m_ellipse.FractionToAngle (fractionA);
    newArc.sweep = (fractionB - fractionA) * m_ellipse.sweep;
    return Create (newArc);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr CurvePrimitiveArc::_CloneAsSingleOffsetPrimitiveXY (CurveOffsetOptionsCR options) const
    {
    double radius;
    double dr = options.GetOffsetDistance ();
    DVec3d normal = DVec3d::FromCrossProduct (m_ellipse.vector0, m_ellipse.vector90);
    if (m_ellipse.sweep * normal.z < 0.0)
        dr = -dr;
    if (m_ellipse.IsCircularXY (radius)) // if you want to prevent zero and negative radius also impose:  && fabs (dr) < radius)
        {
        double r1 = radius + dr;
        DEllipse3d ellipse1 = m_ellipse;
        ellipse1.vector0.z = 0.0;
        ellipse1.vector90.z = 0.0;
        ellipse1.vector0.ScaleToLength (r1);
        ellipse1.vector90.ScaleToLength (r1);
        ellipse1.vector0.z = m_ellipse.vector0.z;
        ellipse1.vector90.z = m_ellipse.vector90.z;
        return Create (ellipse1);
        }
    ICurvePrimitivePtr bcurves = bspconv_convertDEllipse3dOffsetToCurveChain (&m_ellipse, dr);
    return bcurves;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitive::CurvePrimitiveType CurvePrimitiveArc::_GetCurvePrimitiveType() const {return CURVE_PRIMITIVE_TYPE_Arc;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3dCP CurvePrimitiveArc::_GetArcCP() const {return &m_ellipse;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurvePrimitiveArc::_Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const
    {processor._ProcessArc (*this, m_ellipse, interval);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_FractionToPoint(double fraction, DPoint3dR point) const
    {
    m_ellipse.FractionParameterToPoint (point, fraction);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_FractionToPoint(double fraction, DPoint3dR point, DVec3dR vector) const
    {
    DVec3d vector2;
    m_ellipse.FractionParameterToDerivatives (point, vector, vector2, fraction);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_FractionToPoint(double fraction, DPoint3dR point, DVec3dR vector, DVec3dR derivative2) const
    {
    m_ellipse.FractionParameterToDerivatives (point, vector, derivative2, fraction);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_FractionToPoint(double fraction, DPoint3dR point, DVec3dR vector, DVec3dR derivative2, DVec3dR derivative3) const
    {
    m_ellipse.FractionParameterToDerivatives (point, vector, derivative2, fraction);
    derivative3.Zero ();
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_Length(double &length) const
    {
    length = m_ellipse.ArcLength ();
    return true;
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2015
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_Length(RotMatrixCP worldToLocal, double &length) const
    {
    DEllipse3d ellipse = m_ellipse;
    worldToLocal->Multiply (ellipse.vector0);
    worldToLocal->Multiply (ellipse.vector90);
    length = ellipse.ArcLength ();
    return true;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_GetRange(DRange3dR range) const
    {
    m_ellipse.GetRange (range);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_GetRange(DRange3dR range, TransformCR transform) const
    {
    DEllipse3d ellipseA;
    transform.Multiply (ellipseA, m_ellipse);
    ellipseA.GetRange (range);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double CurvePrimitiveArc::_FastMaxAbs() const
    {
    // No ellipse coordinate can get outside the sum of largest entries in center and vectors...
    return m_ellipse.center.MaxAbs ()
            + m_ellipse.vector0.MaxAbs ()
            + m_ellipse.vector90.MaxAbs ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const
    {
    DEllipse3d otherEllipse;
    if (!other.TryGetArc (otherEllipse))
        return false;
    return m_ellipse.IsAlmostEqual (otherEllipse, tolerance);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2013
+--------------------------------------------------------------------------------------*/
size_t CurvePrimitiveArc::_NumComponent () const {return 1;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DRange1d CurvePrimitiveArc::_ProjectedParameterRange(DRay3dCR ray, double fractionA, double fractionB) const
    {
    return DEllipse3d::FromFractionInterval (m_ellipse, fractionA, fractionB).ProjectedParameterRange (ray);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DRange1d CurvePrimitiveArc::_ProjectedParameterRange(DRay3dCR ray) const
    {
    return m_ellipse.ProjectedParameterRange (ray);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_GetBreakFraction(size_t breakFractionIndex, double &fraction) const
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
bool CurvePrimitiveArc::_AdjustFractionToBreakFraction(double fraction, Rounding::RoundingMode mode, size_t &breakIndex, double &adjustedFraction) const
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
bool CurvePrimitiveArc::_GetMSBsplineCurve(MSBsplineCurveR curve, double fractionA, double fractionB) const
        {
        if (DoubleOps::IsExact01 (fractionA, fractionB))
            {
            return SUCCESS ==curve.InitFromDEllipse3d (m_ellipse);
            }
        else
            {
            DEllipse3d newArc = m_ellipse;
            newArc.start = m_ellipse.FractionToAngle (fractionA);
            newArc.sweep = (fractionB - fractionA) * m_ellipse.sweep;
            return SUCCESS == curve.InitFromDEllipse3d (newArc);
            }
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_AddStrokes(bvector <DPoint3d> &points, IFacetOptionsCR options,
                bool includeStartPoint,
                double startFraction,
                double endFraction
                ) const
        {
        DEllipse3d ellipse = DEllipse3d::FromFractionInterval (m_ellipse, startFraction, endFraction);

        size_t count = options.EllipseStrokeCount (ellipse);
        DPoint3d startPoint, endPoint;
        ellipse.EvaluateEndPoints (startPoint, endPoint);
        
        PolylineOps::AddContinuationStartPoint (points, startPoint, includeStartPoint);
        if (count > 1)
            {
            double df = 1.0 / (double) (count - 1);
            for (size_t i = 1; i < count - 1; i++)
                {
                DPoint3d xyz;
                ellipse.FractionParameterToPoint (xyz, i*df);;
                points.push_back (xyz);
                }
            }
        points.push_back (endPoint);

        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_AddStrokes(bvector <PathLocationDetail> &points, IFacetOptionsCR options,
                double startFraction,
                double endFraction
                ) const 
        {
        DEllipse3d localEllipse = DEllipse3d::FromFractionInterval (m_ellipse, startFraction, endFraction);
        bool isCircular = m_ellipse.IsCircular ();
        double activeLength = localEllipse.ArcLength ();

        size_t edgeCount = options.EllipseStrokeCount (localEllipse);
        if (edgeCount < 1)
            edgeCount = 1;
        double df = 1.0 / (double) (edgeCount - 1);
        double distance0 = points.size () == 0 ? 0.0 : points.back ().DistanceFromPathStart ();
        CurveLocationDetail cd;
        FractionToPoint (startFraction, cd);
        points.push_back (PathLocationDetail (cd, 0, distance0));
        for (size_t i = 1; i < edgeCount; i++)
            {
            double g = i *df;   // fraction within localEllipse
            double f = DoubleOps::Interpolate (startFraction, g, endFraction);  // fraction within total ellipse.
            FractionToPoint (f, cd);
            double currentDistance = 0.0;
            if (isCircular)
                currentDistance = g * activeLength;
            else
                m_ellipse.FractionToLength (currentDistance, startFraction, f);
            points.push_back (PathLocationDetail (cd, 0, distance0 + currentDistance));
            }
        FractionToPoint (endFraction, cd);
        points.push_back (PathLocationDetail (cd, 0, distance0 + activeLength));

        return true;
        }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t CurvePrimitiveArc::_GetStrokeCount(IFacetOptionsCR options, double startFraction, double endFraction) const
    {
    DEllipse3d ellipse = DEllipse3d::FromFractionInterval (m_ellipse, startFraction, endFraction);
    return options.EllipseStrokeCount (ellipse);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_SignedDistanceBetweenFractions(double startFraction, double endFraction, double &signedDistance) const
    {
    m_ellipse.FractionToLength (signedDistance, startFraction, endFraction);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_SignedDistanceBetweenFractions(RotMatrixCP worldToLocal, double startFraction, double endFraction, double &signedDistance) const
    {
    DEllipse3d localEllipse = m_ellipse;
    worldToLocal->Multiply (localEllipse.vector0);
    worldToLocal->Multiply (localEllipse.vector90);
    localEllipse.FractionToLength (signedDistance, startFraction, endFraction);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_PointAtSignedDistanceFromFraction(double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const
    {
    location = CurveLocationDetail (this, 1);
    DEllipse3d workEllipse = m_ellipse;
    workEllipse.start = m_ellipse.start + startFraction * m_ellipse.sweep;
    // We want the inversion step to see positive for both sweep direction and signedDistance ...
    if (m_ellipse.sweep * signedDistance < 0.0)
        {
        workEllipse.vector90.Negate ();
        workEllipse.start = - workEllipse.start;
        }
    workEllipse.sweep = msGeomConst_2pi;
    
    double radiansStep = workEllipse.InverseArcLength (fabs (signedDistance));
    if (signedDistance < 0.0)
        radiansStep = - radiansStep;
    double fractionStep;
    if (DoubleOps::SafeDivideParameter (fractionStep, radiansStep, fabs (m_ellipse.sweep), 0.0))
        location.SetSingleComponentFractionAndA (startFraction + fractionStep, signedDistance);
    else
        location.SetSingleComponentFractionAndA (startFraction, 0.0);
    m_ellipse.FractionParameterToPoint (location.point, location.fraction);

    return true;
    }

bool CurvePrimitiveArc::_PointAtSignedDistanceFromFraction(RotMatrixCP worldToLocal, double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const 
    {
    location = CurveLocationDetail (this, 1);
    DEllipse3d workEllipse = m_ellipse;
    worldToLocal->Multiply (workEllipse.vector0);
    worldToLocal->Multiply (workEllipse.vector90);
    workEllipse.start = m_ellipse.start + startFraction * m_ellipse.sweep;
    // We want the inversion step to see positive for both sweep direction and signedDistance ...
    if (m_ellipse.sweep * signedDistance < 0.0)
        {
        workEllipse.vector90.Negate ();
        workEllipse.start = - workEllipse.start;
        }
    workEllipse.sweep = msGeomConst_2pi;
    
    double radiansStep = workEllipse.InverseArcLength (fabs (signedDistance));
    if (signedDistance < 0.0)
        radiansStep = - radiansStep;
    double fractionStep;
    if (DoubleOps::SafeDivideParameter (fractionStep, radiansStep, fabs (m_ellipse.sweep), 0.0))
        location.SetSingleComponentFractionAndA (startFraction + fractionStep, signedDistance);
    else
        location.SetSingleComponentFractionAndA (startFraction, 0.0);
    m_ellipse.FractionParameterToPoint (location.point, location.fraction);

    return true;
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
//using ICurvePrimitive::_ClosestPointBounded;    // suppresses C4266


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_ClosestPointBounded(DPoint3dCR spacePoint, double &fraction, DPoint3dR curvePoint, bool extend0, bool extend1) const
    {
    double radians, distanceSquared;
    if (extend0 || extend1)
        {
        DEllipse3d fullEllipse = m_ellipse;
        fullEllipse.MakeFullSweep ();
        if (fullEllipse.ClosestPointBounded (radians, distanceSquared, curvePoint, spacePoint))
            {
            fraction = Angle::NormalizeToSweep (radians, m_ellipse.start, m_ellipse.sweep);
            return true;
            }
        }
    else if (m_ellipse.ClosestPointBounded (radians, distanceSquared, curvePoint, spacePoint))
        {
        fraction = m_ellipse.AngleToFraction (radians);
        return true;
        }
    fraction = 0.0;
    curvePoint.Zero ();
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_ClosestPointBoundedXY(DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location, bool extend0, bool extend1) const
    {
    location = CurveLocationDetail (this, 1);
    m_ellipse.ClosestPointBoundedXY (location.point, location.fraction, location.a, spacePoint, worldToLocal, extend0, extend1);
    location.SetSingleComponentData ();
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/15
+--------------------------------------------------------------------------------------*/
void CurvePrimitiveArc::_AnnounceKeyPoints
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

    if (collector.NeedKeyPointType (CurveKeyPointCollector::KeyPointType::Perpendicular))
        {
        if (xy)
            {
            }
        else
            {
            DPoint3d xyz[8];
            double radians[8];

            int numProjection = m_ellipse.ProjectPoint (xyz, radians, spacePoint);
            for (int i = 0; i < numProjection; i++)
                {
                double f = m_ellipse.AngleToFraction (radians[i]);
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
bool CurvePrimitiveArc::_TransformInPlace(TransformCR transform)
    {
    transform.Multiply (m_ellipse, m_ellipse);
    DVec3d newZ;
    newZ.NormalizedCrossProduct (m_ellipse.vector0, m_ellipse.vector90);
    newZ.Magnitude ();
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_ReverseCurvesInPlace ()
    {
    DEllipse3d old = m_ellipse;
    m_ellipse.InitReversed (old);   // probably don't need the copy, but avoid any aliasing possibility
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurvePrimitiveArc::_AppendCurvePlaneIntersections(DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tol) const
    {
    AppendTolerancedPlaneIntersections (plane, this, m_ellipse, intersections, tol);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurvePrimitiveArc::_WireCentroid(double &length, DPoint3dR centroid, double fraction0, double fraction1) const
    {
    m_ellipse.WireCentroid (length, centroid, fraction0, fraction1);
    return true;
    }


ICurvePrimitive* CurvePrimitiveArc::Create (DEllipse3dCR ellipse) {return new CurvePrimitiveArc (ellipse);}




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateArc (DEllipse3dCR ellipse)
    {return CurvePrimitiveArc::Create (ellipse);}


END_BENTLEY_GEOMETRY_NAMESPACE

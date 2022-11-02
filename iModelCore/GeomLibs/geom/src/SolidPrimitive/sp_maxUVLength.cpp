/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static bool tryGetMaxUVLength(DEllipse3dCR ellipse, DVec2dR uvLength)
    {
    uvLength.x = 2.0 * ellipse.vector0.Magnitude ();
    uvLength.y = 2.0 * ellipse.vector90.Magnitude ();
    return true;
    }

// ASSUME ellipses are circular -- vector0 gives radii.
// ASSUME ellipse axes are parallel -- no twist concerns.
static double CircularConeSideLength (DEllipse3dCR circle0, DEllipse3dCR circle1)
    {
    double r0 = circle0.vector0.Magnitude ();
    double r1 = circle1.vector0.Magnitude ();
    DPoint3d xyzProjected;
    double cc, ss, d, dr, h;
    dr = fabs (r0 - r1);
    // For equal radii, the center-to-center distance is the side length.
    // For unequal radii, project center of smaller circle to plane of larger.
    // side length is from triangle.
    // vertical leg of triangle is distance to projection.
    // horizontal leg is radius difference plus distance of projection to center in projection plane.
    if (DoubleOps::AlmostEqual (r0, r1))
        {
        return circle0.center.Distance (circle1.center);
        }
    else if (r0 > r1 && circle0.ProjectPointToPlane (xyzProjected, cc, ss, circle1.center))
        {
        d = circle0.center.Distance (xyzProjected);
        h = circle1.center.Distance (xyzProjected);
        return DoubleOps::Magnitude (h, dr + d);
        }
    else if (circle1.ProjectPointToPlane (xyzProjected,  cc, ss, circle0.center))
        {
        d = circle0.center.Distance (xyzProjected);
        h = circle1.center.Distance (xyzProjected);
        return DoubleOps::Magnitude (h, dr + d);
        }
    // Can't get here? 0 radii got to first branch.  Nonzero will succeed in projection.
    return circle0.center.Distance (circle1.center);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const
    {
    uvLength.Zero ();
    if (indices.IsCap0 () && m_capped)
        {
        DEllipse3d ellipse = VFractionToUSectionDEllipse3d (0.0);
        return tryGetMaxUVLength (ellipse, uvLength);
        }
    else if (indices.IsCap1 () && m_capped)
        {
        DEllipse3d ellipse = VFractionToUSectionDEllipse3d (1.0);
        return tryGetMaxUVLength (ellipse, uvLength);
        }
    else if (indices.Is (0,0))
        {
        DEllipse3d majorHoop = UFractionToVSectionDEllipse3d (0.0); // larger hoop ???
        DEllipse3d minorHoop = VFractionToUSectionDEllipse3d (0.0);
        uvLength.x = minorHoop.ArcLength ();
        uvLength.y = majorHoop.ArcLength ();
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DgnSphereDetail::LatitudeToVFraction (double latitude) const
    {
    double v;
    DoubleOps::SafeDivideParameter (v, latitude - m_startLatitude, m_latitudeSweep, 0.0);
    return v;
    }

double DgnSphereDetail::LongitudeToUFraction (double longitude) const
    {
    return Angle::NormalizeToSweep (longitude, 0.0, Angle::TwoPi ());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DgnSphereDetail::VFractionToLatitude (double v) const
    {
    return m_startLatitude + v * m_latitudeSweep;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const
    {
    uvLength.Zero ();
    if (indices.IsCap0 () && IsRealCap (0))
        return tryGetMaxUVLength (VFractionToUSectionDEllipse3d (0.0), uvLength);
    else if (indices.IsCap1 () && IsRealCap (1))
        return tryGetMaxUVLength (VFractionToUSectionDEllipse3d (1.0), uvLength);
    else if (indices.Is (0,0))
        {
        // Equator is always bigger than any cap ...
        if (Angle::InExactSweep (0.0, m_startLatitude, m_latitudeSweep))
            {
            uvLength.x = VFractionToUSectionDEllipse3d (LatitudeToVFraction (0.0)).ArcLength ();
            }
        else
            {
            if (IsRealCap (0))
                uvLength.x = DoubleOps::Max (uvLength.x, VFractionToUSectionDEllipse3d (0.0).ArcLength ());
            if (IsRealCap (1))
                uvLength.x = DoubleOps::Max (uvLength.x, VFractionToUSectionDEllipse3d (1.0).ArcLength ());
            }
        DEllipse3d meridian = UFractionToVSectionDEllipse3d (0.0);
        // Are all meridians identical length?  dunno.
        uvLength.y = meridian.ArcLength ();
        return true;
        }
    return false;
    }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const
    {
    uvLength.Zero ();
    DEllipse3d ellipse, ellipse1;
    if (indices.IsCap0 () && IsRealCap (0) && FractionToSection (0.0, ellipse))
        return tryGetMaxUVLength (ellipse, uvLength);
    else if (indices.IsCap1 () && IsRealCap (1) && FractionToSection (1.0, ellipse))
        return tryGetMaxUVLength (ellipse, uvLength);
    else if (indices.Is (0,0))
        {
        FractionToSection (0.0, ellipse);
        FractionToSection (1.0, ellipse1);
        uvLength.x = DoubleOps::Max (ellipse.ArcLength (), ellipse1.ArcLength ());
        uvLength.y = CircularConeSideLength (ellipse, ellipse1);
        return true;
        }
    return false;
    }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnBoxDetail::TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const
    {
    uvLength.Zero ();
    BoxFaces cornerData;
    int singleIndex;
    cornerData.Load (*this);
    DPoint3d facePoints[5];
    if (   BoxFaces::TrySingleFaceIndex (indices, singleIndex))
        {
        cornerData.Get5PointCCWFace (singleIndex, facePoints);
        uvLength.x = DoubleOps::Max (
                    facePoints[0].Distance (facePoints[1]),
                    facePoints[3].Distance (facePoints[2])
                    );
        uvLength.y = DoubleOps::Max (
                    facePoints[0].Distance (facePoints[3]),
                    facePoints[1].Distance (facePoints[2])
                    );
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsistruct
+--------------------------------------------------------------------------------------*/
struct StrokeMatcher
{
IFacetOptionsPtr options;
StrokeMatcher ()
    {
    options = IFacetOptions::CreateForCurves ();
    }
GEOMAPI_VIRTUAL ~StrokeMatcher (){}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
GEOMAPI_VIRTUAL void ProcessPointPair(double f, DPoint3dCR xyz0, DPoint3dCR xyz1)
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ProcessPrimitivePair(ICurvePrimitiveCR prim0, ICurvePrimitiveCR prim1)
    {
    size_t num0 = prim0.GetStrokeCount (*options);
    size_t num1 = prim1.GetStrokeCount (*options);
    size_t num = num1 > num0 ? num1 : num0;
    if (num < 2)
        num = 2;
    double df = 1.0 / (double)(num - 1);
    for (size_t i = 0; i <= num; i++)
        {
        DPoint3d xyz0, xyz1;
        double f = i * df;
        prim0.FractionToPoint (f, xyz0);
        prim1.FractionToPoint (f, xyz1);
        ProcessPointPair (f, xyz0, xyz1);
        }
    }
};


/*--------------------------------------------------------------------------------**//**
* @bsistruct
+--------------------------------------------------------------------------------------*/
struct CalculateMaxRuledDistance : StrokeMatcher
    {
private:
    double m_maxDist;
public:
    double GetDistance () { return m_maxDist;}
    CalculateMaxRuledDistance ()
        {
        m_maxDist = 0.0;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ProcessPointPair(double f, DPoint3dCR xyz0, DPoint3dCR xyz1) override 
        {
        m_maxDist = DoubleOps::Max (m_maxDist, xyz0.Distance (xyz1));
        }
    };


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static bool tryGetMaxUVLength(CurveVectorCR curves, DVec2dR uvLength)
    {
    Transform localToWorld, worldToLocal;
    DRange3d range;
    CurveVectorPtr localCurves = curves.CloneInLocalCoordinates
                (
                LOCAL_COORDINATE_SCALE_01RangeBothAxes,
                localToWorld, worldToLocal,
                range
                );
    DVec3d xVector, yVector;
    localToWorld.GetMatrixColumn (xVector, 0);
    localToWorld.GetMatrixColumn (yVector, 1);
    uvLength.x = xVector.Magnitude ();
    uvLength.y = yVector.Magnitude ();
    return true;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const
    {
    uvLength.Zero ();
    if (indices.IsCap0 () && m_capped)
        {
        return tryGetMaxUVLength (*m_baseCurve, uvLength);
        }
    else if (indices.IsCap1 () && m_capped)
        {
        // No need to apply the extrusion transform -- it does not affect measured lengths.
        return tryGetMaxUVLength (*m_baseCurve, uvLength);
        }
    else if (indices.Index0 () == 0 && indices.Index1 () >= 0)
        {
        ICurvePrimitivePtr curveA = m_baseCurve->FindIndexedLeaf ((size_t)indices.Index1 ());
        if (curveA.IsValid ())
            {
            bvector<DPoint3d> points;
            IFacetOptionsPtr options = IFacetOptions::CreateForCurves ();
            curveA->AddStrokes (points, *options);
            double d2Max = 0.0;
            for (size_t i = 0; i < points.size (); i++)
                {
                DPoint3d xyz0 = points[i];
                DPoint3d xyz1;
                double rayFraction;
                m_axisOfRotation.ProjectPointUnbounded (xyz1, rayFraction, xyz0);
                d2Max = DoubleOps::Max (d2Max, xyz0.DistanceSquared (xyz1));
                }
            curveA->Length (uvLength.x);
            uvLength.y = m_sweepAngle * sqrt (d2Max);
            return true;
            }
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::TryGetCurvePair (
SolidLocationDetail::FaceIndices const &indices,
ICurvePrimitivePtr &curveA,
ICurvePrimitivePtr &curveB
) const
    {
    int i0 = (int)indices.Index0();
    int i1 = (int)indices.Index1();
    if (i0 < 0 || (size_t)i0 + 2 > m_sectionCurves.size ())
        return false;
    curveA = m_sectionCurves[i0]->FindIndexedLeaf ((size_t)i1);
    curveB = m_sectionCurves[i0 + 1]->FindIndexedLeaf ((size_t)i1);
    return curveA.IsValid () && curveB.IsValid ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const
    {
    uvLength.Zero ();

    if (indices.IsCap0 () && m_capped)
        {
        return tryGetMaxUVLength (*m_sectionCurves.front (), uvLength);
        }
    else if (indices.IsCap1 () && m_capped)
        {
        // No need to apply the extrusion transform -- it does not affect measured lengths.
        return tryGetMaxUVLength (*m_sectionCurves.back (), uvLength);
        }
    else
        {
        ICurvePrimitivePtr curveA, curveB;
        if (TryGetCurvePair (indices, curveA, curveB))
            {
            CalculateMaxRuledDistance distanceCalculator = CalculateMaxRuledDistance ();
            distanceCalculator.ProcessPrimitivePair  (*curveA, *curveB);
            double lengthA, lengthB;
            curveA->Length (lengthA);
            curveB->Length (lengthB);
            uvLength.x = DoubleOps::Max (lengthA, lengthB);
            uvLength.y = distanceCalculator.GetDistance ();
            return true;
            }
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const
    {
    uvLength.Zero ();
    if (indices.IsCap0 () && m_capped)
        {
        return tryGetMaxUVLength (*m_baseCurve, uvLength);
        }
    else if (indices.IsCap1 () && m_capped)
        {
        // No need to apply the extrusion transform -- it does not affect measured lengths.
        return tryGetMaxUVLength (*m_baseCurve, uvLength);
        }
    else if (indices.Index0 () == 0 && indices.Index1 () >= 0)
        {
        ICurvePrimitivePtr curveA = m_baseCurve->FindIndexedLeaf ((size_t)indices.Index1 ());
        if (curveA.IsValid ())
            {
            uvLength.y = m_extrusionVector.Magnitude ();
            curveA->Length (uvLength.x);
            return true;
            }
        }
    return false;
    }

 END_BENTLEY_GEOMETRY_NAMESPACE



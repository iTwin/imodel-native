/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/SolidPrimitive/sp_Range.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


double DgnTorusPipeDetail::GetVector90Sign () { return -1.0;}
bool DgnTorusPipeDetail::GetReverseVector90 () { return true;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DgnTorusPipeDetail::VFractionToUSectionDEllipse3d (double fraction) const
    {
    double vSign = GetVector90Sign ();
    double theta = fraction * m_sweepAngle;
    double c = cos (theta);
    double s = sin (theta);
    DVec3d vectorU, vectorV;
    vectorU.SumOf (m_vectorX, c, m_vectorY, s);
    vectorV.SumOf (m_vectorX, -s * vSign, m_vectorY, c * vSign);
    DPoint3d center =  DPoint3d::FromSumOf (m_center, vectorU, m_majorRadius);
    DVec3d unitW;
    unitW.NormalizedCrossProduct (vectorU, vectorV);
    if (m_sweepAngle < 0.0)
        unitW.Negate ();
    return DEllipse3d::FromScaledVectors (center, vectorU, unitW, m_minorRadius, m_minorRadius, 0.0, Angle::TwoPi());
    }
bool DgnTorusPipeDetail::HasRealCaps () const
    {
    return m_capped && !Angle::IsFullCircle (m_sweepAngle);
    }

bool DgnRotationalSweepDetail::HasRealCaps () const
    {
    return m_capped && !Angle::IsFullCircle (m_sweepAngle);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DgnSphereDetail::UFractionToVSectionDEllipse3d (double fraction) const
    {
    double theta = fraction * Angle::TwoPi ();
    DPoint3d center;
    DVec3d vectorX, vectorY, vectorZ, vectorF;
    m_localToWorld.GetOriginAndVectors (center, vectorX, vectorY, vectorZ);
    vectorF.SumOf (vectorX, cos(theta), vectorY, sin(theta));
    return DEllipse3d::FromVectors (center, vectorF, vectorZ,
                    m_startLatitude, m_latitudeSweep);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DgnSphereDetail::VFractionToUSectionDEllipse3d (double fraction) const
    {
    double phi = m_startLatitude + fraction * m_latitudeSweep;
    double c = cos (phi);
    double s = sin (phi);
    DPoint3d center;
    DVec3d vectorX, vectorY, vectorZ;
    m_localToWorld.GetOriginAndVectors (center, vectorX, vectorY, vectorZ);

    DPoint3d planeCenter = DPoint3d::FromSumOf (center, vectorZ, s);
    return DEllipse3d::FromScaledVectors (planeCenter, vectorX, vectorY, c, c,
                    0.0, Angle::TwoPi ());
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DgnTorusPipeDetail::UFractionToVSectionDEllipse3d (double fraction) const
    {
    double phi = fraction * Angle::TwoPi ();
    double cR = cos (phi) * m_minorRadius;
    double sR = sin (phi) * m_minorRadius * GetVector90Sign ();;
    DVec3d unitW, vectorU, vectorV;
    unitW.NormalizedCrossProduct (m_vectorX, m_vectorY);
    
    vectorU.SumOf (m_vectorX, m_majorRadius, m_vectorX, cR);
    vectorV.SumOf (m_vectorY, m_majorRadius, m_vectorY, cR);
    DPoint3d center = DPoint3d::FromSumOf (m_center, unitW, sR);
    return DEllipse3d::FromVectors (center, vectorU, vectorV, 0.0, m_sweepAngle);
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::GetRange (DRange3dR range) const
    {
    range.Init ();
    static size_t numChord = 24;   //ugh.  Just a sample.
    double df = 1.0 / (double)numChord;
    for (size_t i = 0; i <= numChord; i++)
        {
        range.Extend (VFractionToUSectionDEllipse3d (i * df));
        }
    // Allow for chordal effects ..
    double delta = 0.05 * range.low.Distance (range.high);
    range.Extend (delta);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::GetRange (DRange3dR range, TransformCR transform) const
    {
    range.Init ();
    static size_t numChord = 24;   //ugh.  Just a sample.
    double df = 1.0 / (double)numChord;
    for (size_t i = 0; i <= numChord; i++)
        {
        DEllipse3d ellipse = VFractionToUSectionDEllipse3d (i * df);
        transform.Multiply (ellipse);
        range.Extend (ellipse);
        }
    // Allow for chordal effects ..
    double delta = 0.05 * range.low.Distance (range.high);
    range.Extend (delta);
    return true;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::GetRange (DRange3dR range) const
    {
    range.Init ();
    DRange3d rangeA, rangeB;
    DEllipse3d::FromScaledVectors (m_centerA, m_vector0, m_vector90, m_radiusA, m_radiusA, 0.0, Angle::TwoPi ()).GetRange (rangeA);
    range.Extend (rangeA);
    DEllipse3d::FromScaledVectors (m_centerB, m_vector0, m_vector90, m_radiusB, m_radiusB, 0.0, Angle::TwoPi ()).GetRange (rangeB);
    range.Extend (rangeB);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::GetRange (DRange3dR range, TransformCR transform) const
    {
    range.Init ();
    DRange3d rangeA, rangeB;
    
    DEllipse3d ellipseA = DEllipse3d::FromScaledVectors (m_centerA, m_vector0, m_vector90, m_radiusA, m_radiusA, 0.0, Angle::TwoPi ());
    transform.Multiply (ellipseA);
    ellipseA.GetRange (rangeA);
    range.Extend (rangeA);
    
    DEllipse3d ellipseB = DEllipse3d::FromScaledVectors (m_centerB, m_vector0, m_vector90, m_radiusB, m_radiusB, 0.0, Angle::TwoPi ());
    transform.Multiply (ellipseB);
    ellipseB.GetRange (rangeB);
    range.Extend (rangeB);
    
    return true;
    }

// Needs work: sweep range is not considered.
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::GetRange (DRange3dR range) const
    {
    DPoint3d center;
    DVec3d vectorX, vectorY, vectorZ;
    m_localToWorld.GetOriginAndVectors (center, vectorX, vectorY, vectorZ);
    range.low = range.high = center;
    double ax = DoubleOps::Magnitude (vectorX.x, vectorY.x, vectorZ.x);
    double ay = DoubleOps::Magnitude (vectorX.y, vectorY.y, vectorZ.y);
    double az = DoubleOps::Magnitude (vectorX.z, vectorY.z, vectorZ.z);
    range.low.x -= ax;
    range.high.x += ax;

    range.low.y -= ay;
    range.high.y += ay;

    range.low.z -= az;
    range.high.z += az;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::GetRange (DRange3dR range, TransformCR transform) const
    {
    DPoint3d center;
    DVec3d vectorX, vectorY, vectorZ;
    m_localToWorld.GetOriginAndVectors (center, vectorX, vectorY, vectorZ);
    transform.Multiply (center);
    transform.MultiplyMatrixOnly (vectorX);
    transform.MultiplyMatrixOnly (vectorY);
    transform.MultiplyMatrixOnly (vectorZ);
    range.low = range.high = center;
    double ax = DoubleOps::Magnitude (vectorX.x, vectorY.x, vectorZ.x);
    double ay = DoubleOps::Magnitude (vectorX.y, vectorY.y, vectorZ.y);
    double az = DoubleOps::Magnitude (vectorX.z, vectorY.z, vectorZ.z);
    range.low.x -= ax;
    range.high.x += ax;

    range.low.y -= ay;
    range.high.y += ay;

    range.low.z -= az;
    range.high.z += az;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void ExtendByRectangle(DRange3dR range, DPoint3dCR origin, DVec3dCR vectorX, double sizeX, DVec3dCR vectorY, double sizeY)
    {
    range.Extend (origin);
    DPoint3d point10, point11, point01;
    point10.SumOf (origin, vectorX, sizeX);
    range.Extend (point10);
    point11.SumOf (point10, vectorY, sizeY);
    range.Extend (point11);
    point01.SumOf (origin, vectorY, sizeY);
    range.Extend (point01);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void ExtendByRectangle(DRange3dR range, TransformCR transform, DPoint3dCR origin, DVec3dCR vectorX, double sizeX, DVec3dCR vectorY, double sizeY)
    {
    DPoint3d point00 = origin;
    transform.Multiply (point00);
    range.Extend (point00);
    
    DPoint3d point10, point11, point01;
    point10.SumOf (origin, vectorX, sizeX);
    point11.SumOf (point10, vectorY, sizeY);

    transform.Multiply (point10);
    range.Extend (point10);

    transform.Multiply (point11);
    range.Extend (point11);

    point01.SumOf (origin, vectorY, sizeY);
    transform.Multiply (point01);
    range.Extend (point01);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnBoxDetail::GetRange (DRange3dR range) const
    {
    range.Init ();
    ExtendByRectangle (range, m_baseOrigin, m_vectorX, m_baseX, m_vectorY, m_baseY);
    ExtendByRectangle (range, m_topOrigin, m_vectorX, m_topX, m_vectorY, m_topY);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnBoxDetail::GetRange (DRange3dR range, TransformCR transform) const
    {
    range.Init ();
    ExtendByRectangle (range, transform, m_baseOrigin, m_vectorX, m_baseX, m_vectorY, m_baseY);
    ExtendByRectangle (range, transform, m_topOrigin, m_vectorX, m_topX, m_vectorY, m_topY);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::GetRange (DRange3dR range) const
    {
    range.Init ();
    if (NULL == m_baseCurve.get ())
        return false;
    m_baseCurve->GetRange (range);
    if (range.IsNull ())
        return false;
    range.ExtendBySweep (m_extrusionVector);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::GetRange (DRange3dR range, TransformCR transform) const
    {
    range.Init ();
    if (NULL == m_baseCurve.get ())
        return false;
    m_baseCurve->GetRange (range, transform);
    if (range.IsNull ())
        return false;
    DVec3d extrusion = m_extrusionVector;
    transform.MultiplyMatrixOnly (extrusion);
    range.ExtendBySweep (extrusion);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::GetRange (DRange3dR range) const
    {
    range.Init ();
    for (size_t i = 0; i < m_sectionCurves.size (); i++)
        {
        DRange3d newRange;
        m_sectionCurves[i]->GetRange (newRange);
        range.Extend (newRange);
        }
    return !range.IsNull ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::GetRange (DRange3dR range, TransformCR transform) const
    {
    range.Init ();
    for (size_t i = 0; i < m_sectionCurves.size (); i++)
        {
        DRange3d newRange;
        m_sectionCurves[i]->GetRange (newRange, transform);
        range.Extend (newRange);
        }
    return !range.IsNull ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::GetRange (DRange3dR range, TransformCR transform) const
    {
    static double s_angleTolerance = 0.1;
    static double s_sampleErrorFraction = 0.02;
    Transform localToWorld, worldToLocal;
    
    // Compute points on the base curve.
    // For each point, compute the (exact) range of the elliptic curve it sweeps on the rotation surface.
    // Note that the point selection is only approximate -- the true range extremes may be
    //    at some unsampled point.  Because sampling is only one one direction, 
    //    
    DMatrix4d localConstEffects, localCosineEffects, localSineEffects;
    // decompose the "rotate around z" transform as
    // [c -1 0 0]     [0 0 0 0]     [1 0 0 0]     [0 -1 0 0]
    // [s  c 0 0]  =  [0 0 0 0] + c*[0 1 0 0] + s*[1  0 0 0]  = constEffects + c * localCosineEffects + s * localSineEffects
    // [0  0 1 0]     [0 0 1 0]     [0 0 0 0]     [0  0 0 0]
    // [0  0 0 1]     [0 0 0 1]     [0 0 0 0]     [0  0 0 0]
    localConstEffects.InitFromRowValues (
            0,0,0,0,
            0,0,0,0,
            0,0,1,0,
            0,0,0,1
            );

    localCosineEffects.InitFromRowValues (
            1,0,0,0,
            0,1,0,0,
            0,0,0,0,
            0,0,0,0
            );

    localSineEffects.InitFromRowValues (
            0,-1, 0,0,
            1, 0, 0,0,
            0, 0, 0,0,
            0, 0, 0,0
            );

    range.Init ();

    if (m_baseCurve.IsValid () && GetTransforms (localToWorld, worldToLocal))
        {
        DMatrix4d globalConstEffects, globalCosineEffects, globalSineEffects;
        DMatrix4d localToWorld4d = DMatrix4d::From (localToWorld);
        DMatrix4d worldToLocal4d = DMatrix4d::From (worldToLocal);
        // For a point X on the base curve, the swept point at angle theta with cosine and sine (c,s)
        // Y = localToWorld * (constEffects + c * localCosineEffects + s * localSineEffects) * worldToLocal * X
        //   =   localToWorld * constEffects * worldToLocal * X
        //   + c*localToWorld * localCosineEffects * worldTolocal * X
        //   + s*localToWorld * localSineEffects * worldToLocal * X
        //   =  A * X + c * C * X + s * S * X
        // where A, C, S can be precomputed.
        globalConstEffects.InitProduct (localToWorld4d, localConstEffects,  worldToLocal4d);
        globalCosineEffects.InitProduct (localToWorld4d, localCosineEffects, worldToLocal4d);
        globalSineEffects.InitProduct (localToWorld4d, localSineEffects,   worldToLocal4d);

        bvector <DPoint3d> strokes;
        double samplingError = 0.0;
        IFacetOptionsPtr facetOptions = IFacetOptions::New ();
        facetOptions->SetAngleTolerance (s_angleTolerance);
        m_baseCurve->AddStrokePoints (strokes, *facetOptions);

        for(DPoint3d xyz : strokes)
            {
            DPoint4d AX, CX, SX;

            double w1 = 1.0;
            globalConstEffects.Multiply (&AX, &xyz, &w1, 1);
            globalCosineEffects.Multiply (&CX, &xyz, &w1, 1);
            globalSineEffects.Multiply (&SX, &xyz, &w1, 1);
            assert (AX.w == 1.0);
            assert (CX.w == 0.0);
            assert (SX.w == 0.0);
            DEllipse3d ellipse = DEllipse3d::From (
                        AX.x, AX.y, AX.z,
                        CX.x, CX.y, CX.z,
                        SX.x, SX.y, SX.z,
                        0.0, m_sweepAngle);
            transform.Multiply (ellipse);
            range.Extend (ellipse);
            }
        samplingError = s_sampleErrorFraction * range.low.Distance (range.high); 
        range.Extend (samplingError);
        }
    return !range.IsNull ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::GetRange (DRange3dR range) const
    {
    return GetRange (range, Transform::FromIdentity ());
    }

END_BENTLEY_GEOMETRY_NAMESPACE



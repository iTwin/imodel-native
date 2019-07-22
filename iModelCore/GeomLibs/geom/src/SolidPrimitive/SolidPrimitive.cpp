/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "BoxFaces.h"
#include "../DeprecatedFunctions.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// Macro to invoke m_data detail methods.
// every detail type supports the methods for publication as simple (nonvirtual) methods
//  with no underbar.
// Every type also has the nonpublished virtual (with underbar) so that the containing ISolidPrimitivePtr
//   can route to it as a virtual call.
//
#define EXPAND_SOLID_PRIMITIVE_VIRTUALS(Type) \
bool _GetRange (DRange3dR range) const override\
    {return m_data.GetRange (range);}\
bool _GetRange (DRange3dR range, TransformCR transform) const override\
    {return m_data.GetRange (range, transform);}\
bool _TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const override\
    {\
    return m_data.TryGetConstructiveFrame (localToWorld, worldToLocal);\
    }    \
void _AddRayIntersections (\
bvector<SolidLocationDetail> &pickData,\
DRay3dCR ray,\
int parentId,\
double minParameter\
) const override\
    {\
    return m_data.AddRayIntersections (pickData, ray, parentId, minParameter);\
    }\
\
void _AddCurveIntersections (\
    CurveVectorCR curves,\
    bvector<CurveLocationDetail> &curvePoints,\
    bvector<SolidLocationDetail> &solidPoints,\
    MeshAnnotationVector &messages\
    ) const override \
    {\
    return m_data.AddCurveIntersections (curves, curvePoints, solidPoints, messages);\
    }\
void _AddCurveIntersections (\
    ICurvePrimitiveCR curve,\
    bvector<CurveLocationDetail> &curvePoints,\
    bvector<SolidLocationDetail> &solidPoints,\
    MeshAnnotationVector &messages\
    ) const override \
    {\
    return m_data.AddCurveIntersections (curve, curvePoints, solidPoints, messages);\
    }\
bool _ClosestPoint (DPoint3dCR spacePoint, SolidLocationDetailR pickDetail) const override \
    {\
    return m_data.ClosestPoint (spacePoint, pickDetail);\
    }\
\
bool _TryUVFractionToXYZ (\
    SolidLocationDetail::FaceIndices const &selector,\
    double uFraction, double vFraction,\
    DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv\
) const override\
    {\
    return m_data.TryUVFractionToXYZ (selector, uFraction, vFraction, xyz, dXdu, dXdv);\
    }\
\
IGeometryPtr _GetFace (SolidLocationDetail::FaceIndices const &indices) const override \
    {\
    return m_data.GetFace (indices);\
    }\
\
void _GetFaceIndices (bvector<SolidLocationDetail::FaceIndices> &indices) const override \
    {\
    return m_data.GetFaceIndices (indices);\
    }\
ICurvePrimitivePtr _GetConstantVSection (\
    SolidLocationDetail::FaceIndices const & indices, double fraction) const override \
        {return m_data.GetConstantVSection (indices, fraction);} \
ICurvePrimitivePtr _GetConstantUSection (\
    SolidLocationDetail::FaceIndices const & indices, double fraction) const override \
        {return m_data.GetConstantUSection (indices, fraction);} \
bool _TryGetMaxUVLength (\
    SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const override \
        {return m_data.TryGetMaxUVLength (indices, uvLength);} \
void _SetCapped (bool value) override {m_data.m_capped = value;}\
bool _GetCapped () const override {return m_data.m_capped;}\
ISolidPrimitivePtr _Clone () const override {return m_data.Clone ();}\
bool _TransformInPlace (TransformCR transform) override {return m_data.TransformInPlace (transform);}\
bool _ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR products) const override \
    {return m_data.ComputeSecondMomentAreaProducts (localToWorld, products);}\
bool _ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR products) const override \
    {return m_data.ComputeSecondMomentVolumeProducts (localToWorld, products);}\
bool _IsSameStructure (ISolidPrimitiveCR other) const override {return m_data.IsSameStructure (other);}\
bool _IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const  override { return m_data.IsSameStructureAndGeometry (other, tolerance);}\
bool _IsClosedVolume () const  override { return m_data.IsClosedVolume ();}\
bool _SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const override \
    {\
    return m_data.SilhouetteCurves (eyePoint, curves);\
    }

//! Return curves which are silhoutte curves OTHER than hard edges
bool ISolidPrimitive::SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const
    {
    curves = nullptr;
    return _SilhouetteCurves (eyePoint, curves);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      01/2015
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::IsClosedVolume () const
    {
    return _IsClosedVolume ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool QVNormalizedAxes
(
DVec3dCR   vectorX,
DVec3dCR   vectorY,
RotMatrixR rotMatrix
)
    {
    DVec3d unitX, unitY, unitZ;
    unitX.Normalize (vectorX);
    unitZ.NormalizedCrossProduct (unitX, vectorY);
    unitY.NormalizedCrossProduct (unitZ, unitX);
    rotMatrix.InitFromColumnVectors (unitX, unitY, unitZ);
    return rotMatrix.IsOrthogonal ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool ResolveRotationAxis
(
DPoint3dCR centerA,
DVec3dCR vectorX,
DVec3dCR vectorY,
double   sweepA,
int      select,    // select x,y,z axis as rotation axis.
DPoint3dR center,
DVec3dR  axis,
double   &sweep

)
    {
    center = centerA;
    RotMatrix axes;
    sweep = sweepA;
    bool ok = QVNormalizedAxes (vectorX, vectorY, axes);
    axes.GetColumn (axis, select);
    sweep = sweepA;
    if (sweep < 0.0)
        {
        sweep = -sweep;
        axis.Negate ();
        }
    return ok;
    }


// Generate orthogonal localToGlobal and inverse from full transform
static bool QVNormalizedTransforms
(
TransformCR fullLocalToWorld,
TransformR localToWorld,
TransformR worldToLocal
)
    {
    DPoint3d origin;
    DVec3d vectorX, vectorY, vectorZ;
    fullLocalToWorld.GetOriginAndVectors (origin, vectorX, vectorY, vectorZ);
    vectorX.Normalize ();
    vectorZ.NormalizedCrossProduct (vectorX, vectorY);
    vectorY.NormalizedCrossProduct (vectorZ, vectorX);
    localToWorld.InitFromOriginAndVectors (origin, vectorX, vectorY, vectorZ);
    // If vectors are degenerate, inverse will fail.
    return worldToLocal.InverseOf (localToWorld);    
    }

// Generate orthogonal localToGlobal and inverse from origin, x vector, and y vector.
static bool QVNormalizedTransforms
(
DPoint3dCR origin,
DVec3dCR   axis,
TransformR localToWorld,
TransformR worldToLocal
)
    {
    DVec3d unitX, unitY, unitZ;
    axis.GetNormalizedTriad (unitX, unitY, unitZ);
    localToWorld.InitFromOriginAndVectors (origin, unitX, unitY, unitZ);
    // If vectors are degenerate, inverse will fail.
    return worldToLocal.InverseOf (localToWorld);    
    }




// ****************************************************************
//                           DgnTorusPipe
// ****************************************************************



/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct DgnTorusPipe : public ISolidPrimitive
{
friend struct ISolidPrimitive;
private:
DgnTorusPipeDetail m_data;

protected:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TryGetDgnTorusPipeDetail(DgnTorusPipeDetailR data) const override
    {
    data = m_data;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TrySetDgnTorusPipeDetail(DgnTorusPipeDetailCR data) override
    {
    m_data = data;
    return true;
    }

EXPAND_SOLID_PRIMITIVE_VIRTUALS(DgnTorusPipe)


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
SolidPrimitiveType _GetSolidPrimitiveType() const override {return SolidPrimitiveType_DgnTorusPipe;}

public:

  DgnTorusPipe (DgnTorusPipeDetailCR data)
    : m_data (data)
    {}

};

// Detail consructor with complete field list as parameters ...
DgnTorusPipeDetail::DgnTorusPipeDetail
(
DPoint3dCR  center,
DVec3dCR  vectorX,
DVec3dCR  vectorY,
double  majorRadius,
double  minorRadius,
double  sweepAngle,
bool    capped
)
    {
    m_center = center;
    m_vectorX = vectorX;
    m_vectorY = vectorY;
    m_majorRadius = majorRadius;
    m_minorRadius = minorRadius;
    m_sweepAngle = sweepAngle;
    m_capped = capped;
    }

// Detail consructor with complete field list as parameters ...
DgnTorusPipeDetail::DgnTorusPipeDetail
(
DEllipse3dCR arc,
double  minorRadius,
bool    capped
)
    {
    Transform frame = arc.FractionToCenterFrame (0.0);
    DPoint3d center;
    DVec3d vectorX, vectorY, vectorZ;
    frame.GetOriginAndVectors (center, vectorX, vectorY, vectorZ);
    DPoint3d startPoint = arc.FractionToPoint (0.0);
    m_center = center;
    m_vectorX = vectorX;
    m_vectorY = vectorY;
    m_majorRadius = m_center.Distance (startPoint);
    m_minorRadius = minorRadius;
    m_sweepAngle = arc.sweep;
    m_capped = capped;
    }



// Detail consructor with complete field list as parameters ...
DgnTorusPipeDetail::DgnTorusPipeDetail ()
    {
    m_center = DPoint3d::From (0,0,0);
    m_vectorX = DVec3d::From (1,0,0);
    m_vectorY = DVec3d::From (0,1,0);
    m_majorRadius = 1;
    m_minorRadius = 0;
    m_sweepAngle = Angle::PiOver2 ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::TryGetFrame
(
DPoint3dR center,
RotMatrixR axes,
double &radiusA,
double &radiusB,
double &sweepRadians
) const
    {
    center = m_center;
    radiusA = m_majorRadius;
    radiusB = m_minorRadius;
    sweepRadians = m_sweepAngle;
    return QVNormalizedAxes (m_vectorX, m_vectorY, axes);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::TryGetFrame
(
TransformR localToWorld,
TransformR worldToLocal,
double &radiusA,
double &radiusB,
double &sweepRadians
) const
    {
    radiusA = m_majorRadius;
    radiusB = m_minorRadius;
    sweepRadians = m_sweepAngle;
    RotMatrix axes;
    if (QVNormalizedAxes (m_vectorX, m_vectorY, axes))
        {
        localToWorld.InitFrom (axes, m_center);
        worldToLocal.InvertRigidBodyTransformation (localToWorld);
        return true;
        }
    else
        {
        localToWorld.InitFrom (m_center);

        return false;
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::TryGetRotationAxis (DPoint3dR center, DVec3dR axis, double &sweepRadians) const
    {
    return ResolveRotationAxis (m_center, m_vectorX, m_vectorY, m_sweepAngle, 2,
            center, axis, sweepRadians);
    }

static double Check (DPoint3dCR xyz0, DPoint3dCR xyz1)
    {
    double a = xyz0.Distance (xyz1);
    return a;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 01/16
+---------------+---------------+---------------+---------------+---------------+------*/
//! set point, uv coordinates, and uv derivatives vectors 
void DgnTorusPipeDetail::SetDetailCoordinatesFromLocalPipeCoordinates
(
SolidLocationDetail &detail,//!< [in,out] detail to update
DPoint3dCR  localuvw,       //!< [in] coordinates in local fraction space.
TransformCR localToWorld,   //! [in] transform to world coordinates
double &rMajor,            //!< [in] major radius
double &rMinor,             //!< [in] minor radius
double &sweepRadians        //!< [in]
)
    {
    Polynomial::Implicit::Torus torus (rMajor, rMinor, GetReverseVector90 ());
    double theta, phi, r, xyDist;
    torus.XYZToThetaPhiDistance (localuvw, theta, phi, r, xyDist);
    double u = Angle::NormalizeToSweep (phi, 0.0, Angle::TwoPi ());
    double v = Angle::NormalizeToSweep (theta, 0.0, sweepRadians);
    DPoint3d worldXYZ;
    DVec3d dXdu, dXdv;
    DVec3d dUdTheta, dUdPhi;
    torus.EvaluateDerivativesThetaPhi (theta, phi, dUdTheta, dUdPhi);
    DPoint3d uvwCheck = torus.EvaluateThetaPhi (theta, phi);
    Check (localuvw, uvwCheck);
    double a = DoubleOps::ValidatedDivideParameter (1.0, sweepRadians);
    localToWorld.Multiply (worldXYZ, localuvw);
    localToWorld.MultiplyMatrixOnly (dXdu, dUdPhi);
    dXdu.Scale (1.0 / Angle::TwoPi ());
    localToWorld.MultiplyMatrixOnly (dXdv, dUdTheta);
    dXdv.Scale (a);
    detail = SolidLocationDetail (0, 0.0, worldXYZ, u, v, dXdu, dXdv);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 01/16
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP void DgnTorusPipeDetail::IntersectCurveLocal
(
ICurvePrimitiveCR curve,
bvector<double> &curveFractions,
bvector<DPoint3d> &normalizedConePoints,
TransformR localToWorld,
TransformR worldToLocal,
double &rMajor,
double &rMinor,
double &sweepRadians,
bool boundedConeZ
) const
    {
    curveFractions.clear ();
    normalizedConePoints.clear ();
    if (TryGetFrame (localToWorld, worldToLocal, rMajor, rMinor, sweepRadians))
        {
        //static double s_startFraction = 0.012387665756;        // strange problem.   Intesections "at the seam" fail.  For now, yank the seam around a little
#define s_startFraction (0.0)
        auto localHoop = DEllipse3d::From (rMajor, 0.0, 0.0,
                                        rMinor, 0,0,
                                        0,0,rMinor,
                                        s_startFraction * Angle::TwoPi (), Angle::TwoPi());
        DEllipse3d worldHoop;
        localToWorld.Multiply (worldHoop, localHoop);
        auto worldPrim = ICurvePrimitive::CreateArc (worldHoop);
        auto worldVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        worldVector->push_back (worldPrim);
        bvector<CurveLocationDetail> detailA, detailB;
        CurveCurve::IntersectRotatedCurveSpaceCurve (worldToLocal, *worldVector, curve, detailA, detailB);

        for (size_t i = 0; i < detailA.size (); i++)
            {
            double fB = detailB[i].fraction;
            // (?? no option for z bounding. remove the arg or reimplement intersection step)
            DPoint3d uvw;
            worldToLocal.Multiply (uvw, detailB[i].point);  // the "curve" point is in uvw space !!!
            curveFractions.push_back (fB);
            normalizedConePoints.push_back (uvw);
            }
        }
    }



GEOMDLLIMPEXP ISolidPrimitivePtr ISolidPrimitive::CreateDgnTorusPipe (DgnTorusPipeDetailCR data)
    {
    ISolidPrimitivePtr newPrimitive =  new DgnTorusPipe (data);
    return newPrimitive;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TryGetDgnTorusPipeDetail (DgnTorusPipeDetailR data) const { return false;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TrySetDgnTorusPipeDetail (DgnTorusPipeDetailCR data) { return false;}


GEOMDLLIMPEXP bool ISolidPrimitive::TryGetDgnTorusPipeDetail (DgnTorusPipeDetailR data ) const { return _TryGetDgnTorusPipeDetail (data);}
GEOMDLLIMPEXP bool ISolidPrimitive::TrySetDgnTorusPipeDetail (DgnTorusPipeDetailCR data)       { return _TrySetDgnTorusPipeDetail (data);}


// ****************************************************************
//                           DgnCone
// ****************************************************************




/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct DgnCone : public ISolidPrimitive
{
friend struct ISolidPrimitive;
private:
DgnConeDetail m_data;

protected:


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TryGetDgnConeDetail(DgnConeDetailR data) const override
    {
    data = m_data;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TrySetDgnConeDetail(DgnConeDetailCR data) override
    {
    m_data = data;
    return true;
    }

EXPAND_SOLID_PRIMITIVE_VIRTUALS(DgnCone)


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
SolidPrimitiveType _GetSolidPrimitiveType() const override {return SolidPrimitiveType_DgnCone;}

public:

  DgnCone (DgnConeDetailCR data)
    : m_data (data)
    {}

};

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DgnConeDetail::DgnConeDetail
(
DPoint3dCR  centerA,
DPoint3dCR  centerB,
double      radiusA,
double      radiusB,
bool        capped
)
    {
    DVec3d      axis = DVec3d::FromStartEnd (centerA, centerB);
    if (centerA.AlmostEqual(centerB))
        axis = DVec3d::UnitZ ();
    RotMatrix   rMatrix = RotMatrix::From1Vector (axis, 2, true);

    rMatrix.GetColumn (m_vector0, 0);
    rMatrix.GetColumn (m_vector90, 1);

    m_centerA = centerA;
    m_centerB = centerB;
    m_radiusA = radiusA;
    m_radiusB = radiusB;
    m_capped = capped;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DgnConeDetail::DgnConeDetail
(
DPoint3dCR  centerA,
DPoint3dCR  centerB,
DVec3dCR    vector0,
DVec3dCR    vector90,
double      radiusA,
double      radiusB,
bool        capped
)
    {
    m_centerA = centerA;
    m_centerB = centerB;
    m_vector0 = vector0;
    m_vector90 = vector90;
    m_radiusA = radiusA;
    m_radiusB = radiusB;
    m_capped = capped;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
GEOMDLLIMPEXP DgnConeDetail::DgnConeDetail
(
DPoint3dCR  centerA,
DPoint3dCR  centerB,
RotMatrixCR axes,
double      radiusA,
double      radiusB,
bool        capped
)
    {
    m_centerA = centerA;
    m_centerB = centerB;
    DVec3d vector0, vector90, vectorZ;
    axes.GetColumns (vector0, vector90, vectorZ);
    m_vector0 = vector0;
    m_vector90 = vector90;
    m_radiusA = radiusA;
    m_radiusB = radiusB;
    m_capped = capped;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DgnConeDetail::DgnConeDetail ()
    {
    m_centerA = DPoint3d::From (0,0,0);
    m_centerB = DPoint3d::From (0,0,1);
    m_vector0 = DVec3d::From (1,0,0);
    m_vector90 = DVec3d::From (0,1,0);
    m_radiusA = 1;
    m_radiusB = 1;
    m_capped = false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::GetTransforms
(
TransformR localToWorld,
TransformR worldToLocal,
double     &radius0,
double     &radius1,
bool       fractionalRadii
) const
    {
    DVec3d vectorZ = DVec3d::FromStartEnd (m_centerA, m_centerB);
    DVec3d vectorX = m_vector0;
    DVec3d vectorY = m_vector90;
    if (fractionalRadii)
        {
        double r = DoubleOps::Max (m_radiusA, m_radiusB);
        DoubleOps::SafeDivide (radius0, m_radiusA, r, 0.0);
        DoubleOps::SafeDivide (radius1, m_radiusB, r, 0.0);
        vectorX.Scale (r);
        vectorY.Scale (r);
        }
    else
        {
        radius0 = m_radiusA;
        radius1 = m_radiusB;
        }
    localToWorld = Transform::FromOriginAndVectors (m_centerA, vectorX, vectorY, vectorZ);
    return worldToLocal.InverseOf (localToWorld);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::IsCircular
(
DPoint3dR centerA,
DPoint3dR centerB,
RotMatrixR rotMatrix,
double &radiusA,
double &radiusB,
bool &capped
) const
    {
    centerA = m_centerA;
    centerB = m_centerB;
    radiusA = m_radiusA * m_vector0.Magnitude ();
    radiusB = m_radiusB * m_vector0.Magnitude ();
    DVec3d vector0 = m_vector0;
    DVec3d vector90 = m_vector90;
    vector0.Normalize ();
    vector90.Normalize ();
    capped = m_capped;
    DVec3d vectorZ = DVec3d::FromNormalizedCrossProduct (vector0, vector90);
    rotMatrix = RotMatrix::FromColumnVectors (vector0, vector90, vectorZ);
    return vector0.IsPerpendicularTo (vector90);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::IsCylinder
(
DPoint3dR centerA,
DPoint3dR centerB,
double &radius,
bool &capped
) const
    {
    centerA = m_centerA;
    centerB = m_centerB;
    double radiusA = m_radiusA * m_vector0.Magnitude ();
    double radiusB = m_radiusB * m_vector0.Magnitude ();
    radius = radiusA;
    DVec3d vector0 = m_vector0;
    DVec3d vector90 = m_vector90;
    DVec3d vectorAB = DVec3d::FromStartEnd (centerA, centerB);
    vector0.Normalize ();
    vector90.Normalize ();
    capped = m_capped;
    return vector0.IsPerpendicularTo (vector90)
        && vector0.IsPerpendicularTo (vectorAB)
        && vector90.IsPerpendicularTo (vectorAB)
        && DoubleOps::AlmostEqual (radiusA, radiusB);
    }
GEOMDLLIMPEXP ISolidPrimitivePtr ISolidPrimitive::CreateDgnCone (DgnConeDetailCR data)
    {
    ISolidPrimitivePtr newPrimitive =  new DgnCone (data);
    return newPrimitive;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TryGetDgnConeDetail (DgnConeDetailR data) const { return false;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TrySetDgnConeDetail (DgnConeDetailCR data) { return false;}


GEOMDLLIMPEXP bool ISolidPrimitive::TryGetDgnConeDetail (DgnConeDetailR data ) const { return _TryGetDgnConeDetail (data);}
GEOMDLLIMPEXP bool ISolidPrimitive::TrySetDgnConeDetail (DgnConeDetailCR data)       { return _TrySetDgnConeDetail (data);}

GEOMDLLIMPEXP bool DgnConeDetail::FractionToSection (double fraction, DEllipse3dR ellipse) const
    {
    ellipse.center.Interpolate (m_centerA, fraction, m_centerB);
    double r = DoubleOps::Interpolate (m_radiusA, fraction, m_radiusB);
    ellipse.vector0.Scale (m_vector0, r);
    ellipse.vector90.Scale (m_vector90, r);
    ellipse.start = 0.0;
    ellipse.sweep = Angle::TwoPi ();
    return !DoubleOps::AlmostEqual (r, 0.0);
    }

GEOMDLLIMPEXP bool DgnConeDetail::FractionToRule (double fraction, DSegment3dR segment) const
    {
    double theta = fraction * Angle::TwoPi ();
    double c = cos (theta);
    double s = sin (theta);
    segment.point[0] = DPoint3d::FromSumOf (m_centerA,
                m_vector0, c * m_radiusA,
                m_vector90, s * m_radiusA
                );
    segment.point[1] = DPoint3d::FromSumOf (m_centerB,
                m_vector0, c * m_radiusB,
                m_vector90, s * m_radiusB
                );
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool DgnConeDetail::GetSilhouettes (DSegment3dR segmentA, DSegment3dR segmentB, DMatrix4dCR viewToLocal) const
    {
    double      lambda;
    DVec3d      xAxis, yAxis, zAxis;
    DPoint3d    origin;
    Transform   frame;
    DCone3d     cone;

    // Pick larger radius end as xy plane of the coordinate frame...
    if (m_radiusA < m_radiusB)
        {
        origin = m_centerB;
        zAxis.DifferenceOf (m_centerA, m_centerB);
        xAxis.Scale (m_vector0, m_radiusB);
        yAxis.Scale (m_vector90, m_radiusB);
        lambda = m_radiusA / m_radiusB;
        }
    else
        {
        origin = m_centerA;
        zAxis.DifferenceOf (m_centerB, m_centerA);
        xAxis.Scale (m_vector0, m_radiusA);
        yAxis.Scale (m_vector90, m_radiusA);
        lambda = m_radiusB / m_radiusA;
        }

    frame.InitFromOriginAndVectors (origin, xAxis, yAxis, zAxis);
    bsiDCone3d_setFrameAndFraction (&cone, &frame, lambda, NULL);

    DPoint4d    eyePoint;
    DPoint3d    trigPointBuffer[2];

    viewToLocal.GetColumn (eyePoint, 2);

    if (2 != bsiDCone3d_silhouetteAngles (&cone, trigPointBuffer, NULL, &eyePoint))
        return false;

    bsiDCone3d_getRuleLine (&cone, &segmentA, trigPointBuffer[0].z);
    bsiDCone3d_getRuleLine (&cone, &segmentB, trigPointBuffer[1].z);

    return true;
    }

struct BezierProductIndexDetail
{
double m_coefficient;
int    m_i, m_j;
BezierProductIndexDetail (double coefficient, int i, int j) : m_coefficient (coefficient), m_i(i), m_j(j){}
};

static int solveBezierConeQuartic
(
DPoint4d *arcPole,           //! 3 poles of quadratic bezier for bezier.
DPoint3d *trigPole,         //! 3 poles of trig form
double rA,                  //! radius at z=0
double rB,                  //! radius at z=1
double *angles        //! Array (size at least 5) to receive solution angles.
)
    {
    int numRoot = 0;
    int productOrder = 5;   // quadratic*quadratic = quartic, 5 poles
    double univariatePole[10];
    for(int i = 0; i < productOrder; i++)
        univariatePole[i] = 0.0;
    // X^2 + Y^2 - rA2 * W^2 - 2 rA (rB-rA) W Z - (rA-rB)^2 Z^2
    double dr = rB - rA;

    BezierProductIndexDetail detail [5] =
        {
        BezierProductIndexDetail (1.0, 0, 0),        // X^2
        BezierProductIndexDetail (1.0, 1, 1),        // Y^2
        BezierProductIndexDetail (- rA * rA, 3, 3),        // W^2
        BezierProductIndexDetail (-2.0 * rA * dr , 2, 3),        // ZW
        BezierProductIndexDetail (- dr * dr, 2, 2),        // Z^2
        };

    for (int k = 0; k < 5; k++)
        bsiBezier_accumulateUnivariateProduct (
            univariatePole, 0, 1,
            detail[k].m_coefficient,
            (double*)arcPole, 3, detail[k].m_i, 4,
            (double*)arcPole, 3, detail[k].m_j, 4
            );
    double bezierRoot[10];
    bsiBezier_univariateRoots (bezierRoot, &numRoot, univariatePole, 5);
    for (int i = 0; i < numRoot; i++)
        {
        double f = bezierRoot[i];
        angles[i] = bsiTrig_quadricTrigPointBezierFractionToAngle (trigPole, f);
        }
    return numRoot;
    }


/*---------------------------------------------------------------------------------**//**
* (Note this is a static method -- caller provides cone-specific data)
* @bsimethod                                                    Earlin.Lutz 11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnConeDetail::SetDetailCoordinatesFromFractionalizedConeCoordinates
(
SolidLocationDetail &detail,
DPoint3dCR  localuvw,       //!< coordinates in local fraction space.
TransformCR localToWorld,   //! transform to world coordinates
double      r0,             //!< [in] cone radius at w=0
double      r1             //!< [in] cone radius at w=1
)
    {
    DVec3d uDirection, vDirection;
    double theta = atan2 (localuvw.y, localuvw.x);
    double uFraction = Angle::NormalizeToSweep (theta, 0.0, Angle::TwoPi ());
    double vFraction = localuvw.z;
    double c = cos (theta);
    double s = sin (theta);
    DVec3d uDirectionLocal = DVec3d::From (-localuvw.y, localuvw.x, 0.0);
    uDirectionLocal.Scale (Angle::TwoPi ());
    double dr = r1 - r0;
    DVec3d vDirectionLocal = DVec3d::From (dr * c, dr * s, 1.0);
    localToWorld.MultiplyMatrixOnly (uDirection, uDirectionLocal);
    localToWorld.MultiplyMatrixOnly (vDirection, vDirectionLocal);
    detail.SetUV (uFraction, vFraction, uDirection, vDirection);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 11/15
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP void DgnConeDetail::IntersectBoundedArc
(
DEllipse3dCR arc,
bvector<double> &arcFractions,
bvector<DPoint3d> &normalizedConePoints,
TransformR localToWorld,
TransformR worldToLocal,
double &radius0,
double &radius1,
bool boundedConeZ
) const
    {
    arcFractions.clear ();
    normalizedConePoints.clear ();
    if (GetTransforms (localToWorld, worldToLocal, radius0, radius1, true))
        {
        DEllipse3d localArc;
        worldToLocal.Multiply (localArc, arc);
        // local condition for being on the cone is x^2 + y^2 = ((1-z)rA + z rB))^2
        //              x^2 + y^2 = ((1-z)^2 rA^2 + 2 z (1-z) rA*rB + z^2 rB^2
        //              x^2 + y^2 = rA2 - 2z rA2 + z^2 (rA2 + rB2) + 2z rA rB - 2 z^2 rA rB 
        //              x^2 + y^2 = rA2 + 2z rA (rB - rA) + z^2 (rA2 - 2 rA rB + rB2)
        // CONFUSING POINT HERE: xyz are in the local space of the normalized cone.
        //    Points on the ellipse are homogeneous, with a weight . ..
        // arc coordinates for uvw are quadratic function of arc fraction ...
        DPoint4d qPoles[8];     // poles in cone local space
        DPoint3d stPoles[8];    // poles in plane of arc
        int numSpan, numPoles;
        localArc.QuadricBezierPoles (qPoles, stPoles, &numPoles, &numSpan, 5);
        for (int spanIndex = 0, pole0Index = 0; spanIndex < numSpan; spanIndex++, pole0Index += 2)
            {
            double angles[10];
            int numRoot = solveBezierConeQuartic (qPoles + pole0Index, stPoles + pole0Index, radius0, radius1, angles);
            for (int i = 0; i < numRoot; i++)
                {
                double f = localArc.AngleToFraction (angles[i]);
                DPoint3d uvw = localArc.FractionToPoint (f);
                if (!boundedConeZ || DoubleOps::IsIn01 (uvw.z))
                    {
                    arcFractions.push_back (f);
                    normalizedConePoints.push_back (uvw);
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 11/15
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP void DgnConeDetail::IntersectCurveLocal
(
ICurvePrimitiveCR curve,
bvector<double> &curveFractions,
bvector<DPoint3d> &normalizedConePoints,
TransformR localToWorld,
TransformR worldToLocal,
double &radius0,
double &radius1,
bool boundedConeZ
) const
    {
    curveFractions.clear ();
    normalizedConePoints.clear ();
    if (GetTransforms (localToWorld, worldToLocal, radius0, radius1, true))
        {
        DSegment3d localSegment = DSegment3d::From (radius0, 0.0, 0.0, radius1, 0.0, 1.0);
        DSegment3d worldSegment;
        localToWorld.Multiply (worldSegment, localSegment);
        auto worldLine = ICurvePrimitive::CreateLine (worldSegment);
        auto worldLineVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        worldLineVector->push_back (worldLine);
        bvector<CurveLocationDetail> detailA, detailB;
        CurveCurve::IntersectRotatedCurveSpaceCurve (worldToLocal, *worldLineVector, curve, detailA, detailB);

        for (size_t i = 0; i < detailA.size (); i++)
            {
            double fB = detailB[i].fraction;
            // (?? no option for z bounding. remove the arg or reimplement intersection step)
            DPoint3d uvw;
            worldToLocal.Multiply (uvw, detailB[i].point);  // the "curve" point is in uvw space !!!
            curveFractions.push_back (fB);
            normalizedConePoints.push_back (uvw);
            }
        }
    }



// ****************************************************************
//                           DgnBox
// ****************************************************************




/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct DgnBox : public ISolidPrimitive
{
friend struct ISolidPrimitive;
private:
DgnBoxDetail m_data;

protected:


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TryGetDgnBoxDetail(DgnBoxDetailR data) const override
    {
    data = m_data;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TrySetDgnBoxDetail(DgnBoxDetailCR data) override
    {
    m_data = data;
    return true;
    }

EXPAND_SOLID_PRIMITIVE_VIRTUALS(DgnBox)


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
SolidPrimitiveType _GetSolidPrimitiveType() const override {return SolidPrimitiveType_DgnBox;}

public:

  DgnBox (DgnBoxDetailCR data)
    : m_data (data)
    {}

};

// Detail constructor with complete field list as parameters ...
DgnBoxDetail::DgnBoxDetail
(
DPoint3dCR  baseOrigin,
DPoint3dCR  topOrigin,
DVec3dCR  vectorX,
DVec3dCR  vectorY,
double  baseX,
double  baseY,
double  topX,
double  topY,
bool capped
)
    {
    m_baseOrigin = baseOrigin;
    m_topOrigin = topOrigin;
    m_vectorX = vectorX;
    m_vectorY = vectorY;
    m_baseX = baseX;
    m_baseY = baseY;
    m_topX = topX;
    m_topY = topY;
    m_capped = capped;
    }

// Detail constructor ...
DgnBoxDetail::DgnBoxDetail ()
    {
    m_baseOrigin = DPoint3d::From (0,0,0);
    m_topOrigin = DPoint3d::From (0,0,1);
    m_vectorX = DVec3d::From (1,0,0);
    m_vectorY = DVec3d::From (0,1,0);
    m_baseX = 1;
    m_baseY = 1;
    m_topX = 1;
    m_topX = 1;
    m_capped = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
DgnBoxDetail DgnBoxDetail::InitFromCenterAndSize (DPoint3dCR center, DPoint3dCR size, bool capped)
    {
    double halfHeight = size.z / 2.0;
    return DgnBoxDetail::InitFromCenters (DPoint3d::From (center.x, center.y, center.z - halfHeight),
                                          DPoint3d::From (center.x, center.y, center.z + halfHeight),
                                          DVec3d::From (1.0, 0.0), DVec3d::From (0.0, 1.0), size.x, size.y,
                                          size.x, size.y, capped);
    }

// Initialize detail fields specifying top/base centers instead of origins...
DgnBoxDetail DgnBoxDetail::InitFromCenters (DPoint3dCR baseCenter, DPoint3dCR topCenter, DVec3dCR vectorX, DVec3dCR vectorY, double baseX, double baseY, double topX, double topY, bool capped)
    {
    DPoint3d  baseOrigin, topOrigin;

    baseOrigin = DPoint3d::FromSumOf (baseCenter, vectorX, baseX * -0.5);
    baseOrigin = DPoint3d::FromSumOf (baseOrigin, vectorY, baseY * -0.5);
          
    topOrigin = DPoint3d::FromSumOf (topCenter, vectorX, topX * -0.5);
    topOrigin = DPoint3d::FromSumOf (topOrigin, vectorY, topY * -0.5);

    DgnBoxDetail  detail (baseOrigin, topOrigin, vectorX, vectorY, baseX, baseY, topX, topY, capped);

    return detail;
    }

GEOMDLLIMPEXP ISolidPrimitivePtr ISolidPrimitive::CreateDgnBox (DgnBoxDetailCR data)
    {
    ISolidPrimitivePtr newPrimitive =  new DgnBox (data);
    return newPrimitive;
    }

GEOMDLLIMPEXP void DgnBoxDetail::GetCorners (bvector<DPoint3d> &corners) const
    {
    corners.clear ();
    DPoint3d xPoint0 = DPoint3d::FromSumOf (m_baseOrigin, m_vectorX, m_baseX);
    DPoint3d xPoint1 = DPoint3d::FromSumOf (m_topOrigin, m_vectorX, m_topX);

    corners.push_back (m_baseOrigin);
    corners.push_back (xPoint0);

    corners.push_back (DPoint3d::FromSumOf (m_baseOrigin, m_vectorY, m_baseY));
    corners.push_back (DPoint3d::FromSumOf (xPoint0, m_vectorY, m_baseY));

    corners.push_back (m_topOrigin);
    corners.push_back (xPoint1);
    corners.push_back (DPoint3d::FromSumOf (m_topOrigin, m_vectorY, m_topY));
    corners.push_back (DPoint3d::FromSumOf (xPoint1, m_vectorY, m_topY));
    }

GEOMDLLIMPEXP void DgnBoxDetail::GetCorners (DPoint3dP corners) const
    {
    int count = 0;
    DPoint3d xPoint0 = DPoint3d::FromSumOf (m_baseOrigin, m_vectorX, m_baseX);
    DPoint3d xPoint1 = DPoint3d::FromSumOf (m_topOrigin, m_vectorX, m_topX);

    corners[count++] = m_baseOrigin;
    corners[count++] = xPoint0;

    corners[count++] = DPoint3d::FromSumOf (m_baseOrigin, m_vectorY, m_baseY);
    corners[count++] = DPoint3d::FromSumOf (xPoint0, m_vectorY, m_baseY);

    corners[count++] = m_topOrigin;
    corners[count++] = xPoint1;
    corners[count++] = DPoint3d::FromSumOf (m_topOrigin, m_vectorY, m_topY);
    corners[count++] = DPoint3d::FromSumOf (xPoint1, m_vectorY, m_topY);
    }


GEOMDLLIMPEXP bool DgnBoxDetail::IsBlock (DPoint3dR origin, RotMatrixR unitAxes, DVec3dR localDiagonal, double originXFraction, double originYFraction, double originZFraction) const
    {
    BoxFaces box;
    box.Load (*this);
    bool perp = box.AreAllEdgesPerpendicular ();
    Transform baseFrame = box.GetBaseTransform ();
    origin = DPoint3d::FromProduct (baseFrame, originXFraction, originYFraction, originZFraction);
    DVec3d xEdge, yEdge, zEdge;
    DPoint3d corner0;
    baseFrame.GetOriginAndVectors (corner0, xEdge, yEdge, zEdge);
    localDiagonal = DVec3d::From (xEdge.Magnitude (), yEdge.Magnitude (), zEdge.Magnitude ());
    RotMatrix skewAxes = RotMatrix::FromColumnVectors (xEdge, yEdge, zEdge);
    bool squaredUp = unitAxes.SquareAndNormalizeColumns (skewAxes, 0, 1);
    return perp & squaredUp;
    }


//! Return (nonuniform) transforms and rectangle sizes.   (ax,ay) rectangle is on z=0.  (bx,by) is on z=1;
void DgnBoxDetail::GetNonUniformTransform
(
TransformR localToWorld,
double &ax,
double &ay,
double &bx,
double &by
) const
    {
    localToWorld.InitFromOriginAndVectors (m_baseOrigin, m_vectorX, m_vectorY, 
                DVec3d::FromStartEnd (m_baseOrigin, m_topOrigin));
    ax = m_baseX;
    ay = m_baseY;
    bx = m_topX;
    by = m_topY;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TryGetDgnBoxDetail (DgnBoxDetailR data) const { return false;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TrySetDgnBoxDetail (DgnBoxDetailCR data) { return false;}


GEOMDLLIMPEXP bool ISolidPrimitive::TryGetDgnBoxDetail (DgnBoxDetailR data ) const { return _TryGetDgnBoxDetail (data);}
GEOMDLLIMPEXP bool ISolidPrimitive::TrySetDgnBoxDetail (DgnBoxDetailCR data)       { return _TrySetDgnBoxDetail (data);}


// ****************************************************************
//                           DgnSphere
// ****************************************************************



/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct DgnSphere : public ISolidPrimitive
{
friend struct ISolidPrimitive;
private:
DgnSphereDetail m_data;

protected:


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TryGetDgnSphereDetail(DgnSphereDetailR data) const override
    {
    data = m_data;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TrySetDgnSphereDetail(DgnSphereDetailCR data) override
    {
    m_data = data;
    return true;
    }

EXPAND_SOLID_PRIMITIVE_VIRTUALS(DgnSphere)



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
SolidPrimitiveType _GetSolidPrimitiveType() const override {return SolidPrimitiveType_DgnSphere;}

public:

  DgnSphere (DgnSphereDetailCR data)
    : m_data (data)
    {}

};

// Detail consructor with complete field list as parameters ...
DgnSphereDetail::DgnSphereDetail
(
DPoint3dCR  center,
DVec3dCR  vectorX,
DVec3dCR  vectorZ,
double  radiusXY,
double  radiusZ,
double  startLatitude,
double  latitudeSweep,
bool     capped)
    {
    DVec3d columnX = vectorX;
    DVec3d columnZ = vectorZ;
    DVec3d columnY;
    columnX.Scale (radiusXY);
    columnZ.Scale (radiusZ);
    columnY.SizedCrossProduct (columnZ, columnX, radiusXY);
    m_localToWorld.InitFromOriginAndVectors (center, columnX, columnY, columnZ); 
    m_startLatitude = startLatitude;
    m_latitudeSweep = latitudeSweep;
    m_capped = capped;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DgnSphereDetail::DgnSphereDetail (
    DPoint3dCR center,
    RotMatrixCR axes,
    double radius
    )
    {
    DVec3d vectorX, vectorY, vectorZ;
    axes.GetColumns (vectorX, vectorY, vectorZ);
    vectorX.Scale (radius);
    vectorY.Scale (radius);
    vectorZ.Scale (radius);
    m_localToWorld.InitFromOriginAndVectors (center, vectorX, vectorY, vectorZ);     
    m_startLatitude = -Angle::PiOver2 ();
    m_latitudeSweep = Angle::Pi ();
    m_capped = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
DgnSphereDetail::DgnSphereDetail (DPoint3dCR center, double radius)
    {
    DVec3d  x = DVec3d::From (radius * 1.0, 0.0),
            y = DVec3d::From (0.0, radius * 1.0),
            z = DVec3d::From (0.0, 0.0, radius * 1.0);

    m_localToWorld.InitFromOriginAndVectors (center, x, y, z);
    m_startLatitude = -Angle::PiOver2 ();
    m_latitudeSweep = Angle::Pi ();
    m_capped = false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DgnSphereDetail::DgnSphereDetail()
    {
    m_localToWorld.InitIdentity ();
    m_startLatitude = -Angle::PiOver2 ();
    m_latitudeSweep = Angle::Pi ();
    m_capped = false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::IsTrueSphere
(
DPoint3dR center,
RotMatrixR rotMatrix,
double &radius
) const
    {
    radius = 1.0;
    m_localToWorld.GetTranslation (center);
    m_localToWorld.GetMatrix (rotMatrix);
    double lat0, lat1, z0, z1;
    return !GetSweepLimits (lat0, lat1, z0, z1, true)
        && rotMatrix.IsRigidScale (rotMatrix, radius);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::IsTrueRotationAroundZ
(
DPoint3dR center,
DVec3dR unitX,
DVec3dR unitY,
DVec3dR unitZ,
double     &equatorRadius,
double     &poleRadius
) const
    {
    m_localToWorld.GetOriginAndVectors (center, unitX, unitY, unitZ);   // But they are not unit yet.
    equatorRadius = unitX.Normalize ();
    double rY = unitY.Normalize ();
    poleRadius = unitZ.Normalize ();
    return
           unitX.IsPerpendicularTo (unitY)
        && unitY.IsPerpendicularTo (unitZ)
        && unitX.IsPerpendicularTo (unitZ)
        && DoubleOps::AlmostEqual (rY, equatorRadius);    
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::TryGetRotationAxis (DPoint3dR center, DVec3dR axis, double &sweepRadians) const
    {
    DVec3d vectorX, vectorY;
    m_localToWorld.GetOriginAndVectors (center, vectorX, vectorY, axis);
    sweepRadians = Angle::TwoPi ();
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::GetTransforms (TransformR localToWorld, TransformR worldToLocal) const
    {
    QVNormalizedTransforms (m_localToWorld, localToWorld, worldToLocal);
    return worldToLocal.InverseOf (m_localToWorld);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::GetNonUniformTransforms (TransformR localToWorld, TransformR worldToLocal) const
    {
    localToWorld = m_localToWorld;
    return worldToLocal.InverseOf (m_localToWorld);
    }

GEOMDLLIMPEXP ISolidPrimitivePtr ISolidPrimitive::CreateDgnSphere (DgnSphereDetailCR data)
    {
    ISolidPrimitivePtr newPrimitive =  new DgnSphere (data);
    return newPrimitive;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TryGetDgnSphereDetail (DgnSphereDetailR data) const { return false;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TrySetDgnSphereDetail (DgnSphereDetailCR data) { return false;}


GEOMDLLIMPEXP bool ISolidPrimitive::TryGetDgnSphereDetail (DgnSphereDetailR data ) const { return _TryGetDgnSphereDetail (data);}
GEOMDLLIMPEXP bool ISolidPrimitive::TrySetDgnSphereDetail (DgnSphereDetailCR data)       { return _TrySetDgnSphereDetail (data);}


static int solveBezierSphereQuartic
(
DPoint4d *arcPole,           //! 3 poles of quadratic bezier for bezier.
DPoint3d *trigPole,
double *angle               //! Array (size at least 5) to receive angles.
)
    {
    int numRoot = 0;
    int productOrder = 5;   // quadratic*quadratic = quartic, 5 poles
    double univariatePole[10];
    for(int i = 0; i < productOrder; i++)
        univariatePole[i] = 0.0;
    // X^2 + Y^2 + Z^2 - W^2 = 0

    double diagonalCoefficient[4] = {1,1,1,-1};
    for (int k = 0; k < 4; k++)
        bsiBezier_accumulateUnivariateProduct (
            univariatePole, 0, 1,
            diagonalCoefficient[k],
            (double*)arcPole, 3, k, 4,
            (double*)arcPole, 3, k, 4
            );
    double bezierRoot[10];
    bsiBezier_univariateRoots (bezierRoot, &numRoot, univariatePole, 5);
    for (int i = 0; i < numRoot; i++)
        angle[i] = bsiTrig_quadricTrigPointBezierFractionToAngle (trigPole, bezierRoot[i]);
    return numRoot;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 11/15
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP void DgnSphereDetail::IntersectBoundedArc
(
DEllipse3dCR arc,
bvector<double> &arcFractions,
bvector<DPoint3d> &normalizedConePoints,
TransformR localToWorld,
TransformR worldToLocal,
bool boundedConeZ
) const
    {
    arcFractions.clear ();
    normalizedConePoints.clear ();
    if (GetTransforms (localToWorld, worldToLocal))
        {
        DEllipse3d localArc;
        worldToLocal.Multiply (localArc, arc);
        // local condition for being on the sphere is x^2 + y^2 + z^2= 1
        // CONFUSING POINT HERE: xyz are in the local space of the normalized cone.
        //    Points on the ellipse are homogeneous, with a weight . ..
        // arc coordinates for uvw are quadratic function of arc fraction ...
        DPoint4d qPoles[8];     // poles in cone local space
        DPoint3d stPoles[8];    // poles in plane of arc
        int numSpan, numPoles;
        localArc.QuadricBezierPoles (qPoles, stPoles, &numPoles, &numSpan, 5);
        for (int spanIndex = 0, pole0Index = 0; spanIndex < numSpan; spanIndex++, pole0Index += 2)
            {
            double angleRoot [10];
            int numRoot = solveBezierSphereQuartic (qPoles + pole0Index, stPoles + pole0Index, angleRoot);
            for (int i = 0; i < numRoot; i++)
                {

                double f = localArc.AngleToFraction (angleRoot[i]);
                DPoint3d uvw = localArc.FractionToPoint (f);
                DPoint3d thetaPhiR;
                bsiVector_cartesianToSpherical (&thetaPhiR, &uvw);

                if (!boundedConeZ || DoubleOps::IsIn01 (LatitudeToVFraction (uvw.y)))
                    {
                    arcFractions.push_back (f);
                    normalizedConePoints.push_back (uvw);
                    }
                }
            }
        }
    }


// ****************************************************************
//                           DgnExtrusion
// ****************************************************************




/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct DgnExtrusion : public ISolidPrimitive
{
friend struct ISolidPrimitive;
private:
DgnExtrusionDetail m_data;

protected:


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TryGetDgnExtrusionDetail(DgnExtrusionDetailR data) const override
    {
    data = m_data;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TrySetDgnExtrusionDetail(DgnExtrusionDetailCR data) override
    {
    m_data = data;
    return true;
    }

EXPAND_SOLID_PRIMITIVE_VIRTUALS(DgnExtrusion)



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
SolidPrimitiveType _GetSolidPrimitiveType() const override {return SolidPrimitiveType_DgnExtrusion;}

public:

  DgnExtrusion (DgnExtrusionDetailCR data)
    : m_data (data)
    {}
};

// Detail consructor with complete field list as parameters ...
DgnExtrusionDetail::DgnExtrusionDetail
(
CurveVectorPtr  const &baseCurve,
DVec3dCR  extrusionVector,
bool  capped)
    {
    m_baseCurve = baseCurve;
    m_extrusionVector = extrusionVector;
    m_capped = capped;
    }

// Detail consructor with complete field list as parameters ...
DgnExtrusionDetail::DgnExtrusionDetail ()
    {
    m_baseCurve = NULL;
    m_extrusionVector = DVec3d::From (0,0,1);
    m_capped = false;
    }

GEOMDLLIMPEXP CurveVectorPtr DgnExtrusionDetail::FractionToProfile (double fraction) const
    {
    CurveVectorPtr profile = m_baseCurve->Clone ();
    if (fraction != 0.0)
        {
        DVec3d fractionalSweep = DVec3d::FromScale (m_extrusionVector, fraction);
        BentleyApi::Transform transform = BentleyApi::Transform::From (fractionalSweep);
        profile->TransformInPlace (transform);
        }
    return profile;
    }
GEOMDLLIMPEXP ISolidPrimitivePtr ISolidPrimitive::CreateDgnExtrusion (DgnExtrusionDetailCR data)
    {
    ISolidPrimitivePtr newPrimitive =  new DgnExtrusion (data);
    return newPrimitive;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TryGetDgnExtrusionDetail (DgnExtrusionDetailR data) const { return false;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TrySetDgnExtrusionDetail (DgnExtrusionDetailCR data) { return false;}


GEOMDLLIMPEXP bool ISolidPrimitive::TryGetDgnExtrusionDetail (DgnExtrusionDetailR data ) const { return _TryGetDgnExtrusionDetail (data);}
GEOMDLLIMPEXP bool ISolidPrimitive::TrySetDgnExtrusionDetail (DgnExtrusionDetailCR data)       { return _TrySetDgnExtrusionDetail (data);}


// ****************************************************************
//                           DgnRotationalSweep
// ****************************************************************




/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct DgnRotationalSweep : public ISolidPrimitive
{
friend struct ISolidPrimitive;
private:
DgnRotationalSweepDetail m_data;

protected:


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TryGetDgnRotationalSweepDetail(DgnRotationalSweepDetailR data) const override
    {
    data = m_data;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TrySetDgnRotationalSweepDetail(DgnRotationalSweepDetailCR data) override
    {
    m_data = data;
    return true;
    }

EXPAND_SOLID_PRIMITIVE_VIRTUALS(DgnRotationalSweep)



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
SolidPrimitiveType _GetSolidPrimitiveType() const override {return SolidPrimitiveType_DgnRotationalSweep;}

public:

  DgnRotationalSweep (DgnRotationalSweepDetailCR data)
    : m_data (data)
    {}

};

// Detail consructor with complete field list as parameters ...
DgnRotationalSweepDetail::DgnRotationalSweepDetail
(
CurveVectorPtr  const &baseCurve,
DPoint3dCR  center,
DVec3dCR  axis,
double  sweepAngle,
bool  capped)
    {
    m_baseCurve = baseCurve;
    m_axisOfRotation = DRay3d::FromOriginAndVector (center, axis);
    m_sweepAngle = sweepAngle;
    m_numVRules = 0;
    m_capped = capped;
    }

// Detail consructor with complete field list as parameters ...
DgnRotationalSweepDetail::DgnRotationalSweepDetail ()
    {
    m_baseCurve = NULL;
    m_axisOfRotation = DRay3d::FromOriginAndVector
                (
                DPoint3d::From (0,0,0),
                DVec3d::From (1,0,0)
                );
    m_sweepAngle = Angle::PiOver2 ();
    m_numVRules = 0;
    m_capped = false;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRotationalSweepDetail::SetVRuleCount (size_t numVRules) {m_numVRules = numVRules;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/13
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnRotationalSweepDetail::GetVRuleCount () const
    {
    size_t  numVRules = ComputeVRuleCount (m_sweepAngle);

    // Only use m_numVRules if it's more than the default/minimum number...
    if (numVRules < m_numVRules)
        numVRules = m_numVRules;

    return numVRules;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/13
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnRotationalSweepDetail::ComputeVRuleCount (double sweepRadians, size_t numVRulesFullSweep)
    {
    double  f = numVRulesFullSweep * fabs (sweepRadians) / Angle::TwoPi ();
    size_t     numRules = (size_t) (f + 0.5);

    if (numRules < 1)
        return 1;

    if (numRules > (int) numVRulesFullSweep)
        return numVRulesFullSweep;
        
    return numRules;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr DgnRotationalSweepDetail::VFractionToProfile (double fraction) const
    {
    CurveVectorPtr profile = m_baseCurve->Clone ();
    if (fraction == 0.0)
        return profile;
    Transform rotation, derivative;
    if (fraction != 0.0 && GetVFractionTransform (fraction, rotation, derivative))
        {
        profile->TransformInPlace (rotation);
        return profile;
        }
    return NULL;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::TryGetRotationAxis (DPoint3dR center, DVec3dR axis, double &sweepRadians) const
    {
    axis = m_axisOfRotation.direction;
    sweepRadians = m_sweepAngle;
    center = m_axisOfRotation.origin;
    if (sweepRadians < 0.0)
        {
        sweepRadians = - sweepRadians;
        axis.Negate ();
        }
    return axis.MagnitudeSquared () > 0.0;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2015
+--------------------------------------------------------------------------------------*/
bool   DgnRotationalSweepDetail::GetRadius (double& radius, RadiusType type) const
    {
    double      area, length;
    DVec3d      normal;
    DPoint3d    centroid;

    if (!m_baseCurve->CentroidNormalArea (centroid, normal,  area) &&
        !m_baseCurve->WireCentroid (length, centroid))
        return false;

    DVec3d      vectorXZ = DVec3d::FromStartEnd (m_axisOfRotation.origin, centroid);
    DVec3d      unitY = DVec3d::FromNormalizedCrossProduct (m_axisOfRotation.direction, vectorXZ);
    DVec3d      unitX = DVec3d::FromNormalizedCrossProduct (unitY, m_axisOfRotation.direction);

    if (RadiusType::Centroidal == type)
        {
        radius = unitX.DotProduct (vectorXZ);
        return true;
        }

    DRange1d    range = m_baseCurve->ProjectedParameterRange (DRay3d::FromOriginAndVector (m_axisOfRotation.origin, unitX));

    if (range.IsNull())
        return false;

    radius = (RadiusType::Minimum == type) ? range.low : range.high;

    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2015
+--------------------------------------------------------------------------------------*/
bool   DgnRotationalSweepDetail::SetRadius (double radius, RadiusType type)
    {
    Transform   localToWorld, worldToLocal;

    if (!GetTransforms (localToWorld, worldToLocal))
        return false;

    DRay3d          ray;
    double          translateDistance;

    localToWorld.GetTranslation (ray.origin);
    localToWorld.GetMatrixColumn (ray.direction, 0);

    DRange1d    range = m_baseCurve->ProjectedParameterRange (DRay3d::FromOriginAndVector (m_axisOfRotation.origin, ray.direction));

    if (range.IsNull())
        return false;

    if (RadiusType::Centroidal == type)
        {
        double      area, length;
        DVec3d      normal;
        DPoint3d    centroid;

        if (!m_baseCurve->CentroidNormalArea (centroid, normal,  area) &&
            !m_baseCurve->WireCentroid (length, centroid))
            return false;

        translateDistance = radius - ray.direction.DotProduct (DVec3d::FromStartEnd (m_axisOfRotation.origin, centroid));
        }
    else
        {
        translateDistance =  radius - ((RadiusType::Minimum == type) ? range.low : range.high);
        }

    if (range.low + translateDistance < 0.0)        
        return false;

    return m_baseCurve->TransformInPlace (Transform::From (DVec3d::FromScale (ray.direction, translateDistance)));
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::GetTransforms (TransformR localToWorld, TransformR worldToLocal) const
    {
    bvector<DPoint3d> points;
    auto options = IFacetOptions::CreateForCurves ();
    // OUCH!! EXPENSIVE!! "just" need a curve point clearly not on the axis.
    m_baseCurve->AddStrokePoints (points, *options);
    size_t index;
    double distance;
    if (DPoint3dOps::MaxDistanceFromUnboundedRay (points, m_axisOfRotation, index, distance)
        && !DoubleOps::AlmostEqual (distance, 0.0))
        {
        DPoint3d farPoint = points[index];
        DVec3d vectorXZ = DVec3d::FromStartEnd (m_axisOfRotation.origin, farPoint);
        DVec3d unitZ;
        unitZ.Normalize (m_axisOfRotation.direction);
        DVec3d unitY = DVec3d::FromNormalizedCrossProduct (unitZ, vectorXZ);
        DVec3d unitX = DVec3d::FromNormalizedCrossProduct (unitY, unitZ);
        localToWorld.InitFromOriginAndVectors (m_axisOfRotation.origin, unitX, unitY, unitZ);
        worldToLocal.InvertRigidBodyTransformation (localToWorld);
        return true;
        }
    return QVNormalizedTransforms (m_axisOfRotation.origin, m_axisOfRotation.direction, localToWorld, worldToLocal);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::GetVFractionTransform (double vFraction, TransformR transform, TransformR derivativeTransform) const
    {
    DRay3d ray;
    double radians;
    bool stat = false;
    if (TryGetRotationAxis (ray.origin, ray.direction, radians))
        {
        double theta = radians * vFraction;
        transform = BentleyApi::Transform::FromAxisAndRotationAngle (ray, theta, derivativeTransform);
        derivativeTransform.ScaleCompleteRows (derivativeTransform, radians, radians, radians);
        stat = true;
        }
    return stat;
    }

GEOMDLLIMPEXP ISolidPrimitivePtr ISolidPrimitive::CreateDgnRotationalSweep (DgnRotationalSweepDetailCR data)
    {
    ISolidPrimitivePtr newPrimitive =  new DgnRotationalSweep (data);
    return newPrimitive;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TryGetDgnRotationalSweepDetail (DgnRotationalSweepDetailR data) const { return false;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TrySetDgnRotationalSweepDetail (DgnRotationalSweepDetailCR data) { return false;}


GEOMDLLIMPEXP bool ISolidPrimitive::TryGetDgnRotationalSweepDetail (DgnRotationalSweepDetailR data ) const { return _TryGetDgnRotationalSweepDetail (data);}
GEOMDLLIMPEXP bool ISolidPrimitive::TrySetDgnRotationalSweepDetail (DgnRotationalSweepDetailCR data)       { return _TrySetDgnRotationalSweepDetail (data);}


// ****************************************************************
//                           DgnRuledSweep
// ****************************************************************




/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct DgnRuledSweep : public ISolidPrimitive
{
friend struct ISolidPrimitive;
private:
DgnRuledSweepDetail m_data;

protected:


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TryGetDgnRuledSweepDetail(DgnRuledSweepDetailR data) const override
    {
    data = m_data;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TrySetDgnRuledSweepDetail(DgnRuledSweepDetailCR data) override
    {
    m_data = data;
    return true;
    }

EXPAND_SOLID_PRIMITIVE_VIRTUALS(DgnRuledSweep)



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
SolidPrimitiveType _GetSolidPrimitiveType() const override {return SolidPrimitiveType_DgnRuledSweep;}

public:

  DgnRuledSweep (DgnRuledSweepDetailCR data)
    : m_data (data)
    {}

};

// Detail consructor with complete field list as parameters ...
DgnRuledSweepDetail::DgnRuledSweepDetail
(
bvector<CurveVectorPtr> const &  sectionCurves,
bool  capped)
    {
    m_sectionCurves = sectionCurves;
    m_capped = capped;
    }

// Detail consructor with complete field list as parameters ...
void DgnRuledSweepDetail::AddSection (CurveVectorP section)
    {
    m_sectionCurves.push_back (CurveVectorPtr (section));
    }


// Detail consructor with complete field list as parameters ...
DgnRuledSweepDetail::DgnRuledSweepDetail
(
CurveVectorPtr const & sectionA,
CurveVectorPtr const & sectionB,
bool  capped)
    {
    m_sectionCurves.push_back (sectionA);
    m_sectionCurves.push_back (sectionB);
    m_capped = capped;
    }


// Detail consructor with complete field list as parameters ...
DgnRuledSweepDetail::DgnRuledSweepDetail ()
    {
    m_sectionCurves = bvector<CurveVectorPtr>();
    m_capped = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+--------------------------------------------------------------------------------------*/
static bool getTranslationTo (DVec3dR translation, bool haveTranslation, CurveVectorCR curves0, CurveVectorCR curves1, double tolerance)
    {
    if (curves0.size () != curves1.size ())
        return false;

    if (curves0.GetBoundaryType () != curves1.GetBoundaryType ())
        return false;

    DPoint3d    point0, point1;

    if (!haveTranslation)
        {
        if (!curves0.GetStartPoint (point0) || !curves1.GetStartPoint (point1)) // Recurse and evaluate first primitive...
            return false;

        translation.DifferenceOf (point1, point0);
        }

    if (curves0.IsUnionRegion () || curves0.IsParityRegion ())
        {
        for (size_t iCurve = 0; iCurve < curves0.size (); ++iCurve)
            {
            ICurvePrimitivePtr  curve0 = curves0.at (iCurve);
            ICurvePrimitivePtr  curve1 = curves1.at (iCurve);

            if (curve0.IsNull () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve0->GetCurvePrimitiveType () ||
                curve1.IsNull () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve1->GetCurvePrimitiveType ())
                return false;

            if (!getTranslationTo (translation, true, *curve0->GetChildCurveVectorCP (), *curve1->GetChildCurveVectorCP (), tolerance))
                return false;
            }

        return true;
        }

    for (size_t iCurve = 0; iCurve < curves0.size (); ++iCurve)
        {
        bvector<double> testFractions; // Test multiple locations to minimize erroneous classification of ruled sweeps with scale/twist as simple extrusions...

        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == curves0.at (iCurve)->GetCurvePrimitiveType () && curves0.at (iCurve)->GetLineStringCP ()->size () > 2)
            {
            // For linestrings it's better to test the middle of a few segments instead of using hard-coded fractions...
            size_t  nSegments = curves0.at (iCurve)->GetLineStringCP ()->size ()-1, nCheckSegments = (nSegments < 100 ? nSegments : 100);
            double  stepFraction = 1.0 / (nSegments * 2.0), fraction = 0.0;

            for (size_t iSegment = 0; iSegment < nCheckSegments; ++iSegment)
                {
                fraction += stepFraction;
                testFractions.push_back (fraction);
                }
            }
        else
            {
            // For single segment curves, testing the middle and one other location should suffice...
            testFractions.push_back (0.5);
            testFractions.push_back (0.25);
            }

        for (size_t iFraction = 0; iFraction < testFractions.size (); ++iFraction)
            {
            if (!curves0.at (iCurve)->FractionToPoint (testFractions.at (iFraction), point0) || !curves1.at (iCurve)->FractionToPoint (testFractions.at (iFraction), point1))
                return false;

            DVec3d  testTranslation;

            testTranslation.DifferenceOf (point1, point0);

            if (testTranslation.Distance (translation) > tolerance)
                return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::GetSectionCurveTranslation (DVec3dR translation, size_t section0, size_t section1) const
    {
    if (section0 == section1 || section0 >= m_sectionCurves.size () || section1 >= m_sectionCurves.size ())
        return false;

    double  tolerance = m_sectionCurves.at (section0)->ResolveTolerance (0.0);

    return getTranslationTo (translation, false, *m_sectionCurves.at (section0), *m_sectionCurves.at (section1), tolerance);
    }

GEOMDLLIMPEXP ISolidPrimitivePtr ISolidPrimitive::CreateDgnRuledSweep (DgnRuledSweepDetailCR data)
    {
    ISolidPrimitivePtr newPrimitive =  new DgnRuledSweep (data);
    return newPrimitive;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TryGetDgnRuledSweepDetail (DgnRuledSweepDetailR data) const { return false;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::_TrySetDgnRuledSweepDetail (DgnRuledSweepDetailCR data) { return false;}


GEOMDLLIMPEXP bool ISolidPrimitive::TryGetDgnRuledSweepDetail (DgnRuledSweepDetailR data ) const { return _TryGetDgnRuledSweepDetail (data);}
GEOMDLLIMPEXP bool ISolidPrimitive::TrySetDgnRuledSweepDetail (DgnRuledSweepDetailCR data)       { return _TrySetDgnRuledSweepDetail (data);}


GEOMDLLIMPEXP SolidPrimitiveType ISolidPrimitive::GetSolidPrimitiveType () const { return _GetSolidPrimitiveType ();}
//SolidPrimitiveType ISolidPrimitive::_GetSolidPrimitiveType () const { return SolidPrimitiveType_None;}

GEOMDLLIMPEXP bool ISolidPrimitive::GetRange (DRange3dR range) const {return _GetRange (range);}
GEOMDLLIMPEXP bool ISolidPrimitive::GetRange (DRange3dR range, TransformCR transform) const {return _GetRange (range, transform);}
GEOMDLLIMPEXP bool ISolidPrimitive::TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const {return _TryGetConstructiveFrame (localToWorld, worldToLocal);}


GEOMDLLIMPEXP IGeometryPtr ISolidPrimitive::GetFace (SolidLocationDetail::FaceIndices const &indices) const
    { 
    return _GetFace (indices);
    }

GEOMDLLIMPEXP void ISolidPrimitive::GetFaceIndices (bvector<SolidLocationDetail::FaceIndices> &indices) const
    { 
    return _GetFaceIndices (indices);
    }

GEOMDLLIMPEXP ICurvePrimitivePtr ISolidPrimitive::GetConstantVSection
    (SolidLocationDetail::FaceIndices const & indices, double fraction) const
        {return _GetConstantVSection (indices, fraction);}
GEOMDLLIMPEXP ICurvePrimitivePtr ISolidPrimitive::GetConstantUSection
    (SolidLocationDetail::FaceIndices const & indices, double fraction) const
        {return _GetConstantUSection (indices, fraction);}
GEOMDLLIMPEXP bool ISolidPrimitive::TryGetMaxUVLength
    (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const
        {return _TryGetMaxUVLength (indices, uvLength);}

GEOMDLLIMPEXP ISolidPrimitivePtr ISolidPrimitive::Clone () const {return _Clone ();}
GEOMDLLIMPEXP bool ISolidPrimitive::TransformInPlace (TransformCR transform) {return _TransformInPlace (transform);}



GEOMDLLIMPEXP bool ISolidPrimitive::TryUVFractionToXYZ
        (
        SolidLocationDetail::FaceIndices  const &indices,
        double uFraction,
        double vFraction,
        DPoint3dR xyz,
        DVec3dR dXdu,
        DVec3dR dXdv
        ) const
    {
    return _TryUVFractionToXYZ (indices, uFraction, vFraction, xyz, dXdu, dXdv);
    }


GEOMDLLIMPEXP bool ISolidPrimitive::GetCapped () const {return _GetCapped();}
GEOMDLLIMPEXP void ISolidPrimitive::SetCapped (bool value) {return _SetCapped(value);}

bool ISolidPrimitive::IsSameStructure (ISolidPrimitiveCR other) const {return _IsSameStructure (other);}
bool ISolidPrimitive::IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const {return _IsSameStructureAndGeometry (other, tolerance);}

GEOMDLLIMPEXP void ISolidPrimitive::AddRayIntersections
(
bvector<SolidLocationDetail> &pickData,
DRay3dCR ray,
int parentId,
double minParameter
) const
    {
    return _AddRayIntersections (pickData, ray, parentId, minParameter);
    }

GEOMDLLIMPEXP void ISolidPrimitive::AddCurveIntersections
(
    ICurvePrimitiveCR curve,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
) const
    {
    return _AddCurveIntersections (curve, curvePoints, solidPoints, messages);
    }

GEOMDLLIMPEXP void ISolidPrimitive::AddCurveIntersections
(
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
) const
    {
    return _AddCurveIntersections (curves, curvePoints, solidPoints, messages);
    }

bool ISolidPrimitive::ClosestPoint (DPoint3dCR spacePoint, SolidLocationDetail &pickDetail) const
    {
    return _ClosestPoint (spacePoint, pickDetail);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+--------------------------------------------------------------------------------------*/
static bool isDgnRotationalSweepDetailSphere (DgnRotationalSweepDetail& detail, ISolidPrimitivePtr& primitive)
    {
    DEllipse3d  ellipse;

    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == detail.m_baseCurve->HasSingleCurvePrimitive ())
        {
        ellipse = *detail.m_baseCurve->front ()->GetArcCP ();
        }
    else
        {
        if (!detail.m_baseCurve->IsClosedPath () || detail.m_baseCurve->size () > 2)
            return false;

        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == detail.m_baseCurve->at (0)->GetCurvePrimitiveType () &&
            ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line == detail.m_baseCurve->at (1)->GetCurvePrimitiveType ())
            ellipse = *detail.m_baseCurve->at (0)->GetArcCP ();
        else if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line == detail.m_baseCurve->at (0)->GetCurvePrimitiveType () &&
                 ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == detail.m_baseCurve->at (1)->GetCurvePrimitiveType ())
            ellipse = *detail.m_baseCurve->at (1)->GetArcCP ();
        else
            return false;
        }

    double      rSweep;
    DVec3d      rAxis;
    DPoint3d    rCenter;

    if (!detail.TryGetRotationAxis (rCenter, rAxis, rSweep))
        return false;

    double      x1, x2, arcStart, arcSweep;
    DVec3d      arcNormal;
    DPoint3d    arcCenter;
    RotMatrix   arcRMatrix;

    ellipse.GetScaledRotMatrix (arcCenter, arcRMatrix, x1, x2, arcStart, arcSweep);
    arcRMatrix.GetColumn (arcNormal, 2);

    DVec3d      delta, cross;

    // If the arc center is on the revolution axis and the revolution axis is in the plane of the boundary arc. then this is at least a partial sphere...
    delta.DifferenceOf (arcCenter, rCenter);
    cross.CrossProduct (delta, rAxis);

    if (!(fabs (cross.Magnitude ()) < mgds_fc_epsilon &&
          fabs (arcNormal.DotProduct (rAxis)) < mgds_fc_epsilon &&
          fabs (1.0 - x1/x2) < 1.0e-6))
        return false;

    // Check for complete sphere
    if (!((fabs (msGeomConst_2pi - fabs (arcSweep)) < 1.0e-6 &&
           fabs (msGeomConst_pi  - fabs (rSweep)) < 1.0e-6) ||
          (fabs (msGeomConst_pi  - fabs (arcSweep)) < 1.0e-6 &&
           fabs (msGeomConst_2pi - fabs (rSweep)) < 1.0e-6)))
        return false;

    DVec3d      xVec, yVec;
    RotMatrix   rMatrix;

    rAxis.Normalize ();
    xVec.NormalizedCrossProduct (arcNormal, rAxis);
    yVec.NormalizedCrossProduct (rAxis, xVec);
    rMatrix.InitFromColumnVectors (xVec, yVec, rAxis);

    DgnSphereDetail  sphereDetail (arcCenter, rMatrix, x1); sphereDetail.m_capped = detail.m_capped;

    primitive = ISolidPrimitive::CreateDgnSphere (sphereDetail);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+--------------------------------------------------------------------------------------*/
static bool isDgnRotationalSweepDetailTorus (DgnRotationalSweepDetail& detail, ISolidPrimitivePtr& primitive)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != detail.m_baseCurve->HasSingleCurvePrimitive ())
        return false;

    DEllipse3dCP  ellipse = detail.m_baseCurve->front ()->GetArcCP ();

    if (!ellipse->IsFullEllipse () || !ellipse->IsCircular ())
        return false;

    double      rSweep;
    DVec3d      rAxis;
    DPoint3d    rCenter;

    if (!detail.TryGetRotationAxis (rCenter, rAxis, rSweep))
        return false;

    double      x1, x2, arcStart, arcSweep;
    DVec3d      arcNormal;
    DPoint3d    arcCenter;
    RotMatrix   arcRMatrix;

    ellipse->GetScaledRotMatrix (arcCenter, arcRMatrix, x1, x2, arcStart, arcSweep);
    arcRMatrix.GetColumn (arcNormal, 2);

    if (fabs (arcNormal.DotProduct (rAxis)) > 1.0e-5)
        return false;

    double      closeParam;

    if (!DRay3d::FromOriginAndVector (rCenter, rAxis).ProjectPointUnbounded (rCenter, closeParam, arcCenter))
        return false;

    double      tRadius = arcCenter.Distance (rCenter);

    if (tRadius < 1.0e-12)
        return false; // degenerate case...

    DVec3d      xVec, yVec;

    xVec.NormalizedDifference (arcCenter, rCenter);
    yVec.NormalizedCrossProduct (rAxis, xVec);

    DgnTorusPipeDetail  torusDetail (rCenter, xVec, yVec, tRadius, x1, rSweep, detail.m_capped);

    primitive = ISolidPrimitive::CreateDgnTorusPipe (torusDetail);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/17
+--------------------------------------------------------------------------------------*/
static bool isDgnExtrusionDetailCone (DgnExtrusionDetail& detail, ISolidPrimitivePtr& primitive)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != detail.m_baseCurve->HasSingleCurvePrimitive ())
        return false;

    DEllipse3dCP  ellipse = detail.m_baseCurve->front ()->GetArcCP ();

    if (!ellipse->IsFullEllipse () || !ellipse->IsCircular ())
        return false;

    double      baseRadius, arcStart, arcSweep;
    DPoint3d    basePt, topPt;
    RotMatrix   baseRMatrix;

    ellipse->GetScaledRotMatrix (basePt, baseRMatrix, baseRadius, baseRadius, arcStart, arcSweep);
    topPt.SumOf(basePt, detail.m_extrusionVector);

    DgnConeDetail  coneDetail (basePt, topPt, baseRMatrix, baseRadius, baseRadius, detail.m_capped);

    primitive = ISolidPrimitive::CreateDgnCone (coneDetail);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+--------------------------------------------------------------------------------------*/
static bool isDgnRuledSweepDetailCone (DgnRuledSweepDetail& detail, ISolidPrimitivePtr& primitive)
    {
    if (2 != detail.m_sectionCurves.size ())
        return false;

    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != detail.m_sectionCurves.front ()->HasSingleCurvePrimitive () ||
        ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != detail.m_sectionCurves.back ()->HasSingleCurvePrimitive ())
        return false;

    DEllipse3dCP    baseEllipse = detail.m_sectionCurves.front ()->front ()->GetArcCP ();
    DEllipse3dCP    topEllipse  = detail.m_sectionCurves.back ()->front ()->GetArcCP ();

    if (!baseEllipse->IsFullEllipse () || !baseEllipse->IsCircular () ||
        !topEllipse->IsFullEllipse () || !topEllipse->IsCircular ())
        return false;

    double      baseRadius, topRadius, arcStart, arcSweep;
    DPoint3d    basePt, topPt;
    RotMatrix   baseRMatrix, topRMatrix;

    baseEllipse->GetScaledRotMatrix (basePt, baseRMatrix, baseRadius, baseRadius, arcStart, arcSweep);
    topEllipse->GetScaledRotMatrix (topPt, topRMatrix, topRadius, topRadius, arcStart, arcSweep);

    DVec3d      baseZ, topZ;

    baseRMatrix.GetColumn (baseZ, 2);
    topRMatrix.GetColumn (topZ, 2);
        
    if (fabs (baseZ.DotProduct (topZ) - 1.0) > mgds_fc_epsilon)
        return false; // Profiles aren't parallel...

    DVec3d      baseX, topX;

    baseX.Normalize (baseEllipse->vector0);
    topX.Normalize (topEllipse->vector0);

    if (!DoubleOps::WithinTolerance (baseX.DotProduct (topX), 1.0, 1.0e-10))
        return false; // Profiles have twist...

    DgnConeDetail  coneDetail (basePt, topPt, baseRMatrix, baseRadius, topRadius, detail.m_capped);

    primitive = ISolidPrimitive::CreateDgnCone (coneDetail);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/12
+--------------------------------------------------------------------------------------*/
static bool isBoxPoints (DVec3dR normal, bvector<DPoint3d> const* points)
    {
    Transform   localToWorld, worldToLocal;

    if (!PolylineOps::IsRectangle (&points->front (), points->size (), localToWorld, worldToLocal, false))
        return false;

    localToWorld.GetMatrixColumn (normal, 2);
    normal.Normalize ();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/17
+--------------------------------------------------------------------------------------*/
static bool isDgnExtrusionDetailBlock (DgnExtrusionDetail& detail, ISolidPrimitivePtr& primitive)
    {
    // NOTE: Treat physically closed open profile as normal extrusion to better preserve element type/id/association with modify handles...
    if (!detail.m_baseCurve->IsClosedPath ())
        return false;

    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString != detail.m_baseCurve->HasSingleCurvePrimitive ())
        return false;

    bvector<DPoint3d> const* basePoints = detail.m_baseCurve->front ()->GetLineStringCP ();
    DVec3d      baseZ;

    if (!isBoxPoints (baseZ, basePoints))
        return false;

    double      baseX, baseY, topX, topY;
    DVec3d      baseVectorX, baseVectorY, topVectorX, topVectorY;

    baseX = baseVectorX.NormalizedDifference (basePoints->at (1), basePoints->at (0));
    baseY = baseVectorY.NormalizedDifference (basePoints->at (2), basePoints->at (1));

    bvector<DPoint3d> topPoints = *basePoints;

    for (DPoint3dR topPt : topPoints)
        topPt.Add(detail.m_extrusionVector);

    topX = topVectorX.NormalizedDifference (topPoints.at (1), topPoints.at (0));
    topY = topVectorY.NormalizedDifference (topPoints.at (2), topPoints.at (1));

    DgnBoxDetail  boxDetail (basePoints->front (), topPoints.front (), baseVectorX, baseVectorY, baseX, baseY, topX, topY, detail.m_capped);

    primitive = ISolidPrimitive::CreateDgnBox (boxDetail);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+--------------------------------------------------------------------------------------*/
static bool isDgnRuledSweepDetailBlock (DgnRuledSweepDetail& detail, ISolidPrimitivePtr& primitive)
    {
    if (2 != detail.m_sectionCurves.size ())
        return false;

    // NOTE: Treat physically closed open profile as normal extrusion to better preserve element type/id/association with modify handles...
    if (!detail.m_sectionCurves.front ()->IsClosedPath () || !detail.m_sectionCurves.back ()->IsClosedPath ())
        return false;

    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString != detail.m_sectionCurves.front ()->HasSingleCurvePrimitive () ||
        ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString != detail.m_sectionCurves.back ()->HasSingleCurvePrimitive ())
        return false;

    bvector<DPoint3d> const* basePoints = detail.m_sectionCurves.front ()->front ()->GetLineStringCP ();
    bvector<DPoint3d> const* topPoints  = detail.m_sectionCurves.back ()->back ()->GetLineStringCP ();
    
    DVec3d      baseZ, topZ;

    if (!isBoxPoints (baseZ, basePoints) || !isBoxPoints (topZ, topPoints))
        return false;

    if (fabs (baseZ.DotProduct (topZ) - 1.0) > mgds_fc_epsilon)
        return false; // Profiles aren't parallel...

    double      baseX, baseY, topX, topY;
    DVec3d      baseVectorX, baseVectorY, topVectorX, topVectorY;

    baseX = baseVectorX.NormalizedDifference (basePoints->at (1), basePoints->at (0));
    baseY = baseVectorY.NormalizedDifference (basePoints->at (2), basePoints->at (1));

    topX = topVectorX.NormalizedDifference (topPoints->at (1), topPoints->at (0));
    topY = topVectorY.NormalizedDifference (topPoints->at (2), topPoints->at (1));

    if (!DoubleOps::WithinTolerance (baseVectorX.DotProduct (topVectorX), 1.0, 1.0e-10) ||
        !DoubleOps::WithinTolerance (baseVectorY.DotProduct (topVectorY), 1.0, 1.0e-10))
        return false; // Profiles have twist...

    DgnBoxDetail  boxDetail (basePoints->front (), topPoints->front (), baseVectorX, baseVectorY, baseX, baseY, topX, topY, detail.m_capped);

    primitive = ISolidPrimitive::CreateDgnBox (boxDetail);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/12
+--------------------------------------------------------------------------------------*/
static bool isDgnRuledSweepDetailExtrusion (DgnRuledSweepDetail& detail, ISolidPrimitivePtr& primitive)
    {
    if (2 != detail.m_sectionCurves.size ())
        return false;

    DVec3d  direction;

    if (!detail.GetSectionCurveTranslation (direction, 0, 1))
        return false; // Reject scale and twist...

    DgnExtrusionDetail  extrudeDetail (detail.m_sectionCurves.front (), direction, detail.m_capped);

    primitive = ISolidPrimitive::CreateDgnExtrusion (extrudeDetail);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::Simplify(ISolidPrimitivePtr& primitive)
    {
    if (!primitive.IsValid())
        return false;

    switch (primitive->GetSolidPrimitiveType())
        {
        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail detail;
    
            if (!primitive->TryGetDgnRotationalSweepDetail(detail))
                return false;

            if (isDgnRotationalSweepDetailSphere(detail, primitive))
                return true;

            if (isDgnRotationalSweepDetailTorus(detail, primitive))
                return true;

            return false;
            }

        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail detail;
    
            if (!primitive->TryGetDgnExtrusionDetail(detail))
                return false;

            if (isDgnExtrusionDetailCone(detail, primitive))
                return true;

            if (isDgnExtrusionDetailBlock(detail, primitive))
                return true;

            return false;
            }

        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail detail;
    
            if (!primitive->TryGetDgnRuledSweepDetail(detail))
                return false;

            if (isDgnRuledSweepDetailCone(detail, primitive))
                return true;

            if (isDgnRuledSweepDetailBlock(detail, primitive))
                return true;

            if (isDgnRuledSweepDetailExtrusion(detail, primitive))
                return true;
            
            return false;
            }

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/12
+--------------------------------------------------------------------------------------*/
bool ISolidPrimitive::HasCurvedFaceOrEdge () const
    {
    switch (GetSolidPrimitiveType ())
        {
        case SolidPrimitiveType_DgnTorusPipe:
        case SolidPrimitiveType_DgnCone:
        case SolidPrimitiveType_DgnSphere:
        case SolidPrimitiveType_DgnRotationalSweep:
            return true;

        case SolidPrimitiveType_DgnBox:
            return false;

        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail  detail;

            if (!TryGetDgnExtrusionDetail (detail))
                return false;

            return detail.m_baseCurve->ContainsNonLinearPrimitive ();
            }

        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail  detail;
    
            if (!TryGetDgnRuledSweepDetail (detail))
                return false;

            if (detail.m_sectionCurves.front ()->ContainsNonLinearPrimitive ())
                return true;

            DVec3d  translation; // Good enough just to check first and last?

            return !detail.GetSectionCurveTranslation (translation, 0, detail.m_sectionCurves.size ()-1);
            }

        default:
            return true;
        }
    }

PolyfaceHeaderPtr ISolidPrimitive::Facet (IFacetOptionsPtr const &options)
    {
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    builder->AddSolidPrimitive (*this);
    return builder->GetClientMeshPtr ();
    }

ISolidPrimitive::ISolidPrimitive ()
    {
    }

END_BENTLEY_GEOMETRY_NAMESPACE


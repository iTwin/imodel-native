/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/SolidPrimitive/sp_UVToXYZ.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "BoxFaces.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool IsValidPrimarySelector(SolidLocationDetail::FaceIndices const &selector, int num0, int num1)
    {
    int index0 = (int)selector.Index0 ();
    int index1 = (int)selector.Index1 ();
    if (    index0 >= 0  && index0 < num0
        &&  index1 >= 0  && index1 < num1
        )
        return true;
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void UVFractionToXYZ(double u, double v, DEllipse3dCR ellipse,
DPoint3dR xyz,
DVec3dR dXdu,
DVec3dR dXdv
)
    {
    xyz = DPoint3d::FromSumOf (ellipse.center,
            ellipse.vector0,  2.0 * (u - 0.5),
            ellipse.vector90, 2.0 * (v - 0.5)
            );
    dXdu.Scale (ellipse.vector0, 2.0);
    dXdv.Scale (ellipse.vector90, 2.0);
    }


GEOMDLLIMPEXP bool DgnConeDetail::TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &selector,
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const
    {
    DEllipse3d ellipse0, ellipse1;
    bool stat = true;
    if (selector.IsCap0 ())
        {
        FractionToSection (0.0, ellipse0);
        UVFractionToXYZ (uFraction, vFraction, ellipse0, xyz, dXdu, dXdv);        
        }
    else if (selector.IsCap1 ())
        {
        FractionToSection (1.0, ellipse1);
        UVFractionToXYZ (uFraction, vFraction, ellipse1, xyz, dXdu, dXdv);        
        }
    else if (IsValidPrimarySelector (selector, 1, 1))
        {
        FractionToSection (0.0, ellipse0);
        FractionToSection (1.0, ellipse1);

        DVec3d tangent0, tangent1;
        DVec3d k0, k1;
        DPoint3d xyz0, xyz1;
        ellipse0.FractionParameterToDerivatives (xyz0, tangent0, k0, uFraction);
        ellipse1.FractionParameterToDerivatives (xyz1, tangent1, k1, uFraction);
        xyz.Interpolate (xyz0, vFraction, xyz1);
        dXdu.Interpolate (tangent0, vFraction, tangent1);
        dXdv.DifferenceOf (xyz1, xyz0);
        }
    else
        {
        stat = false;
        }
    return stat;
    }

GEOMDLLIMPEXP bool DgnTorusPipeDetail::TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &selector,
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const
    {
    DEllipse3d minorEllipse, majorEllipse;
    bool stat = true;
    if (selector.IsCap0 ())
        {
        minorEllipse = VFractionToUSectionDEllipse3d (0.0);
        ISolidPrimitive::ReverseFractionOrientation (uFraction, vFraction);
        UVFractionToXYZ (uFraction, vFraction, minorEllipse, xyz, dXdu, dXdv);        
        ISolidPrimitive::ReverseFractionOrientation (dXdu, dXdv);
        }
    else if (selector.IsCap1 ())
        {
        minorEllipse = VFractionToUSectionDEllipse3d (1.0);
        UVFractionToXYZ (uFraction, vFraction, minorEllipse, xyz, dXdu, dXdv);        
        }
    else if (IsValidPrimarySelector (selector, 1, 1))
        {
        minorEllipse = VFractionToUSectionDEllipse3d (vFraction);
        majorEllipse = UFractionToVSectionDEllipse3d (uFraction);
        DVec3d k0, k1;
        DPoint3d xyz0, xyz1;
        minorEllipse.FractionParameterToDerivatives (xyz0, dXdu, k0, uFraction);
        majorEllipse.FractionParameterToDerivatives (xyz1, dXdv, k1, vFraction);
        xyz = xyz0;     // xyz1 is same point.
        }
    else
        stat = false;
    return stat;
    }


GEOMDLLIMPEXP bool DgnSphereDetail::TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &selector,
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const
    {
    DEllipse3d uEllipse, vEllipse;  // (u,v) ellipse is parameterized by (u,v) and is at constant (v,u)
    if (selector.IsCap0 ())
        {
        uEllipse = VFractionToUSectionDEllipse3d (0.0);
        UVFractionToXYZ (uFraction, vFraction, uEllipse, xyz, dXdu, dXdv);        
        }
    else if (selector.IsCap1 ())
        {
        uEllipse = VFractionToUSectionDEllipse3d (1.0);
        UVFractionToXYZ (uFraction, vFraction, uEllipse, xyz, dXdu, dXdv);        
        }
    else if (IsValidPrimarySelector (selector, 1, 1))
        {
        uEllipse = VFractionToUSectionDEllipse3d (vFraction);
        vEllipse = UFractionToVSectionDEllipse3d (uFraction);
        DVec3d k0, k1;
        DPoint3d xyz0, xyz1;
        uEllipse.FractionParameterToDerivatives (xyz0, dXdu, k0, uFraction);
        vEllipse.FractionParameterToDerivatives (xyz1, dXdv, k1, vFraction);
        xyz = xyz0;     // xyz1 is same point.
        }
    return true;
    }

GEOMDLLIMPEXP bool DgnBoxDetail::TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &selector,
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const
    {
    bool stat = false;
    if (selector.IsCap0 ()
        || selector.IsCap1 ()
        || IsValidPrimarySelector (selector, 1, 4)
        )
        {
        BoxFaces cornerData;
        cornerData.Load (*this);
        cornerData.Evaluate
            (
            BoxFaces::SingleFaceIndex ((int)selector.Index0 (), (int)selector.Index1 ()),
            uFraction, vFraction,
            xyz, dXdu, dXdv);
        stat = true;
        }
    return stat;
    }

GEOMDLLIMPEXP bool DgnExtrusionDetail::TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &selector,
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const
    {
    bool stat = false;
    if (   selector.IsCap0 () || selector.IsCap1 ())
        {
        if (selector.IsCap0 ())
            ISolidPrimitive::ReverseFractionOrientation (uFraction, vFraction);
        stat = m_baseCurve->TryUVFractionToXYZ (uFraction, vFraction, xyz, dXdu, dXdv);
        if (selector.IsCap1 ())
            xyz = DPoint3d::FromSumOf (xyz, m_extrusionVector, 1.0);
        else
            ISolidPrimitive::ReverseFractionOrientation (dXdu, dXdv);

        stat = true;
        }
    else if (selector.Index0 () == 0)
        {
        ICurvePrimitivePtr leaf = m_baseCurve->FindIndexedLeaf (selector.Index1 ());
        if (leaf.IsValid ())
            {
            DPoint3d xyz0;
            leaf->ComponentFractionToPoint (selector.Index2 (), uFraction, xyz0, dXdu);
            xyz = DPoint3d::FromSumOf (xyz0, m_extrusionVector, vFraction);
            dXdv = m_extrusionVector;
            stat = true;
            }
        }
    return stat;
    }

GEOMDLLIMPEXP bool DgnRotationalSweepDetail::TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &selector,
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const
    {
    bool stat = false;
    Transform curvePointToSurfacePoint, curvePointToSurfacePointVDerivative;
    if (   selector.IsCap1 () || selector.IsCap0 ())
        {
        if (selector.IsCap0 ())
            ISolidPrimitive::ReverseFractionOrientation (uFraction, vFraction);

        stat = m_baseCurve->TryUVFractionToXYZ (uFraction, vFraction, xyz, dXdu, dXdv);
        if (selector.IsCap1 () )
            {
            GetVFractionTransform (1.0, curvePointToSurfacePoint, curvePointToSurfacePointVDerivative);
            curvePointToSurfacePoint.Multiply (xyz);
            curvePointToSurfacePoint.MultiplyMatrixOnly (dXdu);
            curvePointToSurfacePoint.MultiplyMatrixOnly (dXdv);
            }
        else
            ISolidPrimitive::ReverseFractionOrientation (dXdu, dXdv);
        stat = true;
        }
    else if (selector.Index0 () == 0)
        {
        ICurvePrimitivePtr leaf =  m_baseCurve->FindIndexedLeaf (selector.Index1 ());
        if (leaf.IsValid ())
            {
            if (!GetVFractionTransform (vFraction, curvePointToSurfacePoint, curvePointToSurfacePointVDerivative))
                return false;
            DPoint3d xyz0;
            leaf->ComponentFractionToPoint ((int)selector.Index2 (), uFraction, xyz0, dXdu);
            curvePointToSurfacePoint.Multiply (xyz, xyz0);
            curvePointToSurfacePointVDerivative.Multiply (dXdv, xyz0);
            curvePointToSurfacePoint.MultiplyMatrixOnly (dXdu);
            stat = true;
            }
        }
    return stat;
    }

GEOMDLLIMPEXP bool DgnRuledSweepDetail::TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &selector,
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const
    {
    bool stat = false;
    if ( selector.IsCap0 ())
        {
        ISolidPrimitive::ReverseFractionOrientation (uFraction, vFraction);
        stat = m_sectionCurves[0]->TryUVFractionToXYZ (uFraction, vFraction, xyz, dXdu, dXdv);
        ISolidPrimitive::ReverseFractionOrientation (dXdu, dXdv);
        }
    else if (selector.IsCap1 ())
        {
        stat = m_sectionCurves.back ()->TryUVFractionToXYZ (uFraction, vFraction, xyz, dXdu, dXdv);
        }
    else
        {
        ICurvePrimitivePtr curveA, curveB;
        if (TryGetCurvePair (
                selector,
                curveA, curveB))
            {
            DPoint3d xyz0, xyz1;
            DVec3d dX0du, dX1du;
            curveA->ComponentFractionToPoint ((int)selector.Index2 (), uFraction, xyz0, dX0du);
            curveB->ComponentFractionToPoint ((int)selector.Index2 (), uFraction, xyz1, dX1du);
            xyz.Interpolate (xyz0, vFraction, xyz1);
            dXdu.Interpolate (dX0du, vFraction, dX1du);
            dXdv.DifferenceOf (xyz1, xyz0);
            stat = true;
            }
        }
    return stat;
    }

END_BENTLEY_GEOMETRY_NAMESPACE



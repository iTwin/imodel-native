/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::IsSameStructure (ISolidPrimitiveCR other) const
    {
    return SolidPrimitiveType_DgnCone == other.GetSolidPrimitiveType ()
        && m_capped == other.GetCapped ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::IsSameStructure (ISolidPrimitiveCR other) const
    {
    return SolidPrimitiveType_DgnTorusPipe == other.GetSolidPrimitiveType ()
        && m_capped == other.GetCapped ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::IsSameStructure (ISolidPrimitiveCR other) const
    {
    return SolidPrimitiveType_DgnSphere == other.GetSolidPrimitiveType ()
        && m_capped == other.GetCapped ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnBoxDetail::IsSameStructure (ISolidPrimitiveCR other) const
    {
    return SolidPrimitiveType_DgnBox == other.GetSolidPrimitiveType ()
        && m_capped == other.GetCapped ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::IsSameStructure (ISolidPrimitiveCR other) const
    {
    DgnExtrusionDetail otherDetail;
    if (!other.TryGetDgnExtrusionDetail (otherDetail))
        return false;
    return m_capped == otherDetail.m_capped
        && m_baseCurve->IsSameStructure (*otherDetail.m_baseCurve);
    }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::IsSameStructure (ISolidPrimitiveCR other) const
    {
    DgnRotationalSweepDetail otherDetail;
    if (!other.TryGetDgnRotationalSweepDetail (otherDetail))
        return false;
    return m_capped == otherDetail.m_capped
        && m_baseCurve->IsSameStructure (*otherDetail.m_baseCurve);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::IsSameStructure (ISolidPrimitiveCR other) const
    {
    DgnRuledSweepDetail otherDetail;
    if (!other.TryGetDgnRuledSweepDetail (otherDetail))
        return false;
    if (m_capped != otherDetail.m_capped)
        return false;        
    if (m_sectionCurves.size () != otherDetail.m_sectionCurves.size ())
        return false;
    for (size_t i = 0, n = m_sectionCurves.size (); i < n; i++)
        if (!m_sectionCurves[i]->IsSameStructure (*otherDetail.m_sectionCurves[i]))
            return false;
    return true;
    }



//Angle::NearlyEqualAllowPeriodShift
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const
    {
    DgnConeDetail otherDetail;
    if (!other.TryGetDgnConeDetail (otherDetail))
        return false;
    if (m_capped != otherDetail.m_capped)
        return false;
        
    if (!DPoint3dOps::AlmostEqual (m_centerA, otherDetail.m_centerA, tolerance))
        return false;        
    if (!DPoint3dOps::AlmostEqual (m_centerB, otherDetail.m_centerB, tolerance))
        return false;        

    if (!DVec3dOps::AlmostEqual (m_vector0, otherDetail.m_vector0, tolerance))
        return false;
    if (!DVec3dOps::AlmostEqual (m_vector90, otherDetail.m_vector90, tolerance))
        return false;        

    if (!DoubleOps::AlmostEqual (m_radiusA, otherDetail.m_radiusA, tolerance))
        return false;        
    if (!DoubleOps::AlmostEqual (m_radiusB, otherDetail.m_radiusB, tolerance))
        return false;       
         
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const
    {
   DgnTorusPipeDetail otherDetail;
    if (!other.TryGetDgnTorusPipeDetail (otherDetail))
        return false;
    if (m_capped != otherDetail.m_capped)
        return false;

    if (!DPoint3dOps::AlmostEqual (m_center, otherDetail.m_center, tolerance))
        return false;        

    if (!DVec3dOps::AlmostEqual (m_vectorX, otherDetail.m_vectorX, tolerance))
        return false;
    if (!DVec3dOps::AlmostEqual (m_vectorY, otherDetail.m_vectorY, tolerance))
        return false;        

    if (!DoubleOps::AlmostEqual (m_majorRadius, otherDetail.m_majorRadius, tolerance))
        return false;        
    if (!DoubleOps::AlmostEqual (m_minorRadius, otherDetail.m_minorRadius, tolerance))
        return false;       
    
    if (!Angle::NearlyEqualAllowPeriodShift (m_sweepAngle, otherDetail.m_sweepAngle))
        return false;
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const
    {
   DgnSphereDetail otherDetail;
    if (!other.TryGetDgnSphereDetail (otherDetail))
        return false;
    if (IsClosedVolume () != otherDetail.IsClosedVolume ())   // allows mismatched cap flags on full sweeps
        return false;

    // The sphere radii (real distances) are built into the transform columns, so the distance tolerance applies to both matrix and origin parts.
    if (!m_localToWorld.IsEqual (otherDetail.m_localToWorld, tolerance, tolerance))
        return false;        
    
    if (!Angle::NearlyEqualAllowPeriodShift (m_startLatitude, otherDetail.m_startLatitude))
        return false;
    if (!Angle::NearlyEqualAllowPeriodShift (m_latitudeSweep, otherDetail.m_latitudeSweep))
        return false;        
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnBoxDetail::IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const
    {
   DgnBoxDetail otherDetail;
    if (!other.TryGetDgnBoxDetail (otherDetail))
        return false;
    if (m_capped != otherDetail.m_capped)
        return false;

    if (!DPoint3dOps::AlmostEqual (m_baseOrigin, otherDetail.m_baseOrigin, tolerance))
        return false;
    if (!DPoint3dOps::AlmostEqual (m_topOrigin, otherDetail.m_topOrigin, tolerance))
        return false;                  

    if (!DVec3dOps::AlmostEqual (m_vectorX, otherDetail.m_vectorX, tolerance))
        return false;
    if (!DVec3dOps::AlmostEqual (m_vectorY, otherDetail.m_vectorY, tolerance))
        return false;        

    if (!DoubleOps::AlmostEqual (m_baseX, otherDetail.m_baseX, tolerance))
        return false;        
    if (!DoubleOps::AlmostEqual (m_baseY, otherDetail.m_baseY, tolerance))
        return false;       

    if (!DoubleOps::AlmostEqual (m_topX, otherDetail.m_topX, tolerance))
        return false;        
    if (!DoubleOps::AlmostEqual (m_topY, otherDetail.m_topY, tolerance))
        return false;        
 
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const
    {
    DgnExtrusionDetail otherDetail;
    if (!other.TryGetDgnExtrusionDetail (otherDetail))
        return false;
    if (m_capped != otherDetail.m_capped)
        return false;
        
    if (!DVec3dOps::AlmostEqual (m_extrusionVector, otherDetail.m_extrusionVector, tolerance))
        return false;
                            
    if (!m_baseCurve->IsSameStructureAndGeometry (*otherDetail.m_baseCurve, tolerance))
        return false;
    return true;
    }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const
    {
    DgnRotationalSweepDetail otherDetail;
    if (!other.TryGetDgnRotationalSweepDetail (otherDetail))
        return false;
    if (m_capped != otherDetail.m_capped)
        return false;
        

    if (!m_axisOfRotation.direction.IsParallelTo (otherDetail.m_axisOfRotation.direction))
        return false;
    if (!DPoint3dOps::AlmostEqual (m_axisOfRotation.origin, otherDetail.m_axisOfRotation.origin, tolerance))
        {
        // centers do not match -- but treat it as equal if they are shifted on the axis
        DVec3d centerToCenter = DVec3d::FromStartEnd (m_axisOfRotation.origin, otherDetail.m_axisOfRotation.origin);
        if (!centerToCenter.IsParallelTo (m_axisOfRotation.direction))
            return false;
        }
    if (!m_baseCurve->IsSameStructureAndGeometry (*otherDetail.m_baseCurve, tolerance))
        return false;
    return true;        
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const
    {
    DgnRuledSweepDetail otherDetail;
    if (!other.TryGetDgnRuledSweepDetail (otherDetail))
        return false;
    if (m_capped != otherDetail.m_capped)
        return false;        
    if (m_sectionCurves.size () != otherDetail.m_sectionCurves.size ())
        return false;
    for (size_t i = 0, n = m_sectionCurves.size (); i < n; i++)
        if (!m_sectionCurves[i]->IsSameStructureAndGeometry (*otherDetail.m_sectionCurves[i], tolerance))
            return false;
    return true;
    }




END_BENTLEY_GEOMETRY_NAMESPACE


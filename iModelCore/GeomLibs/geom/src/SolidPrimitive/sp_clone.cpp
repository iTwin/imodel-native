/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ISolidPrimitivePtr DgnConeDetail::Clone () const {return ISolidPrimitive::CreateDgnCone (*this);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ISolidPrimitivePtr DgnTorusPipeDetail::Clone () const {return ISolidPrimitive::CreateDgnTorusPipe (*this);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ISolidPrimitivePtr DgnSphereDetail::Clone () const {return ISolidPrimitive::CreateDgnSphere (*this);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ISolidPrimitivePtr DgnBoxDetail::Clone () const {return ISolidPrimitive::CreateDgnBox (*this);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ISolidPrimitivePtr DgnExtrusionDetail::Clone () const
    {
    DgnExtrusionDetail newDetail (m_baseCurve->Clone (), m_extrusionVector, m_capped);
    return ISolidPrimitive::CreateDgnExtrusion (newDetail);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ISolidPrimitivePtr DgnRotationalSweepDetail::Clone () const
    {
    DgnRotationalSweepDetail cloneDetail (m_baseCurve->Clone (),
            m_axisOfRotation.origin,
            m_axisOfRotation.direction,
            m_sweepAngle,
            m_capped
            );
    return ISolidPrimitive::CreateDgnRotationalSweep (cloneDetail);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ISolidPrimitivePtr DgnRuledSweepDetail::Clone () const
    {
    DgnRuledSweepDetail clone;
    for (size_t i = 0; i < m_sectionCurves.size (); i++)
        clone.m_sectionCurves.push_back (m_sectionCurves[i]->Clone ());
    clone.m_capped = m_capped;
    return ISolidPrimitive::CreateDgnRuledSweep (clone);
    }

// Transform remarks
// 1) Apply transform "directly" to all points and vectors.
// 2) Transformation of vector returns a scale factor and a normalized vector.
// 3) Apply the scale factor from the "first" (X, 0 degree) vector to all scalar sizes (radii)
// 4) What does a skew transform do?  Hard to say, but the DGN/QV layouts are ambiguous about what to do with it.
// 5) Exception:  Box data allows independent x,y scaling.


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool TransformAndRenormalizeVector(TransformCR transform, DVec3dCR vectorA, DVec3dR vectorB, double &lengthB)
    {
    transform.MultiplyMatrixOnly (vectorB, vectorA);
    lengthB = vectorB.Normalize ();
    return lengthB != 0.0;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::TransformInPlace (TransformCR transform)
    {
    transform.Multiply (m_centerA);
    transform.Multiply (m_centerB);
    double scale0, scale90;
    if (   TransformAndRenormalizeVector (transform, m_vector0, m_vector0, scale0)
        && TransformAndRenormalizeVector (transform, m_vector90, m_vector90, scale90))
        {
        m_radiusA *= scale0;
        m_radiusB *= scale0;
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::TransformInPlace (TransformCR transform)
    {
    transform.Multiply (m_center);

    double scaleX, scaleY;
    if (   TransformAndRenormalizeVector (transform, m_vectorX, m_vectorX, scaleX)
        && TransformAndRenormalizeVector (transform, m_vectorY, m_vectorY, scaleY))
        {
        m_majorRadius *= scaleX;
        m_minorRadius *= scaleX;
        return true;
        }
    return false;
    }

void CorrectSoDeterminantIsPositive(DgnSphereDetailR detail)
    {
    if (detail.m_localToWorld.Determinant() < 0.0)
        {
        static bool s_flipSweep = false;
        // force the determinant positive by flipping y.
        // flip signs of latitude range to compensate.
        detail.m_localToWorld = Transform::FromProduct(detail.m_localToWorld, RotMatrix::FromScaleFactors(1.0, -1.0, 1.0));
        if (s_flipSweep)
            {
            detail.m_startLatitude *= -1.0;
            detail.m_latitudeSweep *= -1.0;
            }
        }

    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::TransformInPlace (TransformCR transform)
    {
    m_localToWorld.InitProduct (transform, m_localToWorld);
    CorrectSoDeterminantIsPositive (*this);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnBoxDetail::TransformInPlace (TransformCR transform)
    {
    //DgnBoxDetail newDetail = *this;
    transform.Multiply (m_baseOrigin);
    transform.Multiply (m_topOrigin);

    double scaleX, scaleY;
    if (   TransformAndRenormalizeVector (transform, m_vectorX, m_vectorX, scaleX)
        && TransformAndRenormalizeVector (transform, m_vectorY, m_vectorY, scaleY))
        {
        m_baseX *= scaleX;
        m_baseY *= scaleY;
        m_topX  *= scaleX;
        m_topY  *= scaleY;
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::TransformInPlace (TransformCR transform)
    {
    transform.MultiplyMatrixOnly (m_extrusionVector);
    m_baseCurve->TransformInPlace (transform);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::TransformInPlace (TransformCR transform)
    {
    transform.Multiply (m_axisOfRotation, m_axisOfRotation);
    m_baseCurve->TransformInPlace (transform);
    
    if (transform.Determinant() < 0.0)
        m_sweepAngle = -m_sweepAngle;

    return true;    
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::TransformInPlace (TransformCR transform)
    {
    for (size_t i = 0, n = m_sectionCurves.size (); i < n; i++)
        {
        m_sectionCurves[i]->TransformInPlace (transform);
        }
    return true;
    }




END_BENTLEY_GEOMETRY_NAMESPACE


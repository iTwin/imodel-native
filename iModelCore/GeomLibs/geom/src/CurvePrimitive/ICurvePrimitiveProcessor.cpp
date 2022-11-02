/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::_ProcessDefault (ICurvePrimitiveCR curve, DSegment1dCP interval)
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::_ProcessLine (ICurvePrimitiveCR curve, DSegment3dCR segment, DSegment1dCP interval)
    {_ProcessDefault (curve, interval);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::_ProcessArc (ICurvePrimitiveCR curve, DEllipse3dCR arc, DSegment1dCP interval)
    {_ProcessDefault (curve, interval);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::_ProcessLineString (ICurvePrimitiveCR curve, bvector<DPoint3d> const &points, DSegment1dCP interval)
    {_ProcessDefault (curve, interval);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::_ProcessBsplineCurve (ICurvePrimitiveCR curve, MSBsplineCurveCR bcurve, DSegment1dCP interval)
    {_ProcessDefault (curve, interval);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::_ProcessProxyBsplineCurve (ICurvePrimitiveCR curve, MSBsplineCurveCR bcurve, DSegment1dCP interval)
    {_ProcessDefault (curve, interval);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::_ProcessPartialCurve (ICurvePrimitiveCR curve, PartialCurveDetailCR detail, DSegment1dCP interval)
    {_ProcessDefault (curve, interval);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::_ProcessPointString (ICurvePrimitiveCR curve, bvector<DPoint3d>const &points, DSegment1dCP interval)
    {_ProcessDefault (curve, interval);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::_ProcessInterpolationCurve (ICurvePrimitiveCR curve, MSInterpolationCurveCR icurve, DSegment1dCP interval)
    {_ProcessProxyBsplineCurve (curve, *curve.GetProxyBsplineCurveCP (), interval);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::_ProcessAkimaCurve (ICurvePrimitiveCR curve, bvector<DPoint3d>const &points, DSegment1dCP interval)
    {_ProcessProxyBsplineCurve (curve, *curve.GetProxyBsplineCurveCP (), interval);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::_ProcessSpiral (ICurvePrimitiveCR curve, DSpiral2dPlacementCR spiral, DSegment1dCP interval)
    {_ProcessProxyBsplineCurve (curve, *curve.GetProxyBsplineCurveCP (), interval);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::_ProcessChildCurveVector (ICurvePrimitiveCR curve, CurveVector const &childVector, DSegment1dCP interval)
    {
    if (!GetAbortFlag ())
        {
        for (size_t i = 0, n = childVector.size (); i < n; i++)
            {
            if (GetAbortFlag ())
                break;
            childVector[i]->_Process (*this, interval);
            }
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitiveProcessor::GetAbortFlag () { return m_aborted;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::SetAbortFlag (bool value) { m_aborted = value;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ICurvePrimitiveProcessor::_ProcessCurveVector (CurveVector const &curveVector, DSegment1dCP interval)
    {
    if (!GetAbortFlag ())
        {
        for (size_t i = 0, n = curveVector.size (); i < n; i++)
            {
            if (GetAbortFlag ())
                break;
            curveVector[i]->_Process (*this, interval);
            }
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE

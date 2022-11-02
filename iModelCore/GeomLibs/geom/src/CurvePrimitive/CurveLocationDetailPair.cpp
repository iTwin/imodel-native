/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveLocationDetailPair::CurveLocationDetailPair(CurveLocationDetailCR _detailA, CurveLocationDetailCR _detailB)
    : detailA (_detailA), detailB (_detailB)
    {}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveLocationDetailPair::CurveLocationDetailPair() {}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveLocationDetailPair::CurveLocationDetailPair(CurveLocationDetailCR _detailA)
    : detailA (_detailA), detailB (_detailA)
    {}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveLocationDetailPair::CurveLocationDetailPair(ICurvePrimitiveCP curve,
                double fraction, DPoint3dCR point)
    : detailA (curve, fraction, point), detailB (curve, fraction, point)
    {}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveLocationDetailPair::CurveLocationDetailPair(ICurvePrimitiveCP curve,
                double fraction, DPoint3dCR point,
                size_t componentIndex, size_t numComponent, double componentFraction)
    : detailA (curve, fraction, point, componentIndex, numComponent, componentFraction),
      detailB (curve, fraction, point, componentIndex, numComponent, componentFraction)
    {}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveLocationDetailPair::CurveLocationDetailPair(ICurvePrimitiveCP curve,
        double fraction0, DPoint3dCR point0, double fraction1, DPoint3dCR point1)
    : detailA (curve, fraction0, point0), detailB (curve, fraction1, point1)
    {}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveLocationDetailPair::CurveLocationDetailPair(
    ICurvePrimitiveCP curve0, double a0, 
    ICurvePrimitiveCP curve1, double a1
    )
    : detailA (curve0), detailB (curve1)
    {
    detailA.a = a0;
    detailB.a = a1;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveLocationDetailPair::Set (double fraction0, DPoint3dCR point0, double a0, double fraction1, DPoint3dCR point1, double a1)
    {
    detailA.fraction = fraction0;
    detailA.point    = point0;
    detailA.a        = a0;
    detailB.fraction = fraction1;
    detailB.point    = point1;
    detailB.a        = a1;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveLocationDetailPair::Set (ICurvePrimitiveP curveA, ICurvePrimitiveP curveB)
    {
    detailA.curve  = curveA;
    detailB.curve  = curveB;
    }


//! Return true if the two details are for (bitwise) identical curve and fraction.
bool CurveLocationDetailPair::SameCurveAndFraction ()
    {
    return     detailA.curve == detailB.curve
            && detailA.fraction == detailB.fraction;
    }

//! Return true if the two details are for (bitwise) identical curve and fraction.
DSegment3d CurveLocationDetailPair::GetDSegment3d () const
    {
    DSegment3d segment;
    segment.point[0] = detailA.point;
    segment.point[1] = detailB.point;
    return segment;
    }

double CurveLocationDetailPair::DistanceSquared () const
    {
    return detailA.point.DistanceSquared (detailB.point);
    }

double CurveLocationDetailPair::DeltaZ() const
    {
    return detailB.point.z - detailA.point.z;
    }


double CurveLocationDetailPair::Distance () const
    {
    return detailA.point.Distance (detailB.point);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveLocationDetailPair::DeltaZExtremes (bvector<CurveLocationDetailPair> const &pairs, size_t &iMin, size_t &iMax)
    {
    iMin = iMax = SIZE_MAX;
    double dMin = DBL_MAX;
    double dMax = -DBL_MAX;
    for (size_t i = 0; i < pairs.size (); i++)
        {
        double d = pairs[i].DeltaZ ();
        if (d < dMin)
            {
            dMin = d;
            iMin = i;
            }
        if (d > dMax)
            {
            dMax = d;
            iMax = i;
            }
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
 void CurveLocationDetailPair::SplitByDeltaZ
 (
 bvector<CurveLocationDetailPair> const &pairs,
 double splitDistance,
 bvector<CurveLocationDetailPair> *pairA,
 bvector<CurveLocationDetailPair> *pairB
 )
    {
    if (pairA != nullptr)
        pairA->clear ();
    if (pairB != nullptr)
        pairB->clear ();
    for (CurveLocationDetailPair const &pair : pairs)
        {
        double d = pair.DeltaZ ();
        if (d <= splitDistance)
            {
            if (pairA != nullptr)
                pairA->push_back (pair);
            }
        else
            {
            if (pairB != nullptr)
                pairB->push_back (pair);
            }
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE
/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/CurveLocationDetail.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// This file is to be included in CurvePrimitive.cpp WITHIN the namespace brackets....
//! Construct from simple fraction in single component curve.
CurveLocationDetail::CurveLocationDetail ()
    : curve(NULL),
      fraction(0.0),
      componentIndex(0),
      numComponent(0),
      componentFraction(fraction),
      a(0.0)
    {
    point.Zero ();
    }

//! Construct from simple fraction in single component curve.
CurveLocationDetail::CurveLocationDetail (ICurvePrimitiveCP _curve, size_t _numComponent)
    : curve((ICurvePrimitiveP)_curve),
      fraction(0.0),
      componentIndex(0),
      numComponent(_numComponent),
      componentFraction(fraction),
      a(0.0)
    {
    point.Zero ();
    }


//! Construct from simple fraction in single component curve.
CurveLocationDetail::CurveLocationDetail (ICurvePrimitiveCP _curve, double _fraction, DPoint3dCR _point)
    : curve((ICurvePrimitiveP)_curve),
      fraction(_fraction),
      componentIndex(0),
      numComponent(1),
      componentFraction(fraction),
      point(_point),
      a(0.0)
    {
    }

//! Construct with full indexing
CurveLocationDetail::CurveLocationDetail (ICurvePrimitiveCP _curve, double _fraction, DPoint3dCR _point, size_t _componentIndex, size_t _numComponent, double _componentFraction)
    : curve((ICurvePrimitiveP)_curve),
      fraction(_fraction),
      componentIndex(_componentIndex),
      numComponent(_numComponent),
      componentFraction(_componentFraction),
      point(_point),
      a(0.0)
    {
    }

#ifdef CurveLocationDetailFullCTorInSource
//! Construct with full indexing
CurveLocationDetail::CurveLocationDetail (ICurvePrimitiveCP _curve, double _fraction, DPoint3dCR _point, size_t _componentIndex, size_t _numComponent, double _componentFraction, double _a)
    : curve((ICurvePrimitiveP)_curve),
      fraction(_fraction),
      componentIndex(_componentIndex),
      numComponent(_numComponent),
      componentFraction(_componentFraction),
      point(_point),
      a(_a)
    {
    }
#endif

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveLocationDetail::UpdateIfCloser (CurveLocationDetailCR otherDetail)
    {
    if (otherDetail.a < a)
        {
        *this = otherDetail;
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double CurveLocationDetail::Distance (CurveLocationDetailCR other) const
    {return point.Distance (other.point);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveLocationDetail::Interpolate (bvector<double> const &params, double &result) const
    {
    size_t n = params.size ();
    if (componentIndex < n - 1)
        {
        result = params[componentIndex] +
                 componentFraction * (params[componentIndex + 1] - params[componentIndex]);
        return true;
        }
    result = 0.0;
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2015
+--------------------------------------------------------------------------------------*/
ValidatedCurveLocationDetail CurveLocationDetail::Interpolate (double interpolationFraction, CurveLocationDetailCR other) const
    {
    size_t newNumComponent = numComponent;
    if (numComponent == 0)
        newNumComponent = 1;
    double newFraction = DoubleOps::Interpolate (fraction, interpolationFraction, other.fraction);
    size_t indexB;
    double uB;
    // (hm.. newNumComponent will get reassigned, should be the same)
    bool isExtrapolated;
    PolylineOps::PolylineFractionToSegmentData (newNumComponent + 1, newFraction, indexB, newNumComponent, uB, isExtrapolated);

    return  ValidatedCurveLocationDetail
                (
                CurveLocationDetail (curve,
                        newFraction,
                        DPoint3d::FromInterpolate (point, interpolationFraction, other.point),
                        indexB, newNumComponent,
                        uB,
                        DoubleOps::Interpolate (a, interpolationFraction, other.a)
                        ),
           curve == other.curve
                && newNumComponent == numComponent
                && newNumComponent == other.numComponent
            );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveLocationDetail::SetMaxDistance ()
    {
    a = DBL_MAX;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveLocationDetail::SetDistance (double value)
    {
    a = value;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveLocationDetail::AlmostEqualPoint (CurveLocationDetailCR other) const
    {
    return point.AlmostEqual (other.point);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveLocationDetail::SetSingleComponentData ()
    {
    componentFraction = fraction;
    componentIndex = 0;
    numComponent = 1;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveLocationDetail::SetSingleComponentFractionAndA (double _fraction, double _a)
    {
    fraction = componentFraction = _fraction;
    componentIndex = 0;
    numComponent = 1;
    a = _a;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double CurveLocationDetail::SetDistanceFrom (DPoint3dCR otherPoint)
    {
    return (a = point.Distance (otherPoint));
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveLocationDetail::TryFractionToPoint (DPoint3dR xyz, DVec3dR dXdf) const
    {
    if (curve != NULL)
        {
        curve->FractionToPoint (fraction, xyz, dXdf);
        return true;
        }
    return false;
    }

ValidatedDRay3d CurveLocationDetail::PointAndUnitTangent () const
    {
    DRay3d ray;
    if (curve != nullptr
        && curve->FractionToPoint (fraction, ray.origin, ray.direction))
            return ray.ValidatedNormalize ();
    ray.origin = point;
    ray.direction.Zero ();
    return ValidatedDRay3d (ray, false);
    }    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveLocationDetail::TryComponentFractionToPoint (DPoint3dR xyz, DVec3dR dXdf) const
    {
    if (curve != NULL)
        {
        curve->ComponentFractionToPoint ((int)componentIndex, componentFraction, xyz, dXdf);
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2015
+--------------------------------------------------------------------------------------*/
void CurveLocationDetail::SetFractionFromComponentFraction (double _componentFraction, size_t _componentIndex, size_t _numComponent)
    {
    componentFraction = _componentFraction;
    componentIndex = _componentIndex;
    numComponent = _numComponent;
    double df = numComponent == 0 ? 1.0 : 1.0 / (double)numComponent;
    fraction = (_componentFraction + (double)_componentIndex) * df;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2015
+--------------------------------------------------------------------------------------*/
void CurveLocationDetail::SetComponentFractionFromFraction (double globalFraction, size_t _numComponent)
    {
    if (_numComponent <= 1)
        {
        fraction = componentFraction = globalFraction;
        numComponent = 1;
        componentIndex = 0;
        }
    else
        {
        numComponent = _numComponent;
        double scaledFraction = numComponent * globalFraction;
        if (scaledFraction < 1.0)
            {
            componentIndex = 0;
            }
        else if (globalFraction >= (numComponent - 1))
            {
            componentIndex = numComponent - 1;
            }
        else
            {
            componentIndex = (size_t)scaledFraction;
            }
        componentFraction = scaledFraction - (double)componentIndex;
        }
    }

static bool cb_lessThan_curveAndFraction (CurveLocationDetailCR dataA, CurveLocationDetailCR dataB)
    {
    if ((size_t)dataA.curve == (size_t)dataB.curve)
        return dataA.fraction < dataB.fraction;
    return (size_t)dataA.curve < (size_t)dataB.curve;
    }

void CurveLocationDetail::SortByCurveAndFraction (bvector<CurveLocationDetail> &detail)
    {
    std::sort (detail.begin (), detail.end (), cb_lessThan_curveAndFraction);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
ValidatedCurveLocationDetail CurveLocationDetail::ClosestPoint (bvector<CurveLocationDetail> &data, DPoint3dCR xyz, double searchRadius)
    {
    double dMin = DBL_MAX;
    ValidatedCurveLocationDetail closestDetail;
    for (auto &detail : data)
        {
        double d = detail.point.Distance (xyz);
        if (d < dMin)
            {
            closestDetail = ValidatedCurveLocationDetail (detail, true);
            dMin = d;
            }
        }
    return closestDetail;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
ValidatedCurveLocationDetailPair CurveLocationDetail::ClosestPoint (bvector<CurveLocationDetailPair> &data, DPoint3dCR xyz, double searchRadius)
    {
    double dMin = DBL_MAX;
    ValidatedCurveLocationDetailPair closestDetail;
    for (auto &pair : data)
        {
        double d = pair.detailA.point.Distance (xyz);
        if (d < dMin && d <= searchRadius)
            {
            closestDetail = ValidatedCurveLocationDetailPair (pair, true);
            dMin = d;
            }
        d = pair.detailB.point.Distance (xyz);
        if (d < dMin && d <= searchRadius)
            {
            closestDetail = ValidatedCurveLocationDetailPair (pair, true);
            dMin = d;
            }
        }
    return closestDetail;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
bool CurveKeyPointCollector::NeedKeyPointType (CurveKeyPointCollector::KeyPointType t) const
    {
    uint32_t it = (uint32_t)t; 
    return it < KeyPointType::NumTypes && m_needKeyPointType [it];
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
void CurveKeyPointCollector::EnableKeyPointType (CurveKeyPointCollector::KeyPointType t, bool value)
    {
    uint32_t it = (uint32_t)t; 
    if (it < KeyPointType::NumTypes)
        m_needKeyPointType [it] = value;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
bool CurveKeyPointCollector::GetWorldToLocal (DMatrix4dR worldToLocal) const
    {
    worldToLocal = m_worldToLocal;
    return m_xyOnly;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
void CurveKeyPointCollector::SetXYOnly (DMatrix4dCR worldToLocal)
    {
    m_worldToLocal = worldToLocal;
    m_xyOnly = true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
CurveKeyPointCollector::CurveKeyPointCollector()
    {
    m_xyOnly = false;
    m_worldToLocal.InitIdentity ();
    EnableKeyPointType (KeyPointType::EndPoint, true);
    EnableKeyPointType (KeyPointType::BreakPoint, true);
    EnableKeyPointType (KeyPointType::Perpendicular, true);
    EnableKeyPointType (KeyPointType::Tangency, false);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
void CurveKeyPointCollector::EnableSingleKeyPointType (CurveKeyPointCollector::KeyPointType type)
    {
    EnableKeyPointType (KeyPointType::EndPoint, false);
    EnableKeyPointType (KeyPointType::BreakPoint, false);
    EnableKeyPointType (KeyPointType::Perpendicular, false);
    EnableKeyPointType (KeyPointType::Tangency, false);

    EnableKeyPointType (type, true);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
void CurveKeyPointCollector::AnnouncePoint (CurveLocationDetailCR worldDetail, CurveKeyPointCollector::KeyPointType selector)
    {

    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
CurveKeyPoint_ClosestPointCollector::CurveKeyPoint_ClosestPointCollector
(
DPoint3dCR biasPoint //!< [in] world point for distance calculations.
) : CurveKeyPointCollector ()
    {
    m_worldBiasPoint = biasPoint;
    m_closestPoint.a = DBL_MAX;
    m_keyPointType = KeyPointType::NumTypes;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
void CurveKeyPoint_ClosestPointCollector::AnnouncePoint (CurveLocationDetailCR worldDetail, KeyPointType selector)
    {
    double a = DBL_MAX;
    if (m_xyOnly)
        {
        DPoint3d biasXYZ;
        DPoint3d newXYZ;
        m_worldToLocal.MultiplyAndRenormalize (biasXYZ, m_worldBiasPoint);
        m_worldToLocal.MultiplyAndRenormalize (newXYZ, worldDetail.point);
        a = biasXYZ.Distance (newXYZ);
        }
    else
        {
        a = m_worldBiasPoint.Distance (worldDetail.point);
        }
    if (a < m_closestPoint.a)
        {
        m_closestPoint = worldDetail;
        m_closestPoint.a = a;
        m_keyPointType = selector;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
bool CurveKeyPoint_ClosestPointCollector::GetResult (CurveLocationDetailR worldDetail, KeyPointType &selector) const
    {
    worldDetail = m_closestPoint;
    selector = m_keyPointType;
    return m_keyPointType != KeyPointType::NumTypes;
    }
END_BENTLEY_GEOMETRY_NAMESPACE

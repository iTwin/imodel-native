/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE





/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/10
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveChildCurveVector : public ICurvePrimitive
{
protected:

CurveVectorPtr m_childCurveVector;


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
explicit CurvePrimitiveChildCurveVector(CurveVector::BoundaryType boundaryType) : m_childCurveVector(CurveVector::Create(boundaryType))  {}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
explicit CurvePrimitiveChildCurveVector(CurveVectorPtr child) : m_childCurveVector(child) {}



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _Clone() const override {return Create_CopyFromSource (*m_childCurveVector);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurvePrimitiveType _GetCurvePrimitiveType() const override {return CURVE_PRIMITIVE_TYPE_CurveVector;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorCP _GetChildCurveVectorCP() const override {return m_childCurveVector.get ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr _GetChildCurveVectorP() override {return m_childCurveVector.get ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override 
    {processor._ProcessChildCurveVector (*this, *m_childCurveVector, interval);}

// We do not implement parameterized curve functions EXCEPT for dispatching ClosetPoint with detail output.
//   The detail output will point at a leaf.


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
using ICurvePrimitive::_ClosestPointBounded;    // suppresses C4266
bool _ClosestPointBounded (DPoint3dCR spacePoint, CurveLocationDetailR location, bool extend0, bool extend1) const override
    {return m_childCurveVector->ClosestPointBounded (spacePoint, location);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _ClosestPointBoundedXY(DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location, bool extend0, bool extend1) const override
    {return m_childCurveVector->ClosestPointBoundedXY (spacePoint, worldToLocal, location, extend0, extend1);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
using ICurvePrimitive::_GetStartEnd;    // suppresses C4266


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _GetStartEnd(DPoint3dR pointA, DPoint3dR pointB) const override {return m_childCurveVector->GetStartEnd (pointA, pointB);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _CloneBetweenFractions
(
double fractionA,
double fractionB,
bool allowExtrapolation
) const override
    {
    // Hmmm. should we test for single primitive?
    // NO -- other ops that take a single fraction fail.
    return NULL;
    }

public: 


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static ICurvePrimitivePtr Create_CopyFromSource(CurveVectorCR source)
    {
    CurvePrimitiveChildCurveVector *child = new CurvePrimitiveChildCurveVector (source.m_boundaryType);
    child->m_childCurveVector = source.Clone ();
    return child;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static ICurvePrimitivePtr Create (CurveVectorPtr source)
    {
    return new CurvePrimitiveChildCurveVector (source);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static ICurvePrimitivePtr Create_SwapFromSource(CurveVectorR source)
    {
    CurvePrimitiveChildCurveVector *child = new CurvePrimitiveChildCurveVector (source.m_boundaryType);
    child->m_childCurveVector->swap (source);
    return child;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _Length(double &length) const override
    {
    length = m_childCurveVector->Length ();
    return true;
    }
	
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _Length(RotMatrixCP worldToLocal, double &length) const override
    {
    length = m_childCurveVector->Length (worldToLocal);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _GetRange(DRange3dR range) const override
    {
    return m_childCurveVector->GetRange (range);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _GetRange(DRange3dR range, TransformCR transform) const override
    {
    return m_childCurveVector->GetRange (range, transform);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double _FastMaxAbs() const override
    {
    return m_childCurveVector->FastMaxAbs ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/    
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    CurveVectorCP otherChild = other.GetChildCurveVectorCP ();
    return NULL != otherChild
        && m_childCurveVector->IsSameStructureAndGeometry (*otherChild);
    }    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2013
+--------------------------------------------------------------------------------------*/
size_t _NumComponent () const override {return 0;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DRange1d _ProjectedParameterRange(DRay3dCR ray, double fractionA, double fractionB) const override
    {
    // ugh.  Fraction doesn't make sense ...
    return m_childCurveVector->ProjectedParameterRange (ray);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DRange1d _ProjectedParameterRange(DRay3dCR ray) const override
    {
    return m_childCurveVector->ProjectedParameterRange (ray);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TransformInPlace (TransformCR transform) override
    {
    for (size_t i = 0, n = m_childCurveVector->size (); i < n; i++)
        m_childCurveVector->at(i)->TransformInPlace(transform);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _ReverseCurvesInPlace () override
    {
    m_childCurveVector->ReverseCurvesInPlace ();
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _AppendCurvePlaneIntersections(DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tol) const override 
    {
    AppendTolerancedPlaneIntersections (plane, this, *m_childCurveVector.get (), intersections, tol);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _WireCentroid(double &length, DPoint3dR centroid, double fraction0, double fraction1) const override 
    {
    if (DoubleOps::IsExact01 (fraction0, fraction1))
        {
        return m_childCurveVector->WireCentroid (length, centroid);
        }
    return false;
    }

}; // CurvePrimitiveChildCurveVector


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateChildCurveVector_CopyFromSource (CurveVectorCR source)  {return CurvePrimitiveChildCurveVector::Create_CopyFromSource (source);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateChildCurveVector_SwapFromSource (CurveVectorR source)  {return CurvePrimitiveChildCurveVector::Create_SwapFromSource (source);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateChildCurveVector (CurveVectorPtr source)  {return CurvePrimitiveChildCurveVector::Create (source);}



END_BENTLEY_GEOMETRY_NAMESPACE

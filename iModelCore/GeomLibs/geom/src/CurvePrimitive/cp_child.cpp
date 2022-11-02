/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE





/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveChildCurveVector : public ICurvePrimitive
{
protected:

CurveVectorPtr m_childCurveVector;


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
explicit CurvePrimitiveChildCurveVector(CurveVector::BoundaryType boundaryType) : m_childCurveVector(CurveVector::Create(boundaryType))  {}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
explicit CurvePrimitiveChildCurveVector(CurveVectorPtr child) : m_childCurveVector(child) {}



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _Clone() const override {return Create_CopyFromSource (*m_childCurveVector);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurvePrimitiveType _GetCurvePrimitiveType() const override {return CURVE_PRIMITIVE_TYPE_CurveVector;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorCP _GetChildCurveVectorCP() const override {return m_childCurveVector.get ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr _GetChildCurveVectorP() override {return m_childCurveVector.get ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override 
    {processor._ProcessChildCurveVector (*this, *m_childCurveVector, interval);}

// We do not implement parameterized curve functions EXCEPT for dispatching ClosetPoint with detail output.
//   The detail output will point at a leaf.


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
using ICurvePrimitive::_ClosestPointBounded;    // suppresses C4266
bool _ClosestPointBounded (DPoint3dCR spacePoint, CurveLocationDetailR location, bool extend0, bool extend1) const override
    {return m_childCurveVector->ClosestPointBounded (spacePoint, location);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _ClosestPointBoundedXY(DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location, bool extend0, bool extend1) const override
    {return m_childCurveVector->ClosestPointBoundedXY (spacePoint, worldToLocal, location, extend0, extend1);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
using ICurvePrimitive::_GetStartEnd;    // suppresses C4266


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _GetStartEnd(DPoint3dR pointA, DPoint3dR pointB) const override {return m_childCurveVector->GetStartEnd (pointA, pointB);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
static ICurvePrimitivePtr Create_CopyFromSource(CurveVectorCR source)
    {
    CurvePrimitiveChildCurveVector *child = new CurvePrimitiveChildCurveVector (source.m_boundaryType);
    child->m_childCurveVector = source.Clone ();
    return child;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static ICurvePrimitivePtr Create (CurveVectorPtr source)
    {
    return new CurvePrimitiveChildCurveVector (source);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static ICurvePrimitivePtr Create_SwapFromSource(CurveVectorR source)
    {
    CurvePrimitiveChildCurveVector *child = new CurvePrimitiveChildCurveVector (source.m_boundaryType);
    child->m_childCurveVector->swap (source);
    return child;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _Length(double &length) const override
    {
    length = m_childCurveVector->Length ();
    return true;
    }
	
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _Length(RotMatrixCP worldToLocal, double &length) const override
    {
    length = m_childCurveVector->Length (worldToLocal);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _GetRange(DRange3dR range) const override
    {
    return m_childCurveVector->GetRange (range);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _GetRange(DRange3dR range, TransformCR transform) const override
    {
    return m_childCurveVector->GetRange (range, transform);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double _FastMaxAbs() const override
    {
    return m_childCurveVector->FastMaxAbs ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/    
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    CurveVectorCP otherChild = other.GetChildCurveVectorCP ();
    return NULL != otherChild
        && m_childCurveVector->IsSameStructureAndGeometry (*otherChild);
    }    
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _IsValidGeometry(GeometryValidatorPtr &validator) const override
    {
    if (!validator.IsValid ())
        return true;
    return m_childCurveVector != nullptr
        && m_childCurveVector->IsValidGeometry(validator);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t _NumComponent () const override {return 0;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DRange1d _ProjectedParameterRange(DRay3dCR ray, double fractionA, double fractionB) const override
    {
    // ugh.  Fraction doesn't make sense ...
    return m_childCurveVector->ProjectedParameterRange (ray);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DRange1d _ProjectedParameterRange(DRay3dCR ray) const override
    {
    return m_childCurveVector->ProjectedParameterRange (ray);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _TransformInPlace (TransformCR transform) override
    {
    for (size_t i = 0, n = m_childCurveVector->size (); i < n; i++)
        m_childCurveVector->at(i)->TransformInPlace(transform);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _ReverseCurvesInPlace () override
    {
    m_childCurveVector->ReverseCurvesInPlace ();
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void _AppendCurvePlaneIntersections(DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tol) const override 
    {
    AppendTolerancedPlaneIntersections (plane, this, *m_childCurveVector.get (), intersections, tol);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateChildCurveVector_CopyFromSource (CurveVectorCR source)  {return CurvePrimitiveChildCurveVector::Create_CopyFromSource (source);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateChildCurveVector_SwapFromSource (CurveVectorR source)  {return CurvePrimitiveChildCurveVector::Create_SwapFromSource (source);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateChildCurveVector (CurveVectorPtr source)  {return CurvePrimitiveChildCurveVector::Create (source);}



END_BENTLEY_GEOMETRY_NAMESPACE

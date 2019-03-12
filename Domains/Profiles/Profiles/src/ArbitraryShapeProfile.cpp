/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/ArbitraryShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>
#include <ProfilesInternal\ProfilesLogging.h>
#include <Profiles\ArbitraryShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (ArbitraryShapeProfileHandler)

#define SINGLE_PERIMETER_TEST_SCALE 1.01
static bool validateCurveVectorGeometry (CurveVector const& curveVector);
static bool validateCurveVectorClosed (CurveVector const& curveVector);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateCurveVectorArea (CurveVector const& curve)
    {
    DPoint3d centroid;
    double area;
    curve.CentroidAreaXY (centroid, area);
    if (ProfilesProperty::IsEqual (0, area))
        {
        ProfilesLog::FailedValidate_InvalidArea (PRF_CLASS_ArbitraryShapeProfile);
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateCurvePrimitiveArea (ICurvePrimitive const& primitive)
    {
    ICurvePrimitivePtr testPrimitive = primitive.Clone();
    CurveVectorPtr testCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer, testPrimitive);
    return validateCurveVectorArea (*testCurve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateCurvePrimitiveClosed (ICurvePrimitive const& primitive)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == primitive.GetCurvePrimitiveType())
        return validateCurveVectorClosed (*primitive.GetChildCurveVectorCP());

    DPoint3d start, end;
    primitive.GetStartEnd (start, end);
    if (!start.AlmostEqual (end))
        {
        ProfilesLog::FailedValidate_NotClosed (PRF_CLASS_ArbitraryShapeProfile);
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateCurveVectorClosed (CurveVector const& curve)
    {
    if (1 == curve.size())
        return validateCurvePrimitiveClosed (*curve[0]);

    return true;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool groupCurvePrimitives (CurveVectorPtr& resultCurves, CurveVector const& source)
    {
    resultCurves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    DPoint3d start, end, tempStart, tempEnd;
    bool shapeStarted = false;
    CurveVectorPtr tempCurves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    for (size_t i = 0; i < source.size(); ++i)
        {
        ICurvePrimitivePtr const& primitive = source[i];
        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == primitive->GetCurvePrimitiveType() && !shapeStarted)
            {
            CurveVector::BoundaryType type;
            if (!source.GetChildBoundaryType (i, type) || CurveVector::BOUNDARY_TYPE_Inner != type)
                resultCurves->Add (primitive);
            continue;
            }

        primitive->GetStartEnd (tempStart, tempEnd);
        if (!shapeStarted)
            {
            start = tempStart;
            end = tempEnd;
            shapeStarted = true;
            }
        else
            {
            end = tempEnd;
            }

        tempCurves->Add (primitive);
        if (start.AlmostEqual (end))
            {
            resultCurves->Add (tempCurves);
            tempCurves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
            shapeStarted = false;
            }
        }

    return !shapeStarted && !resultCurves->empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool checkIfSinglePerimeterShape (CurveVector const& curveVector)
    {
    CurveVectorPtr groupedCurves;
    if (!groupCurvePrimitives (groupedCurves, curveVector))
        {
        ProfilesLog::FailedValidate_NotSinglePerimeter (PRF_CLASS_ArbitraryShapeProfile);
        return false;
        }

    return ProfilesGeometry::ValidateCurveVectorContinious (*groupedCurves, true, PRF_CLASS_ArbitraryShapeProfile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateCurvePrimitiveGeometry (ICurvePrimitive const& curvePrimitive)
    {
    if (!validateCurvePrimitiveArea (curvePrimitive))
        return false;

    if (!validateCurvePrimitiveClosed (curvePrimitive))
        return false;

    switch (curvePrimitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return true;
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
            return validateCurveVectorGeometry (*curvePrimitive.GetChildCurveVectorCP());
        }

    ProfilesLog::FailedValidate_UnhandledShape (PRF_CLASS_ArbitraryShapeProfile);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateCurveVectorGeometry (CurveVector const& curveVector)
    {    
    if (0 == curveVector.size())
        return false;

    if (1 == curveVector.size() && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == curveVector[0]->GetCurvePrimitiveType())
        return validateCurveVectorGeometry (*curveVector[0]->GetChildCurveVectorCP());
    
    for (ICurvePrimitivePtr const& primitive : curveVector)
        {
        switch (primitive->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                continue;
            default:
                ProfilesLog::FailedValidate_UnhandledShape (PRF_CLASS_ArbitraryShapeProfile);
                return false;
            }
        }

    if (!curveVector.IsAnyRegionType())
        {
        ProfilesLog::FailedValidate_ShapeIsNotRegion (PRF_CLASS_ArbitraryShapeProfile);
        return false;
        }

    return  validateCurveVectorArea(curveVector) && validateCurveVectorClosed(curveVector) && checkIfSinglePerimeterShape(curveVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryShapeProfile::CreateParams::CreateParams (DgnModel const& model, DgnClassId const& classId, Utf8CP pName, IGeometryPtr const& geometryPtr)
    : T_Super (model, classId, pName)
    , geometryPtr (geometryPtr)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryShapeProfile::CreateParams::CreateParams (DgnModel const& model, Utf8CP pName, IGeometryPtr const& geometryPtr)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , geometryPtr (geometryPtr)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryShapeProfile::ArbitraryShapeProfile (CreateParams const& params)
    : T_Super (params)
    , m_geometryPtr (nullptr)
    {
    if (params.m_isLoadingElement)
        return;

    m_geometryPtr = params.geometryPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArbitraryShapeProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    if (m_geometryPtr.IsNull())
        return false;

    if (!ProfilesGeometry::ValidateRangeXY(*m_geometryPtr, PRF_CLASS_ArbitraryShapeProfile))
        return false;
    
    switch (m_geometryPtr->GetGeometryType())
        {
        case IGeometry::GeometryType::CurveVector:
          {
          CurveVectorPtr asCurveVector = m_geometryPtr->GetAsCurveVector();
          BeAssert (asCurveVector.IsValid());
          return validateCurveVectorGeometry (*asCurveVector);
          }
        case IGeometry::GeometryType::CurvePrimitive:
          {
          ICurvePrimitivePtr asCurvePrimitive = m_geometryPtr->GetAsICurvePrimitive();
          BeAssert (asCurvePrimitive.IsValid());
          return validateCurvePrimitiveGeometry (*asCurvePrimitive);
          }
        default:
            ProfilesLog::FailedValidate_UnhandledShape (PRF_CLASS_ArbitraryShapeProfile);
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ArbitraryShapeProfile::_CreateShapeGeometry() const
    {
    return m_geometryPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryShapeProfile::_CopyFrom (DgnElement const& source)
    {
    if (auto const* pSourceProfile = dynamic_cast<ArbitraryShapeProfile const*> (&source))
        m_geometryPtr = pSourceProfile->m_geometryPtr;
    else
        ProfilesLog::FailedCopyFrom_InvalidElement (PRF_CLASS_ArbitraryShapeProfile, m_elementId, source.GetElementId());

    return T_Super::_CopyFrom (source);
    }

END_BENTLEY_PROFILES_NAMESPACE

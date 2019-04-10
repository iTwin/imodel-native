/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/ArbitraryCenterLineProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesLogging.h>
#include <ProfilesInternal\ProfilesProperty.h>
#include <Profiles\ArbitraryCenterLineProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (ArbitraryCenterLineProfileHandler)

static bool validateCurvePrimitiveGeometry (ICurvePrimitive const& curvePrimitive);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateLineStringNotSelfIntersecting (bvector<DPoint3d> const& lineString)
    {
    for (size_t i = 0; i < lineString.size() - 1; ++i)
        {
        for (size_t j = i + 1; j < lineString.size() - 1; ++j)
            {
            DSegment3d const line1 = { lineString[i], lineString[i + 1] };
            DSegment3d const line2 = { lineString[j], lineString[j + 1] };
            double fractionA, fractionB;
            DPoint3d intersectionPoint1, intersectionPoint2;
            if (!line1.IntersectXY (fractionA, fractionB, intersectionPoint1, intersectionPoint2, line1, line2))
                continue;

            if (!intersectionPoint1.AlmostEqual (intersectionPoint2))
                {
                ProfilesLog::FailedValidate_SelfIntersecting (PRF_CLASS_ArbitraryCenterLineProfile);
                return false;
                }

            bool const intersectionOnEndPointA = lineString[i].AlmostEqual (intersectionPoint1) || lineString[i + 1].AlmostEqual (intersectionPoint1);
            bool const intersectionOnEndPointB = lineString[j].AlmostEqual (intersectionPoint1) || lineString[j + 1].AlmostEqual (intersectionPoint1);
            if (!intersectionOnEndPointA || !intersectionOnEndPointB)
                {
                ProfilesLog::FailedValidate_SelfIntersecting (PRF_CLASS_ArbitraryCenterLineProfile);
                return false;
                }
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateCurveVectorNotSelfIntersecting (CurveVector const& curveVector)
    {
    CurveVectorPtr selfIntersectionsA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr selfIntersectionsB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr curve = curveVector.Clone();
    CurveCurve::SelfIntersectionsXY (*selfIntersectionsA, *selfIntersectionsB, *curve, nullptr);

    for (size_t i = 0; i < selfIntersectionsA->size(); ++i)
        {
        PartialCurveDetailCP const& partialA = (*selfIntersectionsA)[i]->GetPartialCurveDetailCP();
        PartialCurveDetailCP const& partialB = (*selfIntersectionsB)[i]->GetPartialCurveDetailCP();
        
        // If intersection is not a single point, return false
        if (!partialA->IsSingleFraction() || !partialB->IsSingleFraction())
            {
            ProfilesLog::FailedValidate_SelfIntersecting (PRF_CLASS_ArbitraryCenterLineProfile);
            return false;
            }

        // If intersection is not an end point of both primitives, return false
        DPoint3d startA, endA, startB, endB;
        partialA->parentCurve->GetStartEnd (startA, endA);
        partialB->parentCurve->GetStartEnd (startB, endB);

        DPoint3d intersectionPoint;
        partialA->parentCurve->FractionToPoint (partialA->fraction0, intersectionPoint);

        bool const intersectionOnEndPointA = startA.AlmostEqual(intersectionPoint) || endA.AlmostEqual(intersectionPoint);
        bool const intersectionOnEndPointB = startB.AlmostEqual(intersectionPoint) || endB.AlmostEqual(intersectionPoint);
        if (!intersectionOnEndPointA || !intersectionOnEndPointB)
            {
            ProfilesLog::FailedValidate_SelfIntersecting (PRF_CLASS_ArbitraryCenterLineProfile);
            return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateCurveVectorGeometry (CurveVector const& curveVector)
    {
    for (ICurvePrimitivePtr const& primitive : curveVector)
        {
        if (!validateCurvePrimitiveGeometry (*primitive))
            return false;
        }

    if (!ProfilesGeometry::ValidateCurveVectorContinious (curveVector, false, PRF_CLASS_ArbitraryCenterLineProfile))
        return false;

    return validateCurveVectorNotSelfIntersecting (curveVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateLineStringGeometry (bvector<DPoint3d> const& lineString)
    {
    CurveVectorPtr lineStringAsCurveVector = CurveVector::CreateLinear (lineString);
    return ProfilesGeometry::ValidateCurveVectorContinious (*lineStringAsCurveVector, false, PRF_CLASS_ArbitraryCenterLineProfile) && 
        validateLineStringNotSelfIntersecting (lineString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateCurvePrimitiveGeometry (ICurvePrimitive const& curvePrimitive)
    {
    switch (curvePrimitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            return true;
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return validateLineStringGeometry (*curvePrimitive.GetLineStringCP());
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
            return validateCurveVectorGeometry (*curvePrimitive.GetChildCurveVectorCP());
        default:
            ProfilesLog::FailedValidate_UnhandledShape (PRF_CLASS_ArbitraryCenterLineProfile);
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCenterLineProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName, IGeometryPtr const& geometryPtr, 
                                                        double wallThickness, Angle const& arcAngle, Angle const& chamferAngle)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName, geometryPtr)
    , wallThickness (wallThickness)
    , arcAngle (arcAngle)
    , chamferAngle (chamferAngle)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCenterLineProfile::ArbitraryCenterLineProfile (CreateParams const& params)
    : T_Super (params)
    , m_arcAngle (params.arcAngle)
    , m_chamferAngle (params.chamferAngle)
    {
    if (params.m_isLoadingElement)
        return;

    SetWallThickness (params.wallThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArbitraryCenterLineProfile::_Validate() const
    {
    if (!SinglePerimeterProfile::_Validate()) // Skip ArbitraryShapeProfile::_Validate because shape geometry hasn't been created yet
        return false;

    BeAssert (m_geometryPtr.IsValid() && "Null geometry should be handled in T_Super::_Validate()");
    if (!ProfilesProperty::IsGreaterThanZero (GetWallThickness()))
        return false;

    if (!ProfilesGeometry::ValidateRangeXY (*m_geometryPtr, PRF_CLASS_ArbitraryCenterLineProfile))
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
            ProfilesLog::FailedValidate_UnhandledShape (PRF_CLASS_ArbitraryCenterLineProfile);
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArbitraryCenterLineProfile::_CreateGeometry()
    {
    if (!T_Super::_CreateGeometry())
        return false;

    if (m_geometryPtr.IsValid())
        SetCenterLine (*m_geometryPtr);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ArbitraryCenterLineProfile::_CreateShapeGeometry() const
    {
    BeAssert (m_geometryPtr.IsValid() && "Null geometry should be handled in _Validate()");
    return ProfilesGeometry::CreateArbitraryCenterLineShape (*m_geometryPtr, GetWallThickness(), m_arcAngle, m_chamferAngle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryCenterLineProfile::_CopyFrom (Dgn::DgnElement const & source, CopyFromOptions const& opts)
    {
    ArbitraryCenterLineProfileCP asProfile = dynamic_cast<ArbitraryCenterLineProfileCP>(&source);
    if (nullptr == asProfile)
        {
        ProfilesLog::FailedCopyFrom_InvalidElement (PRF_CLASS_ArbitraryCenterLineProfile, GetElementId(), source.GetElementId());
        return;
        }
    m_arcAngle = asProfile->m_arcAngle;
    m_chamferAngle = asProfile->m_chamferAngle;

    return T_Super::_CopyFrom (source, opts);
    }

END_BENTLEY_PROFILES_NAMESPACE

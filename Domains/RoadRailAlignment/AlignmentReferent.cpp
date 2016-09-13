/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentReferent.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

HANDLER_DEFINE_MEMBERS(AlignmentReferentHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentReferent::AlignmentReferent(CreateParams const& params, AlignmentCR alignment, DistanceExpressionCR distanceExpression, double restartValue) :
    T_Super(params), m_restartValue(restartValue), ILinearlyLocatedElement(alignment.GetElementId()) 
    {
    _AddLinearlyReferencedLocation(*LinearlyReferencedAtLocation::Create(distanceExpression));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentReferentPtr AlignmentReferent::Create(AlignmentCR alignment, DistanceExpressionCR distanceExpression, double restartValue)
    {
    if (!alignment.GetModelId().IsValid())
        return nullptr;

    return new AlignmentReferent(CreateParams(alignment.GetDgnDb(), alignment.GetModelId(), QueryClassId(alignment.GetDgnDb()),
        RoadRailAlignmentDomain::QueryAlignmentCategoryId(alignment.GetDgnDb())), alignment, distanceExpression, restartValue);
    }
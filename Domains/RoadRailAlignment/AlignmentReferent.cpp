/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentReferent.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

HANDLER_DEFINE_MEMBERS(AlignmentReferentElementHandler)
HANDLER_DEFINE_MEMBERS(AlignmentStationHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentReferentElement::AlignmentReferentElement(CreateParams const& params, DistanceExpressionCR distanceExpression, double restartValue) :
    T_Super(params), m_restartValue(restartValue), ILinearlyLocatedElement() 
    {
    _AddLinearlyReferencedLocation(*LinearlyReferencedAtLocation::Create(distanceExpression));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentStationPtr AlignmentStation::Create(AlignmentCR alignment, DistanceExpressionCR distanceExpression, double restartValue)
    {
    if (!alignment.GetModelId().IsValid())
        return nullptr;

    AlignmentStationPtr retVal(new AlignmentStation(CreateParams(alignment.GetDgnDb(), alignment.GetModelId(), QueryClassId(alignment.GetDgnDb()),
        RoadRailAlignmentDomain::QueryAlignmentCategoryId(alignment.GetDgnDb())), distanceExpression, restartValue));

    retVal->_SetLinearElementId(alignment.GetElementId());

    return retVal;
    }
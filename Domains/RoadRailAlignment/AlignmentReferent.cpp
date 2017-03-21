/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentReferent.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

HANDLER_DEFINE_MEMBERS(AlignmentReferentElementHandler)
HANDLER_DEFINE_MEMBERS(AlignmentStationHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentStation::AlignmentStation(CreateParams const& params) :
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentStation::AlignmentStation(CreateParams const& params, DistanceExpressionCR distanceExpression, double restartValue) :
    T_Super(params, restartValue)
    {
    m_unpersistedAtLocation = LinearlyReferencedAtLocation::Create(distanceExpression);
    _AddLinearlyReferencedLocation(*m_unpersistedAtLocation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentStationPtr AlignmentStation::Create(AlignmentCR alignment, DistanceExpressionCR distanceExpression, double restartValue)
    {
    if (!alignment.GetModelId().IsValid() || !alignment.GetElementId().IsValid())
        return nullptr;

    CreateParams params(alignment.GetDgnDb(), alignment.GetModelId(), QueryClassId(alignment.GetDgnDb()), AlignmentCategory::Get(alignment.GetDgnDb()));
    params.SetParentId(alignment.GetElementId(), DgnClassId(alignment.GetDgnDb().Schemas().GetClassId(BRRA_SCHEMA_NAME, BRRA_REL_AlignmentOwnsStations)));

    AlignmentStationPtr retVal(new AlignmentStation(params, distanceExpression, restartValue));
    retVal->_SetLinearElementId(alignment.GetElementId());
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceExpressionP AlignmentStation::GetAtPositionP()
    {
    if (!GetElementId().IsValid())
        return &m_unpersistedAtLocation->GetAtPositionR();

    if (!m_atLocationAspectId.IsValid())
        {
        auto aspectIds = QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_atLocationAspectId = aspectIds.front();
        }

    auto locationP = GetLinearlyReferencedAtLocationP(m_atLocationAspectId);
    BeAssert(locationP);

    return &locationP->GetAtPositionR();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceExpressionCR AlignmentStation::GetAtPosition() const
    {
    return *const_cast<AlignmentStationP>(this)->GetAtPositionP();
    }
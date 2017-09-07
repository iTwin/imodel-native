/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentReferent.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentReferent.h>

HANDLER_DEFINE_MEMBERS(AlignmentReferentElementHandler)
HANDLER_DEFINE_MEMBERS(AlignmentStationHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentReferentElement::AlignmentReferentElement(CreateParams const& params, double distanceAlong) :
    T_Super(params), ILinearlyLocatedSingleAt(distanceAlong)
    {
    }

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
AlignmentStation::AlignmentStation(CreateParams const& params, double distanceAlong, double station) :
    T_Super(params, distanceAlong)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedAtLocation());
    SetStation(station);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentStationPtr AlignmentStation::Create(AlignmentCR alignment, double distanceAlong, double restartValue)
    {
    if (!alignment.GetModelId().IsValid() || !alignment.GetElementId().IsValid())
        return nullptr;

    CreateParams params(alignment.GetDgnDb(), alignment.GetModelId(), QueryClassId(alignment.GetDgnDb()), AlignmentCategory::Get(alignment.GetDgnDb()));
    params.SetParentId(alignment.GetElementId(), DgnClassId(alignment.GetDgnDb().Schemas().GetClassId(BRRA_SCHEMA_NAME, BRRA_REL_AlignmentOwnsStations)));

    AlignmentStationPtr retVal(new AlignmentStation(params, distanceAlong, restartValue));
    retVal->_SetLinearElement(alignment.GetElementId());
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentStationingTranslator::AlignmentStationingTranslator(AlignmentCR alignment)
    {
    m_stations = alignment.QueryOrderedStations();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentStationingTranslatorPtr AlignmentStationingTranslator::Create(AlignmentCR alignment)
    {
    return new AlignmentStationingTranslator(alignment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NullableDouble AlignmentStationingTranslator::ToStation(double distanceAlongFromStart) const
    {
    if (distanceAlongFromStart < m_stations.front().GetDistanceAlong() ||
        distanceAlongFromStart > m_stations.back().GetDistanceAlong())
        return NullableDouble();

    size_t index = (m_stations.size() / 2) - 1;
    size_t scope = MAX(index, 1);

    do
        {
        scope = MAX(scope / 2, 1);
        if (m_stations[index].GetDistanceAlong() <= distanceAlongFromStart &&
            (index + 1 == m_stations.size() || m_stations[index + 1].GetDistanceAlong() > distanceAlongFromStart))
            return m_stations[index].GetStation() + (distanceAlongFromStart - m_stations[index].GetDistanceAlong());
        else if (m_stations[index].GetDistanceAlong() > distanceAlongFromStart)
            index -= scope;
        else
            index += scope;
        } while (true);

    BeAssert(false);
    return NullableDouble();
    }
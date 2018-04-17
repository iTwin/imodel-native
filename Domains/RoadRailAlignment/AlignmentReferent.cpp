/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentReferent.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentReferent.h>

HANDLER_DEFINE_MEMBERS(LinearlyLocatedReferentElementHandler)
HANDLER_DEFINE_MEMBERS(AlignmentStationHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyLocatedReferentElement::LinearlyLocatedReferentElement(CreateParams const& params, double distanceAlong) :
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
AlignmentStation::AlignmentStation(CreateParams const& params, double distanceAlongFromStart, double station) :
    T_Super(params, distanceAlongFromStart)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedAtLocation());
    SetStation(station);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentStationPtr AlignmentStation::Create(AlignmentCR alignment, double distanceAlongFromStart, double station)
    {
    if (!alignment.GetModelId().IsValid() || !alignment.GetElementId().IsValid())
        return nullptr;

    CreateParams params(alignment.GetDgnDb(), alignment.GetModelId(), QueryClassId(alignment.GetDgnDb()), AlignmentCategory::GetAlignment(*alignment.GetAlignmentModel()->GetParentSubject()));
    params.SetParentId(alignment.GetElementId(), DgnClassId(alignment.GetDgnDb().Schemas().GetClassId(BRRA_SCHEMA_NAME, BRRA_REL_AlignmentOwnsReferents)));

    AlignmentStationPtr retVal(new AlignmentStation(params, distanceAlongFromStart, station));
    retVal->_SetLinearElement(alignment.GetElementId());
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentStationingTranslator::AlignmentStationingTranslator(AlignmentCR alignment)
    {
    m_stations = alignment.QueryOrderedStations();
    m_length = alignment.GetLength();
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
    if (distanceAlongFromStart < m_stations.front().GetDistanceAlongFromStart() ||
        distanceAlongFromStart > m_stations.back().GetDistanceAlongFromStart())
        return NullableDouble();

    size_t index = (m_stations.size() / 2) - 1;
    size_t scope = MAX(index, 1);

    do
        {
        scope = MAX(scope / 2, 1);
        if (m_stations[index].GetDistanceAlongFromStart() <= distanceAlongFromStart &&
            (index + 1 == m_stations.size() || m_stations[index + 1].GetDistanceAlongFromStart() > distanceAlongFromStart))
            return m_stations[index].GetStation() + (distanceAlongFromStart - m_stations[index].GetDistanceAlongFromStart());
        else if (m_stations[index].GetDistanceAlongFromStart() > distanceAlongFromStart)
            index -= scope;
        else
            index += scope;
        } while (true);

    BeAssert(false);
    return NullableDouble();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NullableDouble AlignmentStationingTranslator::ToDistanceAlongFromStart(double station) const
    {
    if (m_stations.empty())
        return NullableDouble();

    auto curStation = m_stations.begin();
    do
        {
        if (curStation->GetStation() > station)
            continue;

        auto nextStation = curStation + 1;
        double nextDistanceAlong = (nextStation == m_stations.end()) ? m_length : nextStation->GetDistanceAlongFromStart();
        double endStation = curStation->GetStation() + (nextDistanceAlong - curStation->GetDistanceAlongFromStart());
        if (endStation >= station)
            return (station - curStation->GetStation() + curStation->GetDistanceAlongFromStart());

        } while ((++curStation) != m_stations.end());

    return NullableDouble();
    }
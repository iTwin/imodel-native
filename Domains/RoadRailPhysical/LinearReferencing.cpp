/*--------------------------------------------------------------------------------------+
|
|     $Source: LinearReferencing.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearlyLocatedSingleFromTo::ILinearlyLocatedSingleFromTo(double fromDistanceAlong, double toDistanceAlong)
    {
    m_unpersistedFromToLocationPtr = LinearlyReferencedFromToLocation::Create(DistanceExpression(fromDistanceAlong), DistanceExpression(toDistanceAlong));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double ILinearlyLocatedSingleFromTo::GetFromDistanceAlong() const
    {
    if (!ToElement().GetElementId().IsValid())
        return m_unpersistedFromToLocationPtr->GetFromPosition().GetDistanceAlongFromStart();

    if (!m_fromToLocationAspectId.IsValid())
        {
        auto aspectIds = ToLinearlyLocated().QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_fromToLocationAspectId = aspectIds.front();
        }

    auto locationCP = ToLinearlyLocated().GetLinearlyReferencedFromToLocation(m_fromToLocationAspectId);
    BeAssert(locationCP);

    return locationCP->GetFromPosition().GetDistanceAlongFromStart();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearlyLocatedSingleFromTo::SetFromDistanceAlong(double newFrom)
    {
    if (!ToElement().GetElementId().IsValid())
        {
        m_unpersistedFromToLocationPtr->GetFromPositionR().SetDistanceAlongFromStart(newFrom);
        return;
        }

    if (!m_fromToLocationAspectId.IsValid())
        {
        auto aspectIds = ToLinearlyLocated().QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_fromToLocationAspectId = aspectIds.front();
        }

    auto locationP = ToLinearlyLocatedR().GetLinearlyReferencedFromToLocationP(m_fromToLocationAspectId);
    BeAssert(locationP);

    return locationP->GetFromPositionR().SetDistanceAlongFromStart(newFrom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double ILinearlyLocatedSingleFromTo::GetToDistanceAlong() const
    {
    if (!ToElement().GetElementId().IsValid())
        return m_unpersistedFromToLocationPtr->GetToPosition().GetDistanceAlongFromStart();

    if (!m_fromToLocationAspectId.IsValid())
        {
        auto aspectIds = ToLinearlyLocated().QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_fromToLocationAspectId = aspectIds.front();
        }

    auto locationCP = ToLinearlyLocated().GetLinearlyReferencedFromToLocation(m_fromToLocationAspectId);
    BeAssert(locationCP);

    return locationCP->GetToPosition().GetDistanceAlongFromStart();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearlyLocatedSingleFromTo::SetToDistanceAlong(double newFrom)
    {
    if (!ToElement().GetElementId().IsValid())
        {
        m_unpersistedFromToLocationPtr->GetToPositionR().SetDistanceAlongFromStart(newFrom);
        return;
        }

    if (!m_fromToLocationAspectId.IsValid())
        {
        auto aspectIds = ToLinearlyLocated().QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_fromToLocationAspectId = aspectIds.front();
        }

    auto locationP = ToLinearlyLocatedR().GetLinearlyReferencedFromToLocationP(m_fromToLocationAspectId);
    BeAssert(locationP);

    return locationP->GetToPositionR().SetDistanceAlongFromStart(newFrom);
    }
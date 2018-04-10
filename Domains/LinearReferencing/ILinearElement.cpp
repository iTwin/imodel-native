/*--------------------------------------------------------------------------------------+
|
|     $Source: ILinearElement.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"
#include <LinearReferencing/ILinearElement.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearElement::SetILinearElementSource(ILinearElementSourceCP linearElementSource, DgnClassId relClassId)
    {
    if (linearElementSource)
        {
        if (!relClassId.IsValid())
            relClassId = ToElement().GetDgnDb().Schemas().GetClassId(BLR_SCHEMA_NAME, BLR_REL_ILinearElementSourceProvidesILinearElements);

        ToElementR().SetPropertyValue("ILinearElementSource", linearElementSource->ToElement().GetElementId(), relClassId);
        }
    else
        ToElementR().SetPropertyValue("ILinearElementSource", DgnElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bset<DgnElementId> ILinearElementSource::QueryLinearElements(ECRelationshipClassCP relClass) const
    {
    Utf8String relClassName;
    if (relClass)
        {
        relClassName = relClass->GetSchema().GetName();
        relClassName.append(".");
        relClassName.append(relClass->GetName());
        }
    else
        relClassName = BLR_SCHEMA(BLR_REL_ILinearElementSourceProvidesILinearElements);

    auto ecsql = Utf8PrintfString("SELECT TargetECInstanceId FROM %s WHERE SourceECInstanceId = ?", relClassName.c_str());

    ECSqlStatement stmt;
    stmt.Prepare(ToElement().GetDgnDb(), ecsql.c_str());
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, ToElement().GetElementId());

    bset<DgnElementId> retVal;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        retVal.insert(stmt.GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearlyLocated::ILinearlyLocated()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearlyLocated::_SetLinearElement(DgnElementId elementId)
    {
    DgnDbStatus status = ToElementR().SetPropertyValue(BLR_PROP_ILinearlyLocated_ILinearElement, elementId, 
        DgnClassId(ToElement().GetDgnDb().Schemas().GetClassId(BLR_SCHEMA_NAME, BLR_REL_ILinearlyLocatedAlongILinearElement)));
    BeAssert(DgnDbStatus::Success == status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ILinearlyLocated::GetLinearElementId() const 
    {
    return ToElement().GetPropertyValueId<DgnElementId>(BLR_PROP_ILinearlyLocated_ILinearElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearElementCP ILinearlyLocated::GetLinearElement() const
    {
    return dynamic_cast<ILinearElementCP>(ToElement().GetDgnDb().Elements().GetElement(GetLinearElementId()).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<LinearlyReferencedLocationId> ILinearlyLocated::QueryLinearlyReferencedLocationIds() const
    {
    bvector<LinearlyReferencedLocationId> retVal;

    auto stmtPtr = ToElement().GetDgnDb().GetPreparedECSqlStatement(
        "SELECT ECInstanceId FROM " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedLocation) " WHERE Element.Id = ? ORDER BY ECInstanceId;");

    stmtPtr->BindId(1, ToElement().GetElementId());

    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        retVal.push_back(stmtPtr->GetValueId<LinearlyReferencedLocationId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedLocationCP ILinearlyLocated::GetLinearlyReferencedLocation(LinearlyReferencedLocationId id) const
    {
    LinearlyReferencedLocationCP retVal = GetLinearlyReferencedAtLocation(id);
    if (!retVal)
        retVal = GetLinearlyReferencedFromToLocation(id);

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedLocationP ILinearlyLocated::GetLinearlyReferencedLocationP(LinearlyReferencedLocationId id)
    {
    LinearlyReferencedLocationP retVal = GetLinearlyReferencedAtLocationP(id);
    if (!retVal)
        retVal = GetLinearlyReferencedFromToLocationP(id);

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocationCP ILinearlyLocated::GetLinearlyReferencedAtLocation(LinearlyReferencedLocationId id) const
    {
    return dynamic_cast<LinearlyReferencedAtLocationCP>(
        DgnElement::MultiAspect::GetAspect(ToElement(), *LinearlyReferencedAtLocation::QueryClass(ToElement().GetDgnDb()), id));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocationP ILinearlyLocated::GetLinearlyReferencedAtLocationP(LinearlyReferencedLocationId id)
    {
    return DgnElement::MultiAspect::GetP<LinearlyReferencedAtLocation>(ToElementR(), *LinearlyReferencedAtLocation::QueryClass(ToElement().GetDgnDb()), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationCP ILinearlyLocated::GetLinearlyReferencedFromToLocation(LinearlyReferencedLocationId id) const
    {
    return dynamic_cast<LinearlyReferencedFromToLocationCP>(
        DgnElement::MultiAspect::GetAspect(ToElement(), *LinearlyReferencedFromToLocation::QueryClass(ToElement().GetDgnDb()), id));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationP ILinearlyLocated::GetLinearlyReferencedFromToLocationP(LinearlyReferencedLocationId id)
    {
    return DgnElement::MultiAspect::GetP<LinearlyReferencedFromToLocation>(ToElementR(), *LinearlyReferencedFromToLocation::QueryClass(ToElement().GetDgnDb()), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearlyLocated::_AddLinearlyReferencedLocation(LinearlyReferencedLocationR location)
    {
    DgnElement::MultiAspect::AddAspect(ToElementR(), location);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearlyLocatedAttribution::ILinearlyLocatedAttribution()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearlyLocatedElement::ILinearlyLocatedElement()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearlyLocatedSingleAt::ILinearlyLocatedSingleAt(double atDistanceAlong)
    {
    m_unpersistedAtLocationPtr = LinearlyReferencedAtLocation::Create(DistanceExpression(atDistanceAlong));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double ILinearlyLocatedSingleAt::GetAtDistanceAlongFromStart() const
    {
    if (!ToElement().GetElementId().IsValid())
        return m_unpersistedAtLocationPtr->GetAtPosition().GetDistanceAlongFromStart();

    if (!m_atLocationAspectId.IsValid())
        {
        auto aspectIds = ToLinearlyLocated().QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_atLocationAspectId = aspectIds.front();
        }

    auto locationCP = ToLinearlyLocated().GetLinearlyReferencedAtLocation(m_atLocationAspectId);
    BeAssert(locationCP);

    return locationCP->GetAtPosition().GetDistanceAlongFromStart();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearlyLocatedSingleAt::SetAtDistanceAlongFromStart(double newAt)
    {
    if (!ToElement().GetElementId().IsValid())
        {
        m_unpersistedAtLocationPtr->GetAtPositionR().SetDistanceAlongFromStart(newAt);
        return;
        }

    if (!m_atLocationAspectId.IsValid())
        {
        auto aspectIds = ToLinearlyLocated().QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_atLocationAspectId = aspectIds.front();
        }

    auto locationP = ToLinearlyLocatedR().GetLinearlyReferencedAtLocationP(m_atLocationAspectId);
    BeAssert(locationP);

    return locationP->GetAtPositionR().SetDistanceAlongFromStart(newAt);
    }

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
double ILinearlyLocatedSingleFromTo::GetFromDistanceAlongFromStart() const
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
void ILinearlyLocatedSingleFromTo::SetFromDistanceAlongFromStart(double newFrom)
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
double ILinearlyLocatedSingleFromTo::GetToDistanceAlongFromStart() const
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
void ILinearlyLocatedSingleFromTo::SetToDistanceAlongFromStart(double newFrom)
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

/*--------------------------------------------------------------------------------------+
|
|     $Source: ILinearElement.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"
#include <LinearReferencing/ILinearElement.h>

HANDLER_DEFINE_MEMBERS(GeometricElementAsReferentHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearElement::SetILinearElementSource(ILinearElementSourceCP linearElementSource)
    {
    if (linearElementSource)
        ToElementR().SetPropertyValue("ILinearElementSource", linearElementSource->ToElement().GetElementId(),
            ToElement().GetDgnDb().Schemas().GetClassId(BLR_SCHEMA_NAME, BLR_REL_ILinearElementSourceProvidesILinearElements));
    else
        ToElementR().SetPropertyValue("ILinearElementSource", DgnElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bset<DgnElementId> ILinearElementSource::QueryLinearElements() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(ToElement().GetDgnDb(), "SELECT TargetECInstanceId FROM " BLR_SCHEMA(BLR_REL_ILinearElementSourceProvidesILinearElements) 
        " WHERE SourceECInstanceId = ?");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, ToElement().GetElementId());

    bset<DgnElementId> retVal;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        retVal.insert(stmt.GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ILinearElementSource::_PrepareCascadeChanges(ICascadeLinearLocationChangesAlgorithmR algorithm) const
    {
    return algorithm.Prepare(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ILinearElementSource::_CommitCascadeChanges(ICascadeLinearLocationChangesAlgorithmR algorithm) const
    {
    return algorithm.Commit(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearlyLocated::ILinearlyLocated():
    m_cascadeLocationChangesFlag(CascadeLocationChangesAction::None)
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
    auto retValP = DgnElement::MultiAspect::GetP<LinearlyReferencedAtLocation>(ToElementR(), *LinearlyReferencedAtLocation::QueryClass(ToElement().GetDgnDb()), id);

    // Keeping track of accessed locationIds - needed while determining changes during cascade to neighbors
    if (retValP && m_accessedAtLocationIds.find(id) == m_accessedAtLocationIds.end())
        m_accessedAtLocationIds.insert(id);
        
    return retValP;
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
    auto retValP = DgnElement::MultiAspect::GetP<LinearlyReferencedFromToLocation>(ToElementR(), *LinearlyReferencedFromToLocation::QueryClass(ToElement().GetDgnDb()), id);

    // Keeping track of accessed locationIds - needed while determining changes during cascade to neighbors
    if (retValP && m_accessedFromToLocationIds.find(id) == m_accessedFromToLocationIds.end())
        m_accessedFromToLocationIds.insert(id);
        
    return retValP;
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
double ILinearlyLocatedSingleAt::GetAtDistanceAlong() const
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
void ILinearlyLocatedSingleAt::SetAtDistanceAlong(double newAt)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId GeometricElementAsReferent::QueryGeometricElementId() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement(
        "SELECT SourceECInstanceId FROM " BLR_SCHEMA(BLR_REL_GeometricElementDrivesReferent)
        " WHERE TargetECInstanceId = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnElementId();

    return stmtPtr->GetValueId<DgnElementId>(0);
    }
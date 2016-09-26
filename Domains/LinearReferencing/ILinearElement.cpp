/*--------------------------------------------------------------------------------------+
|
|     $Source: ILinearElement.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <LinearReferencingInternal.h>

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
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearlyLocated::_SetLinearElementId(DgnElementId linearElementId)
    {
    if (linearElementId.IsValid())
        ToElementR().SetPropertyValue(BLR_PROP_ILinearlyLocated_ILinearElement, ECValue(linearElementId.GetValue()));
    else
        {
        ECValue val; val.SetToNull();
        ToElementR().SetPropertyValue(BLR_PROP_ILinearlyLocated_ILinearElement, val);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ILinearlyLocated::GetLinearElementId() const 
    {
    ECValue val; 
    ToElement().GetPropertyValue(val, BLR_PROP_ILinearlyLocated_ILinearElement);
    return DgnElementId((uint64_t)val.GetLong());
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
        "SELECT ECInstanceId FROM " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedLocation) " WHERE ElementId = ? ORDER BY ECInstanceId;");

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
    return const_cast<ILinearlyLocatedP>(this)->GetLinearlyReferencedLocationP(id);
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
    return const_cast<ILinearlyLocatedP>(this)->GetLinearlyReferencedAtLocationP(id);
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
    return const_cast<ILinearlyLocatedP>(this)->GetLinearlyReferencedFromToLocationP(id);
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

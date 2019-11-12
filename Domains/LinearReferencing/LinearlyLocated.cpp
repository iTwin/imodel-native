/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"
#include <LinearReferencing/LinearlyLocated.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LinearLocation::LinearLocation(SpatialLocationElementR element, DgnElementCR locatedElement, CreateAtParams const& atParams) :
    T_Super(element), ILinearlyLocatedSingleAt(atParams)
    {
    _SetLinearElement(atParams.m_linearElementCPtr->GetElementId());
    _SetLocatedElement(locatedElement.GetElementId());
    _AddLinearlyReferencedLocation(*_GetUnpersistedAtLocation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LinearLocationPtr LinearLocation::Create(DgnElementCR locatedElement, DgnCategoryId const& categoryId, CreateAtParams const& atParams)
    {
    auto& dgnDbR = locatedElement.GetDgnDb();
    SpatialLocationElement::CreateParams params(dgnDbR, locatedElement.GetModelId(), QueryClassId(dgnDbR), categoryId);

    return new LinearLocation(*Create(dgnDbR, params), locatedElement, atParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LinearLocation::LinearLocation(SpatialLocationElementR element, DgnElementCR locatedElement, CreateFromToParams const& fromToParams) :
    T_Super(element), ILinearlyLocatedSingleFromTo(fromToParams)
    {
    _SetLinearElement(fromToParams.m_linearElementCPtr->GetElementId());
    _SetLocatedElement(locatedElement.GetElementId());
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LinearLocationPtr LinearLocation::Create(DgnElementCR locatedElement, DgnCategoryId const& categoryId, CreateFromToParams const& fromToParams)
    {
    auto& dgnDbR = locatedElement.GetDgnDb();
    SpatialLocationElement::CreateParams params(dgnDbR, locatedElement.GetModelId(), QueryClassId(dgnDbR), categoryId);

    return new LinearLocation(*Create(dgnDbR, params), locatedElement, fromToParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LinearLocationCPtr LinearLocation::Insert(DgnDbStatus* stat)
    {
    LinearLocationCPtr retCPtr = new LinearLocation(*getP()->GetDgnDb().Elements().Insert<SpatialLocationElement>(*getP(), stat)); 

    DgnDbStatus status = Dgn::DgnDbStatus::Success;
    if (retCPtr.IsNull() || 
        DgnDbStatus::Success != (status = _InsertLinearElementRelationship()) ||
        DgnDbStatus::Success != (status = _InsertLocatedElementRelationship()))
        { 
        if (stat) *stat = status; 
        return nullptr; 
        } 
    
    return retCPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LinearLocationCPtr LinearLocation::Update(DgnDbStatus* stat)
    {
    LinearLocationCPtr retCPtr = new LinearLocation(*getP()->GetDgnDb().Elements().Update<SpatialLocationElement>(*getP(), stat));

    DgnDbStatus status = Dgn::DgnDbStatus::Success;
    if (retCPtr.IsNull() ||
        DgnDbStatus::Success != (status = _UpdateLinearElementRelationship()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retCPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearLocation::_InsertLocatedElementRelationship()
    {
    if (!m_cachedLocatedElementId.IsValid())
        return DgnDbStatus::InvalidId;

    auto& elCR = ToElement();
    auto relClassCP = elCR.GetDgnDb().Schemas().GetClass(BLR_SCHEMA_NAME, BLR_REL_ILinearLocationLocatesElement)->GetRelationshipClassCP();

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != elCR.GetDgnDb().InsertLinkTableRelationship(insKey,
                                                                              *relClassCP,
                                                                              ECInstanceId(elCR.GetElementId().GetValue()),
                                                                              ECInstanceId(m_cachedLocatedElementId.GetValue())))
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId LinearLocation::Query(DgnElementCR locatedElement)
    {
    auto stmtPtr = locatedElement.GetDgnDb().GetPreparedECSqlStatement("SELECT SourceECInstanceId FROM "
        BLR_SCHEMA(BLR_REL_ILinearLocationLocatesElement) " WHERE TargetECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, locatedElement.GetElementId());

    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        return stmtPtr->GetValueId<DgnElementId>(0);

    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ReferentPtr Referent::Create(Dgn::GeometricElement3dCR referencedElement, CreateAtParams const& atParams)
    {
    return nullptr;
    }


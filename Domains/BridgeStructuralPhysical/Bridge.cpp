/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BridgeStructuralPhysicalInternal.h"
#include <BridgeStructuralPhysical/Bridge.h>
#include <BridgeStructuralPhysical/BridgeCategory.h>
#include <BridgeStructuralPhysical/BridgeSubstructure.h>
#include <BridgeStructuralPhysical/BridgeStructuralPhysicalDomain.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Bridge::Bridge(Dgn::PhysicalElementR element, CreateFromToParams const& fromToParams):
    T_Super(element), ILinearlyLocatedSingleFromTo(fromToParams)
    {
    _SetLinearElement(fromToParams.m_linearElementCPtr->GetElementId());
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());

    SetStartValue(0);    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  07/2018
//---------------------------------------------------------------------------------------
BridgePtr Bridge::Create(PhysicalModelCR physModel, DgnCodeCR code, CreateFromToParams const& fromToParams)
    {
    if (!physModel.IsGeometricModel())
        return nullptr;
	DgnDbR dgnDb = physModel.GetDgnDb();
   // auto dgnDb = ;
    Dgn::PhysicalElement::CreateParams createParams(dgnDb, physModel.GetModelId(), QueryClassId(dgnDb), BridgeCategory::Get(dgnDb));
    createParams.m_code = code;
    return new Bridge(*Create(physModel.GetDgnDb(), createParams), fromToParams);
    }


DgnCode Bridge::CreateCode(PhysicalModelCR scopeModel, Utf8StringCR codeValue)
    {
    return CodeSpec::CreateCode(BBP_CODESPEC_Bridge, scopeModel, codeValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Matt.Balnis                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelCP Bridge::QueryStructuralSystemModel() const
    {
    auto multiDisciplinaryModelPtr = this->GetSubModel();
    auto structuralSystemId = StructuralSystem::Query(*multiDisciplinaryModelPtr->ToPhysicalModel());
    if (!structuralSystemId.IsValid())
        return nullptr;

    auto structuralSystemCPtr = StructuralSystem::Get(GetDgnDb(), structuralSystemId);

    return structuralSystemCPtr->GetSubModel()->ToPhysicalModel();
    }

BRIDGESTRUCTURALPHYSICAL_EXPORT Dgn::PhysicalModelCP Bridge::QueryMultidisciplinaryModel() const
    {
    auto multidisciplinaryModelPtr = this->GetSubModel();
    if (!multidisciplinaryModelPtr.IsValid())
        {
        return nullptr;
        }

    return multidisciplinaryModelPtr->ToPhysicalModel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<LinearLocationReference> Bridge::QueryOrderedSupports() const
    {
    bset<DgnClassId> classIds;
    classIds.insert(SubstructureElement::QueryClassId(GetDgnDb()));
    return QueryLinearLocations(classIds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DgnElementId> Bridge::QueryOrderedSuperstructures() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(),
        "SELECT ECInstanceId FROM " BBP_SCHEMA(BBP_CLASS_SuperstructureElement) " "
        "WHERE ParentId = ? AND " 
            BBP_PROP_BridgeSuperstructure_FromSupport " IS NULL AND " BBP_PROP_BridgeSuperstructure_ToSupport " IS NULL "
        "UNION "
        "SELECT ECInstanceId FROM " BBP_SCHEMA(BBP_CLASS_SuperstructureElement) " superstr, "
            BLR_SCHEMA(BLR_CLASS_LinearlyReferencedAtLocation) " supportAt "
        "WHERE superstr.ParentId = ? AND superstr." BBP_PROP_BridgeSuperstructure_FromSupport " = supportAt.ElementId "
        "ORDER BY supportAt.AtPosition.DistanceAlongFromStart;");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, GetElementId());
    stmt.BindId(2, GetElementId());

    bvector<DgnElementId> retVal;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        retVal.push_back(stmt.GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BridgeCPtr Bridge::Insert(DgnDbStatus* stat)
    {
    _SetLength(fabs(GetToDistanceAlongFromStart() - GetFromDistanceAlongFromStart()));
    BridgeCPtr retCPtr = new Bridge(*getP()->GetDgnDb().Elements().Insert<PhysicalElement>(*getP(), stat));

    DgnDbStatus status = Dgn::DgnDbStatus::Success;
    if (retCPtr.IsNull() ||
        DgnDbStatus::Success != (status = _InsertLinearElementRelationship()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retCPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BridgeCPtr Bridge::Update(DgnDbStatus* stat)
    {
    _SetLength(fabs(GetToDistanceAlongFromStart() - GetFromDistanceAlongFromStart()));
    BridgeCPtr retCPtr = new Bridge(*getP()->GetDgnDb().Elements().Update<PhysicalElement>(*getP(), stat));

    DgnDbStatus status = Dgn::DgnDbStatus::Success;
    if (retCPtr.IsNull() ||
        DgnDbStatus::Success != (status = _UpdateLinearElementRelationship()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retCPtr;
    }
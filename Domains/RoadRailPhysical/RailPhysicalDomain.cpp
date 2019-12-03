/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RailPhysicalDomain.h>
#include <RoadRailPhysical/Corridor.h>
#include <RoadRailPhysical/Railway.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus createRailwayStandardsPartition(SubjectCR subject)
    {
    DgnDbStatus status;

    auto railwayStandardsPartitionPtr = DefinitionPartition::Create(subject, RailPhysicalDomain::GetRailwayStandardsPartitionName());
    if (railwayStandardsPartitionPtr->Insert(&status).IsNull())
        return status;

    auto railwayStandardsModelPtr = DefinitionModel::Create(*railwayStandardsPartitionPtr);

    if (DgnDbStatus::Success != (status = railwayStandardsModelPtr->Insert()))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RailPhysicalDomain::SetUpDefinitionPartition(SubjectCR subject)
    {
    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = createRailwayStandardsPartition(subject)))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RailNetwork::CreateCode(PhysicalModelCR scopeModel, Utf8StringCR networkCode)
    {
    return CodeSpec::CreateCode(BRLP_CODESPEC_RailNetwork, scopeModel, networkCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RailNetworkCPtr RailNetwork::Insert(PhysicalModelR parentModel, Utf8StringCR networkName)
    {
    if (!parentModel.GetModelId().IsValid())
        return nullptr;

    PhysicalElement::CreateParams createParams(parentModel.GetDgnDb(), parentModel.GetModelId(), QueryClassId(parentModel.GetDgnDb()),
                                               RoadRailCategory::GetNetwork(parentModel.GetDgnDb()));
    createParams.m_code = CreateCode(parentModel, networkName);

    RailNetworkPtr newPtr(new RailNetwork(*Create(parentModel.GetDgnDb(), createParams)));
    auto networkCPtr = parentModel.GetDgnDb().Elements().Insert<PhysicalElement>(*newPtr->getP());
    if (networkCPtr.IsNull())
        return nullptr;

    auto networkPhysicalModelPtr = PhysicalModel::Create(*networkCPtr);
    if (networkPhysicalModelPtr.IsValid())
        {
        if (DgnDbStatus::Success != networkPhysicalModelPtr->Insert())
            return nullptr;
        }

    auto horizontalPartitionCPtr = HorizontalAlignments::Insert(*networkPhysicalModelPtr);
    if (horizontalPartitionCPtr.IsNull())
        return nullptr;

    auto horizontalBreakDownModelPtr = SpatialLocationModel::Create(*horizontalPartitionCPtr->get());
    horizontalBreakDownModelPtr->SetIsPlanProjection(true);
    if (DgnDbStatus::Success != horizontalBreakDownModelPtr->Insert())
        return nullptr;

    return new RailNetwork(*networkCPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionModelCPtr RailwayStandardsModelUtilities::Query(SubjectCR parentSubject)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = DefinitionPartition::CreateCode(parentSubject, RailPhysicalDomain::GetRailwayStandardsPartitionName());
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    DefinitionPartitionCPtr partition = db.Elements().Get<DefinitionPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return partition->GetSubModel()->ToDefinitionModel();
    }
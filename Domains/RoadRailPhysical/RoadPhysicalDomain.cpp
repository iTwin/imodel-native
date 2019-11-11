/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RoadPhysicalDomain.h>
#include <RoadRailPhysical/Corridor.h>
#include <RoadRailPhysical/Roadway.h>

DOMAIN_DEFINE_MEMBERS(RoadPhysicalDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadPhysicalDomain::RoadPhysicalDomain() : DgnDomain(BRDP_SCHEMA_NAME, "Bentley RoadPhysical Domain", 2)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus createRoadwayStandardsPartition(SubjectCR subject)
    {
    DgnDbStatus status;

    auto roadwayStandardsPartitionPtr = DefinitionPartition::Create(subject, RoadPhysicalDomain::GetRoadwayStandardsPartitionName());
    if (roadwayStandardsPartitionPtr->Insert(&status).IsNull())
        return status;

    auto roadwayStandardsModelPtr = DefinitionModel::Create(*roadwayStandardsPartitionPtr);

    if (DgnDbStatus::Success != (status = roadwayStandardsModelPtr->Insert()))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RoadPhysicalDomain::SetUpDefinitionPartition(SubjectCR subject)
    {
    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = createRoadwayStandardsPartition(subject)))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RoadNetwork::CreateCode(PhysicalModelCR scopeModel, Utf8StringCR networkCode)
    {
    return CodeSpec::CreateCode(BRDP_CODESPEC_RoadNetwork, scopeModel, networkCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RoadNetworkCPtr RoadNetwork::Insert(PhysicalModelR parentModel, Utf8StringCR networkName)
    {
    if (!parentModel.GetModelId().IsValid())
        return nullptr;

    PhysicalElement::CreateParams createParams(parentModel.GetDgnDb(), parentModel.GetModelId(), QueryClassId(parentModel.GetDgnDb()),
                                               RoadRailCategory::GetNetwork(parentModel.GetDgnDb()));
    createParams.m_code = CreateCode(parentModel, networkName);

    RoadNetworkPtr newPtr(new RoadNetwork(*Create(parentModel.GetDgnDb(), createParams)));
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
    if (DgnDbStatus::Success != horizontalBreakDownModelPtr->Insert())
        return nullptr;

    return new RoadNetwork(*networkCPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionModelCPtr RoadwayStandardsModelUtilities::Query(SubjectCR parentSubject)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = DefinitionPartition::CreateCode(parentSubject, RoadPhysicalDomain::GetRoadwayStandardsPartitionName());
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    DefinitionPartitionCPtr partition = db.Elements().Get<DefinitionPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return partition->GetSubModel()->ToDefinitionModel();
    }
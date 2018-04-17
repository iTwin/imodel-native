/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadDesignSpeed.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RoadDesignSpeed.h>
#include <RoadRailPhysical/RoadRailPhysicalDomain.h>

HANDLER_DEFINE_MEMBERS(RoadwayStandardsModelHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RoadwayStandardsModelPtr RoadwayStandardsModel::Query(SubjectCR parentSubject)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = DefinitionPartition::CreateCode(parentSubject, RoadRailPhysicalDomain::GetDefaultStandardsPartitionName());
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    DefinitionPartitionCPtr partition = db.Elements().Get<DefinitionPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return dynamic_cast<RoadwayStandardsModelP>(partition->GetSubModel().get());
    }
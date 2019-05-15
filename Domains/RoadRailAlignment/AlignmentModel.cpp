/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentModel.h>
#include <RoadRailAlignment/Alignment.h>
#include <RoadRailAlignment/RoadRailAlignmentDomain.h>

HANDLER_DEFINE_MEMBERS(VerticalAlignmentModelHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationModelPtr AlignmentModelUtilities::QueryDesignAlignmentsModel(SubjectCR parentSubject)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = SpatialLocationPartition::CreateCode(parentSubject, RoadRailAlignmentDomain::GetDesignPartitionName());
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    SpatialLocationPartitionCPtr partition = db.Elements().Get<SpatialLocationPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return dynamic_cast<SpatialLocationModelP>(partition->GetSubModel().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignmentsCPtr AlignmentModelUtilities::QueryHorizontalPartition(SpatialModelCR alignmentModel)
    {
    ECSqlStatement stmt;
    stmt.Prepare(alignmentModel.GetDgnDb(), 
        "SELECT ECInstanceId FROM " BRRA_SCHEMA(BRRA_CLASS_HorizontalAlignments) " WHERE Model.Id = ?;");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, alignmentModel.GetModelId());

    if (DbResult::BE_SQLITE_ROW != stmt.Step())
        return nullptr;

    return HorizontalAlignments::Get(alignmentModel.GetDgnDb(), stmt.GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet AlignmentModelUtilities::QueryAlignmentIds(SpatialModelCR alignmentModel)
    {
    ECSqlStatement stmt;
    stmt.Prepare(alignmentModel.GetDgnDb(), 
        "SELECT ECInstanceId FROM " BRRA_SCHEMA(BRRA_CLASS_Alignment) " WHERE Model.Id = ?");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, alignmentModel.GetModelId());

    DgnElementIdSet retVal;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        retVal.insert(stmt.GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId HorizontalAlignmentModelUtilities::QueryBreakDownModelId(SpatialModelCR alignmentModel)
    {
    auto stmtPtr = alignmentModel.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT horizModel.ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_SpatialLocationModel) " horizModel, "
        BRRA_SCHEMA(BRRA_CLASS_HorizontalAlignments) " horizAligns "
        "WHERE horizModel.ModeledElement.Id = horizAligns.ECInstanceId AND horizAligns.Model.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, alignmentModel.GetModelId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnModelId();

    return stmtPtr->GetValueId<DgnModelId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentCPtr VerticalAlignmentModel::GetAlignment() const
    {
    return Alignment::Get(GetDgnDb(), GetModeledElementId());
    }

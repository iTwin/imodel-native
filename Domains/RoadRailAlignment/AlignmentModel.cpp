/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentModel.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentModel.h>
#include <RoadRailAlignment/Alignment.h>

HANDLER_DEFINE_MEMBERS(AlignmentModelHandler)
HANDLER_DEFINE_MEMBERS(HorizontalAlignmentModelHandler)
HANDLER_DEFINE_MEMBERS(VerticalAlignmentModelHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentModelPtr AlignmentModel::Query(Dgn::SubjectCR parentSubject, Utf8CP modelName)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = SpatialLocationPartition::CreateCode(parentSubject, modelName);
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    SpatialLocationPartitionCPtr partition = db.Elements().Get<SpatialLocationPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return dynamic_cast<AlignmentModelP>(partition->GetSubModel().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignmentsPortionCPtr AlignmentModel::QueryHorizontalPartition() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT ECInstanceId FROM " BRRA_SCHEMA(BRRA_CLASS_HorizontalAlignmentsPortion) " WHERE Model.Id = ?;");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, GetModelId());

    if (DbResult::BE_SQLITE_ROW != stmt.Step())
        return nullptr;

    return HorizontalAlignmentsPortion::Get(GetDgnDb(), stmt.GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId HorizontalAlignmentModel::QueryBreakDownModelId(AlignmentModelCR model)
    {
    auto stmtPtr = model.GetDgnDb().GetPreparedECSqlStatement("SELECT horizModel.ECInstanceId FROM "
        BRRA_SCHEMA(BRRA_CLASS_HorizontalAlignmentModel) " horizModel, "
        BRRA_SCHEMA(BRRA_CLASS_HorizontalAlignmentsPortion) " horizAligns "
        "WHERE horizModel.ModeledElement.Id = horizAligns.ECInstanceId AND horizAligns.Model.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, model.GetModelId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnModelId();

    return stmtPtr->GetValueId<DgnModelId>(0);
    }
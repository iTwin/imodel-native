/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentModel.h>
#include <RoadRailAlignment/Alignment.h>
#include <RoadRailAlignment/RoadRailAlignmentDomain.h>

HANDLER_DEFINE_MEMBERS(AlignmentModelHandler)
HANDLER_DEFINE_MEMBERS(HorizontalAlignmentModelHandler)
HANDLER_DEFINE_MEMBERS(VerticalAlignmentModelHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentModelPtr AlignmentModel::Query(SubjectCR parentSubject, Utf8CP partitionName)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = SpatialLocationPartition::CreateCode(parentSubject, partitionName);
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    SpatialLocationPartitionCPtr partition = db.Elements().Get<SpatialLocationPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return dynamic_cast<AlignmentModelP>(partition->GetSubModel().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr AlignmentModel::GetParentSubject() const
    {
    auto partitionCP = dynamic_cast<SpatialLocationPartitionCP>(GetModeledElement().get());
    BeAssert(partitionCP != nullptr);

    return GetDgnDb().Elements().Get<Subject>(partitionCP->GetParentId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignmentsCPtr AlignmentModel::QueryHorizontalPartition() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT ECInstanceId FROM " BRRA_SCHEMA(BRRA_CLASS_HorizontalAlignments) " WHERE Model.Id = ?;");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, GetModelId());

    if (DbResult::BE_SQLITE_ROW != stmt.Step())
        return nullptr;

    return HorizontalAlignments::Get(GetDgnDb(), stmt.GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet AlignmentModel::QueryAlignmentIds() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT ECInstanceId FROM " BRRA_SCHEMA(BRRA_CLASS_Alignment)
        " WHERE Model.Id = ?");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, GetModelId());

    DgnElementIdSet retVal;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        retVal.insert(stmt.GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId HorizontalAlignmentModel::QueryBreakDownModelId(AlignmentModelCR model)
    {
    auto stmtPtr = model.GetDgnDb().GetPreparedECSqlStatement("SELECT horizModel.ECInstanceId FROM "
        BRRA_SCHEMA(BRRA_CLASS_HorizontalAlignmentModel) " horizModel, "
        BRRA_SCHEMA(BRRA_CLASS_HorizontalAlignments) " horizAligns "
        "WHERE horizModel.ModeledElement.Id = horizAligns.ECInstanceId AND horizAligns.Model.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, model.GetModelId());

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

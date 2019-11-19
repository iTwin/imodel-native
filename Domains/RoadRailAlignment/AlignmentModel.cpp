/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentModel.h>
#include <RoadRailAlignment/Alignment.h>
#include <RoadRailAlignment/RoadRailAlignmentDomain.h>


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
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentCPtr VerticalAlignmentModel::GetAlignment() const
    {
    return Alignment::Get(GetDgnDb(), GetModeledElementId());
    }

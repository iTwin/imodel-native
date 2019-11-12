/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

HANDLER_DEFINE_MEMBERS(GimbalHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GimbalHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GimbalPtr Gimbal::Create(Dgn::SpatialModelR model)
    {
    DgnClassId classId = QueryClassId(model.GetDgnDb());
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(BDCP_CATEGORY_AcquisitionDevice, model.GetDgnDb());

    GimbalPtr cp = new Gimbal(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId));
    return cp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Gimbal::CreateCode(Dgn::DgnDbR db, Utf8StringCR value)
    {
    Utf8PrintfString internalValue("%s", value.c_str());
    return DataCaptureDomain::CreateCode(db, BDCP_CLASS_Gimbal, internalValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Gimbal::_GenerateDefaultCode() const
    {
    Utf8String defaultName = DataCaptureDomain::BuildDefaultName(MyECClassName(), GetElementId());

    return CreateCode(GetDgnDb(), defaultName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Gimbal::Gimbal(CreateParams const& params) :
    T_Super(params)
    {
    m_gimbalAngleRangeIdSetInit = false;
    m_cameraIdSetInit = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Gimbal::Gimbal(CreateParams const& params, DgnElementIdSet cameraIdSet, DgnElementIdSet gimbalAngleRangeIdSet)
:T_Super(params)
    {
    SetCameraElementIdSet(cameraIdSet);
    SetGimbalAngleRangeElementIdSet(gimbalAngleRangeIdSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GimbalElementId Gimbal::GetId() const { return GimbalElementId(GetElementId().GetValueUnchecked()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet  Gimbal::QueryGimbalHasGimbalAngleRangesRelationship(DgnDbR dgndb, GimbalElementId gimbalElmId)
    {
    DgnElementIdSet gimbalAngleRangeSet;
                                                                                 
    Utf8CP ecSql = "SELECT [TargetECInstanceId] FROM " BDCP_SCHEMA(BDCP_REL_GimbalHasGimbalAngleRanges) " WHERE SourceECInstanceId=?";

    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (!statement.IsValid())
        {
        BeAssert(statement.IsValid() && "Error preparing query. Check if DataCapture schema has been imported.");
        return gimbalAngleRangeSet;
        }

    statement->BindId(1, gimbalElmId);

    while (DbResult::BE_SQLITE_ROW == statement->Step())
        if (!statement->IsValueNull(0))
            gimbalAngleRangeSet.insert(statement->GetValueId<GimbalAngleRangeElementId>(0));

    return gimbalAngleRangeSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet Gimbal::GetGimbalAngleRangeElementIdSet() const
    {
    if (!m_gimbalAngleRangeIdSetInit)
        {
        m_gimbalAngleRangeIdSet = QueryGimbalHasGimbalAngleRangesRelationship(GetDgnDb(), GetId());
        m_gimbalAngleRangeIdSetInit = true;
        }

    return m_gimbalAngleRangeIdSet;   
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Gimbal::SetGimbalAngleRangeElementIdSet(DgnElementIdSet gimbalAngleRangeIdSet)
    {
    m_gimbalAngleRangeIdSet = gimbalAngleRangeIdSet;
    m_gimbalAngleRangeIdSetInit = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet  Gimbal::QueryGimbalHasCamerasRelationship(DgnDbR dgndb, GimbalElementId gimbalElmId)
    {
    DgnElementIdSet cameraSet;

    Utf8CP ecSql = "SELECT [TargetECInstanceId] FROM " BDCP_SCHEMA(BDCP_REL_GimbalHasCameras) " WHERE SourceECInstanceId=?";

    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (!statement.IsValid())
        {
        BeAssert(statement.IsValid() && "Error preparing query. Check if DataCapture schema has been imported.");
        return cameraSet;
        }

    statement->BindId(1, gimbalElmId);

    while (DbResult::BE_SQLITE_ROW == statement->Step())
        if (!statement->IsValueNull(0))
            cameraSet.insert(statement->GetValueId<CameraDeviceElementId>(0));

    return cameraSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet Gimbal::GetCameraElementIdSet() const
    {
    if (!m_cameraIdSetInit)
        {
        m_cameraIdSet = QueryGimbalHasCamerasRelationship(GetDgnDb(), GetId());
        m_cameraIdSetInit = true;
        }

    return m_cameraIdSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Gimbal::SetCameraElementIdSet(DgnElementIdSet cameraIdSet)
    {
    m_cameraIdSet = cameraIdSet;
    m_cameraIdSetInit = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Gimbal::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Gimbal::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Gimbal::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Gimbal::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    return T_Super::_ReadSelectParams(stmt, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Gimbal::_OnInsert()
    {
    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Gimbal::_OnInserted(DgnElementP copiedFrom) const
    {
    T_Super::_OnInserted(copiedFrom);

    //Update relationship
    InsertGimbalHasGimbalAngleRangesRelationship(GetDgnDb());
    InsertGimbalHasCamerasRelationship(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Gimbal::InsertGimbalHasGimbalAngleRangesRelationship(Dgn::DgnDbR dgndb) const
    {
    InsertGimbalHasGimbalAngleRangesRelationship(dgndb, GetId(), GetGimbalAngleRangeElementIdSet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Gimbal::InsertGimbalHasGimbalAngleRangesRelationship(DgnDbR dgndb, GimbalElementId gimbalElmId, DgnElementIdSet gimbaleAngleRangeIdSet)
    {
    if (!gimbalElmId.IsValid())
        {
        BeAssert(false && "Attempt to add GimbalHasGimbalAngleRanges relationship");
        return ERROR;
        }
    
    Utf8CP ecSql = "INSERT INTO " BDCP_SCHEMA(BDCP_REL_GimbalHasGimbalAngleRanges) " (SourceECInstanceId, TargetECInstanceId) VALUES(?, ?)";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());

    for (auto const& gimbaleAngleRangeId : gimbaleAngleRangeIdSet)
        {
        statement->BindId(1, gimbalElmId);
        statement->BindId(2, gimbaleAngleRangeId);

        DbResult stepStatus = statement->Step();
        if (BE_SQLITE_DONE != stepStatus)
            {
            BeAssert(false && "Error creating GimbalHasGimbalAngleRanges Relationship");
            return ERROR;
            }

        statement->Reset();
        statement->ClearBindings();
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Gimbal::DeleteGimbalHasGimbalAngleRangesRelationship(DgnDbR dgndb) const
    {
    if (!GetId().IsValid())
        {
        BeAssert(false && "Attempt to delete an invalid GimbalHasGimbalAngleRanges relationship");
        return;
        }

    //Delete old one 
    Utf8CP ecSql = "DELETE FROM " BDCP_SCHEMA(BDCP_REL_GimbalHasGimbalAngleRanges) " WHERE SourceECInstanceId=?";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());
    statement->BindId(1, GetId());        //Source
    DbResult stepStatus = statement->Step();
    if (BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false && "Error deleting GimbalHasGimbalAngleRanges Relationship");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Gimbal::InsertGimbalHasCamerasRelationship(Dgn::DgnDbR dgndb) const
    {
    InsertGimbalHasCamerasRelationship(dgndb, GetId(), GetCameraElementIdSet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Gimbal::InsertGimbalHasCamerasRelationship(DgnDbR dgndb, GimbalElementId gimbalElmId, DgnElementIdSet cameraIdSet)
    {
    if (!gimbalElmId.IsValid())
        {
        BeAssert(false && "Attempt to add GimbalHasGimbalAngleRanges relationship");
        return ERROR;
        }

    Utf8CP ecSql = "INSERT INTO " BDCP_SCHEMA(BDCP_REL_GimbalHasCameras) " (SourceECInstanceId, TargetECInstanceId) VALUES(?, ?)";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());

    for (auto const& cameraId : cameraIdSet)
        {
        statement->BindId(1, gimbalElmId);
        statement->BindId(2, cameraId);

        DbResult stepStatus = statement->Step();
        if (BE_SQLITE_DONE != stepStatus)
            {
            BeAssert(false && "Error creating GimbalHasCameras Relationship");
            return ERROR;
            }

        statement->Reset();
        statement->ClearBindings();
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Gimbal::DeleteGimbalHasCamerasRelationship(DgnDbR dgndb) const
    {
    if (!GetId().IsValid())
        {
        BeAssert(false && "Attempt to delete an invalid GimbalHasCameras relationship");
        return;
        }

    //Delete old one 
    Utf8CP ecSql = "DELETE FROM " BDCP_SCHEMA(BDCP_REL_GimbalHasCameras) " WHERE SourceECInstanceId=?";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());
    statement->BindId(1, GetId());        //Source
    DbResult stepStatus = statement->Step();
    if (BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false && "Error deleting GimbalHasCameras Relationship");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Gimbal::UpdateGimbalHasGimbalAngleRangesRelationship(DgnDbR dgndb) const
    {
    //Delete old one 
    DeleteGimbalHasGimbalAngleRangesRelationship(dgndb);
    //and then insert new one
    InsertGimbalHasGimbalAngleRangesRelationship(dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Gimbal::UpdateGimbalHasCamerasRelationship(DgnDbR dgndb) const
    {
    //Delete old one 
    DeleteGimbalHasCamerasRelationship(dgndb);
    //and then insert new one
    InsertGimbalHasCamerasRelationship(dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Gimbal::_OnUpdated(DgnElementCR original) const
    {
    T_Super::_OnUpdated(original);

    auto other = dynamic_cast<GimbalCP>(&original);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;
   
    if (GetGimbalAngleRangeElementIdSet().GetBriefcaseBasedIdSet() != other->GetGimbalAngleRangeElementIdSet().GetBriefcaseBasedIdSet())
        UpdateGimbalHasGimbalAngleRangesRelationship(GetDgnDb());

    if (GetCameraElementIdSet().GetBriefcaseBasedIdSet() != other->GetCameraElementIdSet().GetBriefcaseBasedIdSet())
        UpdateGimbalHasCamerasRelationship(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                   03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus Gimbal::_OnDelete() const
    {
    //If we delete a Gimbal, we should also remove all related GimbalAngleRanges
    GimbalPtr gimbalEditedPtr = Gimbal::GetForEdit(GetDgnDb(), GetId());
    gimbalEditedPtr->RemoveAllGimbalAngleRanges();

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Gimbal::_OnDeleted() const
    {
    T_Super::_OnDeleted();

    DeleteGimbalHasGimbalAngleRangesRelationship(GetDgnDb());
    DeleteGimbalHasCamerasRelationship(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Gimbal::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);
    auto other = dynamic_cast<GimbalCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;
    SetGimbalAngleRangeElementIdSet(other->GetGimbalAngleRangeElementIdSet());
    SetCameraElementIdSet(other->GetCameraElementIdSet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Gimbal::RemoveAllGimbalAngleRanges()
    {
    //If we delete a Gimbale, we should also remove all related gimbaleAngleRanges
    for (GimbalAngleRangeEntry const& gimbalAngleRange : MakeGimbalAngleRangeIterator(GetDgnDb(), GetId()))
        {
        GimbalCPtr myGimbalPtr = Gimbal::Get(GetDgnDb(), gimbalAngleRange.GetGimbalAngleRangeElementId());
        //delete them all
        myGimbalPtr->Delete();
        }

    DgnElementIdSet gimbalIdSet = GetGimbalAngleRangeElementIdSet();
    gimbalIdSet.clear();
    SetGimbalAngleRangeElementIdSet(gimbalIdSet);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Gimbal::GimbalAngleRangeIterator Gimbal::MakeGimbalAngleRangeIterator(Dgn::DgnDbCR dgndb, GimbalElementId gimbalId)
    {
    Utf8CP ecSql = "SELECT TargetECInstanceId  FROM " BDCP_SCHEMA(BDCP_REL_GimbalHasGimbalAngleRanges) " WHERE SourceECInstanceId=?";

    Gimbal::GimbalAngleRangeIterator iterator;
    int idSelectColumnIndex = 0;
    ECSqlStatement* statement = iterator.Prepare(dgndb, ecSql, idSelectColumnIndex);
    if (statement != nullptr)
        statement->BindId(1, gimbalId);

    return iterator;
    }

END_BENTLEY_DATACAPTURE_NAMESPACE


/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

HANDLER_DEFINE_MEMBERS(ShotHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ShotHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                 03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef Shot::GetDefaultColor()
    {
    return ColorDef(0xff, 0x7f, 0x27); //orange color
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                 03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
int Shot::GetDefaultWeight()
    {
    return 2;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ShotPtr Shot::Create(Dgn::SpatialModelR model, CameraDeviceElementId cameraDevice, PoseElementId pose)
    {
    if (!cameraDevice.IsValid())
        {
        BeAssert(false && "Cannot create a shot with an invalid cameraDevice");
        return nullptr;
        }

    if (!pose.IsValid())
        {
        BeAssert(false && "Cannot create a shot with an invalid pose");
        return nullptr;
        }

    DgnClassId classId = QueryClassId(model.GetDgnDb());
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(BDCP_CATEGORY_Shot, model.GetDgnDb());

    ShotPtr cp = new Shot(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId),cameraDevice,pose);
    return cp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Shot::CreateCode(Dgn::DgnDbR db, Utf8StringCR CameraDeviceValue, Utf8StringCR value) 
    {
    Utf8PrintfString internalValue("%s/%s",CameraDeviceValue.c_str(),value.c_str());
    return DataCaptureDomain::CreateCode(db,BDCP_CLASS_Shot,internalValue);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Shot::_GenerateDefaultCode() const
    {
    CameraDeviceElementId deviceId = GetCameraDeviceId();
    CameraDeviceCPtr pCameraDevice = CameraDevice::Get(GetDgnDb(), deviceId);
    Utf8String defaultName = DataCaptureDomain::BuildDefaultName(MyECClassName(), GetElementId());

    return CreateCode(GetDgnDb(), pCameraDevice->GetCode().GetValue(), defaultName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Shot::Shot(CreateParams const& params, CameraDeviceElementId cameraDevice,PoseElementId pose)
:T_Super(params), m_cameraDevice(cameraDevice), m_pose(pose)
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ShotElementId   Shot::GetId() const { return ShotElementId(GetElementId().GetValueUnchecked()); }
void            Shot::SetCameraDeviceId(CameraDeviceElementId val) { m_cameraDevice = val; }
void            Shot::SetPoseId(PoseElementId val) { m_pose = val; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraDeviceElementId  Shot::QueryShotIsTakenByCameraDeviceRelationship(DgnDbR dgndb, ShotElementId shotElmId)
    {
    Utf8CP ecSql = "SELECT [TargetECInstanceId]  FROM " BDCP_SCHEMA(BDCP_REL_ShotIsTakenByCameraDevice) " WHERE SourceECInstanceId=?";

    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (!statement.IsValid())
        {
        BeAssert(statement.IsValid() && "Error preparing query. Check if DataCapture schema has been imported.");
        return CameraDeviceElementId();
        }

    statement->BindId(1, shotElmId);

    DbResult stepStatus = statement->Step();
    if (stepStatus != BE_SQLITE_ROW)
        return CameraDeviceElementId();

    return statement->GetValueId<CameraDeviceElementId>(0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraDeviceElementId  Shot::GetCameraDeviceId() const
    {
    //Query and cache the result
    if (!m_cameraDevice.IsValid())
        m_cameraDevice = QueryShotIsTakenByCameraDeviceRelationship(GetDgnDb(),GetId());
    return m_cameraDevice;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PoseElementId  Shot::QueryShotIsTakenAtPoseRelationship(DgnDbR dgndb, ShotElementId shotElmId)
    {
    Utf8CP ecSql = "SELECT [TargetECInstanceId]  FROM " BDCP_SCHEMA(BDCP_REL_ShotIsTakenAtPose) " WHERE SourceECInstanceId=?";

    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (!statement.IsValid())
        {
        BeAssert(statement.IsValid() && "Error preparing query. Check if DataCapture schema has been imported.");
        return PoseElementId();
        }

    statement->BindId(1, shotElmId);

    DbResult stepStatus = statement->Step();
    if (stepStatus != BE_SQLITE_ROW)
        return PoseElementId();

    return statement->GetValueId<PoseElementId>(0);
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PoseElementId  Shot::GetPoseId() const
    {
    //Query and cache the result
    if (!m_pose.IsValid())
        m_pose = QueryShotIsTakenAtPoseRelationship(GetDgnDb(),GetId());
    return m_pose;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Shot::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Shot::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Shot::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Shot::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    return T_Super::_ReadSelectParams(stmt, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Shot::_OnInsert()
    {
    if (!m_cameraDevice.IsValid())
        {
        BeAssert(false && "Cannot insert a shot with an invalid cameraDevice");
        return DgnDbStatus::ValidationFailed;
        }
    if (!m_pose.IsValid())
        {
        BeAssert(false && "Cannot insert a shot with an invalid pose");
        return DgnDbStatus::ValidationFailed;
        }

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Shot::InsertShotIsTakenByCameraDeviceRelationship(Dgn::DgnDbR dgndb) const
    {
    InsertShotIsTakenByCameraDeviceRelationship(dgndb,GetId(),GetCameraDeviceId());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Shot::InsertShotIsTakenByCameraDeviceRelationship(DgnDbR dgndb, ShotElementId shotElmId, CameraDeviceElementId cameraDeviceElmId)
    {
    if (!shotElmId.IsValid() || !cameraDeviceElmId.IsValid())
        {
        BeAssert(false && "Attempt to add invalid shot is taken by relationship");
        return ERROR;
        }

    Utf8CP ecSql = "INSERT INTO " BDCP_SCHEMA(BDCP_REL_ShotIsTakenByCameraDevice) " (SourceECInstanceId, TargetECInstanceId) VALUES(?, ?)";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());

    statement->BindId(1, shotElmId);
    statement->BindId(2, cameraDeviceElmId);

    DbResult stepStatus = statement->Step();
    if (BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false && "Error creating ShotIsTakenByCameraDevice Relationship");
        return ERROR;
        }
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Shot::InsertShotIsTakenAtPoseRelationship(Dgn::DgnDbR dgndb) const
    {
    InsertShotIsTakenAtPoseRelationship(dgndb,GetId(),GetPoseId());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Shot::InsertShotIsTakenAtPoseRelationship(DgnDbR dgndb, ShotElementId shotElmId, PoseElementId poseElmId)
    {
    if (!shotElmId.IsValid() || !poseElmId.IsValid())
        {
        BeAssert(false && "Attempt to add invalid shot is taken at pose relationship");
        return ERROR;
        }

    Utf8CP ecSql = "INSERT INTO " BDCP_SCHEMA(BDCP_REL_ShotIsTakenAtPose) " (SourceECInstanceId, TargetECInstanceId) VALUES(?, ?)";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());

    statement->BindId(1, shotElmId);
    statement->BindId(2, poseElmId);

    DbResult stepStatus = statement->Step();
    if (BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false && "Error creating ShotIsTakenAtPose Relationship");
        return ERROR;
        }
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Shot::_OnInserted(DgnElementP copiedFrom) const
    {
    T_Super::_OnInserted(copiedFrom);

    //Update relationship
    InsertShotIsTakenByCameraDeviceRelationship(GetDgnDb());
    InsertShotIsTakenAtPoseRelationship(GetDgnDb());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Shot::DeleteShotIsTakenByCameraDeviceRelationship(DgnDbR dgndb) const
    {
    if (!GetId().IsValid())
        {
        BeAssert(false && "Attempt to delete an invalid ShotIsTakenByCameraDevice relationship");
        return;
        }

    //Delete old one 
    Utf8CP ecSql = "DELETE FROM " BDCP_SCHEMA(BDCP_REL_ShotIsTakenByCameraDevice) " WHERE SourceECInstanceId=?";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());
    statement->BindId(1, GetId());        //Source
    DbResult stepStatus = statement->Step();
    if (BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false && "Error deleting ShotIsTakenByCameraDevice Relationship");
        }
    
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Shot::DeleteShotIsTakenAtPoseRelationship(DgnDbR dgndb) const
    {
    if (!GetId().IsValid())
        {
        BeAssert(false && "Attempt to delete an invalid ShotIsTakenAtPose relationship");
        return;
        }

    //Delete old one 
    Utf8CP ecSql = "DELETE FROM " BDCP_SCHEMA(BDCP_REL_ShotIsTakenAtPose) " WHERE SourceECInstanceId=?";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());
    statement->BindId(1, GetId());        //Source
    DbResult stepStatus = statement->Step();
    if (BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false && "Error deleting ShotIsTakenAtPose Relationship");
        }
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Shot::UpdateShotIsTakenByCameraDeviceRelationship(DgnDbR dgndb) const
    {
    //Delete old one 
    DeleteShotIsTakenByCameraDeviceRelationship(dgndb);
    //and then insert new one
    InsertShotIsTakenByCameraDeviceRelationship(dgndb);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Shot::UpdateShotIsTakenAtPoseRelationship(DgnDbR dgndb) const
    {
    //Delete old one 
    DeleteShotIsTakenAtPoseRelationship(dgndb);
    //and then insert new one
    InsertShotIsTakenAtPoseRelationship(dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Shot::_OnUpdated(DgnElementCR original) const
    {
    T_Super::_OnUpdated(original);

    auto other = dynamic_cast<ShotCP>(&original);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;

    //Update relationship
    if (GetCameraDeviceId() != other->GetCameraDeviceId())
        UpdateShotIsTakenByCameraDeviceRelationship(GetDgnDb());
    //Update relationship
    if (GetPoseId() != other->GetPoseId())
        UpdateShotIsTakenAtPoseRelationship(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                   02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus Shot::_OnDelete() const
    {
    //NEEDSWORK: not now    
    // Verify if the associated Pose is use by another shot.  I no, delete the pose

    // Remove the associated pose 
    PoseCPtr posePtr = Pose::Get(GetDgnDb(), GetPoseId());
    posePtr->Delete();
    
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Shot::_OnDeleted() const
    {
    T_Super::_OnDeleted();
    DeleteShotIsTakenByCameraDeviceRelationship(GetDgnDb());
    DeleteShotIsTakenAtPoseRelationship(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Shot::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<ShotCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;
    SetCameraDeviceId(other->GetCameraDeviceId());
    SetPoseId(other->GetPoseId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Shot::_RemapIds(DgnImportContext& importer)
    {
    BeAssert(importer.IsBetweenDbs());
    T_Super::_RemapIds(importer);
    DgnElementId cameraId(GetCameraDeviceId());
    DgnElementId poseId(GetPoseId());
    DgnElementId newIdForCamera = importer.FindElementId(cameraId);
    CameraDeviceElementId newCameraId(newIdForCamera.GetValue());
    SetCameraDeviceId(newCameraId);
    DgnElementId newIdForPose = importer.FindElementId(poseId);
    PoseElementId newPoseId(newIdForPose.GetValue());
    SetPoseId(newPoseId);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ShotElementId Shot::QueryForIdByLabel(DgnDbR dgndb, Utf8CP label)
    {
    Utf8CP ecSql = "SELECT Shot.[ECInstanceId] FROM " BDCP_SCHEMA(BDCP_CLASS_Shot) " Shot " \
        "WHERE Shot.Label=?";

    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (!statement.IsValid())
        {
        BeAssert(statement.IsValid() && "Error preparing query. Check if DataCapture schema has been imported.");
        return ShotElementId();
        }

    statement->BindText(1, label, IECSqlBinder::MakeCopy::No);

    DbResult stepStatus = statement->Step();
    if (stepStatus != BE_SQLITE_ROW)
        return ShotElementId();

    return statement->GetValueId<ShotElementId>(0);
    }


END_BENTLEY_DATACAPTURE_NAMESPACE


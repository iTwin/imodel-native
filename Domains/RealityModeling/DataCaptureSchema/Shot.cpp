/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Shot.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

HANDLER_DEFINE_MEMBERS(ShotHandler)

#define Shot_PROPNAME_Pose                     "Pose"
#define Shot_PROPNAME_Pose_Center              "Center"
#define Shot_PROPNAME_Pose_Rotation            "Rotation"
#define Shot_PROPNAME_Pose_Rotation_M_00       "M_00"
#define Shot_PROPNAME_Pose_Rotation_M_01       "M_01"
#define Shot_PROPNAME_Pose_Rotation_M_02       "M_02"
#define Shot_PROPNAME_Pose_Rotation_M_10       "M_10"
#define Shot_PROPNAME_Pose_Rotation_M_11       "M_11"
#define Shot_PROPNAME_Pose_Rotation_M_12       "M_12"
#define Shot_PROPNAME_Pose_Rotation_M_20       "M_20"
#define Shot_PROPNAME_Pose_Rotation_M_21       "M_21"
#define Shot_PROPNAME_Pose_Rotation_M_22       "M_22"



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECSqlStatus RotationMatrixType::BindParameter(IECSqlStructBinder& binder, RotationMatrixTypeCR val)
    {
    if (ECSqlStatus::Success != binder.GetMember(Shot_PROPNAME_Pose_Rotation_M_00).BindDouble(val.GetComponentByRowAndColumn(0, 0)) ||
        ECSqlStatus::Success != binder.GetMember(Shot_PROPNAME_Pose_Rotation_M_01).BindDouble(val.GetComponentByRowAndColumn(0, 1)) ||
        ECSqlStatus::Success != binder.GetMember(Shot_PROPNAME_Pose_Rotation_M_02).BindDouble(val.GetComponentByRowAndColumn(0, 2)) ||
        ECSqlStatus::Success != binder.GetMember(Shot_PROPNAME_Pose_Rotation_M_10).BindDouble(val.GetComponentByRowAndColumn(1, 0)) ||
        ECSqlStatus::Success != binder.GetMember(Shot_PROPNAME_Pose_Rotation_M_11).BindDouble(val.GetComponentByRowAndColumn(1, 1)) ||
        ECSqlStatus::Success != binder.GetMember(Shot_PROPNAME_Pose_Rotation_M_12).BindDouble(val.GetComponentByRowAndColumn(1, 2)) ||
        ECSqlStatus::Success != binder.GetMember(Shot_PROPNAME_Pose_Rotation_M_20).BindDouble(val.GetComponentByRowAndColumn(2, 0)) ||
        ECSqlStatus::Success != binder.GetMember(Shot_PROPNAME_Pose_Rotation_M_21).BindDouble(val.GetComponentByRowAndColumn(2, 1)) ||
        ECSqlStatus::Success != binder.GetMember(Shot_PROPNAME_Pose_Rotation_M_22).BindDouble(val.GetComponentByRowAndColumn(2, 2)))
        {
        return ECSqlStatus::Error;
        }
    return ECSqlStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RotationMatrixType RotationMatrixType::GetValue(IECSqlStructValue const& structValue)
    {
    RotationMatrixType rotation;
    for (int ii = 0; ii < structValue.GetMemberCount(); ii++)
        {
        IECSqlValue const& memberValue = structValue.GetValue(ii);
        ECPropertyCP memberProperty = memberValue.GetColumnInfo().GetProperty();
        BeAssert(memberProperty != nullptr);
        Utf8CP memberName = memberProperty->GetName().c_str();

        if      (0 == BeStringUtilities::Stricmp(Shot_PROPNAME_Pose_Rotation_M_00, memberName))
            rotation.SetComponentByRowAndColumn(0, 0,memberValue.GetDouble());
        else if (0 == BeStringUtilities::Stricmp(Shot_PROPNAME_Pose_Rotation_M_01, memberName))
            rotation.SetComponentByRowAndColumn(0, 1,memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Shot_PROPNAME_Pose_Rotation_M_02, memberName))
            rotation.SetComponentByRowAndColumn(0, 2,memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Shot_PROPNAME_Pose_Rotation_M_10, memberName))
            rotation.SetComponentByRowAndColumn(1, 0, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Shot_PROPNAME_Pose_Rotation_M_11, memberName))
            rotation.SetComponentByRowAndColumn(1, 1, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Shot_PROPNAME_Pose_Rotation_M_12, memberName))
            rotation.SetComponentByRowAndColumn(1, 2, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Shot_PROPNAME_Pose_Rotation_M_20, memberName))
            rotation.SetComponentByRowAndColumn(2, 0, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Shot_PROPNAME_Pose_Rotation_M_21, memberName))
            rotation.SetComponentByRowAndColumn(2, 1, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Shot_PROPNAME_Pose_Rotation_M_22, memberName))
            rotation.SetComponentByRowAndColumn(2, 2, memberValue.GetInt());
        else
            BeAssert(false);
        }
    return rotation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PoseType::PoseType(DPoint3dCR center, RotationMatrixTypeCR rotation) :m_center(center), m_rotation(rotation) {}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PoseType::PoseType():m_rotation(RotationMatrixType::FromIdentity())
    {
    m_center = {0.0,0.0,0.0};
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PoseType::PoseType(PoseTypeCR rhs) { *this = rhs; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PoseTypeR PoseType::operator= (PoseTypeCR rhs)
    {
    m_center = rhs.m_center;
    m_rotation = rhs.m_rotation;
    return *this;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool PoseType::IsEqual(PoseTypeCR rhs) const
    {
    if (m_rotation.IsEqual(rhs.m_rotation) && m_center.IsEqual(rhs.m_center))
        return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d            PoseType::GetCenter() const { return m_center; }
RotationMatrixType  PoseType::GetRotation() const { return m_rotation; }
void                PoseType::SetCenter(DPoint3dCR val) { m_center = val; }
void                PoseType::SetRotation(RotationMatrixTypeCR val) { m_rotation = val; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECSqlStatus PoseType::BindParameter(IECSqlStructBinder& binder, PoseTypeCR val)
    {
    if (ECSqlStatus::Success != binder.GetMember(Shot_PROPNAME_Pose_Center).BindPoint3D(val.GetCenter()) ||
        ECSqlStatus::Success != RotationMatrixType::BindParameter(binder.GetMember(Shot_PROPNAME_Pose_Rotation).BindStruct(), val.GetRotation()))
        {
        return ECSqlStatus::Error;
        }
    return ECSqlStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PoseType PoseType::GetValue(BeSQLite::EC::IECSqlStructValue const& structValue)
    {
    PoseType pose;
    for (int ii = 0; ii < structValue.GetMemberCount(); ii++)
        {
        IECSqlValue const& memberValue = structValue.GetValue(ii);
        ECPropertyCP memberProperty = memberValue.GetColumnInfo().GetProperty();
        BeAssert(memberProperty != nullptr);
        Utf8CP memberName = memberProperty->GetName().c_str();

        if (0 == BeStringUtilities::Stricmp(Shot_PROPNAME_Pose_Center, memberName))
            {
            pose.SetCenter(memberValue.GetPoint3D());
            }
        else if (0 == BeStringUtilities::Stricmp(Shot_PROPNAME_Pose_Rotation, memberName))
            {
            IECSqlStructValue const& rotationStructValue = memberValue.GetStruct();
            pose.SetRotation(RotationMatrixType::GetValue(rotationStructValue));
            }
        else
            {
            BeAssert(false);
            }
        }
    return pose;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ShotHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(Shot_PROPNAME_Pose);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ShotPtr Shot::Create(Dgn::SpatialModelR model, CameraDeviceElementId cameraDevice)
    {
    if (!cameraDevice.IsValid())
        {
        BeAssert(false && "Cannot create a shot with an invalid cameraDevice");
        return nullptr;
        }

    DgnClassId classId = QueryClassId(model.GetDgnDb());
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(BDCP_CATEGORY_Shot, model.GetDgnDb());

    ShotPtr cp = new Shot(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId),cameraDevice);
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
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ShotElementId   Shot::GetId() const { return ShotElementId(GetElementId().GetValueUnchecked()); }
PoseType        Shot::GetPose() const { return m_pose; }
void            Shot::SetPose(PoseTypeCR val) { m_pose = val; }
void            Shot::SetCameraDeviceId(CameraDeviceElementId val) { m_cameraDevice = val; }

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
DgnDbStatus Shot::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
    IECSqlStructBinder& structBinder = statement.BindStruct(statement.GetParameterIndex(Shot_PROPNAME_Pose));
    if (ECSqlStatus::Success != PoseType::BindParameter(structBinder ,GetPose()) )
        {
        return DgnDbStatus::BadArg;
        }
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
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        int poseIndex = params.GetSelectIndex(Shot_PROPNAME_Pose);
        IECSqlStructValue const& structValue = stmt.GetValueStruct(poseIndex);
        SetPose(PoseType::GetValue(structValue));
        }

    return status;
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
void Shot::_OnInserted(DgnElementP copiedFrom) const
    {
    T_Super::_OnInserted(copiedFrom);

    //Update relationship
    InsertShotIsTakenByCameraDeviceRelationship(GetDgnDb());
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
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Shot::_OnDeleted() const
    {
    T_Super::_OnDeleted();
    DeleteShotIsTakenByCameraDeviceRelationship(GetDgnDb());
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

    SetPose(other->GetPose());
    SetCameraDeviceId(other->GetCameraDeviceId());
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


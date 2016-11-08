/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Photo.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

HANDLER_DEFINE_MEMBERS(PhotoHandler)

#define Photo_PROPNAME_PhotoId                  "PhotoId"
#define Photo_PROPNAME_Pose                     "Pose"
#define Photo_PROPNAME_Pose_Center              "Center"
#define Photo_PROPNAME_Pose_Rotation            "Rotation"
#define Photo_PROPNAME_Pose_Rotation_M_00       "M_00"
#define Photo_PROPNAME_Pose_Rotation_M_01       "M_01"
#define Photo_PROPNAME_Pose_Rotation_M_02       "M_02"
#define Photo_PROPNAME_Pose_Rotation_M_10       "M_10"
#define Photo_PROPNAME_Pose_Rotation_M_11       "M_11"
#define Photo_PROPNAME_Pose_Rotation_M_12       "M_12"
#define Photo_PROPNAME_Pose_Rotation_M_20       "M_20"
#define Photo_PROPNAME_Pose_Rotation_M_21       "M_21"
#define Photo_PROPNAME_Pose_Rotation_M_22       "M_22"



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECSqlStatus RotationMatrixType::BindParameter(IECSqlStructBinder& binder, RotationMatrixTypeCR val)
    {
    if (ECSqlStatus::Success != binder.GetMember(Photo_PROPNAME_Pose_Rotation_M_00).BindDouble(val.GetComponentByRowAndColumn(0, 0)) ||
        ECSqlStatus::Success != binder.GetMember(Photo_PROPNAME_Pose_Rotation_M_01).BindDouble(val.GetComponentByRowAndColumn(0, 1)) ||
        ECSqlStatus::Success != binder.GetMember(Photo_PROPNAME_Pose_Rotation_M_02).BindDouble(val.GetComponentByRowAndColumn(0, 2)) ||
        ECSqlStatus::Success != binder.GetMember(Photo_PROPNAME_Pose_Rotation_M_10).BindDouble(val.GetComponentByRowAndColumn(1, 0)) ||
        ECSqlStatus::Success != binder.GetMember(Photo_PROPNAME_Pose_Rotation_M_11).BindDouble(val.GetComponentByRowAndColumn(1, 1)) ||
        ECSqlStatus::Success != binder.GetMember(Photo_PROPNAME_Pose_Rotation_M_12).BindDouble(val.GetComponentByRowAndColumn(1, 2)) ||
        ECSqlStatus::Success != binder.GetMember(Photo_PROPNAME_Pose_Rotation_M_20).BindDouble(val.GetComponentByRowAndColumn(2, 0)) ||
        ECSqlStatus::Success != binder.GetMember(Photo_PROPNAME_Pose_Rotation_M_21).BindDouble(val.GetComponentByRowAndColumn(2, 1)) ||
        ECSqlStatus::Success != binder.GetMember(Photo_PROPNAME_Pose_Rotation_M_22).BindDouble(val.GetComponentByRowAndColumn(2, 2)))
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

        if      (0 == BeStringUtilities::Stricmp(Photo_PROPNAME_Pose_Rotation_M_00, memberName))
            rotation.SetComponentByRowAndColumn(0, 0,memberValue.GetDouble());
        else if (0 == BeStringUtilities::Stricmp(Photo_PROPNAME_Pose_Rotation_M_01, memberName))
            rotation.SetComponentByRowAndColumn(0, 1,memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Photo_PROPNAME_Pose_Rotation_M_02, memberName))
            rotation.SetComponentByRowAndColumn(0, 2,memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Photo_PROPNAME_Pose_Rotation_M_10, memberName))
            rotation.SetComponentByRowAndColumn(1, 0, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Photo_PROPNAME_Pose_Rotation_M_11, memberName))
            rotation.SetComponentByRowAndColumn(1, 1, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Photo_PROPNAME_Pose_Rotation_M_12, memberName))
            rotation.SetComponentByRowAndColumn(1, 2, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Photo_PROPNAME_Pose_Rotation_M_20, memberName))
            rotation.SetComponentByRowAndColumn(2, 0, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Photo_PROPNAME_Pose_Rotation_M_21, memberName))
            rotation.SetComponentByRowAndColumn(2, 1, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Photo_PROPNAME_Pose_Rotation_M_22, memberName))
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
#ifdef WIP_MERGE_Donald
    if (ECSqlStatus::Success != binder.GetMember(Photo_PROPNAME_Pose_Center).BindPoint3D(val.GetCenter()) ||
        ECSqlStatus::Success != RotationMatrixType::BindParameter(binder.GetMember(Photo_PROPNAME_Pose_Rotation).BindStruct(), val.GetRotation()))
        {
        return ECSqlStatus::Error;
        }
#endif
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

        if (0 == BeStringUtilities::Stricmp(Photo_PROPNAME_Pose_Center, memberName))
            {
#ifdef WIP_MERGE_Donald
            pose.SetCenter(memberValue.GetPoint3D());
#endif
            }
        else if (0 == BeStringUtilities::Stricmp(Photo_PROPNAME_Pose_Rotation, memberName))
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
#ifdef WIP_MERGE_Donald
void PhotoHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(Photo_PROPNAME_PhotoId);
    params.Add(Photo_PROPNAME_Pose);
    }
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PhotoPtr Photo::Create(Dgn::SpatialModelR model, CameraElementId camera)
    {
    if (!camera.IsValid())
        {
        BeAssert(false && "Cannot create a photo with an invalid camera");
        return nullptr;
        }

    DgnClassId classId = QueryClassId(model.GetDgnDb());
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(BDCP_CATEGORY_Photo, model.GetDgnDb());

    PhotoPtr cp = new Photo(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId),camera);
    return cp;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PhotoElementId  Photo::GetId() const { return PhotoElementId(GetElementId().GetValueUnchecked()); }
int             Photo::GetPhotoId() const { return m_photoId; }
PoseType        Photo::GetPose() const { return m_pose; }
void            Photo::SetPhotoId(int val) { m_photoId = val; }
void            Photo::SetPose(PoseTypeCR val) { m_pose = val; }
void            Photo::SetCameraId(CameraElementId val) { m_camera = val; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraElementId  Photo::QueryPhotoIsTakenByCameraRelationship(DgnDbR dgndb, PhotoElementId photoElmId)
    {
    Utf8CP ecSql = "SELECT [TargetECInstanceId]  FROM " BDCP_SCHEMA(BDCP_REL_PhotoIsTakenByCamera) " WHERE SourceECInstanceId=?";

    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (!statement.IsValid())
        {
        BeAssert(statement.IsValid() && "Error preparing query. Check if DataCapture schema has been imported.");
        return CameraElementId();
        }

    statement->BindId(1, photoElmId);

    DbResult stepStatus = statement->Step();
    if (stepStatus != BE_SQLITE_ROW)
        return CameraElementId();

    return statement->GetValueId<CameraElementId>(0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraElementId  Photo::GetCameraId() const
    {
    //Query and cache the result
    if (!m_camera.IsValid())
        m_camera = QueryPhotoIsTakenByCameraRelationship(GetDgnDb(),GetId());
    return m_camera;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Photo::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
    IECSqlStructBinder& structBinder = statement.BindStruct(statement.GetParameterIndex(Photo_PROPNAME_Pose));
    if (ECSqlStatus::Success != statement.BindInt(statement.GetParameterIndex(Photo_PROPNAME_PhotoId), GetPhotoId()) ||
        ECSqlStatus::Success != PoseType::BindParameter(structBinder ,GetPose()) )
        {
        return DgnDbStatus::BadArg;
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_MERGE_Donald
DgnDbStatus Photo::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Photo::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Photo::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        //read Photo properties
        SetPhotoId (stmt.GetValueInt(params.GetSelectIndex(Photo_PROPNAME_PhotoId)));

        int poseIndex = params.GetSelectIndex(Photo_PROPNAME_Pose);
        IECSqlStructValue const& structValue = stmt.GetValueStruct(poseIndex);
        SetPose(PoseType::GetValue(structValue));
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Photo::_OnInsert()
    {
    if (!m_camera.IsValid())
        {
        BeAssert(false && "Cannot insert a photo with an invalid camera");
        return DgnDbStatus::ValidationFailed;
        }

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Photo::InsertPhotoIsTakenByCameraRelationship(Dgn::DgnDbR dgndb) const
    {
    InsertPhotoIsTakenByCameraRelationship(dgndb,GetId(),GetCameraId());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Photo::InsertPhotoIsTakenByCameraRelationship(DgnDbR dgndb, PhotoElementId photoElmId, CameraElementId cameraElmId)
    {
    if (!photoElmId.IsValid() || !cameraElmId.IsValid())
        {
        BeAssert(false && "Attempt to add invalid photo is taken by relationship");
        return ERROR;
        }

    Utf8CP ecSql = "INSERT INTO " BDCP_SCHEMA(BDCP_REL_PhotoIsTakenByCamera) " (SourceECInstanceId, TargetECInstanceId) VALUES(?, ?)";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());

    statement->BindId(1, photoElmId);
    statement->BindId(2, cameraElmId);

    DbResult stepStatus = statement->Step();
    if (BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false && "Error creating PhotoIsTakenByCamera Relationship");
        return ERROR;
        }
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Photo::_OnInserted(DgnElementP copiedFrom) const
    {
    T_Super::_OnInserted(copiedFrom);

    //Update relationship
    InsertPhotoIsTakenByCameraRelationship(GetDgnDb());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Photo::DeletePhotoIsTakenByCameraRelationship(DgnDbR dgndb) const
    {
    if (!GetId().IsValid())
        {
        BeAssert(false && "Attempt to delete an invalid PhotoIsTakenByCamera relationship");
        return;
        }

    //Delete old one 
    Utf8CP ecSql = "DELETE FROM " BDCP_SCHEMA(BDCP_REL_PhotoIsTakenByCamera) " WHERE SourceECInstanceId=?";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());
    statement->BindId(1, GetId());        //Source
    DbResult stepStatus = statement->Step();
    if (BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false && "Error deleting PhotoIsTakenByCamera Relationship");
        }
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Photo::UpdatePhotoIsTakenByCameraRelationship(DgnDbR dgndb) const
    {
    //Delete old one 
    DeletePhotoIsTakenByCameraRelationship(dgndb);
    //and then insert new one
    InsertPhotoIsTakenByCameraRelationship(dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Photo::_OnUpdated(DgnElementCR original) const
    {
    T_Super::_OnUpdated(original);

    auto other = dynamic_cast<PhotoCP>(&original);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;
    
    //Update relationship
    if (GetCameraId() != other->GetCameraId())
        UpdatePhotoIsTakenByCameraRelationship(GetDgnDb());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Photo::_OnDeleted() const
    {
    T_Super::_OnDeleted();
    DeletePhotoIsTakenByCameraRelationship(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Photo::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<PhotoCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;

    SetPhotoId(other->GetPhotoId());
    SetPose(other->GetPose());
    SetCameraId(other->GetCameraId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PhotoElementId Photo::QueryForIdByLabel(DgnDbR dgndb, Utf8CP label)
    {
    Utf8CP ecSql = "SELECT Photo.[ECInstanceId] FROM " BDCP_SCHEMA(BDCP_CLASS_Photo) " Photo " \
        "WHERE Photo.Label=?";

    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (!statement.IsValid())
        {
        BeAssert(statement.IsValid() && "Error preparing query. Check if DataCapture schema has been imported.");
        return PhotoElementId();
        }

    statement->BindText(1, label, IECSqlBinder::MakeCopy::No);

    DbResult stepStatus = statement->Step();
    if (stepStatus != BE_SQLITE_ROW)
        return PhotoElementId();

    return statement->GetValueId<PhotoElementId>(0);
    }


END_BENTLEY_DATACAPTURE_NAMESPACE


/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Pose.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

HANDLER_DEFINE_MEMBERS(PoseHandler)

#define Pose_PROPNAME_Pose_Center              "Center"
#define Pose_PROPNAME_Pose_Rotation            "Rotation"
#define Pose_PROPNAME_Pose_Rotation_M_00       "M_00"
#define Pose_PROPNAME_Pose_Rotation_M_01       "M_01"
#define Pose_PROPNAME_Pose_Rotation_M_02       "M_02"
#define Pose_PROPNAME_Pose_Rotation_M_10       "M_10"
#define Pose_PROPNAME_Pose_Rotation_M_11       "M_11"
#define Pose_PROPNAME_Pose_Rotation_M_12       "M_12"
#define Pose_PROPNAME_Pose_Rotation_M_20       "M_20"
#define Pose_PROPNAME_Pose_Rotation_M_21       "M_21"
#define Pose_PROPNAME_Pose_Rotation_M_22       "M_22"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECSqlStatus RotationMatrixType::BindParameter(IECSqlStructBinder& binder, RotationMatrixTypeCR val)
    {
    if (ECSqlStatus::Success != binder.GetMember(Pose_PROPNAME_Pose_Rotation_M_00).BindDouble(val.GetComponentByRowAndColumn(0, 0)) ||
        ECSqlStatus::Success != binder.GetMember(Pose_PROPNAME_Pose_Rotation_M_01).BindDouble(val.GetComponentByRowAndColumn(0, 1)) ||
        ECSqlStatus::Success != binder.GetMember(Pose_PROPNAME_Pose_Rotation_M_02).BindDouble(val.GetComponentByRowAndColumn(0, 2)) ||
        ECSqlStatus::Success != binder.GetMember(Pose_PROPNAME_Pose_Rotation_M_10).BindDouble(val.GetComponentByRowAndColumn(1, 0)) ||
        ECSqlStatus::Success != binder.GetMember(Pose_PROPNAME_Pose_Rotation_M_11).BindDouble(val.GetComponentByRowAndColumn(1, 1)) ||
        ECSqlStatus::Success != binder.GetMember(Pose_PROPNAME_Pose_Rotation_M_12).BindDouble(val.GetComponentByRowAndColumn(1, 2)) ||
        ECSqlStatus::Success != binder.GetMember(Pose_PROPNAME_Pose_Rotation_M_20).BindDouble(val.GetComponentByRowAndColumn(2, 0)) ||
        ECSqlStatus::Success != binder.GetMember(Pose_PROPNAME_Pose_Rotation_M_21).BindDouble(val.GetComponentByRowAndColumn(2, 1)) ||
        ECSqlStatus::Success != binder.GetMember(Pose_PROPNAME_Pose_Rotation_M_22).BindDouble(val.GetComponentByRowAndColumn(2, 2)))
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

        if      (0 == BeStringUtilities::Stricmp(Pose_PROPNAME_Pose_Rotation_M_00, memberName))
            rotation.SetComponentByRowAndColumn(0, 0,memberValue.GetDouble());
        else if (0 == BeStringUtilities::Stricmp(Pose_PROPNAME_Pose_Rotation_M_01, memberName))
            rotation.SetComponentByRowAndColumn(0, 1,memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Pose_PROPNAME_Pose_Rotation_M_02, memberName))
            rotation.SetComponentByRowAndColumn(0, 2,memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Pose_PROPNAME_Pose_Rotation_M_10, memberName))
            rotation.SetComponentByRowAndColumn(1, 0, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Pose_PROPNAME_Pose_Rotation_M_11, memberName))
            rotation.SetComponentByRowAndColumn(1, 1, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Pose_PROPNAME_Pose_Rotation_M_12, memberName))
            rotation.SetComponentByRowAndColumn(1, 2, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Pose_PROPNAME_Pose_Rotation_M_20, memberName))
            rotation.SetComponentByRowAndColumn(2, 0, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Pose_PROPNAME_Pose_Rotation_M_21, memberName))
            rotation.SetComponentByRowAndColumn(2, 1, memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(Pose_PROPNAME_Pose_Rotation_M_22, memberName))
            rotation.SetComponentByRowAndColumn(2, 2, memberValue.GetInt());
        else
            BeAssert(false);
        }
    return rotation;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PoseHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(Pose_PROPNAME_Pose_Center);
    params.Add(Pose_PROPNAME_Pose_Rotation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PosePtr Pose::Create(Dgn::SpatialModelR model)
    {
    DgnClassId classId = QueryClassId(model.GetDgnDb());
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(BDCP_CATEGORY_Pose, model.GetDgnDb());

    PosePtr cp = new Pose(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId));
    return cp;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Pose::CreateCode(Dgn::DgnDbR db, Utf8StringCR value) 
    {
    Utf8PrintfString internalValue("%s",value.c_str());
    return DataCaptureDomain::CreateCode(db,BDCP_CLASS_Pose,internalValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Pose::_GenerateDefaultCode() const
    {
    Utf8String defaultName = DataCaptureDomain::BuildDefaultName(MyECClassName(), GetElementId());

    return CreateCode(GetDgnDb(), defaultName);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Pose::Pose(CreateParams const& params): T_Super(params), m_rotation(RotationMatrixType::FromIdentity())
    {
    m_center = {0.0,0.0,0.0};
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool Pose::IsEqual(PoseCR rhs) const
    {
    if (m_rotation.IsEqual(rhs.m_rotation) && m_center.IsEqual(rhs.m_center))
        return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PoseElementId       Pose::GetId() const { return PoseElementId(GetElementId().GetValueUnchecked()); }
DPoint3d            Pose::GetCenter() const { return m_center; }
RotationMatrixType  Pose::GetRotation() const { return m_rotation; }
void                Pose::SetCenter(DPoint3dCR val) { m_center = val; }
void                Pose::SetRotation(RotationMatrixTypeCR val) { m_rotation = val; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Pose::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
    IECSqlStructBinder& structBinder = statement.BindStruct(statement.GetParameterIndex(Pose_PROPNAME_Pose_Rotation));
    if (ECSqlStatus::Success != statement.BindPoint3D(statement.GetParameterIndex(Pose_PROPNAME_Pose_Center), GetCenter()) ||
        ECSqlStatus::Success != RotationMatrixType::BindParameter(structBinder, GetRotation()))
        {
        return DgnDbStatus::BadArg;
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Pose::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Pose::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Pose::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        SetCenter(stmt.GetValuePoint3D(params.GetSelectIndex(Pose_PROPNAME_Pose_Center)));

        int rotationIndex = params.GetSelectIndex(Pose_PROPNAME_Pose_Rotation);
        IECSqlStructValue const& structValue = stmt.GetValueStruct(rotationIndex);
        SetRotation(RotationMatrixType::GetValue(structValue));
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Pose::_OnInsert()
    {
    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::_OnInserted(DgnElementP copiedFrom) const
    {
    T_Super::_OnInserted(copiedFrom);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::_OnUpdated(DgnElementCR original) const
    {
    T_Super::_OnUpdated(original);

//     auto other = dynamic_cast<PoseCP>(&original);
//     BeAssert(nullptr != other);
//     if (nullptr == other)
//         return;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::_OnDeleted() const
    {
    T_Super::_OnDeleted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<PoseCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;

    SetCenter(other->GetCenter());
    SetRotation(other->GetRotation());
    }


END_BENTLEY_DATACAPTURE_NAMESPACE


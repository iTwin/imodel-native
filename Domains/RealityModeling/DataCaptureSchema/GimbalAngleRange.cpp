/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/GimbalAngleRange.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

HANDLER_DEFINE_MEMBERS(GimbalAngleRangeHandler)

#define POSE_PROPNAME_MinimumAngle            "MinimumAngle"
#define POSE_PROPNAME_MaximumAngle            "MaximumAngle"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GimbalAngleRangeHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(POSE_PROPNAME_MinimumAngle);
    params.Add(POSE_PROPNAME_MaximumAngle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GimbalAngleRangePtr GimbalAngleRange::Create(Dgn::SpatialModelR model)
    {
    DgnClassId classId = QueryClassId(model.GetDgnDb());
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(BDCP_CATEGORY_AcquisitionDevice, model.GetDgnDb());

    GimbalAngleRangePtr cp = new GimbalAngleRange(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId));
    return cp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode GimbalAngleRange::CreateCode(Dgn::DgnDbR db, Utf8StringCR value)
    {
    Utf8PrintfString internalValue("%s", value.c_str());
    return DataCaptureDomain::CreateCode(db, BDCP_CLASS_GimbalAngleRange, internalValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode GimbalAngleRange::_GenerateDefaultCode() const
    {
    Utf8String defaultName = DataCaptureDomain::BuildDefaultName(MyECClassName(), GetElementId());

    return CreateCode(GetDgnDb(), defaultName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GimbalAngleRange::GimbalAngleRange(CreateParams const& params)
:T_Super(params)
    {
    m_minimumAngle = Angle();
    m_maximumAngle = Angle();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GimbalAngleRangeElementId   GimbalAngleRange::GetId() const { return GimbalAngleRangeElementId(GetElementId().GetValueUnchecked()); }
void            GimbalAngleRange::SetMinimumAngle(AngleCR val) { m_minimumAngle = val; }
AngleCR         GimbalAngleRange::GetMinimumAngle() const { return m_minimumAngle; }
void            GimbalAngleRange::SetMaximumAngle(AngleCR val) { m_maximumAngle = val; }
AngleCR         GimbalAngleRange::GetMaximumAngle() const { return m_maximumAngle; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GimbalAngleRange::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
    if (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_MinimumAngle), GetMinimumAngle().Radians()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_MaximumAngle), GetMaximumAngle().Radians()))
        {
        return DgnDbStatus::BadArg;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GimbalAngleRange::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GimbalAngleRange::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GimbalAngleRange::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        //read GimbalAngleRange properties
        SetMinimumAngle(Angle::FromRadians(stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_MinimumAngle))));
        SetMaximumAngle(Angle::FromRadians(stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_MaximumAngle))));
        }

    return status;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GimbalAngleRange::_OnInsert()
    {   
    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GimbalAngleRange::_OnInserted(DgnElementP copiedFrom) const
    {
    T_Super::_OnInserted(copiedFrom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GimbalAngleRange::_OnUpdated(DgnElementCR original) const
    {
    T_Super::_OnUpdated(original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GimbalAngleRange::_OnDeleted() const
    {
    T_Super::_OnDeleted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GimbalAngleRange::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<GimbalAngleRangeCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;
    SetMinimumAngle(other->GetMinimumAngle());
    SetMaximumAngle(other->GetMaximumAngle());
    }

END_BENTLEY_DATACAPTURE_NAMESPACE


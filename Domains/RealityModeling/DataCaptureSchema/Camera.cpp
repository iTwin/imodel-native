/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Camera.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

HANDLER_DEFINE_MEMBERS(CameraHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
//     params.Add(Camera_PROPNAME_PLANID);
//     params.Add(Camera_PROPNAME_OUTLINEINDEX);
//     params.Add(Camera_PROPNAME_USERPROPERTIESINDEX);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraPtr Camera::Create(Dgn::SpatialModelR model)
    {
    DgnClassId classId = QueryClassId(model.GetDgnDb());
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(BDCP_CATEGORY_Camera, model.GetDgnDb());

    CameraPtr cp = new Camera(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId));
    return cp;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
CameraCPtr Camera::Get(DgnDbCR dgndb, Dgn::DgnElementId cameraId)
    {
    DgnElementCPtr element = dgndb.Elements().GetElement(cameraId);
    return CameraCPtr(dynamic_cast<Camera const*> (element.get()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
CameraPtr Camera::GetForEdit(DgnDbCR dgndb, Dgn::DgnElementId cameraId)
    {
    return dgndb.Elements().GetForEdit<Camera>(cameraId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
CameraId Camera::Insert()
    {
    CameraCPtr wb = GetDgnDb().Elements().Insert<Camera>(*this);
    if (!wb.IsValid())
        {
        BeAssert(false && "Could not insert Camera. One possibility is that a Camera with the same label exists already in the immediate parent, or the label is otherwise empty");
        return CameraId();
        }

    return CameraId(wb->GetElementId().GetValue());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
BentleyStatus Camera::Update()
    {
    CameraCPtr wb = GetDgnDb().Elements().Update<Camera>(*this);
    if (!wb.IsValid())
        {
        BeAssert(false && "Could not update Camera. One possibility is that a WorkBreakdown with the same label exists already in the immediate parent, or the label is otherwise empty");
        return ERROR;
        }

    return SUCCESS;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus Camera::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
//     if (ECSqlStatus::Success != statement.BindId(statement.GetParameterIndex(Camera_PROPNAME_PLANID), GetPlanId()) ||
//         ECSqlStatus::Success != statement.BindInt(statement.GetParameterIndex(Camera_PROPNAME_OUTLINEINDEX), GetOutlineIndex()) ||
//         ECSqlStatus::Success != BindUserProperties(statement, statement.GetParameterIndex(Camera_PROPNAME_USERPROPERTIESINDEX)))
//         {
//         return DgnDbStatus::BadArg;
//         }
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    09/2015
//---------------------------------------------------------------------------------------
DgnDbStatus Camera::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus Camera::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Camera::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
//         SetPlanId(stmt.GetValueId<PlanId>(params.GetSelectIndex(Camera_PROPNAME_PLANID)));
//         SetOutlineIndex(stmt.GetValueInt(params.GetSelectIndex(Camera_PROPNAME_OUTLINEINDEX)));
//         SetUserProperties(Camera::GetUserPropertiesFromStatement(stmt, params.GetSelectIndex(Camera_PROPNAME_USERPROPERTIESINDEX)));
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/2015
//---------------------------------------------------------------------------------------
DgnDbStatus Camera::_OnInsert()
    {
//     PlanningModelP planningModel = dynamic_cast<PlanningModelP> (GetModel().get());
//     if (nullptr == planningModel)
//         {
//         BeAssert(false && "Can insert Camera only in a PlanningModel");
//         return DgnDbStatus::WrongModel;
//         }

    return T_Super::_OnInsert();
    }

END_BENTLEY_DATACAPTURE_NAMESPACE


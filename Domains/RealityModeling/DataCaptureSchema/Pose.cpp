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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PoseHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
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
Pose::Pose(CreateParams const& params): T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool Pose::IsEqual(PoseCR rhs) const
    {
    RotMatrix lhsRotMatrix(GetRotMatrix());
    RotMatrix rhsRotMatrix(rhs.GetRotMatrix());
    if (lhsRotMatrix.IsEqual(rhsRotMatrix) && GetCenter().IsEqual(rhs.GetCenter()))
        return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PoseElementId Pose::GetId() const 
    { return PoseElementId(GetElementId().GetValueUnchecked()); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCR Pose::GetCenter() const 
    {
    return GetPlacement().GetOrigin();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RotMatrix  Pose::GetRotMatrix() const 
    {
    return GetPlacement().GetAngles().ToRotMatrix();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetCenter(DPoint3dCR val) 
    {
    Placement3d placement(GetPlacement());
    //Tricky: We must initialize placement bounding box otherwise IsValid will failed on the element
    //        placement and it will never be saved.
    placement.GetElementBoxR().InitFrom(val);
    placement.GetOriginR() = val;
    SetPlacement(placement);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool Pose::SetRotMatrix(RotMatrixCR val) 
    {
    Placement3d placement(GetPlacement());
    YawPitchRollAngles newPlacementAngles;
    if (YawPitchRollAngles::TryFromRotMatrix(placement.GetAnglesR(), val))
        {
        SetPlacement(placement);
        return true;
        }
    BeAssert(!"Cannot transform rotMatrix into YawPitchRollAngles");
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Pose::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
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
    return T_Super::_ReadSelectParams(stmt, params);
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
    }


END_BENTLEY_DATACAPTURE_NAMESPACE


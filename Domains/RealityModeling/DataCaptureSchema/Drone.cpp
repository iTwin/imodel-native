/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Drone.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

HANDLER_DEFINE_MEMBERS(DroneHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void DroneHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DronePtr Drone::Create(Dgn::SpatialModelR model, GimbalElementId gimbalId)
    {
    if (!gimbalId.IsValid())
        {
        BeAssert(false && "Cannot create a PhotoPlan with an invalid gimbalId");
        return nullptr;
        }

    DgnClassId classId = QueryClassId(model.GetDgnDb());
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(BDCP_CATEGORY_AcquisitionDevice, model.GetDgnDb());

    DronePtr cp = new Drone(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId), gimbalId);
    return cp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Drone::CreateCode(Dgn::DgnDbR db, Utf8StringCR value)
    {
    Utf8PrintfString internalValue("%s", value.c_str());
    return DataCaptureDomain::CreateCode(db, BDCP_CLASS_Drone, internalValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Drone::_GenerateDefaultCode() const
    {
    Utf8String defaultName = DataCaptureDomain::BuildDefaultName(MyECClassName(), GetElementId());

    return CreateCode(GetDgnDb(), defaultName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Drone::Drone(CreateParams const& params) :
    T_Super(params)
    {
    m_gimbalId = GimbalElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Drone::Drone(CreateParams const& params, GimbalElementId gimbalId)
:T_Super(params)
    {
    SetGimbalElementId(gimbalId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DroneElementId Drone::GetId() const { return DroneElementId(GetElementId().GetValueUnchecked()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GimbalElementId Drone::QueryDroneHasGimbalRelationship(DgnDbR dgndb, DroneElementId droneElmId)
    {
    Utf8CP ecSql = "SELECT [TargetECInstanceId]  FROM " BDCP_SCHEMA(BDCP_REL_DroneHasGimbal) " WHERE SourceECInstanceId=?";

    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (!statement.IsValid())
        {
        BeAssert(statement.IsValid() && "Error preparing query. Check if DataCapture schema has been imported.");
        return GimbalElementId();
        }

    statement->BindId(1, droneElmId);

    DbResult stepStatus = statement->Step();
    if (stepStatus != BE_SQLITE_ROW)
        return GimbalElementId();

    return statement->GetValueId<GimbalElementId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GimbalElementId Drone::GetGimbalElementId() const
    {
    //Query and cache the result
    if (!m_gimbalId.IsValid())
        m_gimbalId = QueryDroneHasGimbalRelationship(GetDgnDb(), GetId());
    return m_gimbalId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Drone::SetGimbalElementId(GimbalElementId gimbalId)
    {
    m_gimbalId = gimbalId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Drone::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Drone::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Drone::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Drone::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    return T_Super::_ReadSelectParams(stmt, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Drone::_OnInsert()
    {
    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Drone::_OnInserted(DgnElementP copiedFrom) const
    {
    T_Super::_OnInserted(copiedFrom);

    //Update relationship
    InsertDroneHasGimbalRelationship(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Drone::InsertDroneHasGimbalRelationship(Dgn::DgnDbR dgndb) const
    {
    InsertDroneHasGimbalRelationship(dgndb, GetId(), GetGimbalElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Drone::InsertDroneHasGimbalRelationship(DgnDbR dgndb, DroneElementId droneElmId, GimbalElementId gimbalId)
    {
    if (!droneElmId.IsValid() || !gimbalId.IsValid())
        {
        BeAssert(false && "Attempt to add DroneHasGimbal relationship");
        return ERROR;
        }

    Utf8CP ecSql = "INSERT INTO " BDCP_SCHEMA(BDCP_REL_DroneHasGimbal) " (SourceECInstanceId, TargetECInstanceId) VALUES(?, ?)";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());

    statement->BindId(1, droneElmId);
    statement->BindId(2, gimbalId);

    DbResult stepStatus = statement->Step();
    if (BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false && "Error creating DroneHasGimbal Relationship");
        return ERROR;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Drone::DeleteDroneHasGimbalRelationship(DgnDbR dgndb) const
    {
    if (!GetId().IsValid())
        {
        BeAssert(false && "Attempt to delete an invalid DroneHasGimbal relationship");
        return;
        }

    //Delete old one 
    Utf8CP ecSql = "DELETE FROM " BDCP_SCHEMA(BDCP_REL_DroneHasGimbal) " WHERE SourceECInstanceId=?";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());
    statement->BindId(1, GetId());        //Source
    DbResult stepStatus = statement->Step();
    if (BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false && "Error deleting PhotoPlanHasOperationZone Relationship");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Drone::UpdateDroneHasGimbalRelationship(DgnDbR dgndb) const
    {
    //Delete old one 
    DeleteDroneHasGimbalRelationship(dgndb);
    //and then insert new one
    InsertDroneHasGimbalRelationship(dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Drone::_OnUpdated(DgnElementCR original) const
    {
    T_Super::_OnUpdated(original);

    auto other = dynamic_cast<DroneCP>(&original);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;
   
    //Update relationship
    if (GetGimbalElementId() != other->GetGimbalElementId())
        UpdateDroneHasGimbalRelationship(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                   03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus Drone::_OnDelete() const
    {
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Drone::_OnDeleted() const
    {
    T_Super::_OnDeleted();

    DeleteDroneHasGimbalRelationship(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Drone::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<DroneCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;

    SetGimbalElementId(other->GetGimbalElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DroneElementId Drone::QueryForIdByLabel(DgnDbR dgndb, Utf8CP label)
    {
    Utf8CP ecSql = "SELECT Drone.[ECInstanceId] FROM " BDCP_SCHEMA(BDCP_CLASS_Drone) " Drone " \
                   "WHERE Drone.Label=?";

    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (!statement.IsValid())
        {
        BeAssert(statement.IsValid() && "Error preparing query. Check if DataCapture schema has been imported.");
        return DroneElementId();
        }

    statement->BindText(1, label, IECSqlBinder::MakeCopy::No);

    DbResult stepStatus = statement->Step();
    if (stepStatus != BE_SQLITE_ROW)
        return DroneElementId();

    return statement->GetValueId<DroneElementId>(0);
    }
	
END_BENTLEY_DATACAPTURE_NAMESPACE


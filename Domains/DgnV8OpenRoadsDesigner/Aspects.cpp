/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnV8OpenRoadsDesignerInternal.h"
#include <DgnV8OpenRoadsDesigner/Aspects.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Diego.Diaz                              10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorSurfaceAspectPtr CorridorSurfaceAspect::Create(bool isTopMesh, bool isBottomMesh, Utf8CP description, 
                                                       Utf8CP corridorName, Utf8CP horizontalName, Utf8CP profileName)
    {
    return new CorridorSurfaceAspect(isTopMesh, isBottomMesh, description, corridorName, horizontalName, profileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorSurfaceAspectCP CorridorSurfaceAspect::Get(DgnElementCR el)
    {
    return DgnElement::UniqueAspect::Get<CorridorSurfaceAspect>(el, *QueryClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorSurfaceAspectP CorridorSurfaceAspect::GetP(DgnElementR el)
    {
    return dynamic_cast<CorridorSurfaceAspectP>(GetAspectP(el, *QueryClass(el.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CorridorSurfaceAspect::Set(DgnElementR el, CorridorSurfaceAspectR aspect)
    {
    SetAspect(el, aspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CorridorSurfaceAspect::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT " V8ORD_PROP_CorridorSurfaceAspect_IsTopMesh ", " V8ORD_PROP_CorridorSurfaceAspect_IsBottomMesh ", "
        V8ORD_PROP_CorridorSurfaceAspect_Description ", " V8ORD_PROP_CorridorSurfaceAspect_CorridorName ", "
        V8ORD_PROP_CorridorSurfaceAspect_HorizontalName ", " V8ORD_PROP_CorridorSurfaceAspect_ProfileName
        " FROM " V8ORD_SCHEMA(V8ORD_CLASS_CorridorSurfaceAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_isTopMesh = stmtPtr->GetValueBoolean(0);
    m_isBottomMesh = stmtPtr->GetValueBoolean(1);
    m_description = stmtPtr->GetValueText(2);
    m_corridorName = stmtPtr->GetValueText(3);
    m_horizontalName = stmtPtr->GetValueText(4);
    m_profileName = stmtPtr->GetValueText(5);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CorridorSurfaceAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " V8ORD_SCHEMA(V8ORD_CLASS_CorridorSurfaceAspect) " SET " V8ORD_PROP_CorridorSurfaceAspect_IsTopMesh " = ?, "
        V8ORD_PROP_CorridorSurfaceAspect_IsBottomMesh " = ?, " V8ORD_PROP_CorridorSurfaceAspect_Description " = ?, "
        V8ORD_PROP_CorridorSurfaceAspect_CorridorName " = ?, " V8ORD_PROP_CorridorSurfaceAspect_HorizontalName " = ?, "
        V8ORD_PROP_CorridorSurfaceAspect_ProfileName " = ? WHERE Element.Id = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindBoolean(1, m_isTopMesh);
    stmtPtr->BindBoolean(2, m_isBottomMesh);
    stmtPtr->BindText(3, m_description.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindText(4, m_corridorName.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindText(5, m_horizontalName.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindText(6, m_profileName.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindId(7, el.GetElementId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CorridorSurfaceAspect::_GetPropertyValue(ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_IsTopMesh, propertyName))
        value.SetBoolean(m_isTopMesh);
    else if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_IsBottomMesh, propertyName))
        value.SetBoolean(m_isBottomMesh);
    else if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_Description, propertyName))
        value.SetUtf8CP(m_description.c_str());
    else if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_CorridorName, propertyName))
        value.SetUtf8CP(m_corridorName.c_str());
    else if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_HorizontalName, propertyName))
        value.SetUtf8CP(m_horizontalName.c_str());
    else if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_ProfileName, propertyName))
        value.SetUtf8CP(m_profileName.c_str());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CorridorSurfaceAspect::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_IsTopMesh, propertyName))
        SetIsTopMesh(value.GetBoolean());
    else if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_IsBottomMesh, propertyName))
        SetIsBottomMesh(value.GetBoolean());
    else if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_Description, propertyName))
        SetDescription(value.GetUtf8CP());
    else if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_CorridorName, propertyName))
        SetCorridorName(value.GetUtf8CP());
    else if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_HorizontalName, propertyName))
        SetHorizontalName(value.GetUtf8CP());
    else if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_ProfileName, propertyName))
        SetProfileName(value.GetUtf8CP());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Diego.Diaz                              10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
FeatureAspectPtr FeatureAspect::Create(Utf8CP name, Utf8CP definitionName, Utf8CP description)
    {
    return new FeatureAspect(name, definitionName, description);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
FeatureAspectCP FeatureAspect::Get(DgnElementCR el)
    {
    return DgnElement::UniqueAspect::Get<FeatureAspect>(el, *QueryClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
FeatureAspectP FeatureAspect::GetP(DgnElementR el)
    {
    return dynamic_cast<FeatureAspectP>(GetAspectP(el, *QueryClass(el.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void FeatureAspect::Set(DgnElementR el, FeatureAspectR aspect)
    {
    SetAspect(el, aspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus FeatureAspect::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT " V8ORD_PROP_FeatureAspect_Name ", " V8ORD_PROP_FeatureAspect_DefinitionName ", " V8ORD_PROP_FeatureAspect_Description
        " FROM " V8ORD_SCHEMA(V8ORD_CLASS_FeatureAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_name = stmtPtr->GetValueText(0);
    m_definitionName = stmtPtr->GetValueText(1);
    m_description = stmtPtr->GetValueText(2);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus FeatureAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " V8ORD_SCHEMA(V8ORD_CLASS_FeatureAspect) " SET " V8ORD_PROP_FeatureAspect_Name " = ?, "
        V8ORD_PROP_FeatureAspect_DefinitionName " = ?, " V8ORD_PROP_FeatureAspect_Description " = ? WHERE Element.Id = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindText(1, m_name.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindText(2, m_definitionName.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindText(3, m_description.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindId(4, el.GetElementId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus FeatureAspect::_GetPropertyValue(ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (0 == strcmp(V8ORD_PROP_FeatureAspect_Name, propertyName))
        value.SetUtf8CP(m_name.c_str());
    else if (0 == strcmp(V8ORD_PROP_FeatureAspect_DefinitionName, propertyName))
        value.SetUtf8CP(m_definitionName.c_str());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus FeatureAspect::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 == strcmp(V8ORD_PROP_FeatureAspect_Name, propertyName))
        SetName(value.GetUtf8CP());
    else if (0 == strcmp(V8ORD_PROP_FeatureAspect_DefinitionName, propertyName))
        SetDefinitionName(value.GetUtf8CP());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TemplateDropAspectPtr TemplateDropAspect::Create(double interval, Utf8CP templateName, Utf8CP description)
    {
    return new TemplateDropAspect(interval, templateName, description);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TemplateDropAspectCP TemplateDropAspect::Get(DgnElementCR el)
    {
    return DgnElement::UniqueAspect::Get<TemplateDropAspect>(el, *QueryClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TemplateDropAspectP TemplateDropAspect::GetP(DgnElementR el)
    {
    return dynamic_cast<TemplateDropAspectP>(GetAspectP(el, *QueryClass(el.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void TemplateDropAspect::Set(DgnElementR el, TemplateDropAspectR aspect)
    {
    SetAspect(el, aspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TemplateDropAspect::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT " V8ORD_PROP_TemplateDropAspect_Interval ", " V8ORD_PROP_TemplateDropAspect_TemplateName ", " V8ORD_PROP_TemplateDropAspect_Description
        " FROM " V8ORD_SCHEMA(V8ORD_CLASS_TemplateDropAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_interval = stmtPtr->GetValueDouble(0);
    m_templateName = stmtPtr->GetValueText(1);
    m_description = stmtPtr->GetValueText(2);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TemplateDropAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " V8ORD_SCHEMA(V8ORD_CLASS_TemplateDropAspect) " SET " V8ORD_PROP_TemplateDropAspect_Interval " = ?, "
        V8ORD_PROP_TemplateDropAspect_TemplateName " = ?, " V8ORD_PROP_TemplateDropAspect_Description " = ? WHERE Element.Id = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindDouble(1, m_interval);
    stmtPtr->BindText(2, m_templateName.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindText(3, m_description.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindId(4, el.GetElementId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TemplateDropAspect::_GetPropertyValue(ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (0 == strcmp(V8ORD_PROP_TemplateDropAspect_Interval, propertyName))
        value.SetDouble(m_interval);
    else if (0 == strcmp(V8ORD_PROP_TemplateDropAspect_TemplateName, propertyName))
        value.SetUtf8CP(m_templateName.c_str());
    else if (0 == strcmp(V8ORD_PROP_TemplateDropAspect_Description, propertyName))
        value.SetUtf8CP(m_description.c_str());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TemplateDropAspect::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 == strcmp(V8ORD_PROP_TemplateDropAspect_Interval, propertyName))
        SetInterval(value.GetDouble());
    else if (0 == strcmp(V8ORD_PROP_TemplateDropAspect_TemplateName, propertyName))
        SetTemplateName(value.GetUtf8CP());
    else if (0 == strcmp(V8ORD_PROP_TemplateDropAspect_Description, propertyName))
        SetDescription(value.GetUtf8CP());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StationRangeAspectPtr StationRangeAspect::Create(double startStation, double endStation)
    {
    return new StationRangeAspect(startStation, endStation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StationRangeAspectCP StationRangeAspect::Get(DgnElementCR el)
    {
    return DgnElement::UniqueAspect::Get<StationRangeAspect>(el, *QueryClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StationRangeAspectP StationRangeAspect::GetP(DgnElementR el)
    {
    return dynamic_cast<StationRangeAspectP>(GetAspectP(el, *QueryClass(el.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void StationRangeAspect::Set(DgnElementR el, StationRangeAspectR aspect)
    {
    SetAspect(el, aspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus StationRangeAspect::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT " V8ORD_PROP_StationRangeAspect_StartStation ", " V8ORD_PROP_StationRangeAspect_EndStation
        " FROM " V8ORD_SCHEMA(V8ORD_CLASS_StationRangeAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_startStation = stmtPtr->GetValueDouble(0);
    m_endStation = stmtPtr->GetValueDouble(1);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus StationRangeAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " V8ORD_SCHEMA(V8ORD_CLASS_StationRangeAspect) " SET " V8ORD_PROP_StationRangeAspect_StartStation " = ?, "
        V8ORD_PROP_StationRangeAspect_EndStation " = ? WHERE Element.Id = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindDouble(1, m_startStation);
    stmtPtr->BindDouble(2, m_endStation);
    stmtPtr->BindId(3, el.GetElementId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus StationRangeAspect::_GetPropertyValue(ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (0 == strcmp(V8ORD_PROP_StationRangeAspect_StartStation, propertyName))
        value.SetDouble(m_startStation);
    else if (0 == strcmp(V8ORD_PROP_StationRangeAspect_EndStation, propertyName))
        value.SetDouble(m_endStation);
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus StationRangeAspect::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 == strcmp(V8ORD_PROP_StationRangeAspect_StartStation, propertyName))
        SetStartStation(value.GetDouble());
    else if (0 == strcmp(V8ORD_PROP_StationRangeAspect_EndStation, propertyName))
        SetEndStation(value.GetDouble());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SuperelevationAspectPtr SuperelevationAspect::Create(Utf8CP name, double normalCrossSlope)
    {
    return new SuperelevationAspect(name, normalCrossSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SuperelevationAspectCP SuperelevationAspect::Get(DgnElementCR el)
    {
    return DgnElement::UniqueAspect::Get<SuperelevationAspect>(el, *QueryClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SuperelevationAspectP SuperelevationAspect::GetP(DgnElementR el)
    {
    return dynamic_cast<SuperelevationAspectP>(GetAspectP(el, *QueryClass(el.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SuperelevationAspect::Set(DgnElementR el, SuperelevationAspectR aspect)
    {
    SetAspect(el, aspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SuperelevationAspect::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT " V8ORD_PROP_SuperelevationAspect_Name", " V8ORD_PROP_SuperelevationAspect_NormalCrossSlope
        " FROM " V8ORD_SCHEMA(V8ORD_CLASS_SuperelevationAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_name = stmtPtr->GetValueText(0);
    m_normalCrossSlope = stmtPtr->GetValueDouble(1);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SuperelevationAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " V8ORD_SCHEMA(V8ORD_CLASS_SuperelevationAspect) " SET " V8ORD_PROP_SuperelevationAspect_Name " = ?, "
        V8ORD_PROP_SuperelevationAspect_NormalCrossSlope " = ? WHERE Element.Id = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindText(1, m_name.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindDouble(2, m_normalCrossSlope);
    stmtPtr->BindId(3, el.GetElementId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SuperelevationAspect::_GetPropertyValue(ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (0 == strcmp(V8ORD_PROP_SuperelevationAspect_Name, propertyName))
        value.SetUtf8CP(m_name.c_str());
    else if (0 == strcmp(V8ORD_PROP_SuperelevationAspect_NormalCrossSlope, propertyName))
        value.SetDouble(m_normalCrossSlope);
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SuperelevationAspect::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 == strcmp(V8ORD_PROP_SuperelevationAspect_Name, propertyName))
        SetName(value.GetUtf8CP());
    else if (0 == strcmp(V8ORD_PROP_SuperelevationAspect_NormalCrossSlope, propertyName))
        SetNormalCrossSlope(value.GetDouble());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorAspectPtr CorridorAspect::Create(Utf8CP name, Utf8CP horizontalName, Utf8CP activeProfileName)
    {
    return new CorridorAspect(name, horizontalName, activeProfileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorAspectCP CorridorAspect::Get(DgnElementCR el)
    {
    return DgnElement::UniqueAspect::Get<CorridorAspect>(el, *QueryClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorAspectP CorridorAspect::GetP(DgnElementR el)
    {
    return dynamic_cast<CorridorAspectP>(GetAspectP(el, *QueryClass(el.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void CorridorAspect::Set(DgnElementR el, CorridorAspectR aspect)
    {
    SetAspect(el, aspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CorridorAspect::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT " V8ORD_PROP_CorridorAspect_Name ", " V8ORD_PROP_CorridorAspect_HorizontalName ", " V8ORD_PROP_CorridorAspect_ActiveProfileName
        " FROM " V8ORD_SCHEMA(V8ORD_CLASS_CorridorAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_name = stmtPtr->GetValueText(0);
    m_horizontalName = stmtPtr->GetValueText(1);
    m_activeProfileName = stmtPtr->GetValueText(2);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CorridorAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " V8ORD_SCHEMA(V8ORD_CLASS_CorridorAspect) " SET " V8ORD_PROP_CorridorAspect_Name " = ?, "
        V8ORD_PROP_CorridorAspect_HorizontalName " = ?, " V8ORD_PROP_CorridorAspect_ActiveProfileName " = ?"
        "WHERE Element.Id = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindText(1, m_name.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindText(2, m_horizontalName.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindText(3, m_activeProfileName.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindId(4, el.GetElementId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CorridorAspect::_GetPropertyValue(ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (0 == strcmp(V8ORD_PROP_CorridorAspect_Name, propertyName))
        value.SetUtf8CP(m_name.c_str());
    else if (0 == strcmp(V8ORD_PROP_CorridorAspect_ActiveProfileName, propertyName))
        value.SetUtf8CP(m_activeProfileName.c_str());
    else if (0 == strcmp(V8ORD_PROP_CorridorAspect_HorizontalName, propertyName))
        value.SetUtf8CP(m_horizontalName.c_str());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CorridorAspect::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 == strcmp(V8ORD_PROP_CorridorAspect_Name, propertyName))
        SetName(value.GetUtf8CP());
    else if (0 == strcmp(V8ORD_PROP_CorridorAspect_ActiveProfileName, propertyName))
        SetActiveProfileName(value.GetUtf8CP());
    else if (0 == strcmp(V8ORD_PROP_CorridorAspect_HorizontalName, propertyName))
        SetHorizontalName(value.GetUtf8CP());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
VolumetricQuantityAspectPtr VolumetricQuantityAspect::Create(double volume, double surfaceArea)
    {
    return new VolumetricQuantityAspect(volume, surfaceArea);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
VolumetricQuantityAspectCP VolumetricQuantityAspect::Get(DgnElementCR el)
    {
    return DgnElement::UniqueAspect::Get<VolumetricQuantityAspect>(el, *QueryClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
VolumetricQuantityAspectP VolumetricQuantityAspect::GetP(DgnElementR el)
    {
    return dynamic_cast<VolumetricQuantityAspectP>(GetAspectP(el, *QueryClass(el.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void VolumetricQuantityAspect::Set(DgnElementR el, VolumetricQuantityAspectR aspect)
    {
    SetAspect(el, aspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus VolumetricQuantityAspect::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT " V8ORD_PROP_VolumetricQuantityAspect_Volume ", " V8ORD_PROP_VolumetricQuantityAspect_SlopedArea
        " FROM " V8ORD_SCHEMA(V8ORD_CLASS_VolumetricQuantityAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_volume = stmtPtr->GetValueDouble(0);
    m_slopedArea = stmtPtr->GetValueDouble(1);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus VolumetricQuantityAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " V8ORD_SCHEMA(V8ORD_CLASS_VolumetricQuantityAspect) " SET " V8ORD_PROP_VolumetricQuantityAspect_Volume " = ?, "
        V8ORD_PROP_VolumetricQuantityAspect_SlopedArea " = ? WHERE Element.Id = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindDouble(1, m_volume);
    stmtPtr->BindDouble(2, m_slopedArea);
    stmtPtr->BindId(3, el.GetElementId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus VolumetricQuantityAspect::_GetPropertyValue(ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (0 == strcmp(V8ORD_PROP_VolumetricQuantityAspect_Volume, propertyName))
        value.SetDouble(m_volume);
    else if (0 == strcmp(V8ORD_PROP_VolumetricQuantityAspect_SlopedArea, propertyName))
        value.SetDouble(m_slopedArea);
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus VolumetricQuantityAspect::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 == strcmp(V8ORD_PROP_VolumetricQuantityAspect_Volume, propertyName))
        SetVolume(value.GetDouble());
    else if (0 == strcmp(V8ORD_PROP_VolumetricQuantityAspect_SlopedArea, propertyName))
        SetSlopedArea(value.GetDouble());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LinearQuantityAspectPtr LinearQuantityAspect::Create(double length)
    {
    return new LinearQuantityAspect(length);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LinearQuantityAspectCP LinearQuantityAspect::Get(DgnElementCR el)
    {
    return DgnElement::UniqueAspect::Get<LinearQuantityAspect>(el, *QueryClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LinearQuantityAspectP LinearQuantityAspect::GetP(DgnElementR el)
    {
    return dynamic_cast<LinearQuantityAspectP>(GetAspectP(el, *QueryClass(el.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void LinearQuantityAspect::Set(DgnElementR el, LinearQuantityAspectR aspect)
    {
    SetAspect(el, aspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearQuantityAspect::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT " V8ORD_PROP_LinearQuantityAspect_Length 
        " FROM " V8ORD_SCHEMA(V8ORD_CLASS_LinearQuantityAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_length = stmtPtr->GetValueDouble(0);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearQuantityAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " V8ORD_SCHEMA(V8ORD_CLASS_LinearQuantityAspect) " SET " V8ORD_PROP_LinearQuantityAspect_Length " = ? "
        "WHERE Element.Id = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindDouble(1, m_length);
    stmtPtr->BindId(2, el.GetElementId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearQuantityAspect::_GetPropertyValue(ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (0 == strcmp(V8ORD_PROP_LinearQuantityAspect_Length, propertyName))
        value.SetDouble(m_length);
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearQuantityAspect::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 == strcmp(V8ORD_PROP_LinearQuantityAspect_Length, propertyName))
        SetLength(value.GetDouble());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DiscreteQuantityAspectPtr DiscreteQuantityAspect::Create(int32_t count)
    {
    return new DiscreteQuantityAspect(count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DiscreteQuantityAspectCP DiscreteQuantityAspect::Get(DgnElementCR el)
    {
    return DgnElement::UniqueAspect::Get<DiscreteQuantityAspect>(el, *QueryClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DiscreteQuantityAspectP DiscreteQuantityAspect::GetP(DgnElementR el)
    {
    return dynamic_cast<DiscreteQuantityAspectP>(GetAspectP(el, *QueryClass(el.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DiscreteQuantityAspect::Set(DgnElementR el, DiscreteQuantityAspectR aspect)
    {
    SetAspect(el, aspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DiscreteQuantityAspect::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT " V8ORD_PROP_DiscreteQuantityAspect_Count
        " FROM " V8ORD_SCHEMA(V8ORD_CLASS_DiscreteQuantityAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_count = stmtPtr->GetValueInt(0);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DiscreteQuantityAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " V8ORD_SCHEMA(V8ORD_CLASS_DiscreteQuantityAspect) " SET " V8ORD_PROP_DiscreteQuantityAspect_Count " = ? "
        "WHERE Element.Id = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindInt(1, m_count);
    stmtPtr->BindId(2, el.GetElementId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DiscreteQuantityAspect::_GetPropertyValue(ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (0 == strcmp(V8ORD_PROP_DiscreteQuantityAspect_Count, propertyName))
        value.SetDouble(m_count);
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DiscreteQuantityAspect::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 == strcmp(V8ORD_PROP_DiscreteQuantityAspect_Count, propertyName))
        SetCount(value.GetInteger());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentAspectPtr AlignmentAspect::Create(DPoint2d const& startPoint, DPoint2d const& endPoint, Utf8CP activeProfileName)
    {
    return new AlignmentAspect(startPoint, endPoint, activeProfileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentAspectCP AlignmentAspect::Get(DgnElementCR el)
    {
    return DgnElement::UniqueAspect::Get<AlignmentAspect>(el, *QueryClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentAspectP AlignmentAspect::GetP(DgnElementR el)
    {
    return dynamic_cast<AlignmentAspectP>(GetAspectP(el, *QueryClass(el.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentAspect::Set(DgnElementR el, AlignmentAspectR aspect)
    {
    SetAspect(el, aspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AlignmentAspect::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT " V8ORD_PROP_AlignmentAspect_ActiveProfileName ", " V8ORD_PROP_AlignmentAspect_StartPoint ", " V8ORD_PROP_AlignmentAspect_EndPoint
        " FROM " V8ORD_SCHEMA(V8ORD_CLASS_AlignmentAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_activeProfileName = stmtPtr->GetValueText(0);
    m_startPoint = stmtPtr->GetValuePoint2d(1);
    m_endPoint = stmtPtr->GetValuePoint2d(2);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AlignmentAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " V8ORD_SCHEMA(V8ORD_CLASS_AlignmentAspect) " SET " V8ORD_PROP_AlignmentAspect_ActiveProfileName " = ?, "
        V8ORD_PROP_AlignmentAspect_StartPoint " = ?, " V8ORD_PROP_AlignmentAspect_EndPoint " = ? "
        "WHERE Element.Id = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindText(1, m_activeProfileName.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindPoint2d(2, m_startPoint);
    stmtPtr->BindPoint2d(3, m_endPoint);
    stmtPtr->BindId(4, el.GetElementId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AlignmentAspect::_GetPropertyValue(ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (0 == strcmp(V8ORD_PROP_AlignmentAspect_ActiveProfileName, propertyName))
        value.SetUtf8CP(m_activeProfileName.c_str());
    else if (0 == strcmp(V8ORD_PROP_AlignmentAspect_StartPoint, propertyName))
        value.SetPoint2d(m_startPoint);
    else if (0 == strcmp(V8ORD_PROP_AlignmentAspect_EndPoint, propertyName))
        value.SetPoint2d(m_endPoint);
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AlignmentAspect::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 == strcmp(V8ORD_PROP_AlignmentAspect_ActiveProfileName, propertyName))
        SetActiveProfileName(value.GetUtf8CP());
    else if (0 == strcmp(V8ORD_PROP_AlignmentAspect_StartPoint, propertyName))
        SetStartPoint(value.GetPoint2d());
    else if (0 == strcmp(V8ORD_PROP_AlignmentAspect_EndPoint, propertyName))
        SetEndPoint(value.GetPoint2d());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }
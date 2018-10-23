/*--------------------------------------------------------------------------------------+
|
|     $Source: Aspects.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnV8OpenRoadsDesignerInternal.h"
#include <DgnV8OpenRoadsDesigner/Aspects.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Diego.Diaz                              10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorSurfaceAspectPtr CorridorSurfaceAspect::Create(bool isTopMesh, bool isBottomMesh)
    {
    return new CorridorSurfaceAspect(isTopMesh, isBottomMesh);
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
        "SELECT " V8ORD_PROP_CorridorSurfaceAspect_IsTopMesh ", " V8ORD_PROP_CorridorSurfaceAspect_IsBottomMesh 
        " FROM " V8ORD_SCHEMA(V8ORD_CLASS_CorridorSurfaceAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_isTopMesh = stmtPtr->GetValueBoolean(0);
    m_isBottomMesh = stmtPtr->GetValueBoolean(1);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CorridorSurfaceAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " V8ORD_SCHEMA(V8ORD_CLASS_CorridorSurfaceAspect) " SET " V8ORD_PROP_CorridorSurfaceAspect_IsTopMesh " = ?, "
        V8ORD_PROP_CorridorSurfaceAspect_IsBottomMesh " = ? WHERE Element.Id = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindBoolean(1, m_isTopMesh);
    stmtPtr->BindBoolean(2, m_isBottomMesh);
    stmtPtr->BindId(3, el.GetElementId());

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
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CorridorSurfaceAspect::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_IsTopMesh, propertyName) || value.IsNull())
        SetIsTopMesh(value.GetBoolean());
    else if (0 == strcmp(V8ORD_PROP_CorridorSurfaceAspect_IsBottomMesh, propertyName) || value.IsNull())
        SetIsBottomMesh(value.GetBoolean());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Diego.Diaz                              10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
FeatureAspectPtr FeatureAspect::Create(Utf8CP name, Utf8CP definitionName)
    {
    return new FeatureAspect(name, definitionName);
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
        "SELECT " V8ORD_PROP_FeatureAspect_Name ", " V8ORD_PROP_FeatureAspect_DefinitionName
        " FROM " V8ORD_SCHEMA(V8ORD_CLASS_FeatureAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_name = stmtPtr->GetValueText(0);
    m_definitionName = stmtPtr->GetValueText(1);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus FeatureAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " V8ORD_SCHEMA(V8ORD_CLASS_FeatureAspect) " SET " V8ORD_PROP_FeatureAspect_Name " = ?, "
        V8ORD_PROP_FeatureAspect_DefinitionName " = ? WHERE Element.Id = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindText(1, m_name.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindText(2, m_definitionName.c_str(), IECSqlBinder::MakeCopy::No);
    stmtPtr->BindId(3, el.GetElementId());

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
    if (0 == strcmp(V8ORD_PROP_FeatureAspect_Name, propertyName) || value.IsNull())
        SetName(value.GetUtf8CP());
    else if (0 == strcmp(V8ORD_PROP_FeatureAspect_DefinitionName, propertyName) || value.IsNull())
        SetDefinitionName(value.GetUtf8CP());
    else
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }
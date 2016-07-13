/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnLight.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define PROPNAME_Descr "Descr"
#define PROPNAME_Value "Value"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(LightDef);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::LightDef::_TEMPORARY_GetPropertyHandlingCustomAttributes(ECSqlClassParams::PropertyHandlingCustomAttributes& params) // *** WIP_AUTO_HANDLED_PROPERTIES
    {
    T_Super::_TEMPORARY_GetPropertyHandlingCustomAttributes(params);
    params.Add(PROPNAME_Descr);
    params.Add(PROPNAME_Value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LightDefinition::_BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    if (DgnDbStatus::Success == status)
        status = BindParams(stmt);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LightDefinition::_BindUpdateParams(ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    if (DgnDbStatus::Success == status)
        status = BindParams(stmt);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LightDefinition::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        Utf8String descr = stmt.GetValueText(params.GetSelectIndex(PROPNAME_Descr)),
               value = stmt.GetValueText(params.GetSelectIndex(PROPNAME_Value));

        m_data.Init(value, descr);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LightDefinition::BindParams(ECSqlStatement& stmt)
    {
    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(PROPNAME_Descr), m_data.m_descr.c_str(), IECSqlBinder::MakeCopy::No)
        || ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(PROPNAME_Value), m_data.m_value.c_str(), IECSqlBinder::MakeCopy::No))
        return DgnDbStatus::BadArg;
    else
        return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LightDefinition::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<LightDefinitionCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr != other)
        m_data = other->m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LightDefinition::CreateParams::CreateParams(DgnDbR db, Utf8StringCR name, Utf8StringCR value, Utf8StringCR descr)
  : T_Super(db, DgnModel::DictionaryId(), QueryDgnClassId(db), CreateLightDefinitionCode(name)),
    m_data(value, descr)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLightId LightDefinition::QueryLightId(DgnCode const& code, DgnDbR db)
    {
    DgnElementId elemId = db.Elements().QueryElementIdByCode(code);
    return DgnLightId(elemId.GetValueUnchecked());
    }




/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnLight.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
void LightDefinition::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);
    stmt.BindText(stmt.GetParameterIndex(PROPNAME_Descr), m_data.m_descr.c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindText(stmt.GetParameterIndex(PROPNAME_Value), m_data.m_value.c_str(), IECSqlBinder::MakeCopy::No);
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
  : T_Super(db, DgnModel::DictionaryId(), QueryDgnClassId(db), CreateCode(db, name)),
    m_data(value, descr)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLightId LightDefinition::QueryLightId(DgnDbR db, DgnCodeCR code)
    {
    DgnElementId elemId = db.Elements().QueryElementIdByCode(code);
    return DgnLightId(elemId.GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::LightDef::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);
    
    params.RegisterPropertyAccessors(layout, PROPNAME_Descr,
        [] (ECValueR value, DgnElementCR elIn)
            {
            auto& el = (LightDefinition&) elIn;
            value.SetUtf8CP(el.GetDescr().c_str());
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            auto& el = (LightDefinition&) elIn;
            el.SetDescr(value.ToString());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, PROPNAME_Value,
        [] (ECValueR value, DgnElementCR elIn)
            {
            auto& el = (LightDefinition&) elIn;
            value.SetUtf8CP(el.GetValue().c_str());
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            auto& el = (LightDefinition&) elIn;
            // *** WIP_LIGHT - validate input value before calling SetValue?
            el.SetValue(value.ToString());
            return DgnDbStatus::Success;
            });
    }
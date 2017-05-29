/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ClassRefExp.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************************** ClassNameExp *****************************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle            06/2016
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus ClassNameExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        {
        if (m_info == nullptr)
            {
            BeAssert(false);
            return FinalizeParseStatus::Error;
            }

        return FinalizeParseStatus::Completed;
        }

    return FinalizeParseStatus::Completed;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassNameExp::_CreatePropertyNameExpList(ECSqlParseContext const& ctx, std::function<void(std::unique_ptr<PropertyNameExp>&)> addDelegate) const
    {
    if (m_info == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    ClassMap const& classMap = m_info->GetMap();
    for (PropertyMap const* propertyMap : classMap.GetPropertyMaps())
        {
        std::unique_ptr<PropertyNameExp> exp = std::make_unique<PropertyNameExp>(ctx, propertyMap->GetAccessString(), *this, classMap);
        addDelegate(exp);
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ClassNameExp::_ContainProperty(Utf8CP propertyName) const
    {
    if (m_info == nullptr)
        {
        BeAssert(false);
        return false;
        }

    PropertyMap const* propertyMap = m_info->GetMap().GetPropertyMaps().Find(propertyName);
    return propertyMap != nullptr;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR ClassNameExp::_GetId() const
    {
    if (GetAlias().empty())
        return m_className;

    return GetAlias();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ClassNameExp::GetFullName() const
    {
    Utf8String fullName;
    if (!m_catalogName.empty())
        fullName.append(m_catalogName).append(".");

    if (!m_schemaAlias.empty())
        fullName.append(m_schemaAlias).append(".");

    fullName.append(m_className);

    return fullName;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ClassNameExp::_ToString() const
    {
    Utf8String str("ClassName [Catalog: ");
    str.append(m_catalogName).append(", Schema alias: ").append(m_schemaAlias);
    str.append(", Class: ").append(m_className).append(", Alias: ").append(GetAlias()).append("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ClassNameExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (!IsPolymorphic())
        ctx.AppendToECSql("ONLY ");

    ctx.AppendToECSql(GetFullName());

    if (!GetAlias().empty())
        ctx.AppendToECSql(" ").AppendToECSql(GetAlias());
    }


END_BENTLEY_SQLITE_EC_NAMESPACE

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
bool ClassNameExp::_ContainsProperty(Utf8StringCR propertyName) const
    {
    if (m_info == nullptr)
        {
        BeAssert(false);
        return false;
        }

    PropertyMap const* propertyMap = m_info->GetMap().GetPropertyMaps().Find(propertyName.c_str());
    return propertyMap != nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassNameExp::ClassNameExp(Utf8StringCR className, Utf8StringCR schemaAlias, Utf8CP tableSpace, std::shared_ptr<Info> info, bool isPolymorphic, std::unique_ptr<MemberFunctionCallExp> memberFuntionCall)
    : RangeClassRefExp(Type::ClassName, isPolymorphic), m_className(className), m_schemaAlias(schemaAlias), m_tableSpace(tableSpace), m_info(info)
    {
    if (memberFuntionCall)
        AddChild(std::move(memberFuntionCall));
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
    if (!m_tableSpace.empty())
        fullName.append("[").append(m_tableSpace).append("].");

    if (!m_schemaAlias.empty())
        fullName.append("[").append(m_schemaAlias).append("].");

    fullName.append("[").append(m_className).append("]");

    return fullName;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ClassNameExp::_ToString() const
    {
    Utf8String str("ClassName [TableSpace: ");
    str.append(m_tableSpace.empty() ? "-" : m_tableSpace).append(", Schema alias: ").append(m_schemaAlias);
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

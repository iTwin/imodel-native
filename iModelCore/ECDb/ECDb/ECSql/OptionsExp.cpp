/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/OptionsExp.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************************** OptionsExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const OptionsExp::NOECCLASSIDFILTER_OPTION = "NoECClassIdFilter";

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus OptionsExp::AddOptionExp(std::unique_ptr<OptionExp> optionExp)
    {
    Utf8CP name = optionExp->GetName();
    if (m_optionsByName.find(name) != m_optionsByName.end())
        {
        LOG.errorv("Multiple options with same name ('%s') are not supported.", name);
        return ERROR;
        }

    const size_t optionIndex = AddChild(std::move(optionExp));
    m_optionsByName[name] = optionIndex;
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool OptionsExp::HasOption(Utf8CP optionName) const
    {
    OptionExp const* option = nullptr;
    if (!TryGetOption(option, optionName))
        return false;

    if (!option->IsNameValuePair())
        return true;

    Utf8CP val = option->GetValue();
    return BeStringUtilities::Stricmp(val, "true") == 0 || BeStringUtilities::Stricmp(val, "1") == 0;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool OptionsExp::TryGetOption(OptionExp const*& exp, Utf8CP optionName) const
    {
    BeAssert(GetChildrenCount() != 0);

    if (Utf8String::IsNullOrEmpty(optionName))
        return false;

    auto it = m_optionsByName.find(optionName);
    if (it == m_optionsByName.end())
        return false;

    const size_t ix = it->second;
    BeAssert(ix < GetChildrenCount());
    exp = GetChild<OptionExp>(ix);
    BeAssert(exp != nullptr);
    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String OptionsExp::_ToECSql() const
    {
    BeAssert(GetChildrenCount() != 0);

    Utf8String ecsql("OPTIONS");
    for (Exp const* child : GetChildren())
        {
        ecsql.append(" ").append(child->ToECSql());
        }

    return std::move(ecsql);
    }


//****************************** OptionExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String OptionExp::_ToECSql() const
    {
    if (!IsNameValuePair())
        return m_name;

    Utf8String ecsql(m_name);
    ecsql.append("=").append(m_val);
    return std::move(ecsql);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************************** OptionsExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const OptionsExp::NOECCLASSIDFILTER_OPTION = "NoECClassIdFilter";

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const OptionsExp::READONLYPROPERTIESAREUPDATABLE_OPTION = "ReadonlyPropertiesAreUpdatable";

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus OptionsExp::AddOptionExp(std::unique_ptr<OptionExp> optionExp, IssueDataSource const& issues)
    {
    Utf8CP name = optionExp->GetName();
    if (m_optionsByName.find(name) != m_optionsByName.end())
        {
        issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Multiple options with same name ('%s') are not supported.", name);
        return ERROR;
        }

    const size_t optionIndex = AddChild(std::move(optionExp));
    m_optionsByName[name] = optionIndex;
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool OptionsExp::HasOption(Utf8CP optionName) const
    {
    OptionExp const* option = nullptr;
    if (!TryGetOption(option, optionName))
        return false;

    if (!option->IsNameValuePair())
        return true;

    Utf8CP val = option->GetValue();
    return BeStringUtilities::StricmpAscii(val, "true") == 0 || BeStringUtilities::StricmpAscii(val, "1") == 0;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void OptionsExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    BeAssert(GetChildrenCount() != 0);

    ctx.AppendToECSql("ECSQLOPTIONS");
    for (Exp const* child : GetChildren())
        {
        ctx.AppendToECSql(" ").AppendToECSql(*child);
        }
    }


//****************************** OptionExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void OptionExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql(m_name);

    if (!IsNameValuePair())
        return;

    ctx.AppendToECSql("=").AppendToECSql(m_val);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


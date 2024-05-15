/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************************** OptionsExp *****************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus OptionsExp::AddOptionExp(std::unique_ptr<OptionExp> optionExp, IssueDataSource const& issues)
    {
    Utf8CP name = optionExp->GetName();
    if (m_optionsByName.find(name) != m_optionsByName.end())
        {
        issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0548, "Multiple options with same name ('%s') are not supported.", name);
        return ERROR;
        }

    const size_t optionIndex = AddChild(std::move(optionExp));
    m_optionsByName[name] = optionIndex;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool OptionsExp::HasOption(Utf8CP optionName) const
    {
    OptionExp const* option = nullptr;
    if (!TryGetOption(option, optionName))
        return false;

    if (!option->IsNameValuePair())
        return true;

    return option->asBool();
    }

//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void OptionsExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: OptionsExp
    val.SetEmptyObject();
    val["id"] = "OptionsExp";
    auto options = val["options"];
    options.toArray();
    for (Exp const* child : GetChildren())
        child->ToJson(options.appendArray(), fmt);
}

//---------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void OptionExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: OptionExp
    val.SetEmptyObject();
    val["name"] = m_name;
    if (IsNameValuePair())
        val["value"] = m_val;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool OptionExp::asBool() const {
    if (GetValType().IsBoolean()) {
        return m_val.EqualsIAscii("TRUE");
    }
    if (GetValType().IsExactNumeric()) {
        return std::atoi(m_val.c_str()) != 0;
    }
    if (GetValType().IsApproximateNumeric()) {
        return std::atof(m_val.c_str()) != 0.0f;
    }
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void OptionExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql(m_name);

    if (!IsNameValuePair())
        return;

    ctx.AppendToECSql("=").AppendToECSql(m_val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
OptionExp const* OptionsExp::FindLocalOrInheritedOption(Utf8CP optionName, ExpCR exp) {
    OptionExp const* opt;
    auto cur = exp.FindParent(Exp::Type::SingleSelect);
    while(cur != nullptr) {
        auto options = cur->GetAsCP<SingleSelectStatementExp>()->GetOptions();
        if (options) {
            if (options->TryGetOption(opt, optionName)) {
                return opt;
            }
        }
        cur = cur->FindParent(Exp::Type::SingleSelect);
    }
    return nullptr;
}
END_BENTLEY_SQLITE_EC_NAMESPACE


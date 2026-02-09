/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "UpdateStatementExp.h"

#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************** UpdateStatementExp ******************************************

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
UpdateStatementExp::UpdateStatementExp(std::unique_ptr<ClassNameExp> classNameExp, std::unique_ptr<AssignmentListExp> assignmentListExp, std::unique_ptr<WhereExp> whereClauseExp, std::unique_ptr<OptionsExp> optionsExp)
    : Exp(Type::Update), m_whereClauseIndex(UNSET_CHILDINDEX), m_optionsClauseIndex(UNSET_CHILDINDEX) {
    m_classNameExpIndex = AddChild(std::move(classNameExp));
    m_assignmentListExpIndex = AddChild(std::move(assignmentListExp));

    if (whereClauseExp != nullptr)
        m_whereClauseIndex = (int)AddChild(std::move(whereClauseExp));

    if (optionsExp != nullptr)
        m_optionsClauseIndex = (int)AddChild(std::move(optionsExp));
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus UpdateStatementExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode) {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren) {
        ClassNameExp const* classNameExp = GetClassNameExp();
        if (classNameExp == nullptr) {
            BeAssert(false && "ClassNameExp expected to be not null for UpdateStatementExp");
            return FinalizeParseStatus::Error;
        }

        if (classNameExp->GetMemberFunctionCallExp() != nullptr) {
            ctx.Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECSQL,
                ECDbIssueId::ECDb_0577,
                "May not call function on class in a UPDATE statement: %s",
                ToECSql().c_str());
            return FinalizeParseStatus::Error;
        }

        std::vector<RangeClassInfo> classList;
        classList.push_back(RangeClassInfo(*classNameExp, RangeClassInfo::Scope::Local));
        m_rangeClassRefExpCache = classList;
        ctx.PushArg(std::make_unique<ECSqlParseContext::RangeClassArg>(m_rangeClassRefExpCache));
        return FinalizeParseStatus::NotCompleted;
    } else {
        ctx.PopArg();
        m_rangeClassRefExpCache.clear();

        return FinalizeParseStatus::Completed;
    }
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
WhereExp const* UpdateStatementExp::GetWhereClauseExp() const {
    if (m_whereClauseIndex < 0)
        return nullptr;

    return GetChild<WhereExp>((size_t)m_whereClauseIndex);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
OptionsExp const* UpdateStatementExp::GetOptionsClauseExp() const {
    if (m_optionsClauseIndex < 0)
        return nullptr;

    return GetChild<OptionsExp>((size_t)m_optionsClauseIndex);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void UpdateStatementExp::_ToJson(BeJsValue val, JsonFormat const& fmt) const {
    //! ITWINJS_PARSE_TREE: UpdateStatementExp
    val.SetEmptyObject();
    val["id"] = "UpdateStatementExp";
    GetClassNameExp()->ToJson(val["className"], fmt);
    GetAssignmentListExp()->ToJson(val["assignment"], fmt);
    if (GetWhereClauseExp()) {
        GetWhereClauseExp()->ToJson(val["where"], fmt);
    }
    if (GetOptionsClauseExp() != nullptr)
        GetOptionsClauseExp()->ToJson(val["options"], fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void UpdateStatementExp::_ToECSql(ECSqlRenderContext& ctx) const {
    ctx.AppendToECSql("UPDATE ").AppendToECSql(*GetClassNameExp()).AppendToECSql("SET ").AppendToECSql(*GetAssignmentListExp());

    Exp const* exp = GetWhereClauseExp();
    if (exp != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*exp);

    exp = GetOptionsClauseExp();
    if (exp != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*exp);
}

//*************************** AssignmentExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
AssignmentExp::AssignmentExp(std::unique_ptr<PropertyNameExp> propNameExp, std::unique_ptr<ValueExp> valueExp)
    : Exp(Type::Assignment) {
    m_propNameExpIndex = AddChild(std::move(propNameExp));
    m_valueExpIndex = AddChild(std::move(valueExp));
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void AssignmentExp::_ToECSql(ECSqlRenderContext& ctx) const {
    ctx.AppendToECSql(*GetPropertyNameExp()).AppendToECSql(" = ").AppendToECSql(*GetValueExp());
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void AssignmentExp::_ToJson(BeJsValue val, JsonFormat const& fmt) const {
    //! ITWINJS_PARSE_TREE: AssignmentExp
    val["id"] = "AssignmentExp";
    GetPropertyNameExp()->ToJson(val["propertyName"], fmt);
    GetValueExp()->ToJson(val["value"], fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus AssignmentExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode) {
    switch (mode) {
        case FinalizeParseMode::BeforeFinalizingChildren:
            return FinalizeParseStatus::NotCompleted;

        case FinalizeParseMode::AfterFinalizingChildren: {
            Utf8String errorMessage;
            ValueExp const* valueExp = GetValueExp();
            if (!valueExp->IsParameterExp() && !GetPropertyNameExp()->GetTypeInfo().CanCompare(valueExp->GetTypeInfo(), &errorMessage)) {
                ctx.Issues().ReportV(
                    IssueSeverity::Error,
                    IssueCategory::BusinessProperties,
                    IssueType::ECSQL,
                    ECDbIssueId::ECDb_0578,
                    "Type mismatch in SET clause of UPDATE statement: %s",
                    errorMessage.c_str());
                return FinalizeParseStatus::Error;
            }

            return FinalizeParseStatus::Completed;
        }

        default:
            BeAssert(false);
            return FinalizeParseStatus::Error;
    }
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool AssignmentExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const {
    // Assign the prop name exp to the parameter exp
    parameterExp.SetTargetExpInfo(*GetPropertyNameExp());
    return true;
}

END_BENTLEY_SQLITE_EC_NAMESPACE

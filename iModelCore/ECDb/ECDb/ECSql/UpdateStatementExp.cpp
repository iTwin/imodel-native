/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/UpdateStatementExp.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "UpdateStatementExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************** UpdateStatementExp ******************************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
UpdateStatementExp::UpdateStatementExp(std::unique_ptr<ClassRefExp> classNameExp, std::unique_ptr<AssignmentListExp> assignmentListExp, std::unique_ptr<WhereExp> whereClauseExp, std::unique_ptr<OptionsExp> optionsExp)
    : Exp(Type::Update), m_whereClauseIndex(UNSET_CHILDINDEX), m_optionsClauseIndex(UNSET_CHILDINDEX)
    {
    BeAssert(classNameExp->GetType() == Exp::Type::ClassName);
    m_classNameExpIndex = AddChild(std::move(classNameExp));
    m_assignmentListExpIndex = AddChild(std::move(assignmentListExp));

    if (whereClauseExp != nullptr)
        m_whereClauseIndex = (int) AddChild(std::move(whereClauseExp));

    if (optionsExp != nullptr)
        m_optionsClauseIndex = (int) AddChild(std::move(optionsExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus UpdateStatementExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        auto classNameExp = GetClassNameExp();
        RangeClassInfo::List classList;
        classList.push_back(RangeClassInfo(*classNameExp, RangeClassInfo::Scope::Local));
        m_finalizeParsingArgCache = classList;
        ctx.PushArg(std::unique_ptr<ECSqlParseContext::RangeClassArg>(new ECSqlParseContext::RangeClassArg(m_finalizeParsingArgCache)));
        return FinalizeParseStatus::NotCompleted;
        }
    else
        {
        ctx.PopArg();
        m_finalizeParsingArgCache.clear();

        return FinalizeParseStatus::Completed;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
WhereExp const* UpdateStatementExp::GetWhereClauseExp() const
    {
    if (m_whereClauseIndex < 0)
        return nullptr;

    return GetChild<WhereExp>((size_t) m_whereClauseIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
OptionsExp const* UpdateStatementExp::GetOptionsClauseExp() const
    {
    if (m_optionsClauseIndex < 0)
        return nullptr;

    return GetChild<OptionsExp>((size_t) m_optionsClauseIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
void UpdateStatementExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
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
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
AssignmentExp::AssignmentExp(std::unique_ptr<PropertyNameExp> propNameExp, std::unique_ptr<ValueExp> valueExp)
    : Exp(Type::Assignment)
    {
    m_propNameExpIndex = AddChild(move(propNameExp));
    m_valueExpIndex = AddChild(move(valueExp));
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus AssignmentExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    switch (mode)
        {
            case FinalizeParseMode::BeforeFinalizingChildren:
                return FinalizeParseStatus::NotCompleted;

            case FinalizeParseMode::AfterFinalizingChildren:
            {
            Utf8String errorMessage;
            ValueExp const* valueExp = GetValueExp();
            if (!valueExp->IsParameterExp() && !GetPropertyNameExp()->GetTypeInfo().CanCompare(valueExp->GetTypeInfo(), &errorMessage))
                {
                ctx.Issues().Report("Type mismatch in SET clause of UPDATE statement: %s", errorMessage.c_str());
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
// @bsimethod                                    Krischan.Eberle       06/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool AssignmentExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    //Assign the prop name exp to the parameter exp
    parameterExp.SetTargetExpInfo(*GetPropertyNameExp());
    return true;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


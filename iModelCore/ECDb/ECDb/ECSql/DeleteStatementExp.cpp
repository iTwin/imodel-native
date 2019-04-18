/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
DeleteStatementExp::DeleteStatementExp(std::unique_ptr<ClassRefExp> classNameExp, std::unique_ptr<WhereExp> whereClauseExp, std::unique_ptr<OptionsExp> optionsClauseExp)
    : Exp(Type::Delete), m_whereClauseIndex(UNSET_CHILDINDEX), m_optionsClauseIndex(UNSET_CHILDINDEX)
    {
    BeAssert(classNameExp->GetType() == Exp::Type::ClassName);
    m_classNameExpIndex = AddChild(std::move(classNameExp));

    if (whereClauseExp != nullptr)
        m_whereClauseIndex = (int) AddChild(std::move(whereClauseExp));

    if (optionsClauseExp != nullptr)
        m_optionsClauseIndex = (int) AddChild(std::move(optionsClauseExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus DeleteStatementExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {

    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        ClassNameExp const* classNameExp = GetClassNameExp();
        if (classNameExp == nullptr)
            {
            BeAssert(false && "ClassNameExp expected to be not null for DeleteStatementExp");
            return FinalizeParseStatus::Error;
            }

        if (classNameExp->GetMemberFunctionCallExp() != nullptr)
            {
            ctx.Issues().ReportV("May not call function on class in FROM clause in a DELETE statement: %s", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }

        std::vector<RangeClassInfo> classList;
        classList.push_back(RangeClassInfo(*classNameExp, RangeClassInfo::Scope::Local));
        m_rangeClassRefExpCache = std::move(classList);
        ctx.PushArg(std::make_unique<ECSqlParseContext::RangeClassArg>(m_rangeClassRefExpCache));
        return FinalizeParseStatus::NotCompleted;
        }

    ctx.PopArg();
    m_rangeClassRefExpCache.clear();
    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
ClassNameExp const* DeleteStatementExp::GetClassNameExp() const { return GetChild<ClassNameExp>(m_classNameExpIndex); }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
WhereExp const* DeleteStatementExp::GetWhereClauseExp() const
    {
    if (m_whereClauseIndex < 0)
        return nullptr;

    return GetChild<WhereExp>((size_t) m_whereClauseIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   10/2015
//+---------------+---------------+---------------+---------------+---------------+--------
OptionsExp const* DeleteStatementExp::GetOptionsClauseExp() const
    {
    if (m_optionsClauseIndex < 0)
        return nullptr;

    return GetChild<OptionsExp>((size_t) m_optionsClauseIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
void DeleteStatementExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql("DELETE FROM ").AppendToECSql(*GetClassNameExp());

    Exp const* exp = GetWhereClauseExp();
    if (exp != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*exp);

    exp = GetOptionsClauseExp();
    if (exp != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*exp);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


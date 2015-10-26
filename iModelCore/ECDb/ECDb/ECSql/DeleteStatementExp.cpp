/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/DeleteStatementExp.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "DeleteStatementExp.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
DeleteStatementExp::DeleteStatementExp(unique_ptr<ClassRefExp> classNameExp, unique_ptr<WhereExp> whereClauseExp, unique_ptr<OptionsExp> optionsClauseExp)
    : Exp(), m_whereClauseIndex(UNSET_CHILDINDEX), m_optionsClauseIndex(UNSET_CHILDINDEX)
    {
    BeAssert(classNameExp->GetType() == Exp::Type::ClassName);
    m_classNameExpIndex = AddChild(move(classNameExp));

    if (whereClauseExp != nullptr)
        m_whereClauseIndex = (int) AddChild(move(whereClauseExp));

    if (optionsClauseExp != nullptr)
        m_optionsClauseIndex = (int) AddChild(move(optionsClauseExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus DeleteStatementExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        auto classNameExp = GetClassNameExp ();
        auto classList = unique_ptr<RangeClassRefList> (new RangeClassRefList ());
        classList->push_back (classNameExp);
        m_finalizeParsingArgCache = move (classList);
        ctx.PushFinalizeParseArg (m_finalizeParsingArgCache.get ());

        return FinalizeParseStatus::NotCompleted;
        }
    else
        {
        ctx.PopFinalizeParseArg ();
        m_finalizeParsingArgCache = nullptr;

        return FinalizeParseStatus::Completed;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
ClassNameExp const* DeleteStatementExp::GetClassNameExp () const
    {
    return GetChild<ClassNameExp> (m_classNameExpIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
WhereExp const* DeleteStatementExp::GetWhereClauseExp () const
    {
    if (m_whereClauseIndex < 0)
        return nullptr;

    return GetChild<WhereExp> (static_cast<size_t> (m_whereClauseIndex));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   10/2015
//+---------------+---------------+---------------+---------------+---------------+--------
OptionsExp const* DeleteStatementExp::GetOptionsClauseExp() const
    {
    if (m_optionsClauseIndex < 0)
        return nullptr;

    return GetChild<OptionsExp>(static_cast<size_t> (m_optionsClauseIndex));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String DeleteStatementExp::_ToECSql () const
    {
    Utf8String ecsql ("DELETE FROM ");

    ecsql.append (GetClassNameExp ()->ToECSql ());

    Exp const* exp = GetWhereClauseExp ();
    if (exp != nullptr)
        ecsql.append (" ").append (exp->ToECSql ());

    exp = GetOptionsClauseExp();
    if (exp != nullptr)
        ecsql.append(" ").append(exp->ToECSql());

    return ecsql;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE


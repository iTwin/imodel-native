/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/UpdateStatementExp.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "UpdateStatementExp.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************** UpdateStatementExp ******************************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
UpdateStatementExp::UpdateStatementExp (unique_ptr<ClassRefExp> classNameExp, unique_ptr<AssignmentListExp> assignmentListExp, unique_ptr<WhereExp> whereClauseExp, unique_ptr<OptionsExp> optionsExp)
    : Exp (), m_whereClauseIndex (UNSET_CHILDINDEX), m_optionsClauseIndex(UNSET_CHILDINDEX)
    {
    BeAssert (classNameExp->GetType () == Exp::Type::ClassName);
    m_classNameExpIndex = AddChild (move (classNameExp));
    m_assignmentListExpIndex = AddChild (move (assignmentListExp));

    if (whereClauseExp != nullptr)
        m_whereClauseIndex = (int) AddChild (move (whereClauseExp));

    if (optionsExp != nullptr)
        m_optionsClauseIndex = (int) AddChild(move(optionsExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus UpdateStatementExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
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
ClassNameExp const* UpdateStatementExp::GetClassNameExp () const
    {
    return GetChild<ClassNameExp> (m_classNameExpIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
AssignmentListExp const* UpdateStatementExp::GetAssignmentListExp () const
    {
    return GetChild<AssignmentListExp> (m_assignmentListExpIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
WhereExp const* UpdateStatementExp::GetWhereClauseExp () const
    {
    if (m_whereClauseIndex < 0)
        return nullptr;

    return GetChild<WhereExp> ((size_t) m_whereClauseIndex);
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
Utf8String UpdateStatementExp::_ToECSql () const
    {
    Utf8String ecsql ("UPDATE ");

    ecsql.append (GetClassNameExp ()->ToECSql ());

    ecsql.append ("SET ").append (GetAssignmentListExp ()->ToECSql ());

    Exp const* exp = GetWhereClauseExp ();
    if (exp != nullptr)
        ecsql.append (" ").append (exp->ToECSql ());

    exp = GetOptionsClauseExp();
    if (exp != nullptr)
        ecsql.append(" ").append(exp->ToECSql());

    return ecsql;
    }

//*************************** AssignmentExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
AssignmentExp::AssignmentExp (std::unique_ptr<PropertyNameExp> propNameExp, std::unique_ptr<ValueExp> valueExp)
    : Exp ()
    {
    m_propNameExpIndex = AddChild (move (propNameExp));
    m_valueExpIndex = AddChild (move (valueExp));
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
                if (!valueExp->IsParameterExp() && !GetPropertyNameExp()->GetTypeInfo().Matches(valueExp->GetTypeInfo(), &errorMessage))
                    {
                    ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch in SET clause of UPDATE statement: %s", errorMessage.c_str());
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

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameExp const* AssignmentExp::GetPropertyNameExp () const
    {
    return GetChild<PropertyNameExp> (m_propNameExpIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
ValueExp const* AssignmentExp::GetValueExp () const
    {
    return GetChild<ValueExp> (m_valueExpIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//virtual
Utf8String AssignmentExp::_ToECSql () const
    {
    Utf8String ecsql = GetPropertyNameExp ()->ToECSql ();
    ecsql.append (" = ").append (GetValueExp ()->ToECSql ());

    return ecsql;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


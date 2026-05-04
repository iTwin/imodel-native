/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************** OnConflictExp *************************

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
OnConflictExp::OnConflictExp(std::unique_ptr<PropertyNameListExp> conflictTarget)
    : Exp(Type::OnConflict), m_action(Action::DoNothing), m_conflictTargetIndex(-1),
      m_assignmentListIndex(-1), m_whereClauseIndex(-1)
    {
    if (conflictTarget != nullptr)
        m_conflictTargetIndex = (int) AddChild(std::move(conflictTarget));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
OnConflictExp::OnConflictExp(std::unique_ptr<PropertyNameListExp> conflictTarget,
                             std::unique_ptr<AssignmentListExp> assignmentList,
                             std::unique_ptr<WhereExp> whereExp)
    : Exp(Type::OnConflict), m_action(Action::DoUpdate), m_conflictTargetIndex(-1),
      m_assignmentListIndex(-1), m_whereClauseIndex(-1)
    {
    if (conflictTarget != nullptr)
        m_conflictTargetIndex = (int) AddChild(std::move(conflictTarget));

    BeAssert(assignmentList != nullptr);
    m_assignmentListIndex = (int) AddChild(std::move(assignmentList));

    if (whereExp != nullptr)
        m_whereClauseIndex = (int) AddChild(std::move(whereExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus OnConflictExp::_FinalizeParsing(ECSqlParseContext&, FinalizeParseMode)
    {
    return FinalizeParseStatus::NotCompleted;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameListExp const* OnConflictExp::GetConflictTargetExp() const
    {
    if (m_conflictTargetIndex < 0)
        return nullptr;
    return GetChild<PropertyNameListExp>((size_t) m_conflictTargetIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
AssignmentListExp const* OnConflictExp::GetAssignmentListExp() const
    {
    if (m_assignmentListIndex < 0)
        return nullptr;
    return GetChild<AssignmentListExp>((size_t) m_assignmentListIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
WhereExp const* OnConflictExp::GetWhereClauseExp() const
    {
    if (m_whereClauseIndex < 0)
        return nullptr;
    return GetChild<WhereExp>((size_t) m_whereClauseIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void OnConflictExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql(" ON CONFLICT");

    if (HasConflictTarget())
        ctx.AppendToECSql(" (").AppendToECSql(*GetConflictTargetExp()).AppendToECSql(")");

    if (m_action == Action::DoNothing)
        {
        ctx.AppendToECSql(" DO NOTHING");
        }
    else
        {
        ctx.AppendToECSql(" DO UPDATE SET ").AppendToECSql(*GetAssignmentListExp());
        if (GetWhereClauseExp() != nullptr)
            ctx.AppendToECSql(" ").AppendToECSql(*GetWhereClauseExp());
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void OnConflictExp::_ToJson(BeJsValue val, JsonFormat const& fmt) const
    {
    val.SetEmptyObject();
    val["id"] = "OnConflictExp";
    val["action"] = (m_action == Action::DoNothing) ? "DoNothing" : "DoUpdate";
    if (HasConflictTarget())
        GetConflictTargetExp()->ToJson(val["conflictTarget"], fmt);
    if (m_action == Action::DoUpdate && GetAssignmentListExp() != nullptr)
        GetAssignmentListExp()->ToJson(val["assignments"], fmt);
    if (GetWhereClauseExp() != nullptr)
        GetWhereClauseExp()->ToJson(val["where"], fmt);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

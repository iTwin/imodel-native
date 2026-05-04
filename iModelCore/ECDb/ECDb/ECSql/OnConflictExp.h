/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ListExp.h"
#include "WhereExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Represents an ECSQL ON CONFLICT clause.
//! Syntax: ON CONFLICT [( conflictTarget, ... )] DO NOTHING
//!       | ON CONFLICT [( conflictTarget, ... )] DO UPDATE SET assignment [, ...] [WHERE expr]
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct OnConflictExp final : Exp
    {
    public:
        enum class Action { DoNothing, DoUpdate };

    private:
        Action m_action;
        int m_conflictTargetIndex; // index of PropertyNameListExp child, or -1
        int m_assignmentListIndex; // index of AssignmentListExp child (DoUpdate only), or -1
        int m_whereClauseIndex;    // index of WhereExp child (DoUpdate only), or -1

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        void _ToECSql(ECSqlRenderContext&) const override;
        void _ToJson(BeJsValue, JsonFormat const&) const override;
        Utf8String _ToString() const override { return "OnConflict"; }

    public:
        //! Construct a DO NOTHING ON CONFLICT clause (optional conflict target list).
        explicit OnConflictExp(std::unique_ptr<PropertyNameListExp> conflictTarget);

        //! Construct a DO UPDATE ON CONFLICT clause (optional conflict target, required assignments, optional WHERE).
        OnConflictExp(std::unique_ptr<PropertyNameListExp> conflictTarget,
                      std::unique_ptr<AssignmentListExp> assignmentList,
                      std::unique_ptr<WhereExp> whereExp);

        Action GetAction() const { return m_action; }
        bool HasConflictTarget() const { return m_conflictTargetIndex >= 0; }
        PropertyNameListExp const* GetConflictTargetExp() const;
        AssignmentListExp const* GetAssignmentListExp() const;
        WhereExp const* GetWhereClauseExp() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

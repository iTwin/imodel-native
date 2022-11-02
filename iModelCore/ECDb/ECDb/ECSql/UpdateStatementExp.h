/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ClassRefExp.h"
#include "ListExp.h"
#include "OptionsExp.h"
#include "WhereExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct UpdateStatementExp final : Exp
    {
    private:
        size_t m_classNameExpIndex;
        size_t m_assignmentListExpIndex;
        int m_whereClauseIndex;
        int m_optionsClauseIndex;

        std::vector<RangeClassInfo> m_rangeClassRefExpCache;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override { return "Update"; }

    public:
        UpdateStatementExp(std::unique_ptr<ClassRefExp>, std::unique_ptr<AssignmentListExp>, std::unique_ptr<WhereExp>, std::unique_ptr<OptionsExp>);

        ClassNameExp const* GetClassNameExp() const { return GetChild<ClassNameExp>(m_classNameExpIndex); }
        AssignmentListExp const* GetAssignmentListExp() const { return GetChild<AssignmentListExp>(m_assignmentListExpIndex); }
        WhereExp const* GetWhereClauseExp() const;
        OptionsExp const* GetOptionsClauseExp() const;
    };


//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct AssignmentExp final : Exp
    {
    private:
        size_t m_propNameExpIndex;
        size_t m_valueExpIndex;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;

        void _ToECSql(ECSqlRenderContext& ctx) const override { ctx.AppendToECSql(*GetPropertyNameExp()).AppendToECSql(" = ").AppendToECSql(*GetValueExp()); }
        Utf8String _ToString() const override { return "Assignment"; }

    public:
        AssignmentExp(std::unique_ptr<PropertyNameExp> propNameExp, std::unique_ptr<ValueExp> valueExp);

        PropertyNameExp const* GetPropertyNameExp() const { return GetChild<PropertyNameExp>(m_propNameExpIndex); }
        ValueExp const* GetValueExp() const { return GetChild<ValueExp>(m_valueExpIndex); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE


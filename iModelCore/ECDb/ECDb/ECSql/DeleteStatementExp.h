/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ClassRefExp.h"
#include "WhereExp.h"
#include "OptionsExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct DeleteStatementExp final : Exp
    {
    private:
        size_t m_classNameExpIndex;
        int m_whereClauseIndex;
        int m_optionsClauseIndex;

        std::vector<RangeClassInfo> m_rangeClassRefExpCache;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override { return "Delete"; }

    public:
        DeleteStatementExp(std::unique_ptr<ClassRefExp>, std::unique_ptr<WhereExp>, std::unique_ptr<OptionsExp>);

        ClassNameExp const* GetClassNameExp() const;
        WhereExp const* GetWhereClauseExp() const;
        OptionsExp const* GetOptionsClauseExp() const;
    };



END_BENTLEY_SQLITE_EC_NAMESPACE


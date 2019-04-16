/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ClassRefExp.h"
#include "WhereExp.h"
#include "OptionsExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      01/2014
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


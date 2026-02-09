/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECSqlPreparer.h"
#include "../PolicyManager.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlDeletePreparer final
    {
    private:
        struct NativeSqlSnippets final
            {
            NativeSqlBuilder m_classNameNativeSqlSnippet;
            NativeSqlBuilder::List m_pkColumnNamesNativeSqlSnippets;
            NativeSqlBuilder::List m_pkValuesNativeSqlSnippets;
            NativeSqlBuilder m_whereClauseNativeSqlSnippet;
            NativeSqlBuilder m_systemWhereClauseNativeSqlSnippet;
            };

        //static class
        ECSqlDeletePreparer();
        ~ECSqlDeletePreparer();

        static ECSqlStatus GenerateNativeSqlSnippets(NativeSqlSnippets& deleteNativeSqlSnippets, ECSqlPrepareContext&, DeleteStatementExp const&, ClassNameExp const&);
        static void BuildNativeSqlDeleteStatement(NativeSqlBuilder& deleteBuilder, NativeSqlSnippets const& deleteNativeSqlSnippets);

    public:
        static ECSqlStatus Prepare(ECSqlPrepareContext&, DeleteStatementExp const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
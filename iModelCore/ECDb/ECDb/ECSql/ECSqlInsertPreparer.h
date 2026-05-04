/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECSqlPreparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlInsertPreparer final
    {
    private:
        struct NativeSqlSnippets final
            {
            int m_ecinstanceIdExpIndex;
            NativeSqlBuilder m_classNameNativeSqlSnippet;
            NativeSqlBuilder::ListOfLists m_propertyNamesNativeSqlSnippets;
            NativeSqlBuilder::List m_pkColumnNamesNativeSqlSnippets;
            NativeSqlBuilder::ListOfLists m_valuesNativeSqlSnippets;
            NativeSqlBuilder::List m_pkValuesNativeSqlSnippets;
            NativeSqlBuilder m_onConflictClause; // empty if no ON CONFLICT
            };


        //static class
        ECSqlInsertPreparer();
        ~ECSqlInsertPreparer();

        static ECSqlStatus GenerateNativeSqlSnippets(NativeSqlSnippets& insertNativeSqlSnippets, ECSqlPrepareContext&, InsertStatementExp const&, ClassMap const&);
        static void PrepareClassId(ECSqlPrepareContext&, NativeSqlSnippets& nativeSqlSnippets, ClassMap const&);
        static ECSqlStatus GenerateOnConflictClause(NativeSqlBuilder& out, ECSqlPrepareContext&, OnConflictExp const&, InsertStatementExp const&, ClassMap const&);
        static void BuildNativeSqlInsertStatement(NativeSqlBuilder& insertBuilder, NativeSqlSnippets const& insertNativeSqlSnippets, InsertStatementExp const& exp);

    public:
        static ECSqlStatus Prepare(ECSqlPrepareContext&, InsertStatementExp const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
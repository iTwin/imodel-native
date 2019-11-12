/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlPreparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    11/2013
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
            };


        //static class
        ECSqlInsertPreparer();
        ~ECSqlInsertPreparer();

        static ECSqlStatus GenerateNativeSqlSnippets(NativeSqlSnippets& insertNativeSqlSnippets, ECSqlPrepareContext&, InsertStatementExp const&, ClassMap const&);
        static void PrepareClassId(ECSqlPrepareContext&, NativeSqlSnippets& nativeSqlSnippets, ClassMap const&);
        static void BuildNativeSqlInsertStatement(NativeSqlBuilder& insertBuilder, NativeSqlSnippets const& insertNativeSqlSnippets, InsertStatementExp const& exp);

    public:
        static ECSqlStatus Prepare(ECSqlPrepareContext&, InsertStatementExp const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
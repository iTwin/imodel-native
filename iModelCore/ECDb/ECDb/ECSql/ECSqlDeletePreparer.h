/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlDeletePreparer.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlPreparer.h"
#include "../ECDbPolicyManager.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    01/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlDeletePreparer
    {
private:
    struct NativeSqlSnippets
        {
        NativeSqlBuilder m_classNameNativeSqlSnippet;
        NativeSqlBuilder::List m_pkColumnNamesNativeSqlSnippets;
        NativeSqlBuilder::List m_pkValuesNativeSqlSnippets;
        NativeSqlBuilder m_whereClauseNativeSqlSnippet;
        NativeSqlBuilder m_systemWhereClauseNativeSqlSnippet;
        };


    //static class
    ECSqlDeletePreparer ();
    ~ECSqlDeletePreparer ();

    static ECSqlStatus GenerateNativeSqlSnippets (NativeSqlSnippets& deleteNativeSqlSnippets, ECSqlPrepareContext& ctx, 
                        DeleteStatementExp const& exp, ClassNameExp const& classNameExp);

    static ECSqlStatus PrepareForClass (ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets);
    static ECSqlStatus PrepareForEndTableRelationship (ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, RelationshipClassEndTableMapCR classMap);

    static void BuildNativeSqlDeleteStatement (NativeSqlBuilder& deleteBuilder, NativeSqlSnippets const& deleteNativeSqlSnippets);
    static void BuildNativeSqlUpdateStatement (NativeSqlBuilder& updateBuilder, NativeSqlSnippets const& deleteNativeSqlSnippets,
                                    NativeSqlBuilder::List const& propNamesToUnsetNativeSqlSnippets);

public:
    static ECSqlStatus Prepare (ECSqlPrepareContext& ctx, DeleteStatementExp const& exp);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
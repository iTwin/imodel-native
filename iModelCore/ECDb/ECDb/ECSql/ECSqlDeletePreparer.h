/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlDeletePreparer.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlPreparer.h"
#include "../PolicyManager.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    01/2014
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
    ECSqlDeletePreparer ();
    ~ECSqlDeletePreparer ();

    static ECSqlStatus GenerateNativeSqlSnippets (NativeSqlSnippets& deleteNativeSqlSnippets, ECSqlPrepareContext&, 
                        DeleteStatementExp const&, ClassNameExp const&);

    static ECSqlStatus PrepareForClass (ECSqlPrepareContext&, NativeSqlSnippets&);
    static ECSqlStatus PrepareForEndTableRelationship (ECSqlPrepareContext&, NativeSqlSnippets&, RelationshipClassEndTableMap const&);

    static void BuildNativeSqlDeleteStatement (NativeSqlBuilder& deleteBuilder, NativeSqlSnippets const& deleteNativeSqlSnippets);
    static void BuildNativeSqlUpdateStatement (NativeSqlBuilder& updateBuilder, NativeSqlSnippets const& deleteNativeSqlSnippets, NativeSqlBuilder::List const& propNamesToUnsetNativeSqlSnippets);

public:
    static ECSqlStatus Prepare (ECSqlPrepareContext&, DeleteStatementExp const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
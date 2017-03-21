/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlSelectPreparer.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlPreparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    01/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlSelectPreparer final
    {
private:
    //static class
    ECSqlSelectPreparer();
    ~ECSqlSelectPreparer();

    static ECSqlStatus Prepare(ECSqlPrepareContext&, SelectStatementExp const&, std::vector<size_t> const* referenceSelectClauseSqlSnippetCounts);
    static ECSqlStatus Prepare (ECSqlPrepareContext&, NativeSqlBuilder::ListOfLists& selectClauseSqlSnippetList, SingleSelectStatementExp const&, std::vector<size_t> const* referenceSelectClauseSqlSnippetCounts);

    static ECSqlStatus PrepareSelectClauseExp(NativeSqlBuilder::ListOfLists&, ECSqlPrepareContext&, SelectClauseExp const&, std::vector<size_t> const* referenceSelectClauseSqlSnippetCounts);
    static ECSqlStatus PrepareDerivedPropertyExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, DerivedPropertyExp const&, size_t referenceSqliteSnippetCount);
    static BentleyStatus ValidateSelectClauseItems(ECSqlPrepareContext&, SelectClauseExp const& lhs, SelectClauseExp const& rhs);
    static void ExtractPropertyRefs(ECSqlPrepareContext&, Exp const*);
public:
    static ECSqlStatus Prepare(ECSqlPrepareContext&, SelectStatementExp const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
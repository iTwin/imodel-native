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
    static ECSqlStatus Prepare(ECSqlPrepareContext&, CommonTableExp const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
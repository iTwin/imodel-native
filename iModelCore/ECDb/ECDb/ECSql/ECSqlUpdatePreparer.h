/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlUpdatePreparer.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+------------------------------------------------ --------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlPreparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    01/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlUpdatePreparer
    {
private:
    struct NativeSqlSnippets
        {
        std::map<size_t, std::set<size_t>> m_overflowPropertyIndexes;
        NativeSqlBuilder::ListOfLists m_propertyNamesNativeSqlSnippets;
        NativeSqlBuilder::ListOfLists m_valuesNativeSqlSnippets;
        };

    //static class
    ECSqlUpdatePreparer ();
    ~ECSqlUpdatePreparer ();
    static ECSqlStatus CheckForReadonlyProperties(ECSqlPrepareContext& ctx, UpdateStatementExp const& exp);
    static ECSqlStatus PrepareAssignmentListExp (NativeSqlSnippets& snippets, ECSqlPrepareContext& ctx, AssignmentListExp const* assignmentListExp);
  
public:
    static ECSqlStatus Prepare (ECSqlPrepareContext& ctx, UpdateStatementExp const& exp);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
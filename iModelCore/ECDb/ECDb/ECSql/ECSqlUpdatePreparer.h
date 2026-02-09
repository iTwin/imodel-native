/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
+------------------------------------------------ --------------------------------------*/
#pragma once

#include "ECSqlPreparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlUpdatePreparer final
    {
    private:
        struct NativeSqlSnippets final
            {
            NativeSqlBuilder::ListOfLists m_propertyNamesNativeSqlSnippets;
            NativeSqlBuilder::ListOfLists m_valuesNativeSqlSnippets;
            };

        //static class
        ECSqlUpdatePreparer();
        ~ECSqlUpdatePreparer();

        static ECSqlStatus PrepareAssignmentListExp(NativeSqlSnippets& snippets, ECSqlPrepareContext& ctx, AssignmentListExp const* assignmentListExp);

    public:
        static ECSqlStatus Prepare(ECSqlPrepareContext& ctx, UpdateStatementExp const& exp);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
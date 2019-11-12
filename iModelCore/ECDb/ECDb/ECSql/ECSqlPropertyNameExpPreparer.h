/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlPrepareContext.h"
#include "PropertyNameExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    01/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlPropertyNameExpPreparer final
    {
    private:
        //static class
        ECSqlPropertyNameExpPreparer();
        ~ECSqlPropertyNameExpPreparer();

        //! Checks whether the property name exp needs to be translated to native SQL or not.
        //! E.g. if the property name exp refers to a property which is mapped to a virtual column
        //! the exp can be ignored under certain circumstances.
        static bool NeedsPreparation(ECSqlPrepareContext&, ECSqlPrepareContext::ExpScope const&, PropertyMap const&);

        static void PrepareDefault(NativeSqlBuilder::List&, ECSqlPrepareContext&, ECSqlType, PropertyNameExp const&, PropertyMap const&, Utf8StringCR classIdentifier);

        static ECSqlStatus PrepareRelConstraintClassIdPropMap(NativeSqlBuilder::List&, ECSqlType, PropertyNameExp const&, ConstraintECClassIdPropertyMap const&, Utf8StringCR classIdentifier);
        static ECSqlStatus PrepareInSubqueryRef(NativeSqlBuilder::List&, ECSqlPrepareContext&, PropertyNameExp const&);


    public:
        //! Prepares the given property name expression.
        //! @param [out] nativeSqlSnippets resulting list of native SQL snippets. Can be empty even in case of success. This
        //! indicates that property name expression is not needed in the native SQL counterpart. Callers therefore need to 
        //! check for emptiness of the resulting list
        //! @return ECSqlStatus::Success in case of success. Error code otherwise
        static ECSqlStatus Prepare(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext&, PropertyNameExp const&);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
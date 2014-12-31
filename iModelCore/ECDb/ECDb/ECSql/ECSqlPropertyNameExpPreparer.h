/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPropertyNameExpPreparer.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlPrepareContext.h"
#include "PropertyNameExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    01/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlPropertyNameExpPreparer
    {
private:
    //static class
    ECSqlPropertyNameExpPreparer ();
    ~ECSqlPropertyNameExpPreparer ();

    //! Checks whether the property name exp needs to be translated to native SQL or not.
    //! E.g. if the property name exp refers to a property which is mapped to a virtual column
    //! the exp can be ignored under certain cirumstances.
    static bool NeedsPreparation (ECSqlPrepareContext::ExpScope const& currentScope, PropertyMapCR propertyMap);

    static ECSqlStatus PrepareInSubqueryRef (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, PropertyNameExp const& exp);

public:
    //! Prepares the given property name expression.
    //! @param [out] nativeSqlSnippets resulting list of native SQL snippets. Can be empty even in case of success. This
    //! indicates that property name expression is not needed in the native SQL counterpart. Callers therefore need to 
    //! check for emptiness of the resulting list
    //! @param[in, out] ctx Prepare context
    //! @param[in] exp Property name expression to convert
    //! @return ECSqlStatus::Success in case of success. Error code otherwise
    static ECSqlStatus Prepare (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, PropertyNameExp const* exp);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
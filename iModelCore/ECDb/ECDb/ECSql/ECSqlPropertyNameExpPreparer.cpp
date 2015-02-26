/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPropertyNameExpPreparer.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlPropertyNameExpPreparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlPropertyNameExpPreparer::Prepare (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, PropertyNameExp const* exp)
    {
    BeAssert (exp != nullptr);
    auto expInSubqueryRef = exp->GetDerivedPropertyExpInSubqueryRefExp ();
    if (expInSubqueryRef != nullptr)
        return PrepareInSubqueryRef (nativeSqlSnippets, ctx, *exp);

    auto const& propMap = exp->GetPropertyMap ();
    auto const& currentScope = ctx.GetCurrentScope ();

    if (!NeedsPreparation (currentScope, propMap))
            return ECSqlStatus::Success;

    const auto currentScopeECSqlType = currentScope.GetECSqlType ();

    //in SQLite table aliases are only allowed for SELECT statements
    auto classIdentifier = currentScopeECSqlType == ECSqlType::Select ? exp->GetClassRefExp ()->GetId ().c_str () : nullptr;
    //auto classNameExpr =  dynamic_cast<ClassNameExp const*>(exp->GetClassRefExp ());

    //if (classNameExpr == nullptr)
    //    {
    //    BeAssert (false && "Case is only handled for ClassRefExpr of type ClassNameExpr ");
    //    return ECSqlStatus::NotYetSupported;
    //    }

    auto propNameNativeSqlSnippets = exp->GetPropertyMap ().ToNativeSql (classIdentifier, currentScopeECSqlType);

    nativeSqlSnippets.insert (nativeSqlSnippets.end (), propNameNativeSqlSnippets.begin (), propNameNativeSqlSnippets.end ());

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
bool ECSqlPropertyNameExpPreparer::NeedsPreparation (ECSqlPrepareContext::ExpScope const& currentScope, PropertyMapCR propertyMap)
    {
    const auto currentScopeECSqlType = currentScope.GetECSqlType ();

    //Property maps to virtual column which can mean that the exp doesn't need to be translated.
    if (propertyMap.IsVirtual () || !propertyMap.IsMappedToPrimaryTable())
        {
        //In INSERT statements, virtual columns are always ignored
        if (currentScopeECSqlType == ECSqlType::Insert)
            return false;

        const auto expType = currentScope.GetExp ().GetType ();
        switch (expType)
            {
                case Exp::Type::AssignmentList: //UPDATE SET clause
                case Exp::Type::OrderBy:
                    return false;
            }
        }

    return true;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                         08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlPropertyNameExpPreparer::PrepareInSubqueryRef (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, PropertyNameExp const& exp)
    {
    //auto classRef = static_cast<SubQueryRefExp*>(propertyName->GetResolvedClassRef());
    // only when 
    //auto newProperty = ctx.CreateProperty(*propertyName, derivedExp->GetName());

    return ctx.SetError (ECSqlStatus::InvalidECSql, "Subqueries not yet supported.");
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

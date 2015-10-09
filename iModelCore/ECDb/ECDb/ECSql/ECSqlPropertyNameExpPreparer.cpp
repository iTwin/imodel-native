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
void RenderPropertyMap (NativeSqlBuilder::List& snippets, PropertyMapCR propertyMap)
    {
    BeAssert (propertyMap.GetAsPropertyMapToTable () == nullptr);
    auto& children = propertyMap.GetChildren ();
    if (children.Size() == 0)
        {
        Utf8String accessString = Utf8String (propertyMap.GetPropertyAccessString ());
        if (auto mp = dynamic_cast<PropertyMapPoint const*>(&propertyMap))
            {
            snippets.push_back (NativeSqlBuilder{ ("[" + accessString + ".X]").c_str ()});
            snippets.push_back (NativeSqlBuilder{ ("[" + accessString + ".Y]").c_str () });
            if (mp->Is3d())
                snippets.push_back (NativeSqlBuilder{ ("[" + accessString + ".Z]").c_str () });
            }
        else
            {
            snippets.push_back (NativeSqlBuilder{ ("[" + accessString + "]").c_str ()});
            }
        }
    else
        {
        for (auto child : children)
            {
            RenderPropertyMap (snippets, *child);
            }
        }
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlPropertyNameExpPreparer::Prepare (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, PropertyNameExp const* exp)
    {
    BeAssert (exp != nullptr);
    if (exp->IsPropertyRef ())
        {
        return PrepareInSubqueryRef (nativeSqlSnippets, ctx, *exp);
        }

    auto const& propMap = exp->GetPropertyMap ();
    auto const& currentScope = ctx.GetCurrentScope ();

    if (!NeedsPreparation (currentScope, propMap))
            return ECSqlStatus::Success;

    const auto currentScopeECSqlType = currentScope.GetECSqlType ();
    //in SQLite table aliases are only allowed for SELECT statements
    Utf8CP classIdentifier = nullptr; 
    if (currentScopeECSqlType == ECSqlType::Select)
        classIdentifier = exp->GetClassRefExp()->GetId().c_str();
    else if (currentScopeECSqlType == ECSqlType::Delete)
        {
        classIdentifier = exp->GetPropertyMap().GetFirstColumn()->GetTable().GetName().c_str();
        }

    auto propNameNativeSqlSnippets = exp->GetPropertyMap ().ToNativeSql (classIdentifier, currentScopeECSqlType, exp->HasParentheses ());
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
    auto propertyRef = const_cast<PropertyNameExp&>(exp).GetPropertyRefP ();
    auto& derviedPropertyExp = propertyRef->LinkedTo();
    if (derviedPropertyExp.GetName ().empty ())
        {
        BeAssert ("Nested expression must have a name/alias" && false);
        return ECSqlStatus::Error;
        }

    auto valueExp = derviedPropertyExp.GetExpression ();
    //1. Exp-> PropertyName    useSameColumnNames
    //2. Exp-> ValueExpr       useAlias
    //3. Exp-> ScalarQuery     useAlias
    switch (valueExp->GetType ())
        {
        case Exp::Type::PropertyName:
            {
            auto propertyName = static_cast <PropertyNameExp const*>(valueExp);
            if (!propertyName->IsPropertyRef ())
                {
                if (!propertyRef->IsPrepared ())
                    {
                    auto snippets = propertyName->GetPropertyMap ().ToNativeSql (nullptr, ECSqlType::Select, false);
                    auto r = propertyRef->Prepare (snippets);
                    if (!r)
                        return ECSqlStatus::Error;
                    }
                }
            else
                {
                NativeSqlBuilder::List snippets;
                ctx.PushScope (ctx.GetCurrentScope ().GetExp ());
                auto stat = ECSqlPropertyNameExpPreparer::PrepareInSubqueryRef (snippets, ctx, *propertyName);
                if (!stat.IsSuccess())
                    return stat;

                ctx.PopScope ();

                bool r = propertyRef->Prepare (snippets);
                if (!r)
                    return ECSqlStatus::Error;
                }

            nativeSqlSnippets = propertyRef->GetOutSnippets ();
            break;
            }
        case Exp::Type::SubqueryValue:
            {
            auto alias = derviedPropertyExp.GetColumnAlias ();
            if (alias.empty ())
                alias = derviedPropertyExp.GetNestedAlias ();

            if (alias.empty ())
                return ECSqlStatus::Error;

            NativeSqlBuilder sqlSnippet;
            sqlSnippet.Append (alias.c_str ());
            nativeSqlSnippets.push_back (sqlSnippet);
            }
            break;
        default:
            {
            //Here we presume any primitive value expression which must have a alias.
            auto alias = derviedPropertyExp.GetColumnAlias ();
            if (alias.empty ())
                alias = derviedPropertyExp.GetNestedAlias ();
           
            if (alias.empty ())
                return ECSqlStatus::Error;

            NativeSqlBuilder sqlSnippet;
            sqlSnippet.Append (alias.c_str ());
            nativeSqlSnippets.push_back (sqlSnippet);
            }
          
            break;
        }

    
    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

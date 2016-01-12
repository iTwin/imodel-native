/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPropertyNameExpPreparer.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlPropertyNameExpPreparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
void RenderPropertyMap (NativeSqlBuilder::List& snippets, PropertyMapCR propertyMap)
    {
    BeAssert (propertyMap.GetAsPropertyMapStructArray () == nullptr);
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
ECSqlStatus ECSqlPropertyNameExpPreparer::Prepare(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, PropertyNameExp const* exp)
    {
    BeAssert(exp != nullptr);
    if (exp->IsPropertyRef())
        return PrepareInSubqueryRef(nativeSqlSnippets, ctx, *exp);

    PropertyMap const& propMap = exp->GetPropertyMap();
    ECSqlPrepareContext::ExpScope const& currentScope = ctx.GetCurrentScope();

    NavigationPropertyMap const* navPropMap = propMap.GetProperty().GetIsNavigation() ? static_cast<NavigationPropertyMap const*> (&propMap) : nullptr;
    if (navPropMap != nullptr)
        {
        ECSqlStatus stat = ValidateNavigationPropertyExp(nativeSqlSnippets, ctx, *exp, *navPropMap, currentScope);
        if (!stat.IsSuccess())
            return stat;
        }
    else
        {
        if (!NeedsPreparation(currentScope, propMap))
            return ECSqlStatus::Success;
        }

    //in SQLite table aliases are only allowed for SELECT statements
    const ECSqlType currentScopeECSqlType = currentScope.GetECSqlType();

    Utf8String classIdentifier = nullptr;
    if (currentScopeECSqlType == ECSqlType::Select)
        {
        classIdentifier.assign(exp->GetClassRefExp()->GetId());
        }
    else if (currentScopeECSqlType == ECSqlType::Delete)
        {
        if (!ctx.GetCurrentScope().GetExtendedOption(ECSqlPrepareContext::ExpScope::ExtendOptions::SkipTableAliasWhenPreparingDeleteWhereClause))
            classIdentifier.assign(exp->GetPropertyMap().GetFirstColumn()->GetTable().GetName());

        if (exp->GetClassRefExp()->GetType() == Exp::Type::ClassName)
            {
            IClassMap const& classMap = static_cast<ClassNameExp const*>(exp->GetClassRefExp())->GetInfo().GetMap();
            StorageDescription const& desc = classMap.GetStorageDescription();
            bool isPolymorphic = exp->GetClassRefExp()->IsPolymorphic();
            if (isPolymorphic && desc.HierarchyMapsToMultipleTables())
                {
                BeAssert(desc.HierarchyMapsToMultipleTables() && isPolymorphic && "Returned partition is null only for a polymorphic ECSQL where subclasses are in a separate table");
                if (!classMap.HasPersistedView())
                    {
                    BeAssert(false && "[Programmer Error] Database view must exist for this class as it derive classes is map into its on table");
                    return ECSqlStatus::Error;
                    }

                classIdentifier.assign(classMap.GetPersistedViewName());
                }
            }
        }

    NativeSqlBuilder::List propNameNativeSqlSnippets = exp->GetPropertyMap().ToNativeSql((classIdentifier.empty()? nullptr : classIdentifier.c_str()), currentScopeECSqlType, exp->HasParentheses());
    nativeSqlSnippets.insert(nativeSqlSnippets.end(), propNameNativeSqlSnippets.begin(), propNameNativeSqlSnippets.end());

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
bool ECSqlPropertyNameExpPreparer::NeedsPreparation(ECSqlPrepareContext::ExpScope const& currentScope, PropertyMapCR propertyMap)
    {
    const auto currentScopeECSqlType = currentScope.GetECSqlType();

    //Property maps to virtual column which can mean that the exp doesn't need to be translated.
    if (propertyMap.IsVirtual() || (!propertyMap.IsMappedToPrimaryTable() && currentScopeECSqlType != ECSqlType::Select))
        {
        //In INSERT statements, virtual columns are always ignored
        if (currentScopeECSqlType == ECSqlType::Insert)
            return false;

        switch (currentScope.GetExp().GetType())
            {
                case Exp::Type::AssignmentList: //UPDATE SET clause
                case Exp::Type::OrderBy:
                    return false;
            }
        }

    return true;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                01/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlPropertyNameExpPreparer::ValidateNavigationPropertyExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, PropertyNameExp const& exp, NavigationPropertyMap const& propMap, ECSqlPrepareContext::ExpScope const& scope)
    {
    if (!propMap.CanOnlyHaveOneRelatedInstance())
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationProperties can only be used in ECSQL when they point to at most one related instance. NavigationProperty '%s' has a multiplicity of %s though.",
                                                               exp.ToECSql().c_str(), propMap.GetConstraint().GetCardinality().ToString().c_str());
        return ECSqlStatus::InvalidECSql;
        }

    if (propMap.GetRelationshipClassMap().GetClassMapType() == IClassMap::Type::RelationshipLinkTable)
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationProperties with ECRelationships mapped to link tables cannot be used in ECSQL. Expression: %s",
                                                               exp.ToECSql().c_str());
        return ECSqlStatus::InvalidECSql;
        }

    if (scope.GetECSqlType() == ECSqlType::Update && scope.GetExp().GetType() == Exp::Type::AssignmentList)
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationProperties cannot be used in the assignment clause of an ECSQL UPDATE statement. Expression: %s",
                                                               exp.ToECSql().c_str());
        return ECSqlStatus::InvalidECSql;
        }

    return ECSqlStatus::Success;
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

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

    if (propMap.GetType() == PropertyMap::Type::Navigation)
        {
        if (!static_cast<NavigationPropertyMap const&>(propMap).IsSupportedInECSql(true, &ctx.GetECDb()))
            return ECSqlStatus::InvalidECSql;
        }
    else
        {
        if (!NeedsPreparation(currentScope, propMap))
            return ECSqlStatus::Success;
        }

    Utf8String classIdentifier;
    ECSqlStatus stat = DetermineClassIdentifier(classIdentifier, currentScope, *exp, propMap);
    if (!stat.IsSuccess())
        return stat;

    const ECSqlType currentScopeECSqlType = currentScope.GetECSqlType();
    PropertyMap const* effectivePropMap = &propMap;
    if (currentScopeECSqlType == ECSqlType::Delete)
        {
        if (currentScope.HasExtendedOption(ECSqlPrepareContext::ExpScope::ExtendedOptions::SkipTableAliasWhenPreparingDeleteWhereClause) &&
            propMap.IsSystemPropertyMap() && propMap.GetTables().size() > 1)
            {
            BeAssert(exp->GetClassRefExp()->GetType() == Exp::Type::ClassName);
            if (exp->GetClassRefExp()->GetType() != Exp::Type::ClassName)
                return ECSqlStatus::Error;

            ClassMap const* parentOfJoinedTableClassMap = static_cast<ClassNameExp const*>(exp->GetClassRefExp())->GetInfo().GetMap().GetParentOfJoinedTable();
            effectivePropMap = parentOfJoinedTableClassMap->GetPropertyMap(propMap.GetPropertyAccessString());
            if (effectivePropMap == nullptr)
                {
                BeAssert(effectivePropMap != nullptr);
                return ECSqlStatus::Error;
                }
            }
        }

    switch (effectivePropMap->GetType())
        {
            case PropertyMap::Type::RelConstraintECClassId:
                return PrepareRelConstraintClassIdPropMap(nativeSqlSnippets, currentScopeECSqlType, *exp, *static_cast<RelConstraintECClassIdPropertyMap const*>(effectivePropMap), classIdentifier.c_str());
  
            default:
                PrepareDefault(nativeSqlSnippets, currentScopeECSqlType, *exp, *effectivePropMap, classIdentifier.c_str());
                return ECSqlStatus::Success;
        }
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
bool ECSqlPropertyNameExpPreparer::NeedsPreparation(ECSqlPrepareContext::ExpScope const& currentScope, PropertyMapCR propertyMap)
    {
    const ECSqlType currentScopeECSqlType = currentScope.GetECSqlType();

    //Property maps to virtual column which can mean that the exp doesn't need to be translated.
    RelConstraintECClassIdPropertyMap const* constraintClassIdPropMap = propertyMap.GetType() == PropertyMap::Type::RelConstraintECClassId ? static_cast<RelConstraintECClassIdPropertyMap const*>(&propertyMap) : nullptr;
    if (propertyMap.IsVirtual() || (constraintClassIdPropMap != nullptr && !constraintClassIdPropMap->IsMappedToClassMapTables() && currentScopeECSqlType != ECSqlType::Select))
        {
        //In INSERT statements, virtual columns are always ignored
        if (currentScopeECSqlType == ECSqlType::Insert)
            {
            if (ECDbSystemSchemaHelper::IsSystemProperty(propertyMap.GetProperty(), ECSqlSystemProperty::ECClassId))
                {
                return true;
                }

            return false;
            }

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
// @bsimethod                                    Krischan.Eberle                    07/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlPropertyNameExpPreparer::DetermineClassIdentifier(Utf8StringR classIdentifier, ECSqlPrepareContext::ExpScope const& scope, PropertyNameExp const& exp, PropertyMap const& propMap)
    {
    switch (scope.GetECSqlType())
        {
            case ECSqlType::Select:
                classIdentifier.assign(exp.GetClassRefExp()->GetId());
                break;

            case ECSqlType::Delete:
            {
            if (exp.GetClassRefExp()->GetType() == Exp::Type::ClassName)
                {
                ClassMap const& classMap = static_cast<ClassNameExp const*>(exp.GetClassRefExp())->GetInfo().GetMap();
                StorageDescription const& desc = classMap.GetStorageDescription();
                if (exp.GetClassRefExp()->IsPolymorphic() && desc.HierarchyMapsToMultipleTables())
                    {
                    classIdentifier.assign(classMap.GetUpdatableViewName());
                    break;
                    }
                }

            if (!scope.HasExtendedOption(ECSqlPrepareContext::ExpScope::ExtendedOptions::SkipTableAliasWhenPreparingDeleteWhereClause))
                classIdentifier.assign(propMap.GetSingleTable()->GetName());

            break;
            }

            default:
                break;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    07/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlPropertyNameExpPreparer::PrepareDefault(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlType ecsqlType, PropertyNameExp const& exp, PropertyMap const& propMap, Utf8CP classIdentifier)
    {
    NativeSqlBuilder::List propNameNativeSqlSnippets = propMap.ToNativeSql(classIdentifier, ecsqlType, exp.HasParentheses());
    nativeSqlSnippets.insert(nativeSqlSnippets.end(), propNameNativeSqlSnippets.begin(), propNameNativeSqlSnippets.end());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    07/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlPropertyNameExpPreparer::PrepareRelConstraintClassIdPropMap(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlType ecsqlType, PropertyNameExp const& exp, RelConstraintECClassIdPropertyMap const& propMap, Utf8CP classIdentifier)
    {
    if ((ecsqlType == ECSqlType::Delete || ecsqlType == ECSqlType::Update) &&
        !propMap.IsMappedToClassMapTables() && !propMap.IsVirtual())
        {
        if (exp.GetClassRefExp()->GetType() != Exp::Type::ClassName)
            {
            BeAssert(exp.GetClassRefExp()->GetType() == Exp::Type::ClassName);
            return ECSqlStatus::InvalidECSql;
            }

        ClassMap const& classMap = static_cast<ClassNameExp const*>(exp.GetClassRefExp())->GetInfo().GetMap();
        if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
            {
            RelationshipClassEndTableMap const& relClassMap = static_cast<RelationshipClassEndTableMap const&> (classMap);
            RelationshipConstraintMap const& referencedEndConstraintMappings = relClassMap.GetConstraintMap(relClassMap.GetReferencedEnd());
            RelConstraintECClassIdPropertyMap const* classIdPropMap = referencedEndConstraintMappings.GetECClassIdPropMap();
            if (classIdPropMap == &propMap)
                {
                DbColumn const* classIdColumn = classIdPropMap->GetSingleColumn();
                NativeSqlBuilder str;
                str.AppendFormatted("(SELECT [%s] FROM [%s] WHERE [%s] = [%s] LIMIT 1)",
                                    classIdColumn->GetName().c_str(),
                                    classIdColumn->GetTable().GetName().c_str(),
                                    classIdColumn->GetTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId)->GetName().c_str(),
                                    referencedEndConstraintMappings.GetECInstanceIdPropMap()->GetSingleColumn()->GetName().c_str());

                nativeSqlSnippets.push_back(str);
                return ECSqlStatus::Success;
                }
            }
        }

    PrepareDefault(nativeSqlSnippets, ecsqlType, exp, propMap, classIdentifier);
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
                auto stat = PrepareInSubqueryRef (snippets, ctx, *propertyName);
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

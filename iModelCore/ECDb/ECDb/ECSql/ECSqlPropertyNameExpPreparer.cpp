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

    //in SQLite table aliases are only allowed for SELECT statements
    const ECSqlType currentScopeECSqlType = currentScope.GetECSqlType();
    PropertyMapCP propertyMap = &exp->GetPropertyMap();
    Utf8String classIdentifier = nullptr;
    if (currentScopeECSqlType == ECSqlType::Select)
        {
        classIdentifier.assign(exp->GetClassRefExp()->GetId());
        }
    else if (currentScopeECSqlType == ECSqlType::Delete)
        {
        if (!currentScope.HasExtendedOption(ECSqlPrepareContext::ExpScope::ExtendedOptions::SkipTableAliasWhenPreparingDeleteWhereClause))
            {
            classIdentifier.assign(exp->GetPropertyMap().GetSingleTable()->GetName());
            }
        else
            {
            if (propertyMap->IsSystemPropertyMap() && propertyMap->GetTables().size() > 1)
                {
                BeAssert(exp->GetClassRefExp()->GetType() == Exp::Type::ClassName);
                if (exp->GetClassRefExp()->GetType() != Exp::Type::ClassName)
                    return ECSqlStatus::Error;

                auto parentOfJoinedTable = static_cast<ClassNameExp const*>(exp->GetClassRefExp())->GetInfo().GetMap().GetParentOfJoinedTable();
                propertyMap = parentOfJoinedTable->GetPropertyMap(propertyMap->GetPropertyAccessString());
                BeAssert(propertyMap != nullptr);
                if (propertyMap == nullptr)
                    return ECSqlStatus::Error;
                }
            }

        if (exp->GetClassRefExp()->GetType() == Exp::Type::ClassName)
            {
            ClassMap const& classMap = static_cast<ClassNameExp const*>(exp->GetClassRefExp())->GetInfo().GetMap();
            StorageDescription const& desc = classMap.GetStorageDescription();
            bool isPolymorphic = exp->GetClassRefExp()->IsPolymorphic();
            if (isPolymorphic && desc.HierarchyMapsToMultipleTables())
                {
                BeAssert(desc.HierarchyMapsToMultipleTables() && isPolymorphic && "Returned partition is null only for a polymorphic ECSQL where subclasses are in a separate table");
                classIdentifier.assign(classMap.GetUpdatableViewName());
                }
            }
        }

    if (currentScopeECSqlType == ECSqlType::Delete || currentScopeECSqlType == ECSqlType::Update)
        {
        if (auto typeIdPM = dynamic_cast<RelConstraintECClassIdPropertyMap const*>(propertyMap))
            {
            if (!typeIdPM->IsMappedToClassMapTables() && !typeIdPM->IsVirtual())
                {
                if (exp->GetClassRefExp()->GetType() != Exp::Type::ClassName)
                    {
                    BeAssert(exp->GetClassRefExp()->GetType() == Exp::Type::ClassName);
                    return ECSqlStatus::InvalidECSql;
                    }

                if (auto endTableMap = dynamic_cast<RelationshipClassEndTableMap const*>(&static_cast<ClassNameExp const*>(exp->GetClassRefExp())->GetInfo().GetMap()))
                    {
                    auto classIdPropMap = endTableMap->GetConstraintMap(endTableMap->GetReferencedEnd()).GetECClassIdPropMap();
                    auto ecInstanceidPropMap = endTableMap->GetConstraintMap(endTableMap->GetReferencedEnd()).GetECInstanceIdPropMap();
                    if (classIdPropMap == typeIdPM)
                        {
                        auto classIdColumn = classIdPropMap->GetSingleColumn();
                        NativeSqlBuilder str;
                        str.AppendFormatted("(SELECT [%s] FROM [%s] WHERE [%s] = [%s] LIMIT 1)",
                            classIdColumn->GetName().c_str(),
                            classIdColumn->GetTable().GetName().c_str(),
                            classIdColumn->GetTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId)->GetName().c_str(),
                            ecInstanceidPropMap->GetSingleColumn()->GetName().c_str());

                        nativeSqlSnippets.push_back(str);
                        return ECSqlStatus::Success;
                        }
                    }
                }
            }
        }
    NativeSqlBuilder::List propNameNativeSqlSnippets = propertyMap->ToNativeSql((classIdentifier.empty()? nullptr : classIdentifier.c_str()), currentScopeECSqlType, exp->HasParentheses());
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
    RelConstraintECClassIdPropertyMap const* constraintClassIdPropMap = propertyMap.GetType() == PropertyMap::Type::RelConstraintECClassId ? static_cast<RelConstraintECClassIdPropertyMap const*>(&propertyMap) : nullptr;
    if (propertyMap.IsVirtual() || (constraintClassIdPropMap != nullptr && !constraintClassIdPropMap->IsMappedToClassMapTables() && currentScopeECSqlType != ECSqlType::Select))
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

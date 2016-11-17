/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPropertyNameExpPreparer.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlPropertyNameExpPreparer.h"

USING_NAMESPACE_BENTLEY_EC

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

    if (!NeedsPreparation(currentScope, propMap))
        return ECSqlStatus::Success;

    Utf8String classIdentifier;
    ECSqlStatus stat = DetermineClassIdentifier(classIdentifier, currentScope, *exp, propMap);
    if (!stat.IsSuccess())
        return stat;

    const ECSqlType currentScopeECSqlType = currentScope.GetECSqlType();
    PropertyMap const* effectivePropMap = &propMap;
    if (currentScopeECSqlType == ECSqlType::Delete)
        {
        if (currentScope.HasExtendedOption(ECSqlPrepareContext::ExpScope::ExtendedOptions::SkipTableAliasWhenPreparingDeleteWhereClause) &&
            propMap.IsSystem() && static_cast<SystemPropertyMap const&>(propMap).GetTables().size() > 1)
            {
            BeAssert(exp->GetClassRefExp()->GetType() == Exp::Type::ClassName);
            if (exp->GetClassRefExp()->GetType() != Exp::Type::ClassName)
                return ECSqlStatus::Error;

            ClassMap const& classMap = static_cast<ClassNameExp const*>(exp->GetClassRefExp())->GetInfo().GetMap();
            if (classMap.GetMapStrategy().IsTablePerHierarchy() && classMap.GetTphHelper()->HasJoinedTable())
                {
                ECClassId rootClassId = classMap.GetTphHelper()->DetermineTphRootClassId();
                BeAssert(rootClassId.IsValid());
                ECClassCP rootClass = ctx.GetECDb().Schemas().GetECClass(rootClassId);
                BeAssert(rootClass != nullptr);
                ClassMap const* rootClassMap = ctx.GetECDb().Schemas().GetDbMap().GetClassMap(*rootClass);
                BeAssert(rootClassMap != nullptr);
                effectivePropMap = rootClassMap->GetPropertyMaps().Find(propMap.GetAccessString().c_str());
                if (effectivePropMap == nullptr)
                    {
                    BeAssert(effectivePropMap != nullptr);
                    return ECSqlStatus::Error;
                    }
                }
            }
        }

    switch (effectivePropMap->GetType())
        {
            case PropertyMap::Type::ConstraintECClassId:
                return PrepareRelConstraintClassIdPropMap(nativeSqlSnippets, currentScopeECSqlType, *exp, *static_cast<ConstraintECClassIdPropertyMap const*>(effectivePropMap), classIdentifier.c_str());

            default:
                PrepareDefault(nativeSqlSnippets, currentScopeECSqlType, *exp, *effectivePropMap, classIdentifier.c_str());
                return ECSqlStatus::Success;
        }
    }

//Alias
//  UPDATE
//  INSERT
//  DELETE
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
bool ECSqlPropertyNameExpPreparer::NeedsPreparation(ECSqlPrepareContext::ExpScope const& currentScope, PropertyMap const& propertyMap)
    {
    const ECSqlType currentScopeECSqlType = currentScope.GetECSqlType();
    GetColumnsPropertyMapVisitor columnVisitor(PropertyMap::Type::All, true);
    propertyMap.AcceptVisitor(columnVisitor);
    if (columnVisitor.GetColumns().empty())
        {
        BeAssert(false && "WIP");
        return false;
        }
 
    //Property maps to virtual column which can mean that the exp doesn't need to be translated.
    ConstraintECClassIdPropertyMap const* constraintClassIdPropMap = propertyMap.GetType() == PropertyMap::Type::ConstraintECClassId ? static_cast<ConstraintECClassIdPropertyMap const*>(&propertyMap) : nullptr;
    bool isConstraintIdPropertyMap = (constraintClassIdPropMap != nullptr && !constraintClassIdPropMap->IsMappedToClassMapTables() && currentScopeECSqlType != ECSqlType::Select);
    if (columnVisitor.AllColumnsAreVirtual() || isConstraintIdPropertyMap)
        {
        //In INSERT statements, virtual columns are always ignored
        if (currentScopeECSqlType == ECSqlType::Insert)
            {
            if (ECDbSystemSchemaHelper::IsSystemProperty(propertyMap.GetProperty(), ECSqlSystemPropertyKind::ECClassId))
                {
                return true;
                }

            if (propertyMap.IsData())
                {
                return static_cast<DataPropertyMap const&>(propertyMap).IsOverflow();
                }

            return false;
            }
        
        if (currentScopeECSqlType == ECSqlType::Update)
            {
            if (propertyMap.IsData() && static_cast<DataPropertyMap const&>(propertyMap).IsOverflow())
                {
                return true;
                }
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
    //ClassMap const& classMap = propMap.GetClassMap();
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

            //if (!scope.HasExtendedOption(ECSqlPrepareContext::ExpScope::ExtendedOptions::SkipTableAliasWhenPreparingDeleteWhereClause))
            //    classIdentifier.assign(classMap.GetJoinedTable().GetName());

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
    if (propMap.GetType() == PropertyMap::Type::NavigationRelECClassId)
        {
        NavigationPropertyMap::RelECClassIdPropertyMap const& classIdPm = static_cast<NavigationPropertyMap::RelECClassIdPropertyMap const&>(propMap);
        if (classIdPm.IsVirtual())
            {
            if (exp.FindParent(Exp::Type::Where) != nullptr)
                {
                NativeSqlBuilder sql;
                sql.Append(classIdPm.GetDefaultClassId());
                nativeSqlSnippets.push_back(sql);
                return;
                }
            }
        }

    ClassMap const& classMap = propMap.GetClassMap();
    ToSqlPropertyMapVisitor sqlVisitor(classMap.GetJoinedTable(),
                                        ecsqlType == ECSqlType::Select ? ToSqlPropertyMapVisitor::SqlTarget::SelectView : ToSqlPropertyMapVisitor::SqlTarget::Table, classIdentifier, exp.HasParentheses());

    bool isWriteData = (ecsqlType == ECSqlType::Insert && exp.GetParent()->GetType() == Exp::Type::PropertyNameList)
        || (ecsqlType == ECSqlType::Update && exp.GetParent()->GetType() == Exp::Type::Assignment);

    if (isWriteData)
        sqlVisitor.EnableSqlForInsertOrUpdate();

    propMap.AcceptVisitor(sqlVisitor);
    for (ToSqlPropertyMapVisitor::Result const& r : sqlVisitor.GetResultSet())
        {
        //If is a INSERT prop name list clause and the column is virtual, ignore it, as there is nothing to insert in that case
        //(we must check for the prop name list clause, because if it shows up in the values list, it must not be ignored)
        //INSERT INTO Foo(SourceECClassId) -> ignore SourceECClassId if it maps to a virtual column
        //INSERT INTO Foo(MyProp) VALUES(ECClassId + 1000) -> never ignore. If virtual, the ECClassId from the respective ECClass is used
        if (isWriteData)
            {
            if (r.GetColumn().GetPersistenceType() == PersistenceType::Virtual && !r.GetColumn().IsOverflow())
                continue;
            }
        nativeSqlSnippets.push_back(r.GetSqlBuilder());
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    07/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlPropertyNameExpPreparer::PrepareRelConstraintClassIdPropMap(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlType ecsqlType, PropertyNameExp const& exp, ConstraintECClassIdPropertyMap const& propMap, Utf8CP classIdentifier)
    {
    GetColumnsPropertyMapVisitor columnVisitor(PropertyMap::Type::All, true);
    propMap.AcceptVisitor(columnVisitor);

    if ((ecsqlType == ECSqlType::Delete || ecsqlType == ECSqlType::Update) &&
        !propMap.IsMappedToClassMapTables() && !columnVisitor.AllColumnsAreVirtual())
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

            ConstraintECClassIdPropertyMap const* classIdPropMap = referencedEndConstraintMappings.GetECClassIdPropMap();
            if (classIdPropMap == &propMap)
                {
                GetColumnsPropertyMapVisitor  classIdVisitor(PropertyMap::Type::All, true);
                classIdPropMap->AcceptVisitor(classIdVisitor);
                DbColumn const* classIdColumn = classIdVisitor.GetSingleColumn();

                GetColumnsPropertyMapVisitor  ecInstanceIdVisitor(PropertyMap::Type::All, true);
                referencedEndConstraintMappings.GetECInstanceIdPropMap()->AcceptVisitor(ecInstanceIdVisitor);
                NativeSqlBuilder str;
                str.AppendFormatted("(SELECT [%s] FROM [%s] WHERE [%s] = [%s] LIMIT 1)",
                                    classIdColumn->GetName().c_str(),
                                    classIdColumn->GetTable().GetName().c_str(),
                                    classIdColumn->GetTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId)->GetName().c_str(),
                                    ecInstanceIdVisitor.GetSingleColumn()->GetName().c_str());

                nativeSqlSnippets.push_back(str); 
                return ECSqlStatus::Success;
                }
            }
        }

    NativeSqlBuilder selectSql;
    if (ecsqlType == ECSqlType::Delete || ecsqlType == ECSqlType::Update)
        {
        if (columnVisitor.AllColumnsAreVirtual())
            selectSql.Append(propMap.GetDefaultECClassId());
        else
            selectSql.Append(classIdentifier, columnVisitor.GetSingleColumn()->GetName().c_str());
        }
    else
        selectSql.Append(classIdentifier, propMap.GetAccessString().c_str());

    nativeSqlSnippets.push_back(selectSql);

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                         08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlPropertyNameExpPreparer::PrepareInSubqueryRef(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, PropertyNameExp const& exp)
    {
    PropertyNameExp::PropertyRef* propertyRef = const_cast<PropertyNameExp&>(exp).GetPropertyRefP();
    DerivedPropertyExp const& derivedPropertyExp = propertyRef->LinkedTo();
    if (derivedPropertyExp.GetName().empty())
        {
        BeAssert("Nested expression must have a name/alias" && false);
        return ECSqlStatus::Error;
        }

    ValueExp const* valueExp = derivedPropertyExp.GetExpression();
    //1. Exp-> PropertyName    useSameColumnNames
    //2. Exp-> ValueExpr       useAlias
    //3. Exp-> ScalarQuery     useAlias
    switch (valueExp->GetType())
        {
            case Exp::Type::PropertyName:
            {
            PropertyNameExp const* propertyName = static_cast <PropertyNameExp const*>(valueExp);
            if (!propertyName->IsPropertyRef())
                {
                if (!propertyRef->IsConverted())
                    {
                    PropertyMap const& propertyMap = propertyName->GetPropertyMap();
                    ToSqlPropertyMapVisitor sqlVisitor(propertyMap.GetClassMap().GetJoinedTable(), ToSqlPropertyMapVisitor::SelectView, nullptr);
                    propertyMap.AcceptVisitor(sqlVisitor);
                    NativeSqlBuilder::List snippets;
                    for (auto const&r : sqlVisitor.GetResultSet())
                        snippets.push_back(r.GetSqlBuilder());

                    if (SUCCESS != propertyRef->ToNativeSql(snippets))
                        return ECSqlStatus::Error;
                    }
                }
            else
                {
                NativeSqlBuilder::List snippets;
                ctx.PushScope(ctx.GetCurrentScope().GetExp());
                ECSqlStatus stat = PrepareInSubqueryRef(snippets, ctx, *propertyName);
                if (!stat.IsSuccess())
                    return stat;

                ctx.PopScope();

                if (SUCCESS != propertyRef->ToNativeSql(snippets))
                    return ECSqlStatus::Error;
                }

            nativeSqlSnippets = propertyRef->GetOutSnippets();
            break;
            }
            case Exp::Type::SubqueryValue:
            {
            Utf8String alias = derivedPropertyExp.GetColumnAlias();
            if (alias.empty())
                alias = derivedPropertyExp.GetNestedAlias();

            if (alias.empty())
                return ECSqlStatus::Error;

            NativeSqlBuilder sqlSnippet;
            sqlSnippet.Append(alias.c_str());
            nativeSqlSnippets.push_back(sqlSnippet);
            }
            break;
            default:
            {
            //Here we presume any primitive value expression which must have a alias.
            Utf8String alias = derivedPropertyExp.GetColumnAlias();
            if (alias.empty())
                alias = derivedPropertyExp.GetNestedAlias();

            if (alias.empty())
                return ECSqlStatus::Error;

            NativeSqlBuilder sqlSnippet;
            sqlSnippet.Append(alias.c_str());
            nativeSqlSnippets.push_back(sqlSnippet);
            }

            break;
        }


    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

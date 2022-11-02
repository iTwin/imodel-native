/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlPropertyNameExpPreparer.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlPropertyNameExpPreparer::Prepare(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, PropertyNameExp const& exp)
    {
    if (exp.IsPropertyRef())
        return PrepareInSubqueryRef(nativeSqlSnippets, ctx, exp);

    if (exp.IsVirtualProperty()) {
        BeAssert(exp.GetPropertyPath().Size() == 1);
        NativeSqlBuilder builder;
        if (ctx.GetCurrentScope().GetECSqlType() == ECSqlType::Select) {
            // we only append property scope only if its different from tablevalued class name and not empty
            const auto classAlias = exp.GetClassRefExp()->GetId();
            if (!classAlias.empty() && !classAlias.EqualsIAscii(exp.GetVirtualProperty()->GetClass().GetName())) {
                builder.Append(classAlias);
                builder.AppendDot();
            }
        } 
        builder.Append(exp.GetVirtualProperty()->GetName());
        nativeSqlSnippets.push_back(builder);
        return ECSqlStatus::Success;
    }
    PropertyMap const& propMap = exp.GetPropertyMap();
    ECSqlPrepareContext::ExpScope const& currentScope = ctx.GetCurrentScope();

    if (!NeedsPreparation(ctx, currentScope, propMap))
        return ECSqlStatus::Success;

    const ECSqlType currentScopeECSqlType = currentScope.GetECSqlType();
    PropertyMap const* effectivePropMap = &propMap;
    if (currentScopeECSqlType == ECSqlType::Delete)
        {
        if (currentScope.HasExtendedOption(ECSqlPrepareContext::ExpScope::ExtendedOptions::UsePrimaryTableForSystemPropertyResolution) &&
            propMap.IsSystem() && propMap.GetAs<SystemPropertyMap>().GetTables().size() > 1)
            {
            BeAssert(exp.GetClassRefExp()->GetType() == Exp::Type::ClassName);
            if (exp.GetClassRefExp()->GetType() != Exp::Type::ClassName)
                return ECSqlStatus::Error;

            ClassMap const& classMap = exp.GetClassRefExp()->GetAs<ClassNameExp>().GetInfo().GetMap();
            if (classMap.GetMapStrategy().IsTablePerHierarchy())
                {
                ECClassId rootClassId = classMap.GetTphHelper()->DetermineTphRootClassId();
                BeAssert(rootClassId.IsValid());
                ECClassCP rootClass = ctx.GetECDb().Schemas().GetClass(rootClassId);
                BeAssert(rootClass != nullptr);
                ClassMap const* rootClassMap = classMap.GetSchemaManager().GetClassMap(*rootClass);
                BeAssert(rootClassMap != nullptr);
                effectivePropMap = rootClassMap->GetPropertyMaps().Find(propMap.GetAccessString().c_str());
                if (effectivePropMap == nullptr)
                    {
                    BeAssert(false);
                    return ECSqlStatus::Error;
                    }
                }
            }
        }

    Utf8String classIdentifier;
    if (currentScope.GetECSqlType() == ECSqlType::Select)
        classIdentifier.assign(exp.GetClassRefExp()->GetId());

    switch (effectivePropMap->GetType())
        {
            case PropertyMap::Type::ConstraintECClassId:
                return PrepareRelConstraintClassIdPropMap(nativeSqlSnippets, currentScopeECSqlType, exp, effectivePropMap->GetAs<ConstraintECClassIdPropertyMap>(), classIdentifier);

            default:
                PrepareDefault(nativeSqlSnippets, ctx, currentScopeECSqlType, exp, *effectivePropMap, classIdentifier);
                return ECSqlStatus::Success;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
bool ECSqlPropertyNameExpPreparer::NeedsPreparation(ECSqlPrepareContext& ctx, ECSqlPrepareContext::ExpScope const& currentScope, PropertyMap const& propertyMap)
    {
    GetColumnsPropertyMapVisitor columnVisitor(PropertyMap::Type::All, true);
    propertyMap.AcceptVisitor(columnVisitor);
    if (columnVisitor.GetColumns().empty())
        {
        BeAssert(false && "WIP");
        return false;
        }
 
    const ECSqlType currentScopeECSqlType = currentScope.GetECSqlType();
    //Property maps to virtual column which can mean that the exp doesn't need to be translated.
    ConstraintECClassIdPropertyMap const* constraintClassIdPropMap = propertyMap.GetType() == PropertyMap::Type::ConstraintECClassId ? &propertyMap.GetAs<ConstraintECClassIdPropertyMap>() : nullptr;
    const bool isConstraintIdPropertyMap = (constraintClassIdPropMap != nullptr && !constraintClassIdPropMap->IsMappedToClassMapTables() && currentScopeECSqlType != ECSqlType::Select);
    const bool allColumnsAreVirtual = columnVisitor.GetVirtualColumnCount() == columnVisitor.GetColumnCount();

    if (allColumnsAreVirtual || isConstraintIdPropertyMap)
        {
        //In INSERT statements, virtual columns are always ignored
        if (currentScopeECSqlType == ECSqlType::Insert)
            return (ctx.GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemPropertyInfo(propertyMap.GetProperty()) == ECSqlSystemPropertyInfo::ECClassId());
        
        switch (currentScope.GetExp().GetType())
            {
                case Exp::Type::AssignmentList: //UPDATE SET clause
                    return false;
            }
        }

    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlPropertyNameExpPreparer::PrepareDefault(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ECSqlType ecsqlType, PropertyNameExp const& exp, PropertyMap const& propMap, Utf8StringCR classIdentifier)
    {
    ToSqlPropertyMapVisitor::ECSqlScope scope;
    if (ecsqlType == ECSqlType::Select)
        scope = ToSqlPropertyMapVisitor::ECSqlScope::Select;
    else
        scope = exp.IsLhsAssignmentOperandExpression() ? ToSqlPropertyMapVisitor::ECSqlScope::NonSelectAssignmentExp : ToSqlPropertyMapVisitor::ECSqlScope::NonSelectNoAssignmentExp;

    DbTable const* contextTable = nullptr;
    if (ecsqlType == ECSqlType::Insert || ecsqlType == ECSqlType::Update)
        contextTable = &ctx.GetPreparedStatement<SingleContextTableECSqlPreparedStatement>().GetContextTable();
    else
        contextTable = &propMap.GetClassMap().GetJoinedOrPrimaryTable();

    ToSqlPropertyMapVisitor sqlVisitor(*contextTable, scope, classIdentifier, exp.HasParentheses());
    propMap.AcceptVisitor(sqlVisitor);
    for (ToSqlPropertyMapVisitor::Result const& r : sqlVisitor.GetResultSet())
        {
        //If is a INSERT prop name list clause and the column is virtual, ignore it, as there is nothing to insert in that case
        //(we must check for the prop name list clause, because if it shows up in the values list, it must not be ignored)
        //INSERT INTO Foo(SourceECClassId) -> ignore SourceECClassId if it maps to a virtual column
        //INSERT INTO Foo(MyProp) VALUES(ECClassId + 1000) -> never ignore. If virtual, the ECClassId from the respective ECClass is used
        if (sqlVisitor.IsForAssignmentExpression() && r.GetColumn().GetPersistenceType() == PersistenceType::Virtual)
            continue;
        
        nativeSqlSnippets.push_back(r.GetSqlBuilder());
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlPropertyNameExpPreparer::PrepareRelConstraintClassIdPropMap(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlType ecsqlType, PropertyNameExp const& exp, ConstraintECClassIdPropertyMap const& propMap, Utf8StringCR classIdentifier)
    {
    GetColumnsPropertyMapVisitor columnVisitor(PropertyMap::Type::All, true);
    propMap.AcceptVisitor(columnVisitor);
    if (columnVisitor.GetColumns().empty())
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    if ((ecsqlType == ECSqlType::Delete || ecsqlType == ECSqlType::Update) &&
        !propMap.IsMappedToClassMapTables() && 
        columnVisitor.GetVirtualColumnCount() < columnVisitor.GetColumnCount())
        {
        if (exp.GetClassRefExp()->GetType() != Exp::Type::ClassName)
            {
            BeAssert(exp.GetClassRefExp()->GetType() == Exp::Type::ClassName);
            return ECSqlStatus::InvalidECSql;
            }

        ClassMap const& classMap = static_cast<ClassNameExp const*>(exp.GetClassRefExp())->GetInfo().GetMap();
        if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
            {
            RelationshipClassEndTableMap const& relClassMap = classMap.GetAs<RelationshipClassEndTableMap>();
            RelationshipConstraintMap const& referencedEndConstraintMappings = relClassMap.GetConstraintMap(relClassMap.GetReferencedEnd());

            ConstraintECClassIdPropertyMap const* classIdPropMap = referencedEndConstraintMappings.GetECClassIdPropMap();
            if (classIdPropMap == &propMap)
                {
                GetColumnsPropertyMapVisitor classIdVisitor(PropertyMap::Type::All, true);
                classIdPropMap->AcceptVisitor(classIdVisitor);
                DbColumn const* classIdColumn = classIdVisitor.GetSingleColumn();

                GetColumnsPropertyMapVisitor ecInstanceIdVisitor(PropertyMap::Type::All, true);
                referencedEndConstraintMappings.GetECInstanceIdPropMap()->AcceptVisitor(ecInstanceIdVisitor);
                NativeSqlBuilder str;
                str.AppendFormatted("(SELECT [%s] FROM [%s] WHERE [%s]=[%s] LIMIT 1)",
                                    classIdColumn->GetName().c_str(),
                                    classIdColumn->GetTable().GetName().c_str(),
                                    classIdColumn->GetTable().FindFirst(DbColumn::Kind::ECInstanceId)->GetName().c_str(),
                                    ecInstanceIdVisitor.GetSingleColumn()->GetName().c_str());

                nativeSqlSnippets.push_back(str);
                return ECSqlStatus::Success;
                }
            }
        }

    NativeSqlBuilder selectSql;
    if (ecsqlType == ECSqlType::Delete || ecsqlType == ECSqlType::Update)
        {
        if (columnVisitor.GetVirtualColumnCount() == columnVisitor.GetColumnCount())
            {
            BeAssert(propMap.GetDataPropertyMaps().size() == 1 && "DELETE or UPDATE against relationships with constraint class ids mapped to multiple tables should have been caught before.");
            BeAssert(propMap.GetDataPropertyMaps()[0]->GetType() == PropertyMap::Type::SystemPerTableClassId);
            SystemPropertyMap::PerTableClassIdPropertyMap const& perTablePropMap = propMap.GetDataPropertyMaps()[0]->GetAs<SystemPropertyMap::PerTableClassIdPropertyMap>();
            BeAssert(perTablePropMap.GetDefaultECClassId().IsValid());
            selectSql.Append(perTablePropMap.GetDefaultECClassId());
            }
        else
            selectSql.AppendFullyQualified(classIdentifier, columnVisitor.GetSingleColumn()->GetName());
        }
    else
        selectSql.AppendFullyQualified(classIdentifier, propMap.GetAccessString());

    nativeSqlSnippets.push_back(selectSql);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlPropertyNameExpPreparer::PrepareInSubqueryRef(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, PropertyNameExp const& exp)
    {
    PropertyNameExp::PropertyRef const* propertyRef = exp.GetPropertyRef();
    DerivedPropertyExp const& referencedDerivedPropertyExp = propertyRef->LinkedTo();
    if (referencedDerivedPropertyExp.GetName().empty())
        {
        BeAssert("Nested expression must have a name/alias" && false);
        return ECSqlStatus::Error;
        }

    ValueExp const* referencedValueExp = referencedDerivedPropertyExp.GetExpression();
    //1. Exp-> PropertyName    useSameColumnNames
    //2. Exp-> ValueExpr       useAlias
    //3. Exp-> ScalarQuery     useAlias
    switch (referencedValueExp->GetType())
        {
            case Exp::Type::PropertyName:
            {
            PropertyNameExp const& referencedPropertyNameExp = referencedValueExp->GetAs<PropertyNameExp>();
            if (!referencedPropertyNameExp.IsPropertyRef())
                {
                if (!propertyRef->WasToNativeSqlCalled())
                    {
                    if (const auto virtualProp = referencedPropertyNameExp.GetVirtualProperty()) {
                        nativeSqlSnippets.push_back(NativeSqlBuilder(virtualProp->GetName()));
                        if (SUCCESS != propertyRef->ToNativeSql(nativeSqlSnippets)) {
                            return ECSqlStatus::Error;
                        }
                        nativeSqlSnippets = propertyRef->GetNativeSql();
                        return ECSqlStatus::Success;
                    }
                    PropertyMap const& propertyMap = referencedPropertyNameExp.GetPropertyMap();
                    NativeSqlBuilder::List snippets;
                    if (propertyMap.GetType() == PropertyMap::Type::ConstraintECClassId)
                        {
                        snippets.push_back(NativeSqlBuilder(propertyMap.GetAccessString()));
                        if (SUCCESS != propertyRef->ToNativeSql(snippets))
                            return ECSqlStatus::Error;
                        }
                    else
                        {
                        Utf8String accessStringPrefix = exp.GetPropertyPath().ToString();
                        const bool thisPropertyIsAlias = propertyRef->ReferToAlias();
                        if (thisPropertyIsAlias)
                            {
                            if (exp.GetPropertyPath().Size() == 1)
                                accessStringPrefix.clear();
                            else
                                {
                                // Alias qualified property
                                accessStringPrefix = propertyMap.GetAccessString() + "." +  exp.GetPropertyPath().Skip(1).ToString();
                                }
                            }

                        ToSqlPropertyMapVisitor sqlVisitor(propertyMap.GetClassMap().GetJoinedOrPrimaryTable(), ToSqlPropertyMapVisitor::ECSqlScope::Select);
                        propertyMap.AcceptVisitor(sqlVisitor);                       
                        std::vector<bool> filter;
                        for (ToSqlPropertyMapVisitor::Result const& r : sqlVisitor.GetResultSet())
                            {
                            const bool isMatch = r.GetPropertyMap().GetAccessString().StartsWithIAscii(accessStringPrefix.c_str());
                            filter.push_back(isMatch || accessStringPrefix.empty());
                            snippets.push_back(r.GetSqlBuilder());
                            }

                        if (SUCCESS != propertyRef->ToNativeSql(snippets, filter))
                            return ECSqlStatus::Error;
                        }
                    }
                }
            else
                {
                NativeSqlBuilder::List snippets;
                ctx.PushScope(ctx.GetCurrentScope().GetExp());
                ECSqlStatus stat = PrepareInSubqueryRef(snippets, ctx, referencedPropertyNameExp);
                if (!stat.IsSuccess())
                    return stat;

                ctx.PopScope();

                if (SUCCESS != propertyRef->ToNativeSql(snippets))
                    return ECSqlStatus::Error;
                }

            nativeSqlSnippets = propertyRef->GetNativeSql();
            break;
            }
            default: {
                if (exp.GetClassRefExp() != nullptr && exp.GetClassRefExp()->GetType() == Exp::Type::CommonTableBlockName) {
                    NativeSqlBuilder sqlSnippet;
                    auto& ctb = exp.GetClassRefExp()->GetAs<CommonTableBlockNameExp>();
                    if (!ctb.GetAlias().empty()) {
                        sqlSnippet.Append(ctb.GetAlias());
                    } else {
                        sqlSnippet.Append(ctb.GetName());
                    }
                    sqlSnippet.AppendDot();
                    sqlSnippet.Append(exp.GetPropertyPath().Last().GetName());
                    nativeSqlSnippets.push_back(sqlSnippet);
                } else {
                    //Here we presume any primitive value expression which must have a alias.
                    Utf8String alias = referencedDerivedPropertyExp.GetColumnAlias();
                    if (alias.empty() || referencedDerivedPropertyExp.OriginateInASubQuery())
                        alias = referencedDerivedPropertyExp.GetNestedAlias();

                    if (alias.empty())
                        return ECSqlStatus::Error;

                    NativeSqlBuilder sqlSnippet;
                    sqlSnippet.AppendEscaped(alias);
                    nativeSqlSnippets.push_back(sqlSnippet);
                }
                break;
            }
        }

    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

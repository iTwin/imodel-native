/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlInsertPreparer.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <Bentley/BeStringUtilities.h>
#include "ECSqlInsertPreparer.h"
#include "ECSqlPropertyNameExpPreparer.h"
#include "IdECSqlBinder.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlStatus ECSqlInsertPreparer::Prepare(ECSqlPrepareContext& ctx, InsertStatementExp const& exp)
    {
    BeAssert(exp.IsComplete());
    ctx.PushScope(exp);

    ClassMap const& classMap = exp.GetClassNameExp()->GetInfo().GetMap();

    DbTable const& table = ctx.GetPreparedStatement<SingleContextTableECSqlPreparedStatement>().GetContextTable();
    if (table.GetType() == DbTable::Type::Virtual)
        {
        if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report("ECRelationshipClass' foreign key end is abstract. Cannot insert into such an ECRelationshipClass.");
            return ECSqlStatus::InvalidECSql;
            }

        BeAssert(false && "Should have been caught before");
        return ECSqlStatus::InvalidECSql;
        }

    NativeSqlSnippets insertNativeSqlSnippets;
    ECSqlStatus stat = GenerateNativeSqlSnippets(insertNativeSqlSnippets, ctx, exp, classMap);
    if (!stat.IsSuccess())
        return stat;

    if (classMap.IsRelationshipClassMap())
        stat = PrepareInsertIntoRelationship(ctx, insertNativeSqlSnippets, exp, classMap);
    else
        stat = PrepareInsertIntoClass(ctx, insertNativeSqlSnippets, classMap, exp);

    ctx.PopScope();
    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::PrepareInsertIntoClass(ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, ClassMap const& classMap, InsertStatementExp const& exp)
    {
    PrepareClassId(ctx, nativeSqlSnippets, classMap);
    BuildNativeSqlInsertStatement(ctx.GetSqlBuilderR(), nativeSqlSnippets, exp);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::PrepareInsertIntoRelationship(ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, InsertStatementExp const& exp, ClassMap const& classMap)
    {
    BeAssert(classMap.IsRelationshipClassMap());
    if (classMap.GetType() == ClassMap::Type::RelationshipLinkTable)
        return PrepareInsertIntoLinkTableRelationship(ctx, nativeSqlSnippets, exp, classMap.GetAs<RelationshipClassLinkTableMap>());

    return PrepareInsertIntoEndTableRelationship(ctx, nativeSqlSnippets, exp, classMap.GetAs<RelationshipClassEndTableMap>());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::PrepareInsertIntoLinkTableRelationship(ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, InsertStatementExp const& exp, RelationshipClassLinkTableMap const& relationshipClassMap)
    {
    PrepareClassId(ctx, nativeSqlSnippets, relationshipClassMap);
    BuildNativeSqlInsertStatement(ctx.GetSqlBuilderR(), nativeSqlSnippets, exp);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::PrepareInsertIntoEndTableRelationship(ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, InsertStatementExp const& exp, RelationshipClassEndTableMap const& relationshipClassMap)
    {
    const ECRelationshipEnd foreignEnd = relationshipClassMap.GetForeignEnd();

    SystemPropertyExpIndexMap const& specialTokenExpIndexMap = exp.GetPropertyNameListExp()->GetSpecialTokenExpIndexMap();

    std::vector<size_t> expIndexSkipList;
    //if ECInstanceId was specified, put it in skip list as it will always be ignored for end table mappings
    int ecinstanceIdExpIndex = specialTokenExpIndexMap.GetIndex(ECSqlSystemPropertyInfo::ECInstanceId());
    if (ecinstanceIdExpIndex >= 0)
        expIndexSkipList.push_back((size_t) ecinstanceIdExpIndex);

    //This end's ecinstanceid is ecinstanceid of relationship instance (by nature of end table mapping)
    int foreignEndECInstanceIdIndex = specialTokenExpIndexMap.GetIndex(foreignEnd == ECRelationshipEnd_Source ? ECSqlSystemPropertyInfo::SourceECInstanceId() : ECSqlSystemPropertyInfo::TargetECInstanceId());
    int foreignEndECClassIdIndex = specialTokenExpIndexMap.GetIndex(foreignEnd == ECRelationshipEnd_Source ? ECSqlSystemPropertyInfo::SourceECClassId() : ECSqlSystemPropertyInfo::TargetECClassId());
    int referencedEndECClassIdIndex = specialTokenExpIndexMap.GetIndex(foreignEnd == ECRelationshipEnd_Target ? ECSqlSystemPropertyInfo::SourceECClassId() : ECSqlSystemPropertyInfo::TargetECClassId());
    
    if (foreignEndECInstanceIdIndex >= 0)
        {
        //ECSQL contains Source/TargetECInstanceId for foreign end
        const size_t foreignEndECInstanceIdIndexUnsigned = (size_t) foreignEndECInstanceIdIndex;
        NativeSqlBuilder::List const& ecinstanceIdPropNameSnippets = nativeSqlSnippets.m_propertyNamesNativeSqlSnippets[foreignEndECInstanceIdIndexUnsigned];
        nativeSqlSnippets.m_pkColumnNamesNativeSqlSnippets.insert(nativeSqlSnippets.m_pkColumnNamesNativeSqlSnippets.end(), ecinstanceIdPropNameSnippets.begin(), ecinstanceIdPropNameSnippets.end());

        NativeSqlBuilder::List const& ecinstanceIdValueSnippets = nativeSqlSnippets.m_valuesNativeSqlSnippets[foreignEndECInstanceIdIndexUnsigned];
        if (ecinstanceIdValueSnippets.size() != 1)
            {
            BeAssert(!ecinstanceIdValueSnippets.empty() && "Should have been caught before");

            //WIP Shouldn't this be caught much earlier??
            if (ecinstanceIdValueSnippets.size() > 1)
                ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report("Multi-value ECInstanceIds not supported.");

            return ECSqlStatus::InvalidECSql;
            }

        nativeSqlSnippets.m_pkValuesNativeSqlSnippets.insert(nativeSqlSnippets.m_pkValuesNativeSqlSnippets.end(), ecinstanceIdValueSnippets.begin(), ecinstanceIdValueSnippets.end());
        expIndexSkipList.push_back(foreignEndECInstanceIdIndexUnsigned);
        }

    //if SourceECClassId or TargetECClassId was specified, put it in skip list as it will be treated separately
    if (foreignEndECClassIdIndex >= 0)
        expIndexSkipList.push_back((size_t) foreignEndECClassIdIndex);

    if (referencedEndECClassIdIndex >= 0)
        expIndexSkipList.push_back((size_t) referencedEndECClassIdIndex);

    std::sort(expIndexSkipList.begin(), expIndexSkipList.end());
    //now build SQLite SQL
    //Inserting into a relationship with end table mapping translates to an UPDATE statement in SQLite
    BuildNativeSqlUpdateStatement(ctx.GetSqlBuilderR(), nativeSqlSnippets, expIndexSkipList, relationshipClassMap);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::GenerateNativeSqlSnippets(NativeSqlSnippets& insertSqlSnippets, ECSqlPrepareContext& ctx, InsertStatementExp const& exp, ClassMap const& classMap)
    {
    ECSqlStatus status = ECSqlExpPreparer::PrepareClassRefExp(insertSqlSnippets.m_classNameNativeSqlSnippet, ctx, *exp.GetClassNameExp());
    if (!status.IsSuccess())
        return status;

    PropertyNameListExp const* propNameListExp = exp.GetPropertyNameListExp();
    size_t index = 0;
    for (Exp const* childExp : propNameListExp->GetChildren())
        {
        PropertyNameExp const& propNameExp = childExp->GetAs<PropertyNameExp>();
        BeAssert(!propNameExp.IsPropertyRef() && "PropertyRefs are not supported in ECSQL INSERT");
        NativeSqlBuilder::List nativeSqlSnippets;
        ECSqlStatus stat = ECSqlPropertyNameExpPreparer::Prepare(nativeSqlSnippets, ctx, propNameExp);
        if (!stat.IsSuccess())
            return stat;

        insertSqlSnippets.m_propertyNamesNativeSqlSnippets.push_back(move(nativeSqlSnippets));
        index++;
        }

    status = ECSqlExpPreparer::PrepareValueExpListExp(insertSqlSnippets.m_valuesNativeSqlSnippets, ctx, *exp.GetValuesExp(), insertSqlSnippets.m_propertyNamesNativeSqlSnippets);
    if (!status.IsSuccess())
        return status;

    if (insertSqlSnippets.m_propertyNamesNativeSqlSnippets.size() != insertSqlSnippets.m_valuesNativeSqlSnippets.size())
        {
        BeAssert(false && "Error preparing insert statement. Number of property name items does not match number of value items. This should have been caught by parser already.");
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlInsertPreparer::PrepareClassId(ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, ClassMap const& classMap)
    {
    SystemPropertyMap::PerTableIdPropertyMap const* classIdPropMap = classMap.GetECClassIdPropertyMap()->FindDataPropertyMap(classMap.GetPrimaryTable());
    if (classIdPropMap == nullptr || classIdPropMap->GetColumn().GetPersistenceType() == PersistenceType::Virtual)
        return;

    nativeSqlSnippets.m_propertyNamesNativeSqlSnippets.push_back(NativeSqlBuilder::List {NativeSqlBuilder(classIdPropMap->GetColumn().GetName().c_str())});


    NativeSqlBuilder::List classIdSqliteSnippets {NativeSqlBuilder()};
    classIdSqliteSnippets[0].Append(classMap.GetClass().GetId());
    nativeSqlSnippets.m_valuesNativeSqlSnippets.push_back(classIdSqliteSnippets);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlInsertPreparer::BuildNativeSqlInsertStatement(NativeSqlBuilder& insertBuilder, NativeSqlSnippets const& snippets, InsertStatementExp const& exp)
    {
    //For each expression in the property name / value list, a NativeSqlBuilder::List is created. For simple primitive
    //properties, the list will only contain one snippet, but for multi-dimensional properties (points, structs)
    //the list will contain more than one snippet. Consequently the the list of ECSQL expressions is translated
    //into a list of list of native sql snippets. At this point we don't need that jaggedness anymore and flatten it out
    //before building the final SQLite sql string.
    const std::vector<size_t> emptyIndexSkipList;
    NativeSqlBuilder::List propertyNamesNativeSqlSnippets = NativeSqlBuilder::FlattenJaggedList(snippets.m_propertyNamesNativeSqlSnippets, emptyIndexSkipList);
    NativeSqlBuilder::List valuesNativeSqlSnippets = NativeSqlBuilder::FlattenJaggedList(snippets.m_valuesNativeSqlSnippets, emptyIndexSkipList);

    insertBuilder.Append("INSERT INTO ").Append(snippets.m_classNameNativeSqlSnippet);

    insertBuilder.AppendSpace().AppendParenLeft().Append(propertyNamesNativeSqlSnippets);

    if (!snippets.m_pkColumnNamesNativeSqlSnippets.empty())
        {
        if (!propertyNamesNativeSqlSnippets.empty())
            insertBuilder.AppendComma();

        insertBuilder.Append(snippets.m_pkColumnNamesNativeSqlSnippets);
        }

    insertBuilder.AppendParenRight().Append(" VALUES ").AppendParenLeft().Append(valuesNativeSqlSnippets);

    if (!snippets.m_pkValuesNativeSqlSnippets.empty())
        {
        if (!valuesNativeSqlSnippets.empty())
            insertBuilder.AppendComma();

        insertBuilder.Append(snippets.m_pkValuesNativeSqlSnippets);
        }

    insertBuilder.AppendParenRight();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlInsertPreparer::BuildNativeSqlUpdateStatement(NativeSqlBuilder& updateBuilder, NativeSqlSnippets const& insertSqlSnippets, std::vector<size_t> const& expIndexSkipList, RelationshipClassEndTableMap const& classMap)
    {
    ECClassIdPropertyMap const* ecClassIdPropertyMap = classMap.GetECClassIdPropertyMap();
    if (!ecClassIdPropertyMap->IsMappedToSingleTable() || !classMap.IsMappedToSingleTable())
        {
        BeAssert(false && "We should not be able to insert into endtable that mapped top multiple tables");
        return;
        }

    DbTable const& contextTable = classMap.GetPrimaryTable();

    //For each expression in the property name / value list, a NativeSqlBuilder::List is created. For simple primitive
    //properties, the list will only contain one snippet, but for multi-dimensional properties (points, structs)
    //the list will contain more than one snippet. Consequently the list of ECSQL expressions is translated
    //into a list of list of native sql snippets. At this point we don't need that jaggedness anymore and flatten it out
    //before building the final SQLite sql string.
    NativeSqlBuilder::List propertyNamesNativeSqlSnippets = NativeSqlBuilder::FlattenJaggedList(insertSqlSnippets.m_propertyNamesNativeSqlSnippets, expIndexSkipList);
    NativeSqlBuilder::List valuesNativeSqlSnippets = NativeSqlBuilder::FlattenJaggedList(insertSqlSnippets.m_valuesNativeSqlSnippets, expIndexSkipList);

    updateBuilder.Append("UPDATE ").Append(insertSqlSnippets.m_classNameNativeSqlSnippet).Append(" SET ");
    updateBuilder.Append(propertyNamesNativeSqlSnippets, "=", valuesNativeSqlSnippets);

    SystemPropertyMap::PerTableIdPropertyMap const* perTableClassIdPropMap = ecClassIdPropertyMap->FindDataPropertyMap(contextTable);
    BeAssert(perTableClassIdPropMap != nullptr && perTableClassIdPropMap->GetType() == PropertyMap::Type::SystemPerTableClassId);
    DbColumn const& classIdCol = perTableClassIdPropMap->GetColumn();
    if (classIdCol.GetPersistenceType() == PersistenceType::Physical)
        {
        //class id is persisted so append the class id literal to the SQL
        updateBuilder.AppendComma().Append(classIdCol.GetName().c_str()).Append(ExpHelper::ToSql(BooleanSqlOperator::EqualTo)).Append(classMap.GetClass().GetId());
        }

    //add WHERE clause so that the right row in the end table is updated
    updateBuilder.Append(" WHERE ").Append(insertSqlSnippets.m_pkColumnNamesNativeSqlSnippets, "=", insertSqlSnippets.m_pkValuesNativeSqlSnippets, " AND ");
    //add expression to WHERE clause that only updates the row if the other end id is NULL. If it wasn't NULL, it would mean
    //a cardinality constraint violation, as by definition the other end's cardinality in an end table mapping is 0 or 1.
    ToSqlPropertyMapVisitor sqlVisitor(contextTable, ToSqlPropertyMapVisitor::ECSqlScope::NonSelectNoAssignmentExp, nullptr);
    classMap.GetReferencedEndECInstanceIdPropMap()->AcceptVisitor(sqlVisitor);
    for (ToSqlPropertyMapVisitor::Result const& referencedEndECInstanceIdColSnippet : sqlVisitor.GetResultSet())
        {
        updateBuilder.Append(" AND ").Append(referencedEndECInstanceIdColSnippet.GetSql()).Append(" IS NULL");
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

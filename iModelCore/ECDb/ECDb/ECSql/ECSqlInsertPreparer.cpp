/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <Bentley/BeStringUtilities.h>
#include "ECSqlInsertPreparer.h"
#include "ECSqlPropertyNameExpPreparer.h"
#include "IdECSqlBinder.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlStatus ECSqlInsertPreparer::Prepare(ECSqlPrepareContext& ctx, InsertStatementExp const& exp)
    {
    BeAssert(exp.IsComplete());
    ctx.PushScope(exp);

    ClassMap const& classMap = exp.GetClassNameExp()->GetInfo().GetMap();
    if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
        {
        BeAssert(false && "Should have been caught before");
        return ECSqlStatus::InvalidECSql;
        }

    DbTable const& table = ctx.GetPreparedStatement<SingleContextTableECSqlPreparedStatement>().GetContextTable();
    if (table.GetType() == DbTable::Type::Virtual)
        {
        BeAssert(false && "Should have been caught before");
        return ECSqlStatus::InvalidECSql;
        }

    NativeSqlSnippets insertNativeSqlSnippets;
    ECSqlStatus stat = GenerateNativeSqlSnippets(insertNativeSqlSnippets, ctx, exp, classMap);
    if (!stat.IsSuccess())
        return stat;

    PrepareClassId(ctx, insertNativeSqlSnippets, classMap);
    BuildNativeSqlInsertStatement(ctx.GetSqlBuilder(), insertNativeSqlSnippets, exp);

    ctx.PopScope();
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::GenerateNativeSqlSnippets(NativeSqlSnippets& insertSqlSnippets, ECSqlPrepareContext& ctx, InsertStatementExp const& exp, ClassMap const& classMap)
    {
    ECSqlStatus status = ECSqlExpPreparer::PrepareClassRefExp(insertSqlSnippets.m_classNameNativeSqlSnippet, ctx, *exp.GetClassNameExp());
    if (!status.IsSuccess())
        return status;

    PropertyNameListExp const* propNameListExp = exp.GetPropertyNameListExp();
    for (Exp const* childExp : propNameListExp->GetChildren())
        {
        PropertyNameExp const& propNameExp = childExp->GetAs<PropertyNameExp>();
        BeAssert(!propNameExp.IsPropertyRef() && "PropertyRefs are not supported in ECSQL INSERT");
        NativeSqlBuilder::List nativeSqlSnippets;
        ECSqlStatus stat = ECSqlPropertyNameExpPreparer::Prepare(nativeSqlSnippets, ctx, propNameExp);
        if (!stat.IsSuccess())
            return stat;

        insertSqlSnippets.m_propertyNamesNativeSqlSnippets.push_back(move(nativeSqlSnippets));
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
// @bsimethod
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
// @bsimethod
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

END_BENTLEY_SQLITE_EC_NAMESPACE

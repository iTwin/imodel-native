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

        insertSqlSnippets.m_propertyNamesNativeSqlSnippets.push_back(std::move(nativeSqlSnippets));
        }

    status = ECSqlExpPreparer::PrepareValueExpListExp(insertSqlSnippets.m_valuesNativeSqlSnippets, ctx, *exp.GetValuesExp(), insertSqlSnippets.m_propertyNamesNativeSqlSnippets);
    if (!status.IsSuccess())
        return status;

    if (insertSqlSnippets.m_propertyNamesNativeSqlSnippets.size() != insertSqlSnippets.m_valuesNativeSqlSnippets.size())
        {
        BeAssert(false && "Error preparing insert statement. Number of property name items does not match number of value items. This should have been caught by parser already.");
        return ECSqlStatus::Error;
        }

    if (exp.HasOnConflict())
        {
        status = GenerateOnConflictClause(insertSqlSnippets.m_onConflictClause, ctx, *exp.GetOnConflictExp(), exp, classMap);
        if (!status.IsSuccess())
            return status;
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

    if (!snippets.m_onConflictClause.IsEmpty())
        insertBuilder.Append(snippets.m_onConflictClause);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::GenerateOnConflictClause(NativeSqlBuilder& out, ECSqlPrepareContext& ctx,
    OnConflictExp const& onConflict, InsertStatementExp const& /*insertExp*/, ClassMap const& classMap)
    {
    DbTable const& contextTable = ctx.GetPreparedStatement<SingleContextTableECSqlPreparedStatement>().GetContextTable();

    out.Append(" ON CONFLICT");

    // --- Conflict target columns ---
    if (onConflict.HasConflictTarget())
        {
        PropertyNameListExp const* targetList = onConflict.GetConflictTargetExp();
        NativeSqlBuilder::List conflictCols;
        for (Exp const* child : targetList->GetChildren())
            {
            PropertyNameExp const& propExp = child->GetAs<PropertyNameExp>();
            NativeSqlBuilder::List colSnippets;
            ECSqlStatus stat = ECSqlPropertyNameExpPreparer::Prepare(colSnippets, ctx, propExp);
            if (!stat.IsSuccess())
                return stat;
            for (NativeSqlBuilder const& s : colSnippets)
                conflictCols.push_back(s);
            }
        out.AppendParenLeft().Append(conflictCols).AppendParenRight();
        }

    // --- DO NOTHING ---
    if (onConflict.GetAction() == OnConflictExp::Action::DoNothing)
        {
        out.Append(" DO NOTHING");
        return ECSqlStatus::Success;
        }

    // --- DO UPDATE SET ---
    out.Append(" DO UPDATE SET ");

    AssignmentListExp const* assignList = onConflict.GetAssignmentListExp();
    BeAssert(assignList != nullptr);

    ctx.PushScope(*assignList);

    bool firstAssignment = true;
    for (Exp const* child : assignList->GetChildren())
        {
        AssignmentExp const& assignExp = child->GetAs<AssignmentExp>();
        PropertyNameExp const& lhsExp = *assignExp.GetPropertyNameExp();
        ValueExp const* rhsExp = assignExp.GetValueExp();

        // LHS: resolve to column name(s)
        NativeSqlBuilder::List lhsCols;
        ECSqlStatus stat = ECSqlPropertyNameExpPreparer::Prepare(lhsCols, ctx, lhsExp);
        if (!stat.IsSuccess())
            {
            ctx.PopScope();
            return stat;
            }

        if (lhsCols.empty())
            continue; // virtual column — skip

        // RHS: EXCLUDED pseudo-ref or normal expression
        NativeSqlBuilder::List rhsCols;
        if (rhsExp->GetType() == Exp::Type::PropertyName)
            {
            PropertyNameExp const& rhsPropExp = rhsExp->GetAs<PropertyNameExp>();
            if (rhsPropExp.IsExcludedPseudoRef())
                {
                BeAssert(rhsPropExp.GetOriginalPropertyPath().Size() == 2); // [EXCLUDED, PropName]
                Utf8StringCR propName = rhsPropExp.GetOriginalPropertyPath()[1].GetName();
                PropertyMap const* propMap = classMap.GetPropertyMaps().Find(propName.c_str());
                if (propMap == nullptr)
                    {
                    ctx.PopScope();
                    ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties,
                        IssueType::ECSQL, ECDbIssueId::ECDb_0742,
                        "ECSQL ON CONFLICT DO UPDATE SET: EXCLUDED.%s refers to a property not found on class %s.",
                        propName.c_str(), classMap.GetClass().GetFullName());
                    return ECSqlStatus::InvalidECSql;
                    }

                GetColumnsPropertyMapVisitor columnVisitor(contextTable, PropertyMap::Type::Data);
                propMap->AcceptVisitor(columnVisitor);
                for (DbColumn const* col : columnVisitor.GetColumns())
                    {
                    if (col->GetPersistenceType() != PersistenceType::Virtual)
                        {
                        NativeSqlBuilder b;
                        b.AppendFullyQualified("excluded", col->GetName());
                        rhsCols.push_back(b);
                        }
                    }
                if (rhsCols.empty())
                    {
                    ctx.PopScope();
                    return ECSqlStatus::Error;
                    }
                }
            else
                {
                stat = ECSqlExpPreparer::PrepareValueExp(rhsCols, ctx, *rhsExp);
                if (!stat.IsSuccess())
                    {
                    ctx.PopScope();
                    return stat;
                    }
                }
            }
        else
            {
            stat = ECSqlExpPreparer::PrepareValueExp(rhsCols, ctx, *rhsExp);
            if (!stat.IsSuccess())
                {
                ctx.PopScope();
                return stat;
                }
            }

        if (lhsCols.size() != rhsCols.size())
            {
            ctx.PopScope();
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties,
                IssueType::ECSQL, ECDbIssueId::ECDb_0743,
                "ECSQL ON CONFLICT DO UPDATE SET: column count mismatch between LHS (%zu) and RHS (%zu) for assignment.",
                lhsCols.size(), rhsCols.size());
            return ECSqlStatus::Error;
            }

        for (size_t i = 0; i < lhsCols.size(); i++)
            {
            if (!firstAssignment)
                out.AppendComma();
            out.Append(lhsCols[i]).Append(" = ").Append(rhsCols[i]);
            firstAssignment = false;
            }
        }

    ctx.PopScope();

    // Optional WHERE clause
    if (onConflict.GetWhereClauseExp() != nullptr)
        {
        out.AppendSpace();
        ECSqlStatus stat = ECSqlExpPreparer::PrepareWhereExp(out, ctx, *onConflict.GetWhereClauseExp());
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

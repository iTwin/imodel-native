/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlCodeGen.h"
#include "../ViewGenerator.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlCodeGen::Generate(Utf8StringR nativeSql, ECSqlAst::Node const& stmt)
    {
    m_sql.Clear();
    ECSqlStatus status = stmt.Accept(*this);
    if (status != ECSqlStatus::Success)
        return status;

    nativeSql = m_sql.GetSql();
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod — Helper: visit any ExprNode and append SQL to m_sql
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlCodeGen::VisitExpr(ECSqlAst::ExprNode const& expr)
    {
    if (expr.HasParentheses())
        m_sql.AppendParenLeft();

    ECSqlStatus status = expr.Accept(*this);

    if (expr.HasParentheses())
        m_sql.AppendParenRight();

    return status;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod — Helper: visit any table ref node
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlCodeGen::VisitTableRef(ECSqlAst::Node const& tableRef)
    {
    return tableRef.Accept(*this);
    }

//======================================================================================
// ── Statement Visitors ───────────────────────────────────────────────────────────────
//======================================================================================

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::SelectStmtNode const& node)
    {
    PushScope(ECSqlType::Select);

    if (node.GetCte())
        {
        ECSqlStatus status = node.GetCte()->Accept(*this);
        if (status != ECSqlStatus::Success)
            { PopScope(); return status; }
        m_sql.AppendSpace();
        }

    ECSqlStatus status = node.GetQuery().Accept(*this);
    PopScope();
    return status;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::InsertStmtNode const& node)
    {
    PushScope(ECSqlType::Insert);

    m_sql.Append("INSERT INTO ");

    // For INSERT, emit the physical table name from ClassMap
    ClassMap const& classMap = node.GetClassRef().GetClassMap();
    DbTable const& primaryTable = classMap.GetPrimaryTable();
    m_sql.AppendEscaped(primaryTable.GetName());

    // Column list
    m_sql.Append(" (");
    auto const& props = node.GetPropNames().GetProps();
    for (size_t i = 0; i < props.size(); ++i)
        {
        if (i > 0) m_sql.AppendComma();
        NativeSqlBuilder::List snippets;
        ECSqlStatus status = GeneratePropertySql(snippets, *props[i]);
        if (status != ECSqlStatus::Success)
            { PopScope(); return status; }
        m_sql.Append(snippets);
        }
    m_sql.Append(")");

    if (node.IsInsertSelect())
        {
        m_sql.AppendSpace();
        ECSqlStatus status = node.GetSelectQuery().Accept(*this);
        if (status != ECSqlStatus::Success)
            { PopScope(); return status; }
        }
    else
        {
        m_sql.Append(" VALUES (");
        auto const& values = node.GetValues();
        for (size_t i = 0; i < values.size(); ++i)
            {
            if (i > 0) m_sql.AppendComma();
            ECSqlStatus status = VisitExpr(*values[i]);
            if (status != ECSqlStatus::Success)
                { PopScope(); return status; }
            }
        m_sql.AppendParenRight();
        }

    PopScope();
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::UpdateStmtNode const& node)
    {
    PushScope(ECSqlType::Update);

    m_sql.Append("UPDATE ");
    ClassMap const& classMap = node.GetClassRef().GetClassMap();
    DbTable const& primaryTable = classMap.GetPrimaryTable();
    m_sql.AppendEscaped(primaryTable.GetName());

    m_sql.Append(" SET ");
    auto const& assignments = node.GetAssignments();
    for (size_t i = 0; i < assignments.size(); ++i)
        {
        if (i > 0) m_sql.AppendComma();
        ECSqlStatus status = assignments[i]->Accept(*this);
        if (status != ECSqlStatus::Success)
            { PopScope(); return status; }
        }

    if (node.GetWhere())
        {
        m_sql.AppendSpace();
        ECSqlStatus status = node.GetWhere()->Accept(*this);
        if (status != ECSqlStatus::Success)
            { PopScope(); return status; }
        }

    PopScope();
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::DeleteStmtNode const& node)
    {
    PushScope(ECSqlType::Delete);

    m_sql.Append("DELETE FROM ");
    ClassMap const& classMap = node.GetClassRef().GetClassMap();
    DbTable const& primaryTable = classMap.GetPrimaryTable();
    m_sql.AppendEscaped(primaryTable.GetName());

    if (node.GetWhere())
        {
        m_sql.AppendSpace();
        ECSqlStatus status = node.GetWhere()->Accept(*this);
        if (status != ECSqlStatus::Success)
            { PopScope(); return status; }
        }

    PopScope();
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::PragmaStmtNode const&)
    {
    // Pragmas are handled separately via PragmaECSqlPreparedStatement.
    return ECSqlStatus::Success;
    }

//======================================================================================
// ── Query Visitors ───────────────────────────────────────────────────────────────────
//======================================================================================

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::SingleSelectNode const& node)
    {
    m_sql.Append("SELECT ");
    if (node.GetQuantifier() == SqlSetQuantifier::Distinct)
        m_sql.Append("DISTINCT ");
    else if (node.GetQuantifier() == SqlSetQuantifier::All)
        m_sql.Append("ALL ");

    auto const& items = node.GetSelectItems();
    for (size_t i = 0; i < items.size(); ++i)
        {
        if (i > 0) m_sql.AppendComma();
        ECSqlStatus status = items[i]->Accept(*this);
        if (status != ECSqlStatus::Success)
            return status;
        }

    if (node.GetFrom())
        {
        m_sql.AppendSpace();
        ECSqlStatus status = node.GetFrom()->Accept(*this);
        if (status != ECSqlStatus::Success)
            return status;
        }

    if (node.GetWhere())
        {
        m_sql.AppendSpace();
        ECSqlStatus status = node.GetWhere()->Accept(*this);
        if (status != ECSqlStatus::Success)
            return status;
        }

    if (node.GetGroupBy())
        {
        m_sql.AppendSpace();
        ECSqlStatus status = node.GetGroupBy()->Accept(*this);
        if (status != ECSqlStatus::Success)
            return status;
        }

    if (node.GetHaving())
        {
        m_sql.AppendSpace();
        ECSqlStatus status = node.GetHaving()->Accept(*this);
        if (status != ECSqlStatus::Success)
            return status;
        }

    if (node.GetWindowDefs())
        {
        m_sql.AppendSpace();
        ECSqlStatus status = node.GetWindowDefs()->Accept(*this);
        if (status != ECSqlStatus::Success)
            return status;
        }

    if (node.GetOrderBy())
        {
        m_sql.AppendSpace();
        ECSqlStatus status = node.GetOrderBy()->Accept(*this);
        if (status != ECSqlStatus::Success)
            return status;
        }

    if (node.GetLimitOffset())
        {
        m_sql.AppendSpace();
        ECSqlStatus status = node.GetLimitOffset()->Accept(*this);
        if (status != ECSqlStatus::Success)
            return status;
        }

    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::CompoundSelectNode const& node)
    {
    ECSqlStatus status = node.GetLhs().Accept(*this);
    if (status != ECSqlStatus::Success)
        return status;

    switch (node.GetOperator())
        {
        case ECSqlAst::CompoundSelectNode::Op::Union:
            m_sql.Append(node.IsAll() ? " UNION ALL " : " UNION ");
            break;
        case ECSqlAst::CompoundSelectNode::Op::Intersect:
            m_sql.Append(" INTERSECT ");
            break;
        case ECSqlAst::CompoundSelectNode::Op::Except:
            m_sql.Append(" EXCEPT ");
            break;
        }

    return node.GetRhs().Accept(*this);
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::SelectItemNode const& node)
    {
    ECSqlStatus status = VisitExpr(node.GetExpr());
    if (status != ECSqlStatus::Success)
        return status;

    if (node.HasAlias())
        m_sql.AppendSpace().AppendEscaped(node.GetAlias());

    return ECSqlStatus::Success;
    }

//======================================================================================
// ── FROM / Table Reference Visitors ──────────────────────────────────────────────────
//======================================================================================

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::FromNode const& node)
    {
    m_sql.Append("FROM ");
    auto const& refs = node.GetTableRefs();
    for (size_t i = 0; i < refs.size(); ++i)
        {
        if (i > 0) m_sql.AppendComma();
        ECSqlStatus status = VisitTableRef(*refs[i]);
        if (status != ECSqlStatus::Success)
            return status;
        }
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod — Core: generate SQL for a ClassRefNode in FROM clause.
// For simple cases, emits direct table reference; for complex cases delegates to ViewGenerator.
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlCodeGen::GenerateClassRefSql(NativeSqlBuilder& sql, ECSqlAst::ClassRefNode const& classRef)
    {
    ClassMap const& classMap = classRef.GetClassMap();
    PolymorphicInfo const& polymorphic = classRef.GetPolymorphicInfo();

    // Simple case: non-polymorphic OwnTable with single primary table
    // → direct table reference without ViewGenerator
    bool isSimpleCase = polymorphic.IsOnly()
        && classMap.GetMapStrategy().GetStrategy() != MapStrategy::NotMapped
        && classMap.GetClass().GetClassModifier() != ECN::ECClassModifier::Abstract;

    if (isSimpleCase)
        {
        DbTable const& primaryTable = classMap.GetPrimaryTable();
        sql.AppendEscaped(primaryTable.GetName());
        sql.AppendSpace().AppendEscaped(classRef.GetAlias());
        return ECSqlStatus::Success;
        }

    // Complex case: polymorphic, TPH, mixin, etc. → use ViewGenerator
    NativeSqlBuilder viewSql;
    if (SUCCESS != ViewGenerator::GenerateSelectFromViewSql(viewSql, m_ctx, classMap, polymorphic,
                    classRef.DisqualifyPrimaryJoin()))
        return ECSqlStatus::InvalidECSql;

    sql.Append(viewSql);
    sql.AppendSpace().AppendEscaped(classRef.GetAlias());
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::ClassRefNode const& node)
    {
    return GenerateClassRefSql(m_sql, node);
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::SubqueryRefNode const& node)
    {
    m_sql.AppendParenLeft();
    ECSqlStatus status = node.GetQuery().Accept(*this);
    if (status != ECSqlStatus::Success)
        return status;
    m_sql.AppendParenRight();
    if (!node.GetAlias().empty())
        m_sql.AppendSpace().AppendEscaped(node.GetAlias());
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::TableValuedFuncNode const& node)
    {
    if (!node.GetSchemaName().empty())
        m_sql.AppendEscaped(node.GetSchemaName()).AppendDot();
    m_sql.Append(node.GetFunctionName()).AppendParenLeft();
    auto const& args = node.GetArgs();
    for (size_t i = 0; i < args.size(); ++i)
        {
        if (i > 0) m_sql.AppendComma();
        ECSqlStatus status = VisitExpr(*args[i]);
        if (status != ECSqlStatus::Success)
            return status;
        }
    m_sql.AppendParenRight();
    if (!node.GetAlias().empty())
        m_sql.AppendSpace().AppendEscaped(node.GetAlias());
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::CteNode const& node)
    {
    m_sql.Append("WITH ");
    if (node.IsRecursive())
        m_sql.Append("RECURSIVE ");

    auto const& blocks = node.GetBlocks();
    for (size_t i = 0; i < blocks.size(); ++i)
        {
        if (i > 0) m_sql.AppendComma();
        ECSqlStatus status = blocks[i]->Accept(*this);
        if (status != ECSqlStatus::Success)
            return status;
        }
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::CteBlockNode const& node)
    {
    m_sql.AppendEscaped(node.GetName());
    auto const& cols = node.GetColumnNames();
    if (!cols.empty())
        {
        m_sql.AppendParenLeft();
        for (size_t i = 0; i < cols.size(); ++i)
            {
            if (i > 0) m_sql.AppendComma();
            m_sql.AppendEscaped(cols[i]);
            }
        m_sql.AppendParenRight();
        }
    m_sql.Append(" AS ").AppendParenLeft();
    ECSqlStatus status = node.GetQuery().Accept(*this);
    m_sql.AppendParenRight();
    return status;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::CteBlockRefNode const& node)
    {
    m_sql.AppendEscaped(node.GetCteName());
    if (!node.GetAlias().empty() && !node.GetAlias().Equals(node.GetCteName()))
        m_sql.AppendSpace().AppendEscaped(node.GetAlias());
    return ECSqlStatus::Success;
    }

//======================================================================================
// ── JOIN Visitors ────────────────────────────────────────────────────────────────────
//======================================================================================

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::JoinNode const& node)
    {
    ECSqlStatus status = VisitTableRef(node.GetLhs());
    if (status != ECSqlStatus::Success)
        return status;

    switch (node.GetJoinType())
        {
        case ECSqlJoinType::InnerJoin:      m_sql.Append(" INNER JOIN "); break;
        case ECSqlJoinType::LeftOuterJoin:   m_sql.Append(" LEFT JOIN "); break;
        case ECSqlJoinType::RightOuterJoin:  m_sql.Append(" RIGHT JOIN "); break;
        case ECSqlJoinType::FullOuterJoin:   m_sql.Append(" FULL JOIN "); break;
        case ECSqlJoinType::CrossJoin:       m_sql.Append(" CROSS JOIN "); break;
        case ECSqlJoinType::NaturalJoin:     m_sql.Append(" NATURAL JOIN "); break;
        default:                             m_sql.Append(" JOIN "); break;
        }

    status = VisitTableRef(node.GetRhs());
    if (status != ECSqlStatus::Success)
        return status;

    if (node.GetCondition())
        {
        m_sql.Append(" ON ");
        status = VisitExpr(*node.GetCondition());
        }
    return status;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::RelJoinNode const&)
    {
    // TODO: Implement ECSQL relationship join expansion
    // This requires resolving the relationship endpoints and generating
    // appropriate SQL JOINs based on the relationship mapping strategy.
    return ECSqlStatus::InvalidECSql;
    }

//======================================================================================
// ── Clause Visitors ──────────────────────────────────────────────────────────────────
//======================================================================================

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::WhereNode const& node)
    {
    m_sql.Append("WHERE ");
    return VisitExpr(node.GetCondition());
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::GroupByNode const& node)
    {
    m_sql.Append("GROUP BY ");
    auto const& exprs = node.GetExprs();
    for (size_t i = 0; i < exprs.size(); ++i)
        {
        if (i > 0) m_sql.AppendComma();
        ECSqlStatus status = VisitExpr(*exprs[i]);
        if (status != ECSqlStatus::Success)
            return status;
        }
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::HavingNode const& node)
    {
    m_sql.Append("HAVING ");
    return VisitExpr(node.GetCondition());
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::OrderByNode const& node)
    {
    m_sql.Append("ORDER BY ");
    auto const& items = node.GetItems();
    for (size_t i = 0; i < items.size(); ++i)
        {
        if (i > 0) m_sql.AppendComma();
        ECSqlStatus status = items[i]->Accept(*this);
        if (status != ECSqlStatus::Success)
            return status;
        }
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::OrderByItemNode const& node)
    {
    ECSqlStatus status = VisitExpr(node.GetExpr());
    if (status != ECSqlStatus::Success)
        return status;

    if (node.GetDirection() == ECSqlAst::OrderByItemNode::SortDirection::Ascending)
        m_sql.Append(" ASC");
    else if (node.GetDirection() == ECSqlAst::OrderByItemNode::SortDirection::Descending)
        m_sql.Append(" DESC");

    if (node.GetNullsOrder() == ECSqlAst::OrderByItemNode::NullsOrder::First)
        m_sql.Append(" NULLS FIRST");
    else if (node.GetNullsOrder() == ECSqlAst::OrderByItemNode::NullsOrder::Last)
        m_sql.Append(" NULLS LAST");

    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::LimitOffsetNode const& node)
    {
    m_sql.Append("LIMIT ");
    ECSqlStatus status = VisitExpr(node.GetLimit());
    if (status != ECSqlStatus::Success)
        return status;

    if (node.HasOffset())
        {
        m_sql.Append(" OFFSET ");
        status = VisitExpr(*node.GetOffset());
        }
    return status;
    }

//======================================================================================
// ── Expression Visitors ──────────────────────────────────────────────────────────────
//======================================================================================

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::LiteralNode const& node)
    {
    m_sql.Append(node.GetRawValue());
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::ParamNode const&)
    {
    m_sql.Append("?");
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod — Core: property reference → SQL column(s)
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlCodeGen::GeneratePropertySql(NativeSqlBuilder::List& snippets, ECSqlAst::PropRefNode const& propRef)
    {
    PropertyMap const* propMap = propRef.GetPropertyMap();
    if (propMap == nullptr)
        {
        // CTE or subquery column — use the property name directly
        NativeSqlBuilder snippet;
        if (propRef.GetClassRef())
            snippet.AppendFullyQualified(propRef.GetClassRef()->GetAlias(), propRef.GetResolvedPath().ToString());
        else
            snippet.Append(propRef.GetResolvedPath().ToString());
        snippets.push_back(std::move(snippet));
        return ECSqlStatus::Success;
        }

    if (propRef.IsSystemProperty())
        return GenerateSystemPropertySql(snippets, propRef);

    // Use PropertyMap to generate column reference(s)
    // For SingleColumnDataPropertyMap: emit [alias].[columnName]
    // For CompoundDataPropertyMap (struct): fan-out to leaf columns
    // For NavigationPropertyMap: emit both Id and RelECClassId columns

    // TODO: Full implementation for all PropertyMap types
    NativeSqlBuilder snippet;
    if (propRef.GetClassRef())
        snippet.AppendFullyQualified(propRef.GetClassRef()->GetAlias(),
                                     propRef.GetResolvedPath().Last().GetName());
    else
        snippet.AppendEscaped(propRef.GetResolvedPath().Last().GetName());
    snippets.push_back(std::move(snippet));
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::GenerateSystemPropertySql(NativeSqlBuilder::List& snippets, ECSqlAst::PropRefNode const& propRef)
    {
    // System properties map to well-known column names
    // TODO: Full implementation matching all system property types
    NativeSqlBuilder snippet;
    Utf8CP propName = propRef.GetResolvedPath().First().GetName().c_str();
    if (propRef.GetClassRef())
        snippet.AppendFullyQualified(propRef.GetClassRef()->GetAlias(), propName);
    else
        snippet.AppendEscaped(propName);
    snippets.push_back(std::move(snippet));
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::PropRefNode const& node)
    {
    NativeSqlBuilder::List snippets;
    ECSqlStatus status = GeneratePropertySql(snippets, node);
    if (status != ECSqlStatus::Success)
        return status;

    m_sql.Append(snippets);
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::EnumValueNode const& node)
    {
    // Enum values are resolved to their integer values at parse time
    auto const& enumerator = node.GetEnumerator();
    if (enumerator.IsInteger())
        m_sql.AppendFormatted("%" PRId32, enumerator.GetInteger());
    else
        {
        // String enumerators — quote the value
        m_sql.Append("'").Append(enumerator.GetString()).Append("'");
        }
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::BinaryOpNode const& node)
    {
    ECSqlStatus status = VisitExpr(node.GetLhs());
    if (status != ECSqlStatus::Success)
        return status;

    switch (node.GetOperator())
        {
        case BinarySqlOperator::Plus:       m_sql.Append("+"); break;
        case BinarySqlOperator::Minus:      m_sql.Append("-"); break;
        case BinarySqlOperator::Multiply:   m_sql.Append("*"); break;
        case BinarySqlOperator::Divide:     m_sql.Append("/"); break;
        case BinarySqlOperator::Modulo:     m_sql.Append("%"); break;
        case BinarySqlOperator::ShiftLeft:  m_sql.Append("<<"); break;
        case BinarySqlOperator::ShiftRight: m_sql.Append(">>"); break;
        case BinarySqlOperator::BitwiseOr:  m_sql.Append("|"); break;
        case BinarySqlOperator::BitwiseAnd: m_sql.Append("&"); break;
        case BinarySqlOperator::BitwiseXOr: m_sql.Append("^"); break;
        case BinarySqlOperator::Concat:     m_sql.Append("||"); break;
        }

    return VisitExpr(node.GetRhs());
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::UnaryOpNode const& node)
    {
    switch (node.GetOperator())
        {
        case ECSqlAst::UnaryOpNode::Op::Minus:      m_sql.Append("-"); break;
        case ECSqlAst::UnaryOpNode::Op::Plus:        m_sql.Append("+"); break;
        case ECSqlAst::UnaryOpNode::Op::BitwiseNot:  m_sql.Append("~"); break;
        case ECSqlAst::UnaryOpNode::Op::Not:         m_sql.Append("NOT "); break;
        }
    return VisitExpr(node.GetOperand());
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::CompareNode const& node)
    {
    ECSqlStatus status = VisitExpr(node.GetLhs());
    if (status != ECSqlStatus::Success)
        return status;

    switch (node.GetOperator())
        {
        case BooleanSqlOperator::EqualTo:              m_sql.Append("="); break;
        case BooleanSqlOperator::NotEqualTo:           m_sql.Append("<>"); break;
        case BooleanSqlOperator::LessThan:             m_sql.Append("<"); break;
        case BooleanSqlOperator::LessThanOrEqualTo:    m_sql.Append("<="); break;
        case BooleanSqlOperator::GreaterThan:          m_sql.Append(">"); break;
        case BooleanSqlOperator::GreaterThanOrEqualTo: m_sql.Append(">="); break;
        case BooleanSqlOperator::Is:                   m_sql.Append(" IS "); break;
        case BooleanSqlOperator::IsNot:                m_sql.Append(" IS NOT "); break;
        case BooleanSqlOperator::Match:                m_sql.Append(" MATCH "); break;
        case BooleanSqlOperator::NotMatch:             m_sql.Append(" NOT MATCH "); break;
        default: m_sql.Append("="); break;
        }

    if (node.GetRhs())
        return VisitExpr(*node.GetRhs());

    m_sql.Append("NULL");
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::LogicalNode const& node)
    {
    ECSqlStatus status = VisitExpr(node.GetLhs());
    if (status != ECSqlStatus::Success)
        return status;

    m_sql.Append(node.GetOperator() == ECSqlAst::LogicalNode::Op::And ? " AND " : " OR ");

    return VisitExpr(node.GetRhs());
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::BetweenNode const& node)
    {
    ECSqlStatus status = VisitExpr(node.GetOperand());
    if (status != ECSqlStatus::Success)
        return status;

    m_sql.Append(node.IsNot() ? " NOT BETWEEN " : " BETWEEN ");
    status = VisitExpr(node.GetLower());
    if (status != ECSqlStatus::Success)
        return status;

    m_sql.Append(" AND ");
    return VisitExpr(node.GetUpper());
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::LikeNode const& node)
    {
    ECSqlStatus status = VisitExpr(node.GetOperand());
    if (status != ECSqlStatus::Success)
        return status;

    m_sql.Append(node.IsNot() ? " NOT LIKE " : " LIKE ");
    status = VisitExpr(node.GetPattern());
    if (status != ECSqlStatus::Success)
        return status;

    if (node.GetEscape())
        {
        m_sql.Append(" ESCAPE ");
        status = VisitExpr(*node.GetEscape());
        }
    return status;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::FuncCallNode const& node)
    {
    m_sql.Append(node.GetFunctionName());
    if (!node.IsGetter())
        {
        m_sql.AppendParenLeft();
        if (node.GetSetQuantifier() == SqlSetQuantifier::Distinct)
            m_sql.Append("DISTINCT ");

        auto const& args = node.GetArgs();
        for (size_t i = 0; i < args.size(); ++i)
            {
            if (i > 0) m_sql.AppendComma();
            ECSqlStatus status = VisitExpr(*args[i]);
            if (status != ECSqlStatus::Success)
                return status;
            }
        m_sql.AppendParenRight();
        }
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::CastNode const& node)
    {
    m_sql.Append("CAST(");
    ECSqlStatus status = VisitExpr(node.GetOperand());
    if (status != ECSqlStatus::Success)
        return status;

    m_sql.Append(" AS ").Append(node.GetTargetTypeName());
    m_sql.AppendParenRight();
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::CaseNode const& node)
    {
    m_sql.Append("CASE");
    for (auto const& wt : node.GetWhenList())
        {
        m_sql.Append(" WHEN ");
        ECSqlStatus status = VisitExpr(*wt.m_when);
        if (status != ECSqlStatus::Success)
            return status;
        m_sql.Append(" THEN ");
        status = VisitExpr(*wt.m_then);
        if (status != ECSqlStatus::Success)
            return status;
        }

    if (node.GetElse())
        {
        m_sql.Append(" ELSE ");
        ECSqlStatus status = VisitExpr(*node.GetElse());
        if (status != ECSqlStatus::Success)
            return status;
        }

    m_sql.Append(" END");
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::IIFNode const& node)
    {
    m_sql.Append("IIF(");
    ECSqlStatus status = VisitExpr(node.GetCondition());
    if (status != ECSqlStatus::Success)
        return status;
    m_sql.AppendComma();
    status = VisitExpr(node.GetThen());
    if (status != ECSqlStatus::Success)
        return status;
    m_sql.AppendComma();
    status = VisitExpr(node.GetElse());
    m_sql.AppendParenRight();
    return status;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::SubqueryExprNode const& node)
    {
    m_sql.AppendParenLeft();
    ECSqlStatus status = node.GetQuery().Accept(*this);
    m_sql.AppendParenRight();
    return status;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::ExistsTestNode const& node)
    {
    if (node.IsNot())
        m_sql.Append("NOT ");
    m_sql.Append("EXISTS ");
    m_sql.AppendParenLeft();
    ECSqlStatus status = node.GetQuery().Accept(*this);
    m_sql.AppendParenRight();
    return status;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::InListNode const& node)
    {
    ECSqlStatus status = VisitExpr(node.GetOperand());
    if (status != ECSqlStatus::Success)
        return status;

    m_sql.Append(node.IsNot() ? " NOT IN (" : " IN (");

    if (node.IsSubquery())
        {
        status = node.GetSubquery().Accept(*this);
        }
    else
        {
        auto const& values = node.GetValues();
        for (size_t i = 0; i < values.size(); ++i)
            {
            if (i > 0) m_sql.AppendComma();
            status = VisitExpr(*values[i]);
            if (status != ECSqlStatus::Success)
                return status;
            }
        }

    m_sql.AppendParenRight();
    return status;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::AllAnyNode const& node)
    {
    ECSqlStatus status = VisitExpr(node.GetOperand());
    if (status != ECSqlStatus::Success)
        return status;

    // Emit comparison operator
    switch (node.GetCompareOp())
        {
        case BooleanSqlOperator::EqualTo: m_sql.Append("="); break;
        case BooleanSqlOperator::NotEqualTo: m_sql.Append("<>"); break;
        case BooleanSqlOperator::LessThan: m_sql.Append("<"); break;
        case BooleanSqlOperator::GreaterThan: m_sql.Append(">"); break;
        default: m_sql.Append("="); break;
        }

    switch (node.GetQuantifier())
        {
        case SqlCompareListType::All: m_sql.Append(" ALL "); break;
        case SqlCompareListType::Any: m_sql.Append(" ANY "); break;
        case SqlCompareListType::Some: m_sql.Append(" SOME "); break;
        }

    m_sql.AppendParenLeft();
    status = node.GetSubquery().Accept(*this);
    m_sql.AppendParenRight();
    return status;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::TypePredicateNode const& node)
    {
    // Generate: <operand> IN (<class id list or subquery>)
    // Operand is typically ECClassId column reference.
    ECSqlStatus status = VisitExpr(node.GetOperand());
    if (status != ECSqlStatus::Success)
        return status;

    auto const& types = node.GetTypes();
    if (types.empty())
        return ECSqlStatus::InvalidECSql;

    // Partition types into exact (ONLY) and polymorphic lists, deduplicating along the way.
    // An ONLY class is redundant if a polymorphic ancestor already covers it.
    // A duplicate ONLY for the same class is also removed.
    std::vector<ECN::ECClassId> exactIds;
    std::vector<ECN::ECClassId> polyIds;

    for (auto const& entry : types)
        {
        auto const& srcClass = entry.m_classMap.GetClass();
        bool isSrcPoly = entry.m_polymorphic.IsPolymorphic();
        bool redundant = false;

        for (auto const& other : types)
            {
            if (&other == &entry)
                continue;
            if (other.m_polymorphic.IsPolymorphic())
                {
                // src is redundant if it is a subclass of another polymorphic entry
                if (srcClass.Is(&other.m_classMap.GetClass()))
                    { redundant = true; break; }
                }
            else
                {
                // exact duplicate
                if (!isSrcPoly && srcClass.GetId() == other.m_classMap.GetClass().GetId() && &entry > &other)
                    { redundant = true; break; }
                }
            }
        if (redundant)
            continue;

        if (isSrcPoly)
            polyIds.push_back(srcClass.GetId());
        else
            exactIds.push_back(srcClass.GetId());
        }

    bool hasPoly = !polyIds.empty();
    bool hasExact = !exactIds.empty();

    if (!hasPoly && hasExact)
        {
        // Optimized: exact class ids only → simple IN list
        m_sql.Append(" IN (");
        for (size_t i = 0; i < exactIds.size(); i++)
            {
            if (i > 0)
                m_sql.AppendComma();
            m_sql.Append(exactIds[i].ToHexStr());
            }
        m_sql.AppendParenRight();
        }
    else
        {
        // General case: uses ec_cache_ClassHierarchy subquery
        m_sql.Append(" IN (SELECT [ClassId] FROM [ec_cache_ClassHierarchy] WHERE ");
        if (hasPoly && !hasExact)
            {
            m_sql.Append("BaseClassId IN (");
            for (size_t i = 0; i < polyIds.size(); i++)
                {
                if (i > 0)
                    m_sql.AppendComma();
                m_sql.Append(polyIds[i].ToHexStr());
                }
            m_sql.AppendParenRight();
            }
        else if (!hasPoly && hasExact)
            {
            m_sql.Append("ClassId IN (");
            for (size_t i = 0; i < exactIds.size(); i++)
                {
                if (i > 0)
                    m_sql.AppendComma();
                m_sql.Append(exactIds[i].ToHexStr());
                }
            m_sql.AppendParenRight();
            }
        else
            {
            // Both poly and exact
            m_sql.Append("BaseClassId IN (");
            for (size_t i = 0; i < polyIds.size(); i++)
                {
                if (i > 0)
                    m_sql.AppendComma();
                m_sql.Append(polyIds[i].ToHexStr());
                }
            m_sql.Append(") OR ClassId IN (");
            for (size_t i = 0; i < exactIds.size(); i++)
                {
                if (i > 0)
                    m_sql.AppendComma();
                m_sql.Append(exactIds[i].ToHexStr());
                }
            m_sql.AppendParenRight();
            }
        m_sql.AppendParenRight();
        }

    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::NavValueNode const&)
    {
    // TODO: Implement navigation value creation function
    return ECSqlStatus::InvalidECSql;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::ExtractPropNode const&)
    {
    // TODO: Implement $->prop extraction SQL generation
    return ECSqlStatus::InvalidECSql;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::ExtractInstanceNode const&)
    {
    // TODO: Implement $ instance extraction SQL generation
    return ECSqlStatus::InvalidECSql;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::WindowFuncNode const& node)
    {
    ECSqlStatus status = VisitExpr(node.GetFuncExpr());
    if (status != ECSqlStatus::Success)
        return status;

    if (node.GetFilterWhere())
        {
        m_sql.Append(" FILTER (WHERE ");
        status = VisitExpr(*node.GetFilterWhere());
        if (status != ECSqlStatus::Success)
            return status;
        m_sql.AppendParenRight();
        }

    m_sql.Append(" OVER ");
    return node.GetWindowSpec().Accept(*this);
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::ExprListNode const& node)
    {
    auto const& exprs = node.GetExprs();
    for (size_t i = 0; i < exprs.size(); ++i)
        {
        if (i > 0) m_sql.AppendComma();
        ECSqlStatus status = VisitExpr(*exprs[i]);
        if (status != ECSqlStatus::Success)
            return status;
        }
    return ECSqlStatus::Success;
    }

//======================================================================================
// ── DML Visitors ─────────────────────────────────────────────────────────────────────
//======================================================================================

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::AssignmentNode const& node)
    {
    NativeSqlBuilder::List snippets;
    ECSqlStatus status = GeneratePropertySql(snippets, node.GetProperty());
    if (status != ECSqlStatus::Success)
        return status;

    m_sql.Append(snippets);
    m_sql.Append("=");
    return VisitExpr(node.GetValue());
    }

//======================================================================================
// ── Options / Window Visitors ────────────────────────────────────────────────────────
//======================================================================================

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::OptionsNode const&)
    {
    // ECSQLOPTIONS are consumed during prepare, not emitted to SQL
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::WindowSpecNode const& node)
    {
    m_sql.AppendParenLeft();

    if (node.GetPartition())
        {
        m_sql.Append("PARTITION BY ");
        auto const& items = node.GetPartition()->GetItems();
        for (size_t i = 0; i < items.size(); ++i)
            {
            if (i > 0) m_sql.AppendComma();
            ECSqlStatus status = VisitExpr(*items[i].m_expr);
            if (status != ECSqlStatus::Success)
                return status;
            }
        }

    if (node.GetOrderBy())
        {
        if (node.GetPartition()) m_sql.AppendSpace();
        ECSqlStatus status = node.GetOrderBy()->Accept(*this);
        if (status != ECSqlStatus::Success)
            return status;
        }

    // TODO: Window frame clause rendering
    m_sql.AppendParenRight();
    return ECSqlStatus::Success;
    }

ECSqlStatus ECSqlCodeGen::_Visit(ECSqlAst::WindowDefListNode const& node)
    {
    m_sql.Append("WINDOW ");
    auto const& defs = node.GetDefs();
    for (size_t i = 0; i < defs.size(); ++i)
        {
        if (i > 0) m_sql.AppendComma();
        m_sql.AppendEscaped(defs[i]->GetName());
        m_sql.Append(" AS ");
        ECSqlStatus status = defs[i]->GetSpec().Accept(*this);
        if (status != ECSqlStatus::Success)
            return status;
        }
    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

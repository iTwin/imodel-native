/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlAst.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace ECSqlAst {

// ── ParamNode ────────────────────────────────────────────────────────────────────
void ParamNode::ToECSql(Utf8StringR out) const
    {
    if (m_name.empty())
        out.append("?");
    else
        out.append(m_name);
    }

// ── PropRefNode ──────────────────────────────────────────────────────────────────
void PropRefNode::ToECSql(Utf8StringR out) const
    {
    out.append(m_originalPath.ToString(true));
    }

// ── EnumValueNode ────────────────────────────────────────────────────────────────
void EnumValueNode::ToECSql(Utf8StringR out) const
    {
    out.append(m_accessPath.ToString(true));
    }

// ── BinaryOpNode ─────────────────────────────────────────────────────────────────
static Utf8CP BinarySqlOperatorToString(BinarySqlOperator op)
    {
    switch (op)
        {
        case BinarySqlOperator::Plus:       return "+";
        case BinarySqlOperator::Minus:      return "-";
        case BinarySqlOperator::Multiply:   return "*";
        case BinarySqlOperator::Divide:     return "/";
        case BinarySqlOperator::Modulo:     return "%";
        case BinarySqlOperator::ShiftLeft:  return "<<";
        case BinarySqlOperator::ShiftRight: return ">>";
        case BinarySqlOperator::BitwiseOr:  return "|";
        case BinarySqlOperator::BitwiseAnd: return "&";
        case BinarySqlOperator::BitwiseXOr: return "^";
        case BinarySqlOperator::Concat:     return "||";
        default: BeAssert(false); return "?";
        }
    }

void BinaryOpNode::ToECSql(Utf8StringR out) const
    {
    m_lhs->ToECSql(out);
    out.append(" ").append(BinarySqlOperatorToString(m_op)).append(" ");
    m_rhs->ToECSql(out);
    }

// ── UnaryOpNode ──────────────────────────────────────────────────────────────────
void UnaryOpNode::ToECSql(Utf8StringR out) const
    {
    switch (m_op)
        {
        case Op::Minus:      out.append("-"); break;
        case Op::Plus:       out.append("+"); break;
        case Op::BitwiseNot: out.append("~"); break;
        case Op::Not:        out.append("NOT "); break;
        }
    m_operand->ToECSql(out);
    }

// ── CompareNode ──────────────────────────────────────────────────────────────────
static Utf8CP BooleanSqlOperatorToString(BooleanSqlOperator op)
    {
    switch (op)
        {
        case BooleanSqlOperator::EqualTo:              return "=";
        case BooleanSqlOperator::NotEqualTo:           return "<>";
        case BooleanSqlOperator::LessThan:             return "<";
        case BooleanSqlOperator::LessThanOrEqualTo:    return "<=";
        case BooleanSqlOperator::GreaterThan:          return ">";
        case BooleanSqlOperator::GreaterThanOrEqualTo: return ">=";
        case BooleanSqlOperator::Is:                   return "IS";
        case BooleanSqlOperator::IsNot:                return "IS NOT";
        case BooleanSqlOperator::Like:                 return "LIKE";
        case BooleanSqlOperator::NotLike:              return "NOT LIKE";
        case BooleanSqlOperator::Match:                return "MATCH";
        case BooleanSqlOperator::NotMatch:             return "NOT MATCH";
        case BooleanSqlOperator::In:                   return "IN";
        case BooleanSqlOperator::NotIn:                return "NOT IN";
        case BooleanSqlOperator::Between:              return "BETWEEN";
        case BooleanSqlOperator::NotBetween:           return "NOT BETWEEN";
        default: return "?";
        }
    }

void CompareNode::ToECSql(Utf8StringR out) const
    {
    m_lhs->ToECSql(out);
    out.append(" ").append(BooleanSqlOperatorToString(m_op));
    if (m_rhs)
        {
        out.append(" ");
        m_rhs->ToECSql(out);
        }
    }

// ── LogicalNode ──────────────────────────────────────────────────────────────────
void LogicalNode::ToECSql(Utf8StringR out) const
    {
    m_lhs->ToECSql(out);
    out.append(m_op == Op::And ? " AND " : " OR ");
    m_rhs->ToECSql(out);
    }

// ── BetweenNode ──────────────────────────────────────────────────────────────────
void BetweenNode::ToECSql(Utf8StringR out) const
    {
    m_operand->ToECSql(out);
    out.append(m_isNot ? " NOT BETWEEN " : " BETWEEN ");
    m_lower->ToECSql(out);
    out.append(" AND ");
    m_upper->ToECSql(out);
    }

// ── LikeNode ─────────────────────────────────────────────────────────────────────
void LikeNode::ToECSql(Utf8StringR out) const
    {
    m_operand->ToECSql(out);
    out.append(m_isNot ? " NOT LIKE " : " LIKE ");
    m_pattern->ToECSql(out);
    if (m_escape)
        {
        out.append(" ESCAPE ");
        m_escape->ToECSql(out);
        }
    }

// ── FuncCallNode ─────────────────────────────────────────────────────────────────
void FuncCallNode::ToECSql(Utf8StringR out) const
    {
    out.append(m_functionName);
    if (!m_isGetter)
        {
        out.append("(");
        if (m_setQuantifier == SqlSetQuantifier::Distinct)
            out.append("DISTINCT ");
        else if (m_setQuantifier == SqlSetQuantifier::All)
            out.append("ALL ");

        for (size_t i = 0; i < m_args.size(); ++i)
            {
            if (i > 0)
                out.append(", ");
            m_args[i]->ToECSql(out);
            }
        out.append(")");
        }
    }

// ── CastNode ─────────────────────────────────────────────────────────────────────
void CastNode::ToECSql(Utf8StringR out) const
    {
    out.append("CAST(");
    m_operand->ToECSql(out);
    out.append(" AS ");
    if (!m_targetSchemaName.empty())
        out.append(m_targetSchemaName).append(".");
    out.append(m_targetTypeName);
    if (m_targetIsArray)
        out.append("[]");
    out.append(")");
    }

// ── CaseNode ─────────────────────────────────────────────────────────────────────
void CaseNode::ToECSql(Utf8StringR out) const
    {
    out.append("CASE");
    for (auto const& wt : m_whenList)
        {
        out.append(" WHEN ");
        wt.m_when->ToECSql(out);
        out.append(" THEN ");
        wt.m_then->ToECSql(out);
        }
    if (m_elseExpr)
        {
        out.append(" ELSE ");
        m_elseExpr->ToECSql(out);
        }
    out.append(" END");
    }

// ── IIFNode ──────────────────────────────────────────────────────────────────────
void IIFNode::ToECSql(Utf8StringR out) const
    {
    out.append("IIF(");
    m_condition->ToECSql(out);
    out.append(", ");
    m_thenExpr->ToECSql(out);
    out.append(", ");
    m_elseExpr->ToECSql(out);
    out.append(")");
    }

// ── SubqueryExprNode ─────────────────────────────────────────────────────────────
void SubqueryExprNode::ToECSql(Utf8StringR out) const
    {
    out.append("(");
    m_query->ToECSql(out);
    out.append(")");
    }

// ── ExistsTestNode ───────────────────────────────────────────────────────────────
void ExistsTestNode::ToECSql(Utf8StringR out) const
    {
    if (m_isNot)
        out.append("NOT ");
    out.append("EXISTS (");
    m_query->ToECSql(out);
    out.append(")");
    }

// ── InListNode ───────────────────────────────────────────────────────────────────
void InListNode::ToECSql(Utf8StringR out) const
    {
    m_operand->ToECSql(out);
    out.append(m_isNot ? " NOT IN (" : " IN (");
    if (m_subquery)
        m_subquery->ToECSql(out);
    else
        {
        for (size_t i = 0; i < m_values.size(); ++i)
            {
            if (i > 0) out.append(", ");
            m_values[i]->ToECSql(out);
            }
        }
    out.append(")");
    }

// ── AllAnyNode ───────────────────────────────────────────────────────────────────
void AllAnyNode::ToECSql(Utf8StringR out) const
    {
    m_operand->ToECSql(out);
    out.append(" ").append(BooleanSqlOperatorToString(m_compareOp)).append(" ");
    switch (m_quantifier)
        {
        case SqlCompareListType::All: out.append("ALL "); break;
        case SqlCompareListType::Any: out.append("ANY "); break;
        case SqlCompareListType::Some: out.append("SOME "); break;
        }
    out.append("(");
    m_subquery->ToECSql(out);
    out.append(")");
    }

// ── TypePredicateNode ────────────────────────────────────────────────────────────
void TypePredicateNode::ToECSql(Utf8StringR out) const
    {
    // Rendered as function-style: original ECSQL syntax placeholder
    out.append("IsOfType(");
    m_operand->ToECSql(out);
    out.append(", ...)");
    }

// ── NavValueNode ─────────────────────────────────────────────────────────────────
void NavValueNode::ToECSql(Utf8StringR out) const
    {
    out.append("NavClassDef(");
    m_idExpr->ToECSql(out);
    out.append(", ");
    m_relClassIdExpr->ToECSql(out);
    out.append(")");
    }

// ── ExtractPropNode ──────────────────────────────────────────────────────────────
void ExtractPropNode::ToECSql(Utf8StringR out) const
    {
    out.append(m_instancePath.ToString());
    out.append("->");
    out.append(m_targetPath.ToString());
    }

// ── ExtractInstanceNode ──────────────────────────────────────────────────────────
void ExtractInstanceNode::ToECSql(Utf8StringR out) const
    {
    out.append(m_instancePath.ToString());
    }

// ── ExprListNode ─────────────────────────────────────────────────────────────────
void ExprListNode::ToECSql(Utf8StringR out) const
    {
    for (size_t i = 0; i < m_exprs.size(); ++i)
        {
        if (i > 0) out.append(", ");
        m_exprs[i]->ToECSql(out);
        }
    }

// ── ClassRefNode ─────────────────────────────────────────────────────────────────
void ClassRefNode::ToECSql(Utf8StringR out) const
    {
    if (!m_polymorphic.IsPolymorphic())
        out.append("ONLY ");
    if (!m_tableSpace.empty())
        out.append(m_tableSpace).append(".");
    if (!m_schemaAlias.empty())
        out.append("[").append(m_schemaAlias).append("].");
    out.append("[").append(m_className).append("]");
    if (!m_alias.empty())
        out.append(" ").append(m_alias);
    }

// ── SubqueryRefNode ──────────────────────────────────────────────────────────────
void SubqueryRefNode::ToECSql(Utf8StringR out) const
    {
    out.append("(");
    m_query->ToECSql(out);
    out.append(")");
    if (!m_alias.empty())
        out.append(" ").append(m_alias);
    }

// ── TableValuedFuncNode ──────────────────────────────────────────────────────────
void TableValuedFuncNode::ToECSql(Utf8StringR out) const
    {
    if (!m_schemaName.empty())
        out.append(m_schemaName).append(".");
    out.append(m_functionName).append("(");
    for (size_t i = 0; i < m_args.size(); ++i)
        {
        if (i > 0) out.append(", ");
        m_args[i]->ToECSql(out);
        }
    out.append(")");
    if (!m_alias.empty())
        out.append(" ").append(m_alias);
    }

// ── JoinNode ─────────────────────────────────────────────────────────────────────
static Utf8CP JoinTypeToString(ECSqlJoinType jt)
    {
    switch (jt)
        {
        case ECSqlJoinType::InnerJoin:       return "INNER JOIN";
        case ECSqlJoinType::LeftOuterJoin:    return "LEFT OUTER JOIN";
        case ECSqlJoinType::RightOuterJoin:   return "RIGHT OUTER JOIN";
        case ECSqlJoinType::FullOuterJoin:    return "FULL OUTER JOIN";
        case ECSqlJoinType::CrossJoin:        return "CROSS JOIN";
        case ECSqlJoinType::NaturalJoin:      return "NATURAL JOIN";
        default: return "JOIN";
        }
    }

void JoinNode::ToECSql(Utf8StringR out) const
    {
    m_lhs->ToECSql(out);
    out.append(" ").append(JoinTypeToString(m_joinType)).append(" ");
    m_rhs->ToECSql(out);
    if (m_condition)
        {
        out.append(" ON ");
        m_condition->ToECSql(out);
        }
    }

// ── RelJoinNode ──────────────────────────────────────────────────────────────────
void RelJoinNode::ToECSql(Utf8StringR out) const
    {
    m_lhs->ToECSql(out);
    out.append(" JOIN ");
    m_rhs->ToECSql(out);
    out.append(" USING [").append(m_relationshipClassMap.GetClass().GetName()).append("]");
    }

// ── CTE Nodes ────────────────────────────────────────────────────────────────────
void CteBlockNode::ToECSql(Utf8StringR out) const
    {
    out.append(m_name);
    if (!m_columnNames.empty())
        {
        out.append("(");
        for (size_t i = 0; i < m_columnNames.size(); ++i)
            {
            if (i > 0) out.append(", ");
            out.append(m_columnNames[i]);
            }
        out.append(")");
        }
    out.append(" AS (");
    m_query->ToECSql(out);
    out.append(")");
    }

void CteNode::ToECSql(Utf8StringR out) const
    {
    out.append("WITH ");
    if (m_isRecursive)
        out.append("RECURSIVE ");
    for (size_t i = 0; i < m_blocks.size(); ++i)
        {
        if (i > 0) out.append(", ");
        m_blocks[i]->ToECSql(out);
        }
    }

void CteBlockRefNode::ToECSql(Utf8StringR out) const
    {
    out.append(m_cteName);
    if (!m_alias.empty() && !m_alias.Equals(m_cteName))
        out.append(" ").append(m_alias);
    }

// ── Clause Nodes ─────────────────────────────────────────────────────────────────
void SelectItemNode::ToECSql(Utf8StringR out) const
    {
    m_expr->ToECSql(out);
    if (!m_alias.empty())
        out.append(" AS ").append(m_alias);
    }

void FromNode::ToECSql(Utf8StringR out) const
    {
    out.append("FROM ");
    for (size_t i = 0; i < m_tableRefs.size(); ++i)
        {
        if (i > 0) out.append(", ");
        m_tableRefs[i]->ToECSql(out);
        }
    }

void WhereNode::ToECSql(Utf8StringR out) const
    {
    out.append("WHERE ");
    m_condition->ToECSql(out);
    }

void GroupByNode::ToECSql(Utf8StringR out) const
    {
    out.append("GROUP BY ");
    for (size_t i = 0; i < m_exprs.size(); ++i)
        {
        if (i > 0) out.append(", ");
        m_exprs[i]->ToECSql(out);
        }
    }

void HavingNode::ToECSql(Utf8StringR out) const
    {
    out.append("HAVING ");
    m_condition->ToECSql(out);
    }

void OrderByItemNode::ToECSql(Utf8StringR out) const
    {
    m_expr->ToECSql(out);
    if (m_direction == SortDirection::Ascending)
        out.append(" ASC");
    else if (m_direction == SortDirection::Descending)
        out.append(" DESC");
    if (m_nullsOrder == NullsOrder::First)
        out.append(" NULLS FIRST");
    else if (m_nullsOrder == NullsOrder::Last)
        out.append(" NULLS LAST");
    }

void OrderByNode::ToECSql(Utf8StringR out) const
    {
    out.append("ORDER BY ");
    for (size_t i = 0; i < m_items.size(); ++i)
        {
        if (i > 0) out.append(", ");
        m_items[i]->ToECSql(out);
        }
    }

void LimitOffsetNode::ToECSql(Utf8StringR out) const
    {
    out.append("LIMIT ");
    m_limit->ToECSql(out);
    if (m_offset)
        {
        out.append(" OFFSET ");
        m_offset->ToECSql(out);
        }
    }

// ── Window Nodes ─────────────────────────────────────────────────────────────────
void WindowFrameNode::ToECSql(Utf8StringR out) const
    {
    switch (m_units)
        {
        case Units::Rows:   out.append("ROWS "); break;
        case Units::Range:  out.append("RANGE "); break;
        case Units::Groups: out.append("GROUPS "); break;
        }
    // simplified rendering
    out.append("...");
    }

void WindowPartitionNode::ToECSql(Utf8StringR out) const
    {
    out.append("PARTITION BY ");
    for (size_t i = 0; i < m_items.size(); ++i)
        {
        if (i > 0) out.append(", ");
        m_items[i].m_expr->ToECSql(out);
        }
    }

void WindowSpecNode::ToECSql(Utf8StringR out) const
    {
    out.append("(");
    if (m_partition) m_partition->ToECSql(out);
    if (m_orderBy) { out.append(" "); m_orderBy->ToECSql(out); }
    if (m_frame) { out.append(" "); m_frame->ToECSql(out); }
    out.append(")");
    }

void WindowDefNode::ToECSql(Utf8StringR out) const
    {
    out.append(m_name).append(" AS ");
    m_spec->ToECSql(out);
    }

void WindowDefListNode::ToECSql(Utf8StringR out) const
    {
    out.append("WINDOW ");
    for (size_t i = 0; i < m_defs.size(); ++i)
        {
        if (i > 0) out.append(", ");
        m_defs[i]->ToECSql(out);
        }
    }

void WindowFuncNode::ToECSql(Utf8StringR out) const
    {
    m_funcExpr->ToECSql(out);
    if (m_filterWhere)
        {
        out.append(" FILTER (WHERE ");
        m_filterWhere->ToECSql(out);
        out.append(")");
        }
    out.append(" OVER ");
    m_windowSpec->ToECSql(out);
    }

// ── Options ──────────────────────────────────────────────────────────────────────
void OptionNode::ToECSql(Utf8StringR out) const
    {
    out.append(m_name);
    if (!m_value.empty())
        out.append("=").append(m_value);
    }

void OptionsNode::ToECSql(Utf8StringR out) const
    {
    out.append("ECSQLOPTIONS ");
    for (size_t i = 0; i < m_options.size(); ++i)
        {
        if (i > 0) out.append(" ");
        m_options[i]->ToECSql(out);
        }
    }

// ── DML Helpers ──────────────────────────────────────────────────────────────────
void AssignmentNode::ToECSql(Utf8StringR out) const
    {
    m_property->ToECSql(out);
    out.append(" = ");
    m_value->ToECSql(out);
    }

void PropNameListNode::ToECSql(Utf8StringR out) const
    {
    for (size_t i = 0; i < m_props.size(); ++i)
        {
        if (i > 0) out.append(", ");
        m_props[i]->ToECSql(out);
        }
    }

// ── Query Nodes ──────────────────────────────────────────────────────────────────
void SingleSelectNode::ToECSql(Utf8StringR out) const
    {
    out.append("SELECT ");
    if (m_quantifier == SqlSetQuantifier::Distinct)
        out.append("DISTINCT ");
    else if (m_quantifier == SqlSetQuantifier::All)
        out.append("ALL ");

    for (size_t i = 0; i < m_selectItems.size(); ++i)
        {
        if (i > 0) out.append(", ");
        m_selectItems[i]->ToECSql(out);
        }

    if (m_from)
        { out.append(" "); m_from->ToECSql(out); }
    if (m_where)
        { out.append(" "); m_where->ToECSql(out); }
    if (m_groupBy)
        { out.append(" "); m_groupBy->ToECSql(out); }
    if (m_having)
        { out.append(" "); m_having->ToECSql(out); }
    if (m_windowDefs)
        { out.append(" "); m_windowDefs->ToECSql(out); }
    if (m_orderBy)
        { out.append(" "); m_orderBy->ToECSql(out); }
    if (m_limitOffset)
        { out.append(" "); m_limitOffset->ToECSql(out); }
    if (m_options)
        { out.append(" "); m_options->ToECSql(out); }
    }

void CompoundSelectNode::ToECSql(Utf8StringR out) const
    {
    m_lhs->ToECSql(out);
    switch (m_op)
        {
        case Op::Union:     out.append(m_isAll ? " UNION ALL " : " UNION "); break;
        case Op::Intersect: out.append(" INTERSECT "); break;
        case Op::Except:    out.append(" EXCEPT "); break;
        }
    m_rhs->ToECSql(out);
    }

// ── Statement Nodes ──────────────────────────────────────────────────────────────
void SelectStmtNode::ToECSql(Utf8StringR out) const
    {
    if (m_cte)
        { m_cte->ToECSql(out); out.append(" "); }
    m_query->ToECSql(out);
    }

void InsertStmtNode::ToECSql(Utf8StringR out) const
    {
    out.append("INSERT INTO ");
    m_classRef->ToECSql(out);
    out.append(" (");
    m_propNames->ToECSql(out);
    out.append(")");

    if (m_selectQuery)
        {
        out.append(" ");
        m_selectQuery->ToECSql(out);
        }
    else
        {
        out.append(" VALUES (");
        for (size_t i = 0; i < m_values.size(); ++i)
            {
            if (i > 0) out.append(", ");
            m_values[i]->ToECSql(out);
            }
        out.append(")");
        }
    }

void UpdateStmtNode::ToECSql(Utf8StringR out) const
    {
    out.append("UPDATE ");
    m_classRef->ToECSql(out);
    out.append(" SET ");
    for (size_t i = 0; i < m_assignments.size(); ++i)
        {
        if (i > 0) out.append(", ");
        m_assignments[i]->ToECSql(out);
        }
    if (m_where)
        { out.append(" "); m_where->ToECSql(out); }
    if (m_options)
        { out.append(" "); m_options->ToECSql(out); }
    }

void DeleteStmtNode::ToECSql(Utf8StringR out) const
    {
    out.append("DELETE FROM ");
    m_classRef->ToECSql(out);
    if (m_where)
        { out.append(" "); m_where->ToECSql(out); }
    if (m_options)
        { out.append(" "); m_options->ToECSql(out); }
    }

void PragmaStmtNode::ToECSql(Utf8StringR out) const
    {
    out.append("PRAGMA ");
    for (size_t i = 0; i < m_pathTokens.size(); ++i)
        {
        if (i > 0) out.append(".");
        out.append(m_pathTokens[i]);
        }
    out.append(".").append(m_name);
    }

} // namespace ECSqlAst
END_BENTLEY_SQLITE_EC_NAMESPACE

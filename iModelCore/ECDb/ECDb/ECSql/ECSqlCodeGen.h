/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECSqlAst.h"
#include "ECSqlPrepareContext.h"
#include "NativeSqlBuilder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
//! ECSqlCodeGen — single-pass SQL code generator that walks the ECSqlAst tree and
//! produces native SQLite SQL via NativeSqlBuilder.
//!
//! Unlike the old ECSqlPreparer suite (4 files, ~3700 lines), this consolidates all
//! SQL generation into one class that:
//!   1. Uses PropertyMap directly for simple OwnTable / single-table lookups.
//!   2. Delegates to ViewGenerator only for complex cases (polymorphic TPH, mixin UNION ALL,
//!      overflow table JOINs).
//!   3. Operates on a schema-resolved AST — no runtime schema lookups needed.
//!
//! The code generator implements ECSqlAst::IVisitor for traversal.
//======================================================================================
struct ECSqlCodeGen final : ECSqlAst::IVisitor
    {
    private:
        ECSqlPrepareContext& m_ctx;
        NativeSqlBuilder m_sql;

        // Internal state for tracking SELECT clause column count, nested scopes, etc.
        struct Scope
            {
            ECSqlType m_ecsqlType;
            int m_selectColumnCount = 0;
            Scope(ECSqlType t) : m_ecsqlType(t) {}
            };
        std::vector<Scope> m_scopeStack;

        void PushScope(ECSqlType type) { m_scopeStack.push_back(Scope(type)); }
        void PopScope() { BeAssert(!m_scopeStack.empty()); m_scopeStack.pop_back(); }
        Scope& CurrentScope() { BeAssert(!m_scopeStack.empty()); return m_scopeStack.back(); }

        // ── Property path → SQL column translation ───────────────────────────────
        //! Translates a resolved PropRefNode into one or more native SQL column references.
        //! Handles: primitive, struct fan-out, array JSON blob, navigation decompose,
        //! system properties, shared columns, overflow table qualifications.
        ECSqlStatus GeneratePropertySql(NativeSqlBuilder::List& snippets, ECSqlAst::PropRefNode const& propRef);

        //! Generate SQL for a system property (ECInstanceId, ECClassId, etc.).
        ECSqlStatus GenerateSystemPropertySql(NativeSqlBuilder::List& snippets, ECSqlAst::PropRefNode const& propRef);

        //! Generate SQL for a class reference in a FROM clause.
        //! For simple cases (OwnTable, non-polymorphic single table), emits [tableName] alias.
        //! For complex cases, delegates to ViewGenerator::GenerateSelectFromViewSql().
        ECSqlStatus GenerateClassRefSql(NativeSqlBuilder& sql, ECSqlAst::ClassRefNode const& classRef);

        // ── Visitor implementation ───────────────────────────────────────────────
        // Statements
        ECSqlStatus _Visit(ECSqlAst::SelectStmtNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::InsertStmtNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::UpdateStmtNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::DeleteStmtNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::PragmaStmtNode const&) override;

        // Query nodes
        ECSqlStatus _Visit(ECSqlAst::SingleSelectNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::CompoundSelectNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::SelectItemNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::FromNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::WhereNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::GroupByNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::HavingNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::OrderByNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::OrderByItemNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::LimitOffsetNode const&) override;

        // Table references
        ECSqlStatus _Visit(ECSqlAst::ClassRefNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::SubqueryRefNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::TableValuedFuncNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::CteNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::CteBlockNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::CteBlockRefNode const&) override;

        // Joins
        ECSqlStatus _Visit(ECSqlAst::JoinNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::RelJoinNode const&) override;

        // Expressions
        ECSqlStatus _Visit(ECSqlAst::LiteralNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::ParamNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::PropRefNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::EnumValueNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::BinaryOpNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::UnaryOpNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::CompareNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::LogicalNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::FuncCallNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::CastNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::CaseNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::IIFNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::SubqueryExprNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::ExistsTestNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::InListNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::AllAnyNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::TypePredicateNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::NavValueNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::ExtractPropNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::ExtractInstanceNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::BetweenNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::LikeNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::WindowFuncNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::ExprListNode const&) override;

        // DML
        ECSqlStatus _Visit(ECSqlAst::AssignmentNode const&) override;

        // Options
        ECSqlStatus _Visit(ECSqlAst::OptionsNode const&) override;

        // Window
        ECSqlStatus _Visit(ECSqlAst::WindowSpecNode const&) override;
        ECSqlStatus _Visit(ECSqlAst::WindowDefListNode const&) override;

        // Expression visitor helper: generates SQL for any ExprNode and appends to m_sql.
        ECSqlStatus VisitExpr(ECSqlAst::ExprNode const& expr);

        // Table ref visitor helper: generates SQL for any table ref node and appends to m_sql.
        ECSqlStatus VisitTableRef(ECSqlAst::Node const& tableRef);

    public:
        explicit ECSqlCodeGen(ECSqlPrepareContext& ctx) : m_ctx(ctx) {}

        //! Main entry point: generate native SQL from an AST statement node.
        //! @param[out] nativeSql   The generated SQLite SQL string.
        //! @param[in]  stmt        The root AST node (SelectStmtNode, InsertStmtNode, etc.)
        //! @return ECSqlStatus::Success or an error status (errors reported via ctx.Issues()).
        ECSqlStatus Generate(Utf8StringR nativeSql, ECSqlAst::Node const& stmt);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

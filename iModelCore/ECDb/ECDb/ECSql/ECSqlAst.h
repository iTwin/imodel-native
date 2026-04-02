/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "../ECDbInternalTypes.h"
#include "../ClassMap.h"
#include "ECSqlTypeInfo.h"
#include "NativeSqlBuilder.h"
#include "PragmaStatementExp.h"   // PragmaVal
#include <memory>
#include <vector>
#include <set>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// ECSqlAst — Purpose-built, schema-resolved AST for ECSQL.
//
// Design principles:
//   1. Every class/property reference is resolved at parse time (no deferred finalization).
//   2. ClassRefNode holds ClassMap const&; PropRefNode holds resolved PropertyPath + PropertyMap const*.
//   3. Boolean and value expressions share a single ExprNode base (SQL semantics: booleans are values).
//   4. Visitor interface (IECSqlAstVisitor) for code generation and pretty-printing.
//   5. Nodes are immutable after construction — built bottom-up by the parser.
//======================================================================================

namespace ECSqlAst {

// Forward declarations — every concrete node type
struct Node;

// Statements
struct SelectStmtNode;
struct InsertStmtNode;
struct UpdateStmtNode;
struct DeleteStmtNode;
struct PragmaStmtNode;

// Query building blocks
struct SingleSelectNode;
struct CompoundSelectNode;
struct SelectItemNode;
struct FromNode;
struct WhereNode;
struct GroupByNode;
struct HavingNode;
struct OrderByNode;
struct OrderByItemNode;
struct LimitOffsetNode;

// Table references
struct ClassRefNode;
struct SubqueryRefNode;
struct TableValuedFuncNode;
struct CteNode;
struct CteBlockNode;
struct CteBlockRefNode;

// Joins
struct JoinNode;
struct RelJoinNode;

// Expressions
struct ExprNode;
struct LiteralNode;
struct ParamNode;
struct PropRefNode;
struct EnumValueNode;
struct BinaryOpNode;
struct UnaryOpNode;
struct CompareNode;
struct LogicalNode;
struct FuncCallNode;
struct CastNode;
struct CaseNode;
struct IIFNode;
struct SubqueryExprNode;
struct ExistsTestNode;
struct InListNode;
struct AllAnyNode;
struct TypePredicateNode;
struct NavValueNode;
struct ExtractPropNode;
struct ExtractInstanceNode;
struct BetweenNode;
struct LikeNode;
struct WindowFuncNode;
struct ExprListNode;

// DML helpers
struct AssignmentNode;
struct PropNameListNode;

// Options
struct OptionsNode;
struct OptionNode;

// Window function helpers
struct WindowSpecNode;
struct WindowDefNode;
struct WindowDefListNode;
struct WindowFrameNode;
struct WindowPartitionNode;

//=======================================================================================
//! Visitor interface for walking the AST. Code generator and pretty-printer implement this.
//! Return ECSqlStatus::Success to continue, or an error to abort.
//=======================================================================================
struct IVisitor
    {
    virtual ~IVisitor() {}

    // Statements
    virtual ECSqlStatus _Visit(SelectStmtNode const&) = 0;
    virtual ECSqlStatus _Visit(InsertStmtNode const&) = 0;
    virtual ECSqlStatus _Visit(UpdateStmtNode const&) = 0;
    virtual ECSqlStatus _Visit(DeleteStmtNode const&) = 0;
    virtual ECSqlStatus _Visit(PragmaStmtNode const&) = 0;

    // Query nodes
    virtual ECSqlStatus _Visit(SingleSelectNode const&) = 0;
    virtual ECSqlStatus _Visit(CompoundSelectNode const&) = 0;
    virtual ECSqlStatus _Visit(SelectItemNode const&) = 0;
    virtual ECSqlStatus _Visit(FromNode const&) = 0;
    virtual ECSqlStatus _Visit(WhereNode const&) = 0;
    virtual ECSqlStatus _Visit(GroupByNode const&) = 0;
    virtual ECSqlStatus _Visit(HavingNode const&) = 0;
    virtual ECSqlStatus _Visit(OrderByNode const&) = 0;
    virtual ECSqlStatus _Visit(OrderByItemNode const&) = 0;
    virtual ECSqlStatus _Visit(LimitOffsetNode const&) = 0;

    // Table references
    virtual ECSqlStatus _Visit(ClassRefNode const&) = 0;
    virtual ECSqlStatus _Visit(SubqueryRefNode const&) = 0;
    virtual ECSqlStatus _Visit(TableValuedFuncNode const&) = 0;
    virtual ECSqlStatus _Visit(CteNode const&) = 0;
    virtual ECSqlStatus _Visit(CteBlockNode const&) = 0;
    virtual ECSqlStatus _Visit(CteBlockRefNode const&) = 0;

    // Joins
    virtual ECSqlStatus _Visit(JoinNode const&) = 0;
    virtual ECSqlStatus _Visit(RelJoinNode const&) = 0;

    // Expressions
    virtual ECSqlStatus _Visit(LiteralNode const&) = 0;
    virtual ECSqlStatus _Visit(ParamNode const&) = 0;
    virtual ECSqlStatus _Visit(PropRefNode const&) = 0;
    virtual ECSqlStatus _Visit(EnumValueNode const&) = 0;
    virtual ECSqlStatus _Visit(BinaryOpNode const&) = 0;
    virtual ECSqlStatus _Visit(UnaryOpNode const&) = 0;
    virtual ECSqlStatus _Visit(CompareNode const&) = 0;
    virtual ECSqlStatus _Visit(LogicalNode const&) = 0;
    virtual ECSqlStatus _Visit(FuncCallNode const&) = 0;
    virtual ECSqlStatus _Visit(CastNode const&) = 0;
    virtual ECSqlStatus _Visit(CaseNode const&) = 0;
    virtual ECSqlStatus _Visit(IIFNode const&) = 0;
    virtual ECSqlStatus _Visit(SubqueryExprNode const&) = 0;
    virtual ECSqlStatus _Visit(ExistsTestNode const&) = 0;
    virtual ECSqlStatus _Visit(InListNode const&) = 0;
    virtual ECSqlStatus _Visit(AllAnyNode const&) = 0;
    virtual ECSqlStatus _Visit(TypePredicateNode const&) = 0;
    virtual ECSqlStatus _Visit(NavValueNode const&) = 0;
    virtual ECSqlStatus _Visit(ExtractPropNode const&) = 0;
    virtual ECSqlStatus _Visit(ExtractInstanceNode const&) = 0;
    virtual ECSqlStatus _Visit(BetweenNode const&) = 0;
    virtual ECSqlStatus _Visit(LikeNode const&) = 0;
    virtual ECSqlStatus _Visit(WindowFuncNode const&) = 0;
    virtual ECSqlStatus _Visit(ExprListNode const&) = 0;

    // DML helpers
    virtual ECSqlStatus _Visit(AssignmentNode const&) = 0;

    // Options
    virtual ECSqlStatus _Visit(OptionsNode const&) = 0;

    // Window helpers
    virtual ECSqlStatus _Visit(WindowSpecNode const&) = 0;
    virtual ECSqlStatus _Visit(WindowDefListNode const&) = 0;
    };

//=======================================================================================
//! Abstract base for all AST nodes.
//=======================================================================================
struct Node
    {
    enum class Kind
        {
        // Statements
        SelectStmt, InsertStmt, UpdateStmt, DeleteStmt, PragmaStmt,
        // Query
        SingleSelect, CompoundSelect, SelectItem, From, Where,
        GroupBy, Having, OrderBy, OrderByItem, LimitOffset,
        // Table refs
        ClassRef, SubqueryRef, TableValuedFunc, Cte, CteBlock, CteBlockRef,
        // Joins
        Join, RelJoin,
        // Expressions
        Literal, Param, PropRef, EnumValue,
        BinaryOp, UnaryOp, Compare, Logical,
        FuncCall, Cast, Case, IIF,
        SubqueryExpr, ExistsTest, InList, AllAny,
        TypePredicate, NavValue, ExtractProp, ExtractInstance,
        Between, Like, WindowFunc, ExprList,
        // DML
        Assignment, PropNameList,
        // Options
        Options, Option,
        // Window
        WindowSpec, WindowDef, WindowDefList, WindowFrame, WindowPartition,
        };

    private:
        Kind m_kind;

        Node(Node const&) = delete;
        Node& operator=(Node const&) = delete;

    protected:
        explicit Node(Kind kind) : m_kind(kind) {}

    public:
        virtual ~Node() {}

        Kind GetKind() const { return m_kind; }
        virtual ECSqlStatus Accept(IVisitor&) const = 0;

        //! Render this node back to ECSQL text (for diagnostics / round-trip).
        virtual void ToECSql(Utf8StringR out) const = 0;
    };

typedef std::unique_ptr<Node> NodePtr;

//=======================================================================================
// ── Expressions ──────────────────────────────────────────────────────────────────────
//
// Unlike the old Exp hierarchy that splits BooleanExp / ValueExp, the new AST unifies
// them into a single ExprNode base.  In SQL semantics a boolean IS a value (TRUE/FALSE/NULL).
// The ECSqlTypeInfo on each node carries the semantic type for validation.
//=======================================================================================

struct ExprNode : Node
    {
    private:
        ECSqlTypeInfo m_typeInfo;
        bool m_hasParentheses = false;

    protected:
        explicit ExprNode(Kind kind) : Node(kind) {}
        ExprNode(Kind kind, ECSqlTypeInfo typeInfo) : Node(kind), m_typeInfo(std::move(typeInfo)) {}

        void SetTypeInfo(ECSqlTypeInfo const& ti) { m_typeInfo = ti; }

    public:
        ECSqlTypeInfo const& GetTypeInfo() const { return m_typeInfo; }
        bool HasParentheses() const { return m_hasParentheses; }
        void SetHasParentheses() { m_hasParentheses = true; }
    };

typedef std::unique_ptr<ExprNode> ExprNodePtr;

//=======================================================================================
//! Literal value: integer, real, string, hex blob, NULL, TRUE, FALSE, date/time constants.
//=======================================================================================
struct LiteralNode final : ExprNode
    {
    private:
        Utf8String m_rawValue;  // original text from ECSQL

    public:
        LiteralNode(Utf8StringCR rawValue, ECSqlTypeInfo typeInfo)
            : ExprNode(Kind::Literal, std::move(typeInfo)), m_rawValue(rawValue) {}

        Utf8StringCR GetRawValue() const { return m_rawValue; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override { out.append(m_rawValue); }
    };

//=======================================================================================
//! Bound parameter: positional (?) or named (:name, @name).
//=======================================================================================
struct ParamNode final : ExprNode
    {
    private:
        Utf8String m_name;   // empty for positional "?"
        int m_paramIndex;    // 1-based parameter index

    public:
        ParamNode(Utf8StringCR name, int paramIndex)
            : ExprNode(Kind::Param), m_name(name), m_paramIndex(paramIndex) {}

        Utf8StringCR GetName() const { return m_name; }
        int GetParamIndex() const { return m_paramIndex; }
        bool IsNamed() const { return !m_name.empty(); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! Resolved property reference.  Carries the full resolved PropertyPath and the
//! ClassRefNode it belongs to, plus the resolved PropertyMap for direct SQL generation.
//=======================================================================================
struct PropRefNode final : ExprNode
    {
    private:
        PropertyPath m_resolvedPath;     // fully resolved (aliases expanded, EC properties attached)
        PropertyPath m_originalPath;     // as written in ECSQL
        ClassRefNode const* m_classRef;  // the ClassRefNode this property belongs to (non-owning)
        PropertyMap const* m_propertyMap; // resolved property-to-column mapping (may be null for CTE/subquery columns)
        ECSqlSystemPropertyInfo const* m_sysPropInfo;  // non-null if this is ECInstanceId, ECClassId, etc.

    public:
        PropRefNode(PropertyPath originalPath, PropertyPath resolvedPath,
                    ClassRefNode const* classRef,
                    PropertyMap const* propertyMap,
                    ECSqlSystemPropertyInfo const* sysPropInfo,
                    ECSqlTypeInfo typeInfo)
            : ExprNode(Kind::PropRef, std::move(typeInfo))
            , m_originalPath(std::move(originalPath))
            , m_resolvedPath(std::move(resolvedPath))
            , m_classRef(classRef)
            , m_propertyMap(propertyMap)
            , m_sysPropInfo(sysPropInfo)
            {}

        PropertyPath const& GetResolvedPath() const { return m_resolvedPath; }
        PropertyPath const& GetOriginalPath() const { return m_originalPath; }
        ClassRefNode const* GetClassRef() const { return m_classRef; }
        PropertyMap const* GetPropertyMap() const { return m_propertyMap; }
        bool IsSystemProperty() const { return m_sysPropInfo != nullptr; }
        ECSqlSystemPropertyInfo const& GetSystemPropertyInfo() const { BeAssert(m_sysPropInfo != nullptr); return *m_sysPropInfo; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! EC enum value reference (e.g., MySchema.MyEnum.Value1).
//=======================================================================================
struct EnumValueNode final : ExprNode
    {
    private:
        ECN::ECEnumeratorCR m_enumerator;
        PropertyPath m_accessPath;  // schema.enum.value as path

    public:
        EnumValueNode(ECN::ECEnumeratorCR enumerator, PropertyPath accessPath)
            : ExprNode(Kind::EnumValue), m_enumerator(enumerator), m_accessPath(std::move(accessPath)) {}

        ECN::ECEnumeratorCR GetEnumerator() const { return m_enumerator; }
        PropertyPath const& GetAccessPath() const { return m_accessPath; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! Binary arithmetic / bitwise / concatenation operator.
//=======================================================================================
struct BinaryOpNode final : ExprNode
    {
    private:
        BinarySqlOperator m_op;
        ExprNodePtr m_lhs;
        ExprNodePtr m_rhs;

    public:
        BinaryOpNode(BinarySqlOperator op, ExprNodePtr lhs, ExprNodePtr rhs)
            : ExprNode(Kind::BinaryOp), m_op(op), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

        BinarySqlOperator GetOperator() const { return m_op; }
        ExprNode const& GetLhs() const { return *m_lhs; }
        ExprNode const& GetRhs() const { return *m_rhs; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! Unary operator: -, +, ~, NOT.
//=======================================================================================
struct UnaryOpNode final : ExprNode
    {
    enum class Op { Minus, Plus, BitwiseNot, Not };

    private:
        Op m_op;
        ExprNodePtr m_operand;

    public:
        UnaryOpNode(Op op, ExprNodePtr operand)
            : ExprNode(Kind::UnaryOp), m_op(op), m_operand(std::move(operand)) {}

        Op GetOperator() const { return m_op; }
        ExprNode const& GetOperand() const { return *m_operand; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! Comparison: =, <>, <, <=, >, >=, IS [NOT] NULL, IS [NOT] (for boolean).
//=======================================================================================
struct CompareNode final : ExprNode
    {
    private:
        BooleanSqlOperator m_op;
        ExprNodePtr m_lhs;
        ExprNodePtr m_rhs;  // nullptr for IS NULL / IS NOT NULL

    public:
        CompareNode(BooleanSqlOperator op, ExprNodePtr lhs, ExprNodePtr rhs = nullptr)
            : ExprNode(Kind::Compare, ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Boolean))
            , m_op(op), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

        BooleanSqlOperator GetOperator() const { return m_op; }
        ExprNode const& GetLhs() const { return *m_lhs; }
        ExprNode const* GetRhs() const { return m_rhs.get(); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! Logical AND / OR.
//=======================================================================================
struct LogicalNode final : ExprNode
    {
    enum class Op { And, Or };

    private:
        Op m_op;
        ExprNodePtr m_lhs;
        ExprNodePtr m_rhs;

    public:
        LogicalNode(Op op, ExprNodePtr lhs, ExprNodePtr rhs)
            : ExprNode(Kind::Logical, ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Boolean))
            , m_op(op), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

        Op GetOperator() const { return m_op; }
        ExprNode const& GetLhs() const { return *m_lhs; }
        ExprNode const& GetRhs() const { return *m_rhs; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! BETWEEN expr AND expr  /  NOT BETWEEN expr AND expr.
//=======================================================================================
struct BetweenNode final : ExprNode
    {
    private:
        bool m_isNot;
        ExprNodePtr m_operand;
        ExprNodePtr m_lower;
        ExprNodePtr m_upper;

    public:
        BetweenNode(bool isNot, ExprNodePtr operand, ExprNodePtr lower, ExprNodePtr upper)
            : ExprNode(Kind::Between, ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Boolean))
            , m_isNot(isNot), m_operand(std::move(operand)), m_lower(std::move(lower)), m_upper(std::move(upper)) {}

        bool IsNot() const { return m_isNot; }
        ExprNode const& GetOperand() const { return *m_operand; }
        ExprNode const& GetLower() const { return *m_lower; }
        ExprNode const& GetUpper() const { return *m_upper; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! LIKE / NOT LIKE with optional ESCAPE.
//=======================================================================================
struct LikeNode final : ExprNode
    {
    private:
        bool m_isNot;
        ExprNodePtr m_operand;
        ExprNodePtr m_pattern;
        ExprNodePtr m_escape;  // may be nullptr

    public:
        LikeNode(bool isNot, ExprNodePtr operand, ExprNodePtr pattern, ExprNodePtr escape = nullptr)
            : ExprNode(Kind::Like, ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Boolean))
            , m_isNot(isNot), m_operand(std::move(operand)), m_pattern(std::move(pattern)), m_escape(std::move(escape)) {}

        bool IsNot() const { return m_isNot; }
        ExprNode const& GetOperand() const { return *m_operand; }
        ExprNode const& GetPattern() const { return *m_pattern; }
        ExprNode const* GetEscape() const { return m_escape.get(); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! Function call (built-in or custom). Also covers aggregate functions.
//=======================================================================================
struct FuncCallNode final : ExprNode
    {
    private:
        Utf8String m_functionName;
        std::vector<ExprNodePtr> m_args;
        SqlSetQuantifier m_setQuantifier;
        bool m_isStandardSetFunction;
        bool m_isGetter;  // no parentheses (CURRENT_DATE etc.)

    public:
        FuncCallNode(Utf8StringCR name, SqlSetQuantifier setQuantifier = SqlSetQuantifier::NotSpecified,
                     bool isStandardSetFunction = false, bool isGetter = false)
            : ExprNode(Kind::FuncCall)
            , m_functionName(name), m_setQuantifier(setQuantifier)
            , m_isStandardSetFunction(isStandardSetFunction), m_isGetter(isGetter) {}

        Utf8StringCR GetFunctionName() const { return m_functionName; }
        SqlSetQuantifier GetSetQuantifier() const { return m_setQuantifier; }
        bool IsStandardSetFunction() const { return m_isStandardSetFunction; }
        bool IsGetter() const { return m_isGetter; }
        std::vector<ExprNodePtr> const& GetArgs() const { return m_args; }
        void AddArg(ExprNodePtr arg) { m_args.push_back(std::move(arg)); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! CAST(expr AS typename).
//=======================================================================================
struct CastNode final : ExprNode
    {
    private:
        ExprNodePtr m_operand;
        Utf8String m_targetSchemaName;   // empty for primitive types
        Utf8String m_targetTypeName;     // primitive type name or EC class type name
        bool m_targetIsArray;

    public:
        CastNode(ExprNodePtr operand, Utf8StringCR targetTypeName, bool targetIsArray)
            : ExprNode(Kind::Cast), m_operand(std::move(operand))
            , m_targetTypeName(targetTypeName), m_targetIsArray(targetIsArray) {}

        CastNode(ExprNodePtr operand, Utf8StringCR targetSchemaName, Utf8StringCR targetTypeName, bool targetIsArray)
            : ExprNode(Kind::Cast), m_operand(std::move(operand))
            , m_targetSchemaName(targetSchemaName), m_targetTypeName(targetTypeName), m_targetIsArray(targetIsArray) {}

        ExprNode const& GetOperand() const { return *m_operand; }
        Utf8StringCR GetTargetSchemaName() const { return m_targetSchemaName; }
        Utf8StringCR GetTargetTypeName() const { return m_targetTypeName; }
        bool IsTargetArray() const { return m_targetIsArray; }
        bool IsECType() const { return !m_targetSchemaName.empty(); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! CASE WHEN cond THEN value [WHEN ...] [ELSE value] END
//=======================================================================================
struct CaseNode final : ExprNode
    {
    struct WhenThen
        {
        ExprNodePtr m_when;
        ExprNodePtr m_then;
        };

    private:
        std::vector<WhenThen> m_whenList;
        ExprNodePtr m_elseExpr;  // may be nullptr

    public:
        CaseNode(std::vector<WhenThen> whenList, ExprNodePtr elseExpr = nullptr)
            : ExprNode(Kind::Case), m_whenList(std::move(whenList)), m_elseExpr(std::move(elseExpr)) {}

        std::vector<WhenThen> const& GetWhenList() const { return m_whenList; }
        ExprNode const* GetElse() const { return m_elseExpr.get(); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! IIF(condition, thenExpr, elseExpr)
//=======================================================================================
struct IIFNode final : ExprNode
    {
    private:
        ExprNodePtr m_condition;
        ExprNodePtr m_thenExpr;
        ExprNodePtr m_elseExpr;

    public:
        IIFNode(ExprNodePtr condition, ExprNodePtr thenExpr, ExprNodePtr elseExpr)
            : ExprNode(Kind::IIF), m_condition(std::move(condition))
            , m_thenExpr(std::move(thenExpr)), m_elseExpr(std::move(elseExpr)) {}

        ExprNode const& GetCondition() const { return *m_condition; }
        ExprNode const& GetThen() const { return *m_thenExpr; }
        ExprNode const& GetElse() const { return *m_elseExpr; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! Scalar subquery: (SELECT ...) used as a value.
//=======================================================================================
struct SubqueryExprNode final : ExprNode
    {
    private:
        std::unique_ptr<SelectStmtNode> m_query;

    public:
        explicit SubqueryExprNode(std::unique_ptr<SelectStmtNode> query)
            : ExprNode(Kind::SubqueryExpr), m_query(std::move(query)) {}

        SelectStmtNode const& GetQuery() const { return *m_query; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! EXISTS (subquery) / NOT EXISTS (subquery).
//=======================================================================================
struct ExistsTestNode final : ExprNode
    {
    private:
        bool m_isNot;
        std::unique_ptr<SelectStmtNode> m_query;

    public:
        ExistsTestNode(bool isNot, std::unique_ptr<SelectStmtNode> query)
            : ExprNode(Kind::ExistsTest, ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Boolean))
            , m_isNot(isNot), m_query(std::move(query)) {}

        bool IsNot() const { return m_isNot; }
        SelectStmtNode const& GetQuery() const { return *m_query; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! expr [NOT] IN (exprList) or expr [NOT] IN (subquery).
//=======================================================================================
struct InListNode final : ExprNode
    {
    private:
        bool m_isNot;
        ExprNodePtr m_operand;
        std::vector<ExprNodePtr> m_values;                  // populated if value list
        std::unique_ptr<SelectStmtNode> m_subquery;         // populated if subquery

    public:
        InListNode(bool isNot, ExprNodePtr operand, std::vector<ExprNodePtr> values)
            : ExprNode(Kind::InList, ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Boolean))
            , m_isNot(isNot), m_operand(std::move(operand)), m_values(std::move(values)) {}

        InListNode(bool isNot, ExprNodePtr operand, std::unique_ptr<SelectStmtNode> subquery)
            : ExprNode(Kind::InList, ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Boolean))
            , m_isNot(isNot), m_operand(std::move(operand)), m_subquery(std::move(subquery)) {}

        bool IsNot() const { return m_isNot; }
        ExprNode const& GetOperand() const { return *m_operand; }
        bool IsSubquery() const { return m_subquery != nullptr; }
        std::vector<ExprNodePtr> const& GetValues() const { return m_values; }
        SelectStmtNode const& GetSubquery() const { BeAssert(IsSubquery()); return *m_subquery; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! expr op ALL|ANY|SOME (subquery).
//=======================================================================================
struct AllAnyNode final : ExprNode
    {
    private:
        SqlCompareListType m_quantifier;  // All, Any, Some
        BooleanSqlOperator m_compareOp;
        ExprNodePtr m_operand;
        std::unique_ptr<SelectStmtNode> m_subquery;

    public:
        AllAnyNode(SqlCompareListType quantifier, BooleanSqlOperator op,
                   ExprNodePtr operand, std::unique_ptr<SelectStmtNode> subquery)
            : ExprNode(Kind::AllAny, ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Boolean))
            , m_quantifier(quantifier), m_compareOp(op)
            , m_operand(std::move(operand)), m_subquery(std::move(subquery)) {}

        SqlCompareListType GetQuantifier() const { return m_quantifier; }
        BooleanSqlOperator GetCompareOp() const { return m_compareOp; }
        ExprNode const& GetOperand() const { return *m_operand; }
        SelectStmtNode const& GetSubquery() const { return *m_subquery; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! IsOfType(expr, type1, type2, ...) — maps to ECClassId IN (...).
//=======================================================================================
struct TypePredicateNode final : ExprNode
    {
    private:
        ExprNodePtr m_operand;
        struct TypeEntry
            {
            ClassMap const& m_classMap;
            PolymorphicInfo m_polymorphic;
            TypeEntry(ClassMap const& cm, PolymorphicInfo pi) : m_classMap(cm), m_polymorphic(pi) {}
            };
        std::vector<TypeEntry> m_types;

    public:
        TypePredicateNode(ExprNodePtr operand, std::vector<TypeEntry> types)
            : ExprNode(Kind::TypePredicate, ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Boolean))
            , m_operand(std::move(operand)), m_types(std::move(types)) {}

        ExprNode const& GetOperand() const { return *m_operand; }
        std::vector<TypeEntry> const& GetTypes() const { return m_types; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! Navigation property value creation: NavClassDef(id, relClass).
//=======================================================================================
struct NavValueNode final : ExprNode
    {
    private:
        ExprNodePtr m_idExpr;
        ExprNodePtr m_relClassIdExpr;

    public:
        NavValueNode(ExprNodePtr idExpr, ExprNodePtr relClassIdExpr)
            : ExprNode(Kind::NavValue, ECSqlTypeInfo(ECSqlTypeInfo::Kind::Navigation))
            , m_idExpr(std::move(idExpr)), m_relClassIdExpr(std::move(relClassIdExpr)) {}

        ExprNode const& GetIdExpr() const { return *m_idExpr; }
        ExprNode const& GetRelClassIdExpr() const { return *m_relClassIdExpr; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! $->propPath extraction (JSON property extract from instance).
//=======================================================================================
struct ExtractPropNode final : ExprNode
    {
    private:
        PropertyPath m_instancePath;
        PropertyPath m_targetPath;
        bool m_isOptional;

    public:
        ExtractPropNode(PropertyPath instancePath, PropertyPath targetPath, bool isOptional)
            : ExprNode(Kind::ExtractProp, ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_String))
            , m_instancePath(std::move(instancePath)), m_targetPath(std::move(targetPath)), m_isOptional(isOptional) {}

        PropertyPath const& GetInstancePath() const { return m_instancePath; }
        PropertyPath const& GetTargetPath() const { return m_targetPath; }
        bool IsOptional() const { return m_isOptional; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! $ instance extraction (full JSON instance).
//=======================================================================================
struct ExtractInstanceNode final : ExprNode
    {
    private:
        PropertyPath m_instancePath;

    public:
        explicit ExtractInstanceNode(PropertyPath instancePath)
            : ExprNode(Kind::ExtractInstance, ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_String))
            , m_instancePath(std::move(instancePath)) {}

        PropertyPath const& GetInstancePath() const { return m_instancePath; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! Flat list of expressions (used in value lists, GROUP BY lists, etc.).
//=======================================================================================
struct ExprListNode final : ExprNode
    {
    private:
        std::vector<ExprNodePtr> m_exprs;

    public:
        ExprListNode() : ExprNode(Kind::ExprList) {}
        explicit ExprListNode(std::vector<ExprNodePtr> exprs)
            : ExprNode(Kind::ExprList), m_exprs(std::move(exprs)) {}

        std::vector<ExprNodePtr> const& GetExprs() const { return m_exprs; }
        void Add(ExprNodePtr expr) { m_exprs.push_back(std::move(expr)); }
        size_t Count() const { return m_exprs.size(); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
// ── Table References ─────────────────────────────────────────────────────────────────
//=======================================================================================

//=======================================================================================
//! Resolved EC class reference.  Always carries ClassMap const& — resolved at parse time.
//=======================================================================================
struct ClassRefNode final : Node
    {
    private:
        ClassMap const& m_classMap;
        PolymorphicInfo m_polymorphic;
        Utf8String m_alias;
        Utf8String m_className;
        Utf8String m_schemaAlias;
        Utf8String m_tableSpace;
        bool m_disqualifyPrimaryJoin;

    public:
        ClassRefNode(ClassMap const& classMap, PolymorphicInfo polymorphic,
                     Utf8StringCR className, Utf8StringCR schemaAlias, Utf8CP tableSpace,
                     Utf8StringCR alias, bool disqualifyPrimaryJoin = false)
            : Node(Kind::ClassRef)
            , m_classMap(classMap), m_polymorphic(std::move(polymorphic))
            , m_className(className), m_schemaAlias(schemaAlias)
            , m_tableSpace(tableSpace ? tableSpace : "")
            , m_alias(alias), m_disqualifyPrimaryJoin(disqualifyPrimaryJoin) {}

        ClassMap const& GetClassMap() const { return m_classMap; }
        PolymorphicInfo const& GetPolymorphicInfo() const { return m_polymorphic; }
        Utf8StringCR GetAlias() const { return m_alias; }
        Utf8StringCR GetClassName() const { return m_className; }
        Utf8StringCR GetSchemaAlias() const { return m_schemaAlias; }
        Utf8StringCR GetTableSpace() const { return m_tableSpace; }
        bool DisqualifyPrimaryJoin() const { return m_disqualifyPrimaryJoin; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! Subquery used as a table reference in FROM: (SELECT ...) alias.
//=======================================================================================
struct SubqueryRefNode final : Node
    {
    private:
        std::unique_ptr<SelectStmtNode> m_query;
        Utf8String m_alias;

    public:
        SubqueryRefNode(std::unique_ptr<SelectStmtNode> query, Utf8StringCR alias)
            : Node(Kind::SubqueryRef), m_query(std::move(query)), m_alias(alias) {}

        SelectStmtNode const& GetQuery() const { return *m_query; }
        Utf8StringCR GetAlias() const { return m_alias; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! Table-valued function in FROM clause.
//=======================================================================================
struct TableValuedFuncNode final : Node
    {
    private:
        Utf8String m_schemaName;
        Utf8String m_functionName;
        std::vector<ExprNodePtr> m_args;
        Utf8String m_alias;
        ECN::ECEntityClassCP m_virtualEntityClass;

    public:
        TableValuedFuncNode(Utf8StringCR schemaName, Utf8StringCR funcName,
                            std::vector<ExprNodePtr> args, Utf8StringCR alias,
                            ECN::ECEntityClassCP virtualClass = nullptr)
            : Node(Kind::TableValuedFunc)
            , m_schemaName(schemaName), m_functionName(funcName)
            , m_args(std::move(args)), m_alias(alias), m_virtualEntityClass(virtualClass) {}

        Utf8StringCR GetSchemaName() const { return m_schemaName; }
        Utf8StringCR GetFunctionName() const { return m_functionName; }
        std::vector<ExprNodePtr> const& GetArgs() const { return m_args; }
        Utf8StringCR GetAlias() const { return m_alias; }
        ECN::ECEntityClassCP GetVirtualEntityClass() const { return m_virtualEntityClass; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
// ── JOIN Nodes ───────────────────────────────────────────────────────────────────────
//=======================================================================================

//=======================================================================================
//! Standard SQL JOIN (INNER, LEFT, RIGHT, FULL, CROSS) with ON condition.
//=======================================================================================
struct JoinNode final : Node
    {
    private:
        ECSqlJoinType m_joinType;
        NodePtr m_lhs;       // ClassRefNode | JoinNode | SubqueryRefNode
        NodePtr m_rhs;       // ClassRefNode | SubqueryRefNode
        ExprNodePtr m_condition;   // ON clause (nullptr for CROSS JOIN)

    public:
        JoinNode(ECSqlJoinType joinType, NodePtr lhs, NodePtr rhs, ExprNodePtr condition = nullptr)
            : Node(Kind::Join), m_joinType(joinType)
            , m_lhs(std::move(lhs)), m_rhs(std::move(rhs)), m_condition(std::move(condition)) {}

        ECSqlJoinType GetJoinType() const { return m_joinType; }
        Node const& GetLhs() const { return *m_lhs; }
        Node const& GetRhs() const { return *m_rhs; }
        ExprNode const* GetCondition() const { return m_condition.get(); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! ECSQL-specific JOIN USING (RelationshipClass).
//=======================================================================================
struct RelJoinNode final : Node
    {
    enum class EndPointLocation { NotResolved = 0, InSource = 1, InTarget = 2, InBoth = 3 };

    struct ResolvedEndPoint
        {
        ClassRefNode const* m_classRef = nullptr;
        EndPointLocation m_location = EndPointLocation::NotResolved;
        };

    private:
        NodePtr m_lhs;
        NodePtr m_rhs;
        ClassMap const& m_relationshipClassMap;
        JoinDirection m_direction;
        ResolvedEndPoint m_resolvedFrom;
        ResolvedEndPoint m_resolvedTo;

    public:
        RelJoinNode(NodePtr lhs, NodePtr rhs, ClassMap const& relClassMap, JoinDirection direction)
            : Node(Kind::RelJoin)
            , m_lhs(std::move(lhs)), m_rhs(std::move(rhs))
            , m_relationshipClassMap(relClassMap), m_direction(direction) {}

        Node const& GetLhs() const { return *m_lhs; }
        Node const& GetRhs() const { return *m_rhs; }
        ClassMap const& GetRelationshipClassMap() const { return m_relationshipClassMap; }
        JoinDirection GetDirection() const { return m_direction; }
        ResolvedEndPoint const& GetResolvedFrom() const { return m_resolvedFrom; }
        ResolvedEndPoint const& GetResolvedTo() const { return m_resolvedTo; }
        void SetResolvedEndPoints(ResolvedEndPoint from, ResolvedEndPoint to) { m_resolvedFrom = from; m_resolvedTo = to; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
// ── CTE Nodes ────────────────────────────────────────────────────────────────────────
//=======================================================================================

struct CteBlockNode final : Node
    {
    private:
        Utf8String m_name;
        std::vector<Utf8String> m_columnNames;
        std::unique_ptr<SelectStmtNode> m_query;
        bool m_isRecursive;

    public:
        CteBlockNode(Utf8StringCR name, std::vector<Utf8String> columnNames,
                     std::unique_ptr<SelectStmtNode> query, bool isRecursive)
            : Node(Kind::CteBlock), m_name(name), m_columnNames(std::move(columnNames))
            , m_query(std::move(query)), m_isRecursive(isRecursive) {}

        Utf8StringCR GetName() const { return m_name; }
        std::vector<Utf8String> const& GetColumnNames() const { return m_columnNames; }
        SelectStmtNode const& GetQuery() const { return *m_query; }
        bool IsRecursive() const { return m_isRecursive; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

struct CteNode final : Node
    {
    private:
        std::vector<std::unique_ptr<CteBlockNode>> m_blocks;
        bool m_isRecursive;

    public:
        CteNode(std::vector<std::unique_ptr<CteBlockNode>> blocks, bool isRecursive)
            : Node(Kind::Cte), m_blocks(std::move(blocks)), m_isRecursive(isRecursive) {}

        std::vector<std::unique_ptr<CteBlockNode>> const& GetBlocks() const { return m_blocks; }
        bool IsRecursive() const { return m_isRecursive; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//! Reference to a CTE block name in FROM clause.
struct CteBlockRefNode final : Node
    {
    private:
        Utf8String m_cteName;
        Utf8String m_alias;
        CteBlockNode const* m_block;  // resolved back-reference (non-owning)

    public:
        CteBlockRefNode(Utf8StringCR cteName, Utf8StringCR alias, CteBlockNode const* block)
            : Node(Kind::CteBlockRef), m_cteName(cteName), m_alias(alias), m_block(block) {}

        Utf8StringCR GetCteName() const { return m_cteName; }
        Utf8StringCR GetAlias() const { return m_alias; }
        CteBlockNode const* GetBlock() const { return m_block; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
// ── Clause Nodes ─────────────────────────────────────────────────────────────────────
//=======================================================================================

//=======================================================================================
//! Single item in SELECT list: expression [AS alias].
//=======================================================================================
struct SelectItemNode final : Node
    {
    private:
        ExprNodePtr m_expr;
        Utf8String m_alias;    // empty if no alias
        bool m_isWildcard;     // true for * or ClassName.*

    public:
        SelectItemNode(ExprNodePtr expr, Utf8StringCR alias, bool isWildcard = false)
            : Node(Kind::SelectItem), m_expr(std::move(expr)), m_alias(alias), m_isWildcard(isWildcard) {}

        ExprNode const& GetExpr() const { return *m_expr; }
        Utf8StringCR GetAlias() const { return m_alias; }
        bool HasAlias() const { return !m_alias.empty(); }
        bool IsWildcard() const { return m_isWildcard; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! FROM clause: contains table references (ClassRef, SubqueryRef, Joins, CTERefs).
//=======================================================================================
struct FromNode final : Node
    {
    private:
        std::vector<NodePtr> m_tableRefs;  // each is ClassRefNode | SubqueryRefNode | JoinNode | RelJoinNode | CteBlockRefNode | TableValuedFuncNode

    public:
        FromNode() : Node(Kind::From) {}
        explicit FromNode(std::vector<NodePtr> tableRefs)
            : Node(Kind::From), m_tableRefs(std::move(tableRefs)) {}

        std::vector<NodePtr> const& GetTableRefs() const { return m_tableRefs; }
        void AddTableRef(NodePtr ref) { m_tableRefs.push_back(std::move(ref)); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

struct WhereNode final : Node
    {
    private:
        ExprNodePtr m_condition;

    public:
        explicit WhereNode(ExprNodePtr condition) : Node(Kind::Where), m_condition(std::move(condition)) {}

        ExprNode const& GetCondition() const { return *m_condition; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

struct GroupByNode final : Node
    {
    private:
        std::vector<ExprNodePtr> m_exprs;

    public:
        explicit GroupByNode(std::vector<ExprNodePtr> exprs) : Node(Kind::GroupBy), m_exprs(std::move(exprs)) {}

        std::vector<ExprNodePtr> const& GetExprs() const { return m_exprs; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

struct HavingNode final : Node
    {
    private:
        ExprNodePtr m_condition;

    public:
        explicit HavingNode(ExprNodePtr condition) : Node(Kind::Having), m_condition(std::move(condition)) {}

        ExprNode const& GetCondition() const { return *m_condition; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

struct OrderByItemNode final : Node
    {
    enum class SortDirection { Ascending, Descending, NotSpecified };
    enum class NullsOrder { NotSpecified, First, Last };

    private:
        ExprNodePtr m_expr;
        SortDirection m_direction;
        NullsOrder m_nullsOrder;

    public:
        OrderByItemNode(ExprNodePtr expr, SortDirection dir, NullsOrder nulls)
            : Node(Kind::OrderByItem), m_expr(std::move(expr)), m_direction(dir), m_nullsOrder(nulls) {}

        ExprNode const& GetExpr() const { return *m_expr; }
        SortDirection GetDirection() const { return m_direction; }
        NullsOrder GetNullsOrder() const { return m_nullsOrder; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

struct OrderByNode final : Node
    {
    private:
        std::vector<std::unique_ptr<OrderByItemNode>> m_items;

    public:
        explicit OrderByNode(std::vector<std::unique_ptr<OrderByItemNode>> items)
            : Node(Kind::OrderBy), m_items(std::move(items)) {}

        std::vector<std::unique_ptr<OrderByItemNode>> const& GetItems() const { return m_items; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

struct LimitOffsetNode final : Node
    {
    private:
        ExprNodePtr m_limit;
        ExprNodePtr m_offset;  // may be nullptr

    public:
        LimitOffsetNode(ExprNodePtr limit, ExprNodePtr offset = nullptr)
            : Node(Kind::LimitOffset), m_limit(std::move(limit)), m_offset(std::move(offset)) {}

        ExprNode const& GetLimit() const { return *m_limit; }
        ExprNode const* GetOffset() const { return m_offset.get(); }
        bool HasOffset() const { return m_offset != nullptr; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
// ── Window Function Nodes ────────────────────────────────────────────────────────────
//=======================================================================================

struct WindowFrameNode final : Node
    {
    enum class Units { Rows, Range, Groups };
    enum class BoundType { UnboundedPreceding, CurrentRow, ExprPreceding, ExprFollowing, UnboundedFollowing };
    enum class Exclusion { NotSpecified, CurrentRow, Group, Ties, NoOthers };

    private:
        Units m_units;
        BoundType m_startBound;
        ExprNodePtr m_startExpr;     // for ExprPreceding/ExprFollowing
        BoundType m_endBound;        // = CurrentRow if no BETWEEN
        ExprNodePtr m_endExpr;
        Exclusion m_exclusion;
        bool m_hasBetween;

    public:
        WindowFrameNode(Units units, BoundType startBound, ExprNodePtr startExpr,
                        bool hasBetween, BoundType endBound, ExprNodePtr endExpr,
                        Exclusion exclusion)
            : Node(Kind::WindowFrame), m_units(units)
            , m_startBound(startBound), m_startExpr(std::move(startExpr))
            , m_hasBetween(hasBetween), m_endBound(endBound), m_endExpr(std::move(endExpr))
            , m_exclusion(exclusion) {}

        Units GetUnits() const { return m_units; }
        BoundType GetStartBound() const { return m_startBound; }
        ExprNode const* GetStartExpr() const { return m_startExpr.get(); }
        bool HasBetween() const { return m_hasBetween; }
        BoundType GetEndBound() const { return m_endBound; }
        ExprNode const* GetEndExpr() const { return m_endExpr.get(); }
        Exclusion GetExclusion() const { return m_exclusion; }

        ECSqlStatus Accept(IVisitor& v) const override { return ECSqlStatus::Success; } // visited via parent
        void ToECSql(Utf8StringR out) const override;
    };

struct WindowPartitionNode final : Node
    {
    struct PartitionItem
        {
        ExprNodePtr m_expr;
        enum class Collation { NotSpecified, Binary, NoCase, Rtrim };
        Collation m_collation = Collation::NotSpecified;
        };

    private:
        std::vector<PartitionItem> m_items;

    public:
        explicit WindowPartitionNode(std::vector<PartitionItem> items)
            : Node(Kind::WindowPartition), m_items(std::move(items)) {}

        std::vector<PartitionItem> const& GetItems() const { return m_items; }

        ECSqlStatus Accept(IVisitor& v) const override { return ECSqlStatus::Success; }
        void ToECSql(Utf8StringR out) const override;
    };

struct WindowSpecNode final : Node
    {
    private:
        std::unique_ptr<WindowPartitionNode> m_partition;  // PARTITION BY (may be nullptr)
        std::unique_ptr<OrderByNode> m_orderBy;            // ORDER BY (may be nullptr)
        std::unique_ptr<WindowFrameNode> m_frame;          // frame clause (may be nullptr)

    public:
        WindowSpecNode(std::unique_ptr<WindowPartitionNode> partition,
                       std::unique_ptr<OrderByNode> orderBy,
                       std::unique_ptr<WindowFrameNode> frame)
            : Node(Kind::WindowSpec)
            , m_partition(std::move(partition))
            , m_orderBy(std::move(orderBy))
            , m_frame(std::move(frame)) {}

        WindowPartitionNode const* GetPartition() const { return m_partition.get(); }
        OrderByNode const* GetOrderBy() const { return m_orderBy.get(); }
        WindowFrameNode const* GetFrame() const { return m_frame.get(); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

struct WindowDefNode final : Node
    {
    private:
        Utf8String m_name;
        std::unique_ptr<WindowSpecNode> m_spec;

    public:
        WindowDefNode(Utf8StringCR name, std::unique_ptr<WindowSpecNode> spec)
            : Node(Kind::WindowDef), m_name(name), m_spec(std::move(spec)) {}

        Utf8StringCR GetName() const { return m_name; }
        WindowSpecNode const& GetSpec() const { return *m_spec; }

        ECSqlStatus Accept(IVisitor& v) const override { return ECSqlStatus::Success; }
        void ToECSql(Utf8StringR out) const override;
    };

struct WindowDefListNode final : Node
    {
    private:
        std::vector<std::unique_ptr<WindowDefNode>> m_defs;

    public:
        explicit WindowDefListNode(std::vector<std::unique_ptr<WindowDefNode>> defs)
            : Node(Kind::WindowDefList), m_defs(std::move(defs)) {}

        std::vector<std::unique_ptr<WindowDefNode>> const& GetDefs() const { return m_defs; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! Window function: func(...) FILTER(...) OVER (window_spec).
//=======================================================================================
struct WindowFuncNode final : ExprNode
    {
    private:
        ExprNodePtr m_funcExpr;              // the function call
        ExprNodePtr m_filterWhere;           // FILTER (WHERE ...) — may be nullptr
        std::unique_ptr<WindowSpecNode> m_windowSpec;

    public:
        WindowFuncNode(ExprNodePtr funcExpr, ExprNodePtr filterWhere, std::unique_ptr<WindowSpecNode> windowSpec)
            : ExprNode(Kind::WindowFunc)
            , m_funcExpr(std::move(funcExpr)), m_filterWhere(std::move(filterWhere))
            , m_windowSpec(std::move(windowSpec)) {}

        ExprNode const& GetFuncExpr() const { return *m_funcExpr; }
        ExprNode const* GetFilterWhere() const { return m_filterWhere.get(); }
        WindowSpecNode const& GetWindowSpec() const { return *m_windowSpec; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
// ── Options ──────────────────────────────────────────────────────────────────────────
//=======================================================================================

struct OptionNode final : Node
    {
    private:
        Utf8String m_name;
        Utf8String m_value;

    public:
        OptionNode(Utf8StringCR name, Utf8StringCR value)
            : Node(Kind::Option), m_name(name), m_value(value) {}

        Utf8StringCR GetName() const { return m_name; }
        Utf8StringCR GetValue() const { return m_value; }

        ECSqlStatus Accept(IVisitor& v) const override { return ECSqlStatus::Success; }
        void ToECSql(Utf8StringR out) const override;
    };

struct OptionsNode final : Node
    {
    private:
        std::vector<std::unique_ptr<OptionNode>> m_options;

    public:
        OptionsNode() : Node(Kind::Options) {}
        explicit OptionsNode(std::vector<std::unique_ptr<OptionNode>> options)
            : Node(Kind::Options), m_options(std::move(options)) {}

        std::vector<std::unique_ptr<OptionNode>> const& GetOptions() const { return m_options; }
        void AddOption(std::unique_ptr<OptionNode> opt) { m_options.push_back(std::move(opt)); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
// ── DML Helpers ──────────────────────────────────────────────────────────────────────
//=======================================================================================

//! Assignment in UPDATE SET: propRef = expr.
struct AssignmentNode final : Node
    {
    private:
        std::unique_ptr<PropRefNode> m_property;
        ExprNodePtr m_value;

    public:
        AssignmentNode(std::unique_ptr<PropRefNode> property, ExprNodePtr value)
            : Node(Kind::Assignment), m_property(std::move(property)), m_value(std::move(value)) {}

        PropRefNode const& GetProperty() const { return *m_property; }
        ExprNode const& GetValue() const { return *m_value; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//! Property name list for INSERT (col1, col2, ...).
struct PropNameListNode final : Node
    {
    private:
        std::vector<std::unique_ptr<PropRefNode>> m_props;

    public:
        PropNameListNode() : Node(Kind::PropNameList) {}
        explicit PropNameListNode(std::vector<std::unique_ptr<PropRefNode>> props)
            : Node(Kind::PropNameList), m_props(std::move(props)) {}

        std::vector<std::unique_ptr<PropRefNode>> const& GetProps() const { return m_props; }
        void Add(std::unique_ptr<PropRefNode> prop) { m_props.push_back(std::move(prop)); }

        ECSqlStatus Accept(IVisitor& v) const override { return ECSqlStatus::Success; }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
// ── Query Nodes ──────────────────────────────────────────────────────────────────────
//=======================================================================================

//=======================================================================================
//! A single SELECT (no UNION/INTERSECT/EXCEPT).
//=======================================================================================
struct SingleSelectNode final : Node
    {
    private:
        SqlSetQuantifier m_quantifier;
        std::vector<std::unique_ptr<SelectItemNode>> m_selectItems;
        std::unique_ptr<FromNode> m_from;             // may be null for VALUES constructor
        std::unique_ptr<WhereNode> m_where;
        std::unique_ptr<GroupByNode> m_groupBy;
        std::unique_ptr<HavingNode> m_having;
        std::unique_ptr<OrderByNode> m_orderBy;
        std::unique_ptr<LimitOffsetNode> m_limitOffset;
        std::unique_ptr<WindowDefListNode> m_windowDefs;
        std::unique_ptr<OptionsNode> m_options;

    public:
        SingleSelectNode(SqlSetQuantifier quantifier,
                         std::vector<std::unique_ptr<SelectItemNode>> selectItems,
                         std::unique_ptr<FromNode> from,
                         std::unique_ptr<WhereNode> where,
                         std::unique_ptr<GroupByNode> groupBy,
                         std::unique_ptr<HavingNode> having,
                         std::unique_ptr<OrderByNode> orderBy,
                         std::unique_ptr<LimitOffsetNode> limitOffset,
                         std::unique_ptr<WindowDefListNode> windowDefs,
                         std::unique_ptr<OptionsNode> options)
            : Node(Kind::SingleSelect)
            , m_quantifier(quantifier)
            , m_selectItems(std::move(selectItems))
            , m_from(std::move(from)), m_where(std::move(where))
            , m_groupBy(std::move(groupBy)), m_having(std::move(having))
            , m_orderBy(std::move(orderBy)), m_limitOffset(std::move(limitOffset))
            , m_windowDefs(std::move(windowDefs)), m_options(std::move(options)) {}

        SqlSetQuantifier GetQuantifier() const { return m_quantifier; }
        std::vector<std::unique_ptr<SelectItemNode>> const& GetSelectItems() const { return m_selectItems; }
        FromNode const* GetFrom() const { return m_from.get(); }
        WhereNode const* GetWhere() const { return m_where.get(); }
        GroupByNode const* GetGroupBy() const { return m_groupBy.get(); }
        HavingNode const* GetHaving() const { return m_having.get(); }
        OrderByNode const* GetOrderBy() const { return m_orderBy.get(); }
        LimitOffsetNode const* GetLimitOffset() const { return m_limitOffset.get(); }
        WindowDefListNode const* GetWindowDefs() const { return m_windowDefs.get(); }
        OptionsNode const* GetOptions() const { return m_options.get(); }
        bool IsRowConstructor() const { return m_from == nullptr; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! Compound SELECT: lhs UNION [ALL] | INTERSECT | EXCEPT rhs.
//=======================================================================================
struct CompoundSelectNode final : Node
    {
    enum class Op { Union, Intersect, Except };

    private:
        Op m_op;
        bool m_isAll;
        std::unique_ptr<SingleSelectNode> m_lhs;
        NodePtr m_rhs;  // SingleSelectNode or CompoundSelectNode

    public:
        CompoundSelectNode(Op op, bool isAll, std::unique_ptr<SingleSelectNode> lhs, NodePtr rhs)
            : Node(Kind::CompoundSelect), m_op(op), m_isAll(isAll)
            , m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

        Op GetOperator() const { return m_op; }
        bool IsAll() const { return m_isAll; }
        SingleSelectNode const& GetLhs() const { return *m_lhs; }
        Node const& GetRhs() const { return *m_rhs; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
// ── Statement Nodes ──────────────────────────────────────────────────────────────────
//=======================================================================================

//=======================================================================================
//! Top-level SELECT statement, optionally with CTE (WITH clause).
//=======================================================================================
struct SelectStmtNode final : Node
    {
    private:
        std::unique_ptr<CteNode> m_cte;   // WITH clause (may be nullptr)
        NodePtr m_query;                   // SingleSelectNode or CompoundSelectNode

    public:
        SelectStmtNode(std::unique_ptr<CteNode> cte, NodePtr query)
            : Node(Kind::SelectStmt), m_cte(std::move(cte)), m_query(std::move(query)) {}

        CteNode const* GetCte() const { return m_cte.get(); }
        Node const& GetQuery() const { return *m_query; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! INSERT INTO classRef (props) VALUES (exprs)  or  INSERT INTO classRef (props) SELECT ...
//=======================================================================================
struct InsertStmtNode final : Node
    {
    private:
        std::unique_ptr<ClassRefNode> m_classRef;
        std::unique_ptr<PropNameListNode> m_propNames;
        std::vector<ExprNodePtr> m_values;                 // populated if VALUES(...)
        std::unique_ptr<SelectStmtNode> m_selectQuery;     // populated if INSERT...SELECT

    public:
        InsertStmtNode(std::unique_ptr<ClassRefNode> classRef,
                       std::unique_ptr<PropNameListNode> propNames,
                       std::vector<ExprNodePtr> values)
            : Node(Kind::InsertStmt)
            , m_classRef(std::move(classRef)), m_propNames(std::move(propNames))
            , m_values(std::move(values)) {}

        InsertStmtNode(std::unique_ptr<ClassRefNode> classRef,
                       std::unique_ptr<PropNameListNode> propNames,
                       std::unique_ptr<SelectStmtNode> selectQuery)
            : Node(Kind::InsertStmt)
            , m_classRef(std::move(classRef)), m_propNames(std::move(propNames))
            , m_selectQuery(std::move(selectQuery)) {}

        ClassRefNode const& GetClassRef() const { return *m_classRef; }
        PropNameListNode const& GetPropNames() const { return *m_propNames; }
        bool IsInsertSelect() const { return m_selectQuery != nullptr; }
        std::vector<ExprNodePtr> const& GetValues() const { return m_values; }
        SelectStmtNode const& GetSelectQuery() const { BeAssert(IsInsertSelect()); return *m_selectQuery; }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! UPDATE classRef SET assignments [WHERE condition] [ECSQLOPTIONS ...].
//=======================================================================================
struct UpdateStmtNode final : Node
    {
    private:
        std::unique_ptr<ClassRefNode> m_classRef;
        std::vector<std::unique_ptr<AssignmentNode>> m_assignments;
        std::unique_ptr<WhereNode> m_where;
        std::unique_ptr<OptionsNode> m_options;

    public:
        UpdateStmtNode(std::unique_ptr<ClassRefNode> classRef,
                       std::vector<std::unique_ptr<AssignmentNode>> assignments,
                       std::unique_ptr<WhereNode> where,
                       std::unique_ptr<OptionsNode> options)
            : Node(Kind::UpdateStmt)
            , m_classRef(std::move(classRef)), m_assignments(std::move(assignments))
            , m_where(std::move(where)), m_options(std::move(options)) {}

        ClassRefNode const& GetClassRef() const { return *m_classRef; }
        std::vector<std::unique_ptr<AssignmentNode>> const& GetAssignments() const { return m_assignments; }
        WhereNode const* GetWhere() const { return m_where.get(); }
        OptionsNode const* GetOptions() const { return m_options.get(); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! DELETE FROM classRef [WHERE condition] [ECSQLOPTIONS ...].
//=======================================================================================
struct DeleteStmtNode final : Node
    {
    private:
        std::unique_ptr<ClassRefNode> m_classRef;
        std::unique_ptr<WhereNode> m_where;
        std::unique_ptr<OptionsNode> m_options;

    public:
        DeleteStmtNode(std::unique_ptr<ClassRefNode> classRef,
                       std::unique_ptr<WhereNode> where,
                       std::unique_ptr<OptionsNode> options)
            : Node(Kind::DeleteStmt)
            , m_classRef(std::move(classRef)), m_where(std::move(where)), m_options(std::move(options)) {}

        ClassRefNode const& GetClassRef() const { return *m_classRef; }
        WhereNode const* GetWhere() const { return m_where.get(); }
        OptionsNode const* GetOptions() const { return m_options.get(); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

//=======================================================================================
//! PRAGMA name [= value] or PRAGMA name(value).
//=======================================================================================
struct PragmaStmtNode final : Node
    {
    private:
        Utf8String m_name;
        PragmaVal m_value;
        bool m_isReadValue;
        std::vector<Utf8String> m_pathTokens;
        std::unique_ptr<OptionsNode> m_options;

    public:
        PragmaStmtNode(Utf8StringCR name, PragmaVal value, bool isReadValue,
                       std::vector<Utf8String> pathTokens, std::unique_ptr<OptionsNode> options)
            : Node(Kind::PragmaStmt), m_name(name), m_value(std::move(value))
            , m_isReadValue(isReadValue), m_pathTokens(std::move(pathTokens))
            , m_options(std::move(options)) {}

        Utf8StringCR GetName() const { return m_name; }
        PragmaVal const& GetValue() const { return m_value; }
        bool IsReadValue() const { return m_isReadValue; }
        std::vector<Utf8String> const& GetPathTokens() const { return m_pathTokens; }
        OptionsNode const* GetOptions() const { return m_options.get(); }

        ECSqlStatus Accept(IVisitor& v) const override { return v._Visit(*this); }
        void ToECSql(Utf8StringR out) const override;
    };

} // namespace ECSqlAst

END_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlParser.h"
#include "ECSqlLexer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//! Hand-written recursive-descent replacement for the Bison/Flex ECSqlParser.
//! Produces the exact same std::unique_ptr<Exp> tree.
// @bsiclass
struct ECSqlRDParser final
    {
private:
    std::shared_ptr<ECSqlParseContext> m_context;
    ECSqlLexer* m_lexer = nullptr;  // non-owning, valid only during Parse()
    ECSqlToken m_current;           // current lookahead token
    bool m_suppressErrors = false;  // when true, ECSQLERR is silenced (speculative parsing)
    Utf8CP m_ecsql = nullptr;       // ECSql string being parsed, for error message prefix

    // Token stream helpers
    ECSqlToken const& Cur() const { return m_current; }
    bool At(ECSqlTokenType t) const { return m_current.type == t; }
    bool AtEnd() const { return m_current.IsEnd(); }
    ECSqlToken Advance();
    bool Expect(ECSqlTokenType t);
    bool TryConsume(ECSqlTokenType t);

    IssueDataSource const& Issues() const { return m_context->Issues(); }

    //! Strip surrounding single quotes from a String token text and unescape '' → '
    //! The ECSqlLexer returns the full token including quotes (e.g., 'hello')
    //! but the old Flex lexer returned just the content (hello). LiteralValueExp expects unquoted content.
    static Utf8String StripStringQuotes(Utf8StringCR raw)
        {
        if (raw.size() >= 2 && raw.front() == '\'' && raw.back() == '\'')
            {
            Utf8String inner = raw.substr(1, raw.size() - 2);
            inner.ReplaceAll("''", "'");
            return inner;
            }
        return raw;
        }

    // Statement parsers
    BentleyStatus ParseInsertStatement(std::unique_ptr<InsertStatementExp>&);
    BentleyStatus ParseUpdateStatementSearched(std::unique_ptr<UpdateStatementExp>&);
    BentleyStatus ParseDeleteStatementSearched(std::unique_ptr<DeleteStatementExp>&);
    BentleyStatus ParseSelectStatement(std::unique_ptr<SelectStatementExp>&);
    BentleyStatus ParseSingleSelectStatement(std::unique_ptr<SingleSelectStatementExp>&);
    BentleyStatus ParsePragmaStatement(std::unique_ptr<PragmaStatementExp>&);
    BentleyStatus ParseCTE(std::unique_ptr<CommonTableExp>&);
    BentleyStatus ParseCTEBlock(std::unique_ptr<CommonTableBlockExp>&, bool isRecursive);

    // Clause parsers
    BentleyStatus ParseSelection(std::unique_ptr<SelectClauseExp>&);
    BentleyStatus ParseDerivedColumn(std::unique_ptr<DerivedPropertyExp>&);
    BentleyStatus ParseFromClause(std::unique_ptr<FromExp>&);
    BentleyStatus ParseWhereClause(std::unique_ptr<WhereExp>&);
    BentleyStatus ParseGroupByClause(std::unique_ptr<GroupByExp>&);
    BentleyStatus ParseHavingClause(std::unique_ptr<HavingExp>&);
    BentleyStatus ParseOrderByClause(std::unique_ptr<OrderByExp>&);
    BentleyStatus ParseLimitOffsetClause(std::unique_ptr<LimitOffsetExp>&);
    BentleyStatus ParseOptECSqlOptionsClause(std::unique_ptr<OptionsExp>&);
    BentleyStatus ParseWindowClause(std::unique_ptr<WindowFunctionClauseExp>&);

    // Table reference parsers
    BentleyStatus ParseTableRef(std::unique_ptr<ClassRefExp>&, ECSqlType);
    BentleyStatus ParseJoinedTable(std::unique_ptr<JoinExp>&, std::unique_ptr<ClassRefExp> lhs, ECSqlType);
    BentleyStatus ParseJoinSpec(std::unique_ptr<JoinSpecExp>&);
    BentleyStatus ParseJoinType(ECSqlJoinType&);
    BentleyStatus ParseTableNode(std::unique_ptr<ClassNameExp>&, ECSqlType, PolymorphicInfo, bool isInsideTypePredicate = false);
    BentleyStatus ParseTableNodeWithOptMemberCall(std::unique_ptr<ClassNameExp>&, ECSqlType, PolymorphicInfo, bool disqualifyPrimaryJoin, bool isInsideTypePredicate = false);
    BentleyStatus ParsePolymorphicConstraint(PolymorphicInfo&);
    BentleyStatus ParseSubquery(std::unique_ptr<SubqueryExp>&);
    BentleyStatus ParseTableValuedFunction(std::unique_ptr<TableValuedFunctionExp>&, Utf8StringCR schemaName, Utf8StringCR functionName);

    // Value expression parsers
    BentleyStatus ParseValueExp(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseValueExpOr(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseValueExpAnd(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseValueExpBitOr(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseValueExpBitAnd(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseValueExpShift(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseValueExpAddSub(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseValueExpMulDiv(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseValueExpConcat(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseValueExpUnary(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseValueExpPrimary(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseValueExpCommalist(std::unique_ptr<ValueExpListExp>&);
    BentleyStatus ParseAssignmentCommalist(std::unique_ptr<AssignmentListExp>&);
    BentleyStatus ParseOptColumnRefCommalist(std::unique_ptr<PropertyNameListExp>&);
    BentleyStatus ParseColumnRefCommalist(std::unique_ptr<PropertyNameListExp>&);
    BentleyStatus ParseColumnRef(std::unique_ptr<ValueExp>&, bool forcePropertyNameExp);
    BentleyStatus ParseColumnRefAsPropertyNameExp(std::unique_ptr<PropertyNameExp>&);
    BentleyStatus ParseExpressionPath(std::unique_ptr<ValueExp>&, bool forceIntoPropertyNameExp);
    BentleyStatus ParseParameter(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseFctSpec(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseAggregateFct(std::unique_ptr<ValueExp>&, Utf8StringCR functionName);
    BentleyStatus ParseSetFct(std::unique_ptr<ValueExp>&, Utf8StringCR functionName, bool isStandardSetFunction);
    BentleyStatus ParseCastSpec(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseCaseExp(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseIIFExp(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseTypePredicate(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseValuesOrQuerySpec(std::vector<std::unique_ptr<ValueExp>>&);
    BentleyStatus ParseValuesCommalist(std::unique_ptr<SelectStatementExp>&);
    BentleyStatus ParseRowValueConstructorCommalist(std::vector<std::unique_ptr<ValueExp>>&);
    BentleyStatus ParseValueCreationFuncExp(std::unique_ptr<ValueExp>&);
    BentleyStatus ParseNavValueCreationFuncExp(std::unique_ptr<NavValueCreationFuncExp>&);

    // Boolean / predicate parsers
    BentleyStatus ParseSearchCondition(std::unique_ptr<BooleanExp>&);
    BentleyStatus ParseSearchConditionOr(std::unique_ptr<BooleanExp>&);
    BentleyStatus ParseSearchConditionAnd(std::unique_ptr<BooleanExp>&);
    BentleyStatus ParseSearchConditionNot(std::unique_ptr<BooleanExp>&);
    BentleyStatus ParsePredicate(std::unique_ptr<BooleanExp>&);
    BentleyStatus ParseInPredicate(std::unique_ptr<BooleanExp>&, std::unique_ptr<ValueExp> lhs, bool isNot);
    BentleyStatus ParseLikePredicate(std::unique_ptr<BooleanExp>&, std::unique_ptr<ValueExp> lhs, bool isNot);
    BentleyStatus ParseBetweenPredicate(std::unique_ptr<BooleanExp>&, std::unique_ptr<ValueExp> lhs, bool isNot);
    BentleyStatus ParseNullPredicate(std::unique_ptr<BooleanExp>&, std::unique_ptr<ValueExp> lhs);
    BentleyStatus ParseMatchPredicate(std::unique_ptr<BooleanExp>&, std::unique_ptr<ValueExp> lhs, bool isNot);
    BooleanSqlOperator ParseComparisonOp();
    bool IsComparisonOp() const;

    // Window function parsers
    BentleyStatus ParseWindowFunction(std::unique_ptr<ValueExp>&, std::unique_ptr<ValueExp> functionCallExp);
    BentleyStatus ParseWindowSpecification(std::unique_ptr<WindowSpecification>&);
    BentleyStatus ParseWindowPartitionClause(std::unique_ptr<WindowPartitionColumnReferenceListExp>&);
    BentleyStatus ParseWindowFrameClause(std::unique_ptr<WindowFrameClauseExp>&);
    BentleyStatus ParseWindowFrameStart(std::unique_ptr<WindowFrameStartExp>&);
    BentleyStatus ParseWindowFrameBetween(std::unique_ptr<WindowFrameBetweenExp>&);
    BentleyStatus ParseFirstWindowFrameBound(std::unique_ptr<FirstWindowFrameBoundExp>&);
    BentleyStatus ParseSecondWindowFrameBound(std::unique_ptr<SecondWindowFrameBoundExp>&);
    BentleyStatus ParseFilterClause(std::unique_ptr<FilterClauseExp>&);

    // Helpers
    BentleyStatus ParseLiteral(Utf8StringR literalVal, ECSqlTypeInfo& dataType);
    bool IsAggregateFunction(Utf8StringCR name) const;
    bool IsWindowFunctionName(Utf8StringCR name) const;
    bool IsJoinKeyword() const;

    // Internal helpers (not part of the public parsing API)
    BentleyStatus ParseTableRefAndOptJoins(std::unique_ptr<ClassRefExp>&);
    BentleyStatus ParseFctSpecByName(std::unique_ptr<ValueExp>&, Utf8StringCR functionName);
    BentleyStatus ParsePropertyPathInline(PropertyPath&);
    BentleyStatus ParsePragmaValue(PragmaVal& val);
    BentleyStatus ParseALLorONLY(PolymorphicInfo&);

    Utf8String TokenText() const { return m_current.GetText(); }
    Utf8String TokenTextUpper() const;

    struct ScopedContext final
        {
        ECSqlRDParser& m_parser;
        ScopedContext(ECSqlRDParser& p, ECDbCR ecdb, IssueDataSource const& issues, ECSqlRDParser const* parent) : m_parser(p)
            {
            if (parent)
                m_parser.m_context = parent->m_context;
            else
                m_parser.m_context = std::make_shared<ECSqlParseContext>(ecdb, issues);
            }
        ~ScopedContext() { m_parser.m_context = nullptr; }
        };

public:
    ECSqlRDParser() {}
    ~ECSqlRDParser() {}

    std::unique_ptr<Exp> Parse(ECDbCR, Utf8CP ecsql, IssueDataSource const&, ECSqlRDParser const* parentParser = nullptr);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once

#include "CommonTableExp.h"
#include "DeleteStatementExp.h"
#include "InsertStatementExp.h"
#include "PragmaStatementExp.h"
#include "SelectStatementExp.h"
#include "UpdateStatementExp.h"
#include "ValueCreationFuncExp.h"
#include "WindowFunctionExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlParseContext final {
   public:
    typedef bmap<ECN::ECClassId, ECN::ECClassCP> ClassListById;
    struct ParseArg {
        enum class Type {
            RangeClass,
            UnionOrderBy
        };

       private:
        Type m_type;

       protected:
        explicit ParseArg(Type type) : m_type(type) {}

       public:
        virtual ~ParseArg() {}
        Type GetType() const { return m_type; }
    };

    struct RangeClassArg final : ParseArg {
       private:
        std::vector<RangeClassInfo> const& m_arg;

       public:
        explicit RangeClassArg(std::vector<RangeClassInfo> const& arg) : ParseArg(Type::RangeClass), m_arg(arg) {}
        ~RangeClassArg() {};
        std::vector<RangeClassInfo> const& GetRangeClassInfos() const { return m_arg; }
    };

    struct UnionOrderByArg final : ParseArg {
       private:
        std::vector<SingleSelectStatementExp const*> const& m_arg;

       public:
        explicit UnionOrderByArg(std::vector<SingleSelectStatementExp const*> const& arg) : ParseArg(Type::UnionOrderBy), m_arg(arg) {}
        ~UnionOrderByArg() {}
        std::vector<SingleSelectStatementExp const*> const& GetUnionClauses() const { return m_arg; }
    };

    struct ClassViewPrepareStack final {
       private:
        ECSqlParseContext& m_ctx;

       public:
        ClassViewPrepareStack(ECSqlParseContext& ctx, ECN::ECClassCR viewClass);
        bool IsOnStack(ECN::ECClassCR viewClass) const;
        ~ClassViewPrepareStack();
        Utf8String GetStackAsString() const;
    };
    friend struct ClassViewPrepareStack;

   private:
    ECDbCR m_ecdb;
    IssueDataSource const& m_issues;
    bool m_deferFinalize;
    std::vector<std::unique_ptr<ParseArg>> m_finalizeParseArgs;
    bmap<Utf8String, std::shared_ptr<ClassNameExp::Info>, CompareIUtf8Ascii> m_classNameExpInfoList;
    int m_currentECSqlParameterIndex = 0;
    bvector<ParameterExp*> m_parameterExpList;
    bmap<Utf8CP, int, CompareIUtf8Ascii> m_ecsqlParameterNameToIndexMapping;
    int m_aliasCount = 0;
    std::vector<ECN::ECClassCP> m_viewPrepareStack;
    std::vector<Utf8String> m_attachedTableSpaceCache;
    bool m_isAttachedTableSpaceCacheSetup = false;

    std::vector<Utf8String> const& GetAttachedTableSpaces() {
        if (!m_isAttachedTableSpaceCacheSetup) {
            m_isAttachedTableSpaceCacheSetup = true;
            if (SUCCESS != DbUtilities::GetTableSpaces(m_attachedTableSpaceCache, m_ecdb, true)) {
                BeAssert(false);
            }
        }

        return m_attachedTableSpaceCache;
    }

   public:
    ECSqlParseContext(ECDbCR ecdb, IssueDataSource const& issues) : m_ecdb(ecdb), m_issues(issues), m_deferFinalize(false) {}
    BentleyStatus FinalizeParsing(Exp& rootExp);
    void PushArg(std::unique_ptr<ParseArg>);
    ParseArg const* CurrentArg() const;
    void PopArg();

    BentleyStatus TryResolveClass(std::shared_ptr<ClassNameExp::Info>& classMetaInfo, Utf8CP tableSpace, Utf8StringCR schemaNameOrAlias,
                                  Utf8StringCR className, ECSqlType, bool isPolymorphicExp, connectivity::OSQLParseNode const& node);
    BentleyStatus GetSubclasses(ClassListById& classes, ECN::ECClassCR ecClass);
    BentleyStatus GetConstraintClasses(ClassListById& classes, ECN::ECRelationshipConstraintCR constraintEnd);
    Utf8String GenerateAlias();
    int TrackECSqlParameter(ParameterExp& parameterExp);
    IssueDataSource const& Issues() const { return m_issues; }
    SchemaManager const& Schemas() const { return m_ecdb.Schemas(); }
    ECDbCR GetECDb() const { return m_ecdb; }
    void SetDeferFinalize(bool defer) { m_deferFinalize = defer; }
    bool GetDeferFinalize() const { return m_deferFinalize; }
};

//=======================================================================================
//! The name convention here is different as parse_<parser_rule>() style naming is used
//!  so that it easy to lookup rule corresponding to the function that parsing it.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlParser final {
   private:
    //=======================================================================================
    // Creates a context on construction and deletes the context on destruction
    // @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct ScopedContext final {
       private:
        ECSqlParser const& m_parser;

       public:
        ScopedContext(ECSqlParser const& parser, ECDbCR ecdb, IssueDataSource const& issues, const ECSqlParser* parentParser) : m_parser(parser) {
            if (parentParser)
                m_parser.m_context = parentParser->m_context;
            else
                m_parser.m_context = std::make_shared<ECSqlParseContext>(ecdb, issues);
        }

        ~ScopedContext() { m_parser.m_context = nullptr; }
    };

    mutable std::shared_ptr<ECSqlParseContext> m_context;

    // root nodes
    BentleyStatus ParseDeleteStatementSearched(std::unique_ptr<DeleteStatementExp>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseInsertStatement(std::unique_ptr<InsertStatementExp>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseSelectStatement(std::unique_ptr<SelectStatementExp>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseUpdateStatementSearched(std::unique_ptr<UpdateStatementExp>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParsePragmaStatement(std::unique_ptr<PragmaStatementExp>&, connectivity::OSQLParseNode const&) const;

    // Common expressions
    BentleyStatus ParseAllOrDistinctToken(SqlSetQuantifier&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseAllToken(bool& isAll, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseAnyOrAllOrSomeToken(SqlCompareListType&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseAscOrDescToken(OrderBySpecExp::SortDirection&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseAssignmentCommalist(std::unique_ptr<AssignmentListExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseCaseExp(std::unique_ptr<ValueExp>& valueExp, connectivity::OSQLParseNode const* parseNode) const;

    BentleyStatus ParseCastSpec(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseColumn(std::unique_ptr<PropertyNameExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseColumnRef(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*, bool forceIntoPropertyNameExp) const;
    BentleyStatus ParseColumnRefAsPropertyNameExp(std::unique_ptr<PropertyNameExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseColumnRefCommalist(std::unique_ptr<PropertyNameListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseComparison(BooleanSqlOperator&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseConcatenation(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseCrossUnion(std::unique_ptr<CrossJoinExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseDatetimeValueExp(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseDatetimeValueFct(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseDerivedColumn(std::unique_ptr<DerivedPropertyExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseECRelationshipJoin(std::unique_ptr<UsingRelationshipJoinExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseECSqlOption(std::unique_ptr<OptionExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseExpressionPath(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*, bool forcePropertyNameExp = false) const;

    BentleyStatus ParseFactor(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseFctSpec(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseFromClause(std::unique_ptr<FromExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseFunctionArg(std::unique_ptr<ValueExp>& exp, connectivity::OSQLParseNode const& argNode) const { return ParseResult(exp, &argNode); }
    BentleyStatus ParseAndAddFunctionArg(FunctionCallExp&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseAggregateFct(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseGroupByClause(std::unique_ptr<GroupByExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseCTEBlock(std::unique_ptr<CommonTableBlockExp>&, connectivity::OSQLParseNode const*, bool const&) const;
    BentleyStatus ParseCTE(std::unique_ptr<CommonTableExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseCommonTableBlockName(std::unique_ptr<CommonTableBlockNameExp>& exp, connectivity::OSQLParseNode const& tableNode) const;

    BentleyStatus ParseHavingClause(std::unique_ptr<HavingExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseInPredicate(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;
    //!@param[out] inOperator parsed IN operator, e.g. IN or NOT IN
    BentleyStatus ParseInPredicatePart2(std::unique_ptr<ComputedExp>&, BooleanSqlOperator& inOperator, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseJoinCondition(std::unique_ptr<JoinConditionExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseJoinedTable(std::unique_ptr<JoinExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseJoinSpec(std::unique_ptr<JoinSpecExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseJoinType(ECSqlJoinType&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseLikePredicate(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;
    //!@param[out] likeOperator parsed LIKE operator, e.g. LIKE or NOT LIKE
    BentleyStatus ParseLikePredicatePart2(std::unique_ptr<ComputedExp>&, BooleanSqlOperator& likeOperator, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseLimitOffsetClause(std::unique_ptr<LimitOffsetExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseLiteral(Utf8StringR literalVal, ECSqlTypeInfo& dataType, connectivity::OSQLParseNode const&) const;

    BentleyStatus ParseMemberFunctionCall(std::unique_ptr<MemberFunctionCallExp>&, connectivity::OSQLParseNode const&, bool) const;

    BentleyStatus ParseNamedColumnsJoin(std::unique_ptr<NamedPropertiesJoinExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseNotToken(bool& isNot, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseNumValueExp(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseTermAddSub(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseOptColumnRefCommalist(std::unique_ptr<PropertyNameListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseOptECSqlOptionsClause(std::unique_ptr<OptionsExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseOrderByClause(std::unique_ptr<OrderByExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseOuterJoinType(ECSqlJoinType&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseParameter(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseQualifiedJoin(std::unique_ptr<JoinExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseResult(std::unique_ptr<ValueExp>& exp, connectivity::OSQLParseNode const* node) const { return ParseValueExp(exp, node); }
    BentleyStatus ParseRowValueConstructor(std::unique_ptr<ValueExp>& exp, connectivity::OSQLParseNode const* node) const { return ParseValueExp(exp, node); }
    BentleyStatus ParseRowValueConstructorCommalist(std::vector<std::unique_ptr<ValueExp>>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseRTreeMatchPredicate(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseSelectCompoundOperator(SelectStatementExp::CompoundOperator&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseSelection(std::unique_ptr<SelectClauseExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseSingleSelectStatement(std::unique_ptr<SingleSelectStatementExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseSubquery(std::unique_ptr<SubqueryExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseTableNode(std::unique_ptr<ClassNameExp>&, connectivity::OSQLParseNode const& tableNode, ECSqlType, PolymorphicInfo) const;
    BentleyStatus ParseTableNodeRef(std::unique_ptr<ClassRefExp>&, connectivity::OSQLParseNode const&, ECSqlType) const;
    BentleyStatus ParseTableNodeWithOptMemberCall(std::unique_ptr<ClassNameExp>&, connectivity::OSQLParseNode const&, ECSqlType, PolymorphicInfo polymorphic, bool disqualifyPrimaryJoin) const;
    BentleyStatus ParseTableRef(std::unique_ptr<ClassRefExp>&, connectivity::OSQLParseNode const*, ECSqlType ecsqlType) const;
    BentleyStatus ParseTerm(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseTruthValue(std::unique_ptr<ValueExp>& exp, connectivity::OSQLParseNode const* node) const { return ParseValueExp(exp, node); }

    BentleyStatus ParseSearchCondition(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseSetFct(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const&, Utf8StringCR functionName, bool isStandardSetFunction) const;

    BentleyStatus ParseUnaryPredicate(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseValueExp(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseValueExpCommalist(std::unique_ptr<ValueExpListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseValueExpPrimary(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseValuesCommalist(std::unique_ptr<SelectStatementExp>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseValuesOrQuerySpec(std::vector<std::unique_ptr<ValueExp>>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseWindowFunction(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseWhereClause(std::unique_ptr<WhereExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseTableValuedFunction(std::unique_ptr<TableValuedFunctionExp>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseIIFExp(std::unique_ptr<ValueExp>& valueExp, connectivity::OSQLParseNode const* parseNode) const;
    BentleyStatus ParseTypePredicate(std::unique_ptr<ValueExp>& valueExp, connectivity::OSQLParseNode const* parseNode) const;
    BentleyStatus ParseWindowClause(std::unique_ptr<WindowFunctionClauseExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseWindowDefinitionListExp(std::unique_ptr<WindowDefinitionListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseWindowDefinitionExp(std::unique_ptr<WindowDefinitionExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseWindowFunctionType(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseArgumentlessWindowFunction(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseNtileFunction(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseLeadOrLagFunction(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseOptLeadOrLagFunctionArguments(std::unique_ptr<FunctionCallExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseFirstOrLastValueFunction(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseNthValueFunction(std::unique_ptr<ValueExp>& exp, connectivity::OSQLParseNode const* parseNode) const;
    BentleyStatus ParseWindowSpecification(std::unique_ptr<WindowSpecification>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseWindowPartitionClause(std::unique_ptr<WindowPartitionColumnReferenceListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseWindowPartitionColumnRef(std::unique_ptr<WindowPartitionColumnReferenceExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseCollateClause(WindowPartitionColumnReferenceExp::CollateClauseFunction&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseFilterClause(std::unique_ptr<FilterClauseExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseWindowFrameClause(std::unique_ptr<WindowFrameClauseExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseWindowFrameUnit(WindowFrameClauseExp::WindowFrameUnit&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseWindowFrameStart(std::unique_ptr<WindowFrameStartExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseWindowFrameExclusion(WindowFrameClauseExp::WindowFrameExclusionType&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseWindowFrameBetween(std::unique_ptr<WindowFrameBetweenExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseFirstWindowFrameBound(std::unique_ptr<FirstWindowFrameBoundExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseSecondWindowFrameBound(std::unique_ptr<SecondWindowFrameBoundExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseValueCreationFuncExp(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseNavValueCreationFuncExp(std::unique_ptr<NavValueCreationFuncExp>&, connectivity::OSQLParseNode const*) const;

    static BentleyStatus ParsePolymorphicConstraint(PolymorphicInfo& constraint, connectivity::OSQLParseNode const* parseNode);
    static BentleyStatus ParseALLorONLY(PolymorphicInfo& constraint, connectivity::OSQLParseNode const* parseNode);
    IssueDataSource const& Issues() const {
        BeAssert(m_context != nullptr);
        return m_context->Issues();
    }
    static bool IsPredicate(connectivity::OSQLParseNode const&);
    static Utf8CP SqlDataTypeKeywordToString(sal_uInt32 sqlKeywordId);

   public:
    ECSqlParser() {}
    ~ECSqlParser() {}

    std::unique_ptr<Exp> Parse(ECDbCR, Utf8CP ecsql, IssueDataSource const&, const ECSqlParser* parentParser = nullptr) const;
};

END_BENTLEY_SQLITE_EC_NAMESPACE

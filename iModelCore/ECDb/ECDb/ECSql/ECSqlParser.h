/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlParser.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "SelectStatementExp.h"
#include "InsertStatementExp.h"
#include "UpdateStatementExp.h"
#include "DeleteStatementExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle     04/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlParseContext final
    {
public:
    typedef bmap<ECN::ECClassId, ECN::ECClassCP> ClassListById;

    struct ParseArg
        {
        enum class Type
            {
            RangeClass,
            UnionOrderBy
            };

        private:
            Type m_type;

        protected:
            explicit ParseArg(Type type) :m_type(type) {}

        public:
            virtual ~ParseArg() {}
            Type GetType() const { return m_type; }
        };

    struct RangeClassArg final : ParseArg
        {
        private:
            std::vector<RangeClassInfo> const& m_arg;

        public:
            explicit RangeClassArg(std::vector<RangeClassInfo> const& arg) : ParseArg(Type::RangeClass), m_arg(arg) {}
            ~RangeClassArg() {};
            std::vector<RangeClassInfo> const& GetRangeClassInfos() const { return m_arg; }
        };

    struct UnionOrderByArg final : ParseArg
        {
        private:
            std::vector<SingleSelectStatementExp const*> const& m_arg;
        public:
            explicit UnionOrderByArg(std::vector<SingleSelectStatementExp const*> const& arg) : ParseArg(Type::UnionOrderBy), m_arg(arg) {}
            ~UnionOrderByArg() {}
            std::vector<SingleSelectStatementExp const*> const& GetUnionClauses() const { return m_arg; }
        };

private:
    ECDbCR m_ecdb;
    ScopedIssueReporter const& m_issues;

    std::vector<std::unique_ptr<ParseArg>> m_finalizeParseArgs;
    bmap<Utf8String, std::shared_ptr<ClassNameExp::Info>, CompareIUtf8Ascii> m_classNameExpInfoList;
    int m_currentECSqlParameterIndex = 0;
    bvector<ParameterExp*> m_parameterExpList;
    bmap<Utf8CP, int, CompareIUtf8Ascii> m_ecsqlParameterNameToIndexMapping;
    int m_aliasCount = 0;

    std::vector<Utf8String> m_attachedTableSpaceCache;
    bool m_isAttachedTableSpaceCacheSetup = false;

    std::vector<Utf8String> const& GetAttachedTableSpaces() 
        {
        if (!m_isAttachedTableSpaceCacheSetup)
            {
            m_isAttachedTableSpaceCacheSetup = true;
            if (SUCCESS != DbUtilities::GetTableSpaces(m_attachedTableSpaceCache, m_ecdb, true))
                {
                BeAssert(false);
                }
            }

        return m_attachedTableSpaceCache;
        }
public:
    ECSqlParseContext(ECDbCR ecdb, ScopedIssueReporter const& issues) : m_ecdb(ecdb), m_issues(issues) {}
    BentleyStatus FinalizeParsing(Exp& rootExp);
    void PushArg(std::unique_ptr<ParseArg>);
    ParseArg const* CurrentArg() const;
    void PopArg();

    BentleyStatus TryResolveClass(std::shared_ptr<ClassNameExp::Info>& classMetaInfo, Utf8CP tableSpace, Utf8StringCR schemaNameOrAlias, Utf8StringCR className, ECSqlType, bool isPolymorphicExp);
    BentleyStatus GetSubclasses(ClassListById& classes, ECN::ECClassCR ecClass);
    BentleyStatus GetConstraintClasses(ClassListById& classes, ECN::ECRelationshipConstraintCR constraintEnd);
    Utf8String GenerateAlias();
    int TrackECSqlParameter(ParameterExp& parameterExp);
    ScopedIssueReporter const& Issues() const { return m_issues; }
    SchemaManager const& Schemas() const { return m_ecdb.Schemas(); }
    ECDbCR GetECDb() const { return m_ecdb; }
    };

//=======================================================================================
//!The name convention here is different as parse_<parser_rule>() style naming is used 
//! so that it easy to lookup rule corresponding to the function that parsing it.
// @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlParser final
    {
private:
    //=======================================================================================
    // Creates a context on construction and deletes the context on destruction
    // @bsiclass                                                Krischan.Eberle     09/2015
    //+===============+===============+===============+===============+===============+======
    struct ScopedContext final
        {
    private:
        ECSqlParser const& m_parser;
    public:
        ScopedContext(ECSqlParser const& parser, ECDbCR ecdb, ScopedIssueReporter const& issues) : m_parser(parser)
            {
            m_parser.m_context = std::unique_ptr<ECSqlParseContext>(new ECSqlParseContext(ecdb, issues));
            }

        ~ScopedContext() { m_parser.m_context = nullptr; }
        };


    mutable std::unique_ptr<ECSqlParseContext> m_context;
    
    //root nodes
    BentleyStatus ParseDeleteStatementSearched(std::unique_ptr<DeleteStatementExp>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseInsertStatement(std::unique_ptr<InsertStatementExp>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseSelectStatement(std::unique_ptr<SelectStatementExp>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseUpdateStatementSearched(std::unique_ptr<UpdateStatementExp>&, connectivity::OSQLParseNode const&) const;

    //Common expressions
    BentleyStatus ParseAllOrDistinctToken(SqlSetQuantifier&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseAllToken(bool& isAll, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseAnyOrAllOrSomeToken(SqlCompareListType&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseAscOrDescToken(OrderBySpecExp::SortDirection&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseAssignmentCommalist(std::unique_ptr<AssignmentListExp>&, connectivity::OSQLParseNode const*) const;

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

    BentleyStatus ParseECRelationshipJoin(std::unique_ptr<ECRelationshipJoinExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseECSqlOption(std::unique_ptr<OptionExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseExpressionPath(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*, bool forcePropertyNameExp = false) const;

    BentleyStatus ParseFactor(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseFctSpec(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseFromClause(std::unique_ptr<FromExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseFunctionArg(std::unique_ptr<ValueExp>& exp, connectivity::OSQLParseNode const& argNode) const { return ParseResult(exp, &argNode); }
    BentleyStatus ParseAndAddFunctionArg(FunctionCallExp&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseGeneralSetFct(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseGroupByClause(std::unique_ptr<GroupByExp>&, connectivity::OSQLParseNode const*) const;

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

    BentleyStatus ParseMemberFunctionCall(std::unique_ptr<MemberFunctionCallExp>&, connectivity::OSQLParseNode const&) const;

    BentleyStatus ParseNamedColumnsJoin(std::unique_ptr<NamedPropertiesJoinExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseNotToken(bool& isNot, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseNumValueExp(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;

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

    BentleyStatus ParseTableNode(std::unique_ptr<ClassNameExp>&, connectivity::OSQLParseNode const& tableNode, ECSqlType) const;
    BentleyStatus ParseTableNodeWithOptMemberCall(std::unique_ptr<ClassNameExp>&, connectivity::OSQLParseNode const&, ECSqlType, bool isPolymorphic) const;
    BentleyStatus ParseTableRef(std::unique_ptr<ClassRefExp>&, connectivity::OSQLParseNode const*, ECSqlType ecsqlType) const;
    BentleyStatus ParseTerm(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseTruthValue(std::unique_ptr<ValueExp>& exp, connectivity::OSQLParseNode const* node) const { return ParseValueExp(exp, node); }

    BentleyStatus ParseSearchCondition(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseSetFct(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const&, Utf8StringCR functionName, bool isStandardSetFunction) const;

    BentleyStatus ParseUnaryPredicate(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseValueExp(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseValueExpCommalist(std::unique_ptr<ValueExpListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseValueExpPrimary(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseValuesOrQuerySpec(std::vector<std::unique_ptr<ValueExp>>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseWhereClause(std::unique_ptr<WhereExp>&, connectivity::OSQLParseNode const*) const;

    ScopedIssueReporter const& Issues() const { BeAssert(m_context != nullptr); return m_context->Issues(); }
    static bool IsPredicate(connectivity::OSQLParseNode const&);
    static Utf8CP SqlDataTypeKeywordToString(sal_uInt32 sqlKeywordId);

public:
    ECSqlParser() {}
    ~ECSqlParser() {}

    std::unique_ptr<Exp> Parse(ECDbCR, Utf8CP ecsql, ScopedIssueReporter const&) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

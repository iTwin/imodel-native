/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlParser.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
struct ECSqlParseContext
    {
public:
    typedef bmap<ECN::ECClassId, ECN::ECClassCP> ClassListById;

private:
    ECDbCR m_ecdb;

    std::vector<void const*> m_finalizeParseArgs;
    bmap<Utf8CP, std::shared_ptr<ClassNameExp::Info>, CompareIUtf8Ascii> m_classNameExpInfoList;
    int m_currentECSqlParameterIndex;
    bvector<ParameterExp*> m_parameterExpList;
    bmap<Utf8CP, int, CompareIUtf8Ascii> m_ecsqlParameterNameToIndexMapping;
    int m_aliasCount;

public:
    explicit ECSqlParseContext(ECDbCR ecdb) : m_ecdb(ecdb), m_currentECSqlParameterIndex(0), m_aliasCount(0) {}

    BentleyStatus FinalizeParsing(Exp& rootExp);

    void PushFinalizeParseArg(void const* const arg);
    void const* const GetFinalizeParseArg() const;
    void PopFinalizeParseArg();

    BentleyStatus TryResolveClass(std::shared_ptr<ClassNameExp::Info>& classMetaInfo, Utf8CP schemaNameOrPrefix, Utf8CP className);
    void GetSubclasses(ClassListById& classes, ECN::ECClassCR ecClass);
    void GetConstraintClasses(ClassListById& classes, ECN::ECRelationshipConstraintCR constraintEnd, bool* containAnyClass);
    bool IsEndClassOfRelationship(ECN::ECClassCR searchClass, ECN::ECRelationshipEnd searchEnd, ECN::ECRelationshipClassCR relationshipClass);
    Utf8String GenerateAlias();

    int TrackECSqlParameter(ParameterExp& parameterExp);

    IssueReporter const& Issues() const { return m_ecdb.GetECDbImplR().GetIssueReporter(); }
    ECDbSchemaManagerCR Schemas() const { return m_ecdb.Schemas(); }
    ECDbCR GetECDb() const { return m_ecdb; }
    };

//=======================================================================================
//!The name convention here is different as parse_<parser_rule>() style naming is used 
//! so that it easy to lookup rule corresponding to the function that parsing it.
// @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlParser
    {
private:
    //=======================================================================================
    // Creates a context on construction and deletes the context on destruction
    // @bsiclass                                                Krischan.Eberle     09/2015
    //+===============+===============+===============+===============+===============+======
    struct ScopedContext
        {
    private:
        ECSqlParser const& m_parser;

    public:
        ScopedContext(ECSqlParser const& parser, ECDbCR ecdb) : m_parser(parser)
            {
            m_parser.m_context = std::unique_ptr<ECSqlParseContext>(new ECSqlParseContext(ecdb));
            }

        ~ScopedContext() { m_parser.m_context = nullptr; }
        };


    mutable std::unique_ptr<ECSqlParseContext> m_context;

    //No need to free this as it is a static member (See http://bsw-wiki.bentley.com/bin/view.pl/Main/CPlusPlusSpecific)
    static connectivity::OSQLParser* s_sharedParser;

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

    BentleyStatus ParseCatalogName(Utf8CP& catalogName, Utf8CP& schemaName, Utf8CP& className, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseCastSpec(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseColumn(std::unique_ptr<PropertyNameExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseColumnRef(std::unique_ptr<PropertyNameExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseColumnRefCommalist(std::unique_ptr<PropertyNameListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseComparison(BooleanSqlOperator&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseCompoundSelectOperator(SelectStatementExp::Operator&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseConcatenation(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseCrossUnion(std::unique_ptr<CrossJoinExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseDatetimeValueExp(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseDatetimeValueFct(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseDerivedColumn(std::unique_ptr<DerivedPropertyExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseECClassIdFctSpec(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseECRelationshipJoin(std::unique_ptr<ECRelationshipJoinExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseECSqlOption(std::unique_ptr<OptionExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseFactor(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseFctSpec(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseFold(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseFromClause(std::unique_ptr<FromExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseFunctionArg(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const&) const;
    BentleyStatus ParseAndAddFunctionArg(FunctionCallExp&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseGeneralSetFct(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseGetPointCoordinateFctSpec(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const&, Utf8StringCR functionName) const;
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

    BentleyStatus ParseNamedColumnsJoin(std::unique_ptr<NamedPropertiesJoinExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseNotToken(bool& isNot, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseNumValueExp(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseOptColumnRefCommalist(std::unique_ptr<PropertyNameListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseOptECSqlOptionsClause(std::unique_ptr<OptionsExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseOrderByClause(std::unique_ptr<OrderByExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseOuterJoinType(ECSqlJoinType&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseParameter(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParsePropertyPath(std::unique_ptr<PropertyNameExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseQualifiedJoin(std::unique_ptr<JoinExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseResult(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseRowValueConstructor(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseRowValueConstructorCommalist(std::unique_ptr<ValueExpListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseRTreeMatchPredicate(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseSelection(std::unique_ptr<SelectClauseExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseSingleSelectStatement(std::unique_ptr<SingleSelectStatementExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseSubquery(std::unique_ptr<SubqueryExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseTableName(Utf8CP& tableName, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseTableNode(std::unique_ptr<ClassNameExp>&, connectivity::OSQLParseNode const*, bool isPolymorphic) const;
    BentleyStatus ParseTableRef(std::unique_ptr<ClassRefExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseTerm(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseTruthValue(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus ParseSchemaName(Utf8CP& schemaName, Utf8CP& className, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseSearchCondition(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseSetFct(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const&, Utf8CP functionName, bool isStandardSetFunction) const;

    BentleyStatus ParseUnaryPredicate(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseValueExp(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseValueExpCommalist(std::unique_ptr<ValueExpListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseValueExpPrimary(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseValuesOrQuerySpec(std::unique_ptr<ValueExpListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus ParseWhereClause(std::unique_ptr<WhereExp>&, connectivity::OSQLParseNode const*) const;

    IssueReporter const& GetIssueReporter() const { BeAssert(m_context != nullptr); return m_context->Issues(); }

    static Utf8CP DataTypeTokenIdToString(sal_uInt32 tokenId);

    static bool IsPredicate(connectivity::OSQLParseNode const& parseNode);
    static connectivity::OSQLParser& GetSharedParser();

public:
    ECSqlParser() : m_context (nullptr) {}
    ~ECSqlParser() {}

    std::unique_ptr<Exp> Parse(ECDbCR, Utf8CP ecsql) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

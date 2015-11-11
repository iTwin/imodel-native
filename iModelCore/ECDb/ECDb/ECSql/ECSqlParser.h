/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlParser.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    IClassMap::View m_classMapViewMode;

    std::vector<void const*> m_finalizeParseArgs;
    bmap<Utf8String, std::shared_ptr<ClassNameExp::Info>> m_classNameExpInfoList;
    int m_currentECSqlParameterIndex;
    bvector<ParameterExp*> m_parameterExpList;
    bmap<Utf8CP, int, CompareUtf8> m_ecsqlParameterNameToIndexMapping;
    int m_aliasCount;

public:
    ECSqlParseContext(ECDbCR ecdb, IClassMap::View classMapViewMode) : m_ecdb(ecdb), m_classMapViewMode(classMapViewMode), m_currentECSqlParameterIndex(0), m_aliasCount(0) {}

    BentleyStatus FinalizeParsing(Exp& rootExp);

    void PushFinalizeParseArg(void const* const arg);
    void const* const GetFinalizeParseArg() const;
    void PopFinalizeParseArg();

    BentleyStatus TryResolveClass(std::shared_ptr<ClassNameExp::Info>& classMetaInfo, Utf8StringCR schemaNameOrPrefix, Utf8StringCR className);
    void GetSubclasses(ClassListById& classes, ECN::ECClassCR ecClass);
    void GetConstraintClasses(ClassListById& classes, ECN::ECRelationshipConstraintCR constraintEnd, bool* containAnyClass);
    bool IsEndClassOfRelationship(ECN::ECClassCR searchClass, ECN::ECRelationshipEnd searchEnd, ECN::ECRelationshipClassCR relationshipClass);
    Utf8String GenerateAlias();

    int TrackECSqlParameter(ParameterExp& parameterExp);

    IssueReporter const& GetIssueReporter() const { return m_ecdb.GetECDbImplR().GetIssueReporter(); }
    ECDbSchemaManagerCR Schemas() const { return m_ecdb.Schemas(); }
    ECDbCR GetECDb() const { return m_ecdb; }
    };

typedef std::unique_ptr<Exp> ECSqlParseTreePtr;
typedef Exp const& ECSqlParseTreeCR;

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
        ScopedContext(ECSqlParser const& parser, ECDbCR ecdb, IClassMap::View classMapViewMode) : m_parser(parser)
            {
            m_parser.m_context = std::unique_ptr<ECSqlParseContext>(new ECSqlParseContext(ecdb, classMapViewMode));
            }

        ~ScopedContext() { m_parser.m_context = nullptr; }
        };


    mutable std::unique_ptr<ECSqlParseContext> m_context;

    //WIP_ECSQL: Mem leaks where there is error in statement. Need to use shared_ptr instead of unique_ptr
    static connectivity::OSQLParser* GetSharedParser()
        {
        static std::unique_ptr<connectivity::OSQLParser> s_parser = std::unique_ptr<connectivity::OSQLParser>(new connectivity::OSQLParser(com::sun::star::lang::XMultiServiceFactory::CreateInstance()));
        return s_parser.get();
        }
    //root nodes
    BentleyStatus parse_select_statement(std::unique_ptr<SelectStatementExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_insert_statement(std::unique_ptr<InsertStatementExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_update_statement_searched(std::unique_ptr<UpdateStatementExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_delete_statement_searched(std::unique_ptr<DeleteStatementExp>&, connectivity::OSQLParseNode const*) const;

    // SELECT
    BentleyStatus parse_single_select_statement(std::unique_ptr<SingleSelectStatementExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_selection(std::unique_ptr<SelectClauseExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_derived_column(std::unique_ptr<DerivedPropertyExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_from_clause(std::unique_ptr<FromExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_assignment_commalist(std::unique_ptr<AssignmentListExp>&, connectivity::OSQLParseNode const*) const;

    //Common expressions
    BentleyStatus parse_all(bool& isAll, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_any_all_some(SqlCompareListType&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_catalog_name(Utf8CP& catalogName, Utf8CP& schemaName, Utf8CP& className, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_cast_spec(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_column(std::unique_ptr<PropertyNameExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_column_ref_commalist(std::unique_ptr<PropertyNameListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_column_ref(std::unique_ptr<PropertyNameExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_comparison(BooleanSqlOperator&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_concatenation(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_cross_union(std::unique_ptr<CrossJoinExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_datetime_value_exp(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_datetime_term(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_datetime_factor(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_datetime_primary(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_datetime_value_fct(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_ecclassid_fct_spec(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_ecsqloption(std::unique_ptr<OptionExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_fct_spec(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_and_add_functionarg(FunctionCallExp&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_factor(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_fold(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_general_set_fct(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_group_by_clause(std::unique_ptr<GroupByExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_having_clause(std::unique_ptr<HavingExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_in_predicate(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;
    //!@param[out] inOperator parsed IN operator, e.g. IN or NOT IN
    BentleyStatus parse_in_predicate_part_2(std::unique_ptr<ComputedExp>&, BooleanSqlOperator& inOperator, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_join_condition(std::unique_ptr<JoinConditionExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_joined_table(std::unique_ptr<JoinExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_join_spec(std::unique_ptr<JoinSpecExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_join_type(ECSqlJoinType&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_like_predicate(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;
    //!@param[out] likeOperator parsed LIKE operator, e.g. LIKE or NOT LIKE
    BentleyStatus parse_like_predicate_part_2(std::unique_ptr<ComputedExp>&, BooleanSqlOperator& likeOperator, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_limit_offset_clause(std::unique_ptr<LimitOffsetExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_literal(Utf8StringR literalVal, ECSqlTypeInfo& dataType, connectivity::OSQLParseNode const&) const;

    BentleyStatus parse_rtreematch_predicate(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_named_columns_join(std::unique_ptr<NamedPropertiesJoinExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_num_value_exp(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_opt_all_distinct(SqlSetQuantifier&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_opt_asc_desc(OrderBySpecExp::SortDirection&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_opt_column_ref_commalist(std::unique_ptr<PropertyNameListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_opt_ecsqloptions_clause(std::unique_ptr<OptionsExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_opt_where_clause(std::unique_ptr<WhereExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_order_by_clause(std::unique_ptr<OrderByExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_outer_join_type(ECSqlJoinType&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_parameter(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_qualified_join(std::unique_ptr<JoinExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_relationship_join(std::unique_ptr<RelationshipJoinExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_result(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_row_value_constructor(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_row_value_constructor_commalist(std::unique_ptr<ValueExpListExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_subquery(std::unique_ptr<SubqueryExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_table_name(Utf8CP& tableName, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_table_node(std::unique_ptr<ClassNameExp>&, connectivity::OSQLParseNode const*, bool isPolymorphic) const;
    BentleyStatus parse_table_ref(std::unique_ptr<ClassRefExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_term(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_trueth_value(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_schema_name(Utf8CP& schemaName, Utf8CP& className, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_search_condition(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_sql_not(bool& isNot, connectivity::OSQLParseNode const*) const;

    BentleyStatus parse_unary_predicate(std::unique_ptr<BooleanExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_compound_select_op(SelectStatementExp::Operator&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_value_exp(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_value_exp_commalist(std::unique_ptr<ValueExpListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_value_exp_primary(std::unique_ptr<ValueExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_values_or_query_spec(std::unique_ptr<ValueExpListExp>&, connectivity::OSQLParseNode const*) const;
    BentleyStatus parse_property_path(std::unique_ptr<PropertyNameExp>&, connectivity::OSQLParseNode const*) const;

    static bool IsPredicate(connectivity::OSQLParseNode const* parseNode);

    IssueReporter const& GetIssueReporter() const { BeAssert(m_context != nullptr); return m_context->GetIssueReporter(); }

public:
    ECSqlParser() : m_context (nullptr) {}
    ~ECSqlParser() {}

    BentleyStatus Parse(ECSqlParseTreePtr& ecsqlParseTree, ECDbCR, Utf8CP ecsql, IClassMap::View) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

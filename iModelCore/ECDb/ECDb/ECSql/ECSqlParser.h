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
#include "ECSqlParseContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

typedef std::shared_ptr<Exp> ECSqlParseTreePtr;
typedef Exp const& ECSqlParseTreeCR;

//=======================================================================================
//!The name convention here is different as parse_<parser_rule>() style naming is used 
//! so that it easy to lookup rule corresponding to the function that parsing it.
// @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlParser
    {
//WIP_ECSQL: Mem leaks where there is error in statement. Need to use shared_ptr instead of unique_ptr
private:
    //this a static class
    ECSqlParser ();
    ~ECSqlParser ();

    static std::unique_ptr<Exp>                        Parse                           (Utf8CP ecsql, ECSqlParseContext& parseContext);
    // SELECT
    static std::unique_ptr<SelectStatementExp>         parse_select_statement          (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<SelectClauseExp>            parse_selection                 (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<DerivedPropertyExp>         parse_derived_column            (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<FromExp>                    parse_from_clause               (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    
    
    // INSERT
    static std::unique_ptr<InsertStatementExp>         parse_insert_statement          (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    
    // UPDATE
    static std::unique_ptr<UpdateStatementExp>         parse_update_statement_searched (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<AssignmentListExp>          parse_assignment_commalist      (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    // DELETE
    static std::unique_ptr<DeleteStatementExp>         parse_delete_statement_searched (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    //Common expressions
    static bool                                        parse_all                       (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static SqlCompareListType                          parse_any_all_some              (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static Utf8CP                                      parse_catalog_name              (Utf8CP& schemaName, Utf8CP& className, ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<CastExp>                    parse_cast_spec                 (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<PropertyNameExp>            parse_column                    (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<PropertyNameListExp>        parse_column_ref_commalist      (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<PropertyNameExp>            parse_column_ref                (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static SqlBooleanOperator                          parse_comparison                (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ValueExp>                   parse_concatenation             (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<CrossJoinExp>               parse_cross_union               (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<ValueExp>                   parse_datetime_value_exp        (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ValueExp>                   parse_datetime_term             (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ValueExp>                   parse_datetime_factor           (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ConstantValueExp>           parse_datetime_primary          (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ConstantValueExp>           parse_datetime_value_fct        (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<ECClassIdFunctionExp>       parse_ecclassid_fct_spec        (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<FunctionCallExp>            parse_fct_spec                  (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static BentleyStatus                               parse_and_add_functionarg       (ECSqlParseContext& ctx, FunctionCallExp& functionCallExp, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<UnaryExp>                   parse_factor                    (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<FoldFunctionCallExp>      parse_fold                      (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<SetFunctionCallExp>         parse_general_set_fct           (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<GroupByExp>                 parse_group_by_clause           (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<HavingExp>                  parse_having_clause             (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<BooleanExp>                 parse_in_predicate              (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    //!@param[out] inOperator parsed IN operator, e.g. IN or NOT IN
    static std::unique_ptr<ComputedExp>                parse_in_predicate_part_2       (ECSqlParseContext& ctx, SqlBooleanOperator& inOperator, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<JoinConditionExp>           parse_join_condition            (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<JoinExp>                    parse_joined_table              (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<JoinSpecExp>                parse_join_spec                 (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static ECSqlJoinType                               parse_join_type                 (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<BooleanExp>                 parse_like_predicate            (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    //!@param[out] likeOperator parsed LIKE operator, e.g. LIKE or NOT LIKE
    static std::unique_ptr<ComputedExp>                parse_like_predicate_part_2     (ECSqlParseContext& ctx, SqlBooleanOperator& likeOperator, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<LimitOffsetExp>             parse_limit_offset_clause       (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<NamedPropertiesJoinExp>     parse_named_columns_join        (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<QueryExp>                   parse_non_join_query_exp        (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static NonJoinQueryOperator                        parse_non_join_query_operator   (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<QueryExp>                   parse_non_join_query_primary    (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<QueryExp>                   parse_non_join_query_term       (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ValueExp>                   parse_num_value_expr            (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static SqlSetQuantifier                            parse_opt_all_distinct          (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static OrderBySpecExp::SortDirection               parse_opt_asc_desc              (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<PropertyNameListExp>        parse_opt_column_ref_commalist  (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<WhereExp>                   parse_opt_where_clause(ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<OrderByExp>                 parse_order_by_clause           (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static ECSqlJoinType                               parse_outer_join_type           (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<ParameterExp>               parse_parameter                 (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<JoinExp>                    parse_qualified_join            (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<QueryExp>                   parse_query_exp                 (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<QueryExp>                   parse_query_term                (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<RelationshipJoinExp>        parse_relationship_join         (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ValueExp>                   parse_result                    (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ValueExp>                   parse_row_value_constructor     (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ValueExpListExp>            parse_row_value_constructor_commalist(ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<SubqueryExp>                parse_subquery                  (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static Utf8CP                                      parse_table_name                (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ClassNameExp>               parse_table_node                (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode, bool isPolymorphic);
    static std::unique_ptr<ClassRefExp>                parse_table_ref                 (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ValueExp>                   parse_term                      (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ValueExp>                   parse_trueth_value              (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static Utf8CP                                      parse_schema_name               (Utf8CP& className, ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<BooleanExp>                 parse_search_conditon           (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static bool                                        parse_sql_not                   (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<UnionStatementExp>          parse_union_statement           (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static std::unique_ptr<ValueExp>                   parse_value_exp                 (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ValueExpListExp>            parse_value_exp_commalist       (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ValueExp>                   parse_value_exp_primary         (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<ValueExpListExp>            parse_values_or_query_spec      (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);
    static std::unique_ptr<PropertyNameExp>            parse_property_path             (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode);

    static bool                                        isPredicate                     (connectivity::OSQLParseNode const* parseNode);

public:
    static void Parse (ECSqlParseTreePtr& ecsqlParseTree, ECSqlStatusContext& statusContext, ECDbCR db, Utf8CP ecsql, IClassMap::View classView);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

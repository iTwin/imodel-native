/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include "ECDbPch.h"
#include "ErrorCondition.h"
#include "SqlNode.h"
// #define YYBISON      1
// #ifndef BISON_INCLUDED
// #define BISON_INCLUDED
#include "SqlBison.h"
// #endif

#include "SqlParse.h"
#include "DataType.h"
#include "SqlScan.h"
#include <string.h>
#include <algorithm>
#include <functional>

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;
// using namespace ::osl;
using namespace ::dbtools;
using namespace ::comphelper;
extern int SQLyyparse(connectivity::OSQLParser*);

namespace connectivity {
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildPredicateRule(OSQLParseNode*& pAppend, OSQLParseNode* pLiteral, OSQLParseNode*& pCompare, OSQLParseNode* pLiteral2) {
    OSL_ENSURE(inPredicateCheck(), "Only in predicate check allowed!");
    sal_Int16 nErg = 0;
    if (m_xField.IsValid()) {
        TODO_ConvertCode();
    }
    if (!pCompare->getParent())  // I have no parent so I was not used and I must die :-)
        delete pCompare;
    return nErg;
}
// -----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildLikeRule(OSQLParseNode*& pAppend, OSQLParseNode*& pLiteral, const OSQLParseNode* pEscape) {
    sal_Int16 nErg = 0;

    if (!m_xField.IsValid())
        return nErg;

    TODO_ConvertCode();
    return nErg;
}

//=============================================================================
//-----------------------------------------------------------------------------
OSQLParser::OSQLParser(const IParseContext* _pContext)
    : m_pContext(_pContext)

{
    // static data initialization
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
#if YYDEBUG
    SQLyydebug = 1;
#endif

    if (s_aReverseRuleIDLookup.empty()) {
        memset(OSQLParser::s_nRuleIDs, 0, sizeof(OSQLParser::s_nRuleIDs[0]) * (OSQLParseNode::rule_count + 1));
        struct
        {
            OSQLParseNode::Rule eRule;  // the parse node's ID for the rule
            Utf8String sRuleName;       // the name of the rule ("single_select_statement")
        } aRuleDescriptions[] =
            {
                {OSQLParseNode::aggregate_fct, "aggregate_fct"},
                {OSQLParseNode::all_or_any_predicate, "all_or_any_predicate"},
                {OSQLParseNode::as, "as"},
                {OSQLParseNode::assignment, "assignment"},
                {OSQLParseNode::assignment_commalist, "assignment_commalist"},
                {OSQLParseNode::between_predicate, "between_predicate"},
                {OSQLParseNode::between_predicate_part_2, "between_predicate_part_2"},
                {OSQLParseNode::bit_value_fct, "bit_value_fct"},
                {OSQLParseNode::boolean_factor, "boolean_factor"},
                {OSQLParseNode::boolean_primary, "boolean_primary"},
                {OSQLParseNode::boolean_term, "boolean_term"},
                {OSQLParseNode::boolean_test, "boolean_test"},
                {OSQLParseNode::cast_spec, "cast_spec"},
                {OSQLParseNode::cast_target_array, "cast_target_array"},
                {OSQLParseNode::cast_target_scalar, "cast_target_scalar"},
                {OSQLParseNode::char_factor, "char_factor"},
                {OSQLParseNode::char_value_exp, "char_value_exp"},
                {OSQLParseNode::character_like_predicate_part_2, "character_like_predicate_part_2"},
                {OSQLParseNode::class_name, "class_name"},
                {OSQLParseNode::collate_clause, "collate_clause"},
                {OSQLParseNode::collating_function, "collating_function"},
                {OSQLParseNode::column, "column"},
                {OSQLParseNode::column_commalist, "column_commalist"},
                {OSQLParseNode::column_ref, "column_ref"},
                {OSQLParseNode::column_ref_commalist, "column_ref_commalist"},
                {OSQLParseNode::comparison, "comparison"},
                {OSQLParseNode::comparison_predicate, "comparison_predicate"},
                {OSQLParseNode::comparison_predicate_part_2, "comparison_predicate_part_2"},
                {OSQLParseNode::concatenation, "concatenation"},
                {OSQLParseNode::cross_union, "cross_union"},
                {OSQLParseNode::cte, "cte"},
                {OSQLParseNode::cte_block_list, "cte_block_list"},
                {OSQLParseNode::cte_column_list, "cte_column_list"},
                {OSQLParseNode::cte_table_name, "cte_table_name"},
                {OSQLParseNode::datetime_factor, "datetime_factor"},
                {OSQLParseNode::datetime_primary, "datetime_primary"},
                {OSQLParseNode::datetime_term, "datetime_term"},
                {OSQLParseNode::datetime_value_exp, "datetime_value_exp"},
                {OSQLParseNode::datetime_value_fct, "datetime_value_fct"},
                {OSQLParseNode::delete_statement_searched, "delete_statement_searched"},
                {OSQLParseNode::derived_column, "derived_column"},
                {OSQLParseNode::dynamic_parameter_specification, "dynamic_parameter_specification"},
                {OSQLParseNode::ecrelationship_join, "ecrelationship_join"},
                {OSQLParseNode::ecsqloptions_clause, "ecsqloptions_clause"},
                {OSQLParseNode::else_clause, "else_clause"},
                {OSQLParseNode::existence_test, "existence_test"},
                {OSQLParseNode::existing_window_name, "existing_window_name"},
                {OSQLParseNode::factor, "factor"},
                {OSQLParseNode::fct_spec, "fct_spec"},
                {OSQLParseNode::fetch_first_clause, "fetch_first_clause"},
                {OSQLParseNode::fetch_first_row_count, "fetch_first_row_count"},
                {OSQLParseNode::first_or_last_value, "first_or_last_value"},
                {OSQLParseNode::first_or_last_value_function, "first_or_last_value_function"},
                {OSQLParseNode::first_or_next, "first_or_next"},
                {OSQLParseNode::from_clause, "from_clause"},
                {OSQLParseNode::function_args_commalist, "function_args_commalist"},
                {OSQLParseNode::iif_spec, "iif_spec"},
                {OSQLParseNode::in_line_window_specification, "in_line_window_specification"},
                {OSQLParseNode::in_predicate, "in_predicate"},
                {OSQLParseNode::in_predicate_part_2, "in_predicate_part_2"},
                {OSQLParseNode::insert_atom, "insert_atom"},
                {OSQLParseNode::insert_atom_commalist, "insert_atom_commalist"},
                {OSQLParseNode::insert_statement, "insert_statement"},
                {OSQLParseNode::join_condition, "join_condition"},
                {OSQLParseNode::join_type, "join_type"},
                {OSQLParseNode::joined_table, "joined_table"},
                {OSQLParseNode::lead_or_lag, "lead_or_lag"},
                {OSQLParseNode::lead_or_lag_extent, "lead_or_lag_extent"},
                {OSQLParseNode::lead_or_lag_function, "lead_or_lag_function"},
                {OSQLParseNode::like_predicate, "like_predicate"},
                {OSQLParseNode::limit_offset_clause, "limit_offset_clause"},
                {OSQLParseNode::manipulative_statement, "manipulative_statement"},
                {OSQLParseNode::member_function_call, "member_function_call"},
                {OSQLParseNode::named_columns_join, "named_columns_join"},
                {OSQLParseNode::new_window_name, "new_window_name"},
                {OSQLParseNode::non_join_query_exp, "non_join_query_exp"},
                {OSQLParseNode::non_join_query_primary, "non_join_query_primary"},
                {OSQLParseNode::non_join_query_term, "non_join_query_term"},
                {OSQLParseNode::nth_row, "nth_row"},
                {OSQLParseNode::nth_value_function, "nth_value_function"},
                {OSQLParseNode::ntile_function, "ntile_function"},
                {OSQLParseNode::num_value_exp, "num_value_exp"},
                {OSQLParseNode::number_of_tiles, "number_of_tiles"},
                {OSQLParseNode::offset_row_count, "offset_row_count"},
                {OSQLParseNode::op_relationship_direction, "op_relationship_direction"},
                {OSQLParseNode::opt_asc_desc, "opt_asc_desc"},
                {OSQLParseNode::opt_collate_clause, "opt_collate_clause"},
                {OSQLParseNode::opt_column_commalist, "opt_column_commalist"},
                {OSQLParseNode::opt_column_ref_commalist, "opt_column_ref_commalist"},
                {OSQLParseNode::opt_cte_recursive, "opt_cte_recursive"},
                {OSQLParseNode::opt_disqualify_polymorphic_constraint, "opt_disqualify_polymorphic_constraint"},
                {OSQLParseNode::opt_disqualify_primary_join, "opt_disqualify_primary_join"},
                {OSQLParseNode::opt_ecsqloptions_clause, "opt_ecsqloptions_clause"},
                {OSQLParseNode::opt_escape, "opt_escape"},
                {OSQLParseNode::opt_existing_window_name, "opt_existing_window_name"},
                {OSQLParseNode::opt_extract_value, "opt_extract_value"},
                {OSQLParseNode::opt_fetch_first_row_count, "opt_fetch_first_row_count"},
                {OSQLParseNode::opt_filter_clause, "opt_filter_clause"},
                {OSQLParseNode::opt_function_arg, "opt_function_arg"},
                {OSQLParseNode::opt_group_by_clause, "opt_group_by_clause"},
                {OSQLParseNode::opt_having_clause, "opt_having_clause"},
                {OSQLParseNode::opt_lead_or_lag_function, "opt_lead_or_lag_function"},
                {OSQLParseNode::opt_limit_offset_clause, "opt_limit_offset_clause"},
                {OSQLParseNode::opt_member_function_args, "opt_member_function_args"},
                {OSQLParseNode::opt_null_order, "opt_null_order"},
                {OSQLParseNode::opt_only, "opt_only"},
                {OSQLParseNode::opt_only_all, "opt_only_all"},
                {OSQLParseNode::opt_order_by_clause, "opt_order_by_clause"},
                {OSQLParseNode::opt_pragma_for, "opt_pragma_for"},
                {OSQLParseNode::opt_pragma_func, "opt_pragma_func"},
                {OSQLParseNode::opt_pragma_set, "opt_pragma_set"},
                {OSQLParseNode::opt_pragma_set_val, "opt_pragma_set_val"},
                {OSQLParseNode::opt_result_offset_clause, "opt_result_offset_clause"},
                {OSQLParseNode::opt_where_clause, "opt_where_clause"},
                {OSQLParseNode::opt_window_clause, "opt_window_clause"},
                {OSQLParseNode::opt_window_frame_clause, "opt_window_frame_clause"},
                {OSQLParseNode::opt_window_frame_exclusion, "opt_window_frame_exclusion"},
                {OSQLParseNode::opt_window_partition_clause, "opt_window_partition_clause"},
                {OSQLParseNode::ordering_spec, "ordering_spec"},
                {OSQLParseNode::ordering_spec_commalist, "ordering_spec_commalist"},
                {OSQLParseNode::other_like_predicate_part_2, "other_like_predicate_part_2"},
                {OSQLParseNode::outer_join_type, "outer_join_type"},
                {OSQLParseNode::parameter, "parameter"},
                {OSQLParseNode::pragma, "pragma"},
                {OSQLParseNode::pragma_path, "pragma_path"},
                {OSQLParseNode::pragma_value, "pragma_value"},
                {OSQLParseNode::predicate_check, "predicate_check"},
                {OSQLParseNode::property_path, "property_path"},
                {OSQLParseNode::property_path_entry, "property_path_entry"},
                {OSQLParseNode::opt_optional_prop, "opt_optional_prop"},
                {OSQLParseNode::qualified_class_name, "qualified_class_name"},
                {OSQLParseNode::qualified_join, "qualified_join"},
                {OSQLParseNode::query_term, "query_term"},
                {OSQLParseNode::range_variable, "range_variable"},
                {OSQLParseNode::rank_function_type, "rank_function_type"},
                {OSQLParseNode::result_offset_clause, "result_offset_clause"},
                {OSQLParseNode::row_or_rows, "row_or_rows"},
                {OSQLParseNode::row_value_constructor, "row_value_constructor"},
                {OSQLParseNode::row_value_constructor_commalist, "row_value_constructor_commalist"},
                {OSQLParseNode::rtreematch_predicate, "rtreematch_predicate"},
                {OSQLParseNode::scalar_exp, "scalar_exp"},
                {OSQLParseNode::scalar_exp_commalist, "scalar_exp_commalist"},
                {OSQLParseNode::search_condition, "search_condition"},
                {OSQLParseNode::searched_case, "searched_case"},
                {OSQLParseNode::searched_when_clause, "searched_when_clause"},
                {OSQLParseNode::searched_when_clause_list, "searched_when_clause_list"},
                {OSQLParseNode::select_statement, "select_statement"},
                {OSQLParseNode::select_sublist, "select_sublist"},
                {OSQLParseNode::selection, "selection"},
                {OSQLParseNode::simple_value_specification, "simple_value_specification"},
                {OSQLParseNode::single_select_statement, "single_select_statement"},
                {OSQLParseNode::sql_not, "sql_not"},
                {OSQLParseNode::subquery, "subquery"},
                {OSQLParseNode::table_exp, "table_exp"},
                {OSQLParseNode::table_node, "table_node"},
                {OSQLParseNode::table_node_path, "table_node_path"},
                {OSQLParseNode::table_node_path_entry, "table_node_path_entry"},
                {OSQLParseNode::table_node_ref, "table_node_ref"},
                {OSQLParseNode::table_node_with_opt_member_func_call, "table_node_with_opt_member_func_call"},
                {OSQLParseNode::table_primary_as_range_column, "table_primary_as_range_column"},
                {OSQLParseNode::table_ref, "table_ref"},
                {OSQLParseNode::table_ref_commalist, "table_ref_commalist"},
                {OSQLParseNode::tablespace_qualified_class_name, "tablespace_qualified_class_name"},
                {OSQLParseNode::term, "term"},
                {OSQLParseNode::term_add_sub, "term_add_sub"},
                {OSQLParseNode::test_for_null, "test_for_null"},
                {OSQLParseNode::type_predicate, "type_predicate"},
                {OSQLParseNode::unary_predicate, "unary_predicate"},
                {OSQLParseNode::unique_test, "unique_test"},
                {OSQLParseNode::update_statement_searched, "update_statement_searched"},
                {OSQLParseNode::value_creation_fct, "value_creation_fct"},
                {OSQLParseNode::value_exp, "value_exp"},
                {OSQLParseNode::value_exp_commalist, "value_exp_commalist"},
                {OSQLParseNode::value_exp_primary, "value_exp_primary"},
                {OSQLParseNode::values_commalist, "values_commalist"},
                {OSQLParseNode::values_or_query_spec, "values_or_query_spec"},
                {OSQLParseNode::where_clause, "where_clause"},
                {OSQLParseNode::window_clause, "window_clause"},
                {OSQLParseNode::window_definition, "window_definition"},
                {OSQLParseNode::window_definition_list, "window_definition_list"},
                {OSQLParseNode::window_frame_between, "window_frame_between"},
                {OSQLParseNode::window_frame_bound, "window_frame_bound"},
                {OSQLParseNode::window_frame_bound_1, "window_frame_bound_1"},
                {OSQLParseNode::window_frame_bound_2, "window_frame_bound_2"},
                {OSQLParseNode::window_frame_clause, "window_frame_clause"},
                {OSQLParseNode::window_frame_exclusion, "window_frame_exclusion"},
                {OSQLParseNode::window_frame_extent, "window_frame_extent"},
                {OSQLParseNode::window_frame_following, "window_frame_following"},
                {OSQLParseNode::window_frame_preceding, "window_frame_preceding"},
                {OSQLParseNode::window_frame_start, "window_frame_start"},
                {OSQLParseNode::window_frame_units, "window_frame_units"},
                {OSQLParseNode::window_function, "window_function"},
                {OSQLParseNode::window_function_type, "window_function_type"},
                {OSQLParseNode::window_name, "window_name"},
                {OSQLParseNode::window_name_or_specification, "window_name_or_specification"},
                {OSQLParseNode::window_partition_clause, "window_partition_clause"},
                {OSQLParseNode::window_partition_column_reference, "window_partition_column_reference"},
                {OSQLParseNode::window_partition_column_reference_list, "window_partition_column_reference_list"},
                {OSQLParseNode::window_specification, "window_specification"},
            };
        size_t nRuleMapCount = sizeof(aRuleDescriptions) / sizeof(aRuleDescriptions[0]);
        OSL_ENSURE(nRuleMapCount == size_t(OSQLParseNode::rule_count), "OSQLParser::OSQLParser: added a new rule? Adjust this map!");
        for (size_t mapEntry = 0; mapEntry < nRuleMapCount; ++mapEntry) {
            // look up the rule description in the our identifier map
            sal_uInt32 nParserRuleID = StrToRuleID(aRuleDescriptions[mapEntry].sRuleName);
            // map the parser's rule ID to the OSQLParseNode::Rule
            s_aReverseRuleIDLookup[nParserRuleID] = aRuleDescriptions[mapEntry].eRule;
            // and map the OSQLParseNode::Rule to the parser's rule ID
            s_nRuleIDs[aRuleDescriptions[mapEntry].eRule] = nParserRuleID;
        }
    }

    if (m_pContext == nullptr)
        m_pContext = &s_aDefaultContext;
}
//-----------------------------------------------------------------------------
OSQLParser::~OSQLParser() {
}

}  // namespace connectivity

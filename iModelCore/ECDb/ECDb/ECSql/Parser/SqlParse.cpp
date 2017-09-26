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
//#define YYBISON      1
//#ifndef BISON_INCLUDED
//#define BISON_INCLUDED
#include "SqlBison.h"
//#endif

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
//using namespace ::osl;
using namespace ::dbtools;
using namespace ::comphelper;
extern int SQLyyparse(connectivity::OSQLParser*);

namespace connectivity
    {
    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    sal_Int16 OSQLParser::buildPredicateRule(OSQLParseNode*& pAppend, OSQLParseNode* pLiteral, OSQLParseNode*& pCompare, OSQLParseNode* pLiteral2)
        {
        OSL_ENSURE(inPredicateCheck(), "Only in predicate check allowed!");
        sal_Int16 nErg = 0;
        if (m_xField.IsValid())
            {
            TODO_ConvertCode();
            }
        if (!pCompare->getParent()) // I have no parent so I was not used and I must die :-)
            delete pCompare;
        return nErg;
        }
    // -----------------------------------------------------------------------------
    sal_Int16 OSQLParser::buildLikeRule(OSQLParseNode*& pAppend, OSQLParseNode*& pLiteral, const OSQLParseNode* pEscape)
        {
        sal_Int16 nErg = 0;

        if (!m_xField.IsValid())
            return nErg;

        TODO_ConvertCode();
        return nErg;
        }

    //=============================================================================
    //-----------------------------------------------------------------------------
    OSQLParser::OSQLParser(const RefCountedPtr< ::com::sun::star::lang::XMultiServiceFactory >& _xServiceFactory, const IParseContext* _pContext)
        :m_pContext(_pContext)
        , m_nFormatKey(0)
        , m_nDateFormatKey(0)
        , m_xServiceFactory(_xServiceFactory)
        {

#if YYDEBUG
        SQLyydebug = 1;
#endif

        if (s_aReverseRuleIDLookup.empty())
            {
            memset(OSQLParser::s_nRuleIDs, 0, sizeof(OSQLParser::s_nRuleIDs[0]) * (OSQLParseNode::rule_count + 1));
            struct
                {
                OSQLParseNode::Rule eRule;      // the parse node's ID for the rule
                Utf8String          sRuleName;  // the name of the rule ("single_select_statement")
                }   aRuleDescriptions[] =
                    {
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
                            {OSQLParseNode::cast_target_scalar, "cast_target_scalar"},
                            {OSQLParseNode::cast_target_array, "cast_target_array"},
                            {OSQLParseNode::catalog_name, "catalog_name"},
                            {OSQLParseNode::char_factor, "char_factor"},
                            {OSQLParseNode::char_substring_fct, "char_substring_fct"},
                            {OSQLParseNode::char_value_exp, "char_value_exp"},
                            {OSQLParseNode::char_value_fct, "char_value_fct"},
                            {OSQLParseNode::character_like_predicate_part_2, "character_like_predicate_part_2"},
                            {OSQLParseNode::column, "column"},
                            {OSQLParseNode::column_commalist, "column_commalist"},
                            {OSQLParseNode::column_ref, "column_ref"},
                            {OSQLParseNode::column_ref_commalist, "column_ref_commalist"},
                            {OSQLParseNode::comparison, "comparison"},
                            {OSQLParseNode::comparison_predicate, "comparison_predicate"},
                            {OSQLParseNode::comparison_predicate_part_2, "comparison_predicate_part_2"},
                            {OSQLParseNode::concatenation, "concatenation"},
                            {OSQLParseNode::cross_union, "cross_union"},
                            {OSQLParseNode::datetime_factor, "datetime_factor"},
                            {OSQLParseNode::datetime_primary, "datetime_primary"},
                            {OSQLParseNode::datetime_term, "datetime_term"},
                            {OSQLParseNode::datetime_value_exp, "datetime_value_exp"},
                            {OSQLParseNode::datetime_value_fct, "datetime_value_fct"},
                            {OSQLParseNode::delete_statement_searched, "delete_statement_searched"},
                            {OSQLParseNode::derived_column, "derived_column"},
                            {OSQLParseNode::ecsqloptions_clause, "ecsqloptions_clause"},
                            {OSQLParseNode::ecrelationship_join, "ecrelationship_join"},
                            {OSQLParseNode::existence_test, "existence_test"},
                            {OSQLParseNode::extract_exp, "extract_exp"},
                            {OSQLParseNode::factor, "factor"},
                            {OSQLParseNode::fct_spec, "fct_spec"},
                            {OSQLParseNode::fold, "fold"},
                            {OSQLParseNode::from_clause, "from_clause"},
                            {OSQLParseNode::function_args_commalist, "function_args_commalist"},
                            {OSQLParseNode::general_set_fct, "general_set_fct"},
                            {OSQLParseNode::in_predicate, "in_predicate"},
                            {OSQLParseNode::in_predicate_part_2, "in_predicate_part_2"},
                            {OSQLParseNode::insert_atom, "insert_atom"},
                            {OSQLParseNode::insert_atom_commalist, "insert_atom_commalist"},
                            {OSQLParseNode::insert_statement, "insert_statement"},
                            {OSQLParseNode::join_condition, "join_condition"},
                            {OSQLParseNode::join_type, "join_type"},
                            {OSQLParseNode::joined_table, "joined_table"},
                            {OSQLParseNode::length_exp, "length_exp"},
                            {OSQLParseNode::like_predicate, "like_predicate"},
                            {OSQLParseNode::limit_offset_clause, "limit_offset_clause"},
                            {OSQLParseNode::manipulative_statement, "manipulative_statement"},
                            {OSQLParseNode::rtreematch_predicate, "rtreematch_predicate"},
                            {OSQLParseNode::named_columns_join, "named_columns_join"},
                            {OSQLParseNode::num_value_exp, "num_value_exp"},
                            {OSQLParseNode::non_join_query_exp, "non_join_query_exp"},
                            {OSQLParseNode::non_join_query_primary, "non_join_query_primary"},
                            {OSQLParseNode::non_join_query_term, "non_join_query_term"},
                            {OSQLParseNode::op_relationship_direction, "op_relationship_direction"},
                            {OSQLParseNode::opt_asc_desc, "opt_asc_desc"},
                            {OSQLParseNode::opt_column_commalist, "opt_column_commalist"},
                            {OSQLParseNode::opt_column_ref_commalist, "opt_column_ref_commalist"},
                            {OSQLParseNode::opt_ecsqloptions_clause, "opt_ecsqloptions_clause"},
                            {OSQLParseNode::opt_escape, "opt_escape"},
                            {OSQLParseNode::opt_group_by_clause, "opt_group_by_clause"},
                            {OSQLParseNode::opt_having_clause, "opt_having_clause"},
                            {OSQLParseNode::opt_limit_offset_clause, "opt_limit_offset_clause"},
                            {OSQLParseNode::opt_order_by_clause, "opt_order_by_clause"},
                            {OSQLParseNode::opt_where_clause, "opt_where_clause"},
                            {OSQLParseNode::ordering_spec_commalist, "ordering_spec_commalist"},
                            {OSQLParseNode::ordering_spec, "ordering_spec"},
                            {OSQLParseNode::other_like_predicate_part_2, "other_like_predicate_part_2"},
                            {OSQLParseNode::outer_join_type, "outer_join_type"},
                            {OSQLParseNode::parameter, "parameter"},
                            {OSQLParseNode::parameter_ref, "parameter_ref"},
                            {OSQLParseNode::position_exp, "position_exp"},
                            {OSQLParseNode::predicate_check, "predicate_check"},
                            {OSQLParseNode::property_path, "property_path"},
                            {OSQLParseNode::property_path_entry, "property_path_entry"},
                            {OSQLParseNode::qualified_join, "qualified_join"},
                            {OSQLParseNode::query_term, "query_term"},
                            {OSQLParseNode::range_variable, "range_variable"},
                            {OSQLParseNode::row_value_constructor, "row_value_constructor"},
                            {OSQLParseNode::row_value_constructor_commalist, "row_value_constructor_commalist"},
                            {OSQLParseNode::scalar_exp, "scalar_exp"},
                            {OSQLParseNode::scalar_exp_commalist, "scalar_exp_commalist"},
                            {OSQLParseNode::schema_name, "schema_name"},
                            {OSQLParseNode::search_condition, "search_condition"},
                            {OSQLParseNode::single_select_statement, "single_select_statement"},
                            {OSQLParseNode::select_sublist, "select_sublist"},
                            {OSQLParseNode::selection, "selection"},
                            {OSQLParseNode::sql_not, "sql_not"},
                            {OSQLParseNode::subquery, "subquery"},
                            {OSQLParseNode::table_exp, "table_exp"},
                            {OSQLParseNode::table_name, "table_name"},
                            {OSQLParseNode::table_node, "table_node"},
                            {OSQLParseNode::table_primary_as_range_column, "table_primary_as_range_column"},
                            {OSQLParseNode::table_ref_commalist, "table_ref_commalist"},
                            {OSQLParseNode::table_ref, "table_ref"},
                            {OSQLParseNode::term, "term"},
                            {OSQLParseNode::test_for_null, "test_for_null"},
                            {OSQLParseNode::unary_predicate, "unary_predicate"},
                            {OSQLParseNode::select_statement, "select_statement"},
                            {OSQLParseNode::unique_test, "unique_test"},
                            {OSQLParseNode::update_statement_searched, "update_statement_searched"},
                            {OSQLParseNode::value_exp, "value_exp"},
                            {OSQLParseNode::value_exp_commalist, "value_exp_commalist"},
                            {OSQLParseNode::value_exp_primary, "value_exp_primary"},
                            {OSQLParseNode::values_or_query_spec, "values_or_query_spec"},
                            {OSQLParseNode::where_clause, "where_clause"},
                    };
                size_t nRuleMapCount = sizeof(aRuleDescriptions) / sizeof(aRuleDescriptions[0]);
                OSL_ENSURE(nRuleMapCount == size_t(OSQLParseNode::rule_count), "OSQLParser::OSQLParser: added a new rule? Adjust this map!");
                for (size_t mapEntry = 0; mapEntry < nRuleMapCount; ++mapEntry)
                    {
                    // look up the rule description in the our identifier map
                    sal_uInt32 nParserRuleID = StrToRuleID(aRuleDescriptions[mapEntry].sRuleName);
                    // map the parser's rule ID to the OSQLParseNode::Rule
                    s_aReverseRuleIDLookup[nParserRuleID] = aRuleDescriptions[mapEntry].eRule;
                    // and map the OSQLParseNode::Rule to the parser's rule ID
                    s_nRuleIDs[aRuleDescriptions[mapEntry].eRule] = nParserRuleID;
                    }
            }

        if (m_pContext == NULL)
            m_pContext = &s_aDefaultContext;
        }
    //-----------------------------------------------------------------------------
    OSQLParser::~OSQLParser()
        {
        }

    //-----------------------------------------------------------------------------
    sal_Int32 OSQLParser::getFunctionReturnType(const Utf8String& _sFunctionName, const IParseContext* pContext)
        {
        sal_Int32 nType = DataType::VARCHAR;
        Utf8String sFunctionName = _sFunctionName;

        if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_BIT_LENGTH, pContext)))                nType = DataType::INTEGER;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_CHAR_LENGTH, pContext)))          nType = DataType::INTEGER;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_INSERT, pContext)))               nType = DataType::VARCHAR;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_LEFT, pContext)))                 nType = DataType::VARCHAR;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_OCTET_LENGTH, pContext)))         nType = DataType::INTEGER;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_POSITION, pContext)))             nType = DataType::INTEGER;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_RIGHT, pContext)))                nType = DataType::VARCHAR;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_SUBSTRING, pContext)))            nType = DataType::VARCHAR;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_CURRENT_DATE, pContext)))         nType = DataType::DATE;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_CURRENT_TIMESTAMP, pContext)))    nType = DataType::TIMESTAMP;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_CURDATE, pContext)))              nType = DataType::DATE;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_EXTRACT, pContext)))              nType = DataType::VARCHAR;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_HOUR, pContext)))                 nType = DataType::INTEGER;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_MINUTE, pContext)))               nType = DataType::INTEGER;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_MONTH, pContext)))                nType = DataType::INTEGER;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_NOW, pContext)))                  nType = DataType::TIMESTAMP;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_SECOND, pContext)))               nType = DataType::INTEGER;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_WEEK, pContext)))                 nType = DataType::INTEGER;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_YEAR, pContext)))                 nType = DataType::INTEGER;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_COUNT, pContext)))                nType = DataType::INTEGER;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_MAX, pContext)))                  nType = DataType::DOUBLE;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_MIN, pContext)))                  nType = DataType::DOUBLE;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_AVG, pContext)))                  nType = DataType::DOUBLE;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_SUM, pContext)))                  nType = DataType::DOUBLE;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_LOWER, pContext)))                nType = DataType::VARCHAR;
        else if (sFunctionName.EqualsI(TokenIDToStr(SQL_TOKEN_UPPER, pContext)))                nType = DataType::VARCHAR;

        return nType;
        }
    }  // namespace connectivity

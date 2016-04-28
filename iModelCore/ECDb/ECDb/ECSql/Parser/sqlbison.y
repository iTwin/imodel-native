%code top   {
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

//#pragma warning(disable:4603) Give Error on iOS
#include "ECDbPch.h"
#include "SqlTypes.h"
#include <vector>
#include <string.h>

#ifndef _CONNECTIVITY_SQLNODE_HXX
#include "SqlNode.h"
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include "SqlParse.h"
#endif
#ifndef _CONNECTIVITY_SQLINTERNALNODE_HXX
#include "InternalNode.h"
#endif
#ifndef _CONNECTIVITY_SQLSCAN_HXX
#include "SqlScan.h"
#endif

#if defined __GNUC__
//    #pragma GCC system_header
#elif defined __SUNPRO_CC
#pragma disable_warn
#elif defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable:4273 4701 4706)
#endif

}

%{
static Utf8String aEmptyString;

static connectivity::OSQLInternalNode* newNode(const sal_Char* pNewValue,
                                 const connectivity::SQLNodeType eNodeType,
                                 const sal_uInt32 nNodeID = 0)
{
    return new connectivity::OSQLInternalNode(pNewValue, eNodeType, nNodeID);
}

static connectivity::OSQLInternalNode* newNode(const Utf8String& _NewValue,
                                const connectivity::SQLNodeType eNodeType,
                                const sal_uInt32 nNodeID = 0)
{
    return new connectivity::OSQLInternalNode(_NewValue, eNodeType, nNodeID);
}

// yyi ist die interne Nr. der Regel, die gerade reduziert wird.
// Ueber die Mapping-Tabelle yyrmap wird daraus eine externe Regel-Nr.
#define SQL_NEW_RULE             newNode(aEmptyString, SQL_NODE_RULE, yyr1[yyn])
#define SQL_NEW_LISTRULE         newNode(aEmptyString, SQL_NODE_LISTRULE, yyr1[yyn])
#define SQL_NEW_COMMALISTRULE   newNode(aEmptyString, SQL_NODE_COMMALISTRULE, yyr1[yyn])
#define SQL_NEW_DOTLISTRULE   newNode(aEmptyString, SQL_NODE_DOTLISTRULE, yyr1[yyn])


connectivity::OSQLParser* xxx_pGLOBAL_SQLPARSER;

#if !(defined MACOSX && defined PPC)
#define YYERROR_VERBOSE
#endif

#define SQLyyerror(s)                        \
{                                            \
    xxx_pGLOBAL_SQLPARSER->error(s);        \
}

using namespace connectivity;
#define SQLyylex xxx_pGLOBAL_SQLPARSER->SQLlex
%}
    /* symbolic tokens */

%union {
    connectivity::OSQLParseNode * pParseNode;
}
%type <pParseNode> '(' ')' ',' ':' ';' '?' '[' ']' '{' '}' '.' 'K' 'M' 'G' 'T' 'P'

%token <pParseNode> SQL_TOKEN_ACCESS_DATE SQL_TOKEN_REAL_NUM
%token <pParseNode> SQL_TOKEN_INTNUM SQL_TOKEN_APPROXNUM SQL_TOKEN_NOT SQL_TOKEN_NAME SQL_TOKEN_ARRAY_INDEX


%nonassoc <pParseNode> SQL_TOKEN_UMINUS



    /* literal keyword tokens */

%token <pParseNode> SQL_TOKEN_ALL SQL_TOKEN_ALTER SQL_TOKEN_AMMSC SQL_TOKEN_ANY SQL_TOKEN_AS SQL_TOKEN_ASC SQL_TOKEN_AT SQL_TOKEN_AUTHORIZATION SQL_TOKEN_AVG

%token <pParseNode> SQL_TOKEN_BETWEEN SQL_TOKEN_BIT SQL_TOKEN_BY

%token <pParseNode> SQL_TOKEN_CAST SQL_TOKEN_CHARACTER SQL_TOKEN_CHECK SQL_TOKEN_COLLATE SQL_TOKEN_COMMIT SQL_TOKEN_CONTINUE SQL_TOKEN_CONVERT SQL_TOKEN_COUNT SQL_TOKEN_CREATE SQL_TOKEN_CROSS
%token <pParseNode> SQL_TOKEN_CURRENT SQL_TOKEN_CURSOR

%token <pParseNode> SQL_TOKEN_DATEVALUE SQL_TOKEN_DAY SQL_TOKEN_DEC SQL_TOKEN_DECIMAL SQL_TOKEN_DECLARE SQL_TOKEN_DEFAULT SQL_TOKEN_DELETE SQL_TOKEN_DESC
%token <pParseNode> SQL_TOKEN_DISTINCT SQL_TOKEN_DROP SQL_TOKEN_FORWARD SQL_TOKEN_BACKWARD

%token <pParseNode> SQL_TOKEN_ESCAPE SQL_TOKEN_EXCEPT SQL_TOKEN_EXISTS SQL_TOKEN_FALSE SQL_TOKEN_FETCH SQL_TOKEN_FLOAT SQL_TOKEN_FOR SQL_TOKEN_FOREIGN SQL_TOKEN_FOUND SQL_TOKEN_FROM SQL_TOKEN_FULL

%token <pParseNode> SQL_TOKEN_GROUP SQL_TOKEN_HAVING SQL_TOKEN_IN SQL_TOKEN_INDICATOR SQL_TOKEN_INNER SQL_TOKEN_INSERT SQL_TOKEN_INTO SQL_TOKEN_IS SQL_TOKEN_INTERSECT

%token <pParseNode> SQL_TOKEN_JOIN SQL_TOKEN_KEY SQL_TOKEN_LIKE SQL_TOKEN_LOCAL SQL_TOKEN_LEFT SQL_TOKEN_RIGHT SQL_TOKEN_LOWER
%token <pParseNode> SQL_TOKEN_MAX SQL_TOKEN_MIN SQL_TOKEN_NATURAL SQL_TOKEN_NCHAR SQL_TOKEN_NULL SQL_TOKEN_NUMERIC

%token <pParseNode> SQL_TOKEN_OCTET_LENGTH SQL_TOKEN_OF SQL_TOKEN_ON SQL_TOKEN_OPTION SQL_TOKEN_ORDER SQL_TOKEN_OUTER

%token <pParseNode> SQL_TOKEN_PRECISION SQL_TOKEN_PRIMARY SQL_TOKEN_PROCEDURE SQL_TOKEN_PUBLIC
%token <pParseNode> SQL_TOKEN_REAL SQL_TOKEN_REFERENCES SQL_TOKEN_ROLLBACK

%token <pParseNode> SQL_TOKEN_SCHEMA SQL_TOKEN_SELECT SQL_TOKEN_SET SQL_TOKEN_SMALLINT SQL_TOKEN_SOME SQL_TOKEN_SQLCODE SQL_TOKEN_SQLERROR SQL_TOKEN_SUM
/* No TIME and no TIMEZONE support in ECSQL 
%token <pParseNode> SQL_TOKEN_TIME SQL_TOKEN_TIMEZONE_HOUR SQL_TOKEN_TIMEZONE_MINUTE SQL_TOKEN_ZONE SQL_TOKEN_WITHOUT */
%token <pParseNode> SQL_TOKEN_TABLE SQL_TOKEN_TO SQL_TOKEN_TRANSLATE SQL_TOKEN_TRUE SQL_TOKEN_UNION
%token <pParseNode> SQL_TOKEN_UNIQUE SQL_TOKEN_UNKNOWN SQL_TOKEN_UPDATE SQL_TOKEN_UPPER SQL_TOKEN_USING SQL_TOKEN_VALUES SQL_TOKEN_VIEW
%token <pParseNode> SQL_TOKEN_WHERE SQL_TOKEN_WITH SQL_TOKEN_WORK 

%token <pParseNode> SQL_TOKEN_BIT_LENGTH SQL_TOKEN_CHAR SQL_TOKEN_CHAR_LENGTH SQL_TOKEN_POSITION SQL_TOKEN_SUBSTRING SQL_TOKEN_SQL_TOKEN_INTNUM

/* time and date functions */
%token <pParseNode> SQL_TOKEN_CURRENT_DATE SQL_TOKEN_CURRENT_TIMESTAMP SQL_TOKEN_CURDATE SQL_TOKEN_NOW SQL_TOKEN_EXTRACT
/*time not supported in ECSQL %token <pParseNode> SQL_TOKEN_CURRENT_TIME SQL_TOKEN_CURTIME */
%token <pParseNode> SQL_TOKEN_DAYNAME  SQL_TOKEN_DAYOFMONTH  SQL_TOKEN_DAYOFWEEK  SQL_TOKEN_DAYOFYEAR 
%token <pParseNode> SQL_TOKEN_HOUR SQL_TOKEN_MINUTE  SQL_TOKEN_MONTH  SQL_TOKEN_MONTHNAME SQL_TOKEN_QUARTER SQL_TOKEN_DATEDIFF
%token <pParseNode> SQL_TOKEN_SECOND SQL_TOKEN_TIMESTAMPADD SQL_TOKEN_TIMESTAMPDIFF SQL_TOKEN_TIMEVALUE SQL_TOKEN_WEEK SQL_TOKEN_YEAR

// computational operation
%token <pParseNode> SQL_TOKEN_EVERY

%token <pParseNode> SQL_TOKEN_WITHIN SQL_TOKEN_ARRAY_AGG
%token <pParseNode> SQL_TOKEN_CASE SQL_TOKEN_THEN SQL_TOKEN_END SQL_TOKEN_NULLIF SQL_TOKEN_COALESCE SQL_TOKEN_WHEN SQL_TOKEN_ELSE
%token <pParseNode> SQL_TOKEN_BEFORE SQL_TOKEN_AFTER SQL_TOKEN_INSTEAD SQL_TOKEN_EACH SQL_TOKEN_REFERENCING SQL_TOKEN_BEGIN SQL_TOKEN_ATOMIC SQL_TOKEN_TRIGGER SQL_TOKEN_ROW SQL_TOKEN_STATEMENT
%token <pParseNode> SQL_TOKEN_NEW SQL_TOKEN_OLD
%token <pParseNode> SQL_TOKEN_VALUE SQL_TOKEN_CURRENT_CATALOG SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP SQL_TOKEN_CURRENT_PATH SQL_TOKEN_CURRENT_ROLE SQL_TOKEN_CURRENT_SCHEMA
%token <pParseNode> SQL_TOKEN_VARCHAR SQL_TOKEN_VARBINARY SQL_TOKEN_VARYING SQL_TOKEN_OBJECT SQL_TOKEN_NCLOB SQL_TOKEN_NATIONAL
%token <pParseNode> SQL_TOKEN_LARGE SQL_TOKEN_CLOB SQL_TOKEN_BLOB SQL_TOKEN_BIGI SQL_TOKEN_INTERVAL
// window function
%token <pParseNode> SQL_TOKEN_OVER SQL_TOKEN_ROW_NUMBER SQL_TOKEN_NTILE SQL_TOKEN_LEAD SQL_TOKEN_LAG SQL_TOKEN_RESPECT SQL_TOKEN_IGNORE SQL_TOKEN_NULLS
%token <pParseNode> SQL_TOKEN_FIRST_VALUE SQL_TOKEN_LAST_VALUE SQL_TOKEN_NTH_VALUE SQL_TOKEN_FIRST SQL_TOKEN_LAST
%token <pParseNode> SQL_TOKEN_EXCLUDE SQL_TOKEN_OTHERS SQL_TOKEN_TIES SQL_TOKEN_FOLLOWING SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING SQL_TOKEN_RANGE SQL_TOKEN_ROWS
%token <pParseNode> SQL_TOKEN_PARTITION SQL_TOKEN_WINDOW SQL_TOKEN_NO

//EC functions
%token <pParseNode> SQL_TOKEN_GETECCLASSID

// LIMIT and OFFSEt
%token <pParseNode> SQL_TOKEN_LIMIT SQL_TOKEN_OFFSET SQL_TOKEN_NEXT SQL_TOKEN_ONLY

//non-standard
%token <pParseNode> SQL_TOKEN_MATCH SQL_TOKEN_ECSQLOPTIONS

//EC data types
%token <pParseNode> SQL_TOKEN_BINARY SQL_TOKEN_BOOLEAN SQL_TOKEN_DOUBLE SQL_TOKEN_INTEGER SQL_TOKEN_INT SQL_TOKEN_LONG SQL_TOKEN_INT64 SQL_TOKEN_STRING SQL_TOKEN_DATE SQL_TOKEN_TIMESTAMP SQL_TOKEN_DATETIME 

/* operators */
%left SQL_TOKEN_NAME
%left SQL_TOKEN_ARRAY_INDEX

%left <pParseNode> SQL_TOKEN_OR
%left <pParseNode> SQL_TOKEN_AND

%left <pParseNode> '|'
%left <pParseNode> '&'

%left <pParseNode> SQL_LESSEQ SQL_GREATEQ SQL_NOTEQUAL SQL_LESS SQL_GREAT SQL_EQUAL
%left <pParseNode> '+' '-' SQL_CONCAT
%left <pParseNode> '*' '/' '%'
%left SQL_TOKEN_NATURAL SQL_TOKEN_CROSS SQL_TOKEN_FULL SQL_TOKEN_LEFT SQL_TOKEN_RIGHT
%right '~'
%left ')'
%right '='
%right '.'
%right '('


%token <pParseNode> SQL_TOKEN_INVALIDSYMBOL

/*%type <pParseNode> sql_single_statement */

%type <pParseNode> sql /*schema */
%type <pParseNode> column_def_opt_list column_def_opt table_constraint_def column_commalist opt_column_array_idx property_path_entry property_path
%type <pParseNode> view_def opt_with_check_option opt_column_commalist column_ref_commalist opt_column_ref_commalist 
%type <pParseNode> opt_order_by_clause ordering_spec_commalist
%type <pParseNode> ordering_spec opt_asc_desc manipulative_statement commit_statement
%type <pParseNode> delete_statement_searched fetch_statement
%type <pParseNode> insert_statement values_or_query_spec
%type <pParseNode> rollback_statement select_statement_into opt_all_distinct
%type <pParseNode> assignment_commalist assignment
%type <pParseNode> update_statement_searched target_commalist target opt_where_clause
%type <pParseNode> single_select_statement selection table_exp from_clause table_ref_commalist table_ref
%type <pParseNode> where_clause opt_group_by_clause opt_having_clause
%type <pParseNode> search_condition predicate comparison_predicate comparison_predicate_part_2 between_predicate between_predicate_part_2
%type <pParseNode> like_predicate opt_escape test_for_null null_predicate_part_2 in_predicate in_predicate_part_2 character_like_predicate_part_2 other_like_predicate_part_2
%type <pParseNode> all_or_any_predicate any_all_some existence_test subquery quantified_comparison_predicate_part_2
%type <pParseNode> scalar_exp_commalist parameter_ref literal parenthesized_boolean_value_expression
%type <pParseNode> column_ref data_type column cursor parameter range_variable /*user like_check*/
/* neue Regeln bei OJ */
%type <pParseNode> derived_column as_clause table_name num_primary term num_value_exp
%type <pParseNode> value_exp_primary num_value_fct unsigned_value_spec cast_spec fct_spec  scalar_subquery
%type <pParseNode> position_exp extract_exp length_exp general_value_spec
%type <pParseNode> general_set_fct set_fct_type joined_table ecrelationship_join op_relationship_direction
%type <pParseNode> row_value_constructor_commalist row_value_constructor  row_value_constructor_elem
%type <pParseNode> qualified_join value_exp join_type outer_join_type join_condition boolean_term unary_predicate
%type <pParseNode> boolean_factor truth_value boolean_test boolean_primary named_columns_join join_spec
%type <pParseNode> cast_operand cast_target factor datetime_value_exp datetime_term datetime_factor
%type <pParseNode> datetime_primary datetime_value_fct /*time_zone time_zone_specifier interval_term */ interval_qualifier
%type <pParseNode> start_field non_second_datetime_field end_field single_datetime_field extract_field datetime_field /*time_zone_field opt_with_or_without_time_zone*/
%type <pParseNode> char_length_exp octet_length_exp bit_length_exp select_sublist string_value_exp
%type <pParseNode> char_value_exp concatenation char_factor char_primary string_value_fct char_substring_fct fold
%type <pParseNode> form_conversion char_translation bit_value_fct bit_substring_fct
%type <pParseNode> /*bit_concatenation*/ bit_value_exp bit_factor bit_primary collate_clause char_value_fct unique_spec value_exp_commalist in_predicate_value unique_test update_source
%type <pParseNode> date_function_0Argument  function_name12 function_name0 
%type <pParseNode> all sql_not for_length upper_lower comparison /* column_val */  cross_union /*opt_schema_element_list*/
%type <pParseNode> /*op_authorization op_schema*/ nil_fkt schema_element base_table_def base_table_element base_table_element_commalist
%type <pParseNode> column_def select_statement
%type <pParseNode> function_args_commalist function_arg
%type <pParseNode> catalog_name schema_name table_node function_name table_primary_as_range_column opt_as
%type <pParseNode> case_expression else_clause result_expression result case_abbreviation case_specification searched_when_clause simple_when_clause searched_case simple_case
%type <pParseNode> when_operand_list when_operand case_operand
%type <pParseNode> trigger_definition trigger_name trigger_action_time trigger_event transition_table_or_variable_list triggered_action trigger_column_list triggered_when_clause triggered_SQL_statement SQL_procedure_statement old_transition_variable_name new_transition_variable_name
%type <pParseNode> op_referencing op_trigger_columnlist op_triggered_action_for opt_row trigger_for SQL_procedure_statement_list transition_table_or_variable old_transition_table_name new_transition_table_name transition_table_name
%type <pParseNode> searched_when_clause_list simple_when_clause_list predefined_type opt_char_set_spec opt_collate_clause character_string_type national_character_string_type
%type <pParseNode> binary_string_type numeric_type boolean_type datetime_type interval_type opt_paren_precision paren_char_length opt_paren_char_large_length paren_character_large_object_length
%type <pParseNode> large_object_length opt_multiplier character_large_object_type national_character_large_object_type binary_large_object_string_type 
%type <pParseNode> approximate_numeric_type exact_numeric_type opt_paren_precision_scale
/* window function rules */
%type <pParseNode> window_function window_function_type ntile_function number_of_tiles lead_or_lag_function lead_or_lag lead_or_lag_extent offset default_expression null_treatment
%type <pParseNode> first_or_last_value_function first_or_last_value nth_value_function nth_row from_first_or_last window_name_or_specification in_line_window_specification opt_lead_or_lag_function
%type <pParseNode> opt_null_treatment opt_from_first_or_last simple_value_specification dynamic_parameter_specification window_name window_clause window_definition_list window_definition
%type <pParseNode> new_window_name window_specification_details existing_window_name window_partition_clause window_partition_column_reference_list window_partition_column_reference window_frame_clause
%type <pParseNode> window_frame_units window_frame_extent window_frame_start window_frame_preceding window_frame_between window_frame_bound_1 window_frame_bound_2 window_frame_bound window_frame_following window_frame_exclusion
%type <pParseNode> opt_window_frame_clause opt_window_partition_clause opt_existing_window_name window_specification opt_window_frame_exclusion opt_window_clause opt_offset
%type <pParseNode> opt_fetch_first_row_count fetch_first_clause offset_row_count fetch_first_row_count first_or_next row_or_rows opt_result_offset_clause result_offset_clause
/* LIMIT and OFFSET */
%type <pParseNode> opt_limit_offset_clause limit_offset_clause opt_fetch_first_clause opt_only ecclassid_fct_spec union_op
/* non-standard */
%type <pParseNode> rtreematch_predicate rtreematch_predicate_part_2
%type <pParseNode> opt_ecsqloptions_clause ecsqloptions_clause ecsqloptions_list ecsqloption ecsqloptionvalue
%%

/* Parse Tree an OSQLParser zurueckliefern
 * (der Zugriff ueber yyval nach Aufruf des Parsers scheitert,
 *
 */
sql_single_statement:
        sql
        { xxx_pGLOBAL_SQLPARSER->setParseTree( $1 ); }
    |    sql ';'
        { xxx_pGLOBAL_SQLPARSER->setParseTree( $1 ); }
    ;

    /* schema definition language */
    /* Note: other ``sql:sal_Char() rules appear later in the grammar */
    
sql:
        manipulative_statement
    |    schema_element
           {
                $$ = SQL_NEW_RULE;
        $$->append($1);
       }
    ;
    
/***

op_authorization:
        {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_AUTHORIZATION user
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;

user:    SQL_TOKEN_NAME
    ;

op_schema:
        {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_NAME
    |    SQL_TOKEN_NAME '.' SQL_TOKEN_NAME
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
    ;

schema:
        SQL_TOKEN_CREATE SQL_TOKEN_SCHEMA op_schema op_authorization opt_schema_element_list
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
            $$->append($5);
        }
    ;

opt_schema_element_list:
            {$$ = SQL_NEW_RULE;}
    |       schema_glement_list
    ;

schema_element_list:
        schema_element
            {$$ = SQL_NEW_LISTRULE;
            $$->append($1);}
    |       schema_element_list schema_element
            {$1->append($2);
            $$ = $1;}
    ;
*/

schema_element:
            base_table_def
    |       view_def
    |        trigger_definition
    ;

base_table_def:
        SQL_TOKEN_CREATE SQL_TOKEN_TABLE table_node '(' base_table_element_commalist ')'
        {$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4 = newNode("(", SQL_NODE_PUNCTUATION));
        $$->append($5);
        $$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));}
    ;

base_table_element_commalist:
        base_table_element
        {$$ = SQL_NEW_COMMALISTRULE;
        $$->append($1);}
    |   base_table_element_commalist ',' base_table_element
        {$1->append($3);
        $$ = $1;}
    ;

base_table_element:
        column_def
    |    table_constraint_def
    ;

column_def:
        column data_type column_def_opt_list
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            }
    ;

column_def_opt_list:
        /* empty */                 {$$ = SQL_NEW_LISTRULE;}
    |       column_def_opt_list column_def_opt
            {$1->append($2);
            $$ = $1;}
    ;

nil_fkt:
    datetime_value_fct
    ;
unique_spec:
        SQL_TOKEN_UNIQUE
    |    SQL_TOKEN_PRIMARY SQL_TOKEN_KEY
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
column_def_opt:
        SQL_TOKEN_NOT SQL_TOKEN_NULL
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);}
    |       unique_spec
    |       SQL_TOKEN_DEFAULT literal
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);}
    |       SQL_TOKEN_DEFAULT SQL_TOKEN_NULL
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);}
    |       SQL_TOKEN_DEFAULT nil_fkt
            {
                $$ = SQL_NEW_RULE;
                $$->append($1);
                $$->append($2);
            }
    |       SQL_TOKEN_CHECK
    |       SQL_TOKEN_CHECK '(' search_condition ')'
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));}
    |       SQL_TOKEN_REFERENCES table_node
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);}
    |       SQL_TOKEN_REFERENCES table_node '(' column_commalist ')'
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($4);
            $$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));}
    ;

table_constraint_def:
        unique_spec '(' column_commalist ')'
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));}
    |       SQL_TOKEN_FOREIGN SQL_TOKEN_KEY '(' column_commalist ')' SQL_TOKEN_REFERENCES table_node
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($4);
            $$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
            $$->append($6);
            $$->append($7);}
    |       SQL_TOKEN_FOREIGN SQL_TOKEN_KEY '(' column_commalist ')' SQL_TOKEN_REFERENCES table_node '(' column_commalist ')'
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($4);
            $$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
            $$->append($6);
            $$->append($7);
            $$->append($8 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($9);
            $$->append($10 = newNode(")", SQL_NODE_PUNCTUATION));}
    |       SQL_TOKEN_CHECK '(' search_condition ')'
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));}
    ;

column_commalist:
        column_commalist ',' column
        {
            $1->append($3);
            $$ = $1;
        }
    |   column
        {
            $$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);
        }
    ;

column_ref_commalist:
        column_ref_commalist ',' column_ref
        {
            $1->append($3);
            $$ = $1;
        }
    |   column_ref
        {
            $$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);
        }
    ;

view_def:
        SQL_TOKEN_CREATE SQL_TOKEN_VIEW table_node opt_column_ref_commalist SQL_TOKEN_AS single_select_statement opt_with_check_option
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
            $$->append($5);
            $$->append($6);
            $$->append($7);}
    ;

opt_with_check_option:
        /* empty */         {$$ = SQL_NEW_RULE;}
    |       SQL_TOKEN_WITH SQL_TOKEN_CHECK SQL_TOKEN_OPTION
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);}
    ;

/* TODO: check whether further usages of opt_column_commalist or column_commalist should actually
 be turned into opt_column_ref_commalist or column_ref_commalist respectively. */
opt_column_commalist:
        /* empty */         {$$ = SQL_NEW_RULE;}
    |       '(' column_commalist ')'
            {$$ = SQL_NEW_RULE;
            $$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));}
    ;

opt_column_ref_commalist:
        /* empty */         {$$ = SQL_NEW_RULE;}
    |       '(' column_ref_commalist ')'
            {$$ = SQL_NEW_RULE;
            $$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));}
    ;

    /* module language */

opt_order_by_clause:
        /* empty */         
        {$$ = SQL_NEW_RULE;}
    |   SQL_TOKEN_ORDER SQL_TOKEN_BY ordering_spec_commalist
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    ;

ordering_spec_commalist:
        ordering_spec
        {
            $$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);
        }
    |   ordering_spec_commalist ',' ordering_spec
        {
            $1->append($3);
            $$ = $1;
        }
    ;

ordering_spec:
/*        SQL_TOKEN_INTNUM opt_asc_desc
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            }
*/
    predicate opt_asc_desc
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }

    |   
    row_value_constructor_elem opt_asc_desc
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;

opt_asc_desc:
        {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_ASC
    |    SQL_TOKEN_DESC
    ;


/***
manipulative_statement_list:
        manipulative_statement
            {$$ = SQL_NEW_LISTRULE;
            $$->append($1);}
    |       manipulative_statement_list manipulative_statement
            {$1->append($2);
            $$ = $1;}
    ;
***/

sql_not:
    {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_NOT
    ;

/* manipulative statements */

manipulative_statement:
            commit_statement
    |       delete_statement_searched
    |       fetch_statement
    |       insert_statement
    |       rollback_statement
    |       select_statement_into
    |       update_statement_searched
    |        select_statement
    ;

select_statement:
        single_select_statement 
            {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            }
    |    single_select_statement union_op all select_statement
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    ;

union_op:
    SQL_TOKEN_INTERSECT
    | SQL_TOKEN_UNION
    | SQL_TOKEN_EXCEPT
    ;
commit_statement:
        SQL_TOKEN_COMMIT SQL_TOKEN_WORK
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);}
    ;

delete_statement_searched:
        SQL_TOKEN_DELETE SQL_TOKEN_FROM table_ref opt_where_clause opt_ecsqloptions_clause
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
            $$->append($5);
            }
    ;

fetch_statement:
        SQL_TOKEN_FETCH cursor SQL_TOKEN_INTO target_commalist
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);}
    ;

insert_statement:
        SQL_TOKEN_INSERT SQL_TOKEN_INTO table_node opt_column_ref_commalist values_or_query_spec
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
            $$->append($5);}
    ;
values_or_query_spec:
        SQL_TOKEN_VALUES '(' row_value_constructor_commalist ')'
        {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;

row_value_constructor_commalist:
        row_value_constructor
        {
            $$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);
        }
    |   row_value_constructor_commalist ',' row_value_constructor
        {    
            $1->append($3);
            $$ = $1;
        }
    ;

row_value_constructor:
            row_value_constructor_elem
    ;
row_value_constructor_elem:
        value_exp
    /* ECSQL or SQL doesnt support DEFAULT in boolean expression
    |   SQL_TOKEN_DEFAULT
    */
    ;


rollback_statement:
        SQL_TOKEN_ROLLBACK SQL_TOKEN_WORK
        {
        $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
    }
    ;


        /* INTO target_commalist herausgenommen */
select_statement_into:
        SQL_TOKEN_SELECT opt_all_distinct selection SQL_TOKEN_INTO target_commalist table_exp
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
            $$->append($5);
            $$->append($6); }
    ;

opt_all_distinct:
            {$$ = SQL_NEW_RULE;}
        |    SQL_TOKEN_ALL
        |    SQL_TOKEN_DISTINCT

    ;
assignment_commalist:
            assignment
            {$$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);}
    |       assignment_commalist ',' assignment
            {$1->append($3);
            $$ = $1;}
    ;

assignment:
        column_ref SQL_EQUAL update_source
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);}
    ;
update_source:
        value_exp
      | SQL_TOKEN_DEFAULT
    ;
update_statement_searched:
        SQL_TOKEN_UPDATE table_ref SQL_TOKEN_SET assignment_commalist opt_where_clause opt_ecsqloptions_clause
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
            $$->append($5);
            $$->append($6);
            }
    ;

target_commalist:
        target
            {$$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);}
    |       target_commalist ',' target
            {$1->append($3);
            $$ = $1;}
    ;

target:
        parameter_ref
    ;

opt_where_clause:
        /* empty */             {$$ = SQL_NEW_RULE;}
    |       where_clause
    ;

    /* query expressions */


/* SELECT STATEMENT */
single_select_statement:
        SQL_TOKEN_SELECT opt_all_distinct selection table_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
        | values_or_query_spec 
    ;

selection:
        '*'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("*", SQL_NODE_PUNCTUATION));
        }
    |    scalar_exp_commalist
    ;
opt_result_offset_clause:
        /* empty */ {$$ = SQL_NEW_RULE;}
    |    result_offset_clause
    ;    
result_offset_clause:
    SQL_TOKEN_OFFSET offset_row_count row_or_rows
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
    }
    ;
opt_fetch_first_row_count:
        /* empty */ {$$ = SQL_NEW_RULE;}
    |    fetch_first_row_count
    ;
first_or_next:
        SQL_TOKEN_FIRST 
    |    SQL_TOKEN_NEXT
    ;
row_or_rows:
        SQL_TOKEN_ROW 
    |    SQL_TOKEN_ROWS
    ;
opt_fetch_first_clause:
        /* empty */ {$$ = SQL_NEW_RULE;}
    |    fetch_first_clause
    ;
fetch_first_clause:
    SQL_TOKEN_FETCH first_or_next opt_fetch_first_row_count row_or_rows SQL_TOKEN_ONLY
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4);
        $$->append($5);
    }
    ;
offset_row_count:
    literal
    ;
fetch_first_row_count:
    literal
    ;

opt_limit_offset_clause:
        /* empty */ {$$ = SQL_NEW_RULE;}
    |    limit_offset_clause
    ;
opt_offset:
        /* empty */ {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_OFFSET num_value_exp
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
    ;
limit_offset_clause:
    SQL_TOKEN_LIMIT num_value_exp opt_offset
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
    }
    ;
table_exp:
        from_clause opt_where_clause opt_group_by_clause opt_having_clause opt_window_clause opt_order_by_clause opt_limit_offset_clause opt_result_offset_clause opt_fetch_first_clause opt_ecsqloptions_clause
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
            $$->append($5);
            $$->append($6);
            $$->append($7);
            $$->append($8);
            $$->append($9);
            $$->append($10);
        }
    ;

from_clause:
        SQL_TOKEN_FROM table_ref_commalist
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;

table_ref_commalist:

        table_ref
            {$$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);}
    |       table_ref_commalist ',' table_ref
            {$1->append($3);
            $$ = $1;}
    ;

opt_as:
        /* empty */ {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_AS
    ;
opt_row:
        /* empty */ {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_ROW
    ;
table_primary_as_range_column:
        {
            $$ = SQL_NEW_RULE;
        }
    |   opt_as SQL_TOKEN_NAME opt_column_commalist
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    ;
opt_only:
        /* empty */ {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_ONLY


table_ref:
        opt_only table_node table_primary_as_range_column
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    |   opt_only subquery range_variable opt_column_ref_commalist
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    |    joined_table
    /* ECSQL: Absolute sytax for outer join.
    |    '{' SQL_TOKEN_OJ joined_table '}'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("{", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3);
            $$->append($4 = newNode("}", SQL_NODE_PUNCTUATION));
        }
    
    |   opt_only'(' joined_table ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));

        }    
    */
    ;
where_clause:
        SQL_TOKEN_WHERE search_condition
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;

opt_group_by_clause:
        /* empty */      {$$ = SQL_NEW_RULE;}
    |   SQL_TOKEN_GROUP SQL_TOKEN_BY value_exp_commalist
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);}
    ;

opt_having_clause:
        /* empty */                 {$$ = SQL_NEW_RULE;}
    |       SQL_TOKEN_HAVING search_condition
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);}
    ;

    /* search conditions */
truth_value:
        SQL_TOKEN_TRUE
      | SQL_TOKEN_FALSE
      | SQL_TOKEN_UNKNOWN
      | SQL_TOKEN_NULL
      ;
boolean_primary:
        predicate
    |   unary_predicate
    |   '(' search_condition ')'
        { // boolean_primary: rule 2
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;

// cannot be in the predicate rule as it would conflict with other predicates
//starting with a value_exp
unary_predicate:
    // in order to support predicates made of unary boolean expressions.
    // validation that the value is of bool type
    // has to be done in the post-processing steps
    value_exp
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
    }
    ;

parenthesized_boolean_value_expression:
   '(' search_condition ')'
    { // boolean_primary: rule 2
        $$ = SQL_NEW_RULE;
        $$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
        $$->append($2);
        $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
    }
    ;
boolean_test:
        boolean_primary
    |    boolean_primary SQL_TOKEN_IS sql_not truth_value
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    ;
boolean_factor:
        boolean_test
    |   SQL_TOKEN_NOT boolean_test
        { // boolean_factor: rule 1
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
boolean_term:
        boolean_factor
    |    boolean_term SQL_TOKEN_AND boolean_factor
        {
            $$ = SQL_NEW_RULE; // boolean_term: rule 1
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    ;
search_condition:
        boolean_term
    |    search_condition SQL_TOKEN_OR boolean_term
        {
            $$ = SQL_NEW_RULE; // search_condition
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    ;

predicate:
        comparison_predicate
    |   between_predicate
    |   all_or_any_predicate
    |   existence_test
    |   unique_test
    |   test_for_null
    |   in_predicate
    |   like_predicate
    |   rtreematch_predicate
    ;



comparison_predicate_part_2:    
        comparison row_value_constructor
        {
            $$ = SQL_NEW_RULE; // comparison_predicate: rule 1
            $$->append($1);
            $$->append($2);
        }
comparison_predicate:
        row_value_constructor comparison row_value_constructor
        {
            $$ = SQL_NEW_RULE; // comparison_predicate: rule 1
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    |    comparison row_value_constructor
        {
            if(xxx_pGLOBAL_SQLPARSER->inPredicateCheck()) // comparison_predicate: rule 2
            {
                $$ = SQL_NEW_RULE;
                sal_Int16 nErg = xxx_pGLOBAL_SQLPARSER->buildPredicateRule($$,$2,$1);
                if(nErg == 1)
                {
                    OSQLParseNode* pTemp = $$;
                    $$ = pTemp->removeAt((sal_uInt32)0);
                    delete pTemp;
                }
                else
                {
                    delete $$;
                    YYABORT;
                }
            }
            else
            {
                YYERROR;
            }
        }
    ;
comparison:
        SQL_LESS
      | SQL_NOTEQUAL
      | SQL_EQUAL
      | SQL_GREAT
      | SQL_LESSEQ
      | SQL_GREATEQ
      /* IS [NOT] DISTINCT FROM operator not supported in ECSQL yet
      | SQL_TOKEN_IS sql_not SQL_TOKEN_DISTINCT SQL_TOKEN_FROM
          {
          $$ = SQL_NEW_RULE;
          $$->append($1);
          $$->append($2);
          $$->append($3);
          $$->append($4);
        }
        */
      | SQL_TOKEN_IS sql_not
        {
          $$ = SQL_NEW_RULE;
          $$->append($1);
          $$->append($2);
        }
    ;
between_predicate_part_2:
    sql_not SQL_TOKEN_BETWEEN row_value_constructor SQL_TOKEN_AND row_value_constructor
        {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck()) // between_predicate: rule 2 
            {
                $$ = SQL_NEW_RULE;
                
                sal_Int16 nErg = xxx_pGLOBAL_SQLPARSER->buildPredicateRule($$,$3,$2,$5);
                if(nErg == 1)
                {
                    OSQLParseNode* pTemp = $$;
                    $$ = pTemp->removeAt((sal_uInt32)0);
                    OSQLParseNode* pColumnRef = $$->removeAt((sal_uInt32)0);
                    $$->insert(0,$1);
                    OSQLParseNode* pBetween_predicate = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::between_predicate));
                    pBetween_predicate->append(pColumnRef);
                    pBetween_predicate->append($$);
                    $$ = pBetween_predicate;
                    
                    delete pTemp;
                    delete $4;
                }
                else
                {
                    delete $$;
                    YYABORT;
                }
            }
            else
            {
                $$ = SQL_NEW_RULE; // between_predicate: rule 1
                $$->append($1);
                $$->append($2);
                $$->append($3);
                $$->append($4);
                $$->append($5);
            }
        }
between_predicate:
        row_value_constructor between_predicate_part_2
        {    
            $$ = SQL_NEW_RULE; // between_predicate: rule 1
            $$->append($1);
            $$->append($2);
        }
    /* This seem wrong why would we need Z BETWEEN X AND Y without Z
    |    between_predicate_part_2
    */
    ;
character_like_predicate_part_2:
    sql_not SQL_TOKEN_LIKE string_value_exp opt_escape
        {
            $$ = SQL_NEW_RULE; // like_predicate: rule 1
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    ;
other_like_predicate_part_2:
    sql_not SQL_TOKEN_LIKE value_exp_primary opt_escape
        {
            $$ = SQL_NEW_RULE; // like_predicate: rule 1
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    ;
like_predicate:
        row_value_constructor character_like_predicate_part_2
        {
            $$ = SQL_NEW_RULE; // like_predicate: rule 1
            $$->append($1);
            $$->append($2);
        }
    |    row_value_constructor other_like_predicate_part_2
        {
            $$ = SQL_NEW_RULE;  // like_predicate: rule 3
            $$->append($1);
            $$->append($2);
        }
    |    character_like_predicate_part_2
        {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())  // like_predicate: rule 5
            {
                OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

                $$ = SQL_NEW_RULE;
                $$->append(pColumnRef);
                $$->append($1);
                OSQLParseNode* p2nd = $1->removeAt(2);
                OSQLParseNode* p3rd = $1->removeAt(2);
                if ( !xxx_pGLOBAL_SQLPARSER->buildLikeRule($1,p2nd,p3rd) )
                {
                    delete $$;
                    YYABORT;
                }
                $1->append(p3rd);
            }
            else
                YYERROR;
        }
    |    other_like_predicate_part_2
        {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck()) // like_predicate: rule 6
            {
                OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

                $$ = SQL_NEW_RULE;
                $$->append(pColumnRef);
                $$->append($1);
                OSQLParseNode* p2nd = $1->removeAt(2);
                OSQLParseNode* p3rd = $1->removeAt(2);
                if ( !xxx_pGLOBAL_SQLPARSER->buildLikeRule($1,p2nd,p3rd) )
                {
                    delete $$;
                    YYABORT;
                }
                $1->append(p3rd);
            }
            else
                YYERROR;
        }
    ;

opt_escape:
        /* empty */                 {$$ = SQL_NEW_RULE;}
    |   SQL_TOKEN_ESCAPE string_value_exp
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);}
    /* This syntax is not compliant with SQL-99 and doesn't seem to be necessary for us anyways. Might remove this completely later.
    |    '{' SQL_TOKEN_ESCAPE SQL_TOKEN_STRING '}'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("{", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3);
            $$->append($4 = newNode("}", SQL_NODE_PUNCTUATION));
        } */
    ;

null_predicate_part_2:
    SQL_TOKEN_IS sql_not SQL_TOKEN_NULL
    {
        $$ = SQL_NEW_RULE; // test_for_null: rule 1
        $$->append($1);
        $$->append($2);
        $$->append($3);
    }
    ;
test_for_null:
        row_value_constructor null_predicate_part_2
        {
            $$ = SQL_NEW_RULE; // test_for_null: rule 1
            $$->append($1);
            $$->append($2);
        }
    |    null_predicate_part_2
        {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())// test_for_null: rule 2
            {
                OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

                $$ = SQL_NEW_RULE;
                $$->append(pColumnRef);
                $$->append($1);
            }
            else
                YYERROR;
        }
    ;
in_predicate_value:
        subquery
        {$$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | '(' value_exp_commalist ')'
        {$$ = SQL_NEW_RULE;
            $$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;
in_predicate_part_2:
    sql_not SQL_TOKEN_IN in_predicate_value
    {
        $$ = SQL_NEW_RULE;// in_predicate: rule 1
        $$->append($1);
        $$->append($2);
        $$->append($3);
    }
    ;
in_predicate:
        row_value_constructor in_predicate_part_2
        {
            $$ = SQL_NEW_RULE;// in_predicate: rule 1
            $$->append($1);
            $$->append($2);
        }
    |    in_predicate_part_2
        {
            if ( xxx_pGLOBAL_SQLPARSER->inPredicateCheck() )// in_predicate: rule 2
            {
                OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

                $$ = SQL_NEW_RULE;
                $$->append(pColumnRef);
                $$->append($1);
            }
            else
                YYERROR;
        }
    ;
quantified_comparison_predicate_part_2:
    comparison any_all_some subquery
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
    }
    ;
all_or_any_predicate:
        row_value_constructor quantified_comparison_predicate_part_2
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    /*Seem to be not valid for SQL 
    |    quantified_comparison_predicate_part_2
        {
            if(xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
            {
                OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

                $$ = SQL_NEW_RULE;
                $$->append(pColumnRef);
                $$->append($1);
            }
            else
                YYERROR;
        }
    */
    ;

rtreematch_predicate:
       row_value_constructor rtreematch_predicate_part_2
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;

rtreematch_predicate_part_2:
       sql_not SQL_TOKEN_MATCH fct_spec
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    ;

any_all_some:
            SQL_TOKEN_ANY
    |       SQL_TOKEN_ALL
    |       SQL_TOKEN_SOME
    ;

existence_test:
        SQL_TOKEN_EXISTS subquery
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
unique_test:
        SQL_TOKEN_UNIQUE subquery
        {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);}
    ;
subquery:
        '(' select_statement ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;

    /* scalar expressions */
scalar_exp_commalist:
        select_sublist
        {
            $$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);
        }
    |   scalar_exp_commalist ',' select_sublist
        {
            $1->append($3);
            $$ = $1;
        }
    ;
select_sublist:
/*
        table_node '.' '*'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($3 = newNode("*", SQL_NODE_PUNCTUATION));
        }
    |*/
        derived_column

    ;

parameter_ref:
        parameter
    ;

/*
op_like:
        '*'
        {
            $$ = newNode("*", SQL_NODE_PUNCTUATION);
        }
    |    '?'
        {
            $$ = newNode("?", SQL_NODE_PUNCTUATION);
        }
    |   op_like    '*'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("*", SQL_NODE_PUNCTUATION));
            xxx_pGLOBAL_SQLPARSER->reduceLiteral($$, sal_False);
        }
    |    op_like '?'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("?", SQL_NODE_PUNCTUATION));
            xxx_pGLOBAL_SQLPARSER->reduceLiteral($$, sal_False);
        }
    ;
*/

literal:
/*        SQL_TOKEN_STRING
    |   */SQL_TOKEN_INT
    |   SQL_TOKEN_REAL_NUM
    |   SQL_TOKEN_INTNUM
    |   SQL_TOKEN_APPROXNUM
    |    SQL_TOKEN_ACCESS_DATE
/*    rules for predicate check */
    |    literal SQL_TOKEN_STRING
        {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
            {
                $$ = SQL_NEW_RULE;
                $$->append($1);
                $$->append($2);
                xxx_pGLOBAL_SQLPARSER->reduceLiteral($$, sal_True);
            }
            else
                YYERROR;
        }
    |    literal SQL_TOKEN_INT
        {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
            {
                $$ = SQL_NEW_RULE;
                $$->append($1);
                $$->append($2);
                xxx_pGLOBAL_SQLPARSER->reduceLiteral($$, sal_True);
            }
            else
                YYERROR;
        }
    |    literal SQL_TOKEN_REAL_NUM
        {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
            {
                $$ = SQL_NEW_RULE;
                $$->append($1);
                $$->append($2);
                xxx_pGLOBAL_SQLPARSER->reduceLiteral($$, sal_True);
            }
            else
                YYERROR;
        }
    |    literal SQL_TOKEN_APPROXNUM
        {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
            {
                $$ = SQL_NEW_RULE;
                $$->append($1);
                $$->append($2);
                xxx_pGLOBAL_SQLPARSER->reduceLiteral($$, sal_True);
            }
            else
                YYERROR;
        }
    ;

    /* miscellaneous */
as_clause:
        /* empty */ {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_AS column
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    |    column
    ;
position_exp:
        SQL_TOKEN_POSITION '(' value_exp SQL_TOKEN_IN value_exp ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5);
            $$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    |    SQL_TOKEN_POSITION '(' value_exp_commalist ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;
num_value_fct:
        position_exp
    |    extract_exp
    |    length_exp
    ;
char_length_exp:
        SQL_TOKEN_CHAR_LENGTH '(' value_exp ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    |    SQL_TOKEN_SQL_TOKEN_INTNUM '(' value_exp ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    
    ;
octet_length_exp:
        SQL_TOKEN_OCTET_LENGTH '(' value_exp ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;
bit_length_exp:
        SQL_TOKEN_BIT_LENGTH '(' value_exp ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;
length_exp:
        char_length_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | octet_length_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | bit_length_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    ;
datetime_field:
        non_second_datetime_field
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | SQL_TOKEN_SECOND
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    ;
extract_field:
/*time zones not supported in ECSQL       time_zone_field
      |*/ datetime_field
      |    value_exp
    ;
/*
time_zone_field:
        SQL_TOKEN_TIMEZONE_HOUR
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | SQL_TOKEN_TIMEZONE_MINUTE
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    ;
    */
/*
extract_source:
        datetime_value_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | interval_value_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        } 
    ;
*/
extract_exp:
        SQL_TOKEN_EXTRACT '(' extract_field SQL_TOKEN_FROM value_exp ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5);
            $$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;
unsigned_value_spec:
        general_value_spec
    |    literal
    ;
general_value_spec:
        parameter
    | SQL_TOKEN_NULL
    | SQL_TOKEN_FALSE
    | SQL_TOKEN_TRUE
    | SQL_TOKEN_VALUE
    | SQL_TOKEN_CURRENT_CATALOG
    | SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP
    | SQL_TOKEN_CURRENT_PATH
    | SQL_TOKEN_CURRENT_ROLE
    | SQL_TOKEN_CURRENT_SCHEMA
    ;
fct_spec:
        general_set_fct
  /*  |   fold */
    |    function_name '(' ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    |    function_name0 '(' ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
        }
       |    function_name '(' function_args_commalist ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    |    function_name12 '(' function_args_commalist ')'
        {
            if ( $3->count() == 1 || $3->count() == 2 )
            {
                $$ = SQL_NEW_RULE;
                $$->append($1);
                $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
                $$->append($3);
                $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
            }
            else
                YYERROR;
        }
    |
        function_name '(' opt_all_distinct function_arg ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
        };

    ;
function_name0:
        date_function_0Argument
    ;
function_name12:
        SQL_TOKEN_WEEK
    ;

function_name:
    SQL_TOKEN_NAME
    ;

date_function_0Argument:
        SQL_TOKEN_CURDATE             
/*time not supported in ECSQL    |    SQL_TOKEN_CURTIME             */
    |    SQL_TOKEN_NOW                 
    ;
	/*
date_function_1Argument:
        SQL_TOKEN_DAYOFWEEK           
    |    SQL_TOKEN_DAYOFMONTH          
    |    SQL_TOKEN_DAYOFYEAR           
    |    SQL_TOKEN_MONTH               
    |    SQL_TOKEN_DAYNAME             
    |    SQL_TOKEN_MONTHNAME           
    |    SQL_TOKEN_QUARTER             
    |    SQL_TOKEN_HOUR                
    |    SQL_TOKEN_MINUTE              
    |    SQL_TOKEN_SECOND         
    |    SQL_TOKEN_YEAR
    |    SQL_TOKEN_DAY
    |    SQL_TOKEN_TIMEVALUE
    |    SQL_TOKEN_DATEVALUE
    ;
    
date_function:
        SQL_TOKEN_TIMESTAMPADD        
    |    SQL_TOKEN_TIMESTAMPDIFF       
    ;
	*/
    
window_function:
    window_function_type SQL_TOKEN_OVER window_name_or_specification
    {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
    }
    ;
window_function_type :
    SQL_TOKEN_ROW_NUMBER '(' ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    | general_set_fct
    | ntile_function
    | lead_or_lag_function
    | first_or_last_value_function
    | nth_value_function
;
ntile_function :
    SQL_TOKEN_NTILE '(' number_of_tiles ')'
    {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
    }
    ;
dynamic_parameter_specification:
    parameter
    ;
simple_value_specification:
    literal
    ;
number_of_tiles :
        simple_value_specification
    |    dynamic_parameter_specification
    ;
opt_lead_or_lag_function:
    /* empty */      {$$ = SQL_NEW_RULE;}
    | ',' offset 
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode(",", SQL_NODE_PUNCTUATION));
            $$->append($2);
        }
    | ',' offset ',' default_expression
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode(",", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = newNode(",", SQL_NODE_PUNCTUATION));
            $$->append($4);
        }
    ;
opt_null_treatment:
        /* empty */      {$$ = SQL_NEW_RULE;}
    |    null_treatment
    ;
    
lead_or_lag_function:
    lead_or_lag '(' lead_or_lag_extent opt_lead_or_lag_function ')'    opt_null_treatment
    {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
            $$->append($6);
    }
    ;
lead_or_lag:
        SQL_TOKEN_LEAD 
    |    SQL_TOKEN_LAG
    ;
lead_or_lag_extent:
    value_exp
    ;
offset:
    SQL_TOKEN_INTNUM
    ;
default_expression:
    value_exp
    ;
null_treatment:
        SQL_TOKEN_RESPECT SQL_TOKEN_NULLS 
    |    SQL_TOKEN_IGNORE SQL_TOKEN_NULLS
    ;
first_or_last_value_function:
    first_or_last_value '(' value_exp ')' opt_null_treatment
    {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
            $$->append($5);
    }
    ;
first_or_last_value :
        SQL_TOKEN_FIRST_VALUE 
    |    SQL_TOKEN_LAST_VALUE
    ;
opt_from_first_or_last:
        /* empty */      {$$ = SQL_NEW_RULE;}
    |    from_first_or_last
    ;
nth_value_function:
    SQL_TOKEN_NTH_VALUE '(' value_exp ',' nth_row ')' opt_from_first_or_last opt_null_treatment
    {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(",", SQL_NODE_PUNCTUATION));
            $$->append($5);
            $$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));
            $$->append($7);
            $$->append($8);
    }
    ;
nth_row:
        simple_value_specification
    |    dynamic_parameter_specification
    ;
from_first_or_last:
        SQL_TOKEN_FROM SQL_TOKEN_FIRST
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    |    SQL_TOKEN_FROM SQL_TOKEN_LAST
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
window_name:
    SQL_TOKEN_NAME
    ;
window_name_or_specification:
        window_name
    |    in_line_window_specification
    ;
in_line_window_specification: 
    window_specification
    ;
opt_window_clause:
        /* empty */      {$$ = SQL_NEW_RULE;}
    |    window_clause
    ;
window_clause:
    SQL_TOKEN_WINDOW window_definition_list
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
    ;
window_definition_list:
        window_definition_list ',' window_definition
            {$1->append($3);
            $$ = $1;}
    |    window_definition
            {$$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);}
    ;
window_definition:
    new_window_name SQL_TOKEN_AS window_specification
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
    }
    ;
new_window_name:
    window_name
    ;
window_specification:
    '(' window_specification_details ')'
    {
        $$ = SQL_NEW_RULE;
        $$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
        $$->append($2);
        $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
    }
    ;
opt_existing_window_name:
        /* empty */      {$$ = SQL_NEW_RULE;}
    |    existing_window_name
    ;
opt_window_partition_clause:    
    /* empty */      {$$ = SQL_NEW_RULE;}
    |    window_partition_clause
    ;
opt_window_frame_clause:    
    /* empty */      {$$ = SQL_NEW_RULE;}
    |    window_frame_clause
    ;
window_specification_details:
    opt_existing_window_name
    opt_window_partition_clause
    opt_order_by_clause
    opt_window_frame_clause
    ;
existing_window_name:
    window_name
    ;
window_partition_clause:
    SQL_TOKEN_PARTITION SQL_TOKEN_BY window_partition_column_reference_list
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
    }
    ;
window_partition_column_reference_list:
    window_partition_column_reference_list ',' window_partition_column_reference
            {$1->append($3);
            $$ = $1;}
    |    window_partition_column_reference
            {$$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);}
    ;
window_partition_column_reference:
    column_ref opt_collate_clause
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
    ;
opt_window_frame_exclusion:
    /* empty */      {$$ = SQL_NEW_RULE;}
    |    window_frame_exclusion
    ;
window_frame_clause:
    window_frame_units window_frame_extent opt_window_frame_exclusion
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
    }    
    ;
window_frame_units:
        SQL_TOKEN_ROWS
    |    SQL_TOKEN_RANGE
    ;
window_frame_extent:
        window_frame_start
    |    window_frame_between
    ;
window_frame_start:
        SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }    
    |    window_frame_preceding
    |    SQL_TOKEN_CURRENT SQL_TOKEN_ROW
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }    
    ;
window_frame_preceding:
    unsigned_value_spec SQL_TOKEN_PRECEDING
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
    ;
window_frame_between:
    SQL_TOKEN_BETWEEN window_frame_bound_1 SQL_TOKEN_AND window_frame_bound_2
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4);
    }
    ;
window_frame_bound_1:
    window_frame_bound
    ;
window_frame_bound_2:
    window_frame_bound
    ;
window_frame_bound:
    window_frame_start
    | SQL_TOKEN_UNBOUNDED SQL_TOKEN_FOLLOWING
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
    | window_frame_following
    ;
window_frame_following:
    unsigned_value_spec SQL_TOKEN_FOLLOWING
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
    ;
window_frame_exclusion:
        SQL_TOKEN_EXCLUDE SQL_TOKEN_CURRENT SQL_TOKEN_ROW
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    |    SQL_TOKEN_EXCLUDE SQL_TOKEN_GROUP
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    |    SQL_TOKEN_EXCLUDE SQL_TOKEN_TIES
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    |    SQL_TOKEN_EXCLUDE SQL_TOKEN_NO SQL_TOKEN_OTHERS
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    ;
	/*
op_parameter:
        {$$ = SQL_NEW_RULE;}
    |    '?' SQL_EQUAL
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("?", SQL_NODE_PUNCTUATION));
            $$->append($2);
        }
    ;
	*/
general_set_fct:
        set_fct_type '(' opt_all_distinct function_arg ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    |    SQL_TOKEN_COUNT '(' '*' ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3 = newNode("*", SQL_NODE_PUNCTUATION));
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    |    SQL_TOKEN_COUNT '(' opt_all_distinct function_arg ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;

set_fct_type:
        SQL_TOKEN_AVG
    |   SQL_TOKEN_MAX
    |   SQL_TOKEN_MIN
    |   SQL_TOKEN_SUM
    |   SQL_TOKEN_EVERY
    |   SQL_TOKEN_ANY
    |   SQL_TOKEN_SOME
    ;

outer_join_type:
        SQL_TOKEN_LEFT %prec SQL_TOKEN_LEFT
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | SQL_TOKEN_RIGHT %prec SQL_TOKEN_RIGHT
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | SQL_TOKEN_FULL %prec SQL_TOKEN_FULL
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      ;
join_condition:
        SQL_TOKEN_ON search_condition
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
        ;
join_spec:
        join_condition
      | named_columns_join
      ;
join_type:
        /* empty */     {$$ = SQL_NEW_RULE;}
      | SQL_TOKEN_INNER
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | outer_join_type
      | outer_join_type SQL_TOKEN_OUTER
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
cross_union:
        table_ref SQL_TOKEN_CROSS SQL_TOKEN_JOIN table_ref
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    ;

qualified_join:
        /* if SQL_TOKEN_NATURAL, then no join_spec */
        table_ref SQL_TOKEN_NATURAL join_type SQL_TOKEN_JOIN table_ref
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
            $$->append($5);
        }

    |    table_ref join_type SQL_TOKEN_JOIN table_ref join_spec
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
            $$->append($5);
        }
    |    cross_union
    ;

/*ECSQL extension*/
ecrelationship_join:
        table_ref join_type SQL_TOKEN_JOIN table_ref SQL_TOKEN_USING table_node op_relationship_direction
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
            $$->append($5);
            $$->append($6);
            $$->append($7);
        }
    ;

op_relationship_direction:
        {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_FORWARD
    |    SQL_TOKEN_BACKWARD
    ;
joined_table:
        ecrelationship_join
    |    qualified_join
    ;
named_columns_join:
        SQL_TOKEN_USING '(' column_commalist ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;


all:
    /* empty*/ {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_ALL
    
    ;
scalar_subquery:
        subquery
    ;
cast_operand:
        value_exp
    ;

cast_target:
    SQL_TOKEN_BINARY 
    | SQL_TOKEN_BOOLEAN
    | SQL_TOKEN_DOUBLE
    | SQL_TOKEN_INTEGER
    | SQL_TOKEN_INT
    | SQL_TOKEN_LONG
    | SQL_TOKEN_INT64
    | SQL_TOKEN_STRING
    | SQL_TOKEN_DATETIME
    | SQL_TOKEN_DATE
    | SQL_TOKEN_TIMESTAMP
    | SQL_TOKEN_NAME
  ;

cast_spec:
      SQL_TOKEN_CAST '(' cast_operand SQL_TOKEN_AS cast_target ')'
      {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5);
            $$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;
value_exp_primary:
        unsigned_value_spec
      | fct_spec 
      | ecclassid_fct_spec 
	  | column_ref
      | scalar_subquery
      | case_expression
      | window_function
      | '(' value_exp ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
        }
      | cast_spec
    ;


/* GetECClassId()*/
ecclassid_fct_spec:
        SQL_TOKEN_GETECCLASSID '(' ')'
	{
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
        $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
	}
        |
	property_path '.' SQL_TOKEN_GETECCLASSID '(' ')'
	{
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
        $$->append($3);
        $$->append($4 = newNode("(", SQL_NODE_PUNCTUATION));
        $$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
	}
        ;

num_primary:
        value_exp_primary
      | num_value_fct
    ;

factor:
        num_primary
    |    '-' num_primary  %prec SQL_TOKEN_UMINUS
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("-", SQL_NODE_PUNCTUATION));
            $$->append($2);
        }
    |    '+' num_primary  %prec SQL_TOKEN_UMINUS
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("+", SQL_NODE_PUNCTUATION));
            $$->append($2);
        }
    ;

term:
        factor
      | term '*' factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("*", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | term '/' factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("/", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | term '%' factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("%", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      ;

num_value_exp:
        term
      | num_value_exp '+' term
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("+", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | num_value_exp '-' term
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("-", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      ;
datetime_primary:
/*        value_exp_primary
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      |*/ datetime_value_fct
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    ;
datetime_value_fct:
        SQL_TOKEN_CURRENT_DATE
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }

      | SQL_TOKEN_CURRENT_TIMESTAMP
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | SQL_TOKEN_DATE string_value_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
      | SQL_TOKEN_TIMESTAMP string_value_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }

    ;
/* No time zone support in ECSQL (at least for now)
time_zone:
        SQL_TOKEN_AT time_zone_specifier
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
time_zone_specifier:
        SQL_TOKEN_LOCAL
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | SQL_TOKEN_TIME SQL_TOKEN_ZONE interval_value_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
      ;
      */
datetime_factor:
        datetime_primary
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
/*    |    datetime_primary time_zone
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        } */
    ;
datetime_term:
        datetime_factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    ;

/*
interval_term:
        literal
      | interval_term '*' factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("*", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | interval_term '/' factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("/", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
    ;
*/
datetime_value_exp:
        datetime_term
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
/*      | interval_value_exp '+' datetime_term
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("+", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | datetime_value_exp '+' interval_term
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("+", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | datetime_value_exp '-' interval_term
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("-", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
*/    ;
/*
interval_value_exp:
        interval_term
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | interval_value_exp '+' interval_term
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("+", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | interval_value_exp '-' interval_term
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("-", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | '(' datetime_value_exp '-' datetime_term ')' interval_qualifier
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = newNode("-", SQL_NODE_PUNCTUATION));
            $$->append($4);
            $$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
            $$->append($6);
        }
    ;
    */
non_second_datetime_field:
        SQL_TOKEN_YEAR
    |    SQL_TOKEN_MONTH
    |    SQL_TOKEN_DAY
    |    SQL_TOKEN_HOUR
    |    SQL_TOKEN_MINUTE
    ;
start_field:
        non_second_datetime_field opt_paren_precision
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
end_field:
        non_second_datetime_field
    |    SQL_TOKEN_SECOND opt_paren_precision
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;

single_datetime_field:
        non_second_datetime_field opt_paren_precision
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    |    SQL_TOKEN_SECOND opt_paren_precision_scale
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;

interval_qualifier:
        start_field SQL_TOKEN_TO end_field
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    |    single_datetime_field
    ;


value_exp_commalist:
        value_exp
            {$$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);}
    |   value_exp_commalist ',' value_exp
            {$1->append($3);
            $$ = $1;}
    /*    this rule is only valid if we check predicates */
    |   value_exp_commalist ';' value_exp
        {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
            {
                $1->append($3);
                $$ = $1;
            }
            else
                YYERROR;
        }
    ;
function_arg:
    
        result 
    /* Follow is used for advance features and funtions that we donot support
    |    value_exp comparison value_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    |    value_exp SQL_TOKEN_USING value_exp comparison value_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2)
            $$->append($3);
            $$->append($4);
        }
    |    value_exp SQL_TOKEN_BY value_exp_commalist
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    */
    ;
function_args_commalist:
        function_arg
            {$$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);}
    |   function_args_commalist ',' function_arg
            {$1->append($3);
            $$ = $1;}
    /*    this rule is only valid if we check predicates */
    |   function_args_commalist ';' function_arg
        {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
            {
                $1->append($3);
                $$ = $1;
            }
            else
                YYERROR;
        }
    ;
    
value_exp:
        num_value_exp
      | string_value_exp
      | datetime_value_exp
    ;
string_value_exp:
        char_value_exp
/*      | bit_value_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
*/    ;
char_value_exp:
        char_factor
    |    concatenation
    ;
concatenation:
        char_value_exp '+' char_factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("+", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
    |    value_exp SQL_CONCAT value_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    ;

char_primary:
            SQL_TOKEN_STRING
      |        string_value_fct
    ;
collate_clause:
        SQL_TOKEN_COLLATE table_node
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
char_factor:
        char_primary
    |    char_primary collate_clause
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
string_value_fct:
        char_value_fct
      | bit_value_fct
    ;
bit_value_fct:
        bit_substring_fct
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    ;
bit_substring_fct:
        SQL_TOKEN_SUBSTRING '(' bit_value_exp SQL_TOKEN_FROM string_value_exp for_length ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5);
            $$->append($6);
            $$->append($7 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;
bit_value_exp:
        bit_factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    ;
/*
            bit_concatenation
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      |
bit_concatenation:
        bit_value_exp '+' bit_factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("+", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
    ;
*/
bit_factor:
        bit_primary
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    ;
bit_primary:
    {$$ = SQL_NEW_RULE;}
/*        value_exp_primary
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | string_value_fct
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }*/
    ;
char_value_fct:
        char_substring_fct
      | fold
      | form_conversion
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
      | char_translation
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    ;
for_length:
        {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_FOR value_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
char_substring_fct:
        SQL_TOKEN_SUBSTRING '(' value_exp SQL_TOKEN_FROM value_exp for_length ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5);
            $$->append($6);
            $$->append($7 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    |    SQL_TOKEN_SUBSTRING '(' value_exp_commalist ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;
upper_lower:
        SQL_TOKEN_UPPER
    |    SQL_TOKEN_LOWER
    ;
fold:
        upper_lower '(' value_exp ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;
form_conversion:
        SQL_TOKEN_CONVERT '(' string_value_exp SQL_TOKEN_USING table_node ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5);
            $$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    |    SQL_TOKEN_CONVERT '(' cast_operand ',' cast_target ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($2 = newNode(",", SQL_NODE_PUNCTUATION));
            $$->append($5);
            $$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;
char_translation:
        SQL_TOKEN_TRANSLATE '(' string_value_exp SQL_TOKEN_USING table_node ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5);
            $$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;

derived_column:
        value_exp as_clause
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
/* Tabellenname */
table_node:
        table_name
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    |    schema_name
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    |    catalog_name
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
;
catalog_name:
        SQL_TOKEN_NAME '.' schema_name
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
    |    SQL_TOKEN_NAME ':' schema_name
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode(":", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
;
schema_name:
        SQL_TOKEN_NAME '.' table_name 
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
;

table_name:
        SQL_TOKEN_NAME
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
;

opt_column_array_idx:
		{$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_ARRAY_INDEX
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
;	

property_path:
        property_path_entry
        {
            $$ = SQL_NEW_DOTLISTRULE;
            $$->append ($1);
        }
    |   property_path '.' property_path_entry %prec '.'
        {
			auto last = $1->getLast();
			if (last)
				{
				if (last->getFirst()->getNodeType() == SQL_NODE_PUNCTUATION) //'*'
					{
					SQLyyerror("'*' can only occur at the end of property path\n");
					YYERROR;
					}
				}

            $1->append($3);
            $$ = $1;
        }
    ;	

property_path_entry:
        SQL_TOKEN_NAME opt_column_array_idx
        {
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		} 
	|   '*'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("*", SQL_NODE_PUNCTUATION));
        } 
    ;

column_ref:
		property_path
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
/*    |       table_node '.' column_val %prec '.'
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($3);}
    
    |       SQL_TOKEN_NAME '.' column_val %prec '.'
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($3);
            }
    |       SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' column_val %prec '.'
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($5);}
    |       SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' column_val %prec '.'
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2= newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($5);
            $$->append($6 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($7);
            }
    |       SQL_TOKEN_NAME ':' SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' column_val %prec '.'
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2= newNode(":", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($5);
            $$->append($6 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($7);
            }
    |       SQL_TOKEN_NAME ';' SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' column_val
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2= newNode(";", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($5);
            $$->append($6 = newNode(".", SQL_NODE_PUNCTUATION));
            $$->append($7);
            }
*/    ;

        /* data types */
		/*
column_val:
        column
        {$$ = SQL_NEW_RULE;
            $$->append($1);}
    |    '*'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("*", SQL_NODE_PUNCTUATION));
        }
    ;
	*/
data_type:
    predefined_type
    ;
opt_char_set_spec:    
    {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_CHARACTER SQL_TOKEN_SET SQL_TOKEN_NAME
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }    
    ;
opt_collate_clause:
    {$$ = SQL_NEW_RULE;}
    | collate_clause
    ;
predefined_type:
        character_string_type opt_char_set_spec    opt_collate_clause
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    |    national_character_string_type opt_collate_clause
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    |    binary_string_type
    |    numeric_type
    |    boolean_type
    |    datetime_type
    |    interval_type
    ;
character_string_type:
        SQL_TOKEN_CHARACTER opt_paren_precision
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    |    SQL_TOKEN_CHAR opt_paren_precision
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    |    SQL_TOKEN_CHARACTER SQL_TOKEN_VARYING paren_char_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    |    SQL_TOKEN_CHAR SQL_TOKEN_VARYING paren_char_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    |    SQL_TOKEN_VARCHAR paren_char_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    | character_large_object_type
    ;
opt_paren_precision:
        {$$ = SQL_NEW_RULE;}
    |    paren_char_length
    ;
paren_char_length:
    '(' SQL_TOKEN_INTNUM ')'
    {
        $$ = SQL_NEW_RULE;
        $$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
        $$->append($2);
        $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
    }
    ;
opt_paren_char_large_length:
        {$$ = SQL_NEW_RULE;}
    |    paren_character_large_object_length
    ;
paren_character_large_object_length:    
    '(' large_object_length ')'
    {
        $$ = SQL_NEW_RULE;
        $$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
        $$->append($2);
        $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
    }
    ;
    
large_object_length:
    SQL_TOKEN_INTNUM opt_multiplier
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
    ;
opt_multiplier:
    {$$ = SQL_NEW_RULE;}
    |    'K'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("K", SQL_NODE_PUNCTUATION));
        }
    |    'M'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("M", SQL_NODE_PUNCTUATION));
        }
    |    'G'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("G", SQL_NODE_PUNCTUATION));
        }
    |    'T'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("T", SQL_NODE_PUNCTUATION));
        }
    |    'P'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("P", SQL_NODE_PUNCTUATION));
        }
    ;
character_large_object_type:
        SQL_TOKEN_CHARACTER SQL_TOKEN_LARGE SQL_TOKEN_OBJECT opt_paren_char_large_length
    {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    |    SQL_TOKEN_CHAR SQL_TOKEN_LARGE SQL_TOKEN_OBJECT opt_paren_char_large_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    |    SQL_TOKEN_CLOB opt_paren_char_large_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
national_character_string_type:
        SQL_TOKEN_NATIONAL SQL_TOKEN_CHARACTER opt_paren_precision
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    |    SQL_TOKEN_NATIONAL SQL_TOKEN_CHAR opt_paren_precision
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    |    SQL_TOKEN_NCHAR opt_paren_precision
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    |    SQL_TOKEN_NATIONAL SQL_TOKEN_CHARACTER SQL_TOKEN_VARYING paren_char_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    |    SQL_TOKEN_NATIONAL SQL_TOKEN_CHAR SQL_TOKEN_VARYING paren_char_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    |    SQL_TOKEN_NCHAR SQL_TOKEN_VARYING paren_char_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    |    national_character_large_object_type
    ;
national_character_large_object_type:
        SQL_TOKEN_NATIONAL SQL_TOKEN_CHARACTER SQL_TOKEN_LARGE SQL_TOKEN_OBJECT opt_paren_char_large_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
            $$->append($5);
        }
    |    SQL_TOKEN_NCHAR SQL_TOKEN_LARGE SQL_TOKEN_OBJECT opt_paren_char_large_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    |    SQL_TOKEN_NCLOB opt_paren_char_large_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
binary_string_type:
        SQL_TOKEN_BINARY opt_paren_precision
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    |    SQL_TOKEN_BINARY SQL_TOKEN_VARYING paren_char_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    |    SQL_TOKEN_VARBINARY paren_char_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    |    binary_large_object_string_type
    ;
binary_large_object_string_type:
        SQL_TOKEN_BINARY SQL_TOKEN_LARGE SQL_TOKEN_OBJECT opt_paren_char_large_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    |    SQL_TOKEN_BLOB opt_paren_char_large_length
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
numeric_type:
        exact_numeric_type
    |    approximate_numeric_type
    ;
opt_paren_precision_scale:
    {$$ = SQL_NEW_RULE;}
    |    '(' SQL_TOKEN_INTNUM ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    |    '(' SQL_TOKEN_INTNUM ',' SQL_TOKEN_INTNUM ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = newNode(",", SQL_NODE_PUNCTUATION));
            $$->append($4);
            $$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;

exact_numeric_type:
/*ECSQL doesnt support DECIMAL(3,3) precison
        SQL_TOKEN_NUMERIC opt_paren_precision_scale
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    |    SQL_TOKEN_DECIMAL opt_paren_precision_scale
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    |    SQL_TOKEN_DEC opt_paren_precision_scale
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    
    |    SQL_TOKEN_SMALLINT
    */
         SQL_TOKEN_INTEGER
    |    SQL_TOKEN_INT
    |    SQL_TOKEN_LONG
    |    SQL_TOKEN_INT64


    ;
approximate_numeric_type:
/*ECSQL doesnt support DECIMAL(3,3) precison

        SQL_TOKEN_FLOAT '(' SQL_TOKEN_INTNUM ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
*/

         SQL_TOKEN_FLOAT
    |    SQL_TOKEN_REAL
    |    SQL_TOKEN_DOUBLE
/*ECSQL Do not support
    |    SQL_TOKEN_DOUBLE SQL_TOKEN_PRECISION
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
*/
    ;
boolean_type:
    SQL_TOKEN_BOOLEAN
;
datetime_type:
        SQL_TOKEN_DATE
/* no time support in ECSQL
   |    SQL_TOKEN_TIME opt_paren_precision opt_with_or_without_time_zone
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        } */
    |    SQL_TOKEN_TIMESTAMP opt_paren_precision /* opt_with_or_without_time_zone */
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            /*$$->append($3);*/
        }
    ;

/* no time zone support in ECSQL
opt_with_or_without_time_zone:
        {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_WITH SQL_TOKEN_TIME SQL_TOKEN_ZONE
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    |    SQL_TOKEN_WITHOUT SQL_TOKEN_TIME SQL_TOKEN_ZONE
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
    ;
   */
interval_type:
    SQL_TOKEN_INTERVAL interval_qualifier
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
    ;
    
/* the various things you can name */
/* TODO: change to only allow identifiers, i.e. column must not start with number etc. */
column:
        SQL_TOKEN_NAME
/* Hope we don't need those.
    |    SQL_TOKEN_POSITION
        {
            sal_uInt32 nNod = $$->getRuleID();
            delete $$;
            $$ = newNode(xxx_pGLOBAL_SQLPARSER->TokenIDToStr(nNod), SQL_NODE_NAME);
        }
    |    SQL_TOKEN_CHAR_LENGTH
        {
            sal_uInt32 nNod = $$->getRuleID();
            delete $$;
            $$ = newNode(xxx_pGLOBAL_SQLPARSER->TokenIDToStr(nNod), SQL_NODE_NAME);
        }
    |    SQL_TOKEN_EXTRACT
        {
            sal_uInt32 nNod = $$->getRuleID();
            delete $$;
            $$ = newNode(xxx_pGLOBAL_SQLPARSER->TokenIDToStr(nNod), SQL_NODE_NAME);
        } */
    ;
case_expression:
        case_abbreviation
    |    case_specification
    ;
case_abbreviation:
        SQL_TOKEN_NULLIF '(' value_exp_commalist ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    |    SQL_TOKEN_COALESCE '(' value_exp ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    |    SQL_TOKEN_COALESCE '(' value_exp_commalist ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
        }
    ;
case_specification:
        simple_case
    |    searched_case
    ;
simple_case:
    SQL_TOKEN_CASE case_operand simple_when_clause_list else_clause SQL_TOKEN_END
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
            $$->append($5);
        }
    ;
searched_case:
    SQL_TOKEN_CASE searched_when_clause_list else_clause SQL_TOKEN_END
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    ;
simple_when_clause_list:
        simple_when_clause
        {
            $$ = SQL_NEW_LISTRULE;
            $$->append($1);
        }
    |   searched_when_clause_list simple_when_clause
        {
            $1->append($2);
            $$ = $1;
        }
    ;
simple_when_clause:
    SQL_TOKEN_WHEN when_operand_list SQL_TOKEN_THEN result
    {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    ;
when_operand_list:
        when_operand
        {$$ = SQL_NEW_COMMALISTRULE;
        $$->append($1);}
    |   when_operand_list ',' when_operand
        {$1->append($3);
        $$ = $1;}
    ;
when_operand:
        row_value_constructor_elem
    |    comparison_predicate_part_2
    |    between_predicate_part_2
    |    in_predicate_part_2
    |    character_like_predicate_part_2
    |    null_predicate_part_2
;
searched_when_clause_list:
        searched_when_clause
        {
            $$ = SQL_NEW_LISTRULE;
            $$->append($1);
        }
    |   searched_when_clause_list searched_when_clause
        {
            $1->append($2);
            $$ = $1;
        }
    ;
searched_when_clause:
    SQL_TOKEN_WHEN search_condition SQL_TOKEN_THEN result
    {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
    ;
else_clause:
        {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_ELSE result
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;
result:
        result_expression
   /* |    SQL_TOKEN_NULL  useless in parser*/
    ;
result_expression:
    value_exp
    ;
case_operand:
    row_value_constructor_elem
    ;
    
cursor:    SQL_TOKEN_NAME
            {$$ = SQL_NEW_RULE;
            $$->append($1);}
    ;

/***
module:    SQL_TOKEN_NAME
            {$$ = SQL_NEW_RULE;
            $$->append($1);}
    ;
***/

parameter:
        ':' SQL_TOKEN_NAME
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode(":", SQL_NODE_PUNCTUATION));
            $$->append($2);
            }
    |    '?'
        {
            $$ = SQL_NEW_RULE; // test
            $$->append($1 = newNode("?", SQL_NODE_PUNCTUATION));
        }
/*
    |   '['    SQL_TOKEN_NAME ']'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = newNode("[", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = newNode("]", SQL_NODE_PUNCTUATION));
        }
*/
    ;

/***
procedure:      SQL_TOKEN_NAME
            {$$ = SQL_NEW_RULE;
            $$->append($1);}
    ;
***/

range_variable: 
        {
            $$ = SQL_NEW_RULE;
        }
    |   opt_as SQL_TOKEN_NAME
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;

/* PREDICATECHECK RULES */
sql:
        search_condition /* checking predicats */
        {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck()) // sql: rule 1
            {
                $$ = $1;
                if ( SQL_ISRULE($$,search_condition) )
                {
                    $$->insert(0,newNode("(", SQL_NODE_PUNCTUATION));
                    $$->append(newNode(")", SQL_NODE_PUNCTUATION));
                }
            }
            else
                YYERROR;
        }
    |   '(' sql ')' /* checking predicats */
    ;
trigger_definition:
    SQL_TOKEN_CREATE SQL_TOKEN_TRIGGER trigger_name trigger_action_time trigger_event SQL_TOKEN_ON table_name op_referencing triggered_action
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4);
        $$->append($5);
        $$->append($6);
        $$->append($7);
        $$->append($8);
        $$->append($9);
    }
    ;
op_referencing:
    {
        $$ = SQL_NEW_RULE;
    }
    |    SQL_TOKEN_REFERENCING transition_table_or_variable_list
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
    ;
trigger_action_time:
        SQL_TOKEN_BEFORE
    |    SQL_TOKEN_AFTER
    |    SQL_TOKEN_INSTEAD SQL_TOKEN_OF
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
;
trigger_event:
        SQL_TOKEN_INSERT
    |    SQL_TOKEN_DELETE
    |    SQL_TOKEN_UPDATE op_trigger_columnlist
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
    ;
op_trigger_columnlist:
    {
        $$ = SQL_NEW_RULE;
    }
    |    SQL_TOKEN_OF trigger_column_list
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
    ;
trigger_column_list:
    column_commalist
    ;
triggered_action:
    op_triggered_action_for triggered_when_clause triggered_SQL_statement
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
    }
    ;
op_triggered_action_for:
        {
        $$ = SQL_NEW_RULE;
        }
    |    SQL_TOKEN_FOR SQL_TOKEN_EACH trigger_for
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
    }
    ;
trigger_for:
        SQL_TOKEN_ROW 
    |    SQL_TOKEN_STATEMENT
    ;
triggered_when_clause:
    {
        $$ = SQL_NEW_RULE;
    }
    |    SQL_TOKEN_WHEN parenthesized_boolean_value_expression
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
    ;
triggered_SQL_statement:
        SQL_procedure_statement
    |    SQL_TOKEN_BEGIN SQL_TOKEN_ATOMIC SQL_procedure_statement_list ';' SQL_TOKEN_END
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4 = newNode(";", SQL_NODE_PUNCTUATION));
        $$->append($5);
    }
    ;
SQL_procedure_statement_list:
        SQL_procedure_statement 
        {
            $$ = SQL_NEW_LISTRULE;
            $$->append($1);
        }
    |    SQL_procedure_statement_list ';' SQL_procedure_statement 
        {
            $1->append($3);
            $$ = $1;
        }
    ;
SQL_procedure_statement:
    sql
    ;
    
transition_table_or_variable_list:
        transition_table_or_variable
        {
            $$ = SQL_NEW_LISTRULE;
            $$->append($1);
        }
    |   transition_table_or_variable_list transition_table_or_variable
        {
            $1->append($2);
            $$ = $1;
        }
    ;

transition_table_or_variable:
        SQL_TOKEN_OLD opt_row opt_as old_transition_variable_name
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4);
    }
    |    SQL_TOKEN_NEW opt_row opt_as new_transition_variable_name
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4);
    }
    |    SQL_TOKEN_OLD SQL_TOKEN_TABLE opt_as old_transition_table_name
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4);
    }
    |    SQL_TOKEN_NEW SQL_TOKEN_TABLE opt_as new_transition_table_name
    {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4);
    }
;
old_transition_table_name:
    transition_table_name
;
new_transition_table_name:
    transition_table_name
;
transition_table_name:
    SQL_TOKEN_NAME
;
old_transition_variable_name:
    SQL_TOKEN_NAME
;
new_transition_variable_name:
    SQL_TOKEN_NAME
;
trigger_name:
    SQL_TOKEN_NAME
;

opt_ecsqloptions_clause:
        /* empty */         {$$ = SQL_NEW_RULE;}
    |   ecsqloptions_clause
    ;

ecsqloptions_clause:
    SQL_TOKEN_ECSQLOPTIONS ecsqloptions_list
        {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        }
    ;

ecsqloptions_list:
    ecsqloptions_list ecsqloption
        {
            $1->append($2);
            $$ = $1;
        }
    |   ecsqloption
        {
            $$ = SQL_NEW_LISTRULE;
            $$->append($1);
        }
    ;

ecsqloption:
   SQL_TOKEN_NAME
            {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            }           
   |
   SQL_TOKEN_NAME SQL_EQUAL ecsqloptionvalue
            {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            }
    ;

ecsqloptionvalue:
    literal
  | SQL_TOKEN_NAME
  | truth_value
  ;
%%


using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::dbtools;

//============================================================
//= a helper for static ascii pseudo-unicode strings
//============================================================
// string constants
struct _ConstAsciiString_
{
    sal_Int32 length;
    sal_Char  const* str;
    //operator Utf8String () const { return Utf8String (str); }
    operator const sal_Char * () const { return str; }
    operator Utf8String() const { return str; }
};

#define IMPLEMENT_CONSTASCII_STRING( name, string ) \
    _ConstAsciiString_ const name = { sizeof(string)-1, string }

IMPLEMENT_CONSTASCII_STRING(ERROR_STR_GENERAL, "Syntax error in SQL expression");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_VALUE_NO_LIKE, "The value #1 can not be used with LIKE.");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_FIELD_NO_LIKE, "LIKE can not be used with this field.");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_COMPARE, "The entered criterion can not be compared with this field.");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_DATE_COMPARE, "The field can not be compared with a date.");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_REAL_COMPARE,    "The field can not be compared with a floating point number.");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_INT_COMPARE,    "The field can not be compared with a number.");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_TABLE,    "The database does not contain a table named \"#\".");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_TABLE_OR_QUERY,   "The database does contain neither a table nor a query named \"#\".");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_COLUMN,    "The column \"#1\" is unknown in the table \"#2\".");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_TABLE_EXIST,    "The database already contains a table or view with name \"#\".");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_QUERY_EXIST,    "The database already contains a query with name \"#\".");

IMPLEMENT_CONSTASCII_STRING(KEY_STR_LIKE, "LIKE");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_NOT, "NOT");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_NULL, "NULL");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_TRUE, "True");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_FALSE, "False");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_IS, "IS");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_BETWEEN, "BETWEEN");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_OR, "OR");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_AND, "AND");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_AVG, "AVG");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_COUNT, "COUNT");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_MAX, "MAX");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_MIN, "MIN");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_SUM, "SUM");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_EVERY, "EVERY");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_ANY, "ANY");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_SOME, "SOME");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_STDDEV_POP, "STDDEV_POP");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_STDDEV_SAMP, "STDDEV_SAMP");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_VAR_SAMP, "VAR_SAMP");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_VAR_POP, "VAR_POP");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_COLLECT, "COLLECT");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_FUSION, "FUSION");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_INTERSECTION, "INTERSECTION");

IMPLEMENT_CONSTASCII_STRING(FIELD_STR_NULLDATE, "NullDate");

IMPLEMENT_CONSTASCII_STRING(STR_SQL_TOKEN, "SQL_TOKEN_");

//==========================================================================
//= OParseContext
//==========================================================================
//-----------------------------------------------------------------------------
OParseContext::OParseContext()
{
}

//-----------------------------------------------------------------------------
OParseContext::~OParseContext()
{
}

//-----------------------------------------------------------------------------
Utf8String OParseContext::getErrorMessage(ErrorCode _eCode) const
{
    Utf8String aMsg;
    switch (_eCode)
    {
        case ERROR_GENERAL:                    aMsg = ERROR_STR_GENERAL; break;
        case ERROR_VALUE_NO_LIKE:            aMsg = ERROR_STR_VALUE_NO_LIKE; break;
        case ERROR_FIELD_NO_LIKE:            aMsg = ERROR_STR_FIELD_NO_LIKE; break;
        case ERROR_INVALID_COMPARE:            aMsg = ERROR_STR_INVALID_COMPARE; break;
        case ERROR_INVALID_INT_COMPARE:        aMsg = ERROR_STR_INVALID_INT_COMPARE; break;
        case ERROR_INVALID_DATE_COMPARE:    aMsg = ERROR_STR_INVALID_DATE_COMPARE; break;
        case ERROR_INVALID_REAL_COMPARE:    aMsg = ERROR_STR_INVALID_REAL_COMPARE; break;
        case ERROR_INVALID_TABLE:            aMsg = ERROR_STR_INVALID_TABLE; break;
        case ERROR_INVALID_TABLE_OR_QUERY:  aMsg = ERROR_STR_INVALID_TABLE_OR_QUERY; break;
        case ERROR_INVALID_COLUMN:            aMsg = ERROR_STR_INVALID_COLUMN; break;
        case ERROR_INVALID_TABLE_EXIST:        aMsg = ERROR_STR_INVALID_TABLE_EXIST; break;
        case ERROR_INVALID_QUERY_EXIST:        aMsg = ERROR_STR_INVALID_QUERY_EXIST; break;
        default:
            OSL_ENSURE( false, "OParseContext::getErrorMessage: unknown error code!" );
            break;
    }
    return aMsg;
}

//-----------------------------------------------------------------------------
Utf8String OParseContext::getIntlKeywordAscii(InternationalKeyCode _eKey) const
{
    Utf8String aKeyword;
    switch (_eKey)
    {
        case KEY_LIKE:        aKeyword = KEY_STR_LIKE; break;
        case KEY_NOT:        aKeyword = KEY_STR_NOT; break;
        case KEY_NULL:        aKeyword = KEY_STR_NULL; break;
        case KEY_TRUE:        aKeyword = KEY_STR_TRUE; break;
        case KEY_FALSE:        aKeyword = KEY_STR_FALSE; break;
        case KEY_IS:        aKeyword = KEY_STR_IS; break;
        case KEY_BETWEEN:    aKeyword = KEY_STR_BETWEEN; break;
        case KEY_OR:        aKeyword = KEY_STR_OR; break;
        case KEY_AND:        aKeyword = KEY_STR_AND; break;
        case KEY_AVG:        aKeyword = KEY_STR_AVG; break;
        case KEY_COUNT:        aKeyword = KEY_STR_COUNT; break;
        case KEY_MAX:        aKeyword = KEY_STR_MAX; break;
        case KEY_MIN:        aKeyword = KEY_STR_MIN; break;
        case KEY_SUM:        aKeyword = KEY_STR_SUM; break;
        case KEY_EVERY:     aKeyword = KEY_STR_EVERY; break;
        case KEY_ANY:       aKeyword = KEY_STR_ANY; break;
        case KEY_SOME:      aKeyword = KEY_STR_SOME; break;
        case KEY_COLLECT:   aKeyword = KEY_STR_COLLECT; break;
        case KEY_FUSION:    aKeyword = KEY_STR_FUSION; break;
        case KEY_INTERSECTION:aKeyword = KEY_STR_INTERSECTION; break;
        case KEY_NONE:      break;
        default:
            OSL_ENSURE( false, "OParseContext::getIntlKeywordAscii: unknown key!" );
            break;
    }
    return aKeyword;
}

//-----------------------------------------------------------------------------
IParseContext::InternationalKeyCode OParseContext::getIntlKeyCode(const Utf8String& rToken) const
{
    static IParseContext::InternationalKeyCode Intl_TokenID[] =
    {
        KEY_LIKE, KEY_NOT, KEY_NULL, KEY_TRUE,
        KEY_FALSE, KEY_IS, KEY_BETWEEN, KEY_OR,
        KEY_AND, KEY_AVG, KEY_COUNT, KEY_MAX,
        KEY_MIN, KEY_SUM, KEY_EVERY,KEY_ANY,KEY_SOME,
        KEY_COLLECT,KEY_FUSION,KEY_INTERSECTION
    };

    sal_uInt32 nCount = sizeof Intl_TokenID / sizeof Intl_TokenID[0];
    for (sal_uInt32 i = 0; i < nCount; i++)
    {
        Utf8String aKey = getIntlKeywordAscii(Intl_TokenID[i]);
        if (rToken.EqualsI(aKey))
            return Intl_TokenID[i];
    }

    return KEY_NONE;
}

//------------------------------------------------------------------------------
static Locale& impl_getLocaleInstance( )
{
    static Locale s_aLocale(
        Utf8String("en"),
        Utf8String("US"),
        Utf8String( )
    );
    return s_aLocale;
}

//------------------------------------------------------------------------------
void OParseContext::setDefaultLocale( const ::com::sun::star::lang::Locale& _rLocale )
{
    impl_getLocaleInstance() = _rLocale;
}

//------------------------------------------------------------------------------
Locale OParseContext::getPreferredLocale( ) const
{
    return getDefaultLocale();
}

//------------------------------------------------------------------------------
const Locale& OParseContext::getDefaultLocale()
{
    return impl_getLocaleInstance();
}

//==========================================================================
//= misc
//==========================================================================
// Der (leider globale) yylval fuer die Uebergabe von
// Werten vom Scanner an den Parser. Die globale Variable
// wird nur kurzzeitig verwendet, der Parser liest die Variable
// sofort nach dem Scanner-Aufruf in eine gleichnamige eigene
// Member-Variable.

const double fMilliSecondsPerDay = 86400000.0;

//------------------------------------------------------------------------------


//------------------------------------------------------------------
Utf8String ConvertLikeToken(const OSQLParseNode* pTokenNode, const OSQLParseNode* pEscapeNode, sal_Bool bInternational)
{
    Utf8String aMatchStr;
    if (pTokenNode->isToken())
    {
        sal_Char cEscape = 0;
        if (pEscapeNode->count())
            cEscape = Utf8StringHelper::toChar (pEscapeNode->getChild(1)->getTokenValue());

        // Platzhalter austauschen
        aMatchStr = pTokenNode->getTokenValue();
        const size_t nLen = aMatchStr.size();
        Utf8String sSearch,sReplace;
        if ( bInternational )
        {
            sSearch.append("%_");
            sReplace.append("*?");
        }
        else
        {
            sSearch.append("*?");
            sReplace.append("%_");
        }
        
        bool wasEscape = false;
        for (size_t i = 0; i < nLen; i++)
        {
                const sal_Char c = aMatchStr[i];
                // SQL standard requires the escape to be followed
                // by a meta-character ('%', '_' or itself), else error
                // We are more lenient here and let it escape anything.
                // Especially since some databases (e.g. Microsoft SQL Server)
                // have more meta-characters than the standard, such as e.g. '[' and ']'
            if (wasEscape)
            {
                wasEscape=false;
                continue;
            }
            if (c == cEscape)
            {
                wasEscape=true;
                continue;
            }
            int match = -1;
            if (c == sSearch[0])
                match=0;
            else if (c == sSearch[1])
                match=1;

            if (match != -1)
            {
                aMatchStr[i] = sReplace[(size_t)match];
            }
        }
    }
    return aMatchStr;
}


//==========================================================================
//= OSQLParser
//==========================================================================

sal_uInt32                OSQLParser::s_nRuleIDs[OSQLParseNode::rule_count + 1];
OSQLParser::RuleIDMap   OSQLParser::s_aReverseRuleIDLookup;
OParseContext            OSQLParser::s_aDefaultContext;

sal_Int32                  OSQLParser::s_nRefCount    = 0;
OSQLScanner*            OSQLParser::s_pScanner = 0;
OSQLParseNodesGarbageCollector*        OSQLParser::s_pGarbageCollector = 0;
RefCountedPtr< ::com::sun::star::i18n::XLocaleData>        OSQLParser::s_xLocaleData = NULL;
//-----------------------------------------------------------------------------
void setParser(OSQLParser* _pParser)
{
    xxx_pGLOBAL_SQLPARSER = _pParser;
}
// -------------------------------------------------------------------------
void OSQLParser::setParseTree(OSQLParseNode * pNewParseTree)
{
    BeMutexHolder aGuard(getCriticalSection());
    m_pParseTree = pNewParseTree;
}
//-----------------------------------------------------------------------------

/** Delete all comments in a query.

    See also getComment()/concatComment() implementation for
    OQueryController::translateStatement().
 */
static Utf8String delComment(Utf8String const& rQuery)
{
    // First a quick search if there is any "--" or "//" or "/*", if not then the whole
    // copying loop is pointless.
      if (rQuery.find("--") == Utf8String::npos < 0 && rQuery.find( "//") == Utf8String::npos  &&
          rQuery.find( "/*") == Utf8String::npos)
        return rQuery;

    const sal_Char* pCopy = rQuery.c_str();
    size_t nQueryLen = rQuery.size();
    bool bIsText1  = false;     // "text"
    bool bIsText2  = false;     // 'text'
    bool bComment2 = false;     // /* comment */
    bool bComment  = false;     // -- or // comment
    Utf8String aBuf;
    aBuf.reserve(nQueryLen);
    for (sal_Int32 i=0; i < nQueryLen; ++i)
    {
        if (bComment2)
        {
            if ((i+1) < nQueryLen)
            {
                if (pCopy[i]=='*' && pCopy[i+1]=='/')
                {
                    bComment2 = false;
                    ++i;
                }
            }
            else
            {
                // comment can't close anymore, actually an error, but..
            }
            continue;
        }
        if (pCopy[i] == '\n')
            bComment = false;
        else if (!bComment)
        {
            if (pCopy[i] == '\"' && !bIsText2)
                bIsText1 = !bIsText1;
            else if (pCopy[i] == '\'' && !bIsText1)
                bIsText2 = !bIsText2;
            if (!bIsText1 && !bIsText2 && (i+1) < nQueryLen)
            {
                if ((pCopy[i]=='-' && pCopy[i+1]=='-') || (pCopy[i]=='/' && pCopy[i+1]=='/'))
                    bComment = true;
                else if ((pCopy[i]=='/' && pCopy[i+1]=='*'))
                    bComment2 = true;
            }
        }
        if (!bComment && !bComment2)
            aBuf.append(&pCopy[i], 1);
    }
    return aBuf;
}

OSQLParseNode* OSQLParser::parseTree(Utf8String& rErrorMessage,
                                     Utf8String const& rStatement,
                                     sal_Bool bInternational)
{


    // Guard the parsing
    BeMutexHolder aGuard(getCriticalSection());
    // must be reset
    setParser(this);

    // delete comments before parsing
    Utf8String sTemp = delComment(rStatement);
    // defines how to scan
    s_pScanner->SetRule(s_pScanner->GetSQLRule()); // initial
    s_pScanner->prepareScan(sTemp, m_pContext, bInternational);

    SQLyylval.pParseNode = NULL;
    //    SQLyypvt = NULL;
    m_pParseTree = NULL;
    m_sErrorMessage = Utf8String();

    // ... und den Parser anwerfen ...
    if (SQLyyparse() != 0)
    {
        // only set the error message, if it's not already set
        if (!m_sErrorMessage.size())
            m_sErrorMessage = s_pScanner->getErrorMessage();
        if (!m_sErrorMessage.size())
            m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_GENERAL);

        rErrorMessage = m_sErrorMessage;

        // clear the garbage collector
        (*s_pGarbageCollector)->clearAndDelete();
        return NULL;
    }
    else
    {
        (*s_pGarbageCollector)->clear();

        // Das Ergebnis liefern (den Root Parse Node):

        //    OSL_ENSURE(Sdbyyval.pParseNode != NULL,"OSQLParser: Parser hat keinen ParseNode geliefert");
        //    return Sdbyyval.pParseNode;
        // geht nicht wegen Bug in MKS YACC-erzeugtem Code (es wird ein falscher ParseNode
        // geliefert).

        // Stattdessen setzt die Parse-Routine jetzt den Member pParseTree
        // - einfach diesen zurueckliefern:
        OSL_ENSURE(m_pParseTree != NULL,"OSQLParser: Parser hat keinen ParseTree geliefert");
        return m_pParseTree;
    }
}
//-----------------------------------------------------------------------------
Utf8String OSQLParser::TokenIDToStr(sal_uInt32 nTokenID, const IParseContext* pContext)
{
    Utf8String aStr;
    if (pContext)
    {
        IParseContext::InternationalKeyCode eKeyCode = IParseContext::KEY_NONE;
        switch( nTokenID )
        {
            case SQL_TOKEN_LIKE: eKeyCode = IParseContext::KEY_LIKE; break;
            case SQL_TOKEN_NOT: eKeyCode = IParseContext::KEY_NOT; break;
            case SQL_TOKEN_NULL: eKeyCode = IParseContext::KEY_NULL; break;
            case SQL_TOKEN_TRUE: eKeyCode = IParseContext::KEY_TRUE; break;
            case SQL_TOKEN_FALSE: eKeyCode = IParseContext::KEY_FALSE; break;
            case SQL_TOKEN_IS: eKeyCode = IParseContext::KEY_IS; break;
            case SQL_TOKEN_BETWEEN: eKeyCode = IParseContext::KEY_BETWEEN; break;
            case SQL_TOKEN_OR: eKeyCode = IParseContext::KEY_OR; break;
            case SQL_TOKEN_AND: eKeyCode = IParseContext::KEY_AND; break;
            case SQL_TOKEN_AVG: eKeyCode = IParseContext::KEY_AVG; break;
            case SQL_TOKEN_COUNT: eKeyCode = IParseContext::KEY_COUNT; break;
            case SQL_TOKEN_MAX: eKeyCode = IParseContext::KEY_MAX; break;
            case SQL_TOKEN_MIN: eKeyCode = IParseContext::KEY_MIN; break;
            case SQL_TOKEN_SUM: eKeyCode = IParseContext::KEY_SUM; break;
        }
        if ( eKeyCode != IParseContext::KEY_NONE )
            aStr = pContext->getIntlKeywordAscii(eKeyCode);
    }

    if (!aStr.size())
    {
        aStr = yytname[YYTRANSLATE(nTokenID)];
        if (aStr.substr(0, 10) == "SQL_TOKEN_")
        //if(!aStr.compareTo("SQL_TOKEN_",10))
            aStr = aStr.substr(10); //AK: Copy reset of the string into aStr
    }
    return aStr;
}


//-----------------------------------------------------------------------------
Utf8CP OSQLParser::RuleIDToStr(sal_uInt32 nRuleID)
{
    OSL_ENSURE(nRuleID < (sizeof yytname/sizeof yytname[0]), "OSQLParser::RuleIDToStr: Invalid nRuleId!");
    return yytname[nRuleID];
}

//-----------------------------------------------------------------------------
sal_uInt32 OSQLParser::StrToRuleID(const Utf8String & rValue)
{
    // In yysvar nach dem angegebenen Namen suchen, den Index zurueckliefern
    // (oder 0, wenn nicht gefunden)
    static sal_uInt32 nLen = sizeof(yytname)/sizeof(yytname[0]);
    for (sal_uInt32 i = YYTRANSLATE(SQL_TOKEN_INVALIDSYMBOL); i < (nLen-1); i++)
    {
        if (yytname && rValue == yytname[i])
            return i;
    }

    // Nicht gefunden
    return 0;
}

//-----------------------------------------------------------------------------
OSQLParseNode::Rule OSQLParser::RuleIDToRule( sal_uInt32 _nRule )
{
    OSQLParser::RuleIDMap::const_iterator i (s_aReverseRuleIDLookup.find(_nRule));
    if (i == s_aReverseRuleIDLookup.end())
    {
    /*
        SAL_WARN("connectivity.parse",
         "connectivity::OSQLParser::RuleIDToRule cannot reverse-lookup rule. "
         "Reverse mapping incomplete? "
         "_nRule='" << _nRule << "' "
         "yytname[_nRule]='" << yytname[_nRule] << "'");
    */
        return OSQLParseNode::UNKNOWN_RULE;
    }
    else
        return i->second;
}

//-----------------------------------------------------------------------------
sal_uInt32 OSQLParser::RuleID(OSQLParseNode::Rule eRule)
{
    return s_nRuleIDs[(sal_uInt16)eRule];
}
// -------------------------------------------------------------------------
sal_Int16 OSQLParser::buildNode(OSQLParseNode*& pAppend,OSQLParseNode* pCompare,OSQLParseNode* pLiteral,OSQLParseNode* pLiteral2)
{
    OSQLParseNode* pColumnRef = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
    pColumnRef->append(new OSQLInternalNode(m_sFieldName,SQL_NODE_NAME));
    OSQLParseNode* pComp = NULL;
    if ( SQL_ISTOKEN( pCompare, BETWEEN) && pLiteral2 )
        pComp = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::between_predicate_part_2));
    else
        pComp = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::comparison_predicate));
    
    pComp->append(pColumnRef);
    pComp->append(pCompare);
    pComp->append(pLiteral);
    if ( pLiteral2 )
    {
        pComp->append(new OSQLInternalNode(aEmptyString, SQL_NODE_KEYWORD,SQL_TOKEN_AND));
        pComp->append(pLiteral2);        
    }
    pAppend->append(pComp);
    return 1;
}
//-----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildStringNodes(OSQLParseNode*& pLiteral)
{
    if(!pLiteral)
        return 1;

    if(SQL_ISRULE(pLiteral,fct_spec) || SQL_ISRULE(pLiteral,general_set_fct) || SQL_ISRULE(pLiteral,column_ref)
        || SQL_ISRULE(pLiteral,subquery))
        return 1; // here I have a function that I can't transform into a string

    if(pLiteral->getNodeType() == SQL_NODE_INTNUM || pLiteral->getNodeType() == SQL_NODE_APPROXNUM || pLiteral->getNodeType() == SQL_NODE_ACCESS_DATE)
    {
        OSQLParseNode* pParent = pLiteral->getParent();

        OSQLParseNode* pNewNode = new OSQLInternalNode(pLiteral->getTokenValue(), SQL_NODE_STRING);
        pParent->replace(pLiteral, pNewNode);
        delete pLiteral;
        pLiteral = NULL;
        return 1;
    }

    for(sal_uInt32 i=0;i<pLiteral->count();++i)
    {
        OSQLParseNode* pChild = pLiteral->getChild(i);
        buildStringNodes(pChild);
    }
    if(SQL_ISRULE(pLiteral,term) || SQL_ISRULE(pLiteral,value_exp_primary))
    {
        m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_COMPARE);
        return 0;
    }
    return 1;
}
//-----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildComparsionRule(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral)
{
    OSQLParseNode* pComp = new OSQLInternalNode(Utf8String("="), SQL_NODE_EQUAL);
    return buildPredicateRule(pAppend,pLiteral,pComp);
}


//-----------------------------------------------------------------------------
void OSQLParser::reduceLiteral(OSQLParseNode*& pLiteral, sal_Bool bAppendBlank)
{
    OSL_ENSURE(pLiteral->isRule(), "This is no ::com::sun::star::chaos::Rule");
    OSL_ENSURE(pLiteral->count() == 2, "OSQLParser::ReduceLiteral() Invalid count");
    OSQLParseNode* pTemp = pLiteral;
    Utf8String aValue(pLiteral->getChild(0)->getTokenValue());
    if (bAppendBlank)
    {
        aValue.append(" ");
    }
    
    aValue.append(pLiteral->getChild(1)->getTokenValue());

    pLiteral = new OSQLInternalNode(aValue,SQL_NODE_STRING);
    delete pTemp;
}

// -------------------------------------------------------------------------
void OSQLParser::error( const sal_Char* fmt)
{
    if(!m_sErrorMessage.size())
    {    
        Utf8String sStr(fmt);
        Utf8String sSQL_TOKEN("SQL_TOKEN_");

        size_t nPos1 = sStr.find(sSQL_TOKEN);
        if(nPos1 != Utf8String::npos)
        {
            Utf8String sFirst  = sStr.substr(0,nPos1);
            size_t nPos2 = sStr.find(sSQL_TOKEN,nPos1+1);
            if(nPos2 != Utf8String::npos)
            {
                Utf8String sSecond = sStr.substr(nPos1+sSQL_TOKEN.size(),nPos2-nPos1-sSQL_TOKEN.size());
                sFirst  += sSecond;
                sFirst  += sStr.substr(nPos2+sSQL_TOKEN.size());
            }
            else
                sFirst += sStr.substr(nPos1+sSQL_TOKEN.size());

            m_sErrorMessage = sFirst;
        }
        else
            m_sErrorMessage = sStr;

        Utf8String aError = s_pScanner->getErrorMessage();
        if(aError.size())
        {
            m_sErrorMessage += ", ";
            m_sErrorMessage += aError;
        }
    }
}
// -------------------------------------------------------------------------
int OSQLParser::SQLlex()
{
    return s_pScanner->SQLlex();
}

#if defined __SUNPRO_CC
#pragma enable_warn
#elif defined _MSC_VER
#pragma warning(pop)
#endif

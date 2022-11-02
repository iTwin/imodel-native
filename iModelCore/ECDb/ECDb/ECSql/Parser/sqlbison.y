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
%token-table
%{
static Utf8String aEmptyString;

#define CREATE_NODE  context->GetScanner()->NewNode

// yyi ist die interne Nr. der Regel, die gerade reduziert wird.
// Ueber die Mapping-Tabelle yyrmap wird daraus eine externe Regel-Nr.
#define SQL_NEW_RULE             context->GetScanner()->NewNode(aEmptyString, SQL_NODE_RULE, yyr1[yyn])
#define SQL_NEW_LISTRULE         context->GetScanner()->NewNode(aEmptyString, SQL_NODE_LISTRULE, yyr1[yyn])
#define SQL_NEW_COMMALISTRULE   context->GetScanner()->NewNode(aEmptyString, SQL_NODE_COMMALISTRULE, yyr1[yyn])
#define SQL_NEW_DOTLISTRULE   context->GetScanner()->NewNode(aEmptyString, SQL_NODE_DOTLISTRULE, yyr1[yyn])

#if !(defined MACOSX && defined PPC)
#define YYERROR_VERBOSE
#endif

#define SQLyyerror(context, s) \
    {                                 \
    context->error(s);                \
    }

using namespace connectivity;
#define SQLyylex context->SQLlex
%}
    /* symbolic tokens */

%define api.pure full
%parse-param { connectivity::OSQLParser* context }
%union {
    connectivity::OSQLParseNode * pParseNode;
}
%type <pParseNode> '(' ')' ',' ':' ';' '?' '[' ']' '{' '}' '.' 'K' 'M' 'G' 'T' 'P'

%token <pParseNode> SQL_TOKEN_ACCESS_DATE SQL_TOKEN_REAL_NUM
%token <pParseNode> SQL_TOKEN_INTNUM SQL_TOKEN_APPROXNUM SQL_TOKEN_NOT SQL_TOKEN_NAME SQL_TOKEN_ARRAY_INDEX


%nonassoc <pParseNode> SQL_TOKEN_UMINUS



    /* literal keyword tokens */
%token <pParseNode> SQL_TOKEN_WITH SQL_TOKEN_RECURSIVE 

%token <pParseNode> SQL_TOKEN_ALL SQL_TOKEN_ANY SQL_TOKEN_AS SQL_TOKEN_ASC SQL_TOKEN_AVG

%token <pParseNode> SQL_TOKEN_BETWEEN SQL_TOKEN_BY

%token <pParseNode> SQL_TOKEN_CAST SQL_TOKEN_COMMIT SQL_TOKEN_COUNT SQL_TOKEN_CROSS

%token <pParseNode> SQL_TOKEN_DEFAULT SQL_TOKEN_DELETE SQL_TOKEN_DESC
%token <pParseNode> SQL_TOKEN_DISTINCT SQL_TOKEN_FORWARD SQL_TOKEN_BACKWARD

%token <pParseNode> SQL_TOKEN_ESCAPE SQL_TOKEN_EXCEPT SQL_TOKEN_EXISTS SQL_TOKEN_FALSE SQL_TOKEN_FROM SQL_TOKEN_FULL

%token <pParseNode> SQL_TOKEN_GROUP SQL_TOKEN_HAVING SQL_TOKEN_IN SQL_TOKEN_INNER SQL_TOKEN_INSERT SQL_TOKEN_INTO SQL_TOKEN_IS SQL_TOKEN_INTERSECT

%token <pParseNode> SQL_TOKEN_JOIN SQL_TOKEN_LIKE SQL_TOKEN_LEFT SQL_TOKEN_RIGHT
%token <pParseNode> SQL_TOKEN_MAX SQL_TOKEN_MIN SQL_TOKEN_NATURAL SQL_TOKEN_NULL

%token <pParseNode> SQL_TOKEN_ON SQL_TOKEN_ORDER SQL_TOKEN_OUTER

%token <pParseNode> SQL_TOKEN_ROLLBACK

%token <pParseNode> SQL_TOKEN_IIF

%token <pParseNode> SQL_TOKEN_SELECT SQL_TOKEN_SET SQL_TOKEN_SOME SQL_TOKEN_SUM

%token <pParseNode> SQL_TOKEN_TRUE SQL_TOKEN_UNION
%token <pParseNode> SQL_TOKEN_UNIQUE SQL_TOKEN_UNKNOWN SQL_TOKEN_UPDATE SQL_TOKEN_USING SQL_TOKEN_VALUE SQL_TOKEN_VALUES
%token <pParseNode> SQL_TOKEN_WHERE 

%token <pParseNode> SQL_BITWISE_NOT

/* time and date functions */
%token <pParseNode> SQL_TOKEN_CURRENT_DATE SQL_TOKEN_CURRENT_TIME SQL_TOKEN_CURRENT_TIMESTAMP

// computational operation
%token <pParseNode> SQL_TOKEN_EVERY

%token <pParseNode> SQL_TOKEN_CASE SQL_TOKEN_THEN SQL_TOKEN_END SQL_TOKEN_WHEN SQL_TOKEN_ELSE

// LIMIT and OFFSET
%token <pParseNode> SQL_TOKEN_LIMIT SQL_TOKEN_OFFSET SQL_TOKEN_ONLY

//non-standard
%token <pParseNode> SQL_TOKEN_MATCH SQL_TOKEN_ECSQLOPTIONS

//data types (standard and EC)
%token <pParseNode> SQL_TOKEN_INTEGER SQL_TOKEN_INT SQL_TOKEN_INT64 SQL_TOKEN_LONG SQL_TOKEN_BOOLEAN SQL_TOKEN_DOUBLE SQL_TOKEN_REAL SQL_TOKEN_FLOAT 
%token <pParseNode> SQL_TOKEN_STRING SQL_TOKEN_VARCHAR SQL_TOKEN_BINARY SQL_TOKEN_BLOB SQL_TOKEN_DATE SQL_TOKEN_TIME SQL_TOKEN_TIMESTAMP 

/* operators */
%left SQL_TOKEN_NAME
%left SQL_TOKEN_ARRAY_INDEX

%left <pParseNode> SQL_TOKEN_OR
%left <pParseNode> SQL_TOKEN_AND

%left <pParseNode> SQL_BITWISE_OR
%left <pParseNode> SQL_BITWISE_AND

%left <pParseNode> SQL_BITWISE_SHIFT_LEFT SQL_BITWISE_SHIFT_RIGHT
%left <pParseNode> SQL_LESSEQ SQL_GREATEQ SQL_NOTEQUAL SQL_LESS SQL_GREAT SQL_EQUAL
%left <pParseNode> '+' '-' SQL_CONCAT
%left <pParseNode> '*' '/' '%'

%left SQL_TOKEN_NATURAL SQL_TOKEN_CROSS SQL_TOKEN_FULL SQL_TOKEN_LEFT SQL_TOKEN_RIGHT
%right SQL_BITWISE_NOT
%left ')'
%right '='
%right '.'
%right ':'
%right '('


%token <pParseNode> SQL_TOKEN_INVALIDSYMBOL

%type <pParseNode> sql
%type <pParseNode> column_commalist opt_column_array_idx property_path_entry property_path
%type <pParseNode> opt_column_commalist column_ref_commalist opt_column_ref_commalist 
%type <pParseNode> opt_order_by_clause ordering_spec_commalist
%type <pParseNode> ordering_spec opt_asc_desc manipulative_statement commit_statement
%type <pParseNode> delete_statement_searched
%type <pParseNode> type_predicate type_list type_list_item
%type <pParseNode> insert_statement values_or_query_spec
%type <pParseNode> rollback_statement select_statement_into opt_all_distinct
%type <pParseNode> assignment_commalist assignment
%type <pParseNode> update_statement_searched target_commalist target opt_where_clause
%type <pParseNode> single_select_statement selection table_exp from_clause table_ref_commalist table_ref
%type <pParseNode> where_clause opt_group_by_clause opt_having_clause
%type <pParseNode> search_condition predicate comparison_predicate comparison_predicate_part_2 between_predicate between_predicate_part_2
%type <pParseNode> like_predicate opt_escape test_for_null null_predicate_part_2 in_predicate in_predicate_part_2 character_like_predicate_part_2 other_like_predicate_part_2
%type <pParseNode> all_or_any_predicate any_all_some existence_test subquery quantified_comparison_predicate_part_2
%type <pParseNode> scalar_exp_commalist parameter_ref literal
%type <pParseNode> column_ref column parameter range_variable
/* neue Regeln bei OJ */
%type <pParseNode> derived_column as_clause num_primary term num_value_exp term_add_sub
%type <pParseNode> value_exp_primary unsigned_value_spec cast_spec fct_spec scalar_subquery
%type <pParseNode> general_value_spec iif_spec
%type <pParseNode> general_set_fct set_fct_type joined_table ecrelationship_join op_relationship_direction
%type <pParseNode> row_value_constructor_commalist row_value_constructor row_value_constructor_elem
%type <pParseNode> qualified_join value_exp join_type outer_join_type join_condition boolean_term
%type <pParseNode> boolean_factor truth_value boolean_test boolean_primary named_columns_join join_spec
%type <pParseNode> cast_operand cast_target cast_target_primitive_type cast_target_scalar cast_target_array factor datetime_value_exp datetime_term datetime_factor
%type <pParseNode> datetime_primary datetime_value_fct
%type <pParseNode> select_sublist string_value_exp
%type <pParseNode> char_value_exp concatenation char_factor char_primary
%type <pParseNode> value_exp_commalist in_predicate_value unique_test update_source
%type <pParseNode> all sql_not comparison cross_union
%type <pParseNode> select_statement
%type <pParseNode> function_name function_args_commalist function_arg
%type <pParseNode> table_node tablespace_qualified_class_name qualified_class_name class_name table_primary_as_range_column opt_as
%type <pParseNode> table_node_with_opt_member_func_call table_node_path table_node_path_entry opt_member_function_args
%type <pParseNode> case_expression else_clause result_expression result case_specification searched_when_clause simple_when_clause searched_case simple_case
%type <pParseNode> when_operand_list when_operand case_operand
%type <pParseNode> searched_when_clause_list simple_when_clause_list opt_disqualify_primary_join

/* LIMIT and OFFSET */
%type <pParseNode> opt_limit_offset_clause limit_offset_clause opt_offset opt_only union_op
/* non-standard */
%type <pParseNode> rtreematch_predicate rtreematch_predicate_part_2
%type <pParseNode> opt_ecsqloptions_clause ecsqloptions_clause ecsqloptions_list ecsqloption ecsqloptionvalue
%type <pParseNode> cte opt_cte_recursive cte_column_list cte_table_name cte_block_list
%%

/* Parse Tree an OSQLParser zurueckliefern
 * (der Zugriff ueber yyval nach Aufruf des Parsers scheitert,
 *
 */
sql_single_statement:
        sql
        { context->setParseTree( $1 ); }
    |    sql ';'
        { context->setParseTree( $1 ); }
    ;

sql:
        manipulative_statement
    ;
/* 
 * CTE query support 
 */

opt_cte_recursive:
        {
            $$ = SQL_NEW_RULE;
        }
    | SQL_TOKEN_RECURSIVE
    ;

cte_column_list:
    cte_column_list ',' SQL_TOKEN_NAME
        {
            $1->append($3);
            $$ = $1;
        }
    |   SQL_TOKEN_NAME
        {
            $$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);
        }
    ;

cte_table_name:
    SQL_TOKEN_NAME  '(' cte_column_list ')' SQL_TOKEN_AS '(' select_statement ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
            $$->append($5);
            $$->append($6 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($7);
            $$->append($8 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }

cte_block_list:
    cte_block_list ',' cte_table_name
        {
            $1->append($3);
            $$ = $1;
        }
    |
    cte_table_name
        {
            $$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);
        }
    ;

cte:
    SQL_TOKEN_WITH opt_cte_recursive  cte_block_list select_statement
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
        }
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

/* TODO: check whether further usages of opt_column_commalist or column_commalist should actually
 be turned into opt_column_ref_commalist or column_ref_commalist respectively. */
opt_column_commalist:
        /* empty */         {$$ = SQL_NEW_RULE;}
    |       '(' column_commalist ')'
            {$$ = SQL_NEW_RULE;
            $$->append($1 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));}
    ;

opt_column_ref_commalist:
        /* empty */         {$$ = SQL_NEW_RULE;}
    |       '(' column_ref_commalist ')'
            {$$ = SQL_NEW_RULE;
            $$->append($1 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));}
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

sql_not:
    {$$ = SQL_NEW_RULE;}
    |    SQL_TOKEN_NOT
    ;

/* manipulative statements */

manipulative_statement:
            commit_statement
    |       delete_statement_searched
    |       insert_statement
    |       rollback_statement
    |       select_statement_into
    |       update_statement_searched
    |       select_statement
    |       cte
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
        SQL_TOKEN_COMMIT
            {$$ = SQL_NEW_RULE;
            $$->append($1);
            }
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
            $$->append($2 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
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
    ;


rollback_statement:
        SQL_TOKEN_ROLLBACK
        {
        $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    ;


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
            $$->append($1 = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    |    scalar_exp_commalist
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
        { $$ = SQL_NEW_RULE; }
        | from_clause opt_where_clause opt_group_by_clause opt_having_clause opt_order_by_clause opt_limit_offset_clause opt_ecsqloptions_clause
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


opt_disqualify_primary_join:
        /* empty */ {$$ = SQL_NEW_RULE;}
    |    '+' {
                    $$ = SQL_NEW_RULE;
                    $$->append($1 = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
    }


table_ref:
        opt_only opt_disqualify_primary_join table_node_with_opt_member_func_call table_primary_as_range_column
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
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
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
            $$->append($3);
        }
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
      | type_predicate
      ;
boolean_primary:
        predicate
      | unsigned_value_spec
      | fct_spec 
	  | column_ref
      | scalar_subquery
      | value_exp
      | '(' search_condition ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    ;

boolean_test:
        boolean_primary
    |   boolean_primary SQL_TOKEN_IS sql_not truth_value
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

type_predicate:
        '(' type_list ')'
        {
        $$ = SQL_NEW_RULE;
        $$->append($1 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
        $$->append($2);
        $$->append($3 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
        ;
type_list:
        type_list_item
        {
        $$ = SQL_NEW_COMMALISTRULE;
        $$->append($1);
        }
    |   type_list ',' type_list_item
        {
        $1->append($3);
        $$ = $1;
        }
    ;

type_list_item:
    opt_only table_node
    {
    $$ = SQL_NEW_RULE;
    $$->append($1);
    $$->append($2);
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
            if(context->inPredicateCheck()) // comparison_predicate: rule 2
            {
                $$ = SQL_NEW_RULE;
                sal_Int16 nErg = context->buildPredicateRule($$,$2,$1);
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
            $$ = SQL_NEW_RULE; // between_predicate: rule 1
            $$->append($1);
            $$->append($2);
            $$->append($3);
            $$->append($4);
            $$->append($5);
        }
between_predicate:
        row_value_constructor between_predicate_part_2
        {    
            $$ = SQL_NEW_RULE; // between_predicate: rule 1
            $$->append($1);
            $$->append($2);
        }
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
            if (context->inPredicateCheck())  // like_predicate: rule 5
            {
                OSQLParseNode* pColumnRef = CREATE_NODE(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(CREATE_NODE(context->getFieldName(),SQL_NODE_NAME));

                $$ = SQL_NEW_RULE;
                $$->append(pColumnRef);
                $$->append($1);
                OSQLParseNode* p2nd = $1->removeAt(2);
                OSQLParseNode* p3rd = $1->removeAt(2);
                if ( !context->buildLikeRule($1,p2nd,p3rd) )
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
            if (context->inPredicateCheck()) // like_predicate: rule 6
            {
                OSQLParseNode* pColumnRef = CREATE_NODE(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(CREATE_NODE(context->getFieldName(),SQL_NODE_NAME));

                $$ = SQL_NEW_RULE;
                $$->append(pColumnRef);
                $$->append($1);
                OSQLParseNode* p2nd = $1->removeAt(2);
                OSQLParseNode* p3rd = $1->removeAt(2);
                if ( !context->buildLikeRule($1,p2nd,p3rd) )
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
            if (context->inPredicateCheck())// test_for_null: rule 2
            {
                OSQLParseNode* pColumnRef = CREATE_NODE(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(CREATE_NODE(context->getFieldName(),SQL_NODE_NAME));

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
            $$->append($1 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
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
            if ( context->inPredicateCheck() )// in_predicate: rule 2
            {
                OSQLParseNode* pColumnRef = CREATE_NODE(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(CREATE_NODE(context->getFieldName(),SQL_NODE_NAME));

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
            $$->append($1 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
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
    derived_column
    ;

parameter_ref:
        parameter
    ;

literal:
        SQL_TOKEN_INT
    |   SQL_TOKEN_REAL_NUM
    |   SQL_TOKEN_INTNUM
    |   SQL_TOKEN_APPROXNUM
    |    SQL_TOKEN_ACCESS_DATE
/*    rules for predicate check */
    |    literal SQL_TOKEN_STRING
        {
            if (context->inPredicateCheck())
            {
                $$ = SQL_NEW_RULE;
                $$->append($1);
                $$->append($2);
                context->reduceLiteral($$, sal_True);
            }
            else
                YYERROR;
        }
    |    literal SQL_TOKEN_INT
        {
            if (context->inPredicateCheck())
            {
                $$ = SQL_NEW_RULE;
                $$->append($1);
                $$->append($2);
                context->reduceLiteral($$, sal_True);
            }
            else
                YYERROR;
        }
    |    literal SQL_TOKEN_REAL_NUM
        {
            if (context->inPredicateCheck())
            {
                $$ = SQL_NEW_RULE;
                $$->append($1);
                $$->append($2);
                context->reduceLiteral($$, sal_True);
            }
            else
                YYERROR;
        }
    |    literal SQL_TOKEN_APPROXNUM
        {
            if (context->inPredicateCheck())
            {
                $$ = SQL_NEW_RULE;
                $$->append($1);
                $$->append($2);
                context->reduceLiteral($$, sal_True);
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
   
unsigned_value_spec:
        general_value_spec
    |    literal
    ;
general_value_spec:
        parameter
    | SQL_TOKEN_NULL
    | SQL_TOKEN_FALSE
    | SQL_TOKEN_TRUE
    ;
iif_spec:
    SQL_TOKEN_IIF '(' search_condition ',' result ',' result  ')'
        {
        $$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($3);
        $$->append($5);
        $$->append($7);


        }

fct_spec:
        general_set_fct
    |   iif_spec
    |    function_name '(' ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($3 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
       |    function_name '(' function_args_commalist ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    |
        function_name '(' opt_all_distinct function_arg ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        };
        

    ;
    
function_name:
    SQL_TOKEN_NAME
    ;
	
    
general_set_fct:
        set_fct_type '(' opt_all_distinct function_arg ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    |    SQL_TOKEN_COUNT '(' '*' ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($3 = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            $$->append($4 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    |    SQL_TOKEN_COUNT '(' opt_all_distinct function_arg ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
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
        table_ref join_type SQL_TOKEN_JOIN table_ref SQL_TOKEN_USING table_node_with_opt_member_func_call op_relationship_direction
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
            $$->append($2 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
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

cast_target_primitive_type:
      SQL_TOKEN_BINARY 
    | SQL_TOKEN_BLOB
    | SQL_TOKEN_BOOLEAN
    | SQL_TOKEN_DOUBLE
    | SQL_TOKEN_FLOAT
    | SQL_TOKEN_INT
    | SQL_TOKEN_INTEGER
    | SQL_TOKEN_INT64
    | SQL_TOKEN_LONG
    | SQL_TOKEN_REAL
    | SQL_TOKEN_STRING
    | SQL_TOKEN_DATE
    | SQL_TOKEN_TIME
    | SQL_TOKEN_TIMESTAMP
    | SQL_TOKEN_VARCHAR
    | SQL_TOKEN_NAME
    ;

cast_target_scalar:
    cast_target_primitive_type 
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }  
    | SQL_TOKEN_NAME '.' SQL_TOKEN_NAME
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }  
        ;

cast_target_array:
    cast_target_scalar SQL_TOKEN_ARRAY_INDEX
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }  
        ;

cast_target:
    cast_target_scalar
    | cast_target_array
        ;

cast_spec:
      SQL_TOKEN_CAST '(' cast_operand SQL_TOKEN_AS cast_target ')'
      {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($3);
            $$->append($4);
            $$->append($5);
            $$->append($6 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    ;
value_exp_primary:
        unsigned_value_spec
      | fct_spec 
	  | column_ref
      | scalar_subquery
      | case_expression
      | '(' value_exp ')'
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
      | cast_spec
    ;

num_primary:
        value_exp_primary
    ;

factor:
        num_primary
    |    '-' num_primary  %prec SQL_TOKEN_UMINUS
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            $$->append($2);
        }
    |    '+' num_primary  %prec SQL_TOKEN_UMINUS
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            $$->append($2);
        }
    |    SQL_BITWISE_NOT num_primary    %prec SQL_TOKEN_UMINUS
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = CREATE_NODE("~", SQL_NODE_PUNCTUATION));
            $$->append($2);
        }
    ;

term:
        factor
      | term '*' factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | term '/' factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("/", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | term '%' factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("%", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      ;

term_add_sub:
        term
      | term_add_sub '+' term
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | term_add_sub '-' term
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      ;

num_value_exp:
        term_add_sub
      | num_value_exp SQL_BITWISE_SHIFT_LEFT term_add_sub
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("<<", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | num_value_exp SQL_BITWISE_SHIFT_RIGHT term_add_sub
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE(">>", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | num_value_exp SQL_BITWISE_OR term_add_sub
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("|", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      | num_value_exp SQL_BITWISE_AND term_add_sub
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("&", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
      ;

datetime_primary:
     datetime_value_fct
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
      | SQL_TOKEN_CURRENT_TIME
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
      | SQL_TOKEN_TIME string_value_exp
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;

datetime_factor:
        datetime_primary
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    ;
datetime_term:
        datetime_factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    ;

datetime_value_exp:
        datetime_term
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
  ;

value_exp_commalist:
        value_exp
            {$$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);}
    |   value_exp_commalist ',' value_exp
            {$1->append($3);
            $$ = $1;}
    ;

function_arg:
        result 
    ;

function_args_commalist:
        function_arg
            {
            $$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);
            }
    |   function_args_commalist ',' function_arg
            {
            $1->append($3);
            $$ = $1;
            }
    ;
    
value_exp:
        num_value_exp
      | string_value_exp
      | datetime_value_exp
    ;

string_value_exp:
        char_value_exp
    ;

char_value_exp:
        char_factor
    |    concatenation
    ;
concatenation:
        char_value_exp '+' char_factor
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
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
    ;
char_factor:
        char_primary
    ;

derived_column:
        value_exp as_clause
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2);
        }
    ;


table_node:
    qualified_class_name
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
    |   tablespace_qualified_class_name
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        };
        
tablespace_qualified_class_name:
        SQL_TOKEN_NAME '.' qualified_class_name 
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            $$->append($3);
        };

qualified_class_name:
        SQL_TOKEN_NAME '.' class_name 
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            $$->append($3);
        }
        |
        SQL_TOKEN_NAME ':' class_name 
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
            $$->append($2 = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            $$->append($3);
        };

class_name:
        SQL_TOKEN_NAME 
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
;

table_node_with_opt_member_func_call:
        table_node_path 
        {
            $$ = SQL_NEW_RULE;
            $$->append($1);
        }
       ;  

table_node_path:
        table_node_path_entry
            {
            $$ = SQL_NEW_DOTLISTRULE;
            $$->append($1);
            }
    |   table_node_path '.' table_node_path_entry %prec '.'
            {
            $1->append($3);
            $$ = $1;
            }
    |   table_node_path ':' table_node_path_entry %prec ':'
            {
            $1->append($3);
            $$ = $1;
            }
    ;

table_node_path_entry:
        SQL_TOKEN_NAME opt_member_function_args
        {
            $$ = SQL_NEW_RULE;            
            $$->append($1);
            $$->append($2);
        }
    ;

opt_member_function_args:
		{$$ = SQL_NEW_RULE;}
     | '(' function_args_commalist ')'
        {
			$$ = SQL_NEW_RULE;
            $$->append($1 = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            $$->append($2);
            $$->append($3 = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		};

        
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
					SQLyyerror(context, "'*' can only occur at the end of property path\n");
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
            $$->append($1 = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        } 
    ;

column_ref:
		property_path
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}



/* the various things you can name */
/* TODO: change to only allow identifiers, i.e. column must not start with number etc. */
column:
        SQL_TOKEN_NAME
    ;

case_expression:
        case_specification
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
    ;

result_expression:
    value_exp
    ;

case_operand:
    row_value_constructor_elem
    ;
    
parameter:
        ':' SQL_TOKEN_NAME
        {
            $$ = SQL_NEW_RULE;
            $$->append($1 = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            $$->append($2);
        }
    |    '?'
        {
            $$ = SQL_NEW_RULE; // test
            $$->append($1 = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    ;

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

// -------------------------------------------------------------------------
void OSQLParser::setParseTree(OSQLParseNode * pNewParseTree)
    {
    m_pParseTree =std::unique_ptr<OSQLParseNode>(pNewParseTree->detach());
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
//-----------------------------------------------------------------------------
std::unique_ptr<OSQLParseNode> OSQLParser::parseTree (Utf8String& rErrorMessage,Utf8String const& rStatement, sal_Bool bInternational) {
    Utf8String sTemp = delComment(rStatement);
    m_scanner = std::unique_ptr<OSQLScanner>(new OSQLScanner(sTemp.c_str(), m_pContext, sal_True));
    m_pParseTree = nullptr;
    m_sErrorMessage.clear();
    if (SQLyyparse(this) != 0)
        {
        // only set the error message, if it's not already set
        if (!m_sErrorMessage.size())
            m_sErrorMessage = m_scanner->getErrorMessage();

        rErrorMessage = m_sErrorMessage;
        return nullptr;
        }

    return std::move(m_pParseTree);
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

    pLiteral = m_scanner->NewNode(aValue,SQL_NODE_STRING);
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

        Utf8String aError = m_scanner->getErrorMessage();
        if(aError.size())
        {
            m_sErrorMessage += ", ";
            m_sErrorMessage += aError;
        }
    }
}
// -------------------------------------------------------------------------
int OSQLParser::SQLlex(YYSTYPE* val)
{
    return m_scanner->SQLlex(val);
}

#if defined __SUNPRO_CC
#pragma enable_warn
#elif defined _MSC_VER
#pragma warning(pop)
#endif

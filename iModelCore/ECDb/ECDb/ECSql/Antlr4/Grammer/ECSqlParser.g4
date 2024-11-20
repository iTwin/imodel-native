/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 by Bart Kiers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Project : sqlite-parser; an ANTLR4 grammar for SQLite https://github.com/bkiers/sqlite-parser
 * Developed by:
 *     Bart Kiers, bart@big-o.nl
 *     Martin Mirchev, marti_2203@abv.bg
 *     Mike Lische, mike@lischke-online.de
 */

// $antlr-format alignTrailingComments on, columnLimit 130, minEmptyLines 1, maxEmptyLinesToKeep 1, reflowComments off
// $antlr-format useTab off, allowShortRulesOnASingleLine off, allowShortBlocksOnASingleLine on, alignSemicolons ownLine

parser grammar ECSqlParser;

options {
    tokenVocab = ECSqlLexer;
}

parse
    : (sql_stmt_list)* EOF
;

sql_stmt_list
    : SCOL* sql_stmt (SCOL+ sql_stmt)* SCOL*
;

sql_stmt
    : (
        analyze_stmt
        | attach_stmt
        | delete_stmt
        | detach_stmt
        | insert_stmt
        | pragma_stmt
        | select_stmt
        | update_stmt
        | vacuum_stmt
    )
;

analyze_stmt
    : ANALYZE_ (schema_name | (schema_name DOT)? class_or_index_name)?
;

attach_stmt
    : ATTACH_ DATABASE_? expr AS_ schema_name
;


type_name
    : name+? (
        OPEN_PAR signed_number CLOSE_PAR
        | OPEN_PAR signed_number COMMA signed_number CLOSE_PAR
    )?
;

signed_number
    : (PLUS | MINUS)? NUMERIC_LITERAL
;

with_clause
    : WITH_ RECURSIVE_? cte_class_name AS_ OPEN_PAR select_stmt CLOSE_PAR (
        COMMA cte_class_name AS_ OPEN_PAR select_stmt CLOSE_PAR
    )*
;

cte_class_name
    : class_name (OPEN_PAR property_name ( COMMA property_name)* CLOSE_PAR)?
;

recursive_cte
    : cte_class_name AS_ OPEN_PAR initial_select UNION_ ALL_? recursive_select CLOSE_PAR
;

common_class_expression
    : class_name (OPEN_PAR property_name ( COMMA property_name)* CLOSE_PAR)? AS_ OPEN_PAR select_stmt CLOSE_PAR
;

delete_stmt
    : with_clause? DELETE_ FROM_ qualified_class_name (WHERE_ expr)? returning_clause?
;

detach_stmt
    : DETACH_ DATABASE_? schema_name
;

access_string
    : ((schema_name DOT)? class_name DOT)? property_name (DOT property_name)*
;
/*
 SQLite understands the following binary operators, in order from highest to lowest precedence:
    ||
    * / %
    + -
    << >> & |
    < <= > >=
    = == != <> IS IS NOT IN LIKE GLOB MATCH REGEXP
    AND
    OR
 */
expr
    : literal_value
    | BIND_PARAMETER
    | access_string
    | unary_operator expr
    | expr PIPE2 expr
    | expr ( STAR | DIV | MOD) expr
    | expr ( PLUS | MINUS) expr
    | expr ( LT2 | GT2 | AMP | PIPE) expr
    | expr ( LT | LT_EQ | GT | GT_EQ) expr
    | expr (
        ASSIGN
        | EQ
        | NOT_EQ1
        | NOT_EQ2
        | IS_
        | IS_ NOT_
        | IN_
        | LIKE_
        | GLOB_
        | MATCH_
        | REGEXP_
    ) expr
    | expr AND_ expr
    | expr OR_ expr
    | function_name OPEN_PAR ((DISTINCT_? expr ( COMMA expr)*) | STAR)? CLOSE_PAR filter_clause? over_clause?
    | OPEN_PAR expr (COMMA expr)* CLOSE_PAR
    | CAST_ OPEN_PAR expr AS_ type_name CLOSE_PAR
    | expr COLLATE_ collation_name
    | expr NOT_? (LIKE_ | GLOB_ | REGEXP_ | MATCH_) expr (ESCAPE_ expr)?
    | expr ( ISNULL_ | NOTNULL_ | NOT_ NULL_)
    | expr IS_ NOT_? expr
    | expr NOT_? BETWEEN_ expr AND_ expr
    | expr NOT_? IN_ (
        OPEN_PAR (select_stmt | expr ( COMMA expr)*)? CLOSE_PAR
        | ( schema_name DOT)? class_name
        | (schema_name DOT)? class_function_name OPEN_PAR (expr (COMMA expr)*)? CLOSE_PAR
    )
    | ((NOT_)? EXISTS_)? OPEN_PAR select_stmt CLOSE_PAR
    | CASE_ expr? (WHEN_ expr THEN_ expr)+ (ELSE_ expr)? END_
;

literal_value
    : NUMERIC_LITERAL
    | STRING_LITERAL
    | BLOB_LITERAL
    | NULL_
    | TRUE_
    | FALSE_
    | CURRENT_TIME_
    | CURRENT_DATE_
    | CURRENT_TIMESTAMP_
;

value_row
    : OPEN_PAR expr (COMMA expr)* CLOSE_PAR
;

values_clause
    : VALUES_ value_row (COMMA value_row)*
;

insert_stmt
    : with_clause? (
        INSERT_
        | REPLACE_
        | INSERT_ OR_ ( REPLACE_ | ROLLBACK_ | ABORT_ | FAIL_ | IGNORE_)
    ) INTO_ (schema_name DOT)? class_name (AS_ class_alias)? (
        OPEN_PAR property_name ( COMMA property_name)* CLOSE_PAR
    )? (( ( values_clause | select_stmt) upsert_clause?) | DEFAULT_ VALUES_) returning_clause?
;

returning_clause
    : RETURNING_ result_property (COMMA result_property)*
;
indexed_property
    : (property_name | expr) (COLLATE_ collation_name)? asc_desc?
;
upsert_clause
    : ON_ CONFLICT_ (
        OPEN_PAR indexed_property (COMMA indexed_property)* CLOSE_PAR (WHERE_ expr)?
    )? DO_ (
        NOTHING_
        | UPDATE_ SET_ (
            property_name ASSIGN expr (
                COMMA property_name ASSIGN expr
            )* (WHERE_ expr)?
        )
    )
;

pragma_stmt
    : PRAGMA_ (schema_name DOT)? pragma_name (
        ASSIGN pragma_value
        | OPEN_PAR pragma_value CLOSE_PAR
    )?
;

pragma_value
    : signed_number
    | name
    | STRING_LITERAL
;

select_stmt
    : common_class_stmt? select_core (compound_operator select_core)* order_by_stmt? limit_stmt?
;

join_clause
    : class_or_subquery (join_operator class_or_subquery join_constraint?)*
;

select_core
    : (
        SELECT_ (DISTINCT_ | ALL_)? result_property (COMMA result_property)* (
            FROM_ (class_or_subquery (COMMA class_or_subquery)* | join_clause)
        )? (WHERE_ whereExpr = expr)? (
            GROUP_ BY_ groupByExpr += expr (COMMA groupByExpr += expr)* (
                HAVING_ havingExpr = expr
            )?
        )? (WINDOW_ window_name AS_ window_defn ( COMMA window_name AS_ window_defn)*)?
    )
    | values_clause
;

factored_select_stmt
    : select_stmt
;

simple_select_stmt
    : common_class_stmt? select_core order_by_stmt? limit_stmt?
;

compound_select_stmt
    : common_class_stmt? select_core (
        (UNION_ ALL_? | INTERSECT_ | EXCEPT_) select_core
    )+ order_by_stmt? limit_stmt?
;

class_or_subquery
    : (
        (schema_name DOT)? class_name (AS_? class_alias)? (
            INDEXED_ BY_ index_name
            | NOT_ INDEXED_
        )?
    )
    | (schema_name DOT)? class_function_name OPEN_PAR expr (COMMA expr)* CLOSE_PAR (
        AS_? class_alias
    )?
    | OPEN_PAR (class_or_subquery (COMMA class_or_subquery)* | join_clause) CLOSE_PAR
    | OPEN_PAR select_stmt CLOSE_PAR (AS_? class_alias)?
;

result_property
    : STAR
    | class_name DOT STAR
    | expr ( AS_? property_alias)?
;

join_operator
    : COMMA
    | NATURAL_? (LEFT_ OUTER_? | INNER_ | CROSS_)? JOIN_
;

join_constraint
    : ON_ expr
    | USING_ OPEN_PAR property_name ( COMMA property_name)* CLOSE_PAR
;

compound_operator
    : UNION_ ALL_?
    | INTERSECT_
    | EXCEPT_
;

update_stmt
    : with_clause? UPDATE_ (OR_ (ROLLBACK_ | ABORT_ | REPLACE_ | FAIL_ | IGNORE_))? qualified_class_name SET_ (
        property_name
    ) ASSIGN expr (COMMA property_name ASSIGN expr)* (
        FROM_ (class_or_subquery (COMMA class_or_subquery)* | join_clause)
    )? (WHERE_ expr)? returning_clause?
;

qualified_class_name
    : (schema_name DOT)? class_name (AS_ alias)? (
        INDEXED_ BY_ index_name
        | NOT_ INDEXED_
    )?
;

vacuum_stmt
    : VACUUM_ schema_name? (INTO_ filename)?
;

filter_clause
    : FILTER_ OPEN_PAR WHERE_ expr CLOSE_PAR
;

window_defn
    : OPEN_PAR base_window_name? (PARTITION_ BY_ expr (COMMA expr)*)? (
        ORDER_ BY_ ordering_term (COMMA ordering_term)*
    ) frame_spec? CLOSE_PAR
;

over_clause
    : OVER_ (
        window_name
        | OPEN_PAR base_window_name? (PARTITION_ BY_ expr (COMMA expr)*)? (
            ORDER_ BY_ ordering_term (COMMA ordering_term)*
        )? frame_spec? CLOSE_PAR
    )
;

frame_spec
    : frame_clause (EXCLUDE_ ( NO_ OTHERS_ | CURRENT_ ROW_ | GROUP_ | TIES_))?
;

frame_clause
    : (RANGE_ | ROWS_ | GROUPS_) (
        frame_single
        | BETWEEN_ frame_left AND_ frame_right
    )
;

simple_function_invocation
    : simple_func OPEN_PAR (expr (COMMA expr)* | STAR) CLOSE_PAR
;

aggregate_function_invocation
    : aggregate_func OPEN_PAR (DISTINCT_? expr (COMMA expr)* | STAR)? CLOSE_PAR filter_clause?
;

window_function_invocation
    : window_function OPEN_PAR (expr (COMMA expr)* | STAR)? CLOSE_PAR filter_clause? OVER_ (
        window_defn
        | window_name
    )
;

common_class_stmt
    : //additional structures
    WITH_ RECURSIVE_? common_class_expression (COMMA common_class_expression)*
;

order_by_stmt
    : ORDER_ BY_ ordering_term (COMMA ordering_term)*
;

limit_stmt
    : LIMIT_ expr ((OFFSET_ | COMMA) expr)?
;

ordering_term
    : expr (COLLATE_ collation_name)? asc_desc? (NULLS_ (FIRST_ | LAST_))?
;

asc_desc
    : ASC_
    | DESC_
;

frame_left
    : expr PRECEDING_
    | expr FOLLOWING_
    | CURRENT_ ROW_
    | UNBOUNDED_ PRECEDING_
;

frame_right
    : expr PRECEDING_
    | expr FOLLOWING_
    | CURRENT_ ROW_
    | UNBOUNDED_ FOLLOWING_
;

frame_single
    : expr PRECEDING_
    | UNBOUNDED_ PRECEDING_
    | CURRENT_ ROW_
;

// unknown

window_function
    : (FIRST_VALUE_ | LAST_VALUE_) OPEN_PAR expr CLOSE_PAR OVER_ OPEN_PAR partition_by? order_by_expr_asc_desc frame_clause?
        CLOSE_PAR
    | (CUME_DIST_ | PERCENT_RANK_) OPEN_PAR CLOSE_PAR OVER_ OPEN_PAR partition_by? order_by_expr? CLOSE_PAR
    | (DENSE_RANK_ | RANK_ | ROW_NUMBER_) OPEN_PAR CLOSE_PAR OVER_ OPEN_PAR partition_by? order_by_expr_asc_desc CLOSE_PAR
    | (LAG_ | LEAD_) OPEN_PAR expr offset? default_value? CLOSE_PAR OVER_ OPEN_PAR partition_by? order_by_expr_asc_desc CLOSE_PAR
    | NTH_VALUE_ OPEN_PAR expr COMMA signed_number CLOSE_PAR OVER_ OPEN_PAR partition_by? order_by_expr_asc_desc frame_clause?
        CLOSE_PAR
    | NTILE_ OPEN_PAR expr CLOSE_PAR OVER_ OPEN_PAR partition_by? order_by_expr_asc_desc CLOSE_PAR
;

offset
    : COMMA signed_number
;

default_value
    : COMMA signed_number
;

partition_by
    : PARTITION_ BY_ expr+
;

order_by_expr
    : ORDER_ BY_ expr+
;

order_by_expr_asc_desc
    : ORDER_ BY_ expr_asc_desc
;

expr_asc_desc
    : expr asc_desc? (COMMA expr asc_desc?)*
;

//TODO BOTH OF THESE HAVE TO BE REWORKED TO FOLLOW THE SPEC
initial_select
    : select_stmt
;

recursive_select
    : select_stmt
;

unary_operator
    : MINUS
    | PLUS
    | TILDE
    | NOT_
;

error_message
    : STRING_LITERAL
;


property_alias
    : IDENTIFIER
    | STRING_LITERAL
;

keyword
    : ABORT_
    | ACTION_
    | ADD_
    | AFTER_
    | ALL_
    | ALTER_
    | ANALYZE_
    | AND_
    | AS_
    | ASC_
    | ATTACH_
    | AUTOINCREMENT_
    | BEFORE_
    | BEGIN_
    | BETWEEN_
    | BY_
    | CASCADE_
    | CASE_
    | CAST_
    | CHECK_
    | COLLATE_
    | COLUMN_
    | COMMIT_
    | CONFLICT_
    | CONSTRAINT_
    | CREATE_
    | CROSS_
    | CURRENT_DATE_
    | CURRENT_TIME_
    | CURRENT_TIMESTAMP_
    | DATABASE_
    | DEFAULT_
    | DEFERRABLE_
    | DEFERRED_
    | DELETE_
    | DESC_
    | DETACH_
    | DISTINCT_
    | DROP_
    | EACH_
    | ELSE_
    | END_
    | ESCAPE_
    | EXCEPT_
    | EXCLUSIVE_
    | EXISTS_
    | EXPLAIN_
    | FAIL_
    | FOR_
    | FOREIGN_
    | FROM_
    | FULL_
    | GLOB_
    | GROUP_
    | HAVING_
    | IF_
    | IGNORE_
    | IMMEDIATE_
    | IN_
    | INDEX_
    | INDEXED_
    | INITIALLY_
    | INNER_
    | INSERT_
    | INSTEAD_
    | INTERSECT_
    | INTO_
    | IS_
    | ISNULL_
    | JOIN_
    | KEY_
    | LEFT_
    | LIKE_
    | LIMIT_
    | MATCH_
    | NATURAL_
    | NO_
    | NOT_
    | NOTNULL_
    | NULL_
    | OF_
    | OFFSET_
    | ON_
    | OR_
    | ORDER_
    | OUTER_
    | PLAN_
    | PRAGMA_
    | PRIMARY_
    | QUERY_
    | RAISE_
    | RECURSIVE_
    | REFERENCES_
    | REGEXP_
    | REINDEX_
    | RELEASE_
    | RENAME_
    | REPLACE_
    | RESTRICT_
    | RIGHT_
    | ROLLBACK_
    | ROW_
    | ROWS_
    | SAVEPOINT_
    | SELECT_
    | SET_
    | TABLE_
    | TEMP_
    | TEMPORARY_
    | THEN_
    | TO_
    | TRANSACTION_
    | TRIGGER_
    | UNION_
    | UNIQUE_
    | UPDATE_
    | USING_
    | VACUUM_
    | VALUES_
    | VIEW_
    | VIRTUAL_
    | WHEN_
    | WHERE_
    | WITH_
    | WITHOUT_
    | FIRST_VALUE_
    | OVER_
    | PARTITION_
    | RANGE_
    | PRECEDING_
    | UNBOUNDED_
    | CURRENT_
    | FOLLOWING_
    | CUME_DIST_
    | DENSE_RANK_
    | LAG_
    | LAST_VALUE_
    | LEAD_
    | NTH_VALUE_
    | NTILE_
    | PERCENT_RANK_
    | RANK_
    | ROW_NUMBER_
    | GENERATED_
    | ALWAYS_
    | STORED_
    | TRUE_
    | FALSE_
    | WINDOW_
    | NULLS_
    | FIRST_
    | LAST_
    | FILTER_
    | GROUPS_
    | EXCLUDE_
;

// TODO: check all names below

name
    : any_name
;

function_name
    : any_name
;

schema_name
    : any_name
;

class_name
    : any_name
;

class_or_index_name
    : any_name
;

property_name
    : any_name
;

collation_name
    : any_name
;

index_name
    : any_name
;

module_name
    : any_name
;

pragma_name
    : any_name
;

class_alias
    : any_name
;

window_name
    : any_name
;

alias
    : any_name
;

filename
    : any_name
;

base_window_name
    : any_name
;

simple_func
    : any_name
;

aggregate_func
    : any_name
;

class_function_name
    : any_name
;

any_name
    : IDENTIFIER
    | keyword
    | STRING_LITERAL
    | OPEN_PAR any_name CLOSE_PAR
;
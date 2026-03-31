/* =============================================================================
 * ECSql.y — BISON grammar for ECSQL
 *
 * Generated from the ECSqlRDParser.cpp recursive-descent parser.
 * This grammar describes the syntax accepted by the ECSQL parser; semantic
 * actions are omitted (shown as comments) since they depend on the C++ AST.
 *
 * Operator precedence (value expressions, lowest → highest):
 *   |   (bitwise OR)
 *   &   (bitwise AND)
 *   << >>  (shift)
 *   + -    (additive)
 *   * / %  (multiplicative)
 *   ||     (concatenation)
 *   - + ~  (unary prefix)
 *
 * Boolean precedence (lowest → highest):
 *   OR
 *   AND
 *   NOT
 *   predicate
 *
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * =========================================================================== */

%{
/* C declarations — include AST headers, helpers, etc. */
%}

/* ───────────────────────────── Token declarations ─────────────────────────── */

/* Literal / identifier tokens */
%token NAME            /* bare identifier */
%token INTEGER_LITERAL /* integer literal, e.g. 42 or 0xFF */
%token APPROX_LITERAL  /* floating-point literal, e.g. 3.14 or 1e5 */
%token STRING_LITERAL  /* single-quoted string literal */
%token PARAMETER       /* ? positional parameter */
%token NAMED_PARAM     /* :name named parameter */
%token DOLLAR          /* $ token (instance access paths) */

/* Punctuation / operator tokens */
%token LESS GREAT EQUAL LESS_EQ GREAT_EQ NOT_EQUAL
%token CONCAT          /* || */
%token ARROW           /* -> */
%token BITWISE_NOT     /* ~ */
%token BITWISE_OR      /* | */
%token BITWISE_AND     /* & */
%token SHIFT_LEFT      /* << */
%token SHIFT_RIGHT     /* >> */

/* Single-character punctuation */
%token MINUS PLUS STAR SLASH PERCENT COLON
%token LPAREN RPAREN COMMA DOT SEMICOLON
%token LBRACE RBRACE LBRACKET RBRACKET

/* Keywords */
%token KW_ALL KW_AND KW_ANY KW_AS KW_ASC KW_AVG
%token KW_BACKWARD KW_BETWEEN KW_BINARY KW_BLOB KW_BOOLEAN KW_BY
%token KW_CASE KW_CAST KW_COLLATE KW_COUNT KW_CROSS
%token KW_CUME_DIST KW_CURRENT KW_CURRENT_DATE KW_CURRENT_TIME KW_CURRENT_TIMESTAMP
%token KW_DATE KW_DELETE KW_DENSE_RANK KW_DESC KW_DISTINCT KW_DOUBLE
%token KW_ECSQLOPTIONS KW_ELSE KW_END KW_ESCAPE KW_EVERY KW_EXCEPT
%token KW_EXCLUDE KW_EXISTS KW_FALSE KW_FILTER KW_FIRST KW_FIRST_VALUE
%token KW_FLOAT KW_FOLLOWING KW_FOR KW_FORWARD KW_FROM KW_FULL
%token KW_GROUP KW_GROUP_CONCAT KW_GROUPS KW_HAVING
%token KW_IIF KW_IN KW_INNER KW_INSERT KW_INT KW_INT64 KW_INTERSECT KW_INTO
%token KW_IS KW_JOIN
%token KW_LAG KW_LAST KW_LAST_VALUE KW_LEAD KW_LEFT KW_LIKE KW_LIMIT KW_LONG
%token KW_MATCH KW_MAX KW_MIN KW_NATURAL KW_NAVIGATION_VALUE
%token KW_NO KW_NOCASE KW_NOT KW_NTH_VALUE KW_NTILE KW_NULL KW_NULLS
%token KW_OFFSET KW_ON KW_ONLY KW_OPTIONS KW_OR KW_ORDER KW_OTHERS KW_OUTER KW_OVER
%token KW_PARTITION KW_PERCENT_RANK KW_PRAGMA KW_PRECEDING
%token KW_RANGE KW_RANK KW_REAL KW_RECURSIVE KW_RIGHT
%token KW_ROW KW_ROWS KW_ROW_NUMBER KW_RTRIM
%token KW_SELECT KW_SET KW_SOME KW_STRING_KW KW_SUM
%token KW_THEN KW_TIES KW_TIME KW_TIMESTAMP KW_TOTAL KW_TRUE
%token KW_UNBOUNDED KW_UNION KW_UNIQUE KW_UNKNOWN KW_UPDATE KW_USING
%token KW_VALUE KW_VALUES KW_VARCHAR KW_WHEN KW_WHERE KW_WINDOW KW_WITH

/* ───────────────── Operator precedence (value expressions) ─────────────── */
/* Lowest precedence first */
%left BITWISE_OR
%left BITWISE_AND
%left SHIFT_LEFT SHIFT_RIGHT
%left PLUS MINUS
%left STAR SLASH PERCENT
%left CONCAT
%right UNARY_MINUS UNARY_PLUS BITWISE_NOT

/* Boolean precedence */
%left KW_OR
%left KW_AND
%right KW_NOT

%%

/* ==========================================================================
 * Top-level entry
 * ========================================================================== */

ecsql_statement
    : cte_statement opt_semicolon
    | select_statement opt_semicolon
    | insert_statement opt_semicolon
    | update_statement opt_semicolon
    | delete_statement opt_semicolon
    | pragma_statement opt_semicolon
    ;

opt_semicolon
    : /* empty */
    | SEMICOLON
    ;

/* ==========================================================================
 * INSERT statement
 * INSERT INTO [ONLY] table_node [(col_list)] VALUES (val_list)
 * ========================================================================== */

insert_statement
    : KW_INSERT KW_INTO opt_only_constraint table_node opt_column_ref_commalist
        KW_VALUES LPAREN row_value_constructor_commalist RPAREN
    ;

opt_only_constraint
    : /* empty */
    | KW_ONLY
    ;

/* ==========================================================================
 * UPDATE statement
 * UPDATE [ALL|ONLY] table_node SET assignments [WHERE cond] [OPTIONS ...]
 * ========================================================================== */

update_statement
    : KW_UPDATE opt_all_or_only table_node KW_SET assignment_commalist
        opt_where_clause opt_ecsqloptions_clause
    ;

/* ==========================================================================
 * DELETE statement
 * DELETE FROM [ALL|ONLY] table_node [WHERE cond] [OPTIONS ...]
 * ========================================================================== */

delete_statement
    : KW_DELETE KW_FROM opt_all_or_only table_node
        opt_where_clause opt_ecsqloptions_clause
    ;

/* ==========================================================================
 * PRAGMA statement
 * PRAGMA name [= value | (value)] [FOR path] [OPTIONS ...]
 * ========================================================================== */

pragma_statement
    : KW_PRAGMA identifier opt_pragma_value opt_pragma_for_path opt_ecsqloptions_clause
    ;

opt_pragma_value
    : /* empty */
    | EQUAL pragma_value
    | LPAREN pragma_value RPAREN
    ;

pragma_value
    : KW_TRUE
    | KW_FALSE
    | KW_NULL
    | INTEGER_LITERAL
    | APPROX_LITERAL
    | STRING_LITERAL
    | identifier
    ;

opt_pragma_for_path
    : /* empty */
    | KW_FOR pragma_path
    ;

pragma_path
    : NAME pragma_path_tail
    ;

pragma_path_tail
    : /* empty */
    | DOT NAME pragma_path_tail
    | COLON NAME pragma_path_tail
    | NAMED_PARAM pragma_path_tail
    ;

/* ==========================================================================
 * CTE (Common Table Expression)
 * WITH [RECURSIVE] cte_block [, cte_block ...] select_statement
 * ========================================================================== */

cte_statement
    : KW_WITH opt_recursive cte_block_list select_statement
    ;

opt_recursive
    : /* empty */
    | KW_RECURSIVE
    ;

cte_block_list
    : cte_block
    | cte_block_list COMMA cte_block
    ;

cte_block
    : identifier opt_column_name_list KW_AS LPAREN select_statement RPAREN
    ;

opt_column_name_list
    : /* empty */
    | LPAREN column_name_commalist RPAREN
    ;

column_name_commalist
    : identifier
    | column_name_commalist COMMA identifier
    ;

/* ==========================================================================
 * SELECT statement (handles UNION / INTERSECT / EXCEPT)
 * ========================================================================== */

select_statement
    : values_statement
    | single_select_statement
    | single_select_statement compound_op opt_all select_statement
    ;

values_statement
    : KW_VALUES values_row_list
    | KW_VALUES values_row_list compound_op opt_all select_statement
    ;

values_row_list
    : LPAREN row_value_constructor_commalist RPAREN
    | values_row_list COMMA LPAREN row_value_constructor_commalist RPAREN
    ;

compound_op
    : KW_UNION
    | KW_INTERSECT
    | KW_EXCEPT
    ;

opt_all
    : /* empty */
    | KW_ALL
    ;

/* ==========================================================================
 * Single SELECT statement
 * SELECT [ALL|DISTINCT] selection [FROM ...] [WHERE ...] [GROUP BY ...]
 *     [HAVING ...] [WINDOW ...] [ORDER BY ...] [LIMIT ...] [OPTIONS ...]
 * ========================================================================== */

single_select_statement
    : KW_SELECT opt_set_quantifier selection
        opt_from_clause
        opt_where_clause
        opt_group_by_clause
        opt_having_clause
        opt_window_clause
        opt_order_by_clause
        opt_limit_offset_clause
        opt_ecsqloptions_clause
    ;

opt_set_quantifier
    : /* empty */
    | KW_ALL
    | KW_DISTINCT
    ;

/* ==========================================================================
 * Selection (SELECT clause)
 * ========================================================================== */

selection
    : select_item_list
    ;

select_item_list
    : select_item
    | select_item_list COMMA select_item
    ;

select_item
    : STAR
    | derived_column
    ;

derived_column
    : value_exp opt_column_alias
    ;

opt_column_alias
    : /* empty */
    | KW_AS identifier
    | NAME                      /* implicit alias (bare identifier) */
    ;

/* ==========================================================================
 * FROM clause
 * ========================================================================== */

opt_from_clause
    : /* empty */
    | KW_FROM table_ref_commalist
    ;

table_ref_commalist
    : table_ref_and_joins
    | table_ref_commalist COMMA table_ref_and_joins
    ;

table_ref_and_joins
    : table_ref
    | table_ref_and_joins join_clause
    ;

/* ==========================================================================
 * Table reference
 * [+](ALL|ONLY) [+] ( subquery | table_name | TVF | CTE_name ) [AS alias]
 * ========================================================================== */

table_ref
    : opt_polymorphic_constraint opt_disqualify_prefix table_ref_body opt_table_alias
    ;

opt_disqualify_prefix
    : /* empty */
    | PLUS
    ;

table_ref_body
    : subquery_ref
    | cte_block_name
    | table_valued_function
    | class_name_ref
    | class_name_ref DOT member_function_call
    ;

subquery_ref
    : subquery
    ;

cte_block_name
    : NAME
    ;

table_valued_function
    : NAME LPAREN opt_value_exp_commalist RPAREN
    | NAME DOT NAME LPAREN opt_value_exp_commalist RPAREN
    ;

class_name_ref
    : NAME DOT NAME                         /* schema.Class */
    | NAME NAMED_PARAM                      /* schema:Class */
    | NAME DOT NAME DOT NAME               /* tableSpace.schema.Class */
    ;

member_function_call
    : NAME LPAREN opt_value_exp_commalist RPAREN
    ;

opt_table_alias
    : /* empty */
    | KW_AS identifier
    | NAME
    ;

/* ==========================================================================
 * Polymorphic constraint: [+](ALL | ONLY)
 * ========================================================================== */

opt_polymorphic_constraint
    : /* empty */
    | PLUS KW_ALL
    | PLUS KW_ONLY
    | KW_ALL
    | KW_ONLY
    ;

opt_all_or_only
    : /* empty */
    | KW_ALL
    | KW_ONLY
    ;

/* ==========================================================================
 * Table node (class name) — [tableSpace.]schema.ClassName
 * Used in INSERT/UPDATE/DELETE where no TVF/CTE resolution is needed.
 * ========================================================================== */

table_node
    : NAME DOT NAME
    | NAME NAMED_PARAM
    | NAME DOT NAME DOT NAME
    ;

/* ==========================================================================
 * JOIN clause
 * ========================================================================== */

join_clause
    : opt_natural cross_join_or_qualified_join
    ;

opt_natural
    : /* empty */
    | KW_NATURAL
    ;

cross_join_or_qualified_join
    : KW_CROSS KW_JOIN table_ref
    | KW_JOIN table_ref join_spec
    | join_type opt_outer KW_JOIN table_ref join_spec
    | KW_USING relationship_ref opt_join_direction
    ;

relationship_ref
    : NAME table_ref                        /* USING RELATIONSHIP tableRef */
    | table_ref                             /* USING tableRef */
    ;

opt_join_direction
    : /* empty */
    | KW_FORWARD
    | KW_BACKWARD
    ;

join_type
    : KW_INNER
    | KW_LEFT
    | KW_RIGHT
    | KW_FULL
    ;

opt_outer
    : /* empty */
    | KW_OUTER
    ;

/* ==========================================================================
 * Join specification
 * ========================================================================== */

join_spec
    : KW_ON search_condition
    | KW_USING LPAREN column_name_commalist RPAREN
    ;

/* ==========================================================================
 * Subquery — ( SELECT ... | WITH ... | VALUES ... )
 * ========================================================================== */

subquery
    : LPAREN select_statement RPAREN
    | LPAREN cte_statement RPAREN
    ;

/* ==========================================================================
 * WHERE clause
 * ========================================================================== */

opt_where_clause
    : /* empty */
    | KW_WHERE search_condition
    ;

/* ==========================================================================
 * GROUP BY clause
 * ========================================================================== */

opt_group_by_clause
    : /* empty */
    | KW_GROUP KW_BY value_exp_commalist
    ;

/* ==========================================================================
 * HAVING clause
 * ========================================================================== */

opt_having_clause
    : /* empty */
    | KW_HAVING search_condition
    ;

/* ==========================================================================
 * ORDER BY clause
 * ========================================================================== */

opt_order_by_clause
    : /* empty */
    | KW_ORDER KW_BY order_by_spec_list
    ;

order_by_spec_list
    : order_by_spec
    | order_by_spec_list COMMA order_by_spec
    ;

order_by_spec
    : order_by_sort_expr opt_sort_direction opt_nulls_order
    ;

order_by_sort_expr
    : value_exp
    | value_exp comparison_op value_exp
    | value_exp KW_IS opt_not is_rhs_literal
    ;

opt_sort_direction
    : /* empty */
    | KW_ASC
    | KW_DESC
    ;

opt_nulls_order
    : /* empty */
    | KW_NULLS KW_FIRST
    | KW_NULLS KW_LAST
    ;

/* ==========================================================================
 * LIMIT / OFFSET clause
 * ========================================================================== */

opt_limit_offset_clause
    : /* empty */
    | KW_LIMIT value_exp
    | KW_LIMIT value_exp KW_OFFSET value_exp
    ;

/* ==========================================================================
 * ECSQLOPTIONS clause
 * ========================================================================== */

opt_ecsqloptions_clause
    : /* empty */
    | KW_ECSQLOPTIONS option_list
    ;

option_list
    : option_item
    | option_list opt_comma option_item
    ;

opt_comma
    : /* empty */
    | COMMA
    ;

option_item
    : NAME
    | NAME EQUAL option_value
    ;

option_value
    : KW_TRUE
    | KW_FALSE
    | identifier
    | literal
    ;

/* ==========================================================================
 * WINDOW clause (named window definitions)
 * ========================================================================== */

opt_window_clause
    : /* empty */
    | KW_WINDOW window_definition_list
    ;

window_definition_list
    : window_definition
    | window_definition_list COMMA window_definition
    ;

window_definition
    : identifier KW_AS LPAREN window_specification RPAREN
    ;

/* ==========================================================================
 * VALUES helpers (for INSERT)
 * ========================================================================== */

row_value_constructor_commalist
    : value_exp
    | row_value_constructor_commalist COMMA value_exp
    ;

/* ==========================================================================
 * Column reference list (for INSERT column list)
 * ========================================================================== */

opt_column_ref_commalist
    : /* empty */
    | LPAREN column_ref_commalist RPAREN
    ;

column_ref_commalist
    : property_name_exp
    | column_ref_commalist COMMA property_name_exp
    ;

/* ==========================================================================
 * Assignment list (for UPDATE SET)
 * ========================================================================== */

assignment_commalist
    : assignment
    | assignment_commalist COMMA assignment
    ;

assignment
    : property_name_exp EQUAL value_exp
    ;

/* ==========================================================================
 * Value expressions — precedence climbing
 * ========================================================================== */

value_exp
    : value_exp_or
    ;

value_exp_or
    : value_exp_and
    | value_exp_or BITWISE_OR value_exp_and
    ;

value_exp_and
    : value_exp_shift
    | value_exp_and BITWISE_AND value_exp_shift
    ;

value_exp_shift
    : value_exp_addsub
    | value_exp_shift SHIFT_LEFT value_exp_addsub
    | value_exp_shift SHIFT_RIGHT value_exp_addsub
    ;

value_exp_addsub
    : value_exp_muldiv
    | value_exp_addsub PLUS value_exp_muldiv
    | value_exp_addsub MINUS value_exp_muldiv
    ;

value_exp_muldiv
    : value_exp_concat
    | value_exp_muldiv STAR value_exp_concat
    | value_exp_muldiv SLASH value_exp_concat
    | value_exp_muldiv PERCENT value_exp_concat
    ;

value_exp_concat
    : value_exp_unary
    | value_exp_concat CONCAT value_exp_unary
    ;

value_exp_unary
    : value_exp_primary
    | MINUS value_exp_primary     %prec UNARY_MINUS
    | PLUS value_exp_primary      %prec UNARY_PLUS
    | BITWISE_NOT value_exp_primary
    ;

/* ==========================================================================
 * Primary value expressions (atoms)
 * ========================================================================== */

value_exp_primary
    : literal
    | parameter
    | dollar_access
    | paren_value_or_subquery
    | cast_spec
    | case_exp
    | iif_exp
    | navigation_value_exp
    | current_datetime_func
    | datetime_literal
    | rtrim_func
    | column_ref_or_func
    | STAR                              /* used in contexts like COUNT(*) */
    ;

/* ── Literals ── */

literal
    : KW_NULL
    | KW_TRUE
    | KW_FALSE
    | INTEGER_LITERAL
    | APPROX_LITERAL
    | STRING_LITERAL
    ;

is_rhs_literal
    : KW_NULL
    | KW_TRUE
    | KW_FALSE
    | KW_UNKNOWN
    ;

/* ── Parameters ── */

parameter
    : PARAMETER
    | NAMED_PARAM
    ;

/* ── Dollar / instance access ── */

dollar_access
    : DOLLAR
    | DOLLAR ARROW property_path
    | DOLLAR ARROW property_path PARAMETER          /* optional marker */
    ;

/* ── Parenthesised expression or subquery ── */

paren_value_or_subquery
    : subquery                                     /* (SELECT ...) */
    | LPAREN value_exp RPAREN
    ;

/* ── CAST ── */

cast_spec
    : KW_CAST LPAREN value_exp KW_AS cast_type opt_array_marker RPAREN
    ;

cast_type
    : type_keyword
    | NAME                              /* plain type name */
    | NAME DOT NAME                     /* schema.TypeName */
    ;

type_keyword
    : KW_BINARY
    | KW_BLOB
    | KW_BOOLEAN
    | KW_DATE
    | KW_DOUBLE
    | KW_FLOAT
    | KW_INT
    | KW_INT64
    | KW_LONG
    | KW_REAL
    | KW_STRING_KW
    | KW_TIME
    | KW_TIMESTAMP
    | KW_VARCHAR
    ;

opt_array_marker
    : /* empty */
    | LBRACKET RBRACKET
    ;

/* ── CASE ── */

case_exp
    : KW_CASE when_clause_list opt_else_clause KW_END
    ;

when_clause_list
    : when_clause
    | when_clause_list when_clause
    ;

when_clause
    : KW_WHEN search_condition KW_THEN value_exp
    ;

opt_else_clause
    : /* empty */
    | KW_ELSE value_exp
    ;

/* ── IIF ── */

iif_exp
    : KW_IIF LPAREN search_condition COMMA value_exp COMMA value_exp RPAREN
    ;

/* ── NAVIGATION_VALUE ── */

navigation_value_exp
    : KW_NAVIGATION_VALUE LPAREN derived_column COMMA value_exp opt_comma_value_exp RPAREN
    ;

opt_comma_value_exp
    : /* empty */
    | COMMA value_exp
    ;

/* ── CURRENT_DATE / CURRENT_TIME / CURRENT_TIMESTAMP ── */

current_datetime_func
    : KW_CURRENT_DATE
    | KW_CURRENT_TIME
    | KW_CURRENT_TIMESTAMP
    ;

/* ── DATE / TIME / TIMESTAMP literals ── */

datetime_literal
    : KW_DATE STRING_LITERAL
    | KW_TIME STRING_LITERAL
    | KW_TIMESTAMP STRING_LITERAL
    ;

/* ── RTRIM (keyword-function) ── */

rtrim_func
    : KW_RTRIM LPAREN opt_func_args RPAREN
    ;

/* ==========================================================================
 * Column reference or function call
 * Handles: identifier, property paths, function calls, enum values,
 *          instance access via alias.$
 * ========================================================================== */

column_ref_or_func
    : NAME opt_property_path_or_func_tail
    ;

opt_property_path_or_func_tail
    : /* empty — bare identifier */                         opt_array_index
    | LPAREN opt_set_quantifier opt_func_args RPAREN opt_window_over   /* function call */
    | DOT property_path_continuation
    ;

property_path_continuation
    : DOLLAR                                               /* alias.$ */
    | DOLLAR ARROW property_path opt_optional_marker        /* alias.$ -> path */
    | STAR                                                 /* alias.* */
    | NAME opt_array_index dot_path_rest                   /* a.b[0].c... */
    | NAME LPAREN opt_set_quantifier opt_func_args RPAREN opt_window_over  /* schema.func() */
    ;

dot_path_rest
    : /* empty */
    | DOT NAME opt_array_index dot_path_rest
    | ARROW property_path opt_optional_marker               /* -> extraction */
    ;

opt_array_index
    : /* empty */
    | LBRACKET INTEGER_LITERAL RBRACKET
    ;

opt_optional_marker
    : /* empty */
    | PARAMETER                         /* trailing ? for optional extraction */
    ;

/* ==========================================================================
 * Property path (used in -> extraction RHS)
 * ========================================================================== */

property_path
    : property_path_entry
    | property_path DOT property_path_entry
    ;

property_path_entry
    : identifier opt_array_index
    ;

/* ==========================================================================
 * Property name expression (for assignment LHS, column lists)
 * ========================================================================== */

property_name_exp
    : NAME opt_array_index
    | NAME DOT NAME opt_array_index
    | NAME DOT NAME opt_array_index DOT NAME opt_array_index dot_path_rest
    ;

/* ==========================================================================
 * Function call
 * ========================================================================== */

function_call
    : identifier LPAREN opt_set_quantifier opt_func_args RPAREN
    ;

opt_func_args
    : /* empty */
    | func_arg_list
    ;

func_arg_list
    : value_exp
    | func_arg_list COMMA value_exp
    ;

/* ==========================================================================
 * Aggregate function call
 * COUNT(* | [ALL|DISTINCT] expr)
 * SUM / AVG / MIN / MAX / TOTAL / GROUP_CONCAT / ANY / EVERY / SOME
 * ========================================================================== */

aggregate_function
    : aggregate_name LPAREN opt_set_quantifier aggregate_arg RPAREN
    ;

aggregate_name
    : KW_COUNT
    | KW_SUM
    | KW_AVG
    | KW_MIN
    | KW_MAX
    | KW_GROUP_CONCAT
    | KW_TOTAL
    | KW_ANY
    | KW_EVERY
    | KW_SOME
    ;

aggregate_arg
    : STAR                              /* COUNT(*) */
    | value_exp                         /* single argument */
    | value_exp COMMA value_exp         /* GROUP_CONCAT(expr, separator) */
    ;

/* ==========================================================================
 * Window function
 * func_call [FILTER (WHERE ...)] OVER (window_spec | window_name)
 * ========================================================================== */

opt_window_over
    : /* empty */
    | opt_filter_clause KW_OVER window_name_or_spec
    ;

window_name_or_spec
    : identifier
    | LPAREN window_specification RPAREN
    ;

opt_filter_clause
    : /* empty */
    | KW_FILTER LPAREN KW_WHERE search_condition RPAREN
    ;

/* ==========================================================================
 * Window specification
 * [existing_window_name] [PARTITION BY ...] [ORDER BY ...] [frame_clause]
 * ========================================================================== */

window_specification
    : opt_existing_window_name opt_partition_clause opt_window_order_by opt_frame_clause
    ;

opt_existing_window_name
    : /* empty */
    | NAME
    ;

opt_partition_clause
    : /* empty */
    | KW_PARTITION KW_BY partition_column_list
    ;

partition_column_list
    : partition_column_ref
    | partition_column_list COMMA partition_column_ref
    ;

partition_column_ref
    : column_ref_or_func opt_collate_clause
    ;

opt_collate_clause
    : /* empty */
    | KW_COLLATE KW_BINARY
    | KW_COLLATE KW_NOCASE
    | KW_COLLATE KW_RTRIM
    ;

opt_window_order_by
    : /* empty */
    | KW_ORDER KW_BY order_by_spec_list
    ;

/* ==========================================================================
 * Window frame clause
 * (ROWS | RANGE | GROUPS) (frame_start | BETWEEN bound AND bound) [EXCLUDE ...]
 * ========================================================================== */

opt_frame_clause
    : /* empty */
    | frame_unit frame_extent opt_frame_exclusion
    ;

frame_unit
    : KW_ROWS
    | KW_RANGE
    | KW_GROUPS
    ;

frame_extent
    : frame_start
    | KW_BETWEEN first_frame_bound KW_AND second_frame_bound
    ;

frame_start
    : KW_UNBOUNDED KW_PRECEDING
    | KW_CURRENT KW_ROW
    | value_exp KW_PRECEDING
    ;

first_frame_bound
    : KW_UNBOUNDED KW_PRECEDING
    | KW_CURRENT KW_ROW
    | value_exp KW_PRECEDING
    | value_exp KW_FOLLOWING
    ;

second_frame_bound
    : KW_UNBOUNDED KW_FOLLOWING
    | KW_CURRENT KW_ROW
    | value_exp KW_PRECEDING
    | value_exp KW_FOLLOWING
    ;

opt_frame_exclusion
    : /* empty */
    | KW_EXCLUDE KW_GROUP
    | KW_EXCLUDE KW_TIES
    | KW_EXCLUDE KW_CURRENT KW_ROW
    | KW_EXCLUDE KW_NO KW_OTHERS
    ;

/* ==========================================================================
 * Search condition (boolean expressions)
 * ========================================================================== */

search_condition
    : search_condition_or
    ;

search_condition_or
    : search_condition_and
    | search_condition_or KW_OR search_condition_and
    ;

search_condition_and
    : search_condition_not
    | search_condition_and KW_AND search_condition_not
    ;

search_condition_not
    : predicate
    | KW_NOT predicate
    ;

/* ==========================================================================
 * Predicate
 * ========================================================================== */

predicate
    : LPAREN search_condition RPAREN                      /* parenthesised boolean */
    | KW_EXISTS subquery                                  /* EXISTS (SELECT ...) */
    | KW_UNIQUE subquery                                  /* UNIQUE (SELECT ...) */
    | value_exp                                           /* bare value as boolean */
    | value_exp comparison_op value_exp                   /* comparison */
    | value_exp comparison_op quantifier subquery         /* ALL/ANY/SOME comparison */
    | value_exp KW_IS opt_not is_rhs_literal              /* IS [NOT] NULL/TRUE/FALSE/UNKNOWN */
    | value_exp KW_IS opt_not type_predicate              /* IS [NOT] (type_list) */
    | value_exp opt_not KW_BETWEEN value_exp KW_AND value_exp  /* [NOT] BETWEEN */
    | value_exp opt_not KW_LIKE value_exp opt_escape      /* [NOT] LIKE */
    | value_exp opt_not KW_IN in_rhs                      /* [NOT] IN */
    | value_exp opt_not KW_MATCH function_call            /* [NOT] MATCH func() */
    ;

opt_not
    : /* empty */
    | KW_NOT
    ;

comparison_op
    : EQUAL
    | LESS
    | GREAT
    | LESS_EQ
    | GREAT_EQ
    | NOT_EQUAL
    ;

quantifier
    : KW_ALL
    | KW_ANY
    | KW_SOME
    ;

/* ==========================================================================
 * Type predicate — IS [NOT] ( [ALL|ONLY] schema.Class, ... )
 * ========================================================================== */

type_predicate
    : LPAREN type_list RPAREN
    ;

type_list
    : type_list_entry
    | type_list COMMA type_list_entry
    ;

type_list_entry
    : opt_polymorphic_constraint table_node
    ;

/* ==========================================================================
 * IN predicate RHS
 * ========================================================================== */

in_rhs
    : subquery                                          /* IN (SELECT ...) */
    | LPAREN value_exp_commalist RPAREN                 /* IN (val, val, ...) */
    ;

/* ==========================================================================
 * LIKE ESCAPE clause
 * ========================================================================== */

opt_escape
    : /* empty */
    | KW_ESCAPE STRING_LITERAL
    ;

/* ==========================================================================
 * Value expression comma list
 * ========================================================================== */

value_exp_commalist
    : value_exp
    | value_exp_commalist COMMA value_exp
    ;

opt_value_exp_commalist
    : /* empty */
    | value_exp_commalist
    ;

/* ==========================================================================
 * Window function names (for disambiguation)
 * ========================================================================== */

window_function_name
    : KW_ROW_NUMBER
    | KW_RANK
    | KW_DENSE_RANK
    | KW_PERCENT_RANK
    | KW_CUME_DIST
    | KW_NTILE
    | KW_LEAD
    | KW_LAG
    | KW_FIRST_VALUE
    | KW_LAST_VALUE
    | KW_NTH_VALUE
    ;

/* ==========================================================================
 * Identifier — NAME or keyword used as identifier
 * ========================================================================== */

identifier
    : NAME
    | keyword_as_identifier
    ;

/* Keywords that can be used as identifiers in certain contexts */
keyword_as_identifier
    : KW_ALL | KW_ANY | KW_ASC | KW_AVG | KW_BACKWARD
    | KW_BINARY | KW_BLOB | KW_BOOLEAN | KW_BY
    | KW_CASE | KW_CAST | KW_COLLATE | KW_COUNT | KW_CROSS
    | KW_CUME_DIST | KW_CURRENT | KW_CURRENT_DATE | KW_CURRENT_TIME | KW_CURRENT_TIMESTAMP
    | KW_DATE | KW_DENSE_RANK | KW_DESC | KW_DISTINCT | KW_DOUBLE
    | KW_ECSQLOPTIONS | KW_ELSE | KW_END | KW_ESCAPE | KW_EVERY | KW_EXCEPT
    | KW_EXCLUDE | KW_EXISTS | KW_FALSE | KW_FILTER | KW_FIRST | KW_FIRST_VALUE
    | KW_FLOAT | KW_FOLLOWING | KW_FOR | KW_FORWARD | KW_FULL
    | KW_GROUP | KW_GROUP_CONCAT | KW_GROUPS | KW_HAVING
    | KW_IIF | KW_INNER | KW_INT | KW_INT64 | KW_INTERSECT
    | KW_JOIN
    | KW_LAG | KW_LAST | KW_LAST_VALUE | KW_LEAD | KW_LEFT | KW_LIMIT | KW_LONG
    | KW_MAX | KW_MIN | KW_NATURAL | KW_NAVIGATION_VALUE
    | KW_NO | KW_NOCASE | KW_NTH_VALUE | KW_NTILE | KW_NULL | KW_NULLS
    | KW_OFFSET | KW_ON | KW_ONLY | KW_OPTIONS | KW_ORDER | KW_OTHERS | KW_OUTER | KW_OVER
    | KW_PARTITION | KW_PERCENT_RANK | KW_PRAGMA | KW_PRECEDING
    | KW_RANGE | KW_RANK | KW_REAL | KW_RECURSIVE | KW_RIGHT
    | KW_ROW | KW_ROWS | KW_ROW_NUMBER | KW_RTRIM
    | KW_SET | KW_SOME | KW_STRING_KW | KW_SUM
    | KW_THEN | KW_TIES | KW_TIME | KW_TIMESTAMP | KW_TOTAL | KW_TRUE
    | KW_UNBOUNDED | KW_UNION | KW_UNIQUE | KW_UNKNOWN
    | KW_USING | KW_VALUE | KW_VALUES | KW_VARCHAR
    | KW_WHEN | KW_WINDOW
    ;

%%

/* ==========================================================================
 * Notes on correspondence with ECSqlRDParser.cpp:
 *
 * 1. PRECEDENCE NAMING MISMATCH: The C++ methods have misleading names:
 *    - ParseValueExpOr()     actually handles  |  (bitwise OR)
 *    - ParseValueExpAnd()    actually handles  &  (bitwise AND)
 *    - ParseValueExpBitOr()  actually handles  << >> (shifts)
 *    - ParseValueExpBitAnd() actually handles  + -   (additive)
 *    - ParseValueExpShift()  actually handles  * / % (multiplicative)
 *    - ParseValueExpAddSub() actually handles  ||    (concatenation)
 *    The grammar above uses the CORRECT operation names.
 *
 * 2. SPECULATIVE BACKTRACKING: ParsePredicate uses speculative parsing
 *    for parenthesised expressions — trying value_exp first, then falling
 *    back to (search_condition) if that fails. In BISON this is expressed
 *    as separate alternatives; a GLR parser (%glr-parser) may be needed
 *    for ambiguous cases.
 *
 * 3. NAMED_PARAM DUAL USE: The lexer emits NAMED_PARAM for both :param
 *    (query parameters) and schema:ClassName (the colon + name is one token).
 *    Context in the grammar disambiguates these.
 *
 * 4. VIEW CLASS EXPANSION: When a table reference resolves to a view class,
 *    the parser recursively parses the view's ECSQL. This is a semantic
 *    action, not a grammar concern.
 *
 * 5. AGGREGATE vs WINDOW FUNCTIONS: The parser checks whether a function
 *    name is an aggregate or window function to select the appropriate
 *    parse path. In the grammar, this is expressed via separate rules
 *    (aggregate_function, window_function_name) though BISON may need
 *    semantic predicates for full disambiguation.
 * ========================================================================== */

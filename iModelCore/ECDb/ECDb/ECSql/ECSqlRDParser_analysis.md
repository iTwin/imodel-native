# ECSqlRDParser Complete Analysis
## Recursive Descent Parser → BISON Grammar Reference

---

## Token Types (from ECSqlLexer.h)

### Literal/Identifier Tokens
| Token | Description |
|---|---|
| `Name` | Bare identifier |
| `IntNum` | Integer literal (42, 0xFF) |
| `ApproxNum` | Float literal (3.14, 1e5) |
| `String` | Single-quoted string literal |
| `AccessDate` | Date literal |
| `Parameter` | `?` positional parameter |
| `NamedParam` | `:name` named parameter |
| `Dollar` | `$` instance access |

### Operators
| Token | Symbol |
|---|---|
| `Less` | `<` |
| `Great` | `>` |
| `Equal` | `=` |
| `LessEq` | `<=` |
| `GreatEq` | `>=` |
| `NotEqual` | `<>` or `!=` |
| `Concat` | `\|\|` |
| `Arrow` | `->` |
| `BitwiseNot` | `~` |
| `BitwiseOr` | `\|` |
| `BitwiseAnd` | `&` |
| `ShiftLeft` | `<<` |
| `ShiftRight` | `>>` |

### Single-char Punctuation
`-` `+` `*` `/` `%` `:` `(` `)` `,` `.` `;` `?` `{` `}` `[` `]`

### Keywords (KW_*)
ALL, AND, ANY, AS, ASC, AVG, BACKWARD, BETWEEN, BINARY, BLOB, BOOLEAN, BY, CASE, CAST, COLLATE, COUNT, CROSS, CUME_DIST, CURRENT, CURRENT_DATE, CURRENT_TIME, CURRENT_TIMESTAMP, DATE, DELETE, DENSE_RANK, DESC, DISTINCT, DOUBLE, ECSQLOPTIONS, ELSE, END, ESCAPE, EVERY, EXCEPT, EXCLUDE, EXISTS, FALSE, FILTER, FIRST, FIRST_VALUE, FLOAT, FOLLOWING, FOR, FORWARD, FROM, FULL, GROUP, GROUP_CONCAT, GROUPS, HAVING, IIF, IN, INNER, INSERT, INT, INT64, INTERSECT, INTO, IS, JOIN, LAG, LAST, LAST_VALUE, LEAD, LEFT, LIKE, LIMIT, LONG, MATCH, MAX, MIN, NATURAL, NAVIGATION_VALUE, NO, NOCASE, NOT, NTH_VALUE, NTILE, NULL, NULLS, OFFSET, ON, ONLY, OPTIONS, OR, ORDER, OTHERS, OUTER, OVER, PARTITION, PERCENT_RANK, PRAGMA, PRECEDING, RANGE, RANK, REAL, RECURSIVE, RIGHT, ROW, ROWS, ROW_NUMBER, RTRIM, SELECT, SET, SOME, STRING_KW, SUM, THEN, TIES, TIME, TIMESTAMP, TOTAL, TRUE, UNBOUNDED, UNION, UNIQUE, UNKNOWN, UPDATE, USING, VALUE, VALUES, VARCHAR, WHEN, WHERE, WINDOW, WITH

---

## Parsing Methods — Complete Analysis

---

### 1. `Parse` (Top-level entry point)

**Grammar Rule:**
```
ecsql_statement
    : common_table_exp
    | select_statement
    | insert_statement
    | update_statement
    | delete_statement
    | pragma_statement
    ;
```

**Tokens Expected/Matched:**
- `KW_WITH` → calls `ParseCTE`
- `KW_SELECT` | `KW_VALUES` → calls `ParseSelectStatement`
- `KW_INSERT` → calls `ParseInsertStatement`
- `KW_UPDATE` → calls `ParseUpdateStatementSearched`
- `KW_DELETE` → calls `ParseDeleteStatementSearched`
- `KW_PRAGMA` → calls `ParsePragmaStatement`
- Optional trailing `Semicolon`
- Must reach `EndOfInput` after statement

**Calls:** `ParseCTE`, `ParseSelectStatement`, `ParseInsertStatement`, `ParseUpdateStatementSearched`, `ParseDeleteStatementSearched`, `ParsePragmaStatement`

---

### 2. `ParseInsertStatement`

**Grammar Rule:**
```
insert_statement
    : INSERT INTO opt_only_constraint table_node opt_column_ref_commalist values_or_query_spec
    ;

opt_only_constraint
    : /* empty */   → defaults to ONLY
    | ONLY
    ;
    /* ALL is explicitly rejected for INSERT */
```

**Tokens Expected/Matched:**
- Consumes `KW_INSERT`
- Expects `KW_INTO`
- Rejects `KW_ALL` (error)
- Optional `KW_ONLY`

**Calls:** `ParseTableNode`, `ParseOptColumnRefCommalist`, `ParseValuesOrQuerySpec`

---

### 3. `ParseUpdateStatementSearched`

**Grammar Rule:**
```
update_statement
    : UPDATE opt_polymorphic_constraint table_node SET assignment_commalist opt_where_clause opt_ecsqloptions_clause
    ;

opt_polymorphic_constraint
    : /* empty */
    | ALL
    | ONLY
    ;
```

**Tokens Expected/Matched:**
- Consumes `KW_UPDATE`
- Optional `KW_ALL` | `KW_ONLY`
- Expects `KW_SET`

**Calls:** `ParseTableNode`, `ParseAssignmentCommalist`, `ParseWhereClause`, `ParseOptECSqlOptionsClause`

---

### 4. `ParseDeleteStatementSearched`

**Grammar Rule:**
```
delete_statement
    : DELETE FROM opt_polymorphic_constraint table_node opt_where_clause opt_ecsqloptions_clause
    ;
```

**Tokens Expected/Matched:**
- Consumes `KW_DELETE`
- Expects `KW_FROM`
- Optional `KW_ALL` | `KW_ONLY`

**Calls:** `ParseTableNode`, `ParseWhereClause`, `ParseOptECSqlOptionsClause`

---

### 5. `ParsePragmaStatement`

**Grammar Rule:**
```
pragma_statement
    : PRAGMA pragma_name opt_pragma_value opt_for_path opt_ecsqloptions_clause
    ;

pragma_name
    : Name | keyword
    ;

opt_pragma_value
    : /* empty */          → isReadValue=true
    | '=' pragma_value     → isReadValue=false
    | '(' pragma_value ')' → isReadValue=true
    ;

pragma_value
    : TRUE | FALSE | NULL | IntNum | ApproxNum | String | Name | keyword
    ;

opt_for_path
    : /* empty */
    | FOR name_dot_chain
    ;

name_dot_chain
    : Name ( '.' Name | ':' Name | NamedParam )*
    ;
```

**Tokens Expected/Matched:**
- Consumes `KW_PRAGMA`
- Expects `Name` or keyword for pragma name
- `Equal` → assigns value (isReadValue=false)
- `LParen` → parenthesized value
- `KW_FOR` → path with `Name`, `Dot`, `Colon`, `NamedParam` separators

**Calls:** `ParseOptECSqlOptionsClause`

---

### 6. `ParseCTE`

**Grammar Rule:**
```
common_table_exp
    : WITH opt_recursive cte_block_list select_statement
    ;

opt_recursive
    : /* empty */
    | RECURSIVE
    ;

cte_block_list
    : cte_block ( ',' cte_block )*
    ;
```

**Tokens Expected/Matched:**
- Consumes `KW_WITH`
- Optional `KW_RECURSIVE`
- Expects `KW_SELECT` | `KW_VALUES` after blocks

**Calls:** `ParseCTEBlock` (in loop), `ParseSelectStatement`

---

### 7. `ParseCTEBlock`

**Grammar Rule:**
```
cte_block
    : block_name opt_column_list AS '(' select_statement ')'
    ;

block_name
    : Name | keyword
    ;

opt_column_list
    : /* empty */
    | '(' column_name ( ',' column_name )* ')'
    ;

column_name
    : Name | keyword
    ;
```

**Tokens Expected/Matched:**
- `Name` or keyword for block name
- Optional `LParen` → column list with `Name`/keyword items, comma-separated, `RParen`
- Expects `KW_AS`, `LParen`
- Expects `KW_SELECT` | `KW_VALUES` inside
- Expects `RParen`
- If `isRecursive` and no columns → error

**Calls:** `ParseSelectStatement`

---

### 8. `ParseSelectStatement`

**Grammar Rule:**
```
select_statement
    : VALUES values_rows opt_compound_tail
    | single_select_statement opt_compound_tail
    ;

values_rows
    : '(' row_value_constructor_commalist ')' ( ',' '(' row_value_constructor_commalist ')' )*
    ;

opt_compound_tail
    : /* empty */
    | compound_op opt_all select_statement
    ;

compound_op
    : UNION | INTERSECT | EXCEPT
    ;

opt_all
    : /* empty */
    | ALL
    ;
```

**Tokens Expected/Matched:**
- `KW_VALUES` branch: consumes VALUES, then `LParen`/`RParen` pairs with comma-separated values
- Otherwise delegates to `ParseSingleSelectStatement`
- After either branch: checks for `KW_UNION` | `KW_INTERSECT` | `KW_EXCEPT`
- Optional `KW_ALL` after compound operator
- Left-hand SELECT must not have ORDER BY or LIMIT in compound (IsCoreSelect check)

**Calls:** `ParseSingleSelectStatement`, `ParseRowValueConstructorCommalist`, `ParseSelectStatement` (recursive for RHS)

---

### 9. `ParseSingleSelectStatement`

**Grammar Rule:**
```
single_select_statement
    : SELECT opt_set_quantifier selection opt_from_clause opt_where_clause
      opt_group_by_clause opt_having_clause opt_window_clause
      opt_order_by_clause opt_limit_offset_clause opt_ecsqloptions_clause
    ;

opt_set_quantifier
    : /* empty */
    | ALL
    | DISTINCT
    ;
```

**Tokens Expected/Matched:**
- Expects `KW_SELECT`
- Optional `KW_ALL` | `KW_DISTINCT`
- `KW_FROM` → from clause
- `KW_WHERE` → where clause
- `KW_GROUP` → group by clause
- `KW_HAVING` → having clause
- `KW_WINDOW` → window clause
- `KW_ORDER` → order by clause
- `KW_LIMIT` → limit/offset clause
- `KW_ECSQLOPTIONS` → options clause

**Calls:** `ParseSelection`, `ParseFromClause`, `ParseWhereClause`, `ParseGroupByClause`, `ParseHavingClause`, `ParseWindowClause`, `ParseOrderByClause`, `ParseLimitOffsetClause`, `ParseOptECSqlOptionsClause`

---

### 10. `ParseSelection`

**Grammar Rule:**
```
selection
    : '*' opt_more_columns
    | derived_column_list
    ;

opt_more_columns
    : /* empty */
    | ',' derived_column_or_star_list
    ;

derived_column_or_star_list
    : derived_column_or_star ( ',' derived_column_or_star )*
    ;

derived_column_or_star
    : '*'
    | derived_column
    ;
```

**Tokens Expected/Matched:**
- `Star` → wildcard, optionally followed by `Comma` and more columns
- Otherwise comma-separated `derived_column` items
- `Star` can appear mid-list

**Calls:** `ParseDerivedColumn`

---

### 11. `ParseDerivedColumn`

**Grammar Rule:**
```
derived_column
    : value_exp opt_alias
    ;

opt_alias
    : /* empty */
    | AS (Name | keyword)
    | Name   /* implicit alias */
    ;
```

**Tokens Expected/Matched:**
- After value_exp: `KW_AS` then `Name`/keyword, or bare `Name` (implicit alias)

**Calls:** `ParseValueExp`

---

### 12. `ParseWhereClause`

**Grammar Rule:**
```
opt_where_clause
    : /* empty */
    | WHERE search_condition
    ;
```

**Tokens Expected/Matched:**
- Optional; only if `KW_WHERE` is current
- Consumes `KW_WHERE`

**Calls:** `ParseSearchCondition`

---

### 13. `ParseGroupByClause`

**Grammar Rule:**
```
opt_group_by_clause
    : /* empty */
    | GROUP BY value_exp_commalist
    ;
```

**Tokens Expected/Matched:**
- `KW_GROUP` then `KW_BY`
- Comma-separated value expressions

**Calls:** `ParseValueExp` (in loop)

---

### 14. `ParseHavingClause`

**Grammar Rule:**
```
opt_having_clause
    : /* empty */
    | HAVING search_condition
    ;
```

**Tokens Expected/Matched:** `KW_HAVING`

**Calls:** `ParseSearchCondition`

---

### 15. `ParseOrderByClause`

**Grammar Rule:**
```
opt_order_by_clause
    : /* empty */
    | ORDER BY order_by_spec_list
    ;

order_by_spec_list
    : order_by_spec ( ',' order_by_spec )*
    ;

order_by_spec
    : sort_expression opt_sort_direction opt_nulls_order
    ;

sort_expression
    : value_exp
    | value_exp comparison_op value_exp
    | value_exp IS opt_not (NULL | TRUE | FALSE | UNKNOWN)
    ;

opt_sort_direction
    : /* empty */
    | ASC
    | DESC
    ;

opt_nulls_order
    : /* empty */
    | NULLS FIRST
    | NULLS LAST
    ;
```

**Tokens Expected/Matched:**
- `KW_ORDER`, `KW_BY`
- After value_exp: comparison operators, or `KW_IS` [NOT] NULL/TRUE/FALSE/UNKNOWN
- `KW_ASC` | `KW_DESC`
- `KW_NULLS` then `KW_FIRST` | `KW_LAST`

**Calls:** `ParseValueExp`, `ParseComparisonOp`, `ParseLiteral`, `IsComparisonOp`

---

### 16. `ParseLimitOffsetClause`

**Grammar Rule:**
```
opt_limit_offset_clause
    : /* empty */
    | LIMIT value_exp
    | LIMIT value_exp OFFSET value_exp
    ;
```

**Tokens Expected/Matched:** `KW_LIMIT`, optional `KW_OFFSET`

**Calls:** `ParseValueExp`

---

### 17. `ParseOptECSqlOptionsClause`

**Grammar Rule:**
```
opt_ecsqloptions_clause
    : /* empty */
    | ECSQLOPTIONS option_list
    ;

option_list
    : option ( opt_comma option )*
    ;
    /* continues while current token is Name (not a SQL keyword) */

option
    : option_name opt_option_value
    ;

option_name
    : Name | keyword
    ;

opt_option_value
    : /* empty */
    | '=' option_value
    ;

option_value
    : TRUE | FALSE
    | Name | keyword   → string
    | literal           → typed
    ;
```

**Tokens Expected/Matched:**
- `KW_ECSQLOPTIONS` (also matches OPTIONS)
- `Name`/keyword for option names
- `Equal` then value
- `KW_TRUE`/`KW_FALSE` for boolean values
- Optional `Comma` between options
- Loop continues while `At(Name)` (stops at SQL keywords)

**Calls:** `ParseLiteral`

---

### 18. `ParseWindowClause`

**Grammar Rule:**
```
opt_window_clause
    : /* empty */
    | WINDOW window_definition_list
    ;

window_definition_list
    : window_definition ( ',' window_definition )*
    ;

window_definition
    : window_name AS '(' window_specification ')'
    ;

window_name
    : Name | keyword
    ;
```

**Tokens Expected/Matched:** `KW_WINDOW`, `Name`/keyword, `KW_AS`, `LParen`, `RParen`

**Calls:** `ParseWindowSpecification`

---

### 19. `ParseFromClause`

**Grammar Rule:**
```
from_clause
    : FROM table_ref_and_opt_joins ( ',' table_ref_and_opt_joins )*
    ;
```

**Tokens Expected/Matched:** `KW_FROM`, comma-separated table refs

**Calls:** `ParseTableRefAndOptJoins`

---

### 20. `ParseTableRefAndOptJoins` (internal helper)

**Grammar Rule:**
```
table_ref_and_opt_joins
    : table_ref ( join_clause )*
    ;
```

**Tokens Expected/Matched:** Checks `IsJoinKeyword()` after each table ref

**Calls:** `ParseTableRef`, `ParseJoinedTable`, `IsJoinKeyword`

---

### 21. `ParsePolymorphicConstraint`

**Grammar Rule:**
```
opt_polymorphic_constraint
    : /* empty */        → NotSpecified
    | '+' ALL            → All(disqualify=true)
    | '+' ONLY           → Only(disqualify=true)
    | ALL                → All(disqualify=false)
    | ONLY               → Only(disqualify=false)
    ;
```

**Tokens Expected/Matched:**
- `Plus` (only if followed by `KW_ALL` | `KW_ONLY` — uses `Peek1()`)
- `KW_ALL` | `KW_ONLY`

**Calls:** None (uses `Peek1`)

---

### 22. `ParseTableRef`

**Grammar Rule:**
```
table_ref
    : opt_polymorphic_constraint opt_disqualify_prefix table_ref_body opt_alias
    ;

opt_disqualify_prefix
    : /* empty */
    | '+'
    ;

table_ref_body
    : '(' subquery ')' opt_alias                         → SubqueryRefExp
    | Name '(' args ')' opt_alias                        → TVF (no schema)
    | Name                                               → CTE block name (single ident, select only)
    | Name ('.' | NamedParam) Name '(' args ')' opt_alias → TVF (with schema)
    | Name ('.' | NamedParam) Name ['.' Name ['.' Name '(' args ')']] opt_alias → ClassNameExp
    ;

opt_alias
    : /* empty */
    | AS (Name | keyword)
    | Name   /* implicit alias */
    ;
```

**Tokens Expected/Matched:**
- `Plus` (disqualify, errors before subquery)
- `LParen` → subquery path
- `Name` then:
  - `LParen` (select + polymorphic) → TVF without schema
  - No `Dot`/`NamedParam` (select + polymorphic) → CTE block name
  - `NamedParam` → schema:ClassName
  - `Dot` → two/three/four-part name resolution
    - After `schema.name`, if `LParen` (select+poly) → TVF with schema
    - Three dots → tableSpace.schema.class or schema.class.memberFunc
    - Four parts → tableSpace.schema.class.memberFunc
  - View class expansion (SELECT only) → re-parses view query with new `ECSqlRDParser`
- Alias: `KW_AS` Name/keyword or bare `Name`

**Calls:** `ParsePolymorphicConstraint`, `ParseSubquery`, `ParseTableValuedFunction`, `ParseTableNode`, `ParseValueExp` (for member func args), `m_context->TryResolveClass`

---

### 23. `ParseTableNode`

**Grammar Rule:**
```
table_node
    : Name ('.' | NamedParam) Name opt_third_part
    ;

opt_third_part
    : /* empty */           → two-part: schema.class
    | '.' Name              → three-part: tableSpace.schema.class
    ;
```

**Tokens Expected/Matched:**
- `Name` (first part)
- `NamedParam` (schema:ClassName) **or** `Dot` then `Name`
- Optional second `Dot` then `Name` (three-part)

**Calls:** `m_context->TryResolveClass`

---

### 24. `ParseTableNodeWithOptMemberCall`

**Grammar Rule:**
```
table_node_with_member_call
    : path_entry ( '.' path_entry )*
    ;

path_entry
    : (Name | keyword) opt_func_args
    ;

opt_func_args
    : /* empty */
    | '(' value_exp_commalist ')'
    | '(' ')'
    ;
```

**Constraints:** Path must have 2–4 entries. Last entry may have func args. Schema:Class via `NamedParam`.

**Tokens Expected/Matched:** `Name`/keyword, `Dot`, `LParen`, `RParen`, `Comma`, `NamedParam`

**Calls:** `ParseValueExp` (for func args), `m_context->TryResolveClass`

---

### 25. `ParseTableValuedFunction`

**Grammar Rule:**
```
table_valued_function
    : '(' opt_value_exp_commalist ')'
    ;
    /* schemaName and functionName passed in by caller */
```

**Tokens Expected/Matched:** `LParen`, optional args (value_exp, comma-separated), `RParen`

**Calls:** `ParseValueExp`

---

### 26. `ParseSubquery`

**Grammar Rule:**
```
subquery
    : '(' subquery_body ')'
    ;

subquery_body
    : common_table_exp      (WITH ...)
    | select_statement       (SELECT ...)
    | select_statement       (VALUES ...)
    ;
```

**Tokens Expected/Matched:** `LParen`, `KW_WITH` | `KW_SELECT` | `KW_VALUES`, `RParen`

**Calls:** `ParseCTE`, `ParseSelectStatement`

---

### 27. `ParseJoinedTable`

**Grammar Rule:**
```
joined_table
    : opt_natural opt_cross_or_qualified_join table_ref opt_join_spec
    ;

opt_natural
    : /* empty */
    | NATURAL
    ;

opt_cross_or_qualified_join
    : CROSS JOIN                                → CrossJoinExp
    | JOIN                                      → InnerJoin (default)
    | join_type opt_outer JOIN                   → QualifiedJoin
    ;

opt_join_spec
    : join_spec                                 → ON/USING
    | USING opt_relationship rel_table_ref opt_direction → UsingRelationshipJoinExp
    ;

opt_relationship
    : /* empty */
    | RELATIONSHIP   /* contextual keyword, matched as Name */
    ;

opt_direction
    : /* empty */  → Implied
    | FORWARD
    | BACKWARD
    ;
```

**Tokens Expected/Matched:**
- `KW_NATURAL`
- `KW_CROSS` then `KW_JOIN`
- `KW_JOIN` alone
- `KW_INNER`/`KW_LEFT`/`KW_RIGHT`/`KW_FULL` (via ParseJoinType) then optional `KW_OUTER` then `KW_JOIN`
- `KW_USING` → peeks: if next is `LParen` → standard USING; otherwise → relationship join
  - Relationship: optional `Name("RELATIONSHIP")`, then table ref, optional `KW_FORWARD`/`KW_BACKWARD`

**Calls:** `ParseJoinType`, `ParseTableRef`, `ParseJoinSpec`, `Peek1()`

---

### 28. `ParseJoinType`

**Grammar Rule:**
```
join_type
    : INNER           → InnerJoin
    | LEFT [OUTER]    → LeftOuterJoin
    | RIGHT [OUTER]   → RightOuterJoin
    | FULL [OUTER]    → FullOuterJoin
    | /* default */   → InnerJoin
    ;
```

**Tokens Expected/Matched:** `KW_INNER`, `KW_LEFT`, `KW_RIGHT`, `KW_FULL`, optional `KW_OUTER`

**Calls:** None

---

### 29. `ParseJoinSpec`

**Grammar Rule:**
```
join_spec
    : ON search_condition        → JoinConditionExp
    | USING '(' column_list ')'  → NamedPropertiesJoinExp
    ;

column_list
    : (Name | keyword) ( ',' (Name | keyword) )*
    ;
```

**Tokens Expected/Matched:** `KW_ON` | `KW_USING`, `LParen`, `RParen`, `Name`/keyword, `Comma`

**Calls:** `ParseSearchCondition`

---

### 30. `ParseValueExp`

**Grammar Rule:**
```
value_exp
    : value_exp_or
    ;
```

Direct delegation to `ParseValueExpOr`.

---

### 31. `ParseValueExpOr`

**Grammar Rule:**
```
value_exp_or
    : value_exp_and ( '|' value_exp_and )*
    ;
```

**Tokens:** `BitwiseOr` as binary infix operator

**Calls:** `ParseValueExpAnd`

---

### 32. `ParseValueExpAnd`

**Grammar Rule:**
```
value_exp_and
    : value_exp_bit_or ( '&' value_exp_bit_or )*
    ;
```

**Tokens:** `BitwiseAnd`

**Calls:** `ParseValueExpBitOr`

---

### 33. `ParseValueExpBitOr` (actually handles shifts)

**Grammar Rule:**
```
value_exp_shift
    : value_exp_bit_and ( ('<<' | '>>') value_exp_bit_and )*
    ;
```

**Tokens:** `ShiftLeft`, `ShiftRight`

**Calls:** `ParseValueExpBitAnd`

---

### 34. `ParseValueExpBitAnd` (actually handles add/sub)

**Grammar Rule:**
```
value_exp_add_sub
    : value_exp_shift ( ('+' | '-') value_exp_shift )*
    ;
```

**Tokens:** `Plus`, `Minus`

**Calls:** `ParseValueExpShift`

---

### 35. `ParseValueExpShift` (actually handles mul/div)

**Grammar Rule:**
```
value_exp_mul_div
    : value_exp_add_sub ( ('*' | '/' | '%') value_exp_add_sub )*
    ;
```

**Tokens:** `Star`, `Slash`, `Percent`

**Calls:** `ParseValueExpAddSub`

---

### 36. `ParseValueExpAddSub` (actually handles concat)

**Grammar Rule:**
```
value_exp_concat
    : value_exp_mul_div ( '||' value_exp_mul_div )*
    ;
```

**Tokens:** `Concat`

**Calls:** `ParseValueExpMulDiv`

---

### 37. `ParseValueExpMulDiv` → delegates to `ParseValueExpConcat`

### 38. `ParseValueExpConcat` → delegates to `ParseValueExpUnary`

**Note:** These two are pass-throughs. The actual operator precedence chain (lowest→highest) is:
1. BitwiseOr (`|`) — `ParseValueExpOr`
2. BitwiseAnd (`&`) — `ParseValueExpAnd`
3. Shifts (`<<`, `>>`) — `ParseValueExpBitOr`
4. Add/Sub (`+`, `-`) — `ParseValueExpBitAnd`
5. Mul/Div/Mod (`*`, `/`, `%`) — `ParseValueExpShift`
6. Concat (`||`) — `ParseValueExpAddSub`
7. Unary (`-`, `+`, `~`) — `ParseValueExpUnary`
8. Primary — `ParseValueExpPrimary`

---

### 39. `ParseValueExpUnary`

**Grammar Rule:**
```
value_exp_unary
    : ('-' | '+' | '~') value_exp_primary
    | value_exp_primary
    ;
```

**Tokens:** `Minus`, `Plus`, `BitwiseNot`

**Calls:** `ParseValueExpPrimary`

---

### 40. `ParseValueExpPrimary`

**Grammar Rule (many alternatives):**
```
value_exp_primary
    : NULL                          → LiteralValueExp
    | TRUE                          → LiteralValueExp
    | FALSE                         → LiteralValueExp
    | IntNum                        → LiteralValueExp (Long)
    | ApproxNum                     → LiteralValueExp (Double)
    | String                        → LiteralValueExp (String, quotes stripped)
    | '?'                           → ParameterExp (positional)
    | NamedParam                    → ParameterExp (named)
    | '$'                           → ParseColumnRef (instance access)
    | '(' SELECT|WITH|VALUES ')'    → SubqueryValueExp (via ParseSubquery)
    | '(' value_exp ')'             → parenthesised expr (SetHasParentheses)
    | CAST                          → ParseCastSpec
    | CASE                          → ParseCaseExp
    | IIF                           → ParseIIFExp
    | NAVIGATION_VALUE              → ParseValueCreationFuncExp
    | EXISTS                        → error (only valid in boolean context)
    | CURRENT_DATE                  → FunctionCallExp
    | CURRENT_TIME                  → FunctionCallExp
    | CURRENT_TIMESTAMP             → FunctionCallExp
    | DATE String                   → LiteralValueExp (DateTime)
    | TIME String                   → LiteralValueExp (DateTime)
    | TIMESTAMP String              → LiteralValueExp (DateTime)
    | RTRIM                         → ParseFctSpecByName("RTRIM")
    | Name | keyword                → ParseColumnRef (may become function call or property path)
    | '*'                           → LiteralValueExp (ASTERISK, Kind::Varies)
    ;
```

**Tokens Expected/Matched:** `KW_NULL`, `KW_TRUE`, `KW_FALSE`, `IntNum`, `ApproxNum`, `String`, `Parameter`, `NamedParam`, `Dollar`, `LParen`, `KW_CAST`, `KW_CASE`, `KW_IIF`, `KW_NAVIGATION_VALUE`, `KW_EXISTS`, `KW_CURRENT_DATE`, `KW_CURRENT_TIME`, `KW_CURRENT_TIMESTAMP`, `KW_DATE`, `KW_TIME`, `KW_TIMESTAMP`, `KW_RTRIM`, `Name`, keyword, `Star`

**Calls:** `ParseLiteral`, `ParseColumnRef`, `ParseSubquery`, `ParseValueExp` (for parenthesised), `ParseCastSpec`, `ParseCaseExp`, `ParseIIFExp`, `ParseValueCreationFuncExp`, `ParseFctSpecByName`

---

### 41. `ParseColumnRef`

**Grammar Rule (complex, multi-alternative):**
```
column_ref
    : '$'                                        → ExtractInstanceValueExp
    | '$' '->' property_path opt_optional         → ExtractPropertyValueExp
    | Name '(' args ')'                           → function call (via ParseFctSpecByName/ParseAggregateFct)
    |    with optional OVER/FILTER → window function
    | Name '.' Name '(' args ')'                  → qualified function call
    | Name '.' '$'                                → ExtractInstanceValueExp (aliased)
    | Name '.' '$' '->' property_path opt_optional → ExtractPropertyValueExp (aliased)
    | Name '.' Name property_path_tail             → PropertyNameExp
    |    with optional '->' extraction
    |    3-part path: schema.enum.enumerator check → EnumValueExp
    | Name opt_array_index                         → PropertyNameExp (single identifier)
    ;

property_path_tail
    : ( '[' IntNum ']' )? ( '.' Name ( '[' IntNum ']' )? )*
    ;

opt_optional
    : /* empty */
    | '?'    /* Parameter token */
    ;

opt_array_index
    : /* empty */
    | '[' IntNum ']'
    ;
```

**Special Logic:**
- Keywords used as first name: must be followed by `LParen` (function call) or error, unless it's an operator keyword (LIKE, IN, BETWEEN, MATCH, AND, OR, NOT, IS → always error)
- Aggregate functions → `ParseAggregateFct` then check for `KW_OVER`/`KW_FILTER` → window function
- Window function names → `ParseFctSpecByName` then check for `KW_OVER`/`KW_FILTER`
- Regular functions → `ParseFctSpecByName` then check for `KW_OVER`/`KW_FILTER`
- `Name.Star` → wildcard property path
- `Name.Dollar` → instance access path
- 3-part path → enumeration check
- Arrow `->` → extraction path

**Tokens Expected/Matched:** `Dollar`, `Name`, keyword, `LParen`, `RParen`, `Dot`, `Star`, `Arrow`, `LBracket`, `IntNum`, `RBracket`, `Parameter` (for optional ?), `KW_OVER`, `KW_FILTER`

**Calls:** `ParsePropertyPathInline`, `ParseFctSpecByName`, `ParseAggregateFct`, `ParseWindowFunction`, `ParseValueExp` (for qualified func args), `IsAggregateFunction`, `IsWindowFunctionName`

---

### 42. `ParseExpressionPath`

Delegates to `ParseColumnRef(exp, forceIntoPropertyNameExp)`.

---

### 43. `ParseColumnRefAsPropertyNameExp`

Calls `ParseColumnRef(val, true)` and casts result to `PropertyNameExp`.

---

### 44. `ParsePropertyPathInline`

**Grammar Rule:**
```
property_path
    : path_element ( '.' path_element )*
    ;

path_element
    : (Name | keyword) opt_array_index
    ;

opt_array_index
    : /* empty */
    | '[' IntNum ']'
    ;
```

**Tokens:** `Name`/keyword, `Dot`, `LBracket`, `IntNum`, `RBracket`

**Calls:** None

---

### 45. `ParseLiteral`

**Grammar Rule:**
```
literal
    : IntNum        → Long
    | ApproxNum     → Double
    | String        → String (quotes stripped)
    | NULL          → Null
    | TRUE          → Boolean
    | FALSE         → Boolean
    ;
```

**Tokens:** `IntNum`, `ApproxNum`, `String`, `KW_NULL`, `KW_TRUE`, `KW_FALSE`

**Calls:** None

---

### 46. `ParseParameter`

**Grammar Rule:**
```
parameter
    : '?'           → ParameterExp(null)
    | NamedParam    → ParameterExp(name)
    ;
```

**Tokens:** `Parameter`, `NamedParam`

**Calls:** None

---

### 47. `ParseFctSpecByName`

**Grammar Rule:**
```
function_call
    : '(' opt_set_quantifier opt_arg_list ')'
    ;

opt_set_quantifier
    : /* empty */
    | ALL
    | DISTINCT
    ;

opt_arg_list
    : /* empty */
    | value_exp ( ',' value_exp )*
    ;
```

**Tokens:** `LParen`, `KW_ALL`/`KW_DISTINCT`, `RParen`, `Comma`

**Calls:** `ParseValueExp`

---

### 48. `ParseFctSpec`

Reads function name from current token (uppercased for non-Name tokens). If aggregate, delegates to `ParseAggregateFct`; otherwise `ParseFctSpecByName`.

**Calls:** `ParseAggregateFct`, `ParseFctSpecByName`, `IsAggregateFunction`

---

### 49. `ParseAggregateFct`

Delegates to `ParseSetFct(exp, functionName, true)`.

---

### 50. `ParseSetFct`

**Grammar Rule:**
```
set_function
    : '(' opt_set_quantifier set_function_arg ')'
    ;

set_function_arg
    : '*'                            (COUNT only)
    | value_exp opt_group_concat_sep
    ;

opt_group_concat_sep
    : /* empty */
    | ',' value_exp                  (GROUP_CONCAT only)
    ;
```

**Special rules:**
- MAX()/MIN() with 0 args → syntax error
- COUNT(*) → special asterisk literal
- GROUP_CONCAT with optional second comma-separated arg
- MAX/MIN with multiple args → helpful error suggesting GREATEST/LEAST

**Tokens:** `LParen`, `KW_ALL`/`KW_DISTINCT`, `Star` (COUNT), `Comma`, `RParen`

**Calls:** `ParseValueExp`, `LiteralValueExp::Create`

---

### 51. `ParseCastSpec`

**Grammar Rule:**
```
cast_spec
    : CAST '(' value_exp AS cast_type ')'
    ;

cast_type
    : type_keyword opt_array_marker
    | Name opt_dot_name opt_array_marker
    ;

type_keyword
    : BINARY | BLOB | BOOLEAN | DATE | DOUBLE | FLOAT | INT | INT64
    | LONG | REAL | STRING | TIME | TIMESTAMP | VARCHAR
    ;

opt_dot_name
    : /* empty */
    | '.' (Name | keyword)
    ;

opt_array_marker
    : /* empty */
    | '[' ']'
    ;
```

**Tokens:** `KW_CAST`, `LParen`, `KW_AS`, type keywords, `Name`, `Dot`, `LBracket`, `RBracket`, `RParen`

**Calls:** `ParseValueExp`

---

### 52. `ParseCaseExp`

**Grammar Rule:**
```
case_exp
    : CASE when_clause_list opt_else END
    ;

when_clause_list
    : ( WHEN search_condition THEN value_exp )+
    ;

opt_else
    : /* empty */
    | ELSE value_exp
    ;
```

**Tokens:** `KW_CASE`, `KW_WHEN`, `KW_THEN`, `KW_ELSE`, `KW_END`

**Calls:** `ParseSearchCondition`, `ParseValueExp`

---

### 53. `ParseIIFExp`

**Grammar Rule:**
```
iif_exp
    : IIF '(' search_condition ',' value_exp ',' value_exp ')'
    ;
```

**Tokens:** `KW_IIF`, `LParen`, `Comma`, `RParen`

**Calls:** `ParseSearchCondition`, `ParseValueExp`

---

### 54. `ParseTypePredicate`

**Grammar Rule:**
```
type_predicate
    : '(' type_list ')'
    ;

type_list
    : opt_polymorphic_constraint table_node ( ',' opt_polymorphic_constraint table_node )*
    ;
```

**Tokens:** `LParen`, `RParen`, `Comma`

**Calls:** `ParsePolymorphicConstraint`, `ParseTableNode`

---

### 55. `ParseValueExpCommalist`

**Grammar Rule:**
```
value_exp_commalist
    : value_exp ( ',' value_exp )*
    ;
```

**Calls:** `ParseValueExp`

---

### 56. `ParseValuesOrQuerySpec`

**Grammar Rule:**
```
values_or_query_spec
    : VALUES '(' row_value_constructor_commalist ')'
    ;
```

**Tokens:** `KW_VALUES`, `LParen`, `RParen`

**Calls:** `ParseRowValueConstructorCommalist`

---

### 57. `ParseRowValueConstructorCommalist`

**Grammar Rule:**
```
row_value_constructor_commalist
    : value_exp ( ',' value_exp )*
    ;
```

**Calls:** `ParseValueExp`

---

### 58. `ParseValuesCommalist`

**Grammar Rule:**
```
values_commalist
    : VALUES values_row_list
    ;

values_row_list
    : '(' row_value_constructor_commalist ')' ( ',' '(' row_value_constructor_commalist ')' )*
    ;
```

Builds a UNION ALL chain of SingleSelectStatementExp in reverse order.

**Tokens:** `KW_VALUES`, `LParen`, `RParen`, `Comma`

**Calls:** `ParseRowValueConstructorCommalist`

---

### 59. `ParseOptColumnRefCommalist`

**Grammar Rule:**
```
opt_column_ref_commalist
    : /* empty */
    | column_ref_commalist
    ;
```

Optional; only if `LParen` is current.

**Calls:** `ParseColumnRefCommalist`

---

### 60. `ParseColumnRefCommalist`

**Grammar Rule:**
```
column_ref_commalist
    : '(' property_name ( ',' property_name )* ')'
    ;
```

**Tokens:** `LParen`, `RParen`, `Comma`

**Calls:** `ParseColumnRefAsPropertyNameExp`

---

### 61. `ParseAssignmentCommalist`

**Grammar Rule:**
```
assignment_commalist
    : assignment ( ',' assignment )*
    ;

assignment
    : property_name '=' value_exp
    ;
```

**Tokens:** `Equal`, `Comma`

**Calls:** `ParseColumnRefAsPropertyNameExp`, `ParseValueExp`

---

### 62. `ParseSearchCondition`

```
search_condition
    : search_condition_or
    ;
```

Delegates to `ParseSearchConditionOr`.

---

### 63. `ParseSearchConditionOr`

**Grammar Rule:**
```
search_condition_or
    : search_condition_and ( OR search_condition_and )*
    ;
```

**Tokens:** `KW_OR`

**Calls:** `ParseSearchConditionAnd`

---

### 64. `ParseSearchConditionAnd`

**Grammar Rule:**
```
search_condition_and
    : search_condition_not ( AND search_condition_not )*
    ;
```

**Tokens:** `KW_AND`

**Calls:** `ParseSearchConditionNot`

---

### 65. `ParseSearchConditionNot`

**Grammar Rule:**
```
search_condition_not
    : NOT predicate
    | predicate
    ;
```

**Tokens:** `KW_NOT`

**Calls:** `ParsePredicate`

---

### 66. `ParsePredicate`

**Grammar Rule (complex, with speculative parsing):**
```
predicate
    : '(' predicate_with_speculative_backtrack ')'
    | EXISTS '(' subquery ')'
    | UNIQUE '(' subquery ')'
    | value_exp predicate_tail
    ;

predicate_with_speculative_backtrack
    : /* try value_exp path first (suppressing errors) */
      value_exp predicate_tail                    → if succeeds, use this
    | /* fallback: */ '(' search_condition ')'     → parenthesised boolean
    ;

predicate_tail
    : IS opt_not (NULL | TRUE | FALSE | UNKNOWN)   → BinaryBooleanExp(Is/IsNot)
    | IS opt_not '(' type_list ')'                  → type predicate
    | opt_not BETWEEN value_exp AND value_exp        → BetweenPredicate
    | opt_not LIKE value_exp opt_escape              → LikePredicate
    | opt_not IN '(' subquery | value_list ')'       → InPredicate
    | opt_not MATCH function_call                    → MatchPredicate
    | comparison_op opt_quantifier value_exp_or_subquery → comparison
    | /* empty */                                     → UnaryPredicateExp (bare value as bool)
    ;

opt_not
    : /* empty */
    | NOT
    ;

comparison_op
    : '=' | '<' | '>' | '<=' | '>=' | '<>' | '!='
    ;

opt_quantifier
    : /* empty */
    | ALL '(' subquery ')'
    | ANY '(' subquery ')'
    | SOME '(' subquery ')'
    ;
```

**Speculative Parsing Logic:**
When `LParen` is encountered (and next is not SELECT/WITH):
1. Save lexer position and current token
2. Set `m_suppressErrors = true`
3. Try `ParseValueExp` — if succeeds, handle comparison/IS/BETWEEN/LIKE/IN/MATCH
4. If fails, restore position, parse as `(search_condition)`

**Tokens:** `LParen`, `KW_EXISTS`, `KW_UNIQUE`, `KW_IS`, `KW_NOT`, `KW_BETWEEN`, `KW_LIKE`, `KW_IN`, `KW_MATCH`, comparison tokens, `KW_ALL`/`KW_ANY`/`KW_SOME`

**Calls:** `ParseValueExp`, `ParseSearchCondition`, `ParseSubquery`, `ParseLiteral`, `ParseTypePredicate`, `ParseBetweenPredicate`, `ParseLikePredicate`, `ParseInPredicate`, `ParseMatchPredicate`, `ParseComparisonOp`, `IsComparisonOp`

---

### 67. `IsComparisonOp`

Returns true for: `Equal`, `Less`, `Great`, `LessEq`, `GreatEq`, `NotEqual`

---

### 68. `ParseComparisonOp`

Maps token to `BooleanSqlOperator`: EqualTo, LessThan, GreaterThan, LessThanOrEqualTo, GreaterThanOrEqualTo, NotEqualTo

---

### 69. `ParseInPredicate`

**Grammar Rule:**
```
in_predicate
    : IN '(' subquery ')'           → subquery form
    | IN '(' value_exp_commalist ')' → value list form
    ;
```

Uses `Peek1()` to distinguish subquery (SELECT/WITH/VALUES) from value list.

**Tokens:** `KW_IN`, `LParen`, `RParen`

**Calls:** `ParseSubquery`, `ParseValueExpCommalist`

---

### 70. `ParseLikePredicate`

**Grammar Rule:**
```
like_predicate
    : LIKE value_exp opt_escape
    ;

opt_escape
    : /* empty */
    | ESCAPE String
    ;
```

**Tokens:** `KW_LIKE`, `KW_ESCAPE`, `String`

**Calls:** `ParseValueExp`

---

### 71. `ParseBetweenPredicate`

**Grammar Rule:**
```
between_predicate
    : BETWEEN value_exp AND value_exp
    ;
```

**Tokens:** `KW_BETWEEN`, `KW_AND`

**Calls:** `ParseValueExp`

---

### 72. `ParseNullPredicate`

Dead code — always asserts false. IS NULL handled inline in `ParsePredicate`.

---

### 73. `ParseMatchPredicate`

**Grammar Rule:**
```
match_predicate
    : MATCH function_call
    ;
```

**Tokens:** `KW_MATCH`

**Calls:** `ParseFctSpec`

---

### 74. `ParseWindowFunction`

**Grammar Rule:**
```
window_function
    : opt_filter_clause OVER window_name_or_spec
    ;

window_name_or_spec
    : '(' window_specification ')'
    | Name | keyword                  → window name reference
    ;
```

**Tokens:** `KW_FILTER`, `KW_OVER`, `LParen`, `RParen`, `Name`/keyword

**Calls:** `ParseFilterClause`, `ParseWindowSpecification`

---

### 75. `ParseWindowSpecification`

**Grammar Rule:**
```
window_specification
    : opt_existing_window_name opt_partition_clause opt_order_by_clause opt_frame_clause
    ;

opt_existing_window_name
    : /* empty */
    | Name   /* only if followed by PARTITION/ORDER/ROWS/RANGE/GROUPS/RParen */
    ;
```

**Detection logic:** If current is `Name` and *not* PARTITION/ORDER/ROWS/RANGE/GROUPS, then peek ahead — if next is one of those or `)`, treat current as existing window name.

**Calls:** `ParseWindowPartitionClause`, `ParseOrderByClause`, `ParseWindowFrameClause`

---

### 76. `ParseWindowPartitionClause`

**Grammar Rule:**
```
opt_partition_clause
    : /* empty */
    | PARTITION BY partition_column_list
    ;

partition_column_list
    : partition_column ( ',' partition_column )*
    ;

partition_column
    : column_ref opt_collate
    ;

opt_collate
    : /* empty */
    | COLLATE (BINARY | NOCASE | RTRIM)
    ;
```

**Tokens:** `KW_PARTITION`, `KW_BY`, `KW_COLLATE`, `KW_BINARY`, `KW_NOCASE`, `KW_RTRIM`, `Comma`

**Calls:** `ParseColumnRef`

---

### 77. `ParseWindowFrameClause`

**Grammar Rule:**
```
opt_frame_clause
    : /* empty */
    | frame_unit frame_body opt_exclude
    ;

frame_unit
    : ROWS | RANGE | GROUPS
    ;

frame_body
    : BETWEEN first_bound AND second_bound
    | frame_start
    ;

opt_exclude
    : /* empty */
    | EXCLUDE GROUP
    | EXCLUDE TIES
    | EXCLUDE CURRENT ROW
    | EXCLUDE NO OTHERS
    ;
```

**Tokens:** `KW_ROWS`/`KW_RANGE`/`KW_GROUPS`, `KW_BETWEEN`, `KW_EXCLUDE`, `KW_GROUP`, `KW_TIES`, `KW_CURRENT`, `KW_ROW`, `KW_NO`, `KW_OTHERS`

**Calls:** `ParseWindowFrameBetween`, `ParseWindowFrameStart`

---

### 78. `ParseWindowFrameStart`

**Grammar Rule:**
```
frame_start
    : UNBOUNDED PRECEDING
    | CURRENT ROW
    | value_exp PRECEDING
    ;
```

**Tokens:** `KW_UNBOUNDED`, `KW_PRECEDING`, `KW_CURRENT`, `KW_ROW`

**Calls:** `ParseValueExp`

---

### 79. `ParseWindowFrameBetween`

**Grammar Rule:**
```
frame_between
    : BETWEEN first_bound AND second_bound
    ;
```

**Tokens:** `KW_BETWEEN`, `KW_AND`

**Calls:** `ParseFirstWindowFrameBound`, `ParseSecondWindowFrameBound`

---

### 80. `ParseFirstWindowFrameBound`

**Grammar Rule:**
```
first_bound
    : UNBOUNDED PRECEDING
    | CURRENT ROW
    | value_exp PRECEDING
    | value_exp FOLLOWING
    ;
```

**Tokens:** `KW_UNBOUNDED`, `KW_PRECEDING`, `KW_CURRENT`, `KW_ROW`, `KW_FOLLOWING`

**Calls:** `ParseValueExp`

---

### 81. `ParseSecondWindowFrameBound`

**Grammar Rule:**
```
second_bound
    : UNBOUNDED FOLLOWING
    | CURRENT ROW
    | value_exp PRECEDING
    | value_exp FOLLOWING
    ;
```

**Tokens:** `KW_UNBOUNDED`, `KW_FOLLOWING`, `KW_CURRENT`, `KW_ROW`, `KW_PRECEDING`

**Calls:** `ParseValueExp`

---

### 82. `ParseFilterClause`

**Grammar Rule:**
```
opt_filter_clause
    : /* empty */
    | FILTER '(' where_clause ')'
    ;
```

**Tokens:** `KW_FILTER`, `LParen`, `RParen`

**Calls:** `ParseWhereClause`

---

### 83. `ParseValueCreationFuncExp`

**Grammar Rule:**
```
value_creation_func
    : NAVIGATION_VALUE nav_value_creation_func
    ;
```

Consumes `KW_NAVIGATION_VALUE`, delegates to `ParseNavValueCreationFuncExp`.

---

### 84. `ParseNavValueCreationFuncExp`

**Grammar Rule:**
```
nav_value_creation_func
    : '(' derived_column ',' value_exp opt_rel_class_id ')'
    ;

opt_rel_class_id
    : /* empty */
    | ',' value_exp
    ;
```

**Semantic constraints:**
- First arg must be a 3-part property path (schema.class.property)
- Class must resolve via `TryResolveClass`
- Property must exist in class map

**Tokens:** `LParen`, `Comma`, `RParen`

**Calls:** `ParseDerivedColumn`, `ParseValueExp`, `m_context->TryResolveClass`

---

### 85. `ParsePragmaValue` (standalone helper)

**Grammar Rule:**
```
pragma_value
    : TRUE | FALSE | NULL | IntNum | ApproxNum | String | Name | keyword
    ;
```

Same as the lambda in ParsePragmaStatement but as a standalone method.

---

### 86. `ParseALLorONLY` (standalone helper)

**Grammar Rule:**
```
all_or_only
    : /* empty */ → NotSpecified
    | ALL
    | ONLY
    ;
```

**Tokens:** `KW_ALL`, `KW_ONLY`

---

### 87. `IsAggregateFunction` (helper predicate)

Returns true for: COUNT, SUM, AVG, MIN, MAX, GROUP_CONCAT, TOTAL, ANY, EVERY, SOME

---

### 88. `IsWindowFunctionName` (helper predicate)

Returns true for: ROW_NUMBER, RANK, DENSE_RANK, PERCENT_RANK, CUME_DIST, NTILE, LEAD, LAG, FIRST_VALUE, LAST_VALUE, NTH_VALUE

---

### 89. `IsJoinKeyword` (helper predicate)

Returns true for: JOIN, INNER, LEFT, RIGHT, FULL, CROSS, NATURAL

---

## Operator Precedence Summary (value expressions, lowest → highest)

| Level | Operators | Method |
|---|---|---|
| 1 | `\|` (bitwise OR) | `ParseValueExpOr` |
| 2 | `&` (bitwise AND) | `ParseValueExpAnd` |
| 3 | `<<` `>>` (shifts) | `ParseValueExpBitOr` |
| 4 | `+` `-` (add/sub) | `ParseValueExpBitAnd` |
| 5 | `*` `/` `%` (mul/div/mod) | `ParseValueExpShift` |
| 6 | `\|\|` (concat) | `ParseValueExpAddSub` |
| 7 | `-` `+` `~` (unary) | `ParseValueExpUnary` |
| 8 | Primary (literals, funcs, etc.) | `ParseValueExpPrimary` |

## Boolean Operator Precedence (lowest → highest)

| Level | Operator | Method |
|---|---|---|
| 1 | `OR` | `ParseSearchConditionOr` |
| 2 | `AND` | `ParseSearchConditionAnd` |
| 3 | `NOT` | `ParseSearchConditionNot` |
| 4 | Predicates (IS, BETWEEN, LIKE, IN, MATCH, comparisons) | `ParsePredicate` |

---

## Call Graph Summary

```
Parse
├── ParseCTE
│   ├── ParseCTEBlock
│   │   └── ParseSelectStatement (recursive)
│   └── ParseSelectStatement
├── ParseSelectStatement
│   ├── ParseRowValueConstructorCommalist → ParseValueExp
│   ├── ParseSingleSelectStatement
│   │   ├── ParseSelection
│   │   │   └── ParseDerivedColumn → ParseValueExp
│   │   ├── ParseFromClause
│   │   │   └── ParseTableRefAndOptJoins
│   │   │       ├── ParseTableRef
│   │   │       │   ├── ParsePolymorphicConstraint
│   │   │       │   ├── ParseSubquery → ParseCTE | ParseSelectStatement
│   │   │       │   ├── ParseTableValuedFunction → ParseValueExp
│   │   │       │   └── ParseTableNode / view expansion
│   │   │       └── ParseJoinedTable
│   │   │           ├── ParseJoinType
│   │   │           ├── ParseTableRef
│   │   │           └── ParseJoinSpec → ParseSearchCondition
│   │   ├── ParseWhereClause → ParseSearchCondition
│   │   ├── ParseGroupByClause → ParseValueExp
│   │   ├── ParseHavingClause → ParseSearchCondition
│   │   ├── ParseWindowClause → ParseWindowSpecification
│   │   ├── ParseOrderByClause → ParseValueExp, ParseComparisonOp, ParseLiteral
│   │   ├── ParseLimitOffsetClause → ParseValueExp
│   │   └── ParseOptECSqlOptionsClause → ParseLiteral
│   └── ParseSelectStatement (recursive for UNION/INTERSECT/EXCEPT)
├── ParseInsertStatement
│   ├── ParseTableNode
│   ├── ParseOptColumnRefCommalist → ParseColumnRefCommalist → ParseColumnRefAsPropertyNameExp
│   └── ParseValuesOrQuerySpec → ParseRowValueConstructorCommalist → ParseValueExp
├── ParseUpdateStatementSearched
│   ├── ParseTableNode
│   ├── ParseAssignmentCommalist → ParseColumnRefAsPropertyNameExp, ParseValueExp
│   ├── ParseWhereClause
│   └── ParseOptECSqlOptionsClause
├── ParseDeleteStatementSearched
│   ├── ParseTableNode
│   ├── ParseWhereClause
│   └── ParseOptECSqlOptionsClause
└── ParsePragmaStatement
    └── ParseOptECSqlOptionsClause

ParseSearchCondition
└── ParseSearchConditionOr
    └── ParseSearchConditionAnd
        └── ParseSearchConditionNot
            └── ParsePredicate
                ├── ParseValueExp (speculative + normal)
                ├── ParseSearchCondition (parenthesised boolean fallback)
                ├── ParseSubquery (EXISTS/UNIQUE)
                ├── ParseTypePredicate → ParsePolymorphicConstraint, ParseTableNode
                ├── ParseInPredicate → ParseSubquery | ParseValueExpCommalist
                ├── ParseLikePredicate → ParseValueExp
                ├── ParseBetweenPredicate → ParseValueExp
                └── ParseMatchPredicate → ParseFctSpec

ParseValueExp
└── ParseValueExpOr (|)
    └── ParseValueExpAnd (&)
        └── ParseValueExpBitOr (<<, >>)
            └── ParseValueExpBitAnd (+, -)
                └── ParseValueExpShift (*, /, %)
                    └── ParseValueExpAddSub (||)
                        └── ParseValueExpMulDiv
                            └── ParseValueExpConcat
                                └── ParseValueExpUnary (-, +, ~)
                                    └── ParseValueExpPrimary
                                        ├── ParseColumnRef
                                        │   ├── ParsePropertyPathInline
                                        │   ├── ParseFctSpecByName → ParseValueExp
                                        │   ├── ParseAggregateFct → ParseSetFct → ParseValueExp
                                        │   └── ParseWindowFunction
                                        │       ├── ParseFilterClause → ParseWhereClause
                                        │       └── ParseWindowSpecification
                                        │           ├── ParseWindowPartitionClause → ParseColumnRef
                                        │           ├── ParseOrderByClause
                                        │           └── ParseWindowFrameClause
                                        │               ├── ParseWindowFrameStart → ParseValueExp
                                        │               └── ParseWindowFrameBetween
                                        │                   ├── ParseFirstWindowFrameBound → ParseValueExp
                                        │                   └── ParseSecondWindowFrameBound → ParseValueExp
                                        ├── ParseCastSpec → ParseValueExp
                                        ├── ParseCaseExp → ParseSearchCondition, ParseValueExp
                                        ├── ParseIIFExp → ParseSearchCondition, ParseValueExp
                                        ├── ParseValueCreationFuncExp → ParseNavValueCreationFuncExp
                                        │   └── ParseDerivedColumn, ParseValueExp
                                        └── ParseSubquery
```

---

## Notes for BISON Grammar Generation

1. **Method naming vs. actual precedence:** The method names are misleading due to an apparent naming mismatch. For example, `ParseValueExpBitOr` handles shifts, `ParseValueExpBitAnd` handles add/sub, etc. Use the actual operator precedence table above.

2. **Speculative parsing in `ParsePredicate`:** The parser saves/restores lexer state when encountering `(` in boolean context. In BISON, this would be handled by grammar ambiguity resolution (GLR mode or careful rule ordering).

3. **Keyword-as-identifier:** Many keywords can be used as identifiers when escaped with `[]`. The lexer strips quotes for `Name` tokens. Unescaped keywords are rejected as identifiers unless used as function names.

4. **View class expansion:** This is a semantic action (re-parsing a view query) and would remain a post-parse semantic action in a BISON grammar.

5. **`NamedParam` for schema:ClassName:** The lexer produces `NamedParam` for `schema:ClassName` patterns. The parser handles this as an alternative to `schema.ClassName`.

6. **The ECSQLOPTIONS loop termination:** Options parsing continues while `At(Name)` — stops at SQL keywords. This prevents consuming keywords like UNION, ORDER, LIMIT that belong to the outer query.

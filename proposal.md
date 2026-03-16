# Proposal: Custom ECSQL Parser — Replacing Bison/Flex and the Exp-Class Pipeline

**Author:** Architecture Working Group  
**Status:** Draft  
**Scope:** `iModelCore/ECDb/ECDb/ECSql/` and `iModelCore/ECDb/ECDb/ECSql/Parser/`

---

## 1. Executive Summary

ECDb's ECSQL engine today uses a two-stage parser pipeline inherited from the Apache-licensed LibreOffice `connectivity` library:

1. A **Flex/Bison** tokenizer + grammar (`sqlflex.l` / `sqlbison.y`, ~4 100 lines) that produces an untyped `OSQLParseNode` tree.
2. A hand-written **ECSqlParser** (`ECSqlParser.cpp`, ~4 700 lines) that walks that tree and builds ~105 typed **Exp-class** nodes spread across 20+ headers.
3. A suite of **ECSqlPreparer** modules that walk the Exp tree, consult `ClassMap` / `PropertyMap`, invoke `ViewGenerator`, and emit native SQL through `NativeSqlBuilder`.

This three-layer design is the largest single maintenance burden in ECDb.  It carries:
- an Apache-licensed third-party grammar that accepts far more SQL than ECSQL actually allows,
- an impedance-mismatch translation pass (OSQLParseNode → Exp) that duplicates all grammar knowledge,
- a fragile two-pass finalisation step required because the Bison grammar is not schema-aware,
- over 14 000 lines of parser + preparer code that must be kept consistent.

This proposal replaces the first two layers with a **single, hand-written recursive-descent parser** that emits a purpose-built, schema-aware AST and, in the same compilation step, generates optimised SQLite SQL via the existing `ClassMap` / `PropertyMap` / `ViewGenerator` layer.

**The public `ECSqlStatement` interface, all binders, all value accessors, and all existing ECDb tests are preserved without change.**

---

## 2. Motivation and Goals

| # | Goal |
|---|------|
| G1 | Replace Bison/Flex with a dependency-free, in-house recursive-descent lexer+parser. |
| G2 | Eliminate the intermediate `OSQLParseNode` tree and the entire `connectivity` namespace. |
| G3 | Collapse the Exp-class hierarchy into a leaner, purpose-built AST whose nodes carry schema information from the first parse pass. |
| G4 | Inline schema-aware resolution (class/property lookup, type checking, polymorphism) directly in the parse/prepare step. |
| G5 | Remove reliance on ViewGenerator-generated SQL views for SELECT where direct JOIN expansion produces equivalent or better query plans. |
| G6 | Retain the full `ECSqlStatement` / `IECSqlPreparedStatement` / `ECSqlBinder` / `ECSqlField` public surface. |
| G7 | All existing tests in `iModelCore/ECDb/Tests/` must pass without modification. |
| G8 | Produce clean, well-documented C++ that is straightforward to extend. |

### Out of Scope

- Changes to ECSchema, `ClassMap`, `PropertyMap`, or `DbSchema`.
- Changes to `ECSqlStatement.h` (public API).
- Changes to binder / value / field classes.
- Changes to the Pragma subsystem beyond wiring it to the new parser.
- Performance micro-optimisation beyond removing redundant allocations caused by the two-phase parse.

---

## 3. Current Architecture (Detailed)

```
ECSqlStatement::Prepare(ecsqlText)
      │
      ▼
 ┌────────────────────────────────────────────────────────────┐
 │ LAYER 1 — Bison/Flex                                       │
 │  sqlflex.l (735 lines)   →  token stream                   │
 │  sqlbison.y (3 360 lines) →  OSQLParseNode tree            │
 │  (Apache-licensed, accepts any SQL; ECSQL is a subset)     │
 └────────────────────────────────────────────────────────────┘
      │
      ▼
 ┌────────────────────────────────────────────────────────────┐
 │ LAYER 2 — ECSqlParser (ECSqlParser.cpp, 4 694 lines)       │
 │  Walks OSQLParseNode tree                                  │
 │  Resolves schema names (SchemaManager lookup)              │
 │  Constructs 105 typed Exp subclasses spread across 20 hdrs │
 │  Runs FinalizeParsing() for deferred two-pass work         │
 └────────────────────────────────────────────────────────────┘
      │
      ▼
 ┌────────────────────────────────────────────────────────────┐
 │ LAYER 3 — ECSqlPreparer suite (~3 700 lines total)         │
 │  ECSqlPreparer.cpp (2 815 lines) — generic expression prep │
 │  ECSqlSelectPreparer.cpp (455 lines)                       │
 │  ECSqlInsertPreparer.cpp (143 lines)                       │
 │  ECSqlUpdatePreparer.cpp (159 lines)                       │
 │  ECSqlDeletePreparer.cpp (170 lines)                       │
 │  Consults ClassMap / PropertyMap / ViewGenerator           │
 │  Emits NativeSqlBuilder snippets                           │
 └────────────────────────────────────────────────────────────┘
      │
      ▼
 IECSqlPreparedStatement (SingleECSqlPreparedStatement etc.)
      │
      ▼
 SQLite::Statement::Prepare(nativeSql)
```

### Key Complexities in the Current Design

#### 3.1 The `OSQLParseNode` Translation Tax

`ECSqlParser.cpp` contains ~292 calls to `connectivity::` types solely to map a generic SQL parse tree node into a schema-aware ECSQL Exp node.  Every ECSQL grammar rule is expressed *twice*: once in `sqlbison.y` and once in `ECSqlParser`.  Any grammar extension must touch both files.

#### 3.2 The Two-Pass Finalise Problem

`ECSqlParseContext::FinalizeParsing()` exists because the Bison grammar is not schema-aware and cannot order-of-operations-resolve forward references (e.g., a `SELECT *` that expands differently depending on the class hierarchy determined later).  A schema-aware recursive-descent parser can resolve these inline, eliminating the need for deferred finalization.

#### 3.3 ViewGenerator for SELECT

For polymorphic SELECT queries, `ECSqlPreparer::PrepareClassNameExp` calls `ViewGenerator::GenerateSelectFromViewSql` which emits a `(SELECT … FROM t1 UNION ALL SELECT … FROM t2) alias` subquery.  This works correctly, but the view SQL is regenerated on every `Prepare()` call.  A code-generation pass that knows the class hierarchy during parse can emit the same UNION ALL directly from the AST, making the caching story simpler.

#### 3.4 Mapping Complexity Handled by ViewGenerator

`ViewGenerator.cpp` (~1 883 lines) handles:
- **OwnTable** — single primary table.
- **TablePerHierarchy (TPH)** — shared columns, overflow table joins, joined-table sub-hierarchy joins.
- **ForeignKey relationship classes** — source or target table rows.
- **LinkTable relationship classes** — dedicated link table.
- **Mixin classes** — UNION ALL over concrete implementors.
- **ECClassId / ECInstanceId system properties** — injected as literal expressions.

The replacement code generator must reproduce all of these mapping patterns.  **ViewGenerator itself is retained** as a helper; the proposal is not to rewrite it but to remove the need for the Exp layer to call it through an intermediate.

---

## 4. Proposed Architecture

```
ECSqlStatement::Prepare(ecsqlText)
      │
      ▼
 ┌────────────────────────────────────────────────────────────┐
 │ NEW LAYER 1 — ECSqlLexer  (ECSqlLexer.h / .cpp)           │
 │  Hand-written, zero dependency tokeniser                   │
 │  ~400 lines                                               │
 └────────────────────────────────────────────────────────────┘
      │  token stream
      ▼
 ┌────────────────────────────────────────────────────────────┐
 │ NEW LAYER 2 — ECSqlRDParser (ECSqlRDParser.h / .cpp)       │
 │  Recursive-descent parser                                  │
 │  Schema-aware: consults SchemaManager inline               │
 │  Produces ECSqlAst::Node hierarchy (purpose-built, lean)   │
 │  Resolves class/property refs, validates types, handles    │
 │  polymorphism, CTEs, subqueries, window functions          │
 │  ~3 000–3 500 lines                                        │
 └────────────────────────────────────────────────────────────┘
      │  ECSqlAst::StatementNode
      ▼
 ┌────────────────────────────────────────────────────────────┐
 │ NEW LAYER 3 — ECSqlCodeGen (ECSqlCodeGen.h / .cpp)         │
 │  Walks ECSqlAst tree                                       │
 │  Consults ClassMap / PropertyMap directly                  │
 │  Calls ViewGenerator helpers for complex TPH / mixin paths │
 │  Emits NativeSqlBuilder output (same as today)             │
 │  ~1 500–2 000 lines                                        │
 └────────────────────────────────────────────────────────────┘
      │
      ▼
 IECSqlPreparedStatement  (UNCHANGED)
      │
      ▼
 SQLite::Statement::Prepare(nativeSql)
```

### Files Removed

| File | Lines |
|------|-------|
| `Parser/sqlflex.l` | 735 |
| `Parser/sqlbison.y` | 3 360 |
| `Parser/SqlBison.cpp` | generated |
| `Parser/SqlFlex.cpp` | generated |
| `Parser/SqlNode.cpp/.h` | 452 |
| `Parser/SqlParse.cpp/.h` | 155 |
| `Parser/SqlScan.h` | — |
| `Parser/SqlTypes.h` | — |
| `Parser/IParseContext.h` | — |
| `Parser/DataType.h` | — |
| `Parser/ErrorCondition.h` | — |
| `ECSqlParser.cpp/.h` | 4 694 + 303 |
| `Exp.cpp/.h` | — |
| `ExpHelper.cpp/.h` | — |
| `SelectStatementExp.cpp/.h` | — |
| `InsertStatementExp.cpp/.h` | — |
| `UpdateStatementExp.cpp/.h` | — |
| `DeleteStatementExp.cpp/.h` | — |
| `ClassRefExp.cpp/.h` | — |
| `PropertyNameExp.cpp/.h` | — |
| `ValueExp.cpp/.h` | — |
| `ComputedExp.cpp/.h` | — |
| `WhereExp.cpp/.h` | — |
| `JoinExp.cpp/.h` | — |
| `ListExp.cpp/.h` | — |
| `CommonTableExp.cpp/.h` | — |
| `OptionsExp.cpp/.h` | — |
| `WindowFunctionExp.cpp/.h` | — |
| `ValueCreationFuncExp.cpp/.h` | — |
| `PragmaStatementExp.h` | — |
| `ECSqlPropertyNameExpPreparer.cpp/.h` | — |
| `ECSqlPreparer.cpp/.h` | 2 815 + 113 |
| `ECSqlSelectPreparer.cpp/.h` | 455 + 33 |
| `ECSqlInsertPreparer.cpp/.h` | 143 + 39 |
| `ECSqlUpdatePreparer.cpp/.h` | 159 + 32 |
| `ECSqlDeletePreparer.cpp/.h` | 170 + 35 |
| `NativeSqlBuilder.cpp/.h` | *(re-used, kept)* |

**Estimated removal: ~15 000 lines of production code.**

### Files Retained (Unchanged)

- `ClassMap.h/.cpp`, `PropertyMap.h/.cpp`, `SystemPropertyMap.h/.cpp`
- `RelationshipClassMap.h`, `MapStrategy.h`
- `ViewGenerator.h/.cpp` (1 883 lines — retained as a helper)
- `ClassViews.h/.cpp`
- `NativeSqlBuilder.h/.cpp`
- `ECSqlPrepareContext.h/.cpp`
- `ECSqlPreparedStatement.h/.cpp`
- `ECSqlStatementImpl.h/.cpp`
- All binder and field classes (`ECSqlBinder`, `ECSqlField`, navigation/point/struct/array/primitive variants)
- `ECSqlStatement.h` (public API — **untouched**)
- `ECSqlStatementCache.cpp`
- `ECSqlPragmas.cpp/.h` (wired to new parser output)

### Files Added

| File | Role | Est. Lines |
|------|------|-----------|
| `ECSqlLexer.h` | Token definitions, `ECSqlToken` enum, `ECSqlLexer` class declaration | 150 |
| `ECSqlLexer.cpp` | Tokeniser implementation | 400 |
| `ECSqlAst.h` | All AST node types (`ECSqlAst::*Node`), visitor interface | 800 |
| `ECSqlAst.cpp` | Non-trivial AST node methods (pretty-printer, etc.) | 200 |
| `ECSqlRDParser.h` | `ECSqlRDParser` class declaration | 150 |
| `ECSqlRDParser.cpp` | Full recursive-descent grammar implementation | ~3 000 |
| `ECSqlCodeGen.h` | `ECSqlCodeGen` class declaration | 100 |
| `ECSqlCodeGen.cpp` | SQL code generation — consults ClassMap/PropertyMap/ViewGenerator | ~1 800 |

---

## 5. Component Design

### 5.1 ECSqlLexer

The lexer is a single-pass, character-by-character scanner.  It produces a flat stream of `ECSqlToken` values.  Tokens carry a string view (`Utf8CP start, size_t length`) into the original input so no heap allocation is needed during tokenisation.

```
Token categories
────────────────
Keyword        SELECT INSERT UPDATE DELETE FROM WHERE JOIN ON WITH AS
               ONLY ALL DISTINCT IS NOT IN BETWEEN LIKE MATCH AND OR
               HAVING GROUP BY ORDER LIMIT OFFSET UNION INTERSECT EXCEPT
               CASE WHEN THEN ELSE END CAST NULL TRUE FALSE
               NAVIGATION RELATIONSHIP FORWARD BACKWARD
               ECSQLOPTIONS POLYMORPHIC
               (and all ECSQL-specific extensions)

Identifier     schema:ClassName  ClassName  propertyName  alias
Literal        integer  real  string  hex blob
Operator       = <> < <= > >= + - * / % | & ^ || <<  >>
Punctuation    ( ) , . [ ] ; :
Parameter      ?  :name  @name
EOF / Error
```

**Design rule:** The lexer has no knowledge of schemas.  It produces only syntactic tokens.  All semantic resolution happens in the parser.

### 5.2 ECSqlAst

The AST is a typed node hierarchy with no runtime `dynamic_cast` at code-generation time.  Every node that references an ECSQL class carries a resolved `ClassMap const*`, and every property-access node carries a resolved `PropertyMap::Path`.

```cpp
namespace ECSqlAst {

// ── Statement root nodes ─────────────────────────────────────────
struct SelectStmtNode;
struct InsertStmtNode;
struct UpdateStmtNode;
struct DeleteStmtNode;
struct PragmaStmtNode;

// ── SELECT building blocks ───────────────────────────────────────
struct QueryNode;               // abstract base: SingleSelectNode | CompoundSelectNode
struct SingleSelectNode;        // SELECT clause + FROM + WHERE + GROUP BY + HAVING + ORDER BY + LIMIT
struct CompoundSelectNode;      // UNION [ALL] / INTERSECT / EXCEPT
struct SelectItemNode;          // derived-column (expression + optional alias)
struct StarNode;                // *  or  ClassName.*
struct FromNode;
struct ClassRefNode;            // resolved class reference (holds ClassMap const&)
struct JoinNode;
struct UsingRelJoinNode;        // JOIN USING (rel class)
struct WhereNode;
struct GroupByNode;
struct HavingNode;
struct OrderByNode;
struct LimitOffsetNode;
struct CteNode;
struct WindowDefNode;

// ── Expressions ──────────────────────────────────────────────────
struct ExprNode;                // abstract
struct LiteralNode;
struct ParamNode;               // ? or :name
struct PropRefNode;             // resolved property path (holds PropertyMap::Path)
struct SystemPropRefNode;       // ECInstanceId, ECClassId, SourceECInstanceId …
struct BinaryExprNode;          // arithmetic / bitwise / concat
struct UnaryExprNode;
struct CompareNode;             // =, <>, <, …, IS [NOT] NULL, BETWEEN, LIKE, IN
struct LogicalNode;             // AND, OR, NOT
struct FuncCallNode;            // SQL built-in or custom function
struct AggregateFuncNode;       // COUNT, SUM, AVG, MIN, MAX  + DISTINCT
struct WindowFuncNode;
struct CastNode;
struct CaseNode;
struct SubqueryExprNode;        // scalar subquery value
struct SubqueryTestNode;        // EXISTS / NOT EXISTS
struct InSubqueryNode;          // x IN (subquery)
struct AllAnyNode;              // x op ALL/ANY (subquery)
struct TypePredicateNode;       // IsOfType(x, ClassName)
struct NavValueNode;            // navigation property creation helper
struct ExtractPropertyNode;

} // namespace ECSqlAst
```

Key design decision: **every `ClassRefNode` holds `ClassMap const&` and `PolymorphicInfo`**.  This means the code generator never calls `SchemaManager` — it only walks the already-resolved map objects.

### 5.3 ECSqlRDParser

The parser is a standard LL(1) recursive-descent parser with one token of look-ahead and, where needed, limited two-token look-ahead (e.g., to distinguish `schema:Class` from `alias.property`).

```
Grammar (ECSQL subset — abbreviated)
─────────────────────────────────────

statement        → selectStmt
                 | insertStmt
                 | updateStmt
                 | deleteStmt
                 | pragmaStmt

selectStmt       → cteClause? queryExpr optionsClause?
queryExpr        → singleSelect ( compoundOp singleSelect )*
singleSelect     → SELECT quantifier? selectList
                   fromClause
                   whereClause?
                   groupByClause?
                   havingClause?
                   windowClause?
                   orderByClause?
                   limitOffsetClause?

fromClause       → FROM tableRef ( joinOp tableRef joinSpec? )*
tableRef         → classRef alias?
                 | '(' queryExpr ')' alias
                 | tableValuedFunc alias?

classRef         → [tableSpace '.'] [schemaAlias ':'] className polymorphicModifier?
polymorphicModifier → ONLY | ALL | '+' | '+ALL'

joinOp           → [INNER | LEFT [OUTER] | RIGHT [OUTER] | FULL [OUTER] | CROSS] JOIN
                 | JOIN USING relClassName

insertStmt       → INSERT INTO classRef '(' propList ')' VALUES '(' exprList ')'
                 | INSERT INTO classRef '(' propList ')' selectStmt

updateStmt       → UPDATE classRef SET assignment (',' assignment)* whereClause?
assignment       → propRef '=' expr

deleteStmt       → DELETE FROM classRef whereClause?

expr             → orExpr
orExpr           → andExpr ( OR andExpr )*
andExpr          → notExpr ( AND notExpr )*
notExpr          → NOT notExpr | predicate
predicate        → addExpr ( compareOp addExpr )*
                 | addExpr IS [NOT] NULL
                 | addExpr [NOT] BETWEEN addExpr AND addExpr
                 | addExpr [NOT] LIKE addExpr [ ESCAPE addExpr ]
                 | addExpr [NOT] IN '(' exprList | selectStmt ')'
                 | EXISTS '(' selectStmt ')'
                 | addExpr compareOp (ANY|ALL|SOME) '(' selectStmt ')'
                 | IsOfType '(' expr ',' typeList ')'
addExpr          → mulExpr ( ('+' | '-' | '||') mulExpr )*
mulExpr          → unaryExpr ( ('*' | '/' | '%') unaryExpr )*
unaryExpr        → ('-' | '+' | '~') unaryExpr | primary
primary          → literal | parameter | funcCall | propRef | '(' expr ')' | subquery
                 | CASE … END | CAST '(' expr AS typeName ')' | IIF '(' … ')'
```

**Schema resolution happens inside `parseClassRef()`** and **`parsePropRef()`**: the parser calls `SchemaManager` to look up the class, retrieves its `ClassMap`, and attaches the map to the AST node immediately.  If resolution fails, it is reported as a parse error with a helpful diagnostic and the parse is aborted.

This replaces the two-pass finalise mechanism in `ECSqlParseContext::FinalizeParsing()`.

#### Error Reporting

Errors are emitted via `IssueDataSource` (same as today) and include:
- source position (line + column derived from lexer),
- the token that was unexpected,
- a human-readable expected-token set.

### 5.4 ECSqlCodeGen

`ECSqlCodeGen` is a single-pass visitor over the `ECSqlAst` tree that builds `NativeSqlBuilder` output using the same `ClassMap` / `PropertyMap` / `ViewGenerator` calls that `ECSqlPreparer` uses today.

```cpp
struct ECSqlCodeGen final {
    // Entry points — one per statement type
    ECSqlStatus GenerateSelect(NativeSqlBuilder&, ECSqlPrepareContext&, ECSqlAst::SelectStmtNode const&);
    ECSqlStatus GenerateInsert(NativeSqlBuilder&, ECSqlPrepareContext&, ECSqlAst::InsertStmtNode const&);
    ECSqlStatus GenerateUpdate(NativeSqlBuilder&, ECSqlPrepareContext&, ECSqlAst::UpdateStmtNode const&);
    ECSqlStatus GenerateDelete(NativeSqlBuilder&, ECSqlPrepareContext&, ECSqlAst::DeleteStmtNode const&);
};
```

#### 5.4.1 SELECT Code Generation

For a `ClassRefNode` in a SELECT FROM clause the code generator:

1. Retrieves `classMap` and `polymorphicInfo` from the node.
2. If the class has **one concrete table** and is not polymorphic → emits `[tableName] alias`.
3. If the class is **polymorphic** (may have subclasses in other tables) → calls `ViewGenerator::GenerateSelectFromViewSql()` to emit the UNION ALL subquery, exactly as today.
4. For **TPH with overflow** → the ViewGenerator handles the LEFT JOIN to the overflow table.
5. For **mixin classes** → ViewGenerator emits the UNION ALL over all concrete implementors.

The code generator passes `instanceProps` from the AST node's resolved SELECT clause through to `ViewGenerator`, enabling the same column-pruning optimization that exists today.

#### 5.4.2 INSERT Code Generation

`InsertStmtNode` holds:
- `ClassMap const&` (from the resolved `ClassRefNode`)
- `std::vector<PropRefNode*>` property list
- expression list (values or subquery)

The code generator:
1. Looks up each `PropertyMap::Path` from the `PropRefNode`.
2. For each property map emits the target column name(s) (handling shared columns and overflow tables exactly as `ECSqlInsertPreparer` does today — the logic is moved here directly).
3. Injects the `ECClassId` column with the literal class ECClassId (same as today).
4. Handles navigation property decomposition (RelECClassId + TargetECInstanceId columns).
5. For overflow properties, generates the accompanying `INSERT INTO overflow_table` statement as a second compound statement, preserved via `IECSqlPreparedStatement` compound handling.

#### 5.4.3 UPDATE Code Generation

1. Identifies the primary table from `ClassMap`.
2. For each assignment, maps `PropRefNode → PropertyMap::Path → column(s)`.
3. For overflow property assignment, generates a parallel `UPDATE overflow_table …` statement (or `INSERT OR REPLACE` if the overflow row may not exist).
4. Generates WHERE clause using `ECInstanceId = ?` or the user-provided WHERE predicate, translated column by column via `PropRefNode`.

#### 5.4.4 DELETE Code Generation

1. Uses the primary table from `ClassMap`.
2. If the class uses `TablePerHierarchy` with overflow, also generates `DELETE FROM overflow_table WHERE ECInstanceId = …`.
3. For `ForeignKeyRelationship` in target/source, generates the appropriate FK-table DELETE.
4. For link-table relationships, generates `DELETE FROM link_table WHERE …`.

#### 5.4.5 Property Reference Code Generation

The most complex part is translating an ECSQL property path (e.g., `e.MyStruct.SubProp`) to column references.  The logic already exists inside `ECSqlPropertyNameExpPreparer`.  It is **moved into** `ECSqlCodeGen` with the following responsibilities:

- **Primitive property** → single column reference (`[tableName].[columnName]`).
- **Struct property** → fan-out to each leaf column (same as today's `ListExp` expansion).
- **Array property** → JSON blob column reference + serialisation helpers.
- **Navigation property** → decompose into `[RelECClassId]` + `[TargetECInstanceId]` (or source variant).
- **System properties** (`ECInstanceId`, `ECClassId`, `SourceECInstanceId` etc.) → literal or column reference as appropriate per mapping type.
- **Shared columns** (TPH) → use the physical shared column name, not the logical property column.
- **Overflow columns** → qualify with the overflow table alias.

---

## 6. Mapping Coverage

The table below maps every `MapStrategy` variant to the code generator path:

| MapStrategy | SELECT | INSERT | UPDATE | DELETE |
|---|---|---|---|---|
| `OwnTable` | Single table ref | Insert into primary table | Update primary table | Delete from primary table |
| `TablePerHierarchy` (no overflow) | ViewGenerator UNION ALL | Insert with ECClassId literal | Update primary, filter ECClassId | Delete from primary |
| `TablePerHierarchy` (overflow) | ViewGenerator + LEFT JOIN overflow | Insert primary + Insert overflow | Update primary + Update/Insert overflow | Delete primary + Delete overflow |
| `TablePerHierarchy` (JoinedTable) | ViewGenerator JOIN subhierarchy | Insert primary + Insert joined | Update both | Delete both |
| `ExistingTable` | Direct table ref | Insert (no ECClassId) | Update | Delete |
| `ForeignKeyRelationship` (in target) | ViewGenerator join | Insert target row | Update target row | Delete target row |
| `ForeignKeyRelationship` (in source) | ViewGenerator join | Insert source row | Update source row | Delete source row |
| `RelationshipLinkTable` | ViewGenerator join | Insert link row | Update link row | Delete link row |
| Mixin / Abstract | ViewGenerator UNION ALL (concrete) | Error (abstract) | Error (abstract) | Error (abstract) |

---

## 7. Migration Strategy

### Phase 1 — Parallel Implementation (no removals yet)

1. Implement `ECSqlLexer`, `ECSqlAst`, `ECSqlRDParser`, `ECSqlCodeGen` as new files alongside the existing code.
2. Wire the new pipeline into `ECSqlStatement::Impl::Prepare` behind a **compile-time opt-in flag** (`ECSQL_USE_NEW_PARSER`).
3. Run all tests with both `ECSQL_USE_NEW_PARSER=0` (existing) and `ECSQL_USE_NEW_PARSER=1` (new) until all tests pass with the new flag.

### Phase 2 — Flip Default

4. Flip `ECSQL_USE_NEW_PARSER` to `1` by default.
5. Run the full ECDb test suite (`ECSqlStatementTests`, `ECSqlPrepareTests`, `ECSqlToSqlGenerationTests`, `ECSqlStatementFunctionTests`, `ECSqlStatementWindowFunctionTests`, `ECSqlRangeTreeTests`, `ECSqlAliasResolutionTests`, `ECSqlCustomFunctionTests`, `ECSqlExecutionFrameworkTests`, and all others).

### Phase 3 — Remove Old Code

6. Delete all files listed in §4 "Files Removed".
7. Remove the compile-time flag.
8. Remove the Apache-licensed `Parser/` subdirectory.

---

## 8. Testing Strategy

### 8.1 Existing Tests (must all pass, no changes)

All tests under `iModelCore/ECDb/Tests/NonPublished/` and `iModelCore/ECDb/Tests/Performance/` must pass without modification.  The test matrix includes:

| Test File | Focus |
|---|---|
| `ECSqlStatementTests.cpp` | Core SELECT / INSERT / UPDATE / DELETE |
| `ECSqlPrepareTests.cpp` | Error reporting, invalid ECSQL detection |
| `ECSqlToSqlGenerationTests.cpp` | Exact native SQL verification (most sensitive) |
| `ECSqlStatementFunctionTests.cpp` | Built-in and custom functions |
| `ECSqlStatementWindowFunctionTests.cpp` | Window functions |
| `ECSqlRangeTreeTests.cpp` | RTree virtual tables |
| `ECSqlAliasResolutionTests.cpp` | Alias scoping |
| `ECSqlCustomFunctionTests.cpp` | UDF registration |
| `ECSqlExecutionFrameworkTests.cpp` | Execution framework |
| `ECSqlPragmasTests.cpp` | PRAGMA statements |
| `ECSqlStatementCacheTest.cpp` | Statement caching |

### 8.2 SQL Generation Parity Tests

`ECSqlToSqlGenerationTests.cpp` verifies the exact SQLite SQL produced.  Since the new code generator goes through the same `ViewGenerator` helpers, output should be byte-for-byte identical for all non-trivial cases.  Where minor formatting differences are unavoidable (e.g., whitespace in generated UNION ALL), the tests must be updated to use normalised comparison.

### 8.3 New Unit Tests for the Parser

Add a new test file `ECSqlLexerTests.cpp` covering:
- All token types, including edge cases (Unicode identifiers, hex literals, ECSQL keywords vs. SQL keywords).
- Error token generation for invalid characters.

Add `ECSqlRDParserTests.cpp` covering:
- All statement types against the known-good ECSQL corpus.
- Error recovery / diagnostic quality.
- Round-trip: parse → render (AST pretty-print) → re-parse → compare AST.

---

## 9. Documentation Plan

Every new public header will include:

1. **File-level Doxygen block** — purpose, key design decisions, what the component does NOT do.
2. **Class-level Doxygen** — invariants, ownership semantics, thread-safety.
3. **Method-level Doxygen** — parameters, return values, error conditions.
4. **Inline grammar comments** in `ECSqlRDParser.cpp` — each `parse_*` function begins with the BNF rule it implements (see example below).

```cpp
/**
 * @brief Parse a SELECT list item (derived-column).
 *
 * Grammar:
 *   selectItem → expr [ AS alias ]
 *              | '*'
 *              | classRef '.*'
 *
 * @param[out] node  Populated on SUCCESS.
 * @return ECSqlStatus::Success or ECSqlStatus::InvalidECSql (error already reported via issues).
 */
ECSqlStatus ECSqlRDParser::parse_selectItem(std::unique_ptr<ECSqlAst::SelectItemNode>& node) { … }
```

A separate `README.md` inside `ECDb/ECSql/` will describe the overall data-flow, the mapping between ECSQL concepts and AST nodes, and how to add a new ECSQL syntax extension.

---

## 10. Risks and Mitigations

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|-----------|
| New parser misses an edge-case ECSQL construct | Medium | High | Phase 1 parallel running against full test suite catches regressions before removal. |
| `ECSqlToSqlGenerationTests` requires exact SQL — new code gen produces different (but equivalent) SQL | Medium | Medium | Review each diff; adjust tests where output is semantically equivalent. |
| ViewGenerator is not the only path that produces class SQL — some edge case uses a different code path | Low | Medium | Grep all callers of ViewGenerator; audit `ECSqlPreparer.cpp` exhaustively before removing it. |
| TPH overflow / shared-column edge cases missed by code generator | Medium | Medium | Add targeted unit tests for each MapStrategy × DML combination. |
| Apache-licensed Parser/ removal triggers licensing review | Low | Low | The files are Apache-2.0; removal is always allowed. No new licensing obligation is introduced. |
| Build-system churn (new `.cpp` files require `.mke` / `.PartFile.xml` updates) | Low | Low | Add new files to the build incrementally during Phase 1. |

---

## 11. Estimated Effort

| Task | Estimate |
|---|---|
| ECSqlLexer implementation + tests | 3 days |
| ECSqlAst definition | 2 days |
| ECSqlRDParser — SELECT (single, compound, CTE, subquery, window) | 6 days |
| ECSqlRDParser — INSERT / UPDATE / DELETE | 3 days |
| ECSqlRDParser — expressions (all operators, functions, CASE, CAST, IIF) | 4 days |
| ECSqlCodeGen — SELECT (all MapStrategy variants) | 5 days |
| ECSqlCodeGen — INSERT / UPDATE / DELETE (all MapStrategy variants) | 4 days |
| ECSqlCodeGen — property path expansion (struct, array, navigation, system) | 3 days |
| Integration wiring + compile-flag | 1 day |
| Test suite green (Phase 1) | 5 days |
| Phase 2 flip + Phase 3 removal + cleanup | 2 days |
| Documentation | 3 days |
| **Total** | **~41 working days** |

---

## 12. Success Criteria

- [ ] All tests in `iModelCore/ECDb/Tests/` pass without modification.
- [ ] `ECSqlToSqlGenerationTests` produces identical (or semantically equivalent, explicitly documented) native SQL for every test case.
- [ ] The Apache-licensed `Parser/` subdirectory is fully removed.
- [ ] All 105 `Exp` subclasses and `ECSqlParser.cpp` are removed.
- [ ] `ECSqlPreparer.cpp` and the four statement-preparer files are removed.
- [ ] No `connectivity::` or `OSQLParseNode` references remain in the codebase.
- [ ] All new files have complete Doxygen documentation.
- [ ] A `README.md` in `ECDb/ECSql/` describes the new pipeline.
- [ ] Net code reduction ≥ 10 000 lines.

---

## Appendix A — ECSQL Syntax Reference (for Parser Implementation)

### A.1 ECSQL Extensions Over Standard SQL

ECSQL extends standard SQL with the following Bentley-specific constructs that the new parser must handle:

| Extension | Syntax | Notes |
|---|---|---|
| Schema-qualified class names | `[schema:]ClassName` | Schema alias or schema name prefix |
| TableSpace prefix | `tablespace.schema:ClassName` | For attached databases |
| Polymorphic modifier | `ClassName` (polymorphic by default), `ONLY ClassName`, `+ClassName`, `ALL ClassName` | Controls whether subclasses are included |
| Relationship JOIN | `JOIN USING (RelClassName)` | ECSQL-specific join syntax |
| Navigation property creation | `NavClassDef(id, relClass)` | Constructs a navigation property value |
| ECClassId / ECInstanceId | System property names | Must map to special column expressions |
| SourceECInstanceId / TargetECInstanceId | Relationship constraint system properties | |
| `IsOfType(expr, type1, ...)` | Type predicate | Maps to ECClassId IN (…) filter |
| `ECSQLOPTIONS` clause | Query hints / options | Post-parse options block |
| Pragmas | `PRAGMA schema.name` | Meta-queries |

### A.2 System Property Name Mapping

| ECSQL Property | Primary Table Column | Notes |
|---|---|---|
| `ECInstanceId` | `Id` | Aliased to ECInstanceId in SELECT views |
| `ECClassId` | `ECClassId` | Only present in TPH tables |
| `SourceECInstanceId` | FK column or `SourceId` | Relationship class |
| `TargetECInstanceId` | FK column or `TargetId` | Relationship class |
| `SourceECClassId` | `SourceECClassId` | Link-table relationship |
| `TargetECClassId` | `TargetECClassId` | Link-table relationship |

### A.3 Key Invariants the Parser Must Enforce

- INSERT / UPDATE / DELETE target must be a concrete (non-abstract) class.
- INSERT / UPDATE / DELETE target must not be a mixin class.
- Polymorphic qualifier is only allowed in SELECT FROM clauses.
- A JOIN USING relationship must be a valid `ECRelationshipClass`.
- Property access across a navigation property path must be validated hop by hop.
- Parameter names (`?`, `:name`, `@name`) must be consistently either all positional or all named within a single statement.

---

## Appendix B — Key Mapping Patterns for Code Generator

### B.1 OwnTable Entity Class

```
ECSQL:   SELECT e.Name, e.Area FROM myschema:Widget e
SQL:     SELECT [t0].[Name], [t0].[Area] FROM [Widget] [t0]
```

### B.2 TablePerHierarchy (no overflow, polymorphic SELECT)

```
ECSQL:   SELECT e.ECClassId, e.Name FROM myschema:Base e
SQL:     SELECT [q0].[ECClassId], [q0].[Name]
         FROM (
           SELECT [ECClassId], [Id] ECInstanceId, [c0] [Name] FROM [Base_Table]
           UNION ALL
           SELECT [ECClassId], [Id] ECInstanceId, [c0] [Name] FROM [Base_Table]
               WHERE ECClassId IN (concrete subclass ids)
         ) [q0]
         -- (simplified; ViewGenerator produces the actual UNION ALL)
```

### B.3 Navigation Property (ForeignKeyRelationship in target)

```
ECSQL:   SELECT e.Widget FROM myschema:WidgetOwner e
         -- Widget is a navigation property → WidgetId column + WidgetRelClassId column
SQL:     SELECT [t0].[Widget_Id], [t0].[Widget_RelECClassId] FROM [WidgetOwner] [t0]
```

### B.4 TPH with Overflow Table

```
ECSQL:   SELECT e.RareProperty FROM myschema:Base e
         -- RareProperty lives in the overflow table Base_Overflow
SQL:     SELECT [ov].[c42] FROM [Base_Table] [t0]
         LEFT JOIN [Base_Overflow] [ov] ON [t0].[Id] = [ov].[Id]
```

### B.5 INSERT with ECClassId

```
ECSQL:   INSERT INTO myschema:Widget(Name, Area) VALUES(?, ?)
SQL:     INSERT INTO [Widget]([ECClassId],[Name],[Area]) VALUES(?,?,?)
         -- ECClassId value is bound by ECSqlInsertPreparer as the class's ECClassId
```

### B.6 UPDATE Compound (primary + overflow)

```
ECSQL:   UPDATE myschema:Base SET RareProperty = ? WHERE ECInstanceId = ?
SQL:     [1] UPDATE [Base_Table] SET … WHERE [Id] = ?   (if any primary cols change)
         [2] INSERT OR REPLACE INTO [Base_Overflow]([Id],[c42]) VALUES(?,?)
```

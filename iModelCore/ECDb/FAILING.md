# Pre-existing Test Failures

These 35 tests fail both **before and after** the ECSqlRDParser work on `affak/refactor`. They are not regressions — they were broken when the RD parser was first introduced and remain unaddressed.

Excluded from counts: `ThreadSafetyTests`, `ConcurrentQueryFixture`, `SchemaSyncTests`, `InstanceReaderFixture` (5 failures), `ECDbTestFixture.TestDropSchemasWithInstances` (segfault).

---

## Group 1 — `schema:ClassName` colon-separator not supported (20 failures)

The RD lexer treats `:Name` as a named parameter token, so `ecsql:PSA` is tokenized as
`ecsql` (identifier) + `:PSA` (named param) instead of a schema-qualified class name.
The old Bison grammar supported both `schema.ClassName` (dot) and `schema:ClassName` (colon).
The new RD parser only supports dot notation.

**Fix needed:** In `ParseTableNode` / `ParseClassRef`, detect and handle the colon-as-schema-separator form, or update the lexer to emit the colon as a separator token when followed by an identifier in a class-name context.

Failing tests:
```
ECSqlSelectPrepareTests.Alias
ECSqlSelectPrepareTests.Arrays
ECSqlSelectPrepareTests.From
ECSqlSelectPrepareTests.GroupBy
ECSqlSelectPrepareTests.Join
ECSqlSelectPrepareTests.NestedSubqueries
ECSqlSelectPrepareTests.Options
ECSqlSelectPrepareTests.OrderBy
ECSqlSelectPrepareTests.Structs
ECSqlSelectPrepareTests.TableRefWithoutSchemaNames
ECSqlSelectPrepareTests.Union
ECSqlInsertPrepareTests.Relationships
ECSqlUpdatePrepareTests.MiscellaneousWithALL
ECSqlUpdatePrepareTests.Options
ECSqlUpdatePrepareTests.Structs
ECSqlUpdatePrepareTests.WhereBasics
ECSqlDeletePrepareTests.MiscellaneousWithALL
ECSqlDeletePrepareTests.Options
ECSqlDeletePrepareTests.Structs
ECSqlPrepareTestFixture.ReservedTokens
```

Example failure:
```
Expected: ECSqlStatus::Success
Actual:   ECSqlStatus::InvalidECSql
Query:    SELECT * FROM ONLY ecsql:PSAHasPSA
```

---

## Group 2 — Pragma infrastructure (`SQLiteError 8` = SQLITE_AUTH) (7 failures)

Pragmas `sqlite_sql`, `explain_query`, `parse_tree`, and `PurgeOrphanLinkTableRelationships`
fail with `SQLiteError 8` (SQLITE_AUTH). This suggests the test ECDb is opened without the
authorization callback or write-token that the pragma system requires. Unrelated to parsing.

Failing tests:
```
ECSqlPragmasTestFixture.explain_query
ECSqlPragmasTestFixture.parse_tree
ECSqlPragmasTestFixture.PurgeOrphanLinkTableRelationships
ECSqlPragmasTestFixture.sqlite_sql
ECSqlPragmasTestFixture.view_generator_must_use_escaped_class_name_when_checking_disqualifed_check
ECSqlStatementTestFixture.MultilineStringLiteralOrName
ECSqlStatementTestFixture.view_generator_must_use_escaped_class_name_when_checking_disqualifed_check
```

Example failure:
```
Expected: ECSqlStatus::Success
Actual:   ECSqlStatus::SQLiteError 8
Query:    PRAGMA sqlite_sql("SELECT ...")
```

---

## Group 3 — Semantic: parenthesized literals now valid (3 failures)

The old Bison grammar treated `(expr)` in a WHERE context as a `(search_condition)`, making
comparisons like `(1) = 1` invalid. The new RD parser correctly accepts `(1)` as a
parenthesized value expression, so these queries now succeed. The test expectations are wrong
for the new (correct) behavior.

**Fix needed:** Update test expectations from `InvalidECSql` → `Success`.

Failing tests:
```
ECSqlStatementTestFixture.WhereBitwiseOperators
ECSqlUpdatePrepareTests.WhereBasics
ECSqlDeletePrepareTests.WhereBasics
```

Example failure:
```
Expected: ECSqlStatus::InvalidECSql
Actual:   ECSqlStatus::Success
Query:    WHERE (1)=1
```

---

## Group 4 — SQL generation differences (2 failures)

Tests assert exact JOIN counts or SQL structure in the generated SQLite SQL. The new
codegen produces structurally different (often more optimal) SQL that still returns correct
results but doesn't match hardcoded string expectations.

Failing tests:
```
ECSqlToSqlGenerationTests.OptimisedJoins
ECDbTestFixture.TestGreatestAndLeastFunctionsWithLiterals
ECDbTestFixture.TestGreatestAndLeastFunctionsDQLAndDML
```

Example failure:
```
Expected: GetFrequencyCount(sql, "JOIN") == 2
Actual:   GetFrequencyCount(sql, "JOIN") == 0
```

---

## Group 5 — Miscellaneous (3 failures)

```
ECSqlStatementTestFixture.GetParameterIndex    — named param index lookup (schema:class form)
ECSqlStatementTestFixture.NoECClassIdFilterOption — schema:class name with colon separator
ECSqlStatementTestFixture.WhereBitwiseOperators   — see Group 3
```

---

## Summary Table

| Group | Root Cause | Count | Fix Complexity |
|-------|-----------|-------|----------------|
| 1 | `schema:ClassName` colon syntax unsupported | 20 | Medium — lexer/parser change |
| 2 | Pragma SQLITE_AUTH in test environment | 7 | Low — test setup issue |
| 3 | Test expectations wrong for new correct behavior | 3 | Trivial — update expectations |
| 4 | SQL generation structure differs | 2 | Low — update assertions |
| 5 | Colon syntax + other | 3 | Covered by Group 1 & 3 fixes |
| **Total** | | **35** | |

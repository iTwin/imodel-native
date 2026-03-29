# Pre-existing Test Failures

These 10 tests fail both **before and after** the ECSqlRDParser work on `affak/refactor`. They are not regressions — they were broken when the RD parser was first introduced and remain unaddressed.

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

## Group 3 — Semantic: parenthesized literals now valid ✅ FIXED

The old Bison grammar treated `(expr)` in a WHERE context as a `(search_condition)`, making
comparisons like `(1) = 1` invalid. The new RD parser correctly accepts `(1)` as a
parenthesized value expression, so these queries now succeed.

**Fixed:** Test expectations updated from `InvalidECSql` → `Success` in:
- `ECSqlStatementTestFixture.WhereBitwiseOperators` — `(1)=1`, `(1)=(1)`, `((1)=(1))` forms
- `ECSqlUpdatePrepareTests.WhereBasics` — `(P2D.X)>=(P3D.X)` form; added `(I&1)=1 AND (I|2=I)` Success variant
- `ECSqlDeletePrepareTests.WhereBasics` — same as above

---

## Group 4 — SQL generation differences ✅ FIXED

Tests assert exact JOIN counts or error message text in generated SQLite SQL. The new
codegen / RD parser produces structurally different output that still returns correct
results.

**Fixed:**
- `ECSqlToSqlGenerationTests.OptimisedJoins` — was already passing on `affak/refactor`
- `ECDbTestFixture.TestGreatestAndLeastFunctionsWithLiterals` — fixed by improving RD
  parser error messages to match the old Bison parser format:
  - `ECSQLERR` now prefixes every message with `"Failed to parse ECSQL '<sql>': "`
  - `MAX()`/`MIN()` with 0 args now emits `"syntax error"` instead of `"Unexpected token ')' in value expression"`
  - `MAX(a,b,...)`/`MIN(a,b,...)` with multiple args now emits the helpful
    `"Use GREATEST(...)"` / `"Use LEAST(...)"` diagnostic instead of a cryptic
    `"Expected token 41, got ','"`
  - Comment-only / empty input now emits `"syntax error"` instead of
    `"Unexpected token '' at start of ECSQL statement"`
- `ECDbTestFixture.TestGreatestAndLeastFunctionsDQLAndDML` — fixed by same changes above

---

## Group 5 — Miscellaneous (2 remaining failures)

```
ECSqlStatementTestFixture.NoECClassIdFilterOption       — schema:class name with colon separator (covered by Group 1)
ECSqlStatementFunctionTestFixture.BuiltinFunctions      — LIKE keyword accepted as function call by new RD parser
```

`GetParameterIndex` and `WhereBitwiseOperators` were resolved by the Group 1 and Group 3
fixes respectively.

---

## Summary Table

| Group | Root Cause | Count | Fix Complexity |
|-------|-----------|-------|----------------|
| 1 | `schema:ClassName` colon syntax unsupported | 1 | ✅ Mostly fixed; `ReservedTokens` remains |
| 2 | Pragma SQLITE_AUTH in test environment | 7 | Low — test setup issue |
| 3 | Test expectations wrong for new correct behavior | 0 | ✅ Fixed |
| 4 | SQL generation / error message differences | 0 | ✅ Fixed |
| 5 | Miscellaneous parser keyword/colon issues | 2 | Low |
| **Total** | | **10** | |

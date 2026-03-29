# Pre-existing Test Failures

These 11 tests fail both **before and after** the ECSqlRDParser work on `affak/refactor`. They are not regressions — they were broken when the RD parser was first introduced and remain unaddressed.

Excluded from counts: `ThreadSafetyTests`, `ConcurrentQueryFixture`, `SchemaSyncTests`, `InstanceReaderFixture` (4 failures), `IntegrityCheckerFixture.check_nav_class_ids` (pragma SQLiteError 8 + null-ptr crash; same root cause as Group 2).

---

## Group 1 — `schema:ClassName` colon-separator not supported (1 remaining failure)

The RD lexer treats `:Name` as a named parameter token, so `ecsql:PSA` is tokenized as
`ecsql` (identifier) + `:PSA` (named param) instead of a schema-qualified class name.
The old Bison grammar supported both `schema.ClassName` (dot) and `schema:ClassName` (colon).
The new RD parser only supports dot notation.

Most tests in this group were fixed by commit `338ad8e08f`. One test remains because it
exercises additional colon-separator forms not yet handled.

**Fix needed:** In `ParseTableNode` / `ParseClassRef`, detect and handle the colon-as-schema-separator form, or update the lexer to emit the colon as a separator token when followed by an identifier in a class-name context.

Failing tests:
```
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

**Fixed in commit `6517c92d29`:** Test expectations updated from `InvalidECSql` → `Success` in:
- `ECSqlStatementTestFixture.WhereBitwiseOperators` — `(1)=1`, `(1)=(1)`, `((1)=(1))` forms
- `ECSqlUpdatePrepareTests.WhereBasics` — `(P2D.X)>=(P3D.X)` form
- `ECSqlDeletePrepareTests.WhereBasics` — same as above

---

## Group 4 — SQL generation / error message differences ✅ FIXED

**Fixed in commit `bec7bfca8c`:** Improved RD parser error messages:
- `ECSQLERR` now prefixes every message with `"Failed to parse ECSQL '<sql>': "`
- `MAX()`/`MIN()` with 0 args → `"syntax error"`
- `MAX(a,b,...)`/`MIN(a,b,...)` with multiple args → `"Use GREATEST(...)"` / `"Use LEAST(...)"`
- Comment-only / empty input → `"syntax error"`

---

## Group 5 — Miscellaneous (2 remaining failures)

```
ECSqlStatementTestFixture.NoECClassIdFilterOption       — schema:class name with colon separator (covered by Group 1)
ECSqlStatementFunctionTestFixture.BuiltinFunctions      — LIKE keyword accepted as function call by new RD parser
```

`SELECT LIKE(S,'Sample') FROM ecsql.P LIMIT 1` succeeds when it should fail; `LIKE` is an
ECSql predicate operator, not a callable function, but the RD parser accepts it as a
function name when followed by `(`.

---

## Group 6 — Disqualifying join term (`+` prefix on table references) (1 failure)

The new RD parser does not support the `+` prefix on table references in FROM/JOIN clauses
(used to force a disqualified primary join term). The old Bison grammar accepted this form.

**Fix needed:** In `ParseTableNode` / `ParseFromClause`, detect and handle the `+` prefix
on table references.

Failing tests:
```
JoinedTableTestFixture.Disqualifying_PrimaryJoinTerm
```

Example failures (4 assertions inside one test):
```
Expected: ECSqlStatus::Success
Actual:   ECSqlStatus::InvalidECSql
Query:    SELECT * FROM ts.Goo JOIN +[ts].[Doo] ON Goo.ECInstanceId=Doo.ECInstanceId

Expected: ECSqlStatus::Success
Actual:   ECSqlStatus::InvalidECSql
Query:    SELECT * FROM (SELECT * FROM +ts.Doo)
```

---

## Group 7 — Navigation value column alias not reflected in property name ✅ FIXED

**Fixed in commit (this one):** `PrepareNavValueCreationFuncExp` now passes the **outer**
`DerivedPropertyExp` (which carries the column alias) to `ECSqlFieldFactory::CreateField`
instead of the inner column-reference derived property. `DerivedPropertyExp::GetName()`
already handles the `NavValueCreationFunc` expression type, returning the column alias when
present or the underlying nav property name when not.

Before the fix, `SELECT NAVIGATION_VALUE(ts.Book.Author, ...) [MyNavProp]` created an
"Author"-named generated property regardless of the alias; after the fix it correctly
creates a "MyNavProp"-named property.

---

## Summary Table

| Group | Root Cause | Count | Fix Complexity |
|-------|-----------|-------|----------------|
| 1 | `schema:ClassName` colon syntax unsupported | 1 | Low — `ReservedTokens` remnant |
| 2 | Pragma SQLITE_AUTH in test environment | 7 | Low — test setup issue |
| 3 | Test expectations wrong for new correct behavior | 0 | ✅ Fixed |
| 4 | SQL generation / error message differences | 0 | ✅ Fixed |
| 5 | Miscellaneous parser keyword/colon issues | 2 | Low |
| 6 | Disqualifying `+` prefix on table references | 1 | Medium — parser feature gap |
| 7 | Nav value alias not reflected in column property name | 0 | ✅ Fixed |
| **Total** | | **11** | |

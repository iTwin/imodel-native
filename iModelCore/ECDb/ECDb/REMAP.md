# ECDb Column Remapping — Complete Reference

This document covers all edge cases of ECDb schema-update property remapping, including single-table remap, cross-table remap, cyclical remap, and what information is needed to transform changesets when properties are remapped.

See also: `README_Transform.md` for the high-level implementation flow.

---

## 1. Overview

When an ECSchema is updated, properties may need to be reassigned to different shared columns. This is called **remapping**. The `RemapManager` handles:

1. **Collecting** what changed (added properties, new base classes, deleted property overrides)
2. **Cleaning** old `ec_PropertyMap` entries so re-mapping can assign new columns
3. **Re-mapping** via `MapSchemas()` — fills in the new column assignments
4. **Detecting** which columns actually changed (old vs new)
5. **Sorting** the column moves into a safe execution order
6. **Executing** SQL UPDATEs to physically move instance data between columns

The import **must** use the flag `SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade` to allow data movement.

---

## 2. Property Type → Column Layout

| Property Type | # Columns | Column Kind | Notes |
|---|---|---|---|
| Primitive (int, string, double, bool, binary, DateTime) | 1 | Shared (kind=4) | Single shared column |
| Point2d | 2 | Shared (kind=4) | X, Y in consecutive shared columns |
| Point3d | 3 | Shared (kind=4) | X, Y, Z in consecutive shared columns |
| ECStruct | N | Shared (kind=4) | One per leaf member, recursive. Must be consecutive in a single table |
| Primitive Array (int[], string[], etc.) | 1 | Shared (kind=4) | Stored as JSON blob |
| Struct Array | 1 | Shared (kind=4) | Stored as JSON blob |
| NavigationProperty | 2 | Shared (kind=4) + Virtual (kind=0, IsVirtual=1) | Id column + virtual RelECClassId column |

**Important**: ECStruct properties must occupy **consecutive columns in a single table**. If even one column of a struct conflicts, the entire struct is remapped.

---

## 3. Scenarios That Trigger Remapping

### 3.1 Move Property Up in Hierarchy (Override → Base Definition)

**Trigger**: A property defined independently on derived classes is added to a base class, turning derived definitions into overrides.

```
Before:                          After:
  Base                             Base
  ├─ A (prop: age)                 │ (prop: age)    ← added here
  └─ B (prop: age)                 ├─ A             ← now override
                                   └─ B             ← now override
```

The base class gets a fresh column assignment for `age`. Derived class data (which used different shared columns) must move to the new base column.

**Test**: `MovePropertyUpInHierarchySimplified`, `MoveMultiplePropertiesUpInHierarchy`

### 3.2 Insert New Base Class into Hierarchy

**Trigger**: A new class is inserted between a parent and child in the hierarchy, and that class brings properties that conflict with existing column assignments.

```
Before:                          After:
  Animal                           Animal
  ├─ Duck (prop: age)              └─ Mammal (prop: age, species) ← NEW
  └─ Zebra (prop: name, age)          ├─ Duck
                                       └─ Zebra (prop: name)
```

Two sub-cases:
- **New class already existed** (just re-parented): Its properties already have column assignments. Derived classes with conflicting columns must remap.
- **New class is brand new**: Its properties get mapped fresh. Derived properties with the same name become overrides and may need to move.

**Tests**: `AddNewBaseClassInMiddleMovePropertyUp`, `InsertBaseClassHierarchy`, `PutSiblingsIntoHierarchy`, `InjectBaseClassInBaseSchema`

### 3.3 Delete Property Override

**Trigger**: A property override on a derived class is explicitly deleted. The derived class now inherits the base class column assignment. If data existed in the old column, it must be moved.

**Test**: `RemoveOverwrittenProperty`, `MovePropertyUpInHierarchyRemoveOriginal`

### 3.4 Move Property to Mixin

**Trigger**: A property is added to a mixin that is already applied to classes that have independently defined the same property.

**Test**: `MovePropertyToMixin`, `MovePropertyToMixinAndRemoveOriginal`, `MixinToBaseClass`

### 3.5 Move Property to Non-Shared Column (Base Class owns non-shared columns)

**Trigger**: When `ApplyToSubclassesOnly=True`, base class properties go into dedicated (non-shared) columns. Moving a property from a derived class (shared column) to the base class means moving from a shared column to a non-shared column.

**Test**: `MovePropertyToNonSharedColumn`, `MovePropertiesInNonSharedColumns`

### 3.6 Re-parent a Class (Change Base Class)

**Trigger**: A class changes its base class within the same hierarchy. Column assignments from the new base may conflict.

**Test**: `ChangeBaseClassDownInExistingHierarchy`, `MoveClassToDifferentHierarchy`

### 3.7 Struct Property with Modification + Move

**Trigger**: A struct class gains a new member AND the struct property is moved to a base class in the same schema import.

**Test**: `ModifyAndMoveStruct`

### 3.8 Relationship Class Properties

**Trigger**: Properties on ECRelationshipClass also participate in remapping when moved up or restructured.

**Test**: `MovePropertyOnRelationshipClass`

---

## 4. Single-Table Remap

**Definition**: Old column and new column are in the **same SQLite table**. This is the common case for TablePerHierarchy with shared columns.

### SQL Pattern

```sql
-- Move data from old column to new column
UPDATE [tableName] SET [newCol]=[oldCol] WHERE ([ECClassId]=0xNN)

-- Clean old column
UPDATE [tableName] SET [oldCol]=NULL WHERE ([ECClassId]=0xNN)
```

The `WHERE ECClassId=0xNN` ensures only rows for the specific class are affected.

### Sorting Requirement

If multiple columns move within the same table, **order matters**:
- Moving `ps1→ps2` and `ps2→ps3`: The second must execute before the first, or `ps2` data is lost.
- The `SortRemappedColumnInfos` function performs a topological sort based on column dependencies.

Algorithm:
1. Build a map of `oldColumnId → RemappedColumnInfo`
2. Iteratively extract entries whose `newColumnId` is NOT anyone else's `oldColumnId`
3. Those entries are safe to execute (nothing will overwrite their target)
4. Repeat until map is empty or only circular dependencies remain

---

## 5. Cross-Table Remap

**Definition**: Old column is in one SQLite table, new column is in a different table. This happens with:
- **Overflow tables**: When `MaxSharedColumnsBeforeOverflow` is hit, properties spill into a `_Overflow` table
- **JoinedTablePerDirectSubclass**: Different parts of the hierarchy map to different tables

### SQL Pattern

```sql
-- Move data between tables using correlated subquery
UPDATE [newTable] SET [newCol] = (
    SELECT [oldCol] FROM [oldTable]
    WHERE [oldTable].[oldInstanceIdCol] = [newTable].[newInstanceIdCol]
) WHERE ([ECClassId]=0xNN)

-- Clean old column
UPDATE [oldTable] SET [oldCol] = NULL WHERE ([ECClassId]=0xNN)
```

The **Instance ID column** is crucial — it joins rows between tables. The `GetInstanceIdColumnName()` function queries `ec_Column` for `ColumnKind=1` (the Id column) for each table.

### Custom Instance ID Column

Tables can use custom instance ID column names via `<ECInstanceIdColumn>CustomId</ECInstanceIdColumn>`. The cross-table remap correctly looks up the column name per table.

**Test**: `MovePropertyToOverflowUsingDifferentIdColumn`

### Supported Cross-Table Scenarios

| Scenario | Support Status | Test |
|---|---|---|
| Move TO overflow table | ✅ Supported | `MovePropertyToOverflow` |
| Move FROM overflow table | ✅ Supported | `MovePropertyFromOverflow` |
| Move FROM overflow + drop empty overflow table | ❌ Not supported (returns error) | `MovePropertyFromOverflowDropOverflowTable` |

---

## 6. Cyclical Remap (a→b→c→a)

**Definition**: Multiple columns form a cycle where each needs the value of another:
```
Column A currently holds data for Property X  →  needs to move to Column B
Column B currently holds data for Property Y  →  needs to move to Column C
Column C currently holds data for Property Z  →  needs to move to Column A
```

This is a **circular dependency** — no topological order exists.

### How It Works

1. `SortRemappedColumnInfos` processes all non-circular entries first (topological sort)
2. Remaining entries (those forming cycles) are passed to `SortCircularRemappedColumnInfos`
3. Cycle detection: Start from any remaining entry, follow the chain `old→new→old→...` until it loops back to the start
4. Each detected cycle is collected as a `vector<RemappedColumnInfo*>`

### Single-Table Circular: Supported ✅

When all columns in the cycle are in the **same table**, SQLite's single-statement UPDATE atomically resolves it:

```sql
UPDATE [table] SET [colA]=[colB], [colB]=[colC], [colC]=[colA] WHERE ([ECClassId]=0xNN)
```

SQLite evaluates the right-hand side of all assignments using the **old** row values, then applies all changes. This naturally handles swaps.

**Test**: `MoveMultiplePropertiesInCircle`, `SwapColumnsForProperty`

### Cross-Table Circular: NOT Supported ❌

When a cycle spans multiple tables, the current implementation **returns an error**:

```
"Cannot perform update on instance data for remapped properties. This update requires moving data
between columns in a circular way across tables, which is not supported."
```

**Test**: `SwapColumnsWithOverflow` (asserts `ASSERT_EQ(ERROR, ImportSchema(...))`)

**Potential Fix**: Use a temporary table or in-memory storage to break the cycle:
1. Copy one column's data to a temp location
2. Perform the remaining non-circular moves
3. Copy from temp to the final destination

---

## 7. When Remapping Has Multiple Classes

The `RemapManager` groups remap operations **per ECClassId**. Each class is handled independently:

```
m_remappedColumns: map<ECClassId, vector<RemappedColumnInfo>>
```

For each class:
1. Sort its column moves (topological sort + cycle detection)
2. Execute non-circular moves in sorted order
3. Execute circular moves as single UPDATE statements

This means: If class A has a simple move and class B has a circular swap, they are handled separately — A's move doesn't block B's swap.

---

## 8. Edge Cases and Failure Modes

### 8.1 Struct Must Be Consecutive

ECStruct properties MUST have their leaf columns consecutive within a single table. If a struct has 3 members and the target table has a gap, the entire struct fails to map.

### 8.2 MaxSharedColumnsBeforeOverflow

When the main table is full, new columns go to the overflow table. This can trigger cross-table scenarios unexpectedly. With a very low `MaxSharedColumnsBeforeOverflow` (e.g., 1-2), most remap operations involve overflow tables.

### 8.3 Multiple Base Classes (Mixins)

A class can have multiple base classes (entity + mixin). Properties from a mixin can conflict with properties from the primary base class. Mixin properties don't always share the same column across all implementing classes.

### 8.4 Deep Hierarchy with Multiple Schema Files

Remap can span multiple schema files. The `EnsureInvolvedSchemasAreLoaded` method ensures all relevant schemas are loaded so `DerivedClasses()` navigation works properly.

### 8.5 Same-Name Properties at Different Levels

If `Duck.age` exists and `Animal.age` is added, `Duck.age` becomes an override. The `detectNewPropertyOverridesSql` CTE specifically finds properties that now become overrides due to new base class definitions.

### 8.6 Property on Relationship Classes

ECRelationshipClass properties work identically to entity class properties for remapping. They use the same shared column mechanism.

### 8.7 Navigation Properties

Nav properties occupy 2 column slots: the FK Id column (kind=4) and a virtual RelECClassId column (kind=0, IsVirtual=1). The clean query filters:
```sql
WHERE [col].[ColumnKind] = 4 OR ([col].[ColumnKind] = 0 AND [col].[IsVirtual] = 1)
```

Both columns are moved together.

---

## 9. Changeset-Level Property Transformation

The `ChangesetSchema` / `ChangesetSchemaDiff` / `ChangesetTransformer` system handles transforming changesets after schema remapping. It works with **before/after snapshots** of the schema-to-column mapping.

### 9.1 Architecture (Already Implemented)

See `ChangesetSchema.md` for full details. The system has three stages:

1. **Capture** (`ChangesetSchema::Capture` or `CaptureFromDb`) — Snapshots the class→table→column mapping state.
2. **Diff** (`ChangesetSchemaDiff::Diff`) — Compares before/after snapshots and produces:
   - `m_classIdRemaps` — Class IDs that changed value (same class, different ID)
   - `m_columnSwaps` — Properties that moved to a different column (per class, per accessString)
   - `m_overflowTablesAdded` — New overflow tables that require synthetic rows
   - `m_errors` — Missing classes/properties/tables (should never happen with additive changes)
3. **Transform** (`ChangesetTransformer::Transform`) — Rewrites the changeset binary using the diff.

### 9.2 Information Available in the Diff (per ColumnSwap)

```cpp
struct ColumnSwap {
    Utf8String m_classKey;       // "SchemaName:ClassName"
    Utf8String m_accessString;   // EC property path (e.g., "S.X", "Age", "Origin.X")
    Utf8String m_oldTable;       // SQLite table where value was stored
    Utf8String m_oldColumn;      // Column name in old table
    Utf8String m_newTable;       // SQLite table where value now lives
    Utf8String m_newColumn;      // Column name in new table
};
```

The Transformer also queries the **target database** at transform time for:
- Column ordinals per table (`ec_Column.Ordinal` WHERE `IsVirtual=0`)
- ECClassId column ordinals (ColumnKind=2, plus SourceECClassId/TargetECClassId by name)
- ExclusiveRootClassId per table (for tables without per-row ECClassId)
- Total column count per table in the current db

### 9.3 How the Transformer Handles Multiple Columns (Already Correct)

The transformer iterates ALL `columnSwaps` for a class and builds a `TableRemap` structure:
```cpp
struct TableRemap {
    map<int, int> m_sameTable;     // srcOrd → dstOrd (same table, ordinal changed)
    map<int, XMove> m_crossTable;  // srcOrd → {dstTable, dstOrd} (cross-table move)
    map<int, int> m_dstToSrc;      // reverse: dstOrd → srcOrd (for circular resolution)
    set<int> m_crossTableSrcOrds;  // source ordinals that move cross-table
};
```

**Circular swaps are handled naturally**: For INSERT/DELETE, the transformer iterates by *destination* ordinal and looks up the source via `dstToSrc`. Since all assignments read from the source changeset (immutable during iteration), circular dependencies resolve correctly in a single pass.

For UPDATE, it iterates by *source* ordinal and maps to the destination. Cross-table columns are skipped (handled in overflow rows).

### 9.4 What Additional Information Might Be Needed

Comparing REMAP.md Section 9 (original) against the actual implementation:

| What REMAP.md suggested was needed | Already available in implementation? | Notes |
|---|---|---|
| ECClassId per class | ✅ Via `classIdRemapMap` + `curClassIdToKey` | Looked up from changeset row ECClassId column |
| Old table + column name | ✅ `ColumnSwap.m_oldTable/m_oldColumn` | |
| New table + column name | ✅ `ColumnSwap.m_newTable/m_newColumn` | |
| Column ordinals | ✅ `QueryTableColInfo()` queries at transform time | |
| Instance ID column per table | ✅ Not needed — overflow rows use PK (ordinal 0) | Changeset PK is always first column |
| Circular group detection | ✅ Handled implicitly by `dstToSrc` mapping | No explicit cycle detection needed for changeset (unlike RemapManager) |
| Multi-column properties (Point2d, struct) | ✅ Each leaf is a separate ColumnSwap entry | `S.X`, `S.Y`, `S.Z` are independent swaps |
| Class-specific filtering | ✅ `classKey` lookup per row via ECClassId | |
| Overflow table creation | ✅ `m_overflowTablesAdded` with synthetic INSERT | |

**Conclusion**: The existing `ChangesetTransformer` already has all the information it needs from `ChangesetSchemaDiff`. The diff provides per-class, per-accessString column moves. The transformer resolves ordinals at runtime from the target db. No additional metadata beyond what `ChangesetSchemaDiff` captures is required.

### 9.5 Key Difference from RemapManager

The `RemapManager` must sort and detect circular dependencies because it executes SQL UPDATEs sequentially against a live database. The `ChangesetTransformer` does NOT need this because:
- It reads from the **source changeset** (immutable) and writes to a **new output**
- All source values are available simultaneously
- It maps by destination ordinal (INSERT/DELETE) → reads the correct source ordinal from `dstToSrc`
- Cross-table moves generate separate synthetic overflow rows

### 9.6 Handling Changesets with Mixed Classes

A single table (TPH) can store rows for many classes. The transformer:
1. Reads `ECClassId` from the changeset row (old value for DELETE, new value for INSERT/UPDATE)
2. Falls back to `ExclusiveRootClassId` for tables without per-row ECClassId
3. Looks up `classKey` from either `oldClassIdToKey` (if class was remapped) or `curClassIdToKey`
4. Only applies remap rules matching that class — other class rows pass through unchanged

### 9.7 Edge Cases Handled by the Transformer

| Scenario | How It's Handled |
|---|---|
| Column count mismatch (new columns in target db) | INSERT/DELETE: new ordinals beyond `nCols` get NULL |
| Cross-table move | Synthetic overflow row generated with PK + ECClassId + moved columns |
| Property deleted | Not applicable — schema changes are additive; Diff returns error if property disappears |
| New property added (by other user) | No swap entry → ordinal passes through; new columns get NULL |
| Circular swap (same table) | `dstToSrc` resolves — reads from immutable source, writes to new output |
| UPDATE where only some columns changed | Only defined columns are written; undefined = "unchanged" marker |
| Relationship tables (SourceECClassId, etc.) | Class ID remap applied to all columns in `classIdOrdinals` set |

---

## 10. Summary of Support Status

| Scenario | Status | Notes |
|---|---|---|
| Move primitive property up | ✅ | Core scenario, well tested |
| Move Point2d/Point3d up | ✅ | Multi-column, same table |
| Move ECStruct up | ✅ | Multi-column, consecutive requirement |
| Move primitive array up | ✅ | Single JSON blob column |
| Move struct array up | ✅ | Single JSON blob column |
| Move navigation property up | ✅ | 2 columns (Id + virtual RelClassId) |
| Move to overflow (cross-table) | ✅ | Uses instance ID join |
| Move from overflow (cross-table) | ✅ | Uses instance ID join |
| Column swap (same table) | ✅ | Single atomic UPDATE |
| Circular remap N columns (same table) | ✅ | Single atomic UPDATE |
| Circular remap across tables | ❌ | Returns error |
| Drop empty overflow after move | ❌ | Returns error |
| Insert base class in hierarchy | ✅ | Handles resident vs migrating columns |
| Move property to mixin | ✅ | Mixin treated as base class |
| Remap on relationship class | ✅ | Same mechanism as entities |
| Cross-schema base class injection | ✅ | Multi-schema import supported |

---

## 11. Key SQL Queries Used Internally

### Detect Properties Needing Remap (New Base Class)

```sql
WITH
[residentColumns](ColumnId, AccessString, ColumnName) AS (
    -- Columns already assigned to the new base class hierarchy (excluding derived)
    SELECT DISTINCT col.Id, pp.AccessString, col.Name
    FROM ec_PropertyMap pm
    JOIN ec_Column col ON col.Id = pm.ColumnId
    JOIN ec_PropertyPath pp ON pp.Id = pm.PropertyPathId
    WHERE pm.ClassId IN (
        SELECT ClassId FROM ec_Cache_ClassHierarchy ch
        WHERE ch.BaseClassId = :baseClassId
          AND NOT EXISTS (SELECT 1 FROM ec_Cache_ClassHierarchy WHERE ClassId = ch.ClassId AND BaseClassId = :derivedClassId)
        UNION SELECT BaseClassId FROM ec_Cache_ClassHierarchy WHERE ClassId = :baseClassId
    ) AND col.ColumnKind = 4
),
[migratingColumns](ColumnId, AccessString, ColumnName, RootPropertyId) AS (
    -- Columns currently used by derived class hierarchy
    SELECT DISTINCT col.Id, pp.AccessString, col.Name, pp.RootPropertyId
    FROM ec_PropertyMap pm
    JOIN ec_Column col ON col.Id = pm.ColumnId
    JOIN ec_PropertyPath pp ON pp.Id = pm.PropertyPathId
    WHERE pm.ClassId IN (SELECT DISTINCT ClassId FROM ec_Cache_ClassHierarchy WHERE BaseClassId = :derivedClassId)
      AND col.ColumnKind = 4
)
-- Properties that conflict: same column but different access string
SELECT DISTINCT mc.RootPropertyId, prop.Name, cl.Name
FROM migratingColumns mc
JOIN ec_Property prop ON prop.Id = mc.RootPropertyId
JOIN ec_Class cl ON prop.ClassId = cl.Id
WHERE EXISTS (
    SELECT 1 FROM residentColumns rc
    WHERE rc.ColumnId = mc.ColumnId AND rc.AccessString != mc.AccessString
)
```

### Detect Properties That Become Overrides

```sql
WITH
[propertyNamesUpTheHierarchy](PropertyName) AS (
    SELECT DISTINCT p.Name
    FROM ec_Property p
    WHERE p.ClassId IN (
        SELECT :baseClassId UNION ALL
        SELECT BaseClassId FROM ec_cache_ClassHierarchy WHERE ClassId = :baseClassId
    )
),
[mappedPropertyNamesDownTheHierarchy](PropertyName) AS (
    SELECT DISTINCT p.Name
    FROM ec_Property p
    JOIN ec_PropertyPath pp ON pp.RootPropertyId = p.Id
    JOIN ec_PropertyMap pm ON pm.PropertyPathId = pp.Id
    WHERE p.ClassId IN (
        SELECT :derivedClassId UNION ALL
        SELECT ClassId FROM ec_cache_ClassHierarchy WHERE BaseClassId = :derivedClassId
    )
)
SELECT PropertyName
FROM mappedPropertyNamesDownTheHierarchy
WHERE PropertyName IN propertyNamesUpTheHierarchy
```

### Get Instance ID Column for Table

```sql
SELECT col.Name
FROM ec_Table tab
JOIN ec_Column col ON tab.Id = col.TableId
WHERE tab.Name = :tableName AND col.ColumnKind = 1
```

---

## 12. Diagnostic Query

Use this to inspect the current column mapping state:

```sql
SELECT s.Name schemaName, cl.Id classId, cl.Name className,
       p.Id propertyId, p.Name PropertyName, p.Kind Kind,
       pp.AccessString, pp.Id PropertyPathId,
       t.Name tableName, c.Name columnName
FROM ec_PropertyMap pm
JOIN ec_Column c ON c.Id = pm.ColumnId
JOIN ec_Table t ON t.Id = c.TableId
JOIN ec_Class cl ON cl.Id = pm.ClassId
JOIN ec_PropertyPath pp ON pp.Id = pm.PropertyPathId
JOIN ec_Schema s ON cl.SchemaId = s.Id
JOIN ec_Property p ON pp.RootPropertyId = p.Id
WHERE s.Name = 'TestSchema' AND c.ColumnKind = 4
ORDER BY cl.Id, p.Id
```

---

## 13. Complete Test Case Matrix for Changeset Transform Coverage

This section lists ALL test cases needed to verify full coverage of changeset transformation across all supported remap scenarios.

### 13.1 Existing Test Coverage (in ChangesetSchemaTests.cpp)

| Test Name | What It Covers |
|---|---|
| `CaptureAndRoundTrip` | Basic capture + JSON serialization |
| `PropertyRemapColumnSwap` | Move primitive property up (end-to-end diff detection) |
| `StructPropertyCausesOverflow` | Struct member addition triggers overflow table |
| `DiffErrorMissingClass` | Error: class disappeared from "after" |
| `DiffErrorMissingProperty` | Error: property mapping lost |
| `DiffErrorDisappearedTable` | Error: table removed |
| `DiffClassIdRemap` | ClassId value changed between before/after |
| `DiffColumnSwapWithInheritance` | Column swap detected through inheritance resolution |
| `DiffOverflowTableAdded` | Overflow table creation detected |
| `DiffToJson` | Diff JSON serialization |
| `CaptureFromChangeset_OnlyReferencedClasses` | Changeset-scoped capture (only affected classes) |
| `CaptureFromChangeset_MultipleClasses` | Multi-class changeset capture |
| `CaptureFromChangeset_MatchesCaptureFromDbWhenAllReferenced` | Equivalence with full capture |
| `TransformRefusesOnErrors` | Transform returns ERROR when diff has errors |
| `TransformPassthroughWhenNoTransformNeeded` | Passthrough when no transform needed |
| `EndToEnd_CaptureAndTransform` | Full workflow: capture→schema upgrade→capture→diff→transform |
| `Transform_ECClassIdRemap` | Transform correctly remaps ECClassId values |
| `Transform_ColumnSwap_PropertyMovedUpHierarchy` | **Circular swap**: INSERT + UPDATE with column swap |
| `Transform_OtherUserAddedColumns` | New columns added by other user → NULL padding |
| `Transform_StructPropertyOverflowCrossTableMove` | Cross-table: property moves to overflow table |
| `Transform_RelClassIdRemap` | SourceECClassId/TargetECClassId class ID remap |

### 13.2 Test Cases Needed (Not Yet Covered)

The following tests are needed for full coverage of all remap types that `RemapManager` supports:

#### Category A: Per-Property-Type Column Swap Transform

Each property type uses different number of columns. The transform must correctly handle multi-column properties where EACH leaf column gets its own `ColumnSwap` entry.

| # | Test Case | Property Type | Columns Involved | Ops to Test |
|---|---|---|---|---|
| A1 | `Transform_Point2dPropertyMovedUp` | Point2d | 2 (X, Y) | INSERT, UPDATE, DELETE |
| A2 | `Transform_Point3dPropertyMovedUp` | Point3d | 3 (X, Y, Z) | INSERT, UPDATE, DELETE |
| A3 | `Transform_NavPropertyMovedUp` | NavigationProperty | 1 (Id col; virtual RelClassId excluded from changeset) | INSERT, UPDATE, DELETE |
| A4 | `Transform_PrimitiveArrayPropertyMovedUp` | int[] / string[] | 1 (JSON blob) | INSERT, UPDATE |
| A5 | `Transform_StructArrayPropertyMovedUp` | StructArray | 1 (JSON blob) | INSERT, UPDATE |
| A6 | `Transform_StructPropertyMovedUp` | ECStruct (3 members) | 3 (one per leaf) | INSERT, UPDATE |
| A7 | `Transform_NestedStructPropertyMovedUp` | Nested ECStruct (S.Inner.X) | N (deep leaf paths) | INSERT, UPDATE |

#### Category B: Circular Swap Scenarios (Same Table)

| # | Test Case | Description | Key Verification |
|---|---|---|---|
| B1 | `Transform_TwoPropertyCircularSwap` | A→col1, B→col2 becomes A→col2, B→col1 | Both INSERT and UPDATE correctly swap |
| B2 | `Transform_ThreePropertyCircularSwap` | A→1, B→2, C→3 becomes A→2, B→3, C→1 | 3-way rotation correct |
| B3 | `Transform_FourPropertyCircularSwap` | 4 properties rotating columns | 4-way rotation correct |
| B4 | `Transform_MixedCircularAndNonCircular` | Some props swap, others move linearly | Both handled in same changeset row |
| B5 | `Transform_CircularSwapWithNullValues` | Swap where one column is NULL | NULL correctly moved to new position |

#### Category C: Cross-Table Moves (Overflow)

| # | Test Case | Description | Key Verification |
|---|---|---|---|
| C1 | `Transform_SinglePropertyMovesToOverflow` | One property moves to overflow table | INSERT generates synthetic overflow row |
| C2 | `Transform_MultiplePropertiesMoveToOverflow` | Several properties go to overflow | Synthetic overflow row has all moved values |
| C3 | `Transform_PropertyMovesFromOverflowToMain` | Property returns from overflow to main | Overflow row loses column, main gains it |
| C4 | `Transform_MixedSameTableAndCrossTable` | Some columns swap within table, others cross | Both handled for same class |
| C5 | `Transform_OverflowWithCustomIdColumn` | Table uses ECInstanceIdColumn custom name | PK correctly used in synthetic row |
| C6 | `Transform_DeleteWithOverflowRow` | DELETE operation for class with overflow | Synthetic DELETE for overflow table |
| C7 | `Transform_UpdateOnlyOverflowColumns` | UPDATE touches only columns that moved to overflow | Only overflow UPDATE generated, primary row unchanged |

#### Category D: Multiple Classes in Same Table

| # | Test Case | Description | Key Verification |
|---|---|---|---|
| D1 | `Transform_TwoClassesDifferentRemaps` | Class A and B in same table with different column remaps | Each row remapped per its own class rules |
| D2 | `Transform_OnlyOneClassNeedsRemap` | Class A remaps, Class B unchanged | B's rows pass through unchanged |
| D3 | `Transform_DerivedClassInheritsRemap` | Base property remaps, derived class instances also affected | Derived rows correctly use inherited mapping |
| D4 | `Transform_SiblingClassesOppositeMaps` | Siblings A,B share columns but in different positions | Each sibling's remap applied correctly |

#### Category E: Operation-Specific Edge Cases

| # | Test Case | Description | Key Verification |
|---|---|---|---|
| E1 | `Transform_InsertAllNullPropertiesExceptPK` | INSERT where all properties are NULL | No data loss; NULL values at correct ordinals |
| E2 | `Transform_UpdatePartialColumns` | UPDATE modifies only 1 of 3 remapped columns | Only modified columns written; others left undefined |
| E3 | `Transform_UpdatePrimaryKeyStable` | UPDATE with column swap | PK (ECInstanceId) never changes position |
| E4 | `Transform_DeleteFullRow` | DELETE with all columns present | All values at correct new ordinals |
| E5 | `Transform_UpdateECClassIdColumn` | UPDATE where ECClassId value is remapped | ClassId value replaced with new ID |
| E6 | `Transform_InsertWithBinaryBlob` | INSERT with geometry/binary property that moved | Binary data preserved after column move |
| E7 | `Transform_InsertWithLargeText` | INSERT with large text property that moved | Text data integrity preserved |

#### Category F: Hierarchy & Schema Structure Scenarios

| # | Test Case | Description | Key Verification |
|---|---|---|---|
| F1 | `Transform_InsertBaseClassInMiddle` | New intermediate class causes column remap | Derived class changeset correctly transformed |
| F2 | `Transform_MixinPropertyRemap` | Mixin property moves columns | Mixin implementing classes' changesets transformed |
| F3 | `Transform_MultiSchemaImportRemap` | Remap triggered by cross-schema base class injection | Classes in secondary schema correctly handled |
| F4 | `Transform_DeepHierarchy5Levels` | Property moved up 5 levels | Leaf class changeset transformed |
| F5 | `Transform_RelationshipClassPropertyRemap` | Property on relationship class moves column | Relationship link table changeset transformed |
| F6 | `Transform_RelationshipEndpointClassIdRemap` | SourceECClassId/TargetECClassId values remapped | Endpoint class IDs in link table updated |

#### Category G: Combined Transforms

| # | Test Case | Description | Key Verification |
|---|---|---|---|
| G1 | `Transform_ClassIdRemapPlusColumnSwap` | Same class has both ID remap and column swap | Both applied in single pass |
| G2 | `Transform_ClassIdRemapPlusOverflow` | Class ID changes AND overflow table created | New classId in primary + overflow rows |
| G3 | `Transform_ColumnSwapPlusOverflow` | Some columns swap same-table, others move to overflow | Both handled together |
| G4 | `Transform_AllThreeTransformTypes` | ClassId remap + column swap + overflow table | Full combo in single changeset |
| G5 | `Transform_MultipleChangesetRowsMixed` | Changeset with INSERTs + UPDATEs + DELETEs mixed | All operations transformed correctly |

#### Category H: Error & Edge Conditions

| # | Test Case | Description | Key Verification |
|---|---|---|---|
| H1 | `Transform_EmptyChangeset` | Transform called on empty changeset | Returns SUCCESS with empty output |
| H2 | `Transform_ChangesetWithUnknownTable` | Changeset references table not in ec_Table | Row passes through unchanged |
| H3 | `Transform_ClassIdNotInRemapMap` | Row has ECClassId not matching any remap | Row passes through unchanged |
| H4 | `Transform_SourceChangesetMoreColumnsThanTarget` | Source has more columns than target db knows | Extra columns handled gracefully |
| H5 | `Transform_NoECClassIdColumnInTable` | Table with ExclusiveRootClassId (no per-row classId) | Remap applied via exclusive root |

### 13.3 Priority Ordering

**P0 (Critical — must test before production)**:
- A1-A3 (Point2d, Point3d, NavProperty — multi-column types)
- B1-B3 (Circular swaps — data corruption risk)
- C1, C2, C6 (Cross-table moves — synthetic row generation)
- D1, D3 (Multi-class same table — wrong class data corruption risk)
- G1, G4 (Combined transforms)

**P1 (High — covers all supported remap types)**:
- A4-A7 (Array/Struct types)
- B4-B5 (Mixed/null circular)
- C3-C4, C7 (Complex cross-table)
- E1-E5 (Operation edge cases)
- F1, F5-F6 (Hierarchy + relationship)

**P2 (Good coverage)**:
- D2, D4 (Sibling/unchanged classes)
- E6-E7 (Data type preservation)
- F2-F4 (Mixin, multi-schema, deep hierarchy)
- G2-G3, G5 (Other combos)
- H1-H5 (Error/edge conditions)

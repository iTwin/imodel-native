# ChangesetSchema — Design Reference

## Overview

`ChangesetSchema` captures the **EC-level interpretation** of every row that appears in a SQLite changeset — specifically the mapping from EC class identity and EC property access strings down to the concrete SQLite tables and columns that existed **at the moment the changeset was produced**.

This snapshot travels alongside the changeset. When a peer or downstream briefcase has subsequently applied a schema upgrade, the stored `ChangesetSchema` can be diffed against the evolved ECDb to determine whether the changeset needs to be **transformed** before being replayed.

---

## What a ChangesetSchema Captures

### Scope

| Row type | Captured? | Notes |
|---|---|---|
| Entity class rows (`INSERT`, `UPDATE`, `DELETE`) | ✅ | All physical table segments (primary, overflow, joined) |
| Link-table relationship rows | ✅ | `SourceECClassId` / `TargetECClassId` recorded as classId-ref columns |
| FK end-table relationship rows | ❌ | These surface as ordinary entity rows; not separately captured |
| Non-EC rows (`be_*`, `ec_*`, `dgn_*`, …) | ❌ | Passed through unchanged by Transform |

### Key Data Structures

```
ChangesetSchema
 └─ bmap<ECClassId, ClassEntry>
      ├─ classId          (ECClassId at extraction time)
      ├─ fullName         ("SchemaName:ClassName")
      ├─ isRelationshipClass
      └─ bmap<tableName, TableSegment>
           ├─ tableName
           ├─ idColumnIndex       (position of ECInstanceId in source row)
           ├─ classIdColumnIndex  (position of ECClassId in source row; -1 if virtual)
           ├─ sourceColumnCount   (total columns in source table)
           ├─ columns             (accessString → PropertyColumnMap)
           │    └─ { accessString, tableName, columnName, sourceColumnIndex }
           └─ classIdRefCols      (columnName → ClassIdRefColumn)
                └─ { columnName, tableName, sourceColumnIndex,
                     referencedClassFullName,   ← non-empty for nav-prop RelECClassId
                     referencedClassId }        ← class ID at extraction time
```

### Example JSON (serialised via `ChangesetSchema::To`)

The schema for a single `INSERT INTO ts.Foo(Name, Age)` (entity class, single physical table) looks like:

```json
[
  {
    "classId": "0x45",
    "fullName": "TS:Foo",
    "segments": [
      {
        "table": "ts_Foo",
        "idColumnIndex": 0,
        "classIdColumnIndex": 1,
        "sourceColumnCount": 4,
        "columns": [
          { "accessString": "Name", "columnName": "ps1", "sourceColumnIndex": 2 },
          { "accessString": "Age",  "columnName": "ps2", "sourceColumnIndex": 3 }
        ]
      }
    ]
  }
]
```

A link-table relationship (`ts.AToB`, `SourceECClassId` / `TargetECClassId`) with a sealed modifier (so constraint-classId columns are non-virtual):

```json
[
  {
    "classId": "0x4a",
    "fullName": "TS:AToB",
    "isRelationshipClass": true,
    "segments": [
      {
        "table": "ts_AToB",
        "idColumnIndex": 0,
        "classIdColumnIndex": -1,
        "sourceColumnCount": 5,
        "columns": [
          { "accessString": "SourceECInstanceId", "columnName": "SourceECInstanceId", "sourceColumnIndex": 1 },
          { "accessString": "TargetECInstanceId", "columnName": "TargetECInstanceId", "sourceColumnIndex": 2 }
        ],
        "classIdRefCols": [
          { "columnName": "SourceECClassId", "sourceColumnIndex": 3 },
          { "columnName": "TargetECClassId", "sourceColumnIndex": 4 }
        ]
      }
    ]
  }
]
```

A nav-prop with `modifier=None` (relationship class can be sub-classed, so `RelECClassId` column is non-virtual):

```json
[
  {
    "classId": "0x3f",
    "fullName": "TS:Child",
    "segments": [
      {
        "table": "ts_Child",
        "idColumnIndex": 0,
        "classIdColumnIndex": 1,
        "sourceColumnCount": 5,
        "columns": [
          { "accessString": "Val",             "columnName": "ps1", "sourceColumnIndex": 2 },
          { "accessString": "Parent.Id",       "columnName": "ps2", "sourceColumnIndex": 3 }
        ],
        "classIdRefCols": [
          {
            "columnName": "ps3",
            "sourceColumnIndex": 4,
            "referencedClassFullName": "TS:Rel",
            "referencedClassId": "0x41"
          }
        ]
      }
    ]
  }
]
```

An entity with a joined overflow table (TPH + `JoinedTablePerDirectSubclass` + `ShareColumns`):

```json
[
  {
    "classId": "0x5c",
    "fullName": "TestSchema:Building",
    "segments": [
      {
        "table": "ts_Element",
        "idColumnIndex": 0,
        "classIdColumnIndex": 1,
        "sourceColumnCount": 2,
        "columns": []
      },
      {
        "table": "ts_GeometricElement3d",
        "idColumnIndex": 0,
        "classIdColumnIndex": -1,
        "sourceColumnCount": 2,
        "columns": [
          { "accessString": "Prop1", "columnName": "js1", "sourceColumnIndex": 1 }
        ]
      }
    ]
  }
]
```

---

## How Diff Works (`ChangesetSchemaDiff::Compute`)

`Compute(source, current)` walks every `ClassEntry` in the stored `ChangesetSchema` and compares it against the live ECDb mapping. It produces a list of `ClassDiff` records, each containing zero or more `ColumnDiff` entries.

### Diff Change Kinds

| Kind | Meaning | Transformable? |
|---|---|---|
| `ClassIdRemapped` | Same `fullName`, different `ECClassId` (schema upgrade changed the class's internal ID) | ✅ |
| `ColumnRemapped` | Same access string and table, different SQLite column name (shared-column slot changed) | ✅ |
| `ColumnMoved` | Same access string, different SQLite table (e.g. property moved to overflow table) | ✅ (complex) |
| `PropertyLost` | Access string no longer present in the target ECDb at all | ❌ |
| `ClassLost` | Class `fullName` no longer present in the target ECDb at all | ❌ |

A diff also records:

- **`classIdRefDiffs`** — per nav-prop `RelECClassId` column: if the relationship class was remapped, stores `(oldClassId → newClassId)` so Transform can rewrite raw integer values.
- **`hasConstraintClassIdCols`** — true for link-table entries; triggers rewriting of `SourceECClassId` / `TargetECClassId` using the entity class remap table.
- **`newOverflowTableNames`** — overflow tables that were created by the upgrade but were absent from the source changeset. Transform synthesises stub rows in these tables.

### Example: Column remapped after schema upgrade

Schema v1 — `A.PropA` and `B.PropB` both share column `ps1` in `ts_MyBaseClass`:

```
source schema: { TS:A → ts_MyBaseClass.ps1 (A.PropA) }
```

Schema v2 inserts `NewBase` between `MyBaseClass` and `A`/`B`. The upgrade assigns `NewBase.PropBase` to `ps2`. Existing assignments are **preserved** (`A.PropA` stays at `ps1`).

```
diff: [] (empty — no change for A.PropA)
```

Another upgrade scenario where `A.PropA` is forced to move from `ps1` to `ps3`:

```
diff: [
  ClassDiff {
    fullName: "TS:A",
    targetClassId: 0x45,
    columnDiffs: [
      { kind: ColumnRemapped, accessString: "PropA", oldMap: {ts_MyBaseClass, ps1}, newMap: {ts_MyBaseClass, ps3} }
    ]
  }
]
```

---

## How Transform Works (`ChangesetSchemaDiff::Transform`)

`Transform(source, target, targetDb)` rewrites every EC-mapped row in `source` so it matches `targetDb`'s current physical layout. Non-EC rows are copied verbatim.

### Two-Phase Processing

**Phase 1 — Capture source rows**

Walks the raw SQLite changeset. For each EC-mapped table row:
- Reads every column value by its `sourceColumnIndex` (no schema lookup needed at runtime).
- Groups rows by `(classId, instanceId)` into an `InstanceKey → [SourceTableRow]` map.
- Captures classId-ref column values separately for later remapping.

**Phase 2 — Write transformed rows**

For each captured instance:
1. **Remap ECClassId** — if `ClassIdRemapped` diff is present, rewrite `ECClassId` values in `INSERT`/`DELETE` rows. (UPDATE rows omit `ECClassId` in SQLite changesets — they are left without a classId column even after transform.)
2. **Remap column names** — for `ColumnRemapped` diffs, bind the value to the new column name in the target table layout.
3. **Handle ColumnMoved** — value needs to go to a different physical table. If a target-table row already exists in the group, the value is added to it; otherwise a new synthetic row is emitted for the destination table.
4. **Remap classId-ref columns** — nav-prop `RelECClassId` columns: if `ClassIdRefDiff` matches, replace `oldClassId` integer with `newClassId`. Link-table `SourceECClassId` / `TargetECClassId`: apply the general entity `classIdRemap` table.
5. **Synthesise overflow rows** — for each `newOverflowTableName` in the diff, emit a stub `INSERT` (or `DELETE`) row containing only `ECInstanceId` (and `ECClassId` if the column exists in the overflow table). This ensures replaying a pre-upgrade `INSERT` changeset leaves the DB in the same state as a live insert after the upgrade.

---

## Cases Handled by Transform

| Scenario | Handled |
|---|---|
| Class internal ID changed (`ClassIdRemapped`) | ✅ Rewrites `ECClassId` column values in INSERT/DELETE rows |
| Shared column renaming (`ColumnRemapped`) | ✅ Binds value to new column name using target layout |
| Property moved to another table (`ColumnMoved`) | ✅ Synthesises/merges rows across source and destination tables |
| Nav-prop `RelECClassId` remapped | ✅ Per-column `ClassIdRefDiff` rewrites the integer value |
| Link-table `SourceECClassId` / `TargetECClassId` remapped | ✅ General entity `classIdRemap` table applied |
| New overflow table created by upgrade (INSERT replay) | ✅ Stub INSERT row synthesised in overflow table |
| New overflow table created by upgrade (DELETE replay) | ✅ Stub DELETE row synthesised to remove overflow row |
| UPDATE with column remap | ✅ Rebinds value to new column name (ECClassId is intentionally absent in UPDATE rows) |
| Non-EC rows (`be_*`, `ec_*`, …) | ✅ Passed through unchanged |
| Multiple physical tables per instance (joined, overflow) | ✅ All table segments processed as a group per `(classId, instanceId)` |

## Cases NOT Handled by Transform

| Scenario | Why not handled |
|---|---|
| `PropertyLost` — property existed in source but is absent from target | Semantic information loss; Transform returns `BE_SQLITE_ERROR` |
| `ClassLost` — class existed in source but is absent from target | Cannot map rows to a class that no longer exists; `IsTransformable()` returns false |
| Data-type changes on a property | ECDb schema upgrades do not change property types; this case cannot occur in practice |
| Cross-briefcase instance ID conflicts | Transform does not renumber instance IDs; callers must ensure the target DB has compatible ID sequences |
| Reordering of rows | SQLite changesets are ordered by table, then by PK; Transform preserves that order |

---

## Pull / Push Flow: When Transform Is Needed

The sequence below shows two users working in separate briefcases. **Transform is only required when the incoming changeset was produced against a schema version that differs from the local schema.**

```
Actor Key
──────────
U1  = User 1 (briefcase b1)
U2  = User 2 (briefcase b2)
HUB = iModelHub (central changeset store)
```

```
U1 (b1)                          HUB                          U2 (b2)
   │                              │                              │
   │── ImportSchemas(v2) ────────►│                              │
   │   (local txn: schema cs)     │                              │
   │                              │                              │── ImportSchemas(v1) ──►
   │                              │                              │   (local txn: schema cs)
   │                              │                              │
   │── INSERT ts.Foo(Name='hi') ─►│                              │
   │   ChangesetSchema extracted  │                              │
   │   and stored alongside cs    │                              │
   │                              │                              │
   │── PushChangeset(cs + meta) ─►│                              │
   │                              │◄─── PullChangesets ──────────│
   │                              │                              │
   │                              │   For each incoming cs:      │
   │                              │   ┌──────────────────────────┤
   │                              │   │ 1. Load stored           │
   │                              │   │    ChangesetSchema       │
   │                              │   │                          │
   │                              │   │ 2. Diff against local    │
   │                              │   │    ECDb state            │
   │                              │   │                          │
   │                              │   │ 3. Transform needed?     │
   │                              │   └──────────────────────────┤
   │                              │                              │
   │                              │            ┌─────────────────┤ No diff (same schema)
   │                              │            │  ApplyChanges   │ → Apply directly
   │                              │            │  (raw cs)       │
   │                              │            └─────────────────┤
   │                              │                              │
   │                              │            ┌─────────────────┤ Diff is transformable
   │                              │            │ Transform(cs)   │ → Transform then Apply
   │                              │            │ ApplyChanges    │
   │                              │            │ (transformed)   │
   │                              │            └─────────────────┤
   │                              │                              │
   │                              │            ┌─────────────────┤ Diff NOT transformable
   │                              │            │ ERROR: cannot   │ (PropertyLost / ClassLost)
   │                              │            │ apply cs        │ → Pull blocked; manual
   │                              │            └─────────────────┤   resolution needed
   │                              │                              │
```

### Decision Logic on Pull

```
PullChangeset(cs):
  storedSchema  = ChangesetSchema::From(cs.attachedSchemaJson)
  diff          = ChangesetSchemaDiff::Compute(storedSchema, localECDb)

  if diff.GetClassDiffs().empty():
    // Schema is identical — no transform needed
    cs.ApplyChanges(localECDb)

  elif diff.IsTransformable():
    // Schema evolved but all changes are mechanical remappings
    transformedCs = ChangeSet()
    diff.Transform(cs, transformedCs, localECDb)
    transformedCs.ApplyChanges(localECDb)

  else:
    // PropertyLost or ClassLost — cannot replay without data loss
    FAIL("incoming changeset is incompatible with local schema")
```

### When Transform Is Triggered

Transform is needed **if and only if** the incoming changeset's embedded `ChangesetSchema` differs from the local ECDb's current mapping:

| Condition | Transform needed? |
|---|---|
| Both briefcases on identical schema version | ❌ — diff is empty, apply directly |
| Local briefcase applied additional schema upgrade (new classes/properties, no deletions) | ✅ — `ClassIdRemapped`, `ColumnRemapped`, `ColumnMoved`, or `newOverflowTableNames` diffs present |
| Local schema is an older version than the incoming changeset | ❌ — the incoming schema matches the local schema at time of push; incoming CS was produced against older schema, local schema is newer → same as above |
| Either side deleted a property or class (schema downgrade) | ❌ (pull blocked) — `PropertyLost` / `ClassLost`; not transformable |

### Concrete Example

```
b1 (User 1)                                   b2 (User 2)
─────────────────────────────────────────────────────────────────
Schema v1: Base → A {PropA: ps1}, B {PropB: ps1}

Both cloned from same seed → identical schema v1
                                                ↓
b2 applies schema upgrade v2:
  Adds NewBase between Base and A/B.
  NewBase.PropBase → ps2.
  A.PropA stays at ps1.    ← no column remap for A

b1 (still on v1) inserts A instance:
  Raw CS touches ts_MyBaseClass.ps1 = "hello"
  ChangesetSchema stored: { TS:A → ts_MyBaseClass.ps1 }

b1 pushes CS to hub.

b2 pulls CS:
  diff = Compute(storedSchema, b2)
  → diff.GetClassDiffs() is EMPTY (A.PropA was not remapped by the upgrade)
  → no transform needed
  → ApplyChanges directly
```

If instead the upgrade **had** remapped `A.PropA` to `ps3`:

```
diff contains:
  ClassDiff { TS:A, columnDiffs: [ColumnRemapped: PropA ps1→ps3] }

diff.IsTransformable() == true
  → Transform rewrites the raw CS: column index for PropA points to ps3 slot
  → transformedCs.ApplyChanges(b2) succeeds
```

---

## API Summary

```cpp
// 1. Producer side: extract and store alongside the changeset
ChangesetSchema schema = ChangesetSchema::ExtractFrom(ecdb, changeStream);
BeJsDocument doc;
schema.To(doc);
// … serialize doc.Stringify() into changeset metadata …

// 2. Consumer side: load from metadata
ChangesetSchema stored = ChangesetSchema::From(doc);

// 3. Diff against local ECDb
ChangesetSchemaDiff diff = ChangesetSchemaDiff::Compute(stored, localECDb);

// 4. Decide and act
if (diff.GetClassDiffs().empty()) {
    cs.ApplyChanges(localECDb);                    // no transform needed
} else if (diff.IsTransformable()) {
    ChangeSet transformed;
    diff.Transform(cs, transformed, localECDb);    // rewrite to local layout
    transformed.ApplyChanges(localECDb);
} else {
    // PropertyLost or ClassLost — cannot proceed
}

// Optional: validate a fresh stream against a stored schema
bvector<Utf8String> errors;
schema.Validate(ecdb, freshStream, &errors);
```
{
  // list of all the ClassIds used in changeset this is used to check if old and new id is same in diff or classes are missing and in transform update classid from old to new. These id can be in ECClassId, SourceECClassId, TargetECClassId, RelClassId.
  refClasses :[{"className": "BisCore:Element", "id": "0x10000"}, ...]
  // represent classes that has instances part of 
  refClassDefs :[{
    "id": "0x100000",
      "segments": [
          "tableName": "bis_Element"
          // this will include NavProperties as well. it include only really property and no virtual props.
          { "accessString": "Prop1.X", "columnName": "js1", "sourceColumnIndex": 1 }
          { "accessString": "Parent.RelClassId", "columnName": "js12, "sourceColumnIndex": 2}
        ]
  }]

}
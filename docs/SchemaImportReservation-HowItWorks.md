# Schema Import Reservation — How It Works

**File:** `iModelCore/ECDb/ECDb/SchemaSync.cpp`  
**Key functions:** `SchemaSync::ReserveSchemaImport`, `SchemaSync::SeedReservationStoreInternal`

---

## The Problem Being Solved

In a multi-briefcase environment, several users can try to import new EC schemas into the same iModel at roughly the same time. When ECDb imports a schema it assigns integer IDs to EC objects — every `ec_Schema`, `ec_Class`, `ec_Property`, `ec_Column`, `ec_Table`, `ec_Index`, etc. gets a row with a numeric primary key.

If briefcase A assigns `ec_Class.Id = 5` to `MyDomain:Asset` and briefcase B also assigns `ec_Class.Id = 5` to `MyDomain:Building`, the two sets of metadata are mutually incompatible. When either briefcase pushes its schema changes to the shared sync-db, the other briefcase cannot apply those changes without ID collisions.

**Reservation** solves this by having every briefcase, *before* it starts the local import, contact the shared sync-db and claim a block of IDs for every EC object it is about to create. Once claimed, those IDs are permanently associated with those EC objects for every briefcase in the container.

---

## High-Level Architecture

```
┌────────────────────────────────────────────────┐
│  Shared Sync-DB  (single source of truth)       │
│                                                 │
│  schema_reservation_ids          ← id store     │
│  schema_reservation_columns      ← col store    │
│  schema_reservation_class_hierarchy ← hierarchy │
└────────────────────────────────────────────────┘
         ▲            ▲
         │            │
  ReserveSchemaImport  │  SeedReservationStoreInternal
  (called by briefcase │  (called once at Init)
  before import)       │
                  local db baseline
```

There are **three distinct stores** written into the sync-db:

| Sync-DB Table | What it holds |
|---|---|
| `schema_reservation_ids` | 22 per-EC-table counters + key→id maps |
| `schema_reservation_columns` | Per-physical-table property→column-ordinal maps |
| `schema_reservation_class_hierarchy` | Transitive ancestor+descendant sets per class |

---

## Part 1 — Seeding (happens once at `Init`)

When a container is first created (`SchemaSync::Init`), the reservation stores do not exist yet. The seeding step is the very first population, capturing the baseline state of the container so that future reservations start from correct counter values.

### Entry point

```cpp
SchemaSync::Status SchemaSync::SeedReservationStoreInternal(SyncDbUri const& syncDbUri)
```

This is called at the end of `Init`, right after the initial Push of schema metadata into the sync-db.

### What seeding does — step by step

#### Step 1 — Open the sync-db read-write

```cpp
m_pendingReservationDb.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams)
```

The three reservation tables are created with `CREATE TABLE IF NOT EXISTS`:

```sql
schema_reservation_ids         (TableName TEXT PK, LastReservedId INTEGER, KeyMap BLOB)
schema_reservation_columns     (PhysicalTableName TEXT PK, KeyMap BLOB)
schema_reservation_class_hierarchy (ClassKey TEXT PK, Ancestors BLOB, Descendants BLOB)
```

#### Step 2 — Build the id-reservation store from the local db

```cpp
SchemaReservationHelper::SeedReservationStoreFromLocalDb(m_conn, resStore)
```

This is a three-phase operation:

**Phase A — Walk every schema in the local db** (`SeedSchemaFromLocalDb`)

The function iterates every schema that is already persisted in the local ECDb, and for every object that has an ID in the local db it records the `key → id` mapping:

```
ec_Schema       : key = DeriveSchemaKey(schema)               → schema.GetId()
ec_SchemaRef    : key = schema:refSchema                       → looked up from ec_SchemaReference
ec_Class        : key = schema:className                       → class.GetId()
ec_ClassHasBase : key = class:baseClass                        → looked up from ec_ClassHasBaseClasses
ec_Property     : key = schema:class:prop                      → prop.GetId()
ec_Enumeration  : key = ...                                    → enum.GetId()
ec_KindOfQuantity, ec_UnitSystem, ec_Phenomenon, ec_Unit, ec_Format, ec_FormatCompositeUnit
ec_PropertyCategory
ec_RelationshipConstraint       : key = rel:end                → looked up
ec_RelationshipConstraintClass  : key = rel:end:constraintClass → looked up
ec_CustomAttribute              : key = container:caClass      → looked up
```

Each `key → id` pair is inserted into the appropriate `SchemaReservationTableStore` inside `SchemaReservationStore`.

**Phase B — Seed the counters from MAX(Id)** (`SeedLastReservedIdsFromLocalDb`)

After all key→id pairs are loaded, the counter (`LastReservedId`) of each per-table store is set to the actual maximum ID that already exists in that ec_* table:

```sql
SELECT COALESCE(MAX(Id),0) FROM [main].[ec_Schema]
SELECT COALESCE(MAX(Id),0) FROM [main].[ec_Class]
-- ... repeated for all 22 tables
```

This guarantees that the next `GetOrAllocate()` call will produce `MAX(Id) + 1`, never repeating an ID that's already taken.

**Phase C — Seed the mapping-table key maps** (`SeedMappingTableKeyMapsFromLocalDb`)

Six additional tables — `ec_Table`, `ec_Column` (system columns only), `ec_PropertyPath`, `ec_PropertyMap`, `ec_Index`, `ec_IndexColumn` — do not appear in the schema-graph walk. They are populated here via direct SQL:

```sql
-- ec_Table: every non-virtual physical table
SELECT [Name], [Id] FROM [ec_Table] WHERE [Type] != <Virtual>
-- key: "main:tableName"

-- ec_Column (system): ECInstanceId + ECClassId columns per table
-- key: "main:tableName:ECInstanceId" / "main:tableName:ECClassId"

-- ec_Column (link-table constraint cols): SourceECInstanceId etc.
-- key: "main:tableName:SourceECInstanceId" etc.

-- ec_PropertyPath: one row per root property per declaring class
-- key: "schema:declaringClass:rootPropName:accessString"

-- ec_PropertyMap: concrete mapping of a property to a column+table
-- key: "schema:mappedClass:accessString:main:tableName"

-- ec_Index: one per table+index combination
-- key: "tableName:indexName"

-- ec_IndexColumn: one per index column
-- key: "tableName:indexName:ordinal"
```

#### Step 3 — Build the column store from the local db

```cpp
SchemaReservationHelper::SeedColumnStoreFromLocalDb(m_conn, resStore, colStore)
```

This has a single sub-step (the per-declaring-class high-water ordinals are derived automatically, see below).

**Seed the property-key→column maps** (`SeedColumnKeyMapsFromLocalDb`)

For every property-to-column mapping that already exists in `ec_PropertyMap`, record which ordinal and column ID that property uses:

```sql
-- Non-navigation property columns in primary/overflow tables
SELECT schema, class, accessString, tableName, ordinal, columnId
FROM ec_PropertyMap JOIN ec_PropertyPath JOIN ec_Property JOIN ec_Class
     JOIN ec_Schema JOIN ec_Column JOIN ec_Table
WHERE [Type] IN (<Primary>, <Overflow>) AND kind != 4

-- Navigation property FK columns
SELECT schema, class, accessString, columnId
FROM ec_PropertyMap JOIN ... WHERE kind = 4 AND columnKind = <Default>
```

Each result becomes a `SchemaReservationColumnEntry { columnOrd, columnId }` in the `SchemaReservationColumnTableStore` for the relevant physical table.

**No per-table high-water floor is seeded.** Earlier versions seeded a single per-table high-water = `MAX(ec_Column.Ordinal)` over all `SharedData` columns and used it as a floor. That was removed: it introduced false positives (orphan/unmapped shared columns inflated the floor, hiding reusable columns and wasting ordinal space), while the reuse rule is simply *"a shared column can be reused by a class's property iff no ancestor or descendant of that class has a property on that column."* The per-table high-water used to place a **new** shared column is now derived purely from the reserved entries (`max(columnOrd)`).

**Per-declaring-class high-water is seeded implicitly.** The first (non-navigation) query above filters `pm.ClassId = root_prop.ClassId`, so it only records a column against its **declaring** class. `SchemaReservationColumnTableStore::AddEntry` raises that class's high-water ordinal (`m_classHighWater[schema:class]`) to the entry's ordinal. So seeding the key map also seeds the declared-only per-class high-water — the highest shared ordinal each class has itself declared into. This map is persisted (in the `ClassHighWater` blob) and consumed during reservation (Part 2, Phase 4).

#### Step 4 — Build the class hierarchy store from the local db

```cpp
SchemaReservationHelper::SeedClassHierarchyStoreFromLocalDb(m_conn, hierarchyStore)
```

This computes transitive ancestor/descendant sets for every class already in the db, using a depth-first search over `ec_ClassHasBaseClasses`:

```sql
SELECT s1.Name, c1.Name, s2.Name, c2.Name
FROM ec_ClassHasBaseClasses chbc
JOIN ec_Class c1 ON c1.Id = chbc.ClassId
JOIN ec_Schema s1 ON s1.Id = c1.SchemaId
JOIN ec_Class c2 ON c2.Id = chbc.BaseClassId
JOIN ec_Schema s2 ON s2.Id = c2.SchemaId
```

For each class, a memoized DFS walks the direct-parent map upward to collect all transitive ancestors. Then `RecordClass(classKey, ancestors)` is called, which also populates the `descendants` set of each ancestor entry.

The result is a table like:

```
ClassKey           Ancestors                        Descendants
BisCore:Element    {}                               {BisCore:GeometricElement2d, ...}
BisCore:GeometricElement2d  {BisCore:Element}       {MyDomain:MyWidget, ...}
MyDomain:MyWidget  {BisCore:GeometricElement2d, BisCore:Element}  {}
```

#### Step 5 — Write everything to the sync-db and commit

```cpp
SchemaReservationHelper::WriteReservationStoreToSyncDb(m_pendingReservationDb, resStore)
SchemaReservationHelper::WriteColumnStoreToSyncDb(m_pendingReservationDb, colStore)
SchemaReservationHelper::WriteClassHierarchyStoreToSyncDb(m_pendingReservationDb, hierarchyStore)
CommitPendingReservation()  // calls SaveChanges and closes the db
```

Each `SchemaReservationTableStore` is serialized as:
- `LastReservedId` — a plain `INTEGER`
- `KeyMap` — a [FlexBuffers](https://google.github.io/flatbuffers/flexbuffers.html) binary map of `{ "key": id, ... }`

The column store row (`schema_reservation_columns`, one per physical table) stores two FlexBuffers maps:
- `KeyMap` — `{ "schema:class:prop": [columnOrd, columnId], ... }` (a map of vectors).
- `ClassHighWater` — `{ "schema:class": ordinal, ... }`, the declared-only per-class high-water ordinal for this physical table. It is reconstructable from `KeyMap` via `AddEntry` but persisted explicitly.

The class hierarchy store stores `Ancestors` and `Descendants` each as a FlexBuffers vector of strings.

---

## Part 2 — Reservation (happens before every schema import)

### Entry point

```cpp
SchemaSync::Status SchemaSync::ReserveSchemaImport(
    bvector<ECN::ECSchemaCP> const& schemas,
    SyncDbUri const& syncDbUri)
```

Called by the import pipeline *before* the local `ImportSchemas` begins. The caller holds the schemas in memory (already parsed) but has not yet written a single byte of EC metadata to the local db.

### The `m_pendingReservationDb` connection

The method opens a private read-write connection to the sync-db stored in `m_pendingReservationDb`. All changes accumulate as an in-progress SQLite write transaction. **`SaveChanges` is intentionally not called here** — the caller uses `ReservationTxGuard` to commit or roll back atomically when the import succeeds or fails:

```cpp
// usage pattern in the importer:
SchemaSync::ReservationTxGuard guard(schemaSync);
schemaSync.ReserveSchemaImport(schemas, syncDbUri);
// ... do the actual local ImportSchemas ...
guard.Commit();  // only on success
// guard destructor calls AbandonPendingReservation on failure
```

### The eight-phase walk pipeline

#### Phase 1 — Load the existing reservation store

```cpp
SchemaReservationHelper::LoadReservationStoreFromSyncDb(m_pendingReservationDb, store)
```

All 22 per-table entries from `schema_reservation_ids` are read into the in-memory `SchemaReservationStore`. The `LastReservedId` counters resume from where seeding (or the last reservation) left off.

#### Phase 2 — Walk schemas for EC-object ID reservation (`WalkSchemaForReservation`)

For every schema in the import (and its full transitive reference closure, to handle dependencies), `GetOrAllocate` is called for each EC object:

```
schema.GetOrAllocate("BisCore")           → e.g. returns 3 (already in store)
ecClass.GetOrAllocate("BisCore:Asset")    → e.g. returns 47 (already in store)
ecClass.GetOrAllocate("MyDomain:Widget")  → counter was 47, now returns 48 (new!)
property.GetOrAllocate("MyDomain:Widget:Name")  → counter was 48, now returns 312 (new!)
```

`GetOrAllocate` is idempotent for anything already in the store — it just returns the existing ID. Only genuinely new EC objects get new IDs. The reference closure ensures that base classes from other schemas are also in the store before derived classes are processed.

This phase covers all 16 "metadata" tables:
`ec_Schema`, `ec_SchemaReference`, `ec_Class`, `ec_ClassHasBaseClasses`, `ec_Property`,
`ec_Enumeration`, `ec_KindOfQuantity`, `ec_UnitSystem`, `ec_Phenomenon`, `ec_Unit`,
`ec_Format`, `ec_FormatCompositeUnit`, `ec_PropertyCategory`,
`ec_RelationshipConstraint`, `ec_RelationshipConstraintClass`, `ec_CustomAttribute`

#### Phase 3 — Update the class hierarchy store (`PopulateClassHierarchyStore`)

The class hierarchy store must be updated *before* the column walk, because the column walk uses it to decide whether shared-column slots can be reused.

For each new class in the import:
1. Its direct bases' keys are collected.
2. The existing ancestor sets of those bases are pulled from the store.
3. `RecordClass(classKey, allTransitiveAncestors)` is called, which also adds this class to the `descendants` of each ancestor.

The store now knows the full inheritance tree including classes reserved in this import even before any local import has occurred.

#### Phase 4 — Walk schemas for shared-column slot reservation (`WalkSchemaForColumnReservation`)

This is the most complex phase. It handles the `ShareColumns` custom attribute strategy where multiple sibling classes share the same physical SQLite column.

**For each entity/mixin class that uses `ShareColumns`:**

a) **Derive the primary table name.** The logic mirrors ECDb's own mapper:
   - If the class is a joined-table root (its direct base has `JoinedTablePerDirectSubclass` CA), use its own table name.
   - If the class is in a TablePerHierarchy chain, find the TPH root and use that root's table name.
   - Otherwise use the class's own table name.

b) **Determine the overflow table name**: `primaryTableName + "_Overflow"`.

c) **For each owned non-navigation property without an explicit column name:**

   - Collect leaf access strings (e.g. a struct property `Dimensions` might expand to `Dimensions.Width`, `Dimensions.Height`).
   - Compute leaf keys: `"SchemaName:ClassName:Dimensions.Width"`.
   - Check if already reserved — skip if so.
   - Decide primary vs overflow: mirrors the budget logic in `ClassMapColumnFactory::EvaluateOverflowFromBudget`:
     ```
     availablePhysicalColumns = kMaxPhysicalColumnsPerTable - (highWater + 1)
     reusableSharedColumnCount = count of slots no occupant shares a path with this class
     → overflow if needed
     ```
   - **Try to reuse an existing slot**: Scan the chosen table's slots **starting strictly above this class's high-water ordinal** (`GetClassHighWaterOrd(schema:class)`). Columns at or below that mark were already checked when the class allocated its earlier properties — they are occupied by the class itself or an ancestor and can never be reused by it, so they are skipped. A slot above the mark is reusable if no current occupant is the class, an ancestor, or a descendant of it in `hierarchyStore`. If a slot is reusable:
     ```
     entry.columnOrd = existingSlot.columnOrd   // SAME physical column!
     entry.columnId  = existingSlot.columnId    // SAME ec_Column.Id!
     ```
   - **Allocate a new slot** if nothing can be reused:
     ```
     entry.columnOrd = highWater + 1            // next ordinal (per-table high-water, derived from entries)
     entry.columnId  = idStore.column.GetOrAllocate(leafKey)  // new ec_Column id
     ```
   - Either way `AddEntry` records the leaf and raises the class's high-water ordinal to the chosen
     ordinal, so the next leaf/property of the same class resumes scanning above it.

**For non-shared-column classes:** `ReserveNonSharedColumnIds` is called to still allocate `ec_Column` IDs for dedicated/named columns and navigation FK columns.

##### Where do the "skipped" classes get their column IDs?

A common point of confusion: the walk appears to `continue` (skip) classes that don't use shared columns, so where are *their* data columns reserved? The answer is in the **ordering** of the per-class loop:

```
1. skip relationship / CA / struct classes            → continue
2. skip NotMapped / ExistingTable map strategy         → continue
3. skip classes with no owned properties               → continue
4. ReserveNonSharedColumnIds(idStore, *ecClass)        ← runs for every class that gets here
5. if (!ClassUsesSharedColumns(...)) continue;         ← the "skip"
6. (shared-column classes only) shared-slot placement
```

`ReserveNonSharedColumnIds` on **step 4 runs before the `continue` on step 5**. So a non-shared class already had *all* its leaf column IDs reserved by the time it is skipped — the `continue` only bypasses the shared-column **slot-placement** machinery (primary/overflow budget, slot reuse), which is meaningless for a class whose columns are each dedicated.

Inside `ReserveNonSharedColumnIds` the guard is:

```cpp
if (usesShared && !prop->GetIsNavigation() && !PropertyHasExplicitColumnName(*prop))
    continue; // shared-column walk already covers non-nav, non-explicit-name leaves
```

When `usesShared` is false nothing is skipped, so **every** leaf (data + nav + explicit-name) gets `idStore.column.GetOrAllocate(...)`. When `usesShared` is true, only the leaves that the shared-column walk does *not* handle (navigation FK columns and explicitly-named columns) are reserved here; the shared data leaves are left to the slot-placement logic on step 6.

The classes skipped **entirely** at steps 1–3 have their reservations elsewhere:

| Skipped at | Where its columns are reserved instead |
|---|---|
| Relationship (link-table) class | Phase 8 — `WalkSchemaForRelationshipReservation` |
| `NotMapped` strategy | Nowhere — intentionally maps to no table |
| `ExistingTable` strategy | Nowhere — maps onto a pre-existing table, import creates no columns |
| No owned properties | Nothing to reserve; inherited leaves are reserved by the class that *declares* them |
| System columns (`ECInstanceId`/`ECClassId`) | Phase 7 — `ReserveEntityTableSystemColumnIds` |

#### Phase 5 — Reserve ec_Table IDs for physical tables discovered by the column walk

After the column walk, `colStore` knows the names of every physical table (primary and overflow) that will exist after this import. For each:

```cpp
store.ecTable.GetOrAllocate("main:" + physicalTableName)
```

This runs before the mapping walk so the mapping walk can check `HasKey` for overflow tables.

#### Phase 6 — Walk schemas for mapping-table ID reservation (`WalkSchemaForMappingReservation`)

For every concrete entity/mixin class:

- `ec_Table` ID for the primary table (idempotent, already done above mostly).
- For each property (owned + inherited via `GetProperties(true)`):
  - `ec_PropertyPath` — keyed by the *declaring* class. Shared across all subclasses that inherit the property.
  - `ec_PropertyMap` — keyed by the *concrete mapped* class + access string + physical table. Each concrete class gets its own row.
  - The physical table (primary vs overflow) is resolved from `colStore`, not recomputed — keeping the column walk as the single source of truth.
- `ec_PropertyMap` for system properties `ECInstanceId` and `ECClassId` on both the primary table and the overflow table (if the overflow table is in `ecTable`).

#### Phase 7 — Reserve system column IDs for entity tables (`ReserveEntityTableSystemColumnIds`)

For every entry now in `store.ecTable`, reserve two `ec_Column` IDs:
```
"main:tableName:ECInstanceId"
"main:tableName:ECClassId"
```

This runs *before* the relationship walk so link-table entries are not included.

##### Why only `ECInstanceId` and `ECClassId`?

Both this phase and Phase 6's system-property `ec_PropertyMap` reservation only handle `ECInstanceId` and `ECClassId`, even though ECDb defines more system properties (`ECDbSystemSchemaHelper.h`):

- `ECInstanceId`, `ECClassId`
- `SourceECInstanceId`, `SourceECClassId`, `TargetECInstanceId`, `TargetECClassId`
- `NavPropId` (`Id`), `NavPropRelECClassId` (`RelECClassId`)
- `PointX` / `PointY` / `PointZ`

This is correct, not a gap, because for an **entity table** the only table-level system columns are `ECInstanceId` and `ECClassId`. The remaining system properties belong to a different scope and are reserved elsewhere:

| System property | Scope | Where it is reserved |
|---|---|---|
| `Source*` / `Target*` `ECInstanceId`/`ECClassId` | Relationship **link tables** only | Phase 8 — `WalkSchemaForRelationshipReservation` (both `ec_Column` and `ec_PropertyMap`) |
| `NavPropId` (`Id`), `NavPropRelECClassId` (`RelECClassId`) | Sub-leaves of a **navigation** property | Expanded by `ClassMapColumnFactory::CollectColumnAccessStrings` and reserved through the normal property loops (`ReserveNonSharedColumnIds` for columns, `GetProperties(true)` loop for `ec_PropertyPath`/`ec_PropertyMap`) |
| `PointX` / `PointY` / `PointZ` | Leaf access strings of a **point** property | Same as nav props — expanded by `CollectColumnAccessStrings` and handled by the per-property leaf loops |

So restricting the *table-level* system reservation to `ECInstanceId`/`ECClassId` is complete for entity tables; the other system properties are covered either by the relationship walk or by per-property leaf expansion.

#### Phase 8 — Walk schemas for relationship (link-table) reservation (`WalkSchemaForRelationshipReservation`)

Link-table relationships get their own physical table. For each root relationship class that maps as a link table:

- `ec_Table` ID: `"main:linkTableName"`
- Six system `ec_Column` IDs:
  - `"main:linkTableName:ECInstanceId"`
  - `"main:linkTableName:ECClassId"`
  - `"main:linkTableName:SourceECInstanceId"`
  - `"main:linkTableName:TargetECInstanceId"`
  - `"main:linkTableName:SourceECClassId"`
  - `"main:linkTableName:TargetECClassId"`
- `ec_Column` IDs for any properties owned by the relationship.
- `ec_PropertyMap` IDs for constraint access strings and owned properties.

#### Phase 9 — Walk schemas for index reservation (`WalkSchemaForIndexReservation`)

Reserves `ec_Index` and `ec_IndexColumn` IDs for every index ECDb will auto-generate:

| Index type | Key pattern |
|---|---|
| ECClassId index on entity table | `"tableName:ix_tableName_ecclassid"` |
| ECClassId index on overflow table | `"tableName_Overflow:ix_tableName_Overflow_ecclassid"` |
| Nav-property FK index | `"tableName:ix/uix_tableName_fk_relAlias_relName_source/target"` |
| Nav-property RelECClassId index | `"tableName:ix_tableName_NavPropNameRelECClassId"` |
| Link-table ECClassId index | `"linkTableName:ix_linkTableName_ecclassid"` |
| Link-table source+target index | `"linkTableName:uix_alias_name_sourcetargetclassid"` |
| Link-table target index | `"linkTableName:ix_alias_name_target"` |
| User DbIndexList CA indexes | `"tableName:userDefinedIndexName"` |

For each index, `ReserveIndexAndColumns(idStore, tableName, indexName, columnCount)` calls:
```cpp
idStore.ecIndex.GetOrAllocate("tableName:indexName")
idStore.indexColumn.GetOrAllocate("tableName:indexName:0")
// ... up to columnCount - 1
```

### After all eight phases

The two stores are written back to the sync-db (the column store first, then the id store a second time since the mapping walk added more entries after the first write):

```cpp
SchemaReservationHelper::WriteColumnStoreToSyncDb(m_pendingReservationDb, colStore)
SchemaReservationHelper::WriteReservationStoreToSyncDb(m_pendingReservationDb, store)
// NOTE: SaveChanges NOT called here — the ReservationTxGuard controls the commit.
```

---

## How the Reserved IDs Are Consumed

When ECDb's `ImportSchemas` runs locally after `ReserveSchemaImport`, the `IdFactory` is placed into **keyed mode** via `SchemaSync::KeyedModeGuard`. In keyed mode, instead of generating the next auto-increment integer for a new EC object, the factory calls back into the reservation store:

```
"I am inserting ec_Class row for BisCore:MyNewClass — what ID should I use?"
→ store.ecClass.Lookup("BisCore:MyNewClass") → 312
→ INSERT INTO ec_Class(Id, ...) VALUES(312, ...)
```

Because every briefcase that imports `BisCore:MyNewClass` will call `ReserveSchemaImport` first and receive the same reserved ID `312`, all briefcases produce identical EC metadata. Pushes to the sync-db become conflict-free UPSERT operations.

---

## Concurrency and Atomicity

- The sync-db is a SQLite database. Write access is serialized by SQLite's file-level write lock.
- `ReserveSchemaImport` opens a write transaction but does not commit until the caller's `ReservationTxGuard::Commit()`. If the import fails, `~ReservationTxGuard()` calls `AbandonPendingReservation()` which rolls back the SQLite transaction — no IDs are permanently claimed.
- If two briefcases try to reserve at the same time, one will block on the SQLite write lock until the other commits. The second briefcase then loads the already-committed reservation from the first and extends it.

---

## Glossary of Key Terms

| Term | Meaning |
|---|---|
| **Sync-db** | The shared SQLite database that all briefcases read/write for schema coordination |
| **Local db** | The briefcase's own ECDb file |
| **Reservation store** | The in-memory + persisted structure tracking `key → reserved-id` for 22 EC tables |
| **Column store** | The per-physical-table structure tracking `propertyKey → (columnOrd, columnId)` |
| **Hierarchy store** | The per-class transitive ancestor/descendant sets used for shared-column slot reuse |
| **Keyed mode** | The `IdFactory` operating mode where IDs come from the reservation store, not auto-increment |
| **Slot reuse** | Assigning two sibling classes to the *same* physical column ordinal (because their instances never coexist in the same row) |
| **High-water ordinal** | The highest shared-column ordinal currently allocated in a physical table; new slots start above this |
| **TPH** | TablePerHierarchy — a class mapping strategy where all classes in a hierarchy share one physical table |
| **Joined table** | A sub-hierarchy whose root has a direct base with `JoinedTablePerDirectSubclass` CA |
| **Link table** | A physical SQLite table created for many-to-many relationship classes |
| **GetOrAllocate** | The core reservation operation: return existing ID for key, or assign next counter value if new |
| **PendingReservationDb** | The `m_pendingReservationDb` member holding the uncommitted write connection to the sync-db |

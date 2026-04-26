# Instance Graph Traversal — Design Plan

## Problem

`ECInstanceFinder` traverses relationships but is slow because it goes through ECSql (prepare → parse → resolve class map → generate SQL → execute). For graph operations like finding all related instances, computing graph intersection/union, or exposing a virtual table — we need a **fast path** that skips ECSql entirely and goes straight to raw SQLite prepared statements built from property maps.

## Approach

Build a new `InstanceGraph` API in ECDb that:

1. Given a seed `ECInstanceKey` (classId + instanceId), discovers all applicable relationship classes for that class
2. For each **base** relationship class (no derived — one SQL per base), generates raw SQLite SELECT statements from the property map / class map
3. Lazily prepares and caches these SQLite statements
4. Traverses relationships in BFS fashion with cycle avoidance
5. Supports set operations: overlap check, intersection, union between two `InstanceGraph`s
6. Backs an `ECDbVirtualTable` named `relations()` for ad-hoc relationship queries

## Key Insight: Why One SQL Per Base Relationship

ECDb maps relationship hierarchies using Table-Per-Hierarchy (TPH). Derived relationship classes share the same physical table as the base. A single query on the base table with an `ECClassId IN (...)` filter covers all derived classes. This avoids N separate queries for N derived classes.

---

## 1. Relationship Discovery — Which Relationships Apply to a Class?

Given an `ECClassId`, we need all relationship classes where that class (or a base class, if polymorphic) appears as a source or target constraint.

### Existing infrastructure to reuse

- **`ECInstanceFinder::FindRelationshipsOnEnd()`** — queries `ec_RelationshipConstraint` + `ec_RelationshipConstraintClass` with a recursive CTE on `ec_ClassHasBaseClasses`. Returns `(RelationshipClassId, RelationshipEnd)` pairs.
- **`LightweightCache::GetConstraintClassesForRelationshipClass()`** — given a relationship class, returns all constraint class IDs and which end (Source/Target/Both). Cached.

### New: `RelationshipDiscovery` cache

Build a cache: `ECClassId → vector<ApplicableRelationship>` where:

```cpp
struct ApplicableRelationship {
    ECN::ECRelationshipClassCP m_relClass;      // the base relationship class
    ECN::ECRelationshipEnd     m_thisEnd;        // which end "our" class sits on (Source or Target)
    ClassMap::Type             m_mapType;        // LinkTable or EndTable
    // For EndTable: which physical end has the FK
    ForeignKeyPartitionView::PersistedEnd m_persistedEnd;  
};
```

This is computed once per class (lazy, cached). Uses the same recursive CTE pattern as `FindRelationshipsOnEnd` but resolves to `ClassMap` immediately to know the storage type.

---

## 2. SQL Generation — Raw SQLite Per Relationship

### 2.1 Link Table Relationships (`RelationshipClassLinkTableMap`)

Physical table has columns: `ECInstanceId`, `ECClassId`, `SourceECInstanceId`, `SourceECClassId`, `TargetECInstanceId`, `TargetECClassId`.

**Forward** (given source, find targets):
```sql
-- When SourceECClassId is a physical column (polymorphic constraint):
SELECT lt.TargetECInstanceId, lt.TargetECClassId, lt.ECClassId
FROM [link_table] lt
  INNER JOIN [ec_cache_ClassHierarchy] ch ON ch.ClassId = lt.SourceECClassId
WHERE lt.SourceECInstanceId = ?
  AND ch.BaseClassId = <source_constraint_base_classid>

-- When SourceECClassId is virtual (single source class — no polymorphism):
SELECT lt.TargetECInstanceId, lt.TargetECClassId, lt.ECClassId
FROM [link_table] lt
WHERE lt.SourceECInstanceId = ?
-- (no class filter needed — all rows belong to one class)
```

**Backward** (given target, find sources):
```sql
SELECT lt.SourceECInstanceId, lt.SourceECClassId, lt.ECClassId
FROM [link_table] lt
  INNER JOIN [ec_cache_ClassHierarchy] ch ON ch.ClassId = lt.TargetECClassId
WHERE lt.TargetECInstanceId = ?
  AND ch.BaseClassId = <target_constraint_base_classid>
-- Same virtual column handling applies symmetrically
```

**Class filtering via `ec_cache_ClassHierarchy`**: Instead of baking a literal `IN (0x1A3, 0x1A4, ...)` list, we INNER JOIN with the `ec_cache_ClassHierarchy` table which has a pre-computed flat list of all derived classes. `BaseClassId` is the abstract constraint class (or the concrete class if non-polymorphic). Both `ClassId` and `BaseClassId` columns are indexed. This:
- Avoids enumerating all derived class IDs at prepare time
- Keeps SQL text stable regardless of hierarchy size → better statement caching
- Is the **established pattern** in `ViewGenerator.cpp` and `ECSqlPreparer.cpp`

**If the class ID column is virtual, omit the JOIN entirely** — all rows belong to one class.

**Column names** come from `RelationshipClassMap`'s `ConstraintECInstanceIdPropertyMap` → `GetColumn().GetName()`.
**Virtual check**: `GetColumn().IsVirtual()` — if true, don't reference in SQL; use static class ID in code.

**CRITICAL: Constraint ECClassId may live on the entity table, not the link table.**
When `foreignEndTableCount == 1` (only one entity table participates on that constraint end), `ConfigureForeignECClassIdKey` (RelationshipClassMap.cpp:1066) reuses the entity table's ECClassId column instead of creating a physical column in the link table. In this case, `FirstCol(*constraintECClassIdPropMap).GetTable()` returns the **entity table**, not the link table. The SQL must check `&col.GetTable() == &linkTable` before referencing `lt.[colName]`:
- If the column IS in the link table: use `lt.[colName]` as normal
- If the column is in an entity table: either JOIN to that entity table to read it, or skip the filter if it's redundant
- The **seed constraint class filter** is redundant when the column is external — relationship discovery already ensures applicability
- The **related constraint class ID** requires an INNER JOIN to the entity table: `INNER JOIN [entity_table] _re ON _re.[Id]=lt.[relatedIdCol]`

### 2.2 End Table / Nav Property Relationships (`RelationshipClassEndTableMap`)

The FK columns live in one entity table. Use `ForeignKeyPartitionView` to find partition details.

**`PersistedEnd == SourceTable`** — nav property is on the source entity table, pointing to the target.

Forward (given source instance, find target):
```sql
-- When ECClassId is a physical column (polymorphic):
SELECT et.[navProp.Id]
FROM [source_table] et
  INNER JOIN [ec_cache_ClassHierarchy] ch ON ch.ClassId = et.ECClassId
WHERE et.ECInstanceId = ?
  AND ch.BaseClassId = <source_constraint_base_classid>
  AND et.[navProp.Id] IS NOT NULL

-- When ECClassId is virtual (single class):
SELECT et.[navProp.Id]
FROM [source_table] et
WHERE et.ECInstanceId = ?
  AND et.[navProp.Id] IS NOT NULL
```
Returns: `(targetInstanceId)`. The relationship class ID comes from:
- If `navProp.RelECClassId` column is **physical**: add it to SELECT
- If `navProp.RelECClassId` column is **virtual**: use `RelECClassIdPropertyMap::GetDefaultClassId()` as a static value — don't query it
The target's `ECClassId` must be determined separately (not stored in this table).

Backward (given target instance, find sources pointing to it):
```sql
SELECT et.ECInstanceId, et.ECClassId              -- ECClassId omitted from SELECT if virtual; use static value
FROM [source_table] et
  INNER JOIN [ec_cache_ClassHierarchy] ch ON ch.ClassId = et.ECClassId
WHERE et.[navProp.Id] = ?
  AND ch.BaseClassId = <source_constraint_base_classid>

-- When ECClassId is virtual, omit the JOIN:
SELECT et.ECInstanceId
FROM [source_table] et
WHERE et.[navProp.Id] = ?
```

**`PersistedEnd == TargetTable`** — symmetric, swap source/target roles.

### 2.3 Multi-Partition Handling

When an abstract class with a nav property has subclasses mapped to different tables, `ForeignKeyPartitionView` has multiple partitions. We generate one SQL per partition and union them logically (iterate each statement in sequence).

### 2.4 Virtual (Static) ECClassId Columns

A column may have `PersistenceType::Virtual` (`DbColumn::IsVirtual() == true`), meaning it does **not exist** in the physical SQLite table. This happens when only one class maps to a table (no polymorphism) — storing the same class ID on every row is wasteful, so ECDb marks the column virtual and remembers the static value.

**Where this occurs:**

| Column | Storage Type | Static Value Source |
|--------|-------------|-------------------|
| `ECClassId` on entity table | Entity/Rel | `PerTableClassIdPropertyMap::GetDefaultECClassId()` |
| `SourceECClassId` on link table | Link Table | Reverse-lookup: `LightweightCache::GetClassesForTable(col.GetTable())` → expect exactly 1 class |
| `TargetECClassId` on link table | Link Table | Same reverse-lookup |
| `navProp.RelECClassId` on entity table | End Table (nav prop) | `NavigationPropertyMap::RelECClassIdPropertyMap::GetDefaultClassId()` |

**Impact on SQL generation:**

1. **SELECT clause**: If `column.IsVirtual()`, do NOT reference the column name. Instead emit the literal static class ID in its place:
   ```sql
   -- Physical column:
   SELECT TargetECClassId FROM [link_table] ...
   -- Virtual column (class ID is always 0x1A3):
   SELECT 0x1A3 FROM [link_table] ...
   ```
   In the InstanceGraph code, skip the column in the SQL and hardcode the known value when building `RelatedInstance`:
   ```cpp
   if (targetClassIdCol->IsVirtual()) {
       // Don't read from SQLite — use the static value
       relatedClassId = getDefaultClassIdForColumn(*targetClassIdCol);
   } else {
       relatedClassId = ECClassId(stmt->GetValueUInt64(colIdx));
   }
   ```

2. **WHERE clause**: If the class ID column in a filter is virtual, **omit the filter entirely** — all rows in that table implicitly belong to that one class:
   ```sql
   -- Physical SourceECClassId:
   WHERE SourceECInstanceId = ? AND SourceECClassId IN (0x1A3, 0x1A4)
   -- Virtual SourceECClassId (only class 0x1A3 possible):
   WHERE SourceECInstanceId = ?
   -- (no class filter needed)
   ```

3. **Nav property RelECClassId**: When the `RelECClassIdPropertyMap::GetColumn().IsVirtual()`:
   - The relationship class is always `GetDefaultClassId()` — don't query the column
   - In `ViewGenerator.cpp` line 365-368, this is handled by emitting the default class ID as a literal

**Helper function** (to be implemented):
```cpp
// Returns the static ECClassId for a virtual column, or invalid if column is physical
ECN::ECClassId GetStaticClassId(DbColumn const& col, LightweightCache const& cache) {
    if (!col.IsVirtual())
        return ECN::ECClassId(); // invalid — must read from row
    
    // For NavigationPropertyMap::RelECClassId — the default is stored in the property map
    // For SourceECClassId/TargetECClassId/ECClassId — reverse-lookup from table
    auto const& classIds = cache.GetClassesForTable(col.GetTable());
    BeAssert(classIds.size() == 1);
    return classIds.empty() ? ECN::ECClassId() : classIds[0];
}
```

### 2.5 Column Resolution

All column names come from the property map hierarchy:
- `RelationshipClassMap::GetSourceECInstanceIdPropMap()` → column for source ID
- `RelationshipClassMap::GetTargetECInstanceIdPropMap()` → column for target ID
- `RelationshipClassMap::GetSourceECClassIdPropMap()` → column for source class ID
- `NavigationPropertyMap::GetIdPropertyMap()` → column for nav property FK
- `NavigationPropertyMap::GetRelECClassIdPropertyMap()` → column for rel class ID
- Each `SingleColumnDataPropertyMap` exposes `GetColumn()` → `DbColumn` → `GetName()` for the physical SQLite column name
- `DbTable::GetName()` for the table name

---

## 3. Statement Cache

```cpp
struct StatementKey {
    ECN::ECClassId m_relClassId;    // relationship class
    ECN::ECRelationshipEnd m_end;   // which end we're querying from
    size_t m_partitionIndex;        // for multi-partition end-table rels
    // hash + equality operators
};

// Per-InstanceGraph cache (or shared across InstanceGraphs from same ECDb)
std::unordered_map<StatementKey, BeSQLite::CachedStatementPtr> m_stmtCache;
```

**Lazy preparation**: On first traversal through a relationship, build the SQL string from property maps, prepare via `ECDb::GetCachedSqliteStatement()` (which itself uses BeSQLite's LRU `StatementCache`), and store in our map. Subsequent uses just reset + rebind.

---

## 4. InstanceGraph Core API

```cpp
// A lightweight instance key — just classId + instanceId
// (ECInstanceKey already exists in ECDb)

enum class TraversalDirection : uint8_t {
    Forward  = 1,
    Backward = 2,
    Both     = 3
};

struct RelatedInstance {
    ECInstanceKey          m_key;                // related instance
    ECN::ECClassId         m_relClassId;         // via which relationship
    TraversalDirection     m_direction;           // forward or backward
};

class InstanceGraph {
public:
    explicit InstanceGraph(ECDbCR ecdb);
    ~InstanceGraph();

    // --- Building the graph ---
    
    // Add a seed and expand relationships one hop
    void AddSeed(ECInstanceKeyCR seed);
    
    // Expand from a specific node (lazy — only expands when called)
    BentleyStatus ExpandNode(ECInstanceKeyCR key, TraversalDirection dir = TraversalDirection::Both);
    
    // Expand the full graph up to maxDepth (0 = seed only, UINT8_MAX = unlimited)
    BentleyStatus ExpandAll(uint8_t maxDepth = UINT8_MAX);
    
    // --- Querying ---
    
    bool Contains(ECInstanceKeyCR key) const;
    size_t NodeCount() const;
    
    // Get all related instances of a node (after expansion)
    bvector<RelatedInstance> const* GetRelated(ECInstanceKeyCR key) const;
    
    // --- Set Operations ---
    
    // Do two graphs share any instance?
    static bool Overlaps(InstanceGraph const& a, InstanceGraph const& b);
    
    // Instances present in both graphs
    static std::unique_ptr<InstanceGraph> Intersection(InstanceGraph const& a, InstanceGraph const& b);
    
    // Instances present in either graph
    static std::unique_ptr<InstanceGraph> Union(InstanceGraph const& a, InstanceGraph const& b);

private:
    ECDbCR m_ecdb;
    
    // Visited set for cycle detection (per-traversal)
    bset<ECInstanceKey> m_visited;
    
    // Adjacency: node → related instances (per-traversal)
    bmap<ECInstanceKey, bvector<RelatedInstance>> m_adjacency;
    
    // All caching delegated to ECDb-level GraphStatementCache (see §11)
    // No per-InstanceGraph statement or discovery caches needed.
    
    // Internal
    bvector<ApplicableRelationship> const& GetApplicableRelationships(ECN::ECClassId classId);
    BeSQLite::CachedStatementPtr GetOrPrepareStatement(ApplicableRelationship const& rel, TraversalDirection dir);
    BentleyStatus ExecuteTraversal(ECInstanceKeyCR key, ApplicableRelationship const& rel, TraversalDirection dir);
};
```

### Cycle Avoidance

- `m_visited` is a set of `ECInstanceKey`
- Before expanding a node, check if it's in `m_visited`; if yes, skip
- Mark visited before processing neighbors
- This prevents infinite loops in cyclic relationship graphs (e.g., A references B references A)

### Lazy Evaluation

- `AddSeed()` only registers the node — no SQL executed
- `ExpandNode()` prepares and executes relationship queries on demand
- `ExpandAll()` does BFS with a queue, calling `ExpandNode()` at each level
- Statements are prepared on first use and cached for the lifetime of the `InstanceGraph`

---

## 5. ECDbVirtualTable: `relations()`

A SQLite virtual table that exposes relationship traversal as a table-valued function.

### Schema

```sql
CREATE TABLE relations(
    -- Output columns
    RelatedECInstanceId,          -- related instance ID
    RelatedECClassId,             -- related instance class ID  
    Direction,                    -- 'forward' | 'backward'
    RelationshipECClassId,        -- relationship class ID
    -- Input (hidden) columns
    ECInstanceId HIDDEN,          -- seed instance ID (required)
    ECClassId HIDDEN,             -- seed instance class ID (required)
    TraversalDirection HIDDEN     -- 'forward' | 'backward' | 'both' (default: 'both')
);
```

### Usage

```sql
-- Find all instances related to instance 1 of class 0x123
SELECT * FROM relations WHERE ECInstanceId = 1 AND ECClassId = 0x123;

-- Forward relationships only
SELECT * FROM relations WHERE ECInstanceId = 1 AND ECClassId = 0x123 AND TraversalDirection = 'forward';

-- Join with instance data
SELECT e.* FROM bis.Element e
JOIN relations r ON r.RelatedECInstanceId = e.ECInstanceId
WHERE r.ECInstanceId = 1 AND r.ECClassId = 0x123;
```

### Implementation

```cpp
struct RelationsModule : ECDbModule {
    constexpr static auto NAME = "relations";
    
    struct RelationsTable : ECDbVirtualTable {
        struct RelationsCursor : ECDbCursor {
            enum class OutputColumns {
                RelatedECInstanceId = 0,
                RelatedECClassId = 1,
                Direction = 2,
                RelationshipECClassId = 3,
                // Hidden inputs
                ECInstanceId = 4,
                ECClassId = 5,
                TraversalDirection = 6
            };
            
            // Cursor state
            bvector<RelatedInstance> m_results;   // materialized results
            size_t m_index = 0;
            
            bool Eof() final;
            DbResult Next() final;
            DbResult GetColumn(int i, Context& ctx) final;
            DbResult GetRowId(int64_t& rowId) final;
            DbResult Filter(int idxNum, const char* idxStr, int argc, DbValue* argv) final;
        };
        
        DbResult Open(DbCursor*& cur) override;
        DbResult BestIndex(IndexInfo& indexInfo) final;
    };
    
    RelationsModule(ECDbR db);
    DbResult Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) final;
};
```

### BestIndex Strategy

- **Required constraints**: `ECInstanceId = ?` AND `ECClassId = ?` (idxNum bit 0 + bit 1)
- **Optional constraint**: `TraversalDirection = ?` (idxNum bit 2)
- If required constraints are missing, return high estimated cost to discourage query planner
- Encode constraint positions in `idxNum` bitmask

### Filter Implementation

1. Extract `ECInstanceId`, `ECClassId`, and optional `TraversalDirection` from `argv[]`
2. Construct `ECInstanceKey` from the inputs
3. Use `InstanceGraph` internally:
   - Create graph, add seed, expand one hop (depth = 1)
   - Collect all `RelatedInstance` results into `m_results`
4. Return results row-by-row via `Next()`/`GetColumn()`

### EC Schema for VTable Registration

```xml
<ECSchema schemaName="ECVLib" alias="ECVLib" version="1.0.0"
          xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbVirtual" version="01.00.00" alias="ecdbvir" />
    <ECCustomAttributes>
        <VirtualSchema xmlns="ECDbVirtual.01.00.00"/>
    </ECCustomAttributes>
    <ECEntityClass typeName="Relations" modifier="Abstract">
        <ECCustomAttributes>
            <VirtualType xmlns="ECDbVirtual.01.00.00"/>
        </ECCustomAttributes>
        <ECProperty propertyName="RelatedECInstanceId" typeName="long" extendedTypeName="Id"/>
        <ECProperty propertyName="RelatedECClassId"    typeName="long" extendedTypeName="Id"/>
        <ECProperty propertyName="Direction"            typeName="string"/>
        <ECProperty propertyName="RelationshipECClassId" typeName="long" extendedTypeName="Id"/>
    </ECEntityClass>
</ECSchema>
```

---

## 6. Set Operations Detail

### Overlap

```cpp
static bool Overlaps(InstanceGraph const& a, InstanceGraph const& b) {
    // Iterate the smaller graph, probe the larger
    auto& smaller = (a.NodeCount() < b.NodeCount()) ? a : b;
    auto& larger  = (a.NodeCount() < b.NodeCount()) ? b : a;
    for (auto& key : smaller.m_visited)
        if (larger.Contains(key))
            return true;
    return false;
}
```

### Intersection

Returns a new `InstanceGraph` containing only nodes present in both. Adjacency edges are preserved only where both endpoints are in the intersection.

### Union

Returns a new `InstanceGraph` combining all nodes and edges from both graphs. Duplicate nodes are merged; duplicate edges are deduplicated.

---

## 7. File Layout

```
iModelCore/ECDb/
├── PublicAPI/ECDb/
│   ├── InstanceGraph.h           // Public API: InstanceGraph, RelatedInstance, TraversalDirection
│   └── ECInstanceFinder.h        // (existing, unchanged)
├── ECDb/
│   ├── InstanceGraph.cpp         // Core implementation
│   ├── InstanceGraphVTab.h       // relations() virtual table module
│   ├── InstanceGraphVTab.cpp     // relations() virtual table implementation
│   └── BuiltInVTabs.cpp          // Register relations() alongside existing vtabs
```

---

## 8. Implementation Todos

### Phase 1: Core InstanceGraph
- [ ] Define `InstanceGraph.h` with public API (TraversalDirection, RelatedInstance, InstanceGraph class)
- [ ] Implement relationship discovery: `GetApplicableRelationships()` using ClassMap/PropertyMap
- [ ] Implement SQL generation for link table relationships
- [ ] Implement SQL generation for end-table / nav property relationships (with multi-partition support)
- [ ] Implement lazy statement cache (`GetOrPrepareStatement()`)
- [ ] Implement `ExpandNode()` and `ExpandAll()` BFS with cycle detection
- [ ] Implement set operations: `Overlaps`, `Intersection`, `Union`

### Phase 2: Virtual Table
- [ ] Define `RelationsModule` / `RelationsTable` / `RelationsCursor`
- [ ] Implement `BestIndex()` with idxNum bitmask strategy
- [ ] Implement `Filter()` using InstanceGraph expand
- [ ] Implement cursor methods (Eof, Next, GetColumn, GetRowId)
- [ ] Register in `RegisterBuildInVTabs()`
- [ ] Add EC virtual schema XML for `Relations` entity class

### Phase 3: Version & Changelog
- [ ] Increment ECSql version: `2.0.3.1` → `2.0.3.2` in `ECDb.h` (Sub2 bump — backward-compatible runtime addition of new virtual table)
  - File: `iModelCore/ECDb/PublicAPI/ECDb/ECDb.h` line 326: `static BeVersion GetECSqlVersion() { return BeVersion(2, 0, 3, 2); }`
- [ ] Update `iModelCore/ECDb/CHANGES.md` — add entry at top:
  ```markdown
  ## ## `<date>`: Added `relations()` virtual table and InstanceGraph API
  * ECSql version change `2.0.3.1` -> `2.0.3.2`.
  * Added `relations()` virtual table for fast relationship traversal from a seed instance.
  * Example: `SELECT * FROM relations WHERE ECInstanceId = 1 AND ECClassId = 0x123`
  * Added internal `InstanceGraph` C++ API for BFS graph traversal with set operations.
  * Added ECDb-level `GraphStatementCache` for caching relationship SQL across traversals.
  ```
- [ ] Update version table in CHANGES.md header: `ECSQL` row → `2.0.3.2`

### Phase 4: Tests

Test file: `iModelCore/ECDb/Tests/NonPublished/InstanceGraphTests.cpp`
Test fixture: `struct InstanceGraphTests : ECDbTestFixture {};`

#### Test Schema Design

A dedicated test schema exercising all relationship variants:

```xml
<ECSchema schemaName="IGTest" ...>
  <!-- Entities -->
  <ECEntityClass typeName="Part" />
  <ECEntityClass typeName="Assembly" />
  <ECEntityClass typeName="Connector" />

  <!-- Inheritance hierarchy for constraint polymorphism -->
  <ECEntityClass typeName="BasePart" modifier="Abstract" />
  <ECEntityClass typeName="MechanicalPart" modifier="None">
    <BaseClass>BasePart</BaseClass>
  </ECEntityClass>
  <ECEntityClass typeName="ElectricalPart" modifier="None">
    <BaseClass>BasePart</BaseClass>
  </ECEntityClass>

  <!-- Link table relationship (base + derived) -->
  <ECRelationshipClass typeName="PartToAssembly" strength="referencing" modifier="None">
    <Source multiplicity="(0..*)" polymorphic="true" roleLabel="part">
      <Class class="Part" />
    </Source>
    <Target multiplicity="(0..*)" polymorphic="true" roleLabel="assembly">
      <Class class="Assembly" />
    </Target>
  </ECRelationshipClass>

  <!-- Derived relationship (shares link table with base — TPH) -->
  <ECRelationshipClass typeName="SpecialPartToAssembly" modifier="None">
    <BaseClass>PartToAssembly</BaseClass>
    <!-- inherits constraints -->
  </ECRelationshipClass>

  <!-- Nav property relationship (end-table, FK in source) -->
  <ECRelationshipClass typeName="ConnectorRefsPart" strength="referencing" modifier="None"
      description="ForeignKeyRelationshipInSourceTable">
    <Source multiplicity="(0..*)" polymorphic="true" roleLabel="connector">
      <Class class="Connector" />
    </Source>
    <Target multiplicity="(0..1)" polymorphic="true" roleLabel="part">
      <Class class="Part" />
    </Target>
  </ECRelationshipClass>
  <!-- Connector has nav prop pointing to Part -->
  <ECEntityClass typeName="Connector">
    <ECNavigationProperty propertyName="Part"
        relationshipName="ConnectorRefsPart" direction="Forward" />
  </ECEntityClass>

  <!-- Self-referential relationship (Part references Part) -->
  <ECRelationshipClass typeName="PartDependsOnPart" strength="referencing" modifier="None">
    <Source multiplicity="(0..*)" polymorphic="true" roleLabel="dependent">
      <Class class="Part" />
    </Source>
    <Target multiplicity="(0..*)" polymorphic="true" roleLabel="dependency">
      <Class class="Part" />
    </Target>
  </ECRelationshipClass>

  <!-- Polymorphic constraint relationship -->
  <ECRelationshipClass typeName="BasePartToConnector" strength="referencing" modifier="None">
    <Source multiplicity="(0..*)" polymorphic="true" roleLabel="base part">
      <Class class="BasePart" />
    </Source>
    <Target multiplicity="(0..*)" polymorphic="true" roleLabel="connector">
      <Class class="Connector" />
    </Target>
  </ECRelationshipClass>
</ECSchema>
```

#### Test Cases — InstanceGraph Core

```
TEST_F(InstanceGraphTests, LinkTable_ForwardTraversal)
  - Insert Part→Assembly via PartToAssembly
  - Seed = Part instance, expand forward
  - Assert: finds Assembly with correct relClassId and direction=Forward

TEST_F(InstanceGraphTests, LinkTable_BackwardTraversal)
  - Same data, seed = Assembly instance, expand backward
  - Assert: finds Part

TEST_F(InstanceGraphTests, LinkTable_BothDirections)
  - Seed Part, expand Both
  - Assert: finds Assembly (forward) + any other Parts via PartDependsOnPart (backward)

TEST_F(InstanceGraphTests, NavProperty_ForwardTraversal)
  - Insert Connector with nav prop pointing to Part
  - Seed = Connector, expand forward
  - Assert: finds Part via ConnectorRefsPart

TEST_F(InstanceGraphTests, NavProperty_BackwardTraversal)
  - Same data, seed = Part, expand backward
  - Assert: finds Connector(s) that reference this Part

TEST_F(InstanceGraphTests, CycleDetection_DirectCycle)
  - Insert: PartA depends-on PartB, PartB depends-on PartA (via PartDependsOnPart)
  - Seed = PartA, ExpandAll(maxDepth=10)
  - Assert: terminates (does NOT loop forever)
  - Assert: both PartA and PartB in graph, each visited exactly once

TEST_F(InstanceGraphTests, CycleDetection_TransitiveCycle)
  - Insert: A→B→C→A via PartDependsOnPart (triangle)
  - Seed = A, ExpandAll(maxDepth=UINT8_MAX)
  - Assert: exactly 3 nodes {A, B, C}, terminates

TEST_F(InstanceGraphTests, CycleDetection_SelfLoop)
  - Insert: PartA depends-on PartA
  - Seed = PartA, expand
  - Assert: terminates, graph has 1 node

TEST_F(InstanceGraphTests, InheritedRelationship_DerivedRelInTPH)
  - Insert Part→Assembly via SpecialPartToAssembly (derived rel, shares link table)
  - Seed = Part, expand forward
  - Assert: finds Assembly with relClassId = SpecialPartToAssembly (not base)
  - Verifies TPH ECClassId filter works correctly

TEST_F(InstanceGraphTests, PolymorphicConstraint_DerivedEntityMatchesAbstractConstraint)
  - Insert MechanicalPart (derives from BasePart)
  - Create BasePartToConnector relationship instance: MechanicalPart → Connector
  - Seed = MechanicalPart, expand forward
  - Assert: finds Connector via BasePartToConnector
  - Verifies ec_cache_ClassHierarchy JOIN handles polymorphic constraints

TEST_F(InstanceGraphTests, PolymorphicConstraint_MultipleSubclasses)
  - Insert MechanicalPart→Connector and ElectricalPart→Connector
  - Seed = MechanicalPart, expand
  - Assert: finds only Connector linked to MechanicalPart (not ElectricalPart's)

TEST_F(InstanceGraphTests, SelfReferentialRelationship_BothEnds)
  - Insert Part1 depends-on Part2 via PartDependsOnPart
  - Seed = Part1, expand Both
  - Assert: finds Part2 as forward AND checks backward for anything referencing Part1
  - Seed = Part2, expand Both
  - Assert: finds Part1 as backward

TEST_F(InstanceGraphTests, MixedRelationships_LinkTableAndNavProp)
  - Part has both: link table rel (PartToAssembly) and nav prop rel (ConnectorRefsPart backward)
  - Seed = Part, expand Both
  - Assert: finds Assembly (via link table forward) + Connector(s) (via nav prop backward)
  - Verifies heterogeneous relationship discovery

TEST_F(InstanceGraphTests, MultiHopExpansion_DepthLimit)
  - Chain: Part→Assembly→Connector (via different rels)
  - Seed = Part, ExpandAll(maxDepth=1)
  - Assert: only Assembly found (1 hop)
  - Seed = Part, ExpandAll(maxDepth=2)
  - Assert: Assembly + Connector found (2 hops)

TEST_F(InstanceGraphTests, EmptyGraph_NoRelationships)
  - Insert a Part with no relationships
  - Seed = Part, expand
  - Assert: graph has 1 node (seed), 0 related instances

TEST_F(InstanceGraphTests, NotMappedRelationship_Skipped)
  - If a NotMapped relationship class exists for the seed class
  - Assert: it does not crash, is silently skipped
```

#### Test Cases — Set Operations

```
TEST_F(InstanceGraphTests, Overlaps_SharedNode)
  - Graph A: seed Part1, expands to {Assembly1}
  - Graph B: seed Part2, expands to {Assembly1}
  - Assert: Overlaps(A, B) == true (Assembly1 shared)

TEST_F(InstanceGraphTests, Overlaps_Disjoint)
  - Graph A: Part1→Assembly1
  - Graph B: Part2→Assembly2
  - Assert: Overlaps(A, B) == false

TEST_F(InstanceGraphTests, Intersection_CommonNodes)
  - Graph A: {Part1, Assembly1, Connector1}
  - Graph B: {Part2, Assembly1, Connector1}
  - Assert: Intersection has {Assembly1, Connector1}

TEST_F(InstanceGraphTests, Union_MergesAll)
  - Graph A: {Part1, Assembly1}
  - Graph B: {Part2, Assembly2}
  - Assert: Union has {Part1, Assembly1, Part2, Assembly2}
```

#### Test Cases — `relations()` Virtual Table

```
TEST_F(InstanceGraphTests, VTable_BasicQuery)
  - SELECT * FROM relations WHERE ECInstanceId=1 AND ECClassId=0x123
  - Assert: returns expected related instances

TEST_F(InstanceGraphTests, VTable_ForwardOnly)
  - SELECT * FROM relations WHERE ECInstanceId=1 AND ECClassId=0x123 AND TraversalDirection='forward'
  - Assert: only forward relationships returned

TEST_F(InstanceGraphTests, VTable_BackwardOnly)
  - Same with TraversalDirection='backward'

TEST_F(InstanceGraphTests, VTable_JoinWithEntityTable)
  - SELECT e.ECInstanceId FROM [Part] e
    JOIN relations r ON r.RelatedECInstanceId = e.ECInstanceId
    WHERE r.ECInstanceId = :id AND r.ECClassId = :classId
  - Assert: correct join results

TEST_F(InstanceGraphTests, VTable_MissingRequiredConstraint_HighCost)
  - SELECT * FROM relations (no ECInstanceId/ECClassId constraint)
  - Assert: returns error or empty (BestIndex returns high cost, planner may reject)

TEST_F(InstanceGraphTests, VTable_LinkTableRelationship)
  - Verify relations() correctly returns link table relationship results with correct RelationshipECClassId

TEST_F(InstanceGraphTests, VTable_NavPropertyRelationship)
  - Verify relations() returns nav prop results with correct direction and RelationshipECClassId

TEST_F(InstanceGraphTests, VTable_ECSqlVersion)
  - PRAGMA ecsql_ver returns >= 2.0.3.2
```

#### Test Cases — Edge Cases & Robustness

```
TEST_F(InstanceGraphTests, VirtualColumn_StaticClassId)
  - Create scenario where ECClassId / RelECClassId is virtual (single class per table)
  - Expand graph, verify correct static class ID is returned in results

TEST_F(InstanceGraphTests, SharedColumn_NavPropDisambiguation)
  - Create scenario where nav prop ID column is shared between two nav properties
  - Verify only the correct relationship's instances are returned

TEST_F(InstanceGraphTests, LargeGraph_Performance)
  - Insert 1000+ instances with relationships forming a tree
  - Time ExpandAll() vs ECInstanceFinder::FindRelatedInstances()
  - Assert: InstanceGraph is measurably faster (3x+ expected)
  - Assert: correct result count matches ECInstanceFinder
```

---

## 9. Performance Expectations

| Aspect | ECInstanceFinder (current) | InstanceGraph (new) |
|--------|--------------------------|-------------------|
| Statement prep | ECSql parse + resolve + generate | Direct SQLite prepare from column names |
| Per-query overhead | ECSql bind + step | SQLite bind + step |
| Caching | One ECSqlStatement per (rel, class, end) | One CachedStatementPtr per (rel, end, partition) |
| Class filter | Baked into ECSql WHERE clause | INNER JOIN ec_cache_ClassHierarchy |
| Result handling | ECSql row → ECInstanceKey | Raw sqlite3_column_int64 → ECInstanceKey |

Expected improvement: **3-10x faster** for graph traversal due to eliminating ECSql parsing, property resolution, and view generation overhead on each relationship query.

---

## 10. Review Notes — Issues & Corrections

After reviewing the plan against `ViewGenerator::RenderRelationshipClassEndTableMap()` (the source of truth for end-table SQL generation), several important gaps were identified.

### 10.1 CRITICAL: Referenced-End JOIN for End-Table Relationships

For end-table (nav property) relationships, **the referenced end's ECClassId lives in a DIFFERENT table** — the table of the referenced entity. The FK-holding table only has its own entities' ECClassId, not the related entity's.

`ViewGenerator.cpp` lines 1017-1038 handle this by INNER JOINing the referenced end table:
```sql
-- Example: PersistedEnd=SourceTable (nav prop on source, FK points to target)
-- Forward traversal: seed is source, finding target
SELECT et.[navProp.Id],                         -- target ECInstanceId
       ref_table.ECClassId                      -- target ECClassId (from JOIN!)
FROM [source_table] et
  INNER JOIN [target_table] ref_table
    ON ref_table.ECInstanceId = et.[navProp.Id]
WHERE et.ECInstanceId = ?
  AND et.[navProp.Id] IS NOT NULL
```

**When is the JOIN required?** Only when `refClassIdCol.GetPersistenceType() == PersistenceType::Physical`. If the referenced end's class ID column is virtual, the class is static — no JOIN needed.

**Self-referential relationships** (`isSelf`): When SourceECClassId and TargetECClassId columns are the same (self-referential), the ViewGenerator aliases the JOIN as `_ReferencedEnd` to avoid ambiguity. The InstanceGraph must do the same.

**Summary for InstanceGraph SQL generation by case:**

| Traversal | PersistedEnd | FK holder | Related end | Need JOIN for related ECClassId? |
|-----------|-------------|-----------|-------------|--------------------------------|
| Forward (source→target) | SourceTable | Source table | Target | YES if target ECClassId is physical |
| Backward (target→source) | SourceTable | Source table | Source | NO — source ECClassId is in FK table |
| Forward (source→target) | TargetTable | Target table | Target | NO — target ECClassId is in FK table |
| Backward (target→source) | TargetTable | Target table | Source | YES if source ECClassId is physical |

**Rule**: When we're querying for the entity that is on the **referenced end** (the end WITHOUT the FK), we need a JOIN to get its class ID.

### 10.2 CRITICAL: Missing Relationship ECClassId Filter

For TPH-mapped relationships (common), the physical table may hold rows from **multiple relationship class hierarchies**. The SQL MUST filter by the relationship's own ECClassId.

`ViewGenerator.cpp` line 1043-1051:
```cpp
if (partition->GetECClassIdColumn().GetPersistenceType() == PersistenceType::Physical)
    unionQuerySql.Append(" AND ").Append(ECClassIdCol)
        .AppendFormatted(" IN (SELECT ClassId FROM ec_cache_ClassHierarchy WHERE BaseClassId=%s)",
            relationMap.GetClass().GetId().ToString().c_str());
```

**Updated link table SQL** (forward):
```sql
SELECT lt.TargetECInstanceId, lt.TargetECClassId, lt.ECClassId
FROM [link_table] lt
  INNER JOIN [ec_cache_ClassHierarchy] ch_src ON ch_src.ClassId = lt.SourceECClassId
WHERE lt.SourceECInstanceId = ?
  AND ch_src.BaseClassId = <source_constraint_base_classid>
  AND lt.ECClassId IN (SELECT ClassId FROM [ec_cache_ClassHierarchy]
                       WHERE BaseClassId = <base_rel_classid>)
```
The relationship class filter can also use an INNER JOIN:
```sql
  INNER JOIN [ec_cache_ClassHierarchy] ch_rel ON ch_rel.ClassId = lt.ECClassId
      AND ch_rel.BaseClassId = <base_rel_classid>
```
If the relationship's ECClassId column is virtual, omit the filter.

**Updated end-table SQL** (forward through nav prop):
```sql
SELECT et.[navProp.Id], ref_table.ECClassId, et.[navProp.RelECClassId]
FROM [source_table] et
  INNER JOIN [target_table] ref_table ON ref_table.ECInstanceId = et.[navProp.Id]
  INNER JOIN [ec_cache_ClassHierarchy] ch_self ON ch_self.ClassId = et.ECClassId
WHERE et.ECInstanceId = ?
  AND ch_self.BaseClassId = <source_constraint_base_classid>
  AND et.[navProp.Id] IS NOT NULL
  AND et.[navProp.RelECClassId] IN (SELECT ClassId FROM [ec_cache_ClassHierarchy]
                                     WHERE BaseClassId = <base_rel_classid>)
```
(The `navProp.RelECClassId` filter handles the relationship class. If that column is virtual, the filter is implicit.)

### 10.3 Shared Column Disambiguation

When the nav property's ID column (`referenceIdColumn`) is **shared** (multiple nav properties share the same physical column), the foreign end's class ID must also be filtered. From `ViewGenerator.cpp` line 1053:
```cpp
if (foreignClassIdColumn.GetPersistenceType() == PersistenceType::Physical && referenceIdColumn.IsShared())
    // Add: AND foreignClassIdColumn IN (SELECT ClassId FROM ec_cache_ClassHierarchy
    //                                    WHERE BaseClassId = abstractConstraint->GetId())
```

This prevents false matches when a shared column happens to contain an ID from a different relationship. The InstanceGraph must check `GetColumn().IsShared()` and add this extra filter when true.

### 10.4 Not-Mapped Relationships

Some relationship classes have `MapStrategy::NotMapped` (no physical table). `ECInstanceFinder` already skips these (line 332-335 of ECInstanceFinder.cpp). The InstanceGraph must do the same: during relationship discovery, check `classMap->GetMapStrategy().GetStrategy() != MapStrategy::NotMapped`.

### 10.5 Self-Referential Relationships

When a class appears on **both** ends of a relationship (e.g., Element refers to Element), `FindRelationshipsOnEnd` returns TWO entries — one for Source, one for Target. Both must be traversed. For end-table rels, the INNER JOIN for the referenced end must use an alias (`_ReferencedEnd`) when the source and target tables are the same table.

### 10.6 Performance Table Correction

The class filter row should now say "INNER JOIN ec_cache_ClassHierarchy" instead of "Literal class IDs in SQL text".

### 10.7 ForeignKeyView Relationships

Some end-table relationships use `ForeignKeyView` custom attribute (ViewGenerator line 877). These are special: the relationship class acts as a view over a nav property. The ViewGenerator generates an ECSql query for these. For the InstanceGraph, these should likely be treated the same as standard end-table relationships since the underlying storage is identical — the ForeignKeyView is an ECSql-level abstraction.

### 10.8 Summary of Required Filters per SQL Query

| Filter | Link Table | End Table (FK holder side) | End Table (referenced side) |
|--------|-----------|--------------------------|---------------------------|
| Seed instance ID (bind param) | `SourceECInstanceId = ?` | `ECInstanceId = ?` (FK holder) | `navProp.Id = ?` (seed is referenced) |
| Constraint class (`ec_cache_ClassHierarchy` JOIN) | Yes (on queried end) | Yes (ECClassId if physical) | Yes (ECClassId if physical) |
| Relationship class filter | Yes (ECClassId if physical) | Yes (`navProp.RelECClassId` if physical) | Yes (`navProp.RelECClassId` if physical) |
| Shared column disambiguation | N/A | Yes (if `referenceIdColumn.IsShared()`) | Yes (if `referenceIdColumn.IsShared()`) |
| Referenced end JOIN | N/A (both in same table) | Yes (if related class ID is physical) | N/A (class ID in own table) |
| `IS NOT NULL` guard | N/A | Yes (`navProp.Id IS NOT NULL`) | N/A (filtering on navProp.Id = ?) |

---

## 11. Statement Caching Strategy — Performance Analysis

### Current Caching Infrastructure

| Cache | Location | Key | Eviction | Size |
|-------|----------|-----|----------|------|
| `BeSQLite::Db::GetCachedStatement()` | Per SQLite db | SQL text (string match) | LRU | 20 slots |
| `ECDb::Impl::m_sqliteStatementCache` | Per ECDb | SQL text (string match) | LRU | 20 slots |

Both are LRU with **only 20 slots**, shared with all other ECDb internal queries.

### Problem: Per-InstanceGraph Cache is Insufficient

The plan currently proposes `m_stmtCache` inside `InstanceGraph`. This has two problems:

1. **LRU thrashing**: A BIS model may have 50+ relationship classes. During BFS traversal, we'd need ~50 active statements. The ECDb LRU (20 slots) would evict statements from early relationships before we revisit them at the next BFS depth level. Each eviction means a full re-prepare.

2. **Short-lived graph → wasted prep**: For the `relations()` virtual table, SQLite creates a cursor per query. If we create an `InstanceGraph` per Filter() call, each one re-prepares all needed statements from scratch — even if the previous cursor used identical SQL.

### Recommendation: ECDb-Level Shared `GraphStatementCache`

Add a **pinned (non-LRU) cache** at the ECDb level, surviving across multiple InstanceGraph instances:

```cpp
struct GraphStatementCache final {
    struct Entry {
        Utf8String               m_sql;       // generated SQL text
        BeSQLite::CachedStatementPtr m_stmt;  // prepared statement (pinned, not in LRU)
    };
    
    struct Key {
        ECN::ECClassId          m_relClassId;
        ECN::ECRelationshipEnd  m_thisEnd;       // Source or Target
        TraversalDirection      m_direction;      // Forward or Backward
        size_t                  m_partitionIdx;   // for multi-partition end-table rels
        // hash + equality operators
    };
    
    // Pinned cache — statements stay alive as long as this cache exists
    std::unordered_map<Key, Entry> m_entries;
    
    // Relationship discovery cache (also expensive to compute)
    bmap<ECN::ECClassId, bvector<ApplicableRelationship>> m_relDiscoveryCache;
    
    void Clear();  // Called from ClearECDbCache()
};
```

**Ownership**: `ECDb::Impl` owns the `GraphStatementCache` alongside `m_sqliteStatementCache`. Cleared when `ClearECDbCache()` is called (schema changes, etc.).

**Why pinned, not LRU**:
- Relationship SQL is deterministic — same relationship always generates same SQL
- Number of entries is bounded by `(relationship_classes × 2 directions × partitions)` — typically 100-200 entries, each holding only a ref-counted pointer
- Memory cost is negligible (~few KB for the map + SQLite's prepared statement memory)
- Avoids thrashing under BFS traversal of large graphs

### Two-Phase Lookup

```
InstanceGraph::GetOrPrepareStatement(rel, direction)
  │
  ├─ 1. Check ECDb-level GraphStatementCache (fast hash lookup)
  │     Hit → Reset + Rebind → return
  │
  └─ 2. Cache miss:
        a. Build SQL text from PropertyMap/ClassMap/ForeignKeyPartitionView
        b. Prepare via db.GetCachedStatement(sql) or direct sqlite3_prepare_v2
        c. Store (sql, stmt) in GraphStatementCache
        d. return
```

**Step 2a is the expensive part** — walking property maps, checking virtual columns, building JOIN clauses. This happens only once per relationship direction, then is cached at the ECDb level. All future InstanceGraphs skip directly to step 1.

### Why NOT use ECDb's existing `m_sqliteStatementCache`

| Factor | ECDb LRU cache | GraphStatementCache |
|--------|---------------|-------------------|
| Eviction | LRU (20 slots) | None (pinned) |
| Key type | SQL string (O(n) compare) | Struct hash (O(1)) |
| Shared with | All ECDb internal queries | Graph traversal only |
| SQL generation | Must regenerate on miss | Cached alongside stmt |
| Concurrent use | Stmt in-use blocks reuse | Stmt ref-counted, can be shared |

The ECDb LRU is still useful as a fallback — if `GraphStatementCache` is cleared, the LRU may still have the prepared statement, avoiding a full re-prepare. But we don't rely on it.

### Relationship Discovery Cache

The `bmap<ECClassId, bvector<ApplicableRelationship>>` should also live in `GraphStatementCache` (not per-InstanceGraph). The recursive CTE query on `ec_RelationshipConstraint` is the second most expensive operation. Caching it at ECDb level means the first InstanceGraph pays the cost, and all subsequent ones benefit.

### InstanceGraph Becomes Lightweight

With the shared cache, InstanceGraph holds only:
- Reference to `ECDb` (which owns the cache)
- `m_visited` set (per-traversal cycle detection)
- `m_adjacency` map (per-traversal results)

No statement management, no SQL generation — all delegated to the shared cache. This makes InstanceGraph cheap to create and destroy, which is ideal for the `relations()` vtable use case.

### Thread Safety Note

- `GraphStatementCache` should be protected by `ECDb`'s existing mutex (`BeMutexHolder`)
- `CachedStatementPtr` uses atomic ref-counting — safe to hold across threads
- But `Statement::Step()` is NOT thread-safe — each InstanceGraph must own its own cursor state
- If concurrent graph traversals are needed, each thread gets its own `CachedStatementPtr` (ref-count bumped), does `Reset() + Bind()` independently. SQLite handles concurrent readers fine on WAL mode.

### Updated Section 3 (Statement Cache) — Correction

Replace the per-InstanceGraph `m_stmtCache` with a reference to the ECDb-level `GraphStatementCache`. The `StatementKey` struct and caching logic move to `GraphStatementCache`. InstanceGraph calls `m_ecdb.GetImpl().GetGraphStatementCache().GetOrPrepare(...)` instead of managing its own cache.

---

## 12. Second Review — Issues, Risks & Missing Items

After cross-referencing the plan against BeSQLite `StatementCache` internals, `ForeignKeyPartitionView` API, ECDb vtab patterns, and relationship discovery code, the following issues were found.

### 12.1 CRITICAL: §11 Thread Safety Model is Wrong

The plan states: *"each thread gets its own `CachedStatementPtr` (ref-count bumped), does `Reset() + Bind()` independently"*. **This is incorrect.**

`StatementCache::GetPreparedStatement()` (`BeSQLite.cpp` line 4858) **refuses to return a statement that is already held** (RefCount > 1):
```cpp
if (1 < (*it)->GetRefCount()) // this statement is currently in use, we can't share it
    continue;
```
If the statement is in use, the cache **creates an entirely new prepared statement** with the same SQL. Two threads never share the same underlying `sqlite3_stmt`. Ref-count bumping does NOT give you a second cursor — it gives you a second reference to the **same** cursor (which is NOT safe for concurrent Step/Bind).

**Impact on `GraphStatementCache`**: If we store `CachedStatementPtr` in the shared cache, that ref keeps `RefCount ≥ 2`. Any subsequent call to `Db::GetCachedStatement(same_sql)` will skip the cached statement and re-prepare. Our "pinned" cache **defeats the LRU** rather than leveraging it.

**Corrected design**: `GraphStatementCache` should store **SQL text + metadata only**, not `CachedStatementPtr`. Each traversal call obtains a fresh `CachedStatementPtr` via `Db::GetCachedStatement(sql)`. The value of the cache is avoiding the expensive SQL *generation* (walking property maps), not holding prepared statements.

```cpp
struct GraphStatementCache final {
    struct Entry {
        Utf8String  m_sql;           // generated SQL text (the expensive part to compute)
        bool        m_targetClassIdVirtual;
        ECN::ECClassId m_staticTargetClassId;
        bool        m_relClassIdVirtual;
        ECN::ECClassId m_staticRelClassId;
        // ... other metadata needed when processing results
    };
    // Key and maps remain the same
    std::unordered_map<Key, Entry> m_entries;
    bmap<ECN::ECClassId, bvector<ApplicableRelationship>> m_relDiscoveryCache;
};
```

For single-threaded use (the common case), `Db::GetCachedStatement()` returns the same statement each time (RefCount goes 1→2, used, dropped back to 1 with auto-Reset). LRU works normally.

### 12.2 WRONG: LRU Cache Size is 35, Not 20

The plan says "only 20 slots" throughout §11. Actual defaults:
- `Db::Db()` initializes `StatementCache` with **size=35** (`BeSQLite.cpp` line 1846)
- `DbFile::DbFile()` initializes with **size=10** (line 1482)

ECDb uses the `Db`-level cache (35 slots). With the corrected design (§12.1 — not pinning statements), 35 slots is more reasonable for sequential BFS. During one BFS depth level, statements are used one-at-a-time. LRU keeps the most recent 35. Only schemas with >35 relationship classes would see eviction between passes.

**Recommendation**: Monitor in practice. If thrashing occurs, `StatementCache` size is a constructor parameter — ECDb could increase it.

### 12.3 IMPORTANT: §3 Inconsistent with §4 and §11

§3 still shows the old per-InstanceGraph cache:
```cpp
std::unordered_map<StatementKey, BeSQLite::CachedStatementPtr> m_stmtCache;
```
But §4's class definition (updated) removed it, and §11 says to replace it. §3 should be rewritten to describe the ECDb-level `GraphStatementCache` holding SQL text, not `CachedStatementPtr`.

### 12.4 IMPORTANT: `ForeignKeyPartitionView` Access Path is Wrong

The plan implies direct access from `RelationshipClassEndTableMap`. Actual access path:
- `DbMappingManager::FkRelationshipMappingInfo` holds `std::unique_ptr<ForeignKeyPartitionView>`
- Access: `FkRelationshipMappingInfo::GetPartitionView()`
- Factory: `ForeignKeyPartitionView::CreateReadonly(tableSpaceSchemaManager, relClass)`

The InstanceGraph should call `ForeignKeyPartitionView::CreateReadonly()` directly during SQL generation (then cache the result).

### 12.5 IMPORTANT: Partition Column Accessors Return Nullable Pointers

Several `Partition` accessors return `DbColumn const*` (nullable), not references:
- `GetSourceECClassIdColumn()` → `DbColumn const*`
- `GetTargetECClassIdColumn()` → `DbColumn const*`
- `GetFromECClassIdColumn()` → `DbColumn const*`
- `GetOtherEndTable()` → `DbTable const*`

The SQL generation code MUST null-check these. A null class ID column means the column doesn't exist — equivalent to virtual but at a different level (the partition is incomplete/abstract).

### 12.6 IMPORTANT: Two Accessor Views on Partition — From/To vs Source/Target

`Partition` exposes two parallel accessor sets:
1. **Source/Target (canonical)**: `GetSourceECInstanceIdColumn()`, `GetTargetECInstanceIdColumn()` — always in relationship's source/target order
2. **From/To (FK-holder)**: `GetFromECInstanceIdColumn()`, `GetToECInstanceIdColumn()` — From = FK-holder side, To = referenced side

The plan's SQL generation should use the **From/To** accessors since they map directly to the physical FK layout. The Source/Target accessors can swap direction depending on `PersistedEnd`.

### 12.7 RISK: `IsPolymorphic` Flag on Constraints

The plan's relationship discovery assumes all constraint classes are polymorphic (matching derived classes). In reality, `ec_RelationshipConstraint.IsPolymorphic` can be `0`, meaning **only the exact listed constraint classes match** — not their derived classes.

`ECInstanceFinder::FindRelationshipsOnEnd()` checks this explicitly:
```sql
WHERE ForeignEndConstraintClass.ClassId = :endClassId
   OR (ForeignEndConstraint.IsPolymorphic = 1 
       AND ForeignEndConstraintClass.ClassId = BaseClassesOfEndClass.ClassId)
```

If `IsPolymorphic=0` for a constraint, a derived class of the constraint class should NOT match. The InstanceGraph's relationship discovery query MUST respect this flag.

### 12.8 RISK: Missing `ExistingTable` and `UnsupportedByECVersion` MapStrategies

The `MapStrategy::Strategy` enum has values the plan doesn't address:
- `ExistingTable(3)` — maps to a user-defined existing table (rare for relationships, but possible)
- `UnsupportedByECVersion(99)` — schema version too new for this ECDb build

Both must be handled during relationship discovery:
- `ExistingTable`: Needs investigation — may require different SQL generation logic
- `UnsupportedByECVersion`: Skip (treat as NotMapped)

### 12.9 RISK: `CachedStatement::Release()` Auto-Resets

When a `CachedStatementPtr` drops to RefCount=1 (only cache holds it), `Release()` automatically calls `Reset()` + `ClearBindings()`. This means:
- You do NOT need to manually call `Reset()` before dropping a `CachedStatementPtr`
- But you MUST finish all `Step()` calls before the `CachedStatementPtr` goes out of scope
- If the cache evicts a statement while you hold a ref (`m_inCache=false`), dropping the ref **deletes** the statement entirely instead of resetting it

The plan should note: always keep `CachedStatementPtr` alive for the duration of Step() iteration.

### 12.10 MISSING: `relations()` VTable Implementation Details

Several implementation details are missing from §5:

1. **RowId**: `GetRowId()` must return a unique int64 per row. Use a monotonic counter per Filter() call (e.g., `m_index`).

2. **Config tag**: Use `Config::Tags::Innocuous` since `relations()` is read-only and safe in triggers/views.

3. **ECDb access chain from cursor**: `GetTable().GetModule()` → cast to `RelationsModule&` → `GetECDb()`.

4. **Constructor**: ECDbModule takes 4 params: `(db, NAME, CREATE_TABLE_SQL, EC_SCHEMA_XML)`. The CREATE TABLE string must include HIDDEN columns:
   ```cpp
   "CREATE TABLE x(RelatedECInstanceId, RelatedECClassId, Direction, "
   "RelationshipECClassId, ECInstanceId HIDDEN, ECClassId HIDDEN, "
   "TraversalDirection HIDDEN)"
   ```

5. **Registration**: `(new RelationsModule(ecdb))->Register()` in `RegisterBuildInVTabs()`. Ownership transfers to BeSQLite.

6. **Filter values are `DbValue*`** (not `sqlite3_value*`). Use `argv[i].GetValueInt64()`, `argv[i].GetValueText()`.

### 12.11 MISSING: Error Handling / Memory Bounds

1. **Statement preparation failure**: `Db::GetCachedStatement()` returns nullptr on failure. No propagation strategy defined — should return `BentleyStatus::ERROR` from `ExpandNode()`.

2. **Step() errors**: `Statement::Step()` can return errors (I/O, corrupt db). Must propagate, not silently ignore.

3. **Memory bounds for large graphs**: `ExpandAll()` with `maxDepth=UINT8_MAX` on a large model could consume unbounded memory (all nodes + adjacency in `m_adjacency`). Consider:
   - A configurable node count limit
   - Or a streaming/iterator API that doesn't materialize the full graph
   - At minimum, document the risk

### 12.12 MISSING: Schema Change During Traversal

`GraphStatementCache` is cleared on `ClearECDbCache()`. If a schema import happens mid-traversal (e.g., in a different thread or after a checkpoint), cached SQL text becomes invalid (table/column names may change). The traversal should either:
- Hold a lock preventing schema changes during expansion, or
- Check a generation counter before using cached entries, or
- Accept that schema changes during traversal are undefined behavior (document it)

### 12.13 MINOR: Set Operations on InstanceGraph Return Disconnected Graphs

`Intersection()` and `Union()` return `std::unique_ptr<InstanceGraph>`. These new graphs hold `m_visited` and `m_adjacency` but:
- They reference the same `ECDbCR` — fine for further expansion
- But `Intersection` result has edges only where both endpoints survive — this is a new computation, not a simple set intersection of `m_visited`
- `Union` must merge adjacency lists and deduplicate edges
- Neither operation is trivial. The plan's §6 sketch for `Overlaps()` is correct but `Intersection`/`Union` need more detailed algorithms.

### 12.14 MINOR: EC Schema XML for `relations()` is Incomplete

The §5 schema XML is missing:
- Hidden input properties (`ECInstanceId`, `ECClassId`, `TraversalDirection`) — these should be in the EC schema if the vtable is to be ECSql-queryable, or explicitly excluded with documentation
- The existing `IdSet` vtable only exposes output columns in EC schema and hidden columns in CREATE TABLE. Follow the same pattern.

### Summary — Priority Matrix

| # | Severity | Section | Issue |
|---|----------|---------|-------|
| 12.1 | **CRITICAL** | §11 | Thread safety model wrong — CachedStatementPtr not shareable |
| 12.2 | **WRONG** | §11 | LRU size is 35, not 20 |
| 12.3 | **IMPORTANT** | §3/§4/§11 | Internal inconsistency on cache design |
| 12.4 | **IMPORTANT** | §2 | ForeignKeyPartitionView access path wrong |
| 12.5 | **IMPORTANT** | §2 | Nullable column pointers not handled |
| 12.6 | **IMPORTANT** | §2 | Two accessor views (From/To vs Source/Target) |
| 12.7 | **RISK** | §1 | IsPolymorphic=0 constraints not respected |
| 12.8 | **RISK** | §1 | ExistingTable and UnsupportedByECVersion not handled |
| 12.9 | **RISK** | §11 | CachedStatement auto-reset on Release() |
| 12.10 | **MISSING** | §5 | VTable implementation details |
| 12.11 | **MISSING** | §4 | Error handling and memory bounds |
| 12.12 | **MISSING** | §11 | Schema change during traversal |
| 12.13 | **MINOR** | §6 | Set operation algorithms underspecified |
| 12.14 | **MINOR** | §5 | EC schema XML incomplete |

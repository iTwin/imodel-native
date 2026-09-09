# ECDb Optimizer

## Purpose

The ECDb Optimizer performs selected maintenance operations that remove unused schema
definitions, repair invalid references, and improve the organization of shared physical
storage in an iModel.

Optimization is intentionally selective. Each phase can be enabled independently, or related
phases can be selected through the schema-cleanup, data-cleanup, storage-cleanup, and all-options
groups. This allows an application to run only the maintenance appropriate for its workflow and
to avoid expensive or destructive operations that it does not need.

The Optimizer does not change BisCore or ECDb system schema definitions or their mappings.
Protected definitions, inherited protected properties, and physical storage required by protected
mappings are reserved throughout schema and storage cleanup.

## Execution and transaction model

Optimization runs inside a transaction owned by the caller. The Optimizer does not start,
commit, abandon, or roll back that transaction. A successful run leaves all changes pending for
the caller to save or abandon.

If a phase fails, processing stops at that phase. Changes made by earlier phases remain in the
caller's transaction. The caller should normally abandon the transaction after an optimization
failure unless it deliberately wants to inspect or retain the partial work.

Metadata-changing phases require schema-import authorization when the ECDb enforces that policy.
Data-repair and compaction phases require CRUD-write authorization when that policy is enabled.
Optimization also requires a writable ECDb, an active transaction, and current BeSQLite and ECDb
profiles.

Schema versions are not incremented by optimization. Schema and mapping caches are invalidated
after metadata changes, so schema, class, property, mapping, table, and column objects obtained
before optimization must not be reused afterward.

## Dry run and reporting

A dry run builds the same optimization plan without changing the main database. It may use
temporary tables to calculate dependencies, mapping movements, or projected results, but those
temporary objects are removed before the operation returns.

The result reports each selected phase separately:

- **Candidates** are objects, mappings, or rows identified for action.
- **Changed** is the amount of work actually applied. It remains zero during a dry run.
- **Skipped** identifies candidates that could not safely be optimized, such as unsupported
  property mappings or protected storage.
- **Elapsed time** records the duration of the phase.
- **Messages** explain important exclusions or conditions discovered while planning.

If a phase fails, the result identifies the failed phase and preserves the database error.

## Phase order

Selected phases run in a fixed order so later phases can benefit from earlier cleanup:

1. Purge invalid class identifiers.
2. Delete orphan link-table relationships.
3. Nullify orphan navigation properties.
4. Drop unused schemas.
5. Drop empty dynamic classes.
6. Drop empty dynamic properties.
7. Compact shared columns and clean overflow rows.
8. Drop unmapped tables.
9. Drop unmapped shared columns.
10. Analyze the database.

For example, removing an empty dynamic property may leave a shared column unmapped. A later
storage-cleanup phase can then remove that column. Likewise, compacting mappings can make an
overflow table or overflow row unnecessary before physical storage cleanup begins.

## Phase details

### Purge invalid class identifiers

Every persisted EC instance has a class identity, stored either physically or supplied by its
mapping. Corrupt or obsolete data can contain a physical class identifier that no longer exists
in ECDb metadata.

This phase examines ECDb-managed primary, joined, link, and overflow tables that physically store
an ECClassId. Rows whose stored class identifier does not exist in the class catalog are deleted.
The scan uses physical tables rather than ECSQL class filtering so that invalid rows, which ECSQL
may not expose as valid instances, can still be found.

The phase deletes a row only because the row's own class identity is invalid. It does not delete a
valid owning instance merely because one of its relationships or navigation properties points to
an invalid target. Those conditions are handled by the relationship and navigation phases.

This cleanup prevents unresolvable rows from remaining in storage and removes data that cannot be
interpreted through the current schema.

### Delete orphan link-table relationships

Link-table relationships are stored as separate rows containing source and target endpoint
identifiers. A relationship row is orphaned when one or both required endpoint instances no
longer exist.

This phase reuses ECDb's integrity-checking rules to identify and delete orphan relationship rows.
It evaluates the allowed endpoint constraints and removes the relationship itself without
altering valid endpoint instances. Rows are handled set-wise so large groups of orphan
relationships do not require a separate application-level operation for each instance.

Removing these rows eliminates relationships that can no longer be traversed and prevents stale
relationship data from affecting queries or integrity checks.

### Nullify orphan navigation properties

Navigation properties store a reference from an owning instance to another EC instance. A
navigation value is orphaned when its target does not exist or when its stored relationship class
identifier is unknown.

This phase keeps the owning instance and clears the invalid navigation value. When both the
target identifier and relationship class identifier are physically stored, they are cleared
together so the navigation cannot remain partially populated. Virtual relationship-class
identifiers are not written.

The established BisCore root Model and ModeledElement exception is preserved because that root
reference has special iModel semantics.

This repair retains useful entities while removing references that cannot be resolved.

### Drop unused schemas

This phase identifies schemas that do not contribute definitions needed by surviving data or by
another retained schema.

Schemas containing classes with live instances are retained. The Optimizer then follows schema
references from every retained schema and preserves the full dependency closure. A schema is
eligible for deletion only when it is outside that retained set and existing schema-deletion
validation confirms that it can be removed safely.

BisCore, standard schemas, system schemas, and ECDb-owned profile schemas are always retained,
regardless of instance counts. Dependencies required by those protected schemas are retained as
well.

Dropping an unused schema removes obsolete classes, properties, relationships, mapping metadata,
views, and owned storage through the established ECDb schema-management lifecycle. This reduces
metadata size and removes definitions that can no longer describe meaningful data.

### Drop empty dynamic classes

Dynamic schemas are designed to permit controlled metadata changes after import. This phase
removes eligible dynamic classes that have no instances and are not required by surviving
definitions.

Class emptiness is evaluated with inheritance in mind. A base class is retained when a surviving
derived class still needs it. Class hierarchies selected for deletion are ordered so dependent
classes are removed before their bases. References from relationships, properties, custom
attributes, mixins, and other retained schema definitions can prevent deletion.

Only classes owned by dynamic schemas are considered. Protected schemas and their definitions are
never candidates. Unsupported or still-referenced classes are reported as skipped rather than
being removed through an unsafe cascade.

This phase reduces schema complexity while preserving the definitions needed to interpret all
surviving classes and instances.

### Drop empty dynamic properties

This phase removes supported properties from dynamic schemas when the property has no logical
value in any applicable instance.

A primitive property is empty only when its mapped value is null everywhere. A Point2d or Point3d
property is empty only when every physical component is null everywhere. Zero, false, an empty
string, and any other non-null value count as data and prevent deletion.

The usage check includes applicable concrete classes and mapped storage, including shared and
overflow columns. Properties that require semantics not yet supported by this cleanup are skipped.
These include arrays, navigation properties, compound structures, overrides, properties without
a usable physical mapping, and properties declared as not mapped.

Deleting an empty property removes its schema and property-mapping metadata through the existing
schema writer. It does not automatically remove the physical shared column, because another class
may still use that column. A later unmapped-column phase can remove the column after proving that
no mapping uses it.

This phase removes permanently empty schema surface while avoiding the loss of non-null values or
shared physical storage still needed elsewhere.

### Compact shared columns

ECDb can map properties from multiple classes into reusable shared columns. Over time, schema
changes can leave populated mappings in higher-numbered slots, empty mappings in lower slots, or
unnecessary use of overflow storage.

This phase reorganizes eligible property mappings toward compatible leftmost shared slots.
Mappings containing data are prioritized before mappings whose values are entirely null. Existing
shared slots are reused; the phase does not add columns, widen tables, or change the configured
shared-column threshold.

Compatibility includes the physical data type, collation, nullability, uniqueness, check
constraints, default constraints, table family, and other mapping requirements. Indexed columns,
trigger-sensitive tables, external views, protected mappings, and storage that cannot be proven
safe are pinned in place.

Point2d, Point3d, structs, and nested structures are treated as indivisible mapping groups. All
physical components of a property move together. If a complete group cannot fit in a compatible
destination table, the Optimizer attempts to move the complete group left within its current
table. If that is also unsafe, the group remains unchanged.

Compaction can move mappings within one table, from primary or joined storage to overflow storage,
or from overflow storage back to primary or joined storage. Original values are staged in
temporary storage before movement so swaps and cycles cannot overwrite a value that another move
still needs. Missing overflow destination rows are created from their parent rows when required.
A missing required primary destination is treated as an error rather than inventing an entity.

After values move, property-map metadata is updated to the new columns. Stale source cells and
unmapped class-and-column cells are explicitly set to null, while cells still used by another
class remain untouched. Affected class views are rebuilt from the final mappings.

The phase also cleans overflow rows. An overflow row is retained only when its stored class maps a
data property into that overflow table or derives from a class that does. Rows with null, unknown,
or unrelated class identifiers are removed. Dry-run planning uses projected destination mappings,
so it also identifies rows that will become obsolete after a mapping moves out of an overflow
table.

Compaction improves locality and can reduce unnecessary overflow access. It prepares unused
rightmost storage for later removal, but it does not change the physical ordinal of an existing
column and does not by itself shrink the database file.

### Drop unmapped tables

Schema deletion and remapping can leave an ECDb-owned physical table with no class mapped to it.
This phase removes such tables through ECDb's managed table-cleanup path.

Only tables owned and understood by ECDb are considered. Existing external tables, virtual tables,
the not-mapped sentinel, and tables still required by parent or child mappings are excluded.
Dependencies such as class views and mapping metadata are handled through the schema-management
lifecycle.

Dropping an unmapped table removes storage that can no longer contain valid EC data and avoids
carrying unused tables into future queries, backups, or schema operations.

### Drop unmapped shared columns

This phase physically removes shared columns that have no remaining property mapping.

Column discovery is independent of shared-column compaction. It can therefore be run after
property or schema cleanup even when compaction is not selected. Only physical shared columns in
ECDb-managed tables are candidates. Protected columns, mapped columns, system columns, and storage
with unsafe dependencies are retained.

Each selected column is removed through a strict schema-persistence operation. A failure to remove
a column stops the phase and is returned to the caller; the Optimizer does not silently report a
partial physical cleanup as success.

Removing columns requires SQLite to rewrite physical table storage and can be substantially more
expensive than metadata-only cleanup. The phase is separately timed so callers can measure that
cost.

This phase reduces unused physical width after mappings have been deleted or moved. Actual file
size reduction depends on SQLite page reuse and is not guaranteed without separate file-level
maintenance.

### Analyze the database

The final phase runs SQLite analysis for the main database after all selected cleanup is complete.
It refreshes query-planner statistics to reflect the remaining rows, tables, indexes, and data
distribution.

Analysis runs through the caller's active transaction and does not implicitly commit it. A dry run
reports the phase but does not update statistics.

Fresh statistics help SQLite choose plans that match the optimized database layout, especially
after substantial relationship deletion, table removal, or data movement.

## Option groups

The schema-cleanup group runs unused-schema cleanup, empty dynamic-class cleanup, and empty
dynamic-property cleanup.

The data-cleanup group removes orphan relationships, clears orphan navigation properties, and
purges rows with invalid class identifiers.

The storage-cleanup group compacts shared columns, drops unmapped tables, and drops unmapped
shared columns.

The all-options group runs every cleanup phase followed by database analysis.

## Performance characteristics

The Optimizer favors set-based database operations and temporary worklists over per-instance
application loops. Shared-column movement stages only affected values, and mapping discovery is
grouped around physical storage families.

Each phase records elapsed time, and major compaction activities have additional performance log
scopes for discovery, staging, data movement, and overflow cleanup. This makes it possible to
distinguish metadata analysis from potentially expensive physical operations such as dropping
columns or analyzing the database.

The most expensive phases are generally shared-column compaction, physical column removal, and
analysis. Applications can omit those phases when they only need logical integrity repair or
schema cleanup.

## Operational guidance

Run a dry run first when the application needs to review the expected scope or estimate cost.
Execute optimization while the connection has exclusive maintenance use and no caller-owned
statements are active.

Because optimization can delete metadata and data, keep the operation inside a caller-owned
transaction until the result has been reviewed. Save the transaction only after every requested
phase succeeds. Abandon it when a failure occurs unless retaining partial work is an explicit
application decision.

Optimization is not a replacement for validation of an unreadable or fundamentally incompatible
database. Ambiguous mappings, unsupported metadata, unsafe dependencies, authorization failures,
and SQLite errors are surfaced rather than bypassed.

# SchemaEditor

`SchemaEditor` is a C++ command interface for editing an ECDb schema without
passing ECObjects objects to an import operation. It accepts key/value strings
or JSON arguments and persists edits to the existing `ec_*` metadata tables.
The implementation does not call `ImportSchemas`, `SchemaWriter`, or the bulk
`MapSchemas` pipeline. It uses ECDb's low-level class maps, remapping, and database
infrastructure.

## Transaction contract

- `Begin` requires an open, writable ECDb with a clean default transaction.
  Pending writes and nested savepoints are rejected without abandoning them.
- An active editor excludes another editor, bulk schema import, and schema drop
  on the same connection.
- Each successful command completes metadata persistence, mapping, physical
  schema changes, and any permitted mapping-induced data movement before returning.
  It does **not** commit the transaction.
- Any `Execute` failure abandons the entire session, including earlier commands.
  That editor cannot execute more commands or save changes.
- `Validate` checks arguments and the proposed metadata without writing.
  A failed dry run does not abandon the session. Physical mapping and data-move
  requirements can still cause `Execute` to fail.
- `SaveChanges` validates persisted mappings and commits through
  `ECDb::SaveChanges`. Explicit abandonment or destruction of an unfinished editor
  abandons the transaction.

The connection must outlive the editor. While a session is open, callers must
not independently commit, abandon, close, or write to the connection, including
from schema-change event callbacks. The editor owns the default transaction,
not a nested savepoint. SchemaSync databases are not supported. This API is
ECDb-only; it does not perform the additional DgnDb domain/transaction work.

## C++ usage

```cpp
#include <ECDb/SchemaEditor.h>

SchemaCommandResult CreateEquipmentSchema(ECDbR db)
    {
    SchemaCommandResult result;
    auto editor = SchemaEditor::Begin(db, SchemaImportOptions::None, nullptr, &result);
    if (!editor)
        return result;

    result = editor->Execute("CreateSchema",
        {{"name", "Equipment"}, {"alias", "eq"}, {"version", "01.00.00"}});
    if (!result.IsOk())
        return result;

    result = editor->Execute("CreateEntityClass",
        {{"schema", "Equipment"}, {"name", "Pump"}});
    if (!result.IsOk())
        return result;

    result = editor->Execute("AddProperty",
        {{"class", "eq:Pump"}, {"name", "Code"}, {"kind", "primitive"}, {"type", "string"}});
    if (!result.IsOk())
        return result;

    return editor->SaveChanges();
    }
```

`GetCommandNames` returns the registered operations. Convenience methods forward
to the same execution path. A token-protected connection requires its
`SchemaImportToken` at `Begin`.

The registered command families are:

| Item | Commands |
| --- | --- |
| Schema | `CreateSchema`, `AlterSchema`, `DropSchema` |
| Entity | `CreateEntityClass`, `AlterEntityClass`, `DropEntityClass` |
| Struct | `CreateStructClass`, `AlterStructClass`, `DropStructClass` |
| Relationship | `CreateRelationshipClass`, `AlterRelationshipClass`, `DropRelationshipClass` |
| Mixin | `CreateMixin`, `AddMixin`, `RemoveMixin` |
| Custom attribute class | `CreateCustomAttributeClass`, `AlterCustomAttributeClass`, `DropCustomAttributeClass` |
| Property | `AddProperty`, `AlterProperty`, `DropProperty` |
| Enumeration | `CreateEnumeration`, `AlterEnumeration`, `DropEnumeration`, `AddEnumerator`, `DropEnumerator` |
| Custom attribute | `SetCustomAttribute`, `RemoveCustomAttribute` |
| Unit system | `CreateUnitSystem`, `AlterUnitSystem` |
| Phenomenon | `CreatePhenomenon`, `AlterPhenomenon` |
| Unit | `CreateUnit` (regular, `isConstant:true` constant, or `invertsUnit` inverted unit), `AlterUnit` |
| Format | `CreateFormat`, `AlterFormat` |
| Kind of quantity | `CreateKindOfQuantity`, `AlterKindOfQuantity`, `DropKindOfQuantity` |
| Property category | `CreatePropertyCategory`, `AlterPropertyCategory`, `DropPropertyCategory` |

## JSON

```cpp
BeJsDocument args(R"({"class":"eq:Pump","name":"Pressure","kind":"primitive","type":"double"})");
auto result = editor->Execute("AddProperty", args);

BeJsDocument command(R"({
  "command":"AddProperty",
  "args":{"class":"eq:Pump","name":"Serial","kind":"primitive","type":"string"}
})");
result = editor->Execute(command);
```

`ExecuteBatch` accepts an array of command envelopes. It stops on the first
failure and abandons **all** session changes, not just that batch. The result
contains the failing command index. Native JSON uses nested objects for dotted
key/value paths, booleans, numbers, and arrays. Malformed values, duplicate keys,
and conflicting paths are errors, not defaults.

`SchemaCommandResult::ToJson` serializes the status, issues, failing command, and
blocking instance keys. Issues identify the argument or JSON path when known;
errors raised by low-level mapping are retained alongside command errors.

## Evolution and safety

Schema versions are explicit. Before an operation requiring a major upgrade,
execute `AlterSchema` to increase the read version. The editor compares against
the versions captured at `Begin`; it never bumps a version automatically.
Session options can disallow major upgrades.

Cross-schema references are added implicitly. Unused references are resolved
from a detached schema copy and pruned after applying an edit. Referenced
schemas must already be persisted and cycles are rejected.

Drops do not implicitly delete instance data. Blocking instance keys are
returned with `ERROR_DATA_DELETION_REQUIRED`. Mapping-induced data movement
requires `AllowDataTransformDuringSchemaUpgrade`; otherwise the command returns
`ERROR_DATA_TRANSFORM_REQUIRED` and the whole session is abandoned.

## Mapping and upgrade rules

The mapper selects affected classes, their descendants, transitive struct users,
and navigation-property owners. It does not iterate every class in every
affected schema. Mapping persistence, table/index reconciliation, and overflow
row creation complete in the command. Full persisted-map validation runs at
`SaveChanges`.

`minLength`, `maxLength`, `minValue`, and `maxValue` accept JSON `null` to
clear an existing bound. Omitting a bound leaves it unchanged.

Legacy enumerator renames use the persisted original ECXML version at the start
of the editor session. An import can therefore upgrade ECXML 3.1 to 3.2 and
assign explicit names in the same session; subsequent sessions enforce the
persisted ECXML 3.2 rename restriction.

Removing a local property override does not require a major schema upgrade when
the inherited property and its mappings preserve the values. Removing the
property itself still requires a major upgrade and the editor's data checks.

Adding a local override of an inherited property uses the native remap collector
even when no descendant declares an override. It replaces the old inherited
mapping identities for the class and its descendants before persisting the new
maps, preserving existing values across separate commands and mapping batches.
When persisting an inherited map cloned during loading, the mapper reuses an
existing mapping for the same class and access string, retaining its native
property-path identity rather than inserting a duplicate under a newer override.

KoQ override compatibility follows ECObjects: a missing KoQ on either side is
compatible; persistence units must match when both sides have a KoQ. A base
property can therefore gain a KoQ before its persisted derived overrides do,
including across standalone commands. Existing unit-change restrictions still apply.

Schema policy exceptions are also checked by qualified name for new items that do
not yet have persisted IDs; validation does not allocate IDs to resolve exceptions.

The implemented upgrade rules match `SchemaWriter` and `CustomAttributeValidator`:

| Area | Supported upgrades and enforced restrictions |
| --- | --- |
| `AlterProperty` | Supports primitive `type` upgrades (excluding `Point2d`/`Point3d` and strict enumerations, gated by major version bump and data transform when columns change), `enum`, `koq` (same persistence unit, or unit change/removal when `SchemaUpgradeCustomAttributes:AllowUnitChange` is present), `category`, `minLength`, `maxLength`, `minValue`, `maxValue`, `extendedType`, `readOnly` (preventing `readOnly=false` on a `ClassHasCurrentTimeStampProperty` target), `priority`, `label`, and `description`. Changing `minOccurs`/`maxOccurs` on array properties is rejected. |
| `AddProperty` on base class | Validates derived-class property overrides across the hierarchy: rejects incompatible types/KoQs, upgrades compatible primitive types (with major version bump), and collects `RemapManager` column moves when derived properties are converted or moved out of conflicting physical columns. |
| `AlterRelationshipClass` | Enforces `UpdateRelationshipConstraint` rules: `strength` and `direction` are immutable; `multiplicity` lower bound is immutable and upper bound may only increase (respecting FK end-table orientation); `polymorphic` may change `false -> true`; `classes` may be generalized to a base class that supports all existing constraint classes; `roleLabel` and `allowDuplicateRelationships` (on link tables) may be updated. |
| `CreateRelationshipClass` | Supports `mapAs=foreignKey|linkTable`, `baseClass`, `queryView`, `foreignKeyView`, and cross-schema `navProperty.class` (automatically adding the schema reference and validating both schemas). |
| Base-class and mixin changes | Modifying an entity base class (when the new base class is derived from the existing base class) or applying a mixin (`AddMixin`) validates and upgrades compatible local properties across the hierarchy that become overrides, and triggers `RemapManager` to move data. Adding a non-mixin base class to an existing root class or removing a base class is rejected, matching `SchemaWriter::UpdateBaseClasses`. |
| Custom-attribute upgrades | `ECDbMap:ClassMap`, `ShareColumns`, `JoinedTablePerDirectSubclass`, `SchemaMap`, `PropertyMap`, `ForeignKeyConstraint`, `ForeignKeyView`, and `CoreCustomAttributes:ClassHasCurrentTimeStampProperty` are create-only. `DbIndexList` allows adding indexes and modifying `Indexes.Properties` (both JSON arrays and `name:prop1+prop2[:unique][:where=...]` KV strings are accepted). `LinkTableRelationshipMap` allows modifying `AllowDuplicateRelationships`. `QueryView` allows modifying an existing view when a major version upgrade is allowed. `CoreCustomAttributes:IsMixin` allows generalizing `AppliesToEntityClass` to a base class. `SetCustomAttribute` and `RemoveCustomAttribute` support schema, class, property, and relationship constraint (`Schema:RelClass.source` / `Schema:RelClass.target`) containers. |
| Units, formats, KoQs, categories | Argument keys mirror ECJSON 3.2 (`label`, `description`, `definition`, `numerator`, `denominator`, `offset`, `phenomenon`, `unitSystem`, `isConstant`, `invertsUnit`, `type`, `precision`, `formatTraits`, `composite.{spacer,includeZero,units}`, `persistenceUnit`, `relativeError`, `presentationUnits`, `priority`); item references accept `Schema:Item` or `Schema.Item` and add the schema reference automatically. Unit-family commands require an EC3.2 file. Rows are written exactly as `SchemaWriter` imports them (`SerializeNumericSpec`, `SerializeCompositeSpecWithoutUnits`, `SerializeKoqPresentationFormats`). Following `SchemaWriter::Update*`: unit systems and phenomena only change `label`/`description`; units additionally accept `numerator`/`denominator`/`offset` only when previously undefined and set to the default (1/1/0); formats may change `label`, `description`, the numeric spec (passed as a whole, `type` and `precision` required), `composite.spacer`, `composite.includeZero` and composite unit labels, but never the composite units; KoQs may change `label`, `description`, `relativeError` (not remove) and `presentationUnits`; categories may change `label`, `description`, `priority`. Renames and other changes are rejected (`SchemaEditor.UnsupportedOperation`); passing an immutable attribute with its current value is a no-op. Units, phenomena, unit systems and formats cannot be dropped. `DropKindOfQuantity` requires a major version upgrade; `DropPropertyCategory` does not. Both refuse while a property still references the item. |
| Drops | `DropSchema`, `DropEntityClass`, `DropRelationshipClass`, `DropProperty`, and `RemoveMixin` refuse if instances or non-null column values exist (`ERROR_DATA_DELETION_REQUIRED` with blocking instance keys). Deleting a persisted enumerator is rejected, matching `SchemaWriter::DeleteEnumerationValue`. |

Callers must check each command result; registration of an `Alter*` operation
enforces ECDb's schema evolution invariants before mutating metadata or mappings.

## Importing ECSchemas through commands

`SchemaImportAdapter` (`<ECDb/SchemaImportAdapter.h>`) accepts the same input as
`SchemaManager::ImportSchemas` and executes it as an ordered stream of editor
commands. Schemas that do not exist in the file become creation commands. For
schemas that already exist, only the changes are generated. Schemas absent
from the input are never dropped.

```cpp
SchemaImportPlan plan;
auto result = SchemaImportAdapter::BuildPlan(db, schemas, plan, SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);
if (!result.IsOk())
    return result;
// plan.GetSteps()/plan.ToJson() expose the ordered commands for preview.
SchemaImportCompletion completion;
result = SchemaImportAdapter::ExecutePlan(db, plan, &completion);
if (result.IsOk())
    completion.Commit(); // or completion.Abandon()
```

- `BuildPlan` is read-only. It deep-copies and supplements the incoming
  reference graph without mutating the caller's schemas, compares the copies
  with the persisted schemas, and orders the commands deterministically. The
  ordering is independent of input order:
  - definitions are created before they are used;
  - base classes, mixin targets and relationship constraints come before the
    classes that depend on them;
  - consumers are removed before the definitions they use.

  Each new class is created in one atomic step together with its local
  properties (in declared order) and its creation-only custom attributes.
  Navigation properties, and new foreign-key relationships whose navigation
  belongs to the class, are part of that step. Cyclic navigation/constraint
  dependencies form atomic groups: class shells are created before their
  members, and mapping runs after the complete group. Polymorphic mixin
  constraints include their implementing classes as mapping prerequisites.
  Property and custom-attribute ordinals retain their source order.
  Property additions within a group do not rebuild the unchanged inheritance
  and class-to-table cache tables between children. Schema-reader caches are
  still invalidated after each child, and both cache tables are refreshed
  before mapping the completed group. Policy validation queries property IDs
  only for navigation properties with an explicit foreign-key constraint under
  an opted-in foreign-key policy.
  Consecutive property creations in the same schema reuse an owned working
  graph after each successful metadata write, instead of loading and copying
  the whole schema again. Other commands, a different schema, derived-property
  type upgrades, and group completion invalidate that cached state. Dry-run
  validation never reuses it, and no persisted-reader pointers are retained
  across cache clears.
  The same property-creation run also reuses its persisted policy snapshot;
  policy IDs and exception names are owned values, not schema-reader pointers.
  Other commands invalidate that snapshot after applying their metadata,
  including changes to schema policy custom attributes.
  Kind-of-quantity presentation formats are resolved against the owned graph,
  including unit overrides, so cached formats do not reference reader-owned
  units. Reference pruning skips schema loading/copying when every persisted
  reference is explicitly pinned (or there are no references).
- `ExecutePlan` requires a clean transaction and rejects a plan whose persisted
  schema baseline has changed since it was built (`SchemaImport.StalePlan`).
  It runs every step through one editor session and ends with
  `SchemaEditor::Finish`, which validates the mappings but does **not** commit.
  The caller commits or abandons through the completion handle. If the caller
  instead calls `db.AbandonChanges()` directly, it must then call
  `ClearECDbCache()`. If any step fails, all earlier steps are rolled back. The
  issue names the step index, the facet and the qualified item.
- Native upgrade rules apply unchanged:
  - map strategies are immutable, and an entity cannot change into another
    class kind;
  - persisted enumerators cannot be deleted;
  - units, formats, phenomena and unit systems cannot be dropped;
  - data movement requires `AllowDataTransformDuringSchemaUpgrade`;
  - drops refuse while instances or values exist.

  An older or equal minor version of a persisted schema is skipped, as it is
  in native import.
- Persisted metadata matches a native import of the same input. The physical
  column layout also matches for new schemas and for upgrades that do not move
  data. When an upgrade moves existing values (for example, a struct grows into
  the overflow table), the adaptor can reuse the freed shared columns in a
  different, more compact order than the native single-pass mapping.
- Not supported: SchemaSync and DgnDb domain processing,
  `DoNotCreateOrUpdateDataTables`, `DoNotFailForDeletionsOrModifications`,
  `NotMapped` relationships, dropping a mixin, a negative property-category
  priority, and `ForeignKeyConstraint.OnUpdateAction`.

An omitted `ForeignKeyConstraint.OnDeleteAction` retains native inference
(for example, cascade for embedding relationships); it is not replaced with
`NoAction`. The command argument `fkOnDelete=Default` creates the attribute
without an explicit action. `NoAction`, `Cascade`, and `SetNull` remain explicit.

`SetCustomAttribute` accepts an optional `ordinal` to swap an existing attribute
with the local attribute at that position. This mode requires an unchanged
instance and cannot bypass immutable mapping-attribute rules. The adaptor uses
it to restore source order after dependency-sensitive attribute creation.

### Ad-hoc exported schema history

The disabled `AdHoc/SchemaImportHistoryTests.DISABLED_ReplayExportedChangesets`
test runs a manifest-based export through native import and the adaptor in
separate fresh databases:

```sh
ECDB_SCHEMA_CHANGESET_DIR=/path/to/test-sc ./ECDbTest \
  --timeout=-1 --gtest_also_run_disabled_tests --gtest_filter='AdHoc/SchemaImportHistoryTests.*'
```

`--timeout=-1` disables the runner's default 30-minute suite timeout. Large
snapshots can take substantially longer through the command path. Do not run
another `ECDbTest` process from the same executable directory concurrently:
the runner resets its shared output directory at startup.

For a native-only benchmark, use a Release-built executable and the exact filter
`AdHoc/SchemaImportHistoryTests.DISABLED_ReplayExportedChangesets/Native`.
The replay prints per-snapshot import-and-commit time, cumulative import-and-commit
time, and elapsed replay time including schema reading, reopening, and checks.
On a native import failure it also prints the failed attempt's duration; the
cumulative successful import time excludes that attempt.

It processes numeric index folders starting at `0`, reconstructs each snapshot
from its manifest and earlier retained files, commits and reopens after each
import, and prepares representative BisCore ECSQL queries. It stops each import
path at its first missing snapshot, read error, or import failure and reports the
last successful index. Data transforms and dynamic-schema major upgrades are
enabled; model data changesets are not applied. Use a completed export so its
directory listing and manifest chain remain stable during the run.

To collect failures beyond the first rejected adaptor upgrade, set
`ECDB_SCHEMA_CONTINUE_AFTER_FAILURE=1` with the exact
`AdHoc/SchemaImportHistoryTests.DISABLED_ReplayExportedChangesets/SchemaEditor`
filter. This diagnostic mode records every adaptor rejection, reopens the rolled-back
database, and imports the failed snapshot natively to establish the next baseline.
If the native upgrade also fails, it reports that failure and attempts a fresh native
import of the snapshot in a new database. Such a reset loses prior physical mapping
history; neither a native recovery nor a fresh baseline counts as an adaptor success.
Manifest, recovery, commit, and query-check failures still stop the scan. The final
summary and XML properties distinguish attempted snapshots, adaptor successes and
failures, native recoveries, and fresh baselines. Any adaptor rejection fails the test,
even if all snapshots were attempted. This is not a successful uninterrupted replay
or a comparable adaptor-only timing benchmark.

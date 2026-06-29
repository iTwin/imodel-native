# Feature-Driven ECSchema Evolution

## Table of Contents

1. [Overview](#1-overview)
2. [Motivation](#2-motivation)
3. [Multi-Zone Split of ECVersion: The Three-Zone Model](#3-multi-zone-split-of-ecversion)
4. [ECXml Parsing Forward-Compatibility (ecobjects layer)](#4-ecxml-parsing-forward-compatibility-ecobjects-layer)
   - [1.1 : Strict/Tolerant Schema Loading](#11--stricttolerant-schema-loading)
   - [1.3 : Minimum-Version Write Enforcement](#13--minimum-version-write-enforcement)
   - [1.5 : Test Coverage](#15--test-coverage)
5. [The `ec_feature` Table (ECDb layer)](#5-the-ec_feature-table-ecdb-layer)
   - [5.1 : Table Structure and Profile Bump](#51--table-structure-and-profile-bump)
   - [5.2 : The Two-Registry Model](#52--the-two-registry-model)
   - [5.3 : Compatibility Matrix](#53-compatibility-matrix)
6. [Design Decisions and Rationale](#6-design-decisions-and-rationale)
7. [Future Work (TODOs)](#7-future-work-todos)

---

## 1. Overview

This feature introduces **forward-compatible ECSchema parsing** across the ecobjects and ECDb layers.
Its goals are:

- Let a newer iModel-native binary read an older iModel whose schemas were written by the same or an older binary : **without silently dropping data**.
- Let an older iModel-native binary read an iModel that was written by a newer binary : **without crashing or corrupting data** : by gracefully handling unknown EC constructs rather than treating them as hard errors.
- Establish a versioned **`ec_feature` table** in ECDb so a binary can declare which optional EC features it has activated, and an opening binary can decide whether it is capable of handling them.

---

## 2. Motivation

Previously, any ECXml namespace beyond the version this binary was compiled for caused an immediate `InvalidECSchemaXml` failure in `ReadSchemaStub`. This was overly conservative: a client using a schema with a new attribute it doesn't understand shouldn't have to fail entirely : it should be able to proceed with the known information.

At the same time, ECDb needs to remain strict: it should never import a schema whose ECXml version is newer than what this binary can fully represent, because the stored iModel might then be opened by a capable binary expecting round-trip fidelity.

These two requirements (lenient reader for schema introspection / strict gate for ECDb import) are now separated by a flag on `ECSchemaReadContext`.

---

## 3. Multi-Zone Split of ECVersion

ECXml versions are partitioned into three zones:

| `ECVersion` | Behavior |
|---|---|
`[V3_1 ... Latest=V3_2]`  |            Fully supported: parsed AND re-serializable.
`(Latest ... MaxParsable=V3_3]` |       Parseable only: loaded tolerantly/strictly but blocked from re-serialization and ECDb import.
`> MaxParsable`   |   Rejected immediately when reading XML in strict mode. In tolerant mode, parsed best-effort (unknown constructs silently defaulted). Never stored or serialized.

| `ECVersion` constant | Value | Role |
|---|---|---|
| `Latest` | `V3_2` | Maximum version that can be imported into ECDb and round-tripped (read + re-written). |
| `MaxParsable` | `V3_3` | Maximum version strict mode will accept. Tolerant mode has no upper ceiling. |

**Tolerant mode** (default): no version ceiling. Schemas of any version are parsed best-effort
unknown attributes and elements are silently ignored/defaulted.
Used for schema inspection, migration tooling, and any context where refusing to open is worse than silently degrading.

**Strict mode** (`SetStrictSchemaValidation(true)`): ceiling at `MaxParsable`.
Schemas beyond V3_3 are rejected at `ReadSchemaStub`. Within the parseable range, unknown attribute *names*
and unknown child elements cause `InvalidECSchemaXml`.

**ECDb import gate** (`OriginalECXmlVersionGreaterThan(Latest)` in `PreprocessSchemas`):
Independent of the strict flag. Rejects any schema whose original ECXml version is above
`Latest` (currently V3_2), even if strict mode accepted it at the parsing stage.

---

## 4. ECXml Parsing Forward-Compatibility (ecobjects layer)

### 1.1 : Strict/Tolerant Schema Loading

#### A flag to control strictness to maintain legacy behavior

```cpp
// New private member (default false = tolerant):
bool m_strictSchemaValidation = false;

// New public API:
bool GetStrictSchemaValidation() const { return m_strictSchemaValidation; }
void SetStrictSchemaValidation(bool strict) { m_strictSchemaValidation = strict; }
```

The flag is `false` by default. All existing call sites continue to operate in tolerant mode without code changes.

#### Construct-level strict checks

Unknown XML attributes and child elements at the construct level are now flagged when strict mode is active. The pattern in every reader is:

```cpp
if (context.GetStrictSchemaValidation())
    {
    static const std::set<Utf8String> s_knownAttributes = { /* known attr names */ };
    if (Utf8CP unknown = SchemaParseUtils::FindFirstUnknownXmlAttribute(node, s_knownAttributes))
        {
        LOG.errorv("Invalid ECSchemaXML: Unknown attribute '%s' on ...", unknown, ...);
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    }
```

### 1.3 : Minimum-Version Write Enforcement

#### `GetRequiredECVersion()`

New public method on `ECSchema`. Returns the minimum ECXml version needed to faithfully serialize this schema:

```cpp
ECVersion ECSchema::GetRequiredECVersion() const {
    ECVersion required = ECVersion::V3_2;   // ECDb minimum; all V3.2 content covered by floor
    for (auto const& ref : GetReferencedSchemas())
        required = std::max(required, ref.second->GetECVersion());
    // TODO: Add V3_3 check for EC 3.3 constructs when introduced
    return required;
}
```

ECDb requires ECXml 3.2 as its floor. Reference propagation ensures that a schema referencing a V3_3 schema inherits that requirement.
This will allow a schema to be serialized to a version that fully supports all the constructs/features in the schema.

Practical use case example:
Consider a future where the current latest supported version is ECXml.3.5.
This runtime serializes a schema which contains all 3.2 supported features.
Without this check, the runtime would have serialized the schema to the latest version (i.e. 3.5).
Such a schema would not have been readable by the current ECDb runtime for which 3.5 > current MaxParsable (3.3), even though the schema contains only 3.2 constructs which this runtime supports.

---

### 1.5 : Test Coverage

Tests added to the ECObjects layer, organized by file and (for `StrictSchemaValidationTests.cpp`) by ECXml version zone.

#### `StrictSchemaValidationTests.cpp`

**Utility**

| Test | Expected Output |
|---|---|
| `FlagDefaultAndSetterGetter` | Default is `false` (tolerant); setter/getter round-trips correctly. |

**Zone 1 - version = Latest (V3_2) - Construct-level check is the only defence**

| Test | ECObject | Tolerant | Strict | Notes |
|---|---|---|---|---|
| `ValidCurrentVersionSchema_BothModesSucceed` | All | `Success` | `Success` | Fully-valid V3_2 schema; no unknown constructs. |
| `KnownClassModifier_StrictSucceeds` | `ECEntityClass` | `Success` | `Success` | Known attributes are never rejected; modifier reads back as `Sealed`. |
| `UnknownAttributeOnPropertyCategory_TolerantSucceeds` | `PropertyCategory` | `Success` | - | Unknown attr silently ignored; `"TestCat"` present in schema. |
| `UnknownAttributeOnPropertyCategory_StrictFails` (Expected to Fail) | `PropertyCategory` | - | `InvalidECSchemaXml` | Construct-level check (**not yet implemented**). |
| `UnknownAttributeOnUnitSystem_TolerantSucceeds` | `UnitSystem` | `Success` | - | Unknown attr silently ignored; `"SI"` present in schema. |
| `UnknownAttributeOnUnitSystem_StrictFails` (Expected to Fail) | `UnitSystem` | - | `InvalidECSchemaXml` | Construct-level check (**not yet implemented**). |
| `UnknownAttributeOnPhenomenon_TolerantSucceeds` | `Phenomenon` | `Success` | - | Unknown attr silently ignored; `"LENGTH"` present in schema. |
| `UnknownAttributeOnPhenomenon_StrictFails` (Expected to Fail) | `Phenomenon` | - | `InvalidECSchemaXml` | Construct-level check (**not yet implemented**). |
| `UnknownAttributeOnUnit_TolerantSucceeds` | `ECUnit` | `Success` | - | Unknown attr silently ignored; unit `"M"` present in schema. |
| `UnknownAttributeOnUnit_StrictFails` (Expected to Fail) | `ECUnit` | - | `InvalidECSchemaXml` | Construct-level check (**not yet implemented**). |
| `UnknownAttributeOnFormat_TolerantSucceeds` | `ECFormat` | `Success` | - | Unknown attr silently ignored; format `"DefaultRealU"` present in schema. |
| `UnknownAttributeOnFormat_StrictFails` (Expected to Fail) | `ECFormat` | - | `InvalidECSchemaXml` | Construct-level check (**not yet implemented**). |
| `UnknownAttributeOnKindOfQuantity_TolerantSucceeds` | `KindOfQuantity` | `Success` | - | Unknown attr silently ignored; KoQ `"Length"` present in schema. |
| `UnknownAttributeOnKindOfQuantity_StrictFails` (Expected to Fail) | `KindOfQuantity` | - | `InvalidECSchemaXml` | Construct-level check (**not yet implemented**). |
| `UnknownAttributeOnSchemaReference_TolerantSucceeds` | `ECSchemaReference` | `Success` | - | Unknown attr silently ignored; referenced schema resolves. |
| `UnknownAttributeOnSchemaReference_StrictFails` (Expected to Fail) | `ECSchemaReference` | - | `InvalidECSchemaXml` | Construct-level check (**not yet implemented**). |

**Zone 2 - Latest < version ≤ MaxParsable (V3_3) - Version gate passes; construct-level check is operative**

| Test | ECObject | Tolerant | Strict | Strict failure gate |
|---|---|---|---|---|
| `MaxParsableVersion_UnknownAttributeName_TolerantSucceeds` | `ECEntityClass` | `Success`; `"TestClass"` present | - | No gate - construct checks off in tolerant mode. |
| `MaxParsableVersion_UnknownAttributeName_StrictFails` | `ECEntityClass` | - | `InvalidECSchemaXml` | **Construct-level check** (version gate passes: V3_3 ≤ MaxParsable). |

**Zone 3 - version > MaxParsable (V3_99) - Version gate fires in strict mode**

| Test | ECObject | Tolerant output | Strict output | Strict failure gate |
|---|---|---|---|---|
| `UnknownClassModifier_TolerantSucceeds` | `ECEntityClass` | `Success`; modifier defaults to `None` | - | - |
| `UnknownClassModifier_StrictFails` | `ECEntityClass` | - | != `Success` | **Version gate** (`ecVersion > MaxParsable`) |
| `UnknownRelationshipStrength_TolerantSucceeds` | `ECRelationshipClass` | `Success`; strength defaults to `Referencing` | - | - |
| `UnknownRelationshipStrength_StrictFails` | `ECRelationshipClass` | - | != `Success` | **Version gate** |
| `UnknownEnumBackingType_TolerantSucceeds` | `ECEnumeration` | `Success`; type defaults to `String` | - | - |
| `UnknownEnumBackingType_StrictFails` | `ECEnumeration` | - | != `Success` | **Version gate** |
| `UnknownEnumChildElement_TolerantSucceeds` | `ECEnumeration` | `Success`; unknown `<FutureElement>` skipped; 1 enumerator remains | - | - |
| `UnknownEnumChildElement_StrictFails` | `ECEnumeration` | - | != `Success` | **Version gate** |
| `UnknownPropertyType_TolerantSucceeds` | `ECProperty` | `Success`; property type defaults to `string` | - | - |
| `UnknownPropertyType_StrictFails` | `ECProperty` | - | != `Success` | **Version gate** |
| `MultipleUnknownConstructs_TolerantSucceeds` | Multiple | `Success`; 1 class, 1 enum | - | - |
| `MultipleUnknownConstructs_StrictFails` | Multiple | - | != `Success` | **Version gate** |

| Test | Description |
|---|---|
| `GetRequiredECVersion_EmptySchema_ReturnsV32` | Empty schema returns the V3_2 baseline. |
| `GetRequiredECVersion_KoQOnly_ReturnsV32` | KindOfQuantity alone doesn't bump above V3_2. |
| `GetRequiredECVersion_SchemaWithUnits_RequiresV32` | Units (V3.2 feature) returns V3_2. |
| `GetRequiredECVersion_SchemaWithFormat_RequiresV32` | Formats (V3.2 feature) returns V3_2. |
| `GetRequiredECVersion_SchemaWithNamedEnumerators_RequiresV32` | Named enumerators (V3.2 feature) returns V3_2. |
| `GetRequiredECVersion_ReferenceToV33Schema_RequiresV33` | Reference to a V3_3 schema propagates V3_3 requirement. |
| `WriteToXml_MinimumVersionUsedNotCeiling` | Writing V3_2 schema with V3_3 ceiling produces `3.2` namespace, not `3.3`. |
| `WriteToXml_FailsWhenRequiredVersionExceedsCeiling` | Writing a V3_3-required schema with V3_2 ceiling returns `FailedToSaveXml`. |
| `ECVersion3_3_SchemaVersionValidation` | A programmatically created V3_3 schema passes `BaseECValidator`. |

#### ECObjects strict/lenient loading tests

| Test | Description |
|---|---|
| `StrictLoading_UnknownAttributeOnEntityClass` | Unknown attribute on a V3.2 class fails in strict mode. |
| `LenientLoading_UnknownAttributeOnEntityClass_Succeeds` | Same attribute is silently ignored in lenient mode. |
| `StrictLoading_MaxParsableVersionAccepted` | Exactly V3_3 (`MaxParsable`) passes strict mode. |
| `StrictLoading_BeyondMaxParsableVersionRejected` | V3_4+ is rejected in strict mode at `ReadSchemaStub`. |
| `LenientLoading_BeyondMaxParsableVersionAccepted` | V3_4+ is accepted in lenient mode, but re-serialization fails. |

#### `StrictSchemaImportTests.cpp`

**Zone 1 - version = Latest (V3_2) - Import gate passes; construct-level check is operative**

| Test | ECObject | Expected Output | Failure gate |
|---|---|---|---|
| `ValidCurrentVersionSchema_Succeeds` | All | `SUCCESS` | N/A - fully-valid schema. |
| `CurrentVersionSchemaWithUnknownModifier_Rejected` | `ECEntityClass` | `ERROR` | **Construct level** (ecobjects parser; import gate passes for V3_2). |
| `CurrentVersionSchemaWithUnknownPropertyType_Rejected` | `ECProperty` | `ERROR` | **Construct level** |
| `CurrentVersionSchemaWithUnknownEnumBackingType_Rejected` | `ECEnumeration` | `ERROR` | **Construct level** |
| `CurrentVersionSchemaWithUnknownStrength_Rejected` | `ECRelationshipClass` | `ERROR` | **Construct level** |

**Zone 2 - Latest < version ≤ MaxParsable (V3_3) - Import gate fires**

| Test | ECObject | Expected Output | Failure gate |
|---|---|---|---|
| `MaxParsableVersion_ValidSchema_ImportRejected` | `ECEntityClass` | `ERROR` | **ECDb import gate** (V3_3 > Latest=V3_2). |

**Zone 3 - version > MaxParsable (V3_99) - Import gate fires**

| Test | ECObject | Expected Output | Failure gate |
|---|---|---|---|
| `FutureVersionSchemaWithUnknownModifier_Rejected` | `ECEntityClass` | `ERROR` | **ECDb import gate** (ecobjects tolerant parse silently defaults unknown modifier). |
| `FutureVersionSchemaWithUnknownStrength_Rejected` | `ECRelationshipClass` | `ERROR` | **ECDb import gate** |
| `FutureVersionSchemaWithUnknownEnumType_Rejected` | `ECEnumeration` | `ERROR` | **ECDb import gate** |
| `FutureVersionSchemaWithUnknownPropertyType_Rejected` | `ECProperty` | `ERROR` | **ECDb import gate** |
| `FutureVersionSchemaWithOnlyKnownConstructs_StillRejected` | `ECEntityClass` | `ERROR` | **ECDb import gate** - gate is version-based; construct content is irrelevant. |

---

## 5. The `ec_feature` Table (ECDb layer)

### 5.1 - Table Structure and Profile Bump

A new `ec_Feature` table was added to every ECDb file at the new profile version `4.0.0.6`.

**DDL:**
```sql
CREATE TABLE ec_Feature (
    Name        TEXT PRIMARY KEY NOT NULL COLLATE NOCASE,
    Description TEXT NOT NULL,
    Compat      TEXT NOT NULL CHECK(Compat IN ('Warn', 'ReadOnly', 'Refuse'))
);
```

---

### 5.2 The Two-Registry Model

The feature system operates across two completely separate data stores that answer two distinct questions:

| Data store |Populated by | Answers |
|---|---|---|
| Binary (compiled-in) | Developer, at code-write time via `RegisterKnownFeature` when adding a new feature to ECDb | "What features does **this runtime** understand?" |
| `ec_Feature` table in `.ecdb` on disk |`InsertFeature()`, called at runtime | "What features are actually **in use in this specific file**?" |

Each entry describes one feature:

```cpp
struct FeatureInfo {
    Utf8CP name;        // canonical string key,  e.g. "json-primitive-type"
    Utf8CP description; // human-readable description,  e.g. "JSON Primitive Types"
    Compat compat;      // policy for older binaries (see below)
};
```

#### The role of `Compat` in `FeatureInfo`

`Compat` in the runtime registry is **not** enforcement policy for the current binary. Any feature in `s_knownFeatures` is by definition understood and fully supported. The current binary always opens such files normally, regardless of the `Compat` value stored in the file.

`Compat` is purely **outward-facing metadata**: it is the value that `InsertFeature` writes into the `ec_feature.Compat` column, which then governs the behavior of **older binaries** that open the ECDb file without knowing the feature.

> *"If a binary that has never heard of this feature opens a ECDb file that uses this feature, what should it do?"*

Choosing the right `Compat` level when registering a feature:

| Level | Meaning | Use when |
|---|---|---|
| `Warn` | Older binary opens normally; a warning is logged | Feature is purely additive; an older binary can safely ignore it without risk of data loss or corruption |
| `ReadOnly` | Older binary cannot write to the file | Feature involves new schema constructs or data that an older binary would silently lose or corrupt on write |
| `Refuse` | Older binary cannot open the file at all | Feature changes how existing data is physically interpreted (e.g. new storage layout with a new MapStrategy); even a read by an older binary is unsafe |

---

### 5.3 Compatibility Matrix

The following table covers every combination a binary will encounter as the feature list grows. The opening binary's behavior depends solely on: (a) whether the feature row is present in the file, and (b) whether the feature is in the binary's `s_knownFeatures`.
These scenarios are covered by the iModel evolution tests.

| `ec_Feature` row present | `s_knownFeatures` | Open mode | Result |
|---|---|---|---|
| No | No | Any |  Opens normally. Feature not activated in this file. |
| No | Yes | Any |  Opens normally. Runtime supports it; file doesn't use it. |
| Yes - `Warn` | Yes | Any |  Opens normally. Known feature; `Compat` column is not consulted. |
| Yes - `ReadOnly` | Yes | Any |  Opens normally. Known feature; `Compat` column is not consulted. |
| Yes - `Refuse` | Yes | Any |  Opens normally. Known feature; `Compat` column is not consulted. |
| Yes - `Warn` | **No** | Any |  Opens normally; **warning logged** naming the feature name. |
| Yes - `ReadOnly` | **No** | Read-only |  Opens read-only. No warning - file is safely accessible in read-only mode. |
| Yes - `ReadOnly` | **No** | Read-write |  **Open fails** (`BE_SQLITE_READONLY`). Error logged. |
| Yes - `Refuse` | **No** | Any |  **Open fails** (`BE_SQLITE_ERROR`). Error logged. |

**Key insight:** The `Compat` column is only ever consulted when the feature is **unknown** to the opening binary. For every feature present in `s_knownFeatures`, the `Compat` value in the file is irrelevant to the current binary - it is purely a message left for older binaries.

---

## 6. Design Decisions and Rationale

### `MaxParsable` not `Latest` as the strict ceiling

The strict-mode ceiling in `ReadSchemaStub` is `MaxParsable` (V3_3), not `Latest` (V3_2). This allows ECDb to import V3_3 schemas today while still blocking truly unknown versions. The write-side gate (`IsECVersionSerializable`) uses `Latest` as its sentinel to prevent V3_3 schemas from being accidentally re-serialized (which would drop any V3_3 constructs not yet understood by the serializer).

### `Compat` is outward-facing, not inward-facing

`Compat` in `FeatureInfo` describes the behavior policy for **older binaries** that do not recognize the feature. It is never consulted for the current binary - any feature in `s_knownFeatures` is by definition fully supported and the file always opens normally. This is confirmed by `Feature_KnownFeature_OpensNormally`, which asserts that a file with a `Refuse` feature row opens in read-write mode when the feature is present in the registry.

---

## 7. Future Work (TODOs)

### 7.1 Expand strict reading of all ECObjects

Current changes have only covered strict reading of `ECClass` (Base + Relationship + CA), `ECEnumeration`, and `ECProperty`.
All remaining ECObjects need to check the strict flag in their `ReadXml` / `_ReadXmlAttributes` method and call `SchemaParseUtils::FindFirstUnknownXmlAttribute` when the flag is set.

The `_StrictFails` tests for all these gaps are already written in Zone 1 of `StrictSchemaValidationTests.cpp` (see [Section 4](#4-ecxml-parsing-forward-compatibility-ecobjects-layer) for the full list with expected outcomes) and are currently expected to fail until the implementations below land.

| ECObject | Reader to update | Matching `_StrictFails` test |
|---|---|---|
| `PropertyCategory` | `PropertyCategory::ReadXml` | `UnknownAttributeOnPropertyCategory_StrictFails` |
| `UnitSystem` | `UnitSystem::ReadXml` | `UnknownAttributeOnUnitSystem_StrictFails` |
| `Phenomenon` | `Phenomenon::ReadXml` | `UnknownAttributeOnPhenomenon_StrictFails` |
| `ECUnit` | `ECUnit::ReadXml` | `UnknownAttributeOnUnit_StrictFails` |
| `ECFormat` | `ECFormat::ReadXml` | `UnknownAttributeOnFormat_StrictFails` |
| `KindOfQuantity` | `KindOfQuantity::ReadXml` | `UnknownAttributeOnKindOfQuantity_StrictFails` |
| `ECSchemaReference` | `_ReadSchemaReferencesFromXml` | `UnknownAttributeOnSchemaReference_StrictFails` |
# Overview

**The problem:** Our rigid compatibility model with future-proofing promises is a lie. It has frozen both ECSchema at v3.2 and ECDb itself, blocking new features at two levels — older software may silently corrupt data when encountering schema constructs it doesn't understand, and ECDb-level improvements (such as storage layout changes) cannot be introduced without risking incompatibility with older clients.

**Options on the table:**

- **Monolithic v4.0 release** — bundle all changes into one big migration. Lower complexity upfront, but years of delay and high-risk "big bang" rollout with potential data corruption on older software.
- **Feature-driven model** — embed feature flags in iModel files so software can open what it supports and safely reject what it doesn't. Enables incremental delivery with a phased ecosystem rollout, for both schema-level and storage-level features.

# Details

We are hindered at two levels: we cannot introduce new ECSchema constructs because older software may silently mishandle them, and ECDb storage improvements (e.g., storing geometry in a separate table) cannot be introduced because older clients don't know how to handle files that use them.

The root cause is our "forward compatibility" strategy, which relies on older software ignoring data it doesn't understand. This is unsafe — ignoring unknown constructs (new schema concepts, reorganized storage) can lead to data loss or corruption. We have been unable to improve ECSchema, ECDb storage, or retire expensive legacy features because the cost is too high.

Some attempts have been made, like making schema loading more tolerant and adding the `UseRequiresVersion` CA to warn about unsupported features. These only cover a fraction of the problem surface and do not provide a general path forward.

This proposal outlines a shift from a **Monolithic Versioning model** (waiting for "v4.0") to a **Feature-Driven model** — allowing us to resume evolving both the schema specification and ECDb storage safely, incrementally, and without a high-risk "big bang" migration.

## Proposal

Instead of a single version number (e.g., "v4.0"), an iModel will declare a list of **Features** — effectively feature flags embedded in the file. A feature can represent anything from a new schema construct to a low-level storage layout change.

- **How it works:** When software opens an iModel, it checks the required features.
  - *Scenario A:* The software supports all features → The file opens normally.
  - *Scenario B:* The file uses a feature the software doesn't know (e.g., `"dynamic-types"` or `"geometry-in-separate-table"`) → The software **safely rejects** the file with a clear message: *"This file requires a newer version of the software because it uses feature `<feature-name>`."*
- **Business Value:** We can release features one by one, at either the schema or storage level. We do not need to wait years to bundle them into a massive release.

## Redefining "Evergreen"

We must refine our definition of "Evergreen" to prioritize **Data Integrity** over **Blind Access**.

- *Current:* "Old software opens new files, potentially ignoring critical data."
- *Proposed:* "Old software opens new files IF it is safe to do so. If not, it fails gracefully."

## ECSchema Versioning vs. iModel Features

These are two distinct but related mechanisms:

- **ECSchema XML files** continue to use version numbers (3.2 → 3.3 → 3.4 etc.). The version signals what constructs the schema may contain.
- **iModel files (ECDb)** hold the feature registry. Features are the authoritative source of what a given iModel requires.
- **The relationship:** A schema of a given version *may require* one or more features to be present on the iModel before it can be imported. This is always **explicit** — importing a schema never silently enables a feature on an iModel. If the required feature has not been enabled, the import is **rejected with a clear error** telling the user which feature is missing and must be enabled first.

This explicit gate is intentional and important. Silently enabling a feature during schema import would be a footgun: an iModel could accidentally become unreadable by older software without anyone realizing it until it was too late.

Software that only knows ECSchema 3.2 will reject a 3.3+ schema outright, rather than attempting a partial load and silently losing data.

## Managing Features

Each ECDb release ships with a built-in registry of features identified by **well-known string names** (e.g., `"strict-schema-loading"`, `"dynamic-types"`). String names are self-documenting and unambiguous within our controlled registry.

### Feature Status

Each feature has one of three status values:

- **Experimental**: Under active development. Must be explicitly opted into. May be dropped or changed without notice. Not suitable for production iModels.
- **Stable**: Production-ready and rolled out across the ecosystem. Features promoted to Stable will not be removed without a full deprecation cycle.
- **Deprecated**: Was Stable, but is being retired. iModels using this feature will receive warnings with migration guidance during the deprecation period.

### Feature Descriptor Fields

Each registered feature exposes a descriptor with the following fields:

```
name             — canonical string identifier (e.g. "strict-schema-loading")
label            — short human-readable display name
description      — explains what the feature does and why you'd enable it
status           — Experimental | Stable | Deprecated
enabledByDefault — PM's knob: true means the feature is auto-enabled on new iModels
toggleable       — engineering's knob: true means EnableFeature/DisableFeature are runtime-reversible
ephemeral        — true means this feature is session-scoped only; enabling does NOT persist to the file
```

**`enabledByDefault`** is product management's dial — it controls when a Stable feature gets automatically turned on for newly created iModels. A feature can be `Stable` with `enabledByDefault: false` during an initial rollout window, then flipped to `true` once ecosystem adoption is high enough. This replaces the old `Provisional` status, which conflated "production-ready" with "not yet default" into a single enum value. Keeping them separate makes the intent explicit.

**`toggleable`** is engineering's safety valve. Most persistent features are one-way operations (you can't un-enable `strict-schema-loading` on an iModel that already uses it without risking data integrity). Setting `toggleable: false` makes `DisableFeature` return an error for that feature. Session-scoped (ephemeral) features are always toggleable by nature.

**`ephemeral`** covers features whose effects do not persist to the file and only apply to the current connection session. There is no `Discontinued` status — if a feature is removed from the engine, it is marked `Deprecated` until removed; any data written under it is handled by retained compatibility code.

### Relationship to `experimental_features_enabled` / `OPTIONS ENABLE_EXPERIMENTAL_FEATURES`

ECDb already has a session-scoped boolean flag (`ECSqlConfig::m_experimentalFeaturesEnabled`, default `false`) that gates access to experimental ECSQL functionality. This is controlled via:

```sql
PRAGMA experimental_features_enabled = true;           -- read/write, session-scoped
OPTIONS ENABLE_EXPERIMENTAL_FEATURES = true;           -- per-statement alternative
```

The new feature registry **integrates with this existing mechanism**: enabling `experimental_features_enabled` for a session will also activate all registered `Experimental` + `ephemeral` features for that session. The existing pragma becomes the single knob to "unlock everything experimental for this connection" — no per-feature opt-ins required when you're just testing.

Explicitly calling `EnableFeature("some-experimental-feature")` on a persistent (non-ephemeral) file is still required for features that write persistent state.

> **Open question — concurrent query workers:** ECDb's concurrent query manager opens secondary connections via `OpenSecondaryConnection`, which is a low-level SQLite operation. The `ECSqlConfig` session state (including `m_experimentalFeaturesEnabled`) is **not** automatically propagated to these worker connections today. Session-scoped features enabled on the primary connection will not be available in queries dispatched to the concurrent worker pool. This needs to be addressed in the implementation, either by threading the `ECSqlConfig` state through when initializing worker connections, or by explicitly documenting the limitation.

## Implementation

### Storage

Feature state for persistent (non-ephemeral) features is persisted in `be_prop`. Ephemeral feature state lives only in the `ECDb` instance and is discarded when the connection closes.

### Who Enables Features and How

Persistent features are never enabled automatically. There are three explicit trigger paths:

1. **New iModel creation:** The ECDb engine enables all `Stable` + `enabledByDefault: true` features automatically. No user action required.
2. **Migration tooling:** For features unrelated to schemas (e.g., a future "geometry in separate table" feature), a dedicated migration tool or administrative operation explicitly enables the feature on an existing iModel.
3. **Schema import:** If a schema requires a feature that is not yet enabled on the iModel, the import is **rejected**. The user or tooling must explicitly enable the feature first, then retry the import. There is no implicit or automatic enablement.

Ephemeral features can be enabled or disabled freely at any point during the session — they carry no file-level side effects.

### API Surface

Features are exposed via two complementary surfaces:

**Typed C++ API on `ECDb`** — what code actually uses at runtime:

```cpp
FeatureSet const& GetEnabledFeatures() const;
FeatureSet const& GetAvailableFeatures() const;
BentleyStatus EnableFeature(Utf8CP featureName);
BentleyStatus DisableFeature(Utf8CP featureName);   // error if !toggleable
```

`EnableFeature` on a non-ephemeral, non-toggleable feature is a one-way, potentially irreversible operation. It is an administrative/migration call, not a runtime toggle. `DisableFeature` returns an error if `toggleable == false`.

**PRAGMA-style ECSQL** — for tooling, diagnostics, and migration scripts, consistent with existing pragmas such as `ecdb_ver` and `experimental_features_enabled`:

```sql
PRAGMA ecdb_features;                               -- list all features and their status
PRAGMA enable_feature('strict-schema-loading');     -- enable a feature on this iModel
PRAGMA disable_feature('some-toggleable-feature');  -- disable a toggleable feature
```

The PRAGMA surface is a thin wrapper over the C++ API.

**TypeScript (N-API)** — additive, does not touch existing `@public` APIs:

```typescript
export type ECDbFeatureStatus = "Experimental" | "Stable" | "Deprecated";

export interface ECDbFeatureDescriptor {
  readonly name: string;
  readonly label: string;
  readonly description: string;
  readonly status: ECDbFeatureStatus;
  readonly enabledByDefault: boolean;
  readonly toggleable: boolean;
  readonly ephemeral: boolean;
}

export interface ECDbFeatures {
  readonly used: ReadonlyArray<string>;
  readonly available: ReadonlyArray<ECDbFeatureDescriptor>;
}

// Exposed as iModelDb.getFeatures() / iModelDb.enableFeature(name)
```

### First Feature: Strict Schema Loading

The first concrete feature is `"strict-schema-loading"`. It gates a behavioral change in the existing v3.2 ECSchema XML parser:

- Unknown XML attributes on known elements → **error** (currently: silently ignored)
- Unknown XML elements in known positions → **error** (currently: silently ignored)
- Unrecognized enum values in typed attributes → **error** (currently: inconsistent)

Descriptor for `"strict-schema-loading"`:
```
status:           Experimental
enabledByDefault: false
toggleable:       false   (once an iModel uses strict parsing, you can't un-strict it)
ephemeral:        false   (persisted — the iModel always loads schemas strictly after this)
```

When an iModel has `"strict-schema-loading"` enabled, all schema imports run the parser in strict mode. This is useful immediately even without any new schema features — it catches schema authoring mistakes that currently vanish silently. It also serves as the end-to-end proof of the feature pipeline before any higher-stakes features are built on top of it.

## Rollout

1. **Development:** Implement the feature infrastructure (storage, C++ API, PRAGMA, N-API/TS). Ship `"strict-schema-loading"` as the first `Experimental` feature.
2. **Release:** Ship the feature-aware engine well before any customer iModels are migrated to use new features.
3. **Graduation:** Product management promotes a feature from `Experimental` → `Stable` (and eventually flips `enabledByDefault`) based on known release adoption data.

New feature stories can land against a proven feature infrastructure rather than racing to prove it at the same time as using it.
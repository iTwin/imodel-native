# Schema Sessions Design Document

## 1. Overview

Schema Sessions captures a sequential log of all `ImportSchema` calls made on an ECDb. Each "session" records which schemas were imported, with what flags, and stores the raw XML for dynamic schemas. This enables **replay**: given a fresh ECDb, the exact schema import sequence can be replayed to reconstruct the schema state.

### Key Properties
- **Stateless in memory** — Sessions store nothing in memory; all state lives in `be_Local`
- **All-or-nothing** — Cannot remove/replay a single session; either replay all or clear all
- **Automatic** — Enabled by default when change tracking is on and ECDb is a briefcase
- **Manual override** — Can be enabled/disabled via `Sessions::SetEnabled(true/false)` for testing

---

## 2. Storage Format

### 2.1 be_Local Keys

| Key | Value | Description |
|-----|-------|-------------|
| `ec_schemaSessions` | JSON array | Session log (see below) |
| `ec_schemaXml_<fullName>` | XML string | Schema XML for non-dynamic schemas (stored once) |
| `ec_schemaXml_<fullName>_<sessionId>` | XML string | Schema XML for dynamic schemas (stored per session) |

### 2.2 Session Log JSON

```json
[
  {
    "id": 1,
    "dateTime": "2026-05-04T14:30:00.000Z",
    "schemas": ["BisCore.01.00.14", "Generic.01.00.02"],
    "importFlags": "None"
  },
  {
    "id": 2,
    "dateTime": "2026-05-04T15:00:00.000Z",
    "schemas": ["MyDomain.01.00.00", "MyDynamic.01.00.00_2"],
    "importFlags": "AllowMajorSchemaUpgradeForDynamicSchemas",
    "txnId": 42
  }
]
```

**Fields:**
- `id` — Monotonically increasing session ID (1-based)
- `dateTime` — UTC ISO-8601 timestamp of the import
- `schemas` — Array of full schema names with version (`Name.RR.WW.mm`). For dynamic schemas, `_<sessionId>` is appended to distinguish per-session copies
- `importFlags` — Human-readable string representation of `SchemaImportOptions` flags (e.g. `"None"`, `"AllowDataTransformDuringSchemaUpgrade"`, `"AllowDataTransformDuringSchemaUpgrade|DoNotFailForDeletionsOrModifications"`)
- `txnId` — *(optional)* Integer TxnId assigned by `SetTxnId()` when the schema import was committed. Used by `ReplayForTxnId()` and `DropByTxnId()`.

### 2.3 Schema XML Storage Rules

1. **Non-dynamic schema**: Stored at key `ec_schemaXml_<fullName>`. Only stored once; if same version was already stored, skip.
2. **Dynamic schema**: Stored at key `ec_schemaXml_<fullName>_<sessionId>`. Always stored per session even if version is unchanged. The schema entry in the session's `schemas` array also uses `<fullName>_<sessionId>` format.

### 2.4 Why store in be_Local?

`be_Local` is briefcase-local storage — it is **not** merged with the team server. Schema sessions are local state used for changeset transformation during pull-merge. They never leave the briefcase.

---

## 3. API Design

```cpp
// Declared in iModelCore/ECDb/PublicAPI/ECDb/SchemaManager.h,
// nested in SchemaManager. Implemented in ECDb/ECDb/SchemaSessions.cpp.
struct Sessions final
    {
private:
    ECDb& m_ecdb;
    bool  m_enabled = false;

public:
    Sessions(ECDb& ecdb);

    //! Enable or disable session recording.
    void SetEnabled(bool enabled);

    //! Returns true if sessions are currently enabled.
    bool IsEnabled() const;

    //! Record a schema import session. Called internally after successful ImportSchemas.
    //! @param schemas   The schemas that were imported
    //! @param options   The import options used
    //! @return SUCCESS or ERROR
    BentleyStatus Record(bvector<ECN::ECSchemaCP> const& schemas,
                         SchemaManager::SchemaImportOptions options);

    //! Replay all recorded sessions into the given ECDb.
    //! Loads schema XML from be_Local, creates ECSchemaReadContext with ecdb as locater,
    //! and calls ImportSchemas for each session in order.
    //! @param target   The ECDb to replay into (must be open and writable)
    //! @return The result of the last schema import, or SUCCESS if no sessions
    SchemaImportResult Replay(ECDbR target) const;

    //! Clear all recorded sessions and their stored schema XML.
    BentleyStatus Clear();

    //! Get the number of recorded sessions.
    int GetCount() const;

    //! Associate a TxnId with all sessions that do not yet have one.
    //! Called by TxnManager when a schema change is committed.
    //! @param txnId  The uint64_t value of the committed TxnId.
    BentleyStatus SetTxnId(uint64_t txnId);

    //! Replay only the sessions associated with the given TxnId.
    //! Used during PullMerge rebase to re-import schemas for a specific schema Txn.
    //! @param txnId   The uint64_t TxnId whose sessions should be replayed.
    //! @param target  The ECDb to replay into (must be open and writable).
    SchemaImportResult ReplayForTxnId(uint64_t txnId, ECDbR target) const;

    //! Remove all sessions associated with the given TxnId and their stored schema XML.
    //! @param txnId  The uint64_t TxnId whose sessions should be removed.
    BentleyStatus DropByTxnId(uint64_t txnId);
    };

//! Get the Sessions manager for recording/replaying schema imports.
Sessions& GetSessions() const;
```

### 3.1 Integration Points

- `SchemaManager::ImportSchemas()` — After successful import, calls `GetSessions().Record(...)` if enabled
- `SchemaManager` — Exposes `Sessions& GetSessions() const` accessor (returns lazily-created sessions object)
- `ECDb::Open()` — Auto-enables sessions if briefcase + change tracking is active

### 3.2 Auto-Enable Logic

```
if (ecdb.GetBriefcaseId().IsBriefcase() && tracker && tracker->IsTracking())
    GetSessions().SetEnabled(true);
```

### 3.3 TxnId Tracking

After a schema import commits (producing an ECSchema Txn), `TxnManager` calls `GetSessions().SetTxnId(txnId)` to associate the committed TxnId with all sessions that were recorded without a TxnId. This enables selective replay (`ReplayForTxnId`) and cleanup (`DropByTxnId`) during semantic rebase.

---

## 4. Replay Mechanics

```
Sessions::Replay(ECDbR target):
  1. Load session log JSON from be_Local key "ec_schemaSessions"
  2. Parse into array of session entries
  3. For each session (in order by id):
     a. Create ECSchemaReadContext with target as locater
     b. For each schema fullName in session.schemas:
        - Try dynamic key "ec_schemaXml_<fullName>_<sessionId>" first
        - Fall back to static key "ec_schemaXml_<fullName>" if not found
        - Load XML from be_Local
        - Deserialize XML into ECSchema via ECSchemaReadContext
     c. Call target.Schemas().ImportSchemas(loadedSchemas, session.importFlags)
     d. If result is error, return error immediately
  4. Return SUCCESS

Sessions::ReplayForTxnId(txnId, ECDbR target):
  Same as Replay but only processes sessions whose "txnId" field matches the given txnId.
```

---

## 5. Dynamic Schema Handling

```mermaid
flowchart TD
    A[ImportSchemas called] --> B{Sessions enabled?}
    B -->|No| Z[Done]
    B -->|Yes| C[For each schema in import list]
    C --> D{schema.IsDynamicSchema()?}
    D -->|Yes| E["Store XML at ec_schemaXml_<fullName>_<sessionId>"]
    D -->|No| F{XML already stored for this version?}
    F -->|Yes| G[Skip storage]
    F -->|No| H["Store XML at ec_schemaXml_<fullName>"]
    E --> I[Add "<fullName>_<sessionId>" to session.schemas]
    G --> J["Add <fullName> to session.schemas"]
    H --> J
    I --> K[Continue]
    J --> K
```

---

## 6. Error Handling

- If `Record()` fails to serialize/store, it logs an error but does NOT fail the `ImportSchemas` call (recording is best-effort)
- If `Replay()` encounters missing XML for a schema key, it returns `ERROR` with a diagnostic log
- If `Replay()` `ImportSchemas` fails, it returns the error immediately (partial replay state)
- `Clear()` removes the `ec_schemaSessions` key and all associated `ec_schemaXml_*` keys
- `DropByTxnId()` removes only the sessions for a specific TxnId, preserving static XML keys still referenced by surviving sessions

---

## 7. File Locations

| File | Purpose |
|------|---------|
| `iModelCore/ECDb/PublicAPI/ECDb/SchemaManager.h` | `Sessions` nested struct declaration and `GetSessions()` accessor |
| `iModelCore/ECDb/ECDb/SchemaSessions.cpp` | Full implementation |
| `iModelCore/ECDb/Tests/NonPublished/SchemaSessionsTests.cpp` | Test file |

---

## 8. Test Cases (`ChangesetSchemaTests.cpp`, `SchemaSessionsTests.cpp`)

**`ChangesetSchemaTests.cpp`** covers `ChangesetSchema` and `ChangesetSchemaDiff`:
1. **CaptureAndRoundTrip** — Capture from db, serialize to JSON, deserialize, verify classes and property maps match
2. **DiffNoChange** — Capture twice with same schema, diff returns empty result and `NeedsTransform()` == false
3. **DiffClassIdRemap** — Simulate class ID remap by importing class in different order, diff detects remaps
4. **DiffColumnSwap** — Move property up hierarchy (triggering `RemapManager`), diff detects column swaps
5. **DiffOverflowAdded** — Add enough properties to exceed `MaxSharedColumnsBeforeOverflow`, diff detects overflow table
6. **TransformClassIdRemap** — Apply class ID remap transform, verify class IDs corrected in output changeset
7. **TransformColumnSwap** — Apply column swap transform, verify values moved to correct columns
8. **TransformOverflowINSERT** — Apply overflow transform, verify INSERT row for overflow table appended
9. **TransformOverflowDELETE** — Verify DELETE transform does not add overflow row
10. **TransformOverflowUPDATE** — Verify UPDATE transform emits overflow UPDATE for moved columns
11. **ChangesetScopedCapture** — `Capture(changeset)` only captures classes referenced by the changeset
12. **DiffErrors** — Simulate missing class/property/table in after snapshot, verify errors collected

**`SchemaSessionsTests.cpp`** covers `SchemaManager::Sessions`:
1. **EnableDisable** — Enable/disable manually, verify `IsEnabled()` state
2. **RecordSingleSession** — Record one import, verify JSON structure and schema XML in be_Local
3. **RecordMultipleSessions** — Record two imports, verify session IDs and schema arrays
4. **NoRecordWhenDisabled** — Sessions disabled, import schemas, verify nothing stored
5. **ClearSessions** — Record sessions, call `Clear()`, verify be_Local keys removed and `GetCount() == 0`
6. **ReplaySessions** — Record sessions, replay into fresh ECDb, verify schemas present
7. **ReplayMultipleSessions** — Record multiple sessions in order, replay, verify all schemas
8. **DynamicSchemaPerSession** — Import dynamic schema twice, verify per-session XML keys stored
9. **GetCountEmpty** — `GetCount()` returns 0 when no sessions recorded

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
    "options": 0
  },
  {
    "id": 2,
    "dateTime": "2026-05-04T15:00:00.000Z",
    "schemas": ["MyDomain.01.00.00", "MyDynamic.01.00.00_2"],
    "options": 32
  }
]
```

**Fields:**
- `id` — Monotonically increasing session ID (1-based)
- `dateTime` — UTC ISO-8601 timestamp of the import
- `schemas` — Array of full schema names with version (`Name.RR.WW.mm`). For dynamic schemas, `_<sessionId>` is appended to distinguish per-session copies
- `options` — Numeric value of `SchemaImportOptions` flags used

### 2.3 Schema XML Storage Rules

1. **Non-dynamic schema**: Stored at key `ec_schemaXml_<fullName>`. Only stored once; if same version was already stored, skip.
2. **Dynamic schema**: Stored at key `ec_schemaXml_<fullName>_<sessionId>`. Always stored per session even if version is unchanged. The schema entry in the session's `schemas` array also uses `<fullName>_<sessionId>` format.

### 2.4 Why store in be_Local?

`be_Local` is briefcase-local storage — it is **not** merged with the team server. Schema sessions are local state used for changeset transformation during pull-merge. They never leave the briefcase.

---

## 3. API Design

```cpp
// In SchemaManager.h, nested in SchemaManager
struct Sessions final
    {
private:
    ECDbCR m_ecdb;
    bool m_enabled;

public:
    Sessions(ECDbCR ecdb);

    //! Enable or disable session recording.
    void SetEnabled(bool enabled);

    //! Returns true if sessions are currently enabled.
    bool IsEnabled() const;

    //! Record a schema import session. Called internally after successful ImportSchemas.
    //! @param schemas     The schemas that were imported
    //! @param options     The import options used
    //! @return SUCCESS or ERROR
    BentleyStatus Record(bvector<ECN::ECSchemaCP> const& schemas,
                         SchemaManager::SchemaImportOptions options);

    //! Replay all recorded sessions into the given ECDb.
    //! Loads schema XML from be_Local, creates ECSchemaReadContext with ecdb as locater,
    //! and calls ImportSchemas for each session in order.
    //! @param target     The ECDb to replay into (must be open and writable)
    //! @return The result of the last schema import, or SUCCESS if no sessions
    SchemaImportResult Replay(ECDbR target) const;

    //! Clear all recorded sessions and their stored schema XML.
    BentleyStatus Clear();

    //! Get the number of recorded sessions.
    int GetCount() const;
    };
```

### 3.1 Integration Points

- `SchemaManager::ImportSchemas()` — After successful import, call `Sessions().Record(...)` if enabled
- `SchemaManager` — Expose `Sessions const& GetSessions() const` accessor
- `ECDb::Open()` — Auto-enable sessions if briefcase + change tracking

### 3.2 Auto-Enable Logic

```
if (ecdb.GetBriefcaseId().IsBriefcase() && tracker && tracker->IsTracking())
    Sessions().SetEnabled(true);
```

---

## 4. Replay Mechanics

```
Sessions::Replay(ECDbR target):
  1. Load session log JSON from be_Local key "ec_schemaSessions"
  2. Parse into array of session entries
  3. For each session (in order by id):
     a. Create ECSchemaReadContext with target as locater
     b. For each schema fullName in session.schemas:
        - Determine be_Local key:
          - If name contains "_<sessionId>" suffix → dynamic, use full key
          - Else → use "ec_schemaXml_<fullName>"
        - Load XML from be_Local
        - Deserialize XML into ECSchema via ECSchemaReadContext
     c. Call target.Schemas().ImportSchemas(loadedSchemas, session.options)
     d. If result is error, return error immediately
  4. Return SUCCESS
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

- If `Record()` fails to serialize/store, it logs error but does NOT fail the ImportSchemas call (recording is best-effort)
- If `Replay()` encounters missing XML for a schema key, it returns ERROR with diagnostic log
- If `Replay()` ImportSchemas fails, it returns the error immediately (partial replay state)
- `Clear()` removes all `ec_schemaSessions` and all `ec_schemaXml_*` keys

---

## 7. File Locations

| File | Purpose |
|------|---------|
| `PublicAPI/ECDb/SchemaManager.h` | Add `Sessions` nested struct declaration |
| `ECDb/SchemaSessions.cpp` | Implementation |
| `ECDb/SchemaSessions.h` | Internal header (optional, for helper constants) |
| `Tests/NonPublished/SchemaSessionsTests.cpp` | Test file |
| `ECDb/ECDb.mke` | Build rules |

---

## 8. Test Cases

1. **RecordAndReplay** — Import 2 schemas in 2 sessions, clear ECDb, replay, verify schemas present
2. **DynamicSchemaPerSession** — Import dynamic schema twice (same version), verify both XMLs stored separately
3. **ReplayWithOptions** — Import with `AllowDataTransformDuringSchemaUpgrade`, replay, verify same flags used
4. **ClearDeletesEverything** — Record sessions, clear, verify be_Local keys gone and GetCount()==0
5. **AutoEnableOnBriefcase** — Open briefcase with tracking, verify sessions are enabled
6. **ManualEnableDisable** — Enable/disable manually, verify recording behavior
7. **ReplayMissingXmlFails** — Delete a schema XML key, attempt replay, verify error returned
8. **EmptyReplaySucceeds** — Replay with no sessions recorded returns SUCCESS

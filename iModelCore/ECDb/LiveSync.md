# No lock schema synchronization across multiple briefcases

No lock does not mean lock free. It just mean that briefcases can make changes to schema without requiring them to push there changes.


```mermaid
sequenceDiagram
    participant b1 as Briefcase 1
    participant b2 as Briefcase 2
    participant sy as SyncDb
    b1->>sy: Pull changes from sync db
    b1->>b1: Import a schema
    b1->>sy: Push changes to sync db
    b2->>sy: Pull changes from sync db!
    b2->>b2: Import a schema
    b2->>sy: Push changes to sync db

```

## With write token request to code service

There can only be one writer to sync-db.
```mermaid
sequenceDiagram
    participant b1 as Briefcase 1
    participant b2 as Briefcase 2
    participant sy as SyncDb
    participant cs as CodeService
    b1->>cs: Request write token to Sync-Db
    b1->>sy: Pull changes to sync db
    b1->>b1: Import a schema
    b1->>sy: Push changes to sync db
    b1->>cs: Release write token to Sync-Db
    b2->>cs: Get write token to Sync-Db
    b2->>sy: Pull changes from sync db!
    b2->>b2: Import a schema
    b2->>sy: Push changes to sync db
    b2->>cs: Release write token to Sync-Db


```

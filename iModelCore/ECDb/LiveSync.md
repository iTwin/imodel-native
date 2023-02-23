# No lock schema synchronization across multiple briefcases

No lock does not mean lock free. It just mean that briefcases can make changes to schema without requiring them to push there changes.

## Workflow test

```mermaid
sequenceDiagram

    actor b1 as Briefcase 1
    actor b2 as Briefcase 2

    participant sc as TestSchema.01.00.00
    participant sy as SyncDb
    participant hb as IModel Hub

    sc-->>sc: add two properties and add a index on them
    sy->>b1 : init sync db from b1
    b1->>sc : import schema
    b1->>sy : push changes to sync db
    b2->>sy : pull changes from sync db
    b2-->>b2: verify schema changes are correct
    sc-->>sc: add two more properties and add expand index.
    b2->>sc : import/update schema
    b2->>sy : push changes to sync db
    b1->>sy : pull changes from sync db
    b1-->>b1: verify schema changes are correct
    b1->>hb : pull/merge/push changes to imodel hub
    b2-x hb : pull/merge/push changes to imodel hub (fails)
```

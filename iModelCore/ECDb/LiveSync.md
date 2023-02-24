# No lock schema synchronization across multiple briefcases

No lock does not mean lock free. It just mean that briefcases can make changes to schema without requiring them to push there changes.

## Workflow test

```mermaid
sequenceDiagram
    autonumber
    actor b1 as Briefcase 1
    actor b2 as Briefcase 2

    participant sc as TestSchema.01.00.00
    participant sy as SyncDb
    participant hb as IModel Hub

    sc-->>sc: add two properties and <br>add a index on them
    sy->>b1 : init sync db from b1
    b1->>sc : import schema
    b1->>sy : push changes to sync db
    b2->>sy : pull changes from sync db
    b2-->>b2: verify schema <br>changes are correct
    sc-->>sc: add two more properties <br>and add expand index.
    b2->>sc : import/update schema
    b2->>sy : push changes to sync db
    b1->>sy : pull changes from sync db
    b1-->>b1: verify schema <br>changes are correct
    b1->>hb : pull/merge/push <br>changes to imodel hub
    b2-x hb : pull/merge/push <br>changes to imodel hub (fails)
```

## IModel edit workflow


```mermaid
sequenceDiagram
autonumber
actor a1 as Desktop App / Live Sync
participant a2 as WIP iModel
actor a6 as Team Reviewer
participant a4 as Projectwise
participant a3  as Shared iModel
actor a5 as Design Reviewer
a1->>a2: Push changes
a2->>a1: Pull/Merge changes
a2-->>a6: Team Review App
a2->>a3:  (optional, after review)<br>WIP->Shared Transformer
a4->>a3: (for channels with no WIP->Shared transformer)<br> Connector Job
a3-->>a5: Design Review App
a3->>a2: (for other team's channels)<br>Shared->Wip Transformer



```

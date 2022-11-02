/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <DgnPlatformInternal.h>
#include <ECDb/ChangedIdsIterator.h>

DgnDbStatus EntityIdsChangeGroup::ExtractChangedInstanceIdsFromChangeSets(DgnDbR db, const bvector<BeFileName>& changeSetFiles)
    {
    // this is a non-change-preserving (id only) version of
    // https://www.sqlite.org/session/sqlite3changegroup_add.html
    const auto handleOp = [](bmap<BeInt64Id, DbOpcode>& opMap, BeInt64Id id, DbOpcode newOp)
        {
        const auto found = opMap.find(id);
        const bool wasFound = found != opMap.end();
        if (!wasFound)
            {
            opMap[id] = newOp;
            return;
            }
        const auto& existingOp = found->second;
        switch (existingOp)
            {
            case DbOpcode::Insert:
                switch (newOp)
                    {
                    case DbOpcode::Insert: return;
                    case DbOpcode::Update: return;
                    case DbOpcode::Delete: opMap.erase(found); return;
                    }
            case DbOpcode::Update:
                switch (newOp)
                    {
                    case DbOpcode::Insert: return;
                    case DbOpcode::Update: return;
                    case DbOpcode::Delete: found->second = DbOpcode::Delete; return;
                    }
            case DbOpcode::Delete:
                switch (newOp)
                    {
                    case DbOpcode::Insert: found->second = DbOpcode::Update; return;
                    case DbOpcode::Update: return;
                    case DbOpcode::Delete: return;
                    }
            }
        };

    ECClassId elementClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_Element);
    ECClassId uniqueAspectClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementUniqueAspect);
    ECClassId multiAspectClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);
    ECClassId modelClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_Model);
    // WIP: also consider ElementDrivesElement
    ECClassId relationshipClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementRefersToElements);
    ECClassId codeSpecClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_CodeSpec);

    CachedStatementPtr stmt;
    db.GetStatementCache().GetPreparedStatement(stmt, *db.GetDbFile(), R"sql(
        WITH RECURSIVE parentTable(tableName, Id, ParentId, ExclusiveRootClassId) AS (
            SELECT t.Name, Id, ParentTableId, ExclusiveRootClassId FROM ec_Table t
            UNION ALL
            SELECT p.tableName, t.Id, ParentTableId, t.ExclusiveRootClassId FROM ec_Table t JOIN parentTable p ON t.Id=ParentId
        )
        SELECT p.tableName, ExclusiveRootClassId FROM parentTable p
        JOIN ec_Class c ON p.ExclusiveRootClassId=c.Id
        WHERE p.ParentId IS NULL
    )sql");

    bmap<Utf8String, ECClassId> tableNameToRootEntityClassId;
    DbResult status;
    while (DbResult::BE_SQLITE_ROW == (status = stmt->Step()))
        tableNameToRootEntityClassId[stmt->GetValueText(0)] = stmt->GetValueId<ECClassId>(1);
    if (DbResult::BE_SQLITE_DONE != status)
        return DgnDbStatus::SQLiteError;

    for (const auto& changeSetFile : changeSetFiles)
        {
        RevisionChangesFileReader changeSetReader(changeSetFile, db);
        // changeSetReader.Dump("ExtractChangedInstanceIdsFromChangeSet", db);
        ChangedIdsIterator changeIter(db, changeSetReader);
        for (const auto& changeEntry : changeIter)
            {
            if (!changeEntry.IsMapped() || !changeEntry.IsPrimaryTable())
                {
                if (changeEntry.GetTableName() == "dgn_Font")
                    {
                    Changes::Change const& change = changeEntry.GetChange();
                    Byte* pColumns = nullptr;
                    int numColumns = 0;
                    if (BE_SQLITE_OK != change.GetPrimaryKeyColumns(&pColumns, &numColumns) || (0 == numColumns) || (0 == pColumns[0]))
                        {
                        BeAssert(false && "Expect column 0 to be the primary key of the dgn_Font table");
                        LOG.error("Expect column 0 to be the primary key of the dgn_Font table");
                        return DgnDbStatus::UnknownFormat;
                        }

                    handleOp(
                        fontOps,
                        changeEntry.GetDbOpcode() == DbOpcode::Insert
                            ? change.GetNewValue(0).GetValueId<BeInt64Id>()
                            : change.GetOldValue(0).GetValueId<BeInt64Id>(),
                        changeEntry.GetDbOpcode()
                    );
                    }
                continue;
                }

            const auto& tableName = changeEntry.GetTableName();
            const auto rootClassIter =  tableNameToRootEntityClassId.find(tableName);
            if (rootClassIter == tableNameToRootEntityClassId.end())
                {
                BeAssert(false && "the root class id was not found for a table");
                LOG.errorv("the root class id was not found for table '%s'", tableName.c_str());
                continue;
                }
            const auto rootClassId = rootClassIter->second;
            if (rootClassId == elementClassId)
                handleOp(elementOps, changeEntry.GetPrimaryInstanceId(), changeEntry.GetDbOpcode());
            else if (rootClassId == multiAspectClassId || rootClassId == uniqueAspectClassId)
                handleOp(aspectOps, changeEntry.GetPrimaryInstanceId(), changeEntry.GetDbOpcode());
            else if (rootClassId == modelClassId)
                handleOp(modelOps, changeEntry.GetPrimaryInstanceId(), changeEntry.GetDbOpcode());
            else if (rootClassId == relationshipClassId) // WIP: also consider ElementDrivesElement
                handleOp(relationshipOps, changeEntry.GetPrimaryInstanceId(), changeEntry.GetDbOpcode());
            else if (rootClassId == codeSpecClassId)
                handleOp(codeSpecOps, changeEntry.GetPrimaryInstanceId(), changeEntry.GetDbOpcode());
            }
        }

    return DgnDbStatus::Success;
    }

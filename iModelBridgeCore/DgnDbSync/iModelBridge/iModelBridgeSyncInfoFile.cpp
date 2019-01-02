/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/iModelBridgeSyncInfoFile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeSyncInfoFile.h>
#include <Logging/bentleylogging.h>
#include <GeomJsonWireFormat/JsonUtils.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LOGGING

#define SYNCINFO_ATTACH_ALIAS "IMODELBRIDGE_SYNCINFO"
#define SYNCINFO_TABLE(name)  "imbsync_" name
#define SYNCINFO_ATTACH(name) SYNCINFO_ATTACH_ALIAS "." name

#define SYNC_TABLE_Item     "item"

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridge"))

#define MUSTBEDBRESULT(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {SetLastError(rc); return BSIERROR;}}
#define MUSTBEOK(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_OK)
#define MUSTBEROW(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_ROW)
#define MUSTBEDONE(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_DONE)

#define MUSTBEDBRESULTRC(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {SetLastError(rc); return rc;}}
#define MUSTBEOKRC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_OK)
#define MUSTBEROWRC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_ROW)
#define MUSTBEDONERC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_DONE)

static ProfileVersion s_currentVersion(0, 1, 0, 0);

struct DbCloser : BeSQLite::Db::AppData
    {
    iModelBridgeSyncInfoFile& m_syncinfo;
    DbCloser(iModelBridgeSyncInfoFile& si) : m_syncinfo(si) {}
    void _OnDbClose(Db&) {m_syncinfo.DetachFromBIM();}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult iModelBridgeSyncInfoFile::SavePropertyString(PropertySpecCR spec, Utf8CP stringData, uint64_t id, uint64_t subId)
    {
    Statement stmt;
    auto rc = stmt.Prepare(GetDgnDb(), "INSERT OR REPLACE INTO " SYNCINFO_ATTACH(BEDB_TABLE_Property) " (Namespace,Name,Id,SubId,TxnMode,StrData) VALUES(?,?,?,?,?,?)");
    if (BE_SQLITE_OK != rc)
        return  rc;

    int col = 1;
    stmt.BindText(col++, spec.GetNamespace(), Statement::MakeCopy::No);
    stmt.BindText(col++, spec.GetName(), Statement::MakeCopy::No);
    stmt.BindInt64(col++, id);
    stmt.BindInt64(col++, subId);
    stmt.BindInt(col++, 0);
    stmt.BindText(col++, stringData, Statement::MakeCopy::No);
    rc = stmt.Step();
    return (BE_SQLITE_DONE == rc) ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult iModelBridgeSyncInfoFile::QueryProperty(Utf8StringR value, PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    Statement stmt;
    DbResult rc = stmt.Prepare(GetDgnDb(), "SELECT StrData FROM " SYNCINFO_ATTACH(BEDB_TABLE_Property) " WHERE Namespace=? AND Name=? AND Id=? AND SubId=?");
    if (BE_SQLITE_OK != rc)
        return rc;

    int col = 1;
    stmt.BindText(col++, spec.GetNamespace(), Statement::MakeCopy::No);
    stmt.BindText(col++, spec.GetName(), Statement::MakeCopy::No);
    stmt.BindInt64(col++, id);
    stmt.BindInt64(col++, subId);
    rc = stmt.Step();
    if (BE_SQLITE_ROW != rc)
        return rc;

    value.AssignOrClear(stmt.GetValueText(0));
    return  BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridgeSyncInfoFile::DeleteSyncInfoFileFor(BeFileNameCR briefcaseName)
    {
    BeFileName sfilename(ComputeSyncInfoFileName(briefcaseName));
    if (!sfilename.DoesPathExist())
        return BSISUCCESS;
    return (BeFileNameStatus::Success == sfilename.BeDeleteFile())? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName iModelBridgeSyncInfoFile::ComputeSyncInfoFileName(BeFileNameCR dbname)
    {
    BeFileName name(dbname);
    name.append(L".imodelbridge_syncinfo");
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSyncInfoFile::AttachToBIM(DgnDbR db, bool createIfNecessary)
    {
    m_bim = &db;

    BeFileName sfilename(ComputeSyncInfoFileName(GetDgnDb().GetFileName()));

    if (!sfilename.DoesPathExist())
        CreateEmptyFile(sfilename, false);

    DbResult rc = GetDgnDb().AttachDb(Utf8String(sfilename).c_str(), SYNCINFO_ATTACH_ALIAS);
    if (BE_SQLITE_OK != rc)
        return BSIERROR;

    db.SaveChanges();

    m_isAttached = true;

    return OnAttach();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSyncInfoFile::DetachFromBIM()
    {
    if (!m_bim.IsValid())
        return;

    m_bim->DetachDb(SYNCINFO_ATTACH_ALIAS);
    m_bim = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::~iModelBridgeSyncInfoFile()
    {
    DetachFromBIM();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridgeSyncInfoFile::PerformVersionChecks()
    {
    //  Look at the stored version and see if we have to upgrade
    Utf8String versionString;
    MUSTBEROW(QueryProperty(versionString, SyncInfoProperty::ProfileVersion()));

    ProfileVersion storedVersion(0, 0, 0, 0);
    storedVersion.FromJson(versionString.c_str());

    if (storedVersion.CompareTo(s_currentVersion) == 0)
        return BSISUCCESS;

    if (storedVersion.CompareTo(s_currentVersion) > 0)
        { // version is too new!
        LOG.errorv("compatibility error - storedVersion=%s > currentVersion=%s", versionString.c_str(), s_currentVersion.ToJson().c_str());
        return BSIERROR;
        }

    //  Upgrade - when we change the syncInfo schema, add upgrade steps here ...

    //  Upgraded. Update the stored version.
    MUSTBEOK(SavePropertyString(SyncInfoProperty::ProfileVersion(), s_currentVersion.ToJson().c_str()));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSyncInfoFile::OnAttach()
    {
    if (!GetDgnDb().TableExists(SYNCINFO_ATTACH(SYNC_TABLE_Item)))
        {
        // We are creating a new syncinfo file
        Utf8String currentDbProfileVersion;
        GetDgnDb().QueryProperty(currentDbProfileVersion, Properties::ProfileVersion());

        MUSTBEOK(SavePropertyString(SyncInfoProperty::ProfileVersion(), s_currentVersion.ToJson().c_str()));
        MUSTBEOK(SavePropertyString(Properties::CreationDate(), DateTime::GetCurrentTimeUtc().ToString().c_str()));
        MUSTBEOK(SavePropertyString(SyncInfoProperty::DgnDbGuid(), GetDgnDb().GetDbGuid().ToString().c_str()));
        MUSTBEOK(SavePropertyString(SyncInfoProperty::DbProfileVersion(), currentDbProfileVersion.c_str()));
        MUSTBEOK(SavePropertyString(SyncInfoProperty::DgnDbProfileVersion(), GetDgnDb().GetProfileVersion().ToJson().c_str()));
        // *** WIP_CONVERTER - I'd like to save project's last save time

        return CreateTables();
        }

    //  We are opening an existing syncinfo file
    if (PerformVersionChecks() != BSISUCCESS)
        return BSIERROR;

    //  Check that this syncinfo goes with this project
    Utf8String projguidstr;
    BeSQLite::BeGuid projguid;
    if (QueryProperty(projguidstr, SyncInfoProperty::DgnDbGuid()) != BE_SQLITE_ROW
        || projguid.FromString(projguidstr.c_str()) != BSISUCCESS
        || GetDgnDb().GetDbGuid() != projguid)
        {
        LOG.errorv("GUID mismatch. syncinfo=%s projectguid=%s does not match project guid=%s",
                   GetDgnDb().GetDbFileName(), projguidstr.c_str(), GetDgnDb().GetDbGuid().ToString().c_str());
        return BSIERROR;
        }

    Utf8String savedProjectDbProfileVersion, currentProjectDbProfileVersion;
    if (QueryProperty(savedProjectDbProfileVersion, SyncInfoProperty::DbProfileVersion()) != BE_SQLITE_ROW
        || GetDgnDb().QueryProperty(currentProjectDbProfileVersion, Properties::ProfileVersion()) != BE_SQLITE_ROW
        || !savedProjectDbProfileVersion.Equals(currentProjectDbProfileVersion))
        {
        LOG.warningv("DB schema version mismatch. syncinfo=%s ProjectDbProfileVersion=%s does not match project ProfileVersion=%s.",
                     GetDgnDb().GetDbFileName(), savedProjectDbProfileVersion.c_str(), currentProjectDbProfileVersion.c_str());
        // *** WIP_CONVERTER - Do we really have to throw away project history whenever we make a trivial schema change?
        return BSISUCCESS;//BSIERROR; *** WIP_CONVERTER - support schema evolution 
        }

    Utf8String currentProjectProfileVersion = GetDgnDb().GetProfileVersion().ToJson();
    Utf8String savedProjectProfileVersion;
    if (QueryProperty(savedProjectProfileVersion, SyncInfoProperty::DgnDbProfileVersion()) != BE_SQLITE_ROW
        || !savedProjectProfileVersion.Equals(currentProjectProfileVersion))
        {
        LOG.warningv("project schema version mismatch. syncinfo=%s ProjectProfileVersion=%s does not match project ProjectProfileVersion=%s.",
                     GetDgnDb().GetDbFileName(), savedProjectProfileVersion.c_str(), currentProjectProfileVersion.c_str());
        // *** WIP_CONVERTER - Do we really have to throw away project history whenever we make a trivial schema change?
        return BSISUCCESS;//BSIERROR; *** WIP_CONVERTER - support schema evolution 
        }

    return CreateTables();  // We STILL call CreateTables. That gives subclass a chance to create its TEMP tables.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSyncInfoFile::CreateEmptyFile(BeFileNameCR fileName, bool deleteIfExists)
    {
    if (deleteIfExists)
        fileName.BeDeleteFile();

    Utf8String dbName(fileName);
    Db bootStrapper;
    auto rc = bootStrapper.CreateNewDb(dbName.c_str());
    if (rc != BE_SQLITE_OK)
        {
        LOG.errorv("%s - cannot create. Error code=%s", dbName.c_str(), Db::InterpretDbResult(rc));
        return BSIERROR;
        }

    bootStrapper.CloseDb();
    return BSISUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSyncInfoFile::CreateTables()
    {
    if (!IsAttached())
        {
        BeAssert(false);
        return BentleyStatus::ERROR;
        }

    if (GetDgnDb().TableExists(SYNCINFO_ATTACH(SYNC_TABLE_Item)))
        {
        return BentleyStatus::SUCCESS;
        }

    MUSTBEOK(GetDgnDb().CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Item),
                         "ROWID INTEGER PRIMARY KEY AUTOINCREMENT"
                         ",DgnElementId INT"
                         ",ScopeROWID INTEGER"
                         ",Kind CHAR"
                         ",ID CHAR"
                         ",LastModifiedTime REAL"
                         ",Hash CHAR"
                         ",CONSTRAINT RedundantMappings UNIQUE(Hash,DgnElementId)"
                         ));

    MUSTBEOK(GetDgnDb().ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Item) "IdxSourceId ON "   SYNC_TABLE_Item "(ScopeROWID,Kind,ID)"));
    MUSTBEOK(GetDgnDb().ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Item) "IdxSourceHash ON " SYNC_TABLE_Item "(ScopeROWID,Kind,Hash)"));
    MUSTBEOK(GetDgnDb().ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Item) "IdxElementId ON "  SYNC_TABLE_Item "(DgnElementId)"));

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::Iterator::Iterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);   //               0     1            2          3    4  5                6
    Utf8String sqlString = MakeSqlString("SELECT ROWID,DgnElementId,ScopeROWID,Kind,ID,LastModifiedTime,Hash FROM " SYNCINFO_ATTACH(SYNC_TABLE_Item));
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::ROWID iModelBridgeSyncInfoFile::Iterator::Entry::GetROWID() const
    {
    return ROWID(m_sql->GetValueInt64(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId iModelBridgeSyncInfoFile::Iterator::Entry::GetDgnElementId() const
    {
    return m_sql->GetValueId<DgnElementId>(1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::SourceIdentity iModelBridgeSyncInfoFile::Iterator::Entry::GetSourceIdentity() const
    {
    return SourceIdentity(m_sql->GetValueInt64(2), m_sql->GetValueText(3), m_sql->GetValueText(4));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::SourceState iModelBridgeSyncInfoFile::Iterator::Entry::GetSourceState() const
    {
    return SourceState(m_sql->GetValueDouble(5), m_sql->GetValueText(6));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::Iterator iModelBridgeSyncInfoFile::MakeIterator(Utf8CP where)
    {
    return Iterator(GetDgnDb(), where);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::Iterator iModelBridgeSyncInfoFile::MakeIteratorByElementId(DgnElementId id)
    {
    auto it = Iterator(GetDgnDb(), "DgnElementId=?");
    auto rc = it.GetStatement()->BindId(1, id);
    BeAssert(BE_SQLITE_OK == rc);
    UNUSED_VARIABLE(rc);
    return it;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::Iterator iModelBridgeSyncInfoFile::MakeIteratorBySourceId(SourceIdentity const& sid)
    {
    auto it = Iterator(GetDgnDb(), "ScopeROWID=? AND Kind=? AND ID=?");
    auto rc = it.GetStatement()->BindInt64(1, sid.GetScopeROWID());
    BeAssert(BE_SQLITE_OK == rc);
    it.GetStatement()->BindText(2, sid.GetKind(), Statement::MakeCopy::No);
    BeAssert(BE_SQLITE_OK == rc);
    it.GetStatement()->BindText(3, sid.GetId(), Statement::MakeCopy::No);
    BeAssert(BE_SQLITE_OK == rc);
    UNUSED_VARIABLE(rc);
    return it;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::ROWID iModelBridgeSyncInfoFile::FindRowidBySourceId(SourceIdentity const& sourceId)
    {
    auto iterator = MakeIteratorBySourceId(sourceId);
    auto i0 = iterator.begin();
    return (i0 == iterator.end())? 0: i0.GetROWID();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::Iterator iModelBridgeSyncInfoFile::MakeIteratorByHash(ROWID scopeRowId, Utf8StringCR kind, Utf8StringCR hash)
    {
    auto it = Iterator(GetDgnDb(), "ScopeROWID=? AND Kind=? AND Hash=?");
    auto rc = it.GetStatement()->BindInt64(1, scopeRowId);
    BeAssert(BE_SQLITE_OK == rc);
    it.GetStatement()->BindText(2, kind, Statement::MakeCopy::No);
    BeAssert(BE_SQLITE_OK == rc);
    it.GetStatement()->BindText(3, hash, Statement::MakeCopy::No);
    BeAssert(BE_SQLITE_OK == rc);
    UNUSED_VARIABLE(rc);
    return it;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::Iterator iModelBridgeSyncInfoFile::MakeIteratorByScope(ROWID scopeRowId)
    {
    auto it = Iterator(GetDgnDb(), "ScopeROWID=?");
    auto rc = it.GetStatement()->BindInt64(1, scopeRowId);
    BeAssert(BE_SQLITE_OK == rc);
    UNUSED_VARIABLE(rc);
    return it;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::Iterator::Entry iModelBridgeSyncInfoFile::Iterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSyncInfoFile::WriteResults(ROWID rid, ConversionResults& conversionResults, 
                                                     SourceIdentity const& sid, SourceState const& sstate, ChangeDetector& changeDetector)
    {
    bool isUpdate = (0 != rid);

    DgnElementId eid = conversionResults.m_element.IsValid()? conversionResults.m_element->GetElementId(): DgnElementId();
    conversionResults.m_syncInfoRecord = Record(rid, eid, sid, sstate);
    if (BE_SQLITE_ROW != WriteRecord(conversionResults.m_syncInfoRecord))
        {
        BeAssert(false);
        return BentleyStatus::ERROR;
        }

    changeDetector._OnItemConverted(conversionResults.m_syncInfoRecord, isUpdate? ChangeOperation::Update: ChangeOperation::Create);

    for (ConversionResults& childConversionResults : conversionResults.m_childElements)
        {
        if (!childConversionResults.m_element.IsValid())
            continue;
        Record childMapping = FindFirstByElementId(childConversionResults.m_element->GetElementId());
        BeAssert(childMapping.GetSourceIdentity() == sid);
        // Note that we map the childConversionResults's element content to the PARENT'S SourceId and SourceState
        if (BentleyStatus::SUCCESS != WriteResults(childMapping.GetROWID(), childConversionResults, sid, sstate, changeDetector))
            return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult iModelBridgeSyncInfoFile::WriteRecord(iModelBridgeSyncInfoFile::Record& rec)
    {
    if (0 == rec.GetROWID())
        {
        CachedStatementPtr stmt;                                                                        // 1           2          3    4  5                6
        MUSTBEOKRC(GetDgnDb().GetCachedStatement(stmt, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Item) " (DgnElementId,ScopeROWID,Kind,ID,LastModifiedTime,Hash) VALUES (?,?,?,?,?,?)"));
        if (rec.GetDgnElementId().IsValid())
            {MUSTBEOKRC(stmt->BindId(1, rec.GetDgnElementId()));}
        else
            {MUSTBEOKRC(stmt->BindNull(1));}
        MUSTBEOKRC(stmt->BindInt64(2, rec.GetSourceIdentity().GetScopeROWID()));
        MUSTBEOKRC(stmt->BindText(3, rec.GetSourceIdentity().GetKind(), Statement::MakeCopy::Yes));
        MUSTBEOKRC(stmt->BindText(4, rec.GetSourceIdentity().GetId(), Statement::MakeCopy::Yes));
        MUSTBEOKRC(stmt->BindDouble(5, rec.GetSourceState().GetLastModifiedTime()));
        MUSTBEOKRC(stmt->BindText(6, rec.GetSourceState().GetHash(), Statement::MakeCopy::No));
        auto rc = stmt->Step();
        if (BE_SQLITE_DONE == rc)
            {
            rec.m_ROWID = GetDgnDb().GetLastInsertRowId();
            return BE_SQLITE_ROW;
            }

        if (!BeSQLiteLib::IsConstraintDbResult(rc))
            {
            return rc;
            }

        // We already have this record. Do an update instead.

        // *** TBD: Look up record by SourceIdentity
        // rowid = ...
        BeAssert(false);
        }

    CachedStatementPtr stmt;                                                                      // 1             2            3      4    5                  6            7
    MUSTBEOKRC(GetDgnDb().GetCachedStatement(stmt, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_Item) " SET DgnElementId=?,ScopeROWID=?,Kind=?,ID=?,LastModifiedTime=?,Hash=? WHERE ROWID=?"));
    if (rec.GetDgnElementId().IsValid())
        {MUSTBEOKRC(stmt->BindId(1, rec.GetDgnElementId()));}
    else
        {MUSTBEOKRC(stmt->BindNull(1));}
    MUSTBEOKRC(stmt->BindInt64(2, rec.GetSourceIdentity().GetScopeROWID()));
    MUSTBEOKRC(stmt->BindText(3, rec.GetSourceIdentity().GetKind(), Statement::MakeCopy::Yes));
    MUSTBEOKRC(stmt->BindText(4, rec.GetSourceIdentity().GetId(), Statement::MakeCopy::Yes));
    MUSTBEOKRC(stmt->BindDouble(5, rec.GetSourceState().GetLastModifiedTime()));
    MUSTBEOKRC(stmt->BindText(6, rec.GetSourceState().GetHash(), Statement::MakeCopy::No));
    MUSTBEOKRC(stmt->BindInt64(7, rec.GetROWID()));
    MUSTBEDONERC(stmt->Step());
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::Record iModelBridgeSyncInfoFile::FindFirstByElementId(DgnElementId eid)
    {
    auto records = MakeIteratorByElementId(eid);
    auto i = records.begin();
    if (i == records.end())
        return Record();

    return i.GetRecord();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSyncInfoFile::SetLastError(BeSQLite::DbResult rc)
    {
    m_lastError = rc;
    m_lastErrorDescription = GetDgnDb().GetLastError();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSyncInfoFile::GetLastError(BeSQLite::DbResult& result, Utf8String& descr)
    {
    result = m_lastError;
    descr = m_lastErrorDescription;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename ITERTYPE>
static bool isMappedToSameSourceItem(ITERTYPE& othersMappedToSameSourceItem, DgnElementIdSet const& known)
    {
    for (auto const& otherMappedToSameSourceItem : othersMappedToSameSourceItem)
        {
        if (known.find(otherMappedToSameSourceItem.GetDgnElementId()) != known.end())
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool iModelBridgeSyncInfoFile::IsSourceItemMappedToAnElementThatWasSeen(Iterator::Entry const& entry, DgnElementIdSet const& known)
    {
    auto si = entry.GetSourceIdentity();
    if (si.GetId().empty())
        {
        auto othersMappedToSameSourceItemByHash = MakeIteratorByHash(si.GetScopeROWID(), si.GetKind(), entry.GetSourceState().GetHash());
        return isMappedToSameSourceItem(othersMappedToSameSourceItemByHash, known);
        }
        
    auto othersMappedToSameSourceItemById = MakeIteratorBySourceId(si);
    return isMappedToSameSourceItem(othersMappedToSameSourceItemById, known);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSyncInfoFile::DeleteAllItemsMappedToElement(DgnElementId gid)
    {
    CachedStatementPtr stmt;
    GetDgnDb().GetCachedStatement(stmt, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_Item) " WHERE DgnElementId=?");
    stmt->BindId(1, gid);
    return (stmt->Step() == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSyncInfoFile::DeleteAllItemsInScope(ROWID srid)
    {
    CachedStatementPtr stmt;
    GetDgnDb().GetCachedStatement(stmt, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_Item) " WHERE ScopeROWID=?");
    stmt->BindInt64(1, srid);
    return (stmt->Step() == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSyncInfoFile::DeleteItem(ROWID irid)
    {
    CachedStatementPtr stmt;
    GetDgnDb().GetCachedStatement(stmt, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_Item) " WHERE ROWID=?");
    stmt->BindInt64(1, irid);
    return (stmt->Step() == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeWithSyncInfoBase::_OnOpenBim(DgnDbR db)
    {
    if (BentleyStatus::SUCCESS != T_Super::_OnOpenBim(db))
        return BentleyStatus::ERROR;

    // Note that I must attach my syncinfo in _OnConvertToBim -- outside of the bulk update txn -- I must not wait until _ConvertToBim.
    return m_syncInfo.AttachToBIM(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeWithSyncInfoBase::_OnCloseBim(BentleyStatus status, iModelBridge::ClosePurpose purpose)
    {
    m_syncInfo.DetachFromBIM();
    T_Super::_OnCloseBim(status, purpose);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeWithSyncInfoBase::_DeleteSyncInfo()
    {
    iModelBridgeSyncInfoFile::DeleteSyncInfoFileFor(_GetParams().GetBriefcaseName());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::ConversionResults iModelBridgeWithSyncInfoBase::RecordDocument(iModelBridgeSyncInfoFile::ChangeDetector& changeDetector,
                                                                     BeFileNameCR fileNameIn, iModelBridgeSyncInfoFile::SourceState const* sstateIn,
                                                                     Utf8CP kind, iModelBridgeSyncInfoFile::ROWID srid, Utf8StringCR knownUrn)
    {
    // Get the identity of the document
    BeFileName fileName(fileNameIn);
    if (fileName.empty())
        fileName = _GetParams().GetInputFileName();

    Utf8String urn(knownUrn);
    if (urn.empty())
        urn = GetParamsCR().QueryDocumentURN(fileName);

    // Make a RepositoryLink to represent the document
    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = MakeRepositoryLink(GetDgnDbR(), _GetParams(), fileName, "", urn);
    if (!results.m_element.IsValid())
        {
        BeAssert(false);
        return results; 
        }

    //  Compute the state of the document
    time_t lmt = 0;
    if (sstateIn)
        lmt = sstateIn->GetLastModifiedTime();
    else
        {
        if (BeFileNameStatus::Success == BeFileName::GetFileTime(nullptr, nullptr, &lmt, fileName)) // (may not really be a disk file)
            iModelBridgeSyncInfoFile::SourceState sstate((double)lmt, "");
        }

    auto sha1 = ComputeRepositoryLinkHash(*(RepositoryLink*)results.m_element.get());   // The source state is based on the properties of the RepositoryLink
    if (sstateIn)
        sha1(sstateIn->GetHash());   // if the caller passed in a hash of the file's contents, then include that in the source state hash
    else
        sha1(&lmt, sizeof(lmt)); // otherwise, make the hash depend on the last modified time, as a proxy for the file's contents.

    iModelBridgeSyncInfoFile::SourceState sstate(lmt, sha1.GetHashString());

    //  Write the item to syncinfo, and write the RepositoryLink Element to the BIM
    DocSourceItem docItem(results.m_element->GetCode(), sstate);

    if (results.m_element->GetElementId().IsValid() && !_GetParams().IsUpdating())
        {
        // If this is the initial conversion and if the item that is already there, just return it.
        iModelBridgeSyncInfoFile::SourceIdentity sid(srid, kind, docItem._GetId());
        auto byid = m_syncInfo.MakeIteratorBySourceId(sid);
        auto i = byid.begin();
        if (i != byid.end())
            {
            results.m_syncInfoRecord = iModelBridgeSyncInfoFile::Record(i.GetROWID(), i.GetDgnElementId(), i.GetSourceIdentity(), i.GetSourceState());
            return results;
            }
        }

    auto change = changeDetector._DetectChange(srid, kind, docItem);
    changeDetector._UpdateBimAndSyncInfo(results, change);

    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged != change.GetChangeType())
        {
        LOG.infov(L"[%ls] - document recorded in syncinfo with id=[%ls], rowid=%ld", fileName.c_str(), WString(docItem._GetId().c_str(), true).c_str(), results.m_syncInfoRecord.GetROWID());
        }

    return results;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeWithSyncInfoBase::DetectDeletedDocuments(Utf8CP kind, iModelBridgeSyncInfoFile::ROWID srid)
    {
    auto iterator = m_syncInfo.MakeIterator("Kind=? AND ScopeROWID=?");
    iterator.GetStatement()->BindText(1, kind, Statement::MakeCopy::No);
    iterator.GetStatement()->BindInt64(2, srid);
    
    for (auto it : iterator)
        {
        Utf8String docId = it.GetSourceIdentity().GetId();

        if (IsDocumentAssignedToJob(docId))   // This is how to check if the document still exists.
            continue;                         //  If it does, do nothing.

        auto docrid = m_syncInfo.FindRowidBySourceId(iModelBridgeSyncInfoFile::SourceIdentity(srid, kind, docId.c_str()));

        LOG.infov("[%s] - document was deleted. Deleting content converted from it.", docId.c_str());

        // Tell the bridge to delete related elements and models in the briefcase
        _OnDocumentDeleted(docId, docrid);

        // Delete corresponding items from syncinfo
        if (0 == docrid)
            {
            BeAssert(false && "bridge did not use RecordDocument to record source documents in syncinfo");
            continue;
            }
        m_syncInfo.DeleteAllItemsInScope(docrid);
        m_syncInfo.DeleteItem(docrid);
        }

    return BSISUCCESS;
    }

//=======================================================================================
// The "hash" of this item is the JSON representation of a 3x4 Transform
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct JobTransformSourceItem : iModelBridgeSyncInfoFile::ISourceItem
    {
    Transform m_trans;
    Utf8String m_id;

    JobTransformSourceItem(Utf8StringCR id, TransformCR t) : m_id(id), m_trans(t) {;}

    Utf8String _GetId() override {return m_id;}
    double _GetLastModifiedTime() override {return 0.0;}
    Utf8String _GetHash() override {Json::Value json; JsonUtils::TransformToJson(json, m_trans); return json.ToString();}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool iModelBridgeWithSyncInfoBase::DetectSpatialDataTransformChange(TransformR newTrans, TransformR oldTrans,
    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ROWID srid, Utf8CP kind, Utf8StringCR id)
    {
    newTrans = GetSpatialDataTransform();

    JobTransformSourceItem docItem(id, newTrans);

    auto change = changeDetector._DetectChange(srid, kind, docItem);
    iModelBridgeSyncInfoFile::ConversionResults results;
    changeDetector._UpdateBimAndSyncInfo(results, change);
    if (change.GetChangeType() == iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New)
        {
        oldTrans.InitIdentity();
        return !newTrans.IsIdentity();
        }
    if (change.GetChangeType() == iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Changed)
        {
        Json::Value json;
        json = json.From(change.GetSyncInfoRecord().GetSourceState().GetHash());
        JsonUtils::TransformFromJson(oldTrans, json);
        return true;
        }

    oldTrans = newTrans;
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus iModelSyncInfoAspect::AddTo(DgnElementR el)
    {
    return DgnElement::GenericMultiAspect::AddAspect(el, *m_instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr iModelSyncInfoAspect::MakeInstance(DgnElementId scope, Utf8CP kind, Utf8StringCR sourceId, SourceState const* ss, ECN::ECClassCR aspectClass) 
    {
    auto instance = aspectClass.GetDefaultStandaloneEnabler()->CreateInstance();
    instance->SetValue(SOURCEINFO_Scope, ECN::ECValue(scope));
    instance->SetValue(SOURCEINFO_SourceId, ECN::ECValue(sourceId.c_str()));
    instance->SetValue(SOURCEINFO_Kind, ECN::ECValue(kind));
    if (ss)
        SetSourceState(*instance, *ss);
    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP iModelSyncInfoAspect::GetAspectClass(DgnDbR db)
    {
    return db.Schemas().GetClass(SOURCEINFO_ECSCHEMA_NAME, SOURCEINFO_CLASS_SoureElementInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
iModelSyncInfoAspect iModelSyncInfoAspect::GetAspect(DgnElementCR el, ECN::ECClassCP aspectClass)
    {
    if (!aspectClass)
        aspectClass = GetAspectClass(el.GetDgnDb());
    if (nullptr == aspectClass)
        return iModelSyncInfoAspect();
    auto instance = DgnElement::GenericMultiAspect::GetAspect (el, *aspectClass, BeSQLite::EC::ECInstanceId()); // Get read-only copy of the aspect.
    if (nullptr == instance)
        return iModelSyncInfoAspect();
    return iModelSyncInfoAspect(instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
iModelSyncInfoAspect iModelSyncInfoAspect::GetAspect(DgnElementR el, ECN::ECClassCP aspectClass)
    {
    if (!aspectClass)
        aspectClass = GetAspectClass(el.GetDgnDb());
    if (nullptr == aspectClass)
        return iModelSyncInfoAspect();
    auto instance = DgnElement::GenericMultiAspect::GetAspectP(el, *aspectClass, BeSQLite::EC::ECInstanceId());    // NB: Call GetAspectP, not GetAspect! GetAspectP sets the aspect's dirty flag, which tells its _OnUpdate method to write out changes.
    if (nullptr == instance)
        return iModelSyncInfoAspect();
    return iModelSyncInfoAspect(instance);
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId iModelSyncInfoAspect::GetScope() const
    {
    ECN::ECValue v;
    m_instance->GetValue(v, SOURCEINFO_Scope);
    return v.GetNavigationInfo().GetId<DgnElementId>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP iModelSyncInfoAspect::GetSourceId() const
    {
    ECN::ECValue v;
    m_instance->GetValue(v, SOURCEINFO_SourceId);
    return v.GetUtf8CP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP iModelSyncInfoAspect::GetKind() const 
    {
    ECN::ECValue v;
    m_instance->GetValue(v, SOURCEINFO_Kind);
    return v.GetUtf8CP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelSyncInfoAspect::GetSourceState(SourceState& ss) const
    {
    ECN::ECValue v;
    m_instance->GetValue(v, SOURCEINFO_Hash);
    if (v.IsNull())
        return BSIERROR;
    size_t sz;
    auto b = v.GetBinary(sz);
    unsigned arraySize = sz / sizeof(unsigned char);
    ss.m_hash.insert(ss.m_hash.end(), b, &b[arraySize]);
    
    m_instance->GetValue(v, SOURCEINFO_LastModifiedTime);
    ss.m_lastModifiedTime = v.GetDouble();

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelSyncInfoAspect::SetProperties(rapidjson::Document const& json)
    {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);

    ECN::ECValue props(buffer.GetString());
    m_instance->SetValue(SOURCEINFO_Properties, props);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document iModelSyncInfoAspect::GetProperties() const
    {
    rapidjson::Document json;
    ECN::ECValue props;
    if (ECN::ECObjectsStatus::Success != m_instance->GetValue(props, SOURCEINFO_Properties) || !props.IsString())
        return json;
    json.Parse(props.GetUtf8CP());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelSyncInfoAspect::SetSourceState(ECN::IECInstanceR instance, SourceState const& ss)
    {
    instance.SetValue(SOURCEINFO_Hash, ECN::ECValue(&ss.m_hash[0], ss.m_hash.size()* sizeof(ss.m_hash[0])));
    instance.SetValue(SOURCEINFO_LastModifiedTime, ECN::ECValue(ss.m_lastModifiedTime));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int char2int(char input)
    {
    if (input == '\0')
        return 0;

    if (input >= '0' && input <= '9')
        return input - '0';
    if (input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    if (input >= 'a' && input <= 'f')
        return input - 'a' + 10;
    
    BeAssert(false);
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
iModelSyncInfoAspect::SourceState iModelBridgeSyncInfoFile::SourceState::GetAspectState() const
    {
    iModelSyncInfoAspect::SourceState state;
    state.m_lastModifiedTime = m_lmt;
    
    for (int index = 0; index < m_hash.size(); index = index + 2)
        {
        unsigned char value = char2int(m_hash[index]) * 16 + char2int(m_hash[index + 1]);
        state.m_hash.push_back(value);
        }

    return state;
    }

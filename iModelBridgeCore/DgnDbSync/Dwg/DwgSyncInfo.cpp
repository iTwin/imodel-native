/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgSyncInfo.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DwgImportInternal.h"

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"DwgImporter"))

#define MUSTBEDBRESULT(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {SetLastError(rc); return BSIERROR;}}
#define MUSTBEOK(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_OK)
#define MUSTBEROW(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_ROW)
#define MUSTBEDONE(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_DONE)

#define MUSTBEDBRESULTRC(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {SetLastError(rc); return rc;}}
#define MUSTBEOKRC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_OK)
#define MUSTBEROWRC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_ROW)
#define MUSTBEDONERC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_DONE)

BEGIN_DWG_NAMESPACE

/*---------------------------------------------------------------------------------------
SyncInfo versions:
0.1.0.0     => DgnDb06
0.2.0.0     => BIM02
---------------------------------------------------------------------------------------*/
static ProfileVersion    s_currentVersion(0, 2, 0, 0);
static Utf8Char         s_hexMaskChars[] = "0123456789ABCDEF";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::SavePropertyString(PropertySpecCR spec, Utf8CP stringData, uint64_t id, uint64_t subId)
    {
    Statement stmt;
    auto rc = stmt.Prepare(*m_dgndb, "INSERT OR REPLACE INTO " SYNCINFO_ATTACH(BEDB_TABLE_Property) " (Namespace,Name,Id,SubId,TxnMode,StrData) VALUES(?,?,?,?,?,?)");
    if (BE_SQLITE_OK != rc)
        return  rc;

    int col=1;
    stmt.BindText(col++, spec.GetNamespace(), Statement::MakeCopy::No);
    stmt.BindText(col++, spec.GetName(), Statement::MakeCopy::No);
    stmt.BindInt64(col++, id);
    stmt.BindInt64(col++, subId);
    stmt.BindInt(col++, 0);
    stmt.BindText(col++, stringData, Statement::MakeCopy::No);
    rc = stmt.Step();
    return (BE_SQLITE_DONE==rc) ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::QueryProperty(Utf8StringR value, PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    Statement stmt;
    DbResult rc = stmt.Prepare(*m_dgndb, "SELECT StrData FROM " SYNCINFO_ATTACH(BEDB_TABLE_Property) " WHERE Namespace=? AND Name=? AND Id=? AND SubId=?");
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::CreateTables()
    {
    if (nullptr == m_dgndb)
        {
        BeAssert(false);
        return BSIERROR;
        }

    if (m_dgndb->TableExists(SYNCINFO_ATTACH(SYNC_TABLE_File)))
        {
        ImportJob::CreateTable(*m_dgndb);
        m_dgndb->SaveChanges();
        return BSISUCCESS;
        }

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_File), 
                    "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "UniqueName CHAR NOT NULL UNIQUE,"
                    "DwgName CHAR NOT NULL,"
                    "VersionGuid CHAR NOT NULL,"
                    "LastSaveTime REAL,"
                    "LastModified BIGINT,"
                    "FileSize BIGINT,"
                    "UseHash BOOL");

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Model), 
                    "ModelId BIGINT NOT NULL,"
                    "DwgFileId INTEGER REFERENCES " SYNC_TABLE_File "(Id) ON DELETE CASCADE,"
                    "DwgModelId BIGINT,"
                    "DwgInstanceId BIGINT,"
                    "DwgName CHAR NOT NULL,"
                    "SourceType INT,"
                    "Transform BLOB");

    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Model) "NativeIdx ON "  SYNC_TABLE_Model "(ModelId)");
    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Model) "FileAndModel ON "  SYNC_TABLE_Model "(DwgFileId,DwgInstanceId)");

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Layer), 
                    "Id INT,"
                    "DwgFile INTEGER REFERENCES " SYNC_TABLE_File "(Id) ON DELETE CASCADE,"
                    "DwgModel BIGINT,"
                    "DwgObjectId BIGINT,"
                    "DwgName CHAR NOT NULL,"
                    "CONSTRAINT FileModelId UNIQUE(DwgFile,DwgModel,DwgObjectId)");

    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Layer) "NativeIdx ON "  SYNC_TABLE_Layer "(Id)");

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Linetype),
                    "Id INT,"
                    "DwgFile INTEGER REFERENCES " SYNC_TABLE_File "(Id) ON DELETE CASCADE,"
                    "DwgModel BIGINT,"
                    "DwgObjectId BIGINT,"
                    "DwgName CHAR NOT NULL,"
                    "CONSTRAINT FileModelId UNIQUE(DwgFile,DwgModel,DwgObjectId)");

    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Linetype) "NativeIdx ON "  SYNC_TABLE_Linetype "(Id)");

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_View),
                    "Id INT,"
                    "DwgObjectId BIGINT,"
                    "ViewportType INT,"
                    "DwgName CHAR");

    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_View) "NativeIdx ON "  SYNC_TABLE_View "(Id)");

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Material),
                    "Id INT,"
                    "DwgFileId INTEGER REFERENCES " SYNC_TABLE_File "(Id) ON DELETE CASCADE,"
                    "DwgObjectId BIGINT,"
                    "DwgName CHAR NOT NULL,"
                    "ObjectHash BLOB,"
                    "CONSTRAINT FileId UNIQUE(DwgFileId,DwgObjectId)");

    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Material) "NativeIdx ON "  SYNC_TABLE_Material "(Id)");

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Group),
                    "Id INT,"
                    "DwgFileId INTEGER REFERENCES " SYNC_TABLE_File "(Id) ON DELETE CASCADE,"
                    "DwgObjectId BIGINT,"
                    "DwgName CHAR NOT NULL,"
                    "ObjectHash BLOB,"
                    "CONSTRAINT FileId UNIQUE(DwgFileId,DwgObjectId)");

    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Group) "NativeIdx ON "  SYNC_TABLE_Material "(Id)");

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Element), 
                    "ElementId INT NOT NULL,"
                    "DwgFileId INT NOT NULL,"
                    "DwgModelSyncInfoId BIGINT NOT NULL,"
                    "DwgObjectId BIGINT,"
                    "PrimaryHash BLOB,"
                    "SecondaryHash BLOB");

    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Element) "ElementIdx ON " SYNC_TABLE_Element "(ElementId)");
    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Element) "DwgIdx ON " SYNC_TABLE_Element "(DwgModelSyncInfoId,DwgObjectId)");
    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Element) "HashIdx ON "  SYNC_TABLE_Element "(DwgModelSyncInfoId,PrimaryHash,SecondaryHash) WHERE DwgObjectId IS NULL");

    ImportJob::CreateTable(*m_dgndb);

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Discards), 
                    "DwgModelSyncInfoId BIGINT NOT NULL,"
                    "DwgObjectId BIGINT");

    MUSTBEOK(m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Discards) "DwgIdx ON " SYNC_TABLE_Discards "(DwgModelSyncInfoId,DwgObjectId)"));

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_GeometryPart),
                    "PartId INT NOT NULL,"
                    "PartTag TEXT");

    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_GeometryPart) "PartTagIdx ON " SYNC_TABLE_GeometryPart "(PartTag)");

    m_dgndb->SaveChanges();
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
BentleyStatus DwgSyncInfo::PerformVersionChecks()
    {
    //  Look at the stored version and see if we have to upgrade
    Utf8String versionString;
    MUSTBEROW(QueryProperty(versionString, SyncInfoProperty::ProfileVersion()));

    ProfileVersion storedVersion(0,0,0,0);
    storedVersion.FromJson(versionString.c_str());
        
    if (storedVersion.CompareTo(s_currentVersion) == 0)
        return BSISUCCESS;

    if (storedVersion.CompareTo(s_currentVersion) > 0)
        { // version is too new!
        LOG.errorv("compatibility error - storedVersion=%s > currentVersion=%s", versionString.c_str(), s_currentVersion.ToJson().c_str());
        return static_cast<BentleyStatus>(DgnDbStatus::VersionTooNew);
        }

    // do not attempt upgrading if major or minor version changes
    if (storedVersion.GetMajor() != s_currentVersion.GetMajor() || storedVersion.GetMinor() != s_currentVersion.GetMinor())
        {
        LOG.errorv("compatibility error - storedVersion=%s != currentVersion=%s", versionString.c_str(), s_currentVersion.ToJson().c_str());
        return static_cast<BentleyStatus>(DgnDbStatus::VersionTooOld);
        }

    //  Upgrade - when we change the syncInfo schema, add upgrade steps here ...
    
    //  Upgraded. Update the stored version.
    MUSTBEOK(SavePropertyString(SyncInfoProperty::ProfileVersion(), s_currentVersion.ToJson().c_str()));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::DiskFileInfo::GetInfo(BeFileNameCR fileName)
    {
    // *** WIP_CONVERTER - get file time IN FILETIME (hectonanoseconds), NOT SECONDS!

    time_t mtime;
    if (BeFileName::GetFileSize(m_fileSize, fileName.c_str()) != BeFileNameStatus::Success
     || BeFileName::GetFileTime(nullptr, nullptr, &mtime, fileName.c_str()) != BeFileNameStatus::Success)
        return BSIERROR;

    m_lastModifiedTime = mtime;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSyncInfo::ImportJob::CreateTable (BeSQLite::Db& db)
    {
    if (db.TableExists(SYNCINFO_ATTACH(SYNC_TABLE_ImportJob)))
        return;
    db.CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_ImportJob),
                         "DwgModelSyncInfoId INTEGER PRIMARY KEY,"
                         "SubjectId BIGINT NOT NULL,"
                         "Transform BLOB,"
                         "Type INTEGER,"
                         "Prefix TEXT");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DwgSyncInfo::ImportJob::Insert (BeSQLite::Db& db) const
    {
    Statement stmt;
    stmt.Prepare(db, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_ImportJob) "(DwgModelSyncInfoId,SubjectId,Transform,Type,Prefix) VALUES (?,?,?,?,?)");
    int col = 1;
    stmt.BindInt(col++, m_dwgRootModel.GetValue());
    stmt.BindId(col++, m_subjectId);
    stmt.BindBlob(col++, &m_transform, sizeof(m_transform), Statement::MakeCopy::No);
    stmt.BindInt(col++, (int)m_type);
    stmt.BindText(col++, m_prefix, Statement::MakeCopy::No);
    auto res = stmt.Step();
    m_ROWID = db.GetLastInsertRowId();
    return res;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DwgSyncInfo::ImportJob::Update (BeSQLite::Db& db) const
    {
    Statement stmt;
    stmt.Prepare(db, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_ImportJob) " SET SubjectId=?,Transform=?,Prefix=? WHERE(ROWID=?)");
    int col = 1;
    stmt.BindId(col++, m_subjectId);
    stmt.BindBlob(col++, &m_transform, sizeof(m_transform), Statement::MakeCopy::No);
    stmt.BindText(col++, m_prefix, Statement::MakeCopy::No);
    stmt.BindInt64(col++, m_ROWID);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::InsertImportJob(ImportJob const& importJob)
    {
    return (BE_SQLITE_DONE == importJob.Insert(*m_dgndb))? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::UpdateImportJob(ImportJob const& importJob)
    {
    return (BE_SQLITE_DONE == importJob.Update(*m_dgndb))? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DwgSyncInfo::ImportJob::GetSelectSql()
    {
    return "SELECT ROWID,DwgModelSyncInfoId,SubjectId,Transform,Type,Prefix FROM " SYNCINFO_ATTACH(SYNC_TABLE_ImportJob);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSyncInfo::ImportJob::FromSelect(BeSQLite::Statement& stmt)
    {
    int col = 0;
    m_ROWID = stmt.GetValueInt64(col++);
    SetDwgModelSyncInfoId (DwgModelSyncInfoId(stmt.GetValueInt(col++)));
    SetSubjectId (stmt.GetValueId<DgnElementId>(col++));
    memcpy(&m_transform, stmt.GetValueBlob(col++), sizeof(Transform));
    SetType ((Type)stmt.GetValueInt(col++));
    SetPrefix (stmt.GetValueText(col++));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::ImportJobIterator::ImportJobIterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);
    Utf8String sqlString = MakeSqlString(ImportJob::GetSelectSql().c_str());
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::ImportJob DwgSyncInfo::ImportJobIterator::ImportJobIterator::Entry::GetimportJob()
    {
    DwgSyncInfo::ImportJob importJob;
    importJob.FromSelect(*m_sql);
    return importJob;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::ImportJobIterator::Entry DwgSyncInfo::ImportJobIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::ImportJob::FindById (ImportJob& importJob, DgnDbCR db, DwgModelSyncInfoId const& modelSyncId)
    {
    if (!db.TableExists(SYNCINFO_ATTACH(SYNC_TABLE_ImportJob)))
        return BSIERROR;

    if (!modelSyncId.IsValid())
        return BSIERROR;

    ImportJobIterator iter(db, "DwgModelSyncInfoId=?");
    iter.GetStatement()->BindInt64 (1, modelSyncId.GetValue());
    auto i = iter.begin();
    if (i == iter.end())
        return BSIERROR;
    importJob.FromSelect(*iter.GetStatement());
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::FindImportJobById (ImportJob& importJob, DwgModelSyncInfoId const& modelSyncId)
    {
    return ImportJob::FindById(importJob, *m_dgndb, modelSyncId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedImportJob   DwgImporter::FindSoleImportJobForFile (DwgDbDatabaseR dwg)
    {
    DwgSyncInfo::FileProvenance provenance(dwg, m_syncInfo, _GetDwgFileIdPolicy());
    if (!provenance.FindByName(true))
        return ResolvedImportJob();

    auto fileId = this->GetDwgFileId (dwg, true);
    if (!fileId.IsValid())
        return ResolvedImportJob();

    Statement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT importJob.DwgModelSyncInfoId FROM " 
                 SYNCINFO_ATTACH(SYNC_TABLE_ImportJob) " importJob, "
                 SYNCINFO_ATTACH(SYNC_TABLE_Model) " model "
                 "WHERE model.DwgFileId=? AND importJob.DwgModelSyncInfoId = model.ROWID");
    stmt.BindInt (1, fileId.GetValue());
    if (stmt.Step() != BE_SQLITE_ROW)
        return ResolvedImportJob();
        
    DwgSyncInfo::DwgModelSyncInfoId modelSyncId = stmt.GetValueId<DwgSyncInfo::DwgModelSyncInfoId>(0);

    DwgSyncInfo::ImportJob importJob;
    GetSyncInfo().FindImportJobById(importJob, modelSyncId);    // grab the data now, before we step again

    if (BE_SQLITE_ROW == stmt.Step())                           // check that there is only ONE ImportJob record for this file
        {
        OnFatalError(IssueCategory::CorruptData(), Issue::Error(), "Multiple ImportJobs are registered for the root file. You must specify a root model in order to select the one you want to use.");
        BeAssert(false);
        return ResolvedImportJob();
        }

    return GetResolvedImportJob(importJob);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DwgSyncInfo::GetUniqueName(WStringCR fullFileName)
    {
    //  The unique name is the key into the syncinfo_file table. 
    //  Therefore, we must distinguish between like-named files in different directories.
    //  The unique name must also be stable. If the whole project is moved to a new directory or machine, 
    //  the unique names of the files must be unaffected.

    // If we have a DMS GUID for the document corresponding to this file, that is the unique name.
    BeGuid docGuid = GetDwgImporter().GetOptions().QueryDocumentGuid(BeFileName(fullFileName));
    if (docGuid.IsValid())
        {
        Utf8String lguid = docGuid.ToString();
        lguid.ToLower();
        return lguid;
        }

    // If we do not have a GUID, we try to compute a stable unique name from the filename.
    // The full path should be unique already. To get something that is stable, we use only as much of 
    // the full path as we need to distinguish between like-named files in different directories.
    WString uniqueName(fullFileName);
    auto pdir = GetDwgImporter().GetOptions().GetInputRootDir();
    if (!pdir.empty() && (pdir.size() < fullFileName.size()) && pdir.EqualsI(fullFileName.substr(0, pdir.size())))
        uniqueName = fullFileName.substr(pdir.size());

    uniqueName.ToLower();  // make sure we don't get fooled by case-changes in file system on Windows
    return Utf8String(uniqueName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::FileProvenance::FileProvenance(DwgDbDatabaseCR dwg, DwgSyncInfo& sync, StableIdPolicy policy) : m_syncInfo(sync)
    {
    m_idPolicy = policy;

    this->GetInfo (BeFileName(dwg.GetFileName().c_str()));

    m_versionGuid.Assign (dwg.GetVersionGuid().c_str());

    // get universial date as last edited time stamp
    DwgDbDate   tduupdate = dwg.GetTDUUPDATE ();
    m_lastSaveTime = tduupdate.GetJulianFraction ();

    WString     fullFilename(dwg.GetFileName().c_str());
    m_dwgName = Utf8String(fullFilename);
    m_uniqueName = sync.GetUniqueName (fullFilename);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::FileProvenance::FileProvenance(BeFileNameCR name, DwgSyncInfo& sync, StableIdPolicy policy) : m_syncInfo(sync)
    {
    m_idPolicy = policy;
    m_uniqueName = sync.GetUniqueName(name);                    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::FileProvenance::Insert ()
    {
    Statement stmt;
    stmt.Prepare(*m_syncInfo.m_dgndb, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_File) "(UniqueName,DwgName,VersionGuid,LastSaveTime,LastModified,FileSize,UseHash) VALUES (?,?,?,?,?,?,?)");

    int col = 1;
    stmt.BindText(col++, m_uniqueName, Statement::MakeCopy::No);
    stmt.BindText(col++, m_dwgName, Statement::MakeCopy::No);
    stmt.BindText(col++, m_versionGuid, Statement::MakeCopy::No);
    stmt.BindDouble(col++, m_lastSaveTime);
    stmt.BindInt64(col++, m_lastModifiedTime);
    stmt.BindInt64(col++, m_fileSize);
    stmt.BindInt(col++, m_idPolicy==StableIdPolicy::ByHash ? 1 : 0);

    DbResult rc = stmt.Step();
    BeAssert(rc == BE_SQLITE_DONE);

    auto rowid = m_syncInfo.m_dgndb->GetLastInsertRowId();
    BeAssert(rowid <= UINT32_MAX);
    m_syncId = DwgFileId((uint32_t)rowid);

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::FileProvenance::Update ()
    {
    if (!FindByName(false))
        return BE_SQLITE_ERROR;

    Statement stmt;
    stmt.Prepare(*m_syncInfo.m_dgndb, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_File) " SET VersionGuid=?,LastSaveTime=?,LastModified=?,FileSize=? WHERE Id=?");
    int col = 1;
    stmt.BindText(col++, m_versionGuid, Statement::MakeCopy::No);
    stmt.BindDouble(col++, m_lastSaveTime);
    stmt.BindInt64(col++, m_lastModifiedTime);
    stmt.BindInt64(col++, m_fileSize);
    stmt.BindInt(col++, m_syncId.GetValue());
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSyncInfo::FileProvenance::FindByName(bool fillLastMod)
    {
    CachedStatementPtr stmt;
    m_syncInfo.m_dgndb->GetCachedStatement(stmt, "SELECT Id,UniqueName,DwgName,VersionGuid,UseHash,LastSaveTime,LastModified,FileSize FROM " SYNCINFO_ATTACH(SYNC_TABLE_File) " WHERE UniqueName=?");
    stmt->BindText(1, m_uniqueName, Statement::MakeCopy::No);

    auto result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        {
        m_syncId = DwgFileId();
        return false;
        }

    int col=0;
    m_syncId        = DwgFileId(stmt->GetValueInt(col++));  // Id
    m_uniqueName    = stmt->GetValueText(col++);            // UniqueName
    m_dwgName       = stmt->GetValueText(col++);            // DwgName
    m_versionGuid   = stmt->GetValueText(col++);            // VersionGUID
    m_idPolicy      = stmt->GetValueInt(col++)==1 ? StableIdPolicy::ByHash : StableIdPolicy::ById;

    if (fillLastMod)
        {
        m_lastSaveTime      = stmt->GetValueDouble(col++);  // LastSaveTime
        m_lastModifiedTime  = stmt->GetValueInt64(col++);   // LastModified
        m_fileSize          = stmt->GetValueInt64(col++);   // FileSize
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSyncInfo::HasDiskFileChanged(BeFileNameCR fileName)
    {
    DwgSyncInfo::DiskFileInfo df;
    df.GetInfo(fileName);

    DwgSyncInfo::FileProvenance prov(fileName, *this, StableIdPolicy::ById);
    if (!prov.FindByName(true))
        return true;

    // This is an attempt to tell if a file has *not* changed, looking only at the file's time and size.
    // This is a dangerous test, since we don't look at the contents, but we think that we can narrow 
    // the odds of a mistake by:
    // 1. using times measured in hectonanoseconds (on Windows, at least),
    // 2. also using file size
    return df.m_lastModifiedTime != prov.m_lastModifiedTime || df.m_fileSize != prov.m_fileSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSyncInfo::HasLastSaveTimeChanged(DwgDbDatabaseCR dwg)
    {
    DwgSyncInfo::FileProvenance previous(dwg, *this, StableIdPolicy::ById);
    if (!previous.FindByName(true))
        return true;

    // last save time by TDUUPDATE is ideal, but a non-ACAD based product may not update it, so check both:
    double lastSaveTime = dwg.GetTDUUPDATE().GetJulianFraction();
    if (lastSaveTime != previous.m_lastSaveTime)
        return  true;
    // TDUUPDATE unchanged - also check disk time stamp:
    BeFileName  fn(dwg.GetFileName().c_str());
    return  this->HasDiskFileChanged(fn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSyncInfo::HasVersionGuidChanged (DwgDbDatabaseCR dwg)
    {
    DwgSyncInfo::FileProvenance previous(dwg, *this, StableIdPolicy::ById);
    if (!previous.FindByName(true))
        return true;

    Utf8String  versionGUID (dwg.GetVersionGuid().c_str());
    return 0 != versionGUID.CompareTo (previous.m_versionGuid);
    }

DwgSyncInfo::DwgFileId DwgSyncInfo::FileIterator::Entry::GetSyncId() {return DwgSyncInfo::DwgFileId(m_sql->GetValueInt(0));}
Utf8String DwgSyncInfo::FileIterator::Entry::GetUniqueName() {return m_sql->GetValueText(1);}
Utf8String DwgSyncInfo::FileIterator::Entry::GetDwgName() {return m_sql->GetValueText(2);}
Utf8String DwgSyncInfo::FileIterator::Entry::GetVersionGuid() {return m_sql->GetValueText(3);}
bool DwgSyncInfo::FileIterator::Entry::GetCannotUseElementIds() {return 0 != m_sql->GetValueInt(4);}
double DwgSyncInfo::FileIterator::Entry::GetLastSaveTime() {return m_sql->GetValueDouble(5);}
uint64_t DwgSyncInfo::FileIterator::Entry::GetLastModifiedTime() {return m_sql->GetValueInt64(6);}
uint64_t DwgSyncInfo::FileIterator::Entry::GetFileSize() {return m_sql->GetValueInt64(7);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::FileIterator::FileIterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);
    Utf8String sqlString = MakeSqlString("SELECT Id,UniqueName,DwgName,VersionGuid,UseHash,LastSaveTime,LastModified,FileSize FROM " SYNCINFO_ATTACH(SYNC_TABLE_File));
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::FileIterator::Entry DwgSyncInfo::FileIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::DeleteFile (DwgFileId fileId)
    {
    Statement stmt;
    stmt.Prepare(*m_dgndb, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_File) " WHERE ROWID=?");
    stmt.BindInt64(1, fileId.GetValue());
    return stmt.Step() == BE_SQLITE_DONE ? BSISUCCESS : BSIERROR;
    }

DwgSyncInfo::DwgModelSyncInfoId DwgSyncInfo::ModelIterator::Entry::GetDwgModelSyncInfoId() {return DwgModelSyncInfoId(m_sql->GetValueInt64(0));}
DgnModelId DwgSyncInfo::ModelIterator::Entry::GetModelId() {return m_sql->GetValueId<DgnModelId>(1);}
DwgSyncInfo::DwgFileId DwgSyncInfo::ModelIterator::Entry::GetDwgFileId() {return DwgFileId(m_sql->GetValueInt(2));}
DwgSyncInfo::DwgModelId DwgSyncInfo::ModelIterator::Entry::GetDwgModelId() {return DwgModelId(m_sql->GetValueInt(3));}
uint64_t DwgSyncInfo::ModelIterator::Entry::GetDwgModelInstanceId() {return m_sql->GetValueUInt64(4);}
Utf8CP DwgSyncInfo::ModelIterator::Entry::GetDwgName() {return m_sql->GetValueText(5);}
DwgSyncInfo::ModelSourceType DwgSyncInfo::ModelIterator::Entry::GetSourceType() {return static_cast<DwgSyncInfo::ModelSourceType>(m_sql->GetValueInt(6));}
Transform DwgSyncInfo::ModelIterator::Entry::GetTransform()
    {
    if (m_sql->IsColumnNull(7))
        return Transform::FromIdentity();

    Transform t; 
    memcpy(&t, m_sql->GetValueBlob(7), sizeof(t)); 
    return t;
    }
DwgSyncInfo::DwgModelMapping DwgSyncInfo::ModelIterator::Entry::GetMapping ()
    {
    DwgModelMapping mapping;
    mapping.SetDwgModelSyncInfoId (this->GetDwgModelSyncInfoId());
    mapping.SetSource (DwgSyncInfo::DwgModelSource(this->GetDwgFileId(), this->GetDwgModelId()));
    mapping.SetModelId (this->GetModelId());
    mapping.SetTransform (this->GetTransform());
    mapping.SetDwgName (this->GetDwgName());
    mapping.SetSourceType (this->GetSourceType());
    mapping.SetDwgModelInstanceId (this->GetDwgModelInstanceId());
    return  mapping;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::ModelIterator::ModelIterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);
    Utf8String sqlString = MakeSqlString("SELECT ROWID,ModelId,DwgFileId,DwgModelId,DwgInstanceId,DwgName,SourceType,Transform FROM " SYNCINFO_ATTACH(SYNC_TABLE_Model));
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::ModelIterator::Entry DwgSyncInfo::ModelIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::DwgModelMapping::Insert(Db& db) const
    {
    if (!m_id.IsValid())
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    Statement stmt;
    stmt.Prepare(db, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Model) " (ModelId,DwgFileId,DwgModelId,DwgInstanceId,DwgName,SourceType,Transform) VALUES (?,?,?,?,?,?,?)");
    int col = 1;
    stmt.BindId(col++, m_id);
    stmt.BindInt(col++, m_source.GetDwgFileId().GetValue());
    stmt.BindInt64(col++, m_source.GetDwgModelId().GetValue());
    stmt.BindInt64(col++, m_instanceId);
    stmt.BindText(col++, m_dwgName, Statement::MakeCopy::No);
    stmt.BindInt(col++, static_cast<int>(m_sourceType));
    if (m_transform.IsIdentity())
        stmt.BindNull(col++);
    else
        stmt.BindBlob(col++, &m_transform, sizeof(m_transform), Statement::MakeCopy::No);

    auto rc = stmt.Step();
    if (BE_SQLITE_DONE == rc)
        m_syncInfoId = DwgModelSyncInfoId(db.GetLastInsertRowId());

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/17
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::DwgModelMapping::Update (Db& db) const
    {
    if (!m_id.IsValid() || !m_syncInfoId.IsValid() || m_instanceId == 0)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    Statement stmt;
    stmt.Prepare(db, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_Model) " SET ModelId=?,DwgFileId=?,DwgModelId=?,DwgInstanceId=?,DwgName=?,SourceType=?,Transform=? WHERE (ROWID=?)");
    int col = 1;
    stmt.BindId(col++, m_id);
    stmt.BindInt(col++, m_source.GetDwgFileId().GetValue());
    stmt.BindInt(col++, m_source.GetDwgModelId().GetValue());
    stmt.BindInt64(col++, m_instanceId);
    stmt.BindText(col++, m_dwgName, Statement::MakeCopy::No);
    stmt.BindInt(col++, static_cast<int>(m_sourceType));
    if (m_transform.IsIdentity())
        stmt.BindNull(col++);
    else
        stmt.BindBlob(col++, &m_transform, sizeof(m_transform), Statement::MakeCopy::No);
    stmt.BindInt64(col++, m_syncInfoId.GetValue());

    auto rc = stmt.Step();
    BeAssert(BE_SQLITE_DONE == rc);
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgModelMapping::DwgModelMapping ()
    {
    m_sourceType = ModelSourceType::ModelSpace;
    m_instanceId = 0;
    m_transform.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgModelMapping::DwgModelMapping(DgnModelId mid, DwgDbBlockTableRecordCR block, TransformCR trans)
    {
    // construct a model/paperspace model mapping
    m_dwgName = Utf8String(block.GetName().c_str());

    DwgDbDatabasePtr    dwg = block.GetDatabase().get ();
    if (dwg.IsNull())
        {
        BeAssert (false && L"DWG block not a database resident!");
        m_source = DwgModelSource (block.GetObjectId());
        }
    else
        {
        m_source = DwgModelSource (DwgSyncInfo::DwgFileId::GetFrom(*dwg), DwgModelId(block.GetObjectId().ToUInt64()));
        }

    m_transform  = trans;
    m_id = mid;
    m_sourceType = block.IsModelspace() ? ModelSourceType::ModelSpace : ModelSourceType::PaperSpace;
    m_instanceId = block.GetObjectId().ToUInt64 ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgModelMapping::DwgModelMapping(DgnModelId mid, DwgDbBlockReferenceCR xrefInsert, DwgDbDatabaseR xrefDwg, TransformCR trans)
    {
    // construct an xRef attachment model mapping
    DwgDbObjectId   xrefId = xrefDwg.GetModelspaceId ();

    m_source = DwgModelSource (DwgSyncInfo::DwgFileId::GetFrom(xrefDwg), DwgModelId(xrefId.ToUInt64()));
    m_transform  = trans;
    m_id = mid;
    m_sourceType = ModelSourceType::XRefAttachment;
    m_instanceId = xrefInsert.GetObjectId().ToUInt64 ();

    // open the block and get the block name:
    DwgDbBlockTableRecordPtr    block (xrefInsert.GetBlockTableRecordId(), DwgDbOpenMode::ForRead);
    if (block.OpenStatus() == DwgDbStatus::Success)
        m_dwgName.Assign (block->GetName().c_str());
    else
        m_dwgName.assign ("");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgModelMapping::DwgModelMapping(DgnModelId mid, DwgDbRasterImageCR raster, TransformCR trans)
    {
    // construct a raster attachment model mapping
    DwgString   fileName;
    if (DwgDbStatus::Success != raster.GetFileName(fileName))
        {
        BeAssert (false && L"Invalid DWG raster image!");
        m_dwgName.assign ("");
        }
    else
        {
        m_dwgName.Assign (fileName.c_str());
        }

    DwgDbDatabasePtr    dwg = raster.GetDatabase ();
    if (dwg.IsNull())
        {
        BeAssert (false && L"DWG raster image not a database resident!");
        m_source = DwgModelSource (raster.GetImageDefinitionId());
        }
    else
        {
        m_source = DwgModelSource (DwgSyncInfo::DwgFileId::GetFrom(*dwg), DwgModelId(raster.GetObjectId().ToUInt64()));
        }

    m_transform = trans;
    m_id = mid;
    m_sourceType = ModelSourceType::RasterAttachment;
    m_instanceId = raster.GetObjectId().ToUInt64 ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::InsertModel(DwgModelMapping& modelMap, DgnModelId mid, DwgDbBlockTableRecordCR block, TransformCR trans)
    {
    // insert a modelspace or a paperspace model:
    modelMap = DwgModelMapping(mid, block, trans);

    auto rc = modelMap.Insert(*m_dgndb);

    return (BE_SQLITE_DONE==rc) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::InsertModel(DwgModelMapping& modelMap, DgnModelId mid, DwgDbBlockReferenceCR xrefInsert, DwgDbDatabaseR xrefDwg, TransformCR trans)
    {
    // insert an xref model
    modelMap = DwgModelMapping(mid, xrefInsert, xrefDwg, trans);

    auto rc = modelMap.Insert(*m_dgndb);

    return (BE_SQLITE_DONE==rc) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::InsertModel(DwgModelMapping& modelMap, DgnModelId mid, DwgDbRasterImageCR raster, TransformCR trans)
    {
    // insert a raster model
    modelMap = DwgModelMapping(mid, raster, trans);

    auto rc = modelMap.Insert(*m_dgndb);

    return (BE_SQLITE_DONE==rc) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::DeleteModel(DwgSyncInfo::DwgModelSyncInfoId const& modelSyncId)
    {
    // delete model by ROWID
    Statement stmt;
    stmt.Prepare(*m_dgndb, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_Model) " WHERE ROWID=?");
    stmt.BindInt64 (1, modelSyncId.GetValue());
    return stmt.Step() == BE_SQLITE_DONE? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgFileId   DwgSyncInfo::DwgFileId::GetFrom (DwgDbDatabaseR dwg)
    {
    uint32_t    fileId = 0, idPolicy = 0;
    if (DwgDbStatus::Success != dwg.GetFileIdPolicy(fileId, idPolicy))
        return  DwgFileId ();

    return  DwgFileId (fileId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgFileId   DwgSyncInfo::GetDwgFileId (DwgDbDatabaseR dwg)
    {
    return DwgSyncInfo::DwgFileId::GetFrom (dwg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
StableIdPolicy  DwgSyncInfo::GetFileIdPolicy (DwgDbDatabaseR dwg)
    {
    uint32_t    fileId = 0, idPolicy = 0;
    if (DwgDbStatus::Success != dwg.GetFileIdPolicy(fileId, idPolicy))
        idPolicy = 0;

    return static_cast<StableIdPolicy> (idPolicy);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgModelSource::DwgModelSource (DwgDbObjectIdCR dwgModelId)
    {
    // constructor by objectID of a modelspace, a paperspace, or a raster attachment. For an xref, it must be xref's modelspace.
    if (!dwgModelId.IsObjectDerivedFrom(DwgDbBlockTableRecord::SuperDesc()) && !dwgModelId.IsObjectDerivedFrom(DwgDbRasterImage::SuperDesc()))
        {
        BeAssert (false && "unsupported model source type!");
        m_fileId.Invalidate ();
        m_modelId.Invalidate ();
        return;
        }

    m_modelId = DwgModelId (dwgModelId.ToUInt64());

    DwgDbDatabasePtr    dwg = dwgModelId.GetDatabase ();
    if (dwg.IsNull())
        m_fileId.Invalidate ();
    else
        m_fileId = DwgSyncInfo::DwgFileId::GetFrom (*dwg);
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgModelSource::DwgModelSource (DwgDbObjectCP dwgModel, DwgDbDatabaseP xrefDwg)
    {
    // constructor by a model object. For an xref attachment, xrefDwg must be supplied.
    m_modelId.Invalidate ();
    m_fileId.Invalidate ();

    if (nullptr == dwgModel ||
        (nullptr == DwgDbBlockTableRecord::Cast(dwgModel) &&
         nullptr == DwgDbBlockReference::Cast(dwgModel) &&
         nullptr == DwgDbRasterImage::Cast(dwgModel)))
        {
        BeAssert (false && "Invalid object type for DwgModelSource!");
        return;
        }

    bool    isXref = nullptr != DwgDbBlockReference::Cast (dwgModel);
    if (isXref && nullptr == xrefDwg)
        {
        BeAssert (false && "an xRef DwgModelSource requires its xref DWG!");
        return;
        }

    DwgDbDatabaseP  dwg = nullptr == xrefDwg ? dwgModel->GetDatabase().get() : xrefDwg;
    if (nullptr != dwg)
        m_fileId = DwgFileId::GetFrom (*dwg);
    else
        BeAssert (false && L"Only a persistent DWG model is valid for DwgModelSource!!");

    if (isXref && nullptr != xrefDwg)
        m_modelId = DwgModelId (xrefDwg->GetModelspaceId().ToUInt64());
    else
        m_modelId = DwgModelId (dwgModel->GetObjectId().ToUInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::FindModel (DwgSyncInfo::DwgModelMapping* mapping, DwgDbObjectIdCR modelId, TransformCP trans)
    {
    // find a DgnModel by DWG model object ID, optionally matching a transformation
    if (!modelId.IsValid())
        return  BSIERROR;

    // Note: We can't call DwgImporter::GetDwgModelFromSyncInfo at this stage, because it hasn't set up the m_dwgFiles array yet
    DwgSyncInfo::FileProvenance provenance (*modelId.GetDatabase(), *this, m_dwgImporter.GetCurrentIdPolicy());
    if (!provenance.FindByName(false))
        return BSIERROR;

    DwgSyncInfo::ModelIterator iter (*m_dgndb, "DwgFileId=? AND DwgModelId=?");
    iter.GetStatement()->BindInt (1, provenance.GetDwgFileId().GetValue());
    iter.GetStatement()->BindInt64 (2, modelId.ToUInt64());

    for (auto entry=iter.begin(); entry!=iter.end(); ++entry)
        {
        if (nullptr == trans || entry.GetTransform().IsEqual(*trans))
            {
            if (nullptr != mapping)
                *mapping = entry.GetMapping ();
            return BSISUCCESS;
            }
        }

    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::FindModel (DwgSyncInfo::DwgModelMapping* mapping, DwgModelSyncInfoId syncInfoId)
    {
    DwgSyncInfo::ModelIterator iter (*m_dgndb, "ROWID=?");

    iter.GetStatement()->BindInt64 (1, syncInfoId.GetValue());

    for (auto entry = iter.begin(); entry != iter.end(); ++entry)
        {
        if (syncInfoId == entry.GetDwgModelSyncInfoId())
            {
            if (nullptr != mapping)
                *mapping = entry.GetMapping();
            return BSISUCCESS;
            }
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::UpdateElement(DgnElementId elementId, DwgDbEntityCR entity, DwgObjectProvenance const& prov)
    {
    if (!elementId.IsValid())
        {
        BeAssert(false && L"A model element must come from DWG entity type!");
        return BSIERROR;
        }

    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_Element) " SET PrimaryHash=?,SecondaryHash=? WHERE ElementId=?");

    int col=1;
    stmt->BindBlob(col++, &prov.m_primaryHash, sizeof(prov.m_primaryHash), Statement::MakeCopy::No);
    if (prov.HasSecondaryHash())
        stmt->BindBlob(col++, &prov.m_secondaryHash, sizeof(prov.m_secondaryHash), Statement::MakeCopy::No);
    else
        stmt->BindNull(col++);
    stmt->BindId(col++, elementId);
    return (stmt->Step() == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::InsertElement(DgnElementId elementId, DwgDbEntityCR entity, DwgObjectProvenance const& prov, DwgModelSyncInfoId const& modelSyncId)
    {
    DwgDbObjectId       entityId = entity.GetObjectId ();
    if (!elementId.IsValid() || !modelSyncId.IsValid() || !entityId.IsValid())
        {
        BeAssert(false && L"An invalid model element to be inserted into DwgSyncInfo!");
        return BSIERROR;
        }

    DwgFileId   fileId = DwgSyncInfo::DwgFileId::GetFrom (*entity.GetDatabase());
    if (!fileId.IsValid())
        {
        BeDataAssert(false && L"Can't insert a non-database entity into DwgSyncInfo!");
        fileId = DwgSyncInfo::DwgFileId::GetFrom (this->GetDwgImporter().GetDwgDb());
        }

    if (m_dwgImporter.IsUpdating())
        this->DeleteDiscardedDwgObject (entityId, modelSyncId); // just in case it was previously recorded as a discard

    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Element) " (ElementId,DwgFileId,DwgModelSyncInfoId,DwgObjectId,PrimaryHash,SecondaryHash) VALUES (?,?,?,?,?,?)");

    int col=1;
    stmt->BindId (col++, elementId);
    stmt->BindInt (col++, fileId.GetValue());
    stmt->BindInt64 (col++, modelSyncId.GetValue());

    if (prov.m_idPolicy==StableIdPolicy::ById)
        stmt->BindInt64(col++, entityId.ToUInt64());
    else
        stmt->BindNull(col++);

    stmt->BindBlob(col++, &prov.m_primaryHash, sizeof(prov.m_primaryHash), Statement::MakeCopy::No);

    if (prov.HasSecondaryHash())
        stmt->BindBlob(col++, &prov.m_secondaryHash, sizeof(prov.m_secondaryHash), Statement::MakeCopy::No);
    else
        stmt->BindNull(col++);
    return (stmt->Step() == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::DeleteElement(DwgDbObjectIdCR objid, DwgModelSyncInfoId const& modelSyncId)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_Element) " WHERE DwgModelSyncInfoId=? AND DwgObjectId=?");
    int col = 1;
    stmt->BindInt64 (col++, modelSyncId.GetValue());
    stmt->BindInt64 (col++, objid.ToUInt64());
    return (stmt->Step() == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::DeleteElement(DgnElementId gid)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_Element) " WHERE ElementId=?");
    stmt->BindId(1, gid);
    return (stmt->Step() == BE_SQLITE_DONE)? BSISUCCESS: BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle      03/15
//+---------------+---------------+---------------+---------------+---------------+------
bool DwgSyncInfo::TryFindElement(DgnElementId& elementId, DwgDbObjectCP obj, DwgModelSyncInfoId const& modelSyncId) const
    {
    elementId = DgnElementId ();
    if (nullptr == obj)
        return  false;

    CachedStatementPtr stmt = nullptr;
    m_dgndb->GetCachedStatement(stmt, "SELECT ElementId FROM " SYNCINFO_ATTACH(SYNC_TABLE_Element) " WHERE DwgModelSyncInfoId=? AND DwgObjectId=?");

    stmt->BindInt64 (1, modelSyncId.GetValue());
    stmt->BindInt64 (2, obj->GetObjectId().ToUInt64());

    const DbResult stat = stmt->Step();
    if (BE_SQLITE_ROW == stat)
        {
        if (!stmt->IsColumnNull(0))
            {
            elementId = DgnElementId(stmt->GetValueUInt64(0));
            return true;
            }
        }

    BeAssert(BE_SQLITE_DONE == stmt->Step() && "DwgSyncInfo::TryFindElement is expected to only return one row from the SELECT");
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSyncInfo::FindElements (DgnElementIdSet& ids, DwgDbObjectIdCR objectId) const
    {
    // query all elements mapped from input DWG object:
    auto dwg = objectId.GetDatabase ();
    if (nullptr == dwg)
        return  false;

    ElementIterator elemsInFile(*m_dgndb, "DwgFileId=? AND DwgObjectId=?");

    elemsInFile.GetStatement()->BindInt64 (1, DwgFileId::GetFrom(*dwg).GetValue());
    elemsInFile.GetStatement()->BindInt64 (2, objectId.ToUInt64());

    for (auto elem : elemsInFile)
        ids.insert (elem.GetElementId());

    return  !ids.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSyncInfo::IsMappedToSameDwgObject (DgnElementId elementId, DgnElementIdSet const& known) const
    {
    ElementIterator findByBimId(*m_dgndb, "ElementId=?");
    findByBimId.GetStatement()->BindId(1, elementId);
    auto iThisElement = findByBimId.begin();
    if (iThisElement == findByBimId.end())
        return false;

    if (iThisElement.GetDwgObjectId() == 0)
        {
        // when StableIdPolicy==ByHash
        V8ElementExternalSourceAspectIteratorByChecksum  othersMappedToDwgHash(*m_dgndb);
        othersMappedToDwgHash.Bind(iThisElement.GetDwgModelSyncInfoId(), iThisElement.GetProvenance().GetPrimaryHash(), iThisElement.GetProvenance().GetSecondaryHash());
        for (auto const& otherMappedToDwgHash : othersMappedToDwgHash)
            {
            if (known.find(otherMappedToDwgHash.GetElementId()) != known.end())
                return true;
            }
        }
    else
        {
        // when StableIdPolicy==ById
        ByDwgObjectIdIter othersMappedToDwgId(*m_dgndb);
        othersMappedToDwgId.Bind(iThisElement.GetDwgModelSyncInfoId(), iThisElement.GetDwgObjectId());
        for (auto const& otherMappedToDwgId : othersMappedToDwgId)
            {
            if (known.find(otherMappedToDwgId.GetElementId()) != known.end())
                return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgObjectProvenance::DwgObjectProvenance(StatementP sql) 
    {
    memcpy (&m_primaryHash, sql->GetValueBlob(4), sizeof(m_primaryHash));
    if (!sql->IsColumnNull(5))
        memcpy (&m_secondaryHash, sql->GetValueBlob(5), sizeof(m_secondaryHash));
    else
        memset (&m_secondaryHash, 0, sizeof(m_secondaryHash));
    m_idPolicy = sql->IsColumnNull(3) ? StableIdPolicy::ByHash : StableIdPolicy::ById;
    m_syncAsmBodyInFull = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgObjectProvenance::DwgObjectProvenance (DwgDbObjectCR obj, DwgSyncInfo& sync, StableIdPolicy policy, bool hash2nd)
    {
    m_idPolicy = policy;
    memset (&m_primaryHash, 0, sizeof(m_primaryHash));
    memset (&m_secondaryHash, 0, sizeof(m_secondaryHash));

    // optionally single out ASM objects for the sake of performance
    m_syncAsmBodyInFull = sync.GetDwgImporter().GetOptions().GetSyncAsmBodyInFull ();

    if (!m_syncAsmBodyInFull && BSISUCCESS == this->CreateAsmObjectHash(obj))
        {
        m_primaryHash = m_hasher.GetHashVal ();
        return;
        }

    // hash the DWG object via DxfFiler
    m_hasher.Reset ();
    DwgObjectHash::HashFiler filer(m_hasher, obj);
    if (filer.IsValid())
        {
        // the primary hash is from the object data itself.
        obj.DxfOut (filer);
        this->AppendComplexObjectHash (filer, obj);
        m_primaryHash = m_hasher.GetHashVal ();

        if (hash2nd)
            {
            // this is an INSERT entity and the option SyncBlockChanges is turned on, do secondary hash.
            DwgDbBlockReferenceCP   insert = DwgDbBlockReference::Cast (&obj);
            if (nullptr != insert)
                {
                m_hasher.Reset ();
                if (BSISUCCESS == this->CreateBlockHash(insert->GetBlockTableRecordId()))
                    m_secondaryHash = m_hasher.GetHashVal ();
                }
            }
        }

    BeAssert (!m_primaryHash.IsNull() && L"DwgObjectProvenance has no hash!");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSyncInfo::DwgObjectProvenance::AppendComplexObjectHash (DwgObjectHash::HashFiler& filer, DwgDbObjectCR obj)
    {
    DwgDbEntityCP   entity = DwgDbEntity::Cast(&obj);
    if (nullptr == entity)
        return;

    // handle complex objects which require additional primary hash:
    if (obj.GetDwgClassName().StartsWithI(L"AecDb"))
        {
        // Aec objects do not dxfOut actual data - add range for now - TFS 853852:
        DRange3d    range;
        if (entity->GetRange(range) == DwgDbStatus::Success)
            m_hasher.Add (&range, sizeof(range));
        return;
        }

    if (obj.IsAProxy())
        {
        // a proxy entity does not file out DXF group code 310 - need more data, TFS 933725.
        DwgDbObjectPArray   proxy;
        if (DwgDbStatus::Success == entity->Explode(proxy))
            {
            for (auto ent : proxy)
                {
                ent->DxfOut (filer);
                // operator delete is hidden by Teigha!
                ::free (ent);
                }
            }
        return;
        }

    // 2D/3D polyline and polyface/polygon mesh entities have vertex entities to follow.
    DwgDb2dPolylineCP   pline2d = nullptr;
    DwgDb3dPolylineCP   pline3d = nullptr;
    DwgDbPolyFaceMeshCP pfmesh = nullptr;
    DwgDbPolygonMeshCP  mesh = nullptr;
    DwgDbObjectIteratorPtr  vertexIter;
    if ((pline2d = DwgDb2dPolyline::Cast(entity)) != nullptr)
        vertexIter = pline2d->GetVertexIterator ();
    else if ((pline3d = DwgDb3dPolyline::Cast(entity)) != nullptr)
        vertexIter = pline3d->GetVertexIterator ();
    else if ((pfmesh = DwgDbPolyFaceMesh::Cast(entity)) != nullptr)
        vertexIter = pfmesh->GetVertexIterator ();
    else if ((mesh = DwgDbPolygonMesh::Cast(entity)) != nullptr)
        vertexIter = mesh->GetVertexIterator ();

    if (vertexIter.IsValid() && vertexIter->IsValid())
        {
        for (vertexIter->Start(); !vertexIter->Done(); vertexIter->Next())
            {
            DwgDbEntityPtr vertex(vertexIter->GetObjectId(), DwgDbOpenMode::ForRead);
            if (vertex.OpenStatus() == DwgDbStatus::Success)
                vertex->DxfOut (filer);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::DwgObjectProvenance::CreateBlockHash (DwgDbObjectIdCR blockId)
    {
    DwgDbBlockTableRecordPtr    block(blockId, DwgDbOpenMode::ForRead);
    if (block.IsNull() || block->IsExternalReference() || block->IsLayout())
        return  BSIERROR;

    // this is a performance dragger but unfortunately we cannot use existing hashed blocks as they won't match!
    DwgDbBlockChildIteratorPtr  iter = block->GetBlockChildIterator ();
    if (!iter.IsValid() || !iter->IsValid())
        return  BSIERROR;

    for (iter->Start(); !iter->Done(); iter->Step())
        {
        DwgDbEntityPtr  entity(iter->GetEntityId(), DwgDbOpenMode::ForRead);
        if (entity.IsNull())
            continue;

        DwgDbObjectP    child = DwgDbObject::Cast(entity.get());
        if (nullptr == child)
            continue;

        // optionally single out ASM objects for the sake of performance
        if (!m_syncAsmBodyInFull && BSISUCCESS == this->CreateAsmObjectHash(*child))
            continue;

        // hash current child entity itself and add to output hasher:
        DwgObjectHash::HashFiler filer(m_hasher, *child);
        if (filer.IsValid())
            child->DxfOut (filer);

        // if this is an INSERT entity, recurse into the block and hash all nested blocks as well:
        DwgDbBlockReferenceP    insert = DwgDbBlockReference::Cast (child);
        if (nullptr != insert)
            this->CreateBlockHash (insert->GetBlockTableRecordId());
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::DwgObjectProvenance::CreateAsmObjectHash (DwgDbObjectCR obj)
    {
    /*-----------------------------------------------------------------------------------
    DxfOut from an ASM entity outputs the whole Brep data and is unnecessarily expensive.
    Select and only hash some properties for the sake of performance.
    -----------------------------------------------------------------------------------*/
    auto entity = DwgDbEntity::Cast (&obj);
    if (nullptr == entity)
        return  BSIERROR;

    auto solid3d = DwgDb3dSolid::Cast (entity);
    auto body = DwgDbBody::Cast (entity);
    auto region = DwgDbRegion::Cast (entity);
    if (nullptr == solid3d && nullptr == body && nullptr == region)
        return  BSIERROR;

    DRange3d    range;
    if (entity->GetRange(range) != DwgDbStatus::Success)
        return  BSIERROR;

    auto ecs = Transform::FromIdentity ();
    entity->GetEcs (ecs);

    auto color = entity->GetColor().GetMRGB ();
    auto layer = entity->GetLayerId().ToUInt64 ();
    auto material = entity->GetMaterialId().ToUInt64 ();
    auto transparency = entity->GetTransparency().SerializeOut ();
    bool visibility = entity->GetVisibility() == DwgDbVisibility::Visible;

    // hash entity properties
    m_hasher.Add (&range, sizeof(range));
    m_hasher.Add (&ecs, sizeof(ecs));
    m_hasher.Add (&color, sizeof(color));
    m_hasher.Add (&layer, sizeof(layer));
    m_hasher.Add (&material, sizeof(material));
    m_hasher.Add (&transparency, sizeof(transparency));
    m_hasher.Add (&visibility, sizeof(visibility));

    // hash solid properties
    if (nullptr != solid3d)
        {
        auto numChanges = solid3d->GetNumChanges ();
        m_hasher.Add (&numChanges, sizeof(numChanges));
        
        DwgDb3dSolid::MassProperties    massProps;
        if (DwgDbStatus::Success == solid3d->GetMassProperties(massProps))
            m_hasher.Add (&massProps, sizeof(DwgDb3dSolid::MassProperties));

        double area = 0.0;
        if (DwgDbStatus::Success == solid3d->GetArea(area))
            m_hasher.Add (&area, sizeof(area));
        }
    else if (nullptr != body)
        {
        auto numChanges = body->GetNumChanges ();
        m_hasher.Add (&numChanges, sizeof(numChanges));
        }
    else if (nullptr != region)
        {
        auto numChanges = region->GetNumChanges ();
        m_hasher.Add (&numChanges, sizeof(numChanges));
        double area = 0.0;
        if (DwgDbStatus::Success == region->GetArea(area))
            m_hasher.Add (&area, sizeof(area));
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DwgSyncInfo::DwgObjectHash::IsNull () const
    {
    for (size_t i = 0; i < BentleyApi::MD5::BlockSize; i++)
        {
        if (m_buffer[i] != 0)
            return  false;
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  DwgSyncInfo::DwgObjectHash::AsHexString (Utf8StringR outString)
    {
    outString.clear ();

    size_t  nBytes = sizeof (m_buffer);

    for (size_t i = 0; i < nBytes; i++)
        {
        outString.push_back (s_hexMaskChars[(m_buffer[i] >> 4) & 0x0F]);
        outString.push_back (s_hexMaskChars[m_buffer[i] & 0x0F]);
        }

    return  nBytes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, int8_t v) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, int16_t v) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, int32_t v) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, int64_t v) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, uint8_t v) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, uint16_t v) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, uint32_t v) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, uint64_t v) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, bool v) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, double v, DoublePrecision prec) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, DPoint2dCR v, DoublePrecision prec) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, DPoint3dCR v, DoublePrecision prec) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, DVec2dCR v, DoublePrecision prec) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, DVec3dCR v, DoublePrecision prec) { return Add(&v, sizeof(v)); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, DwgStringCR v) { return Add(v.AsBufferPtr(), v.GetBufferSize()); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, DwgBinaryDataCR v) { return Add(v.GetBuffer(), v.GetSize()); }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, DwgDbHandleCR v)
    {
    uint64_t    handle = v.AsUInt64 ();
    m_hasher.Add (&handle, sizeof(handle));
    return DwgDbStatus::Success;
    }  
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, DwgDbObjectIdCR v)
    {
    uint64_t    handle = v.ToUInt64 ();
    m_hasher.Add (&handle, sizeof(handle));
    return DwgDbStatus::Success;
    }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::_Write (DxfGroupCode code, double x, double y, double z, DoublePrecision prec)
    {
    m_hasher.Add (&x, sizeof(x));
    m_hasher.Add (&y, sizeof(y));
    m_hasher.Add (&z, sizeof(z));
    return DwgDbStatus::Success;
    }
DwgDbStatus DwgSyncInfo::DwgObjectHash::HashFiler::Add (void const* buf, size_t nBytes)
    {
    m_hasher.Add (buf, nBytes);
    return DwgDbStatus::Success;    
    }

DgnElementId                    DwgSyncInfo::ElementIterator::Entry::GetElementId() const {return m_sql->GetValueId<DgnElementId>(0);}
DwgSyncInfo::DwgFileId          DwgSyncInfo::ElementIterator::Entry::GetDwgFileId() const {return DwgFileId(m_sql->GetValueInt(1));}
DwgSyncInfo::DwgModelSyncInfoId DwgSyncInfo::ElementIterator::Entry::GetDwgModelSyncInfoId() const {return DwgModelSyncInfoId(m_sql->GetValueInt(2));}
uint64_t                        DwgSyncInfo::ElementIterator::Entry::GetDwgObjectId() const {return m_sql->GetValueInt64(3);}
DwgSyncInfo::DwgObjectProvenance DwgSyncInfo::ElementIterator::Entry::GetProvenance() const {return DwgSyncInfo::DwgObjectProvenance(m_sql);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgObjectMapping   DwgSyncInfo::ElementIterator::Entry::GetObjectMapping () const
    {
    DwgObjectMapping omap(GetElementId(), GetDwgObjectId(), GetDwgModelSyncInfoId(), GetProvenance());
    return  omap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::ElementIterator::ElementIterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);
    Utf8String sqlString = MakeSqlString("SELECT ElementId,DwgFileId,DwgModelSyncInfoId,DwgObjectId,PrimaryHash,SecondaryHash FROM " SYNCINFO_ATTACH(SYNC_TABLE_Element));
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSyncInfo::V8ElementExternalSourceAspectIteratorByChecksum::Bind (DwgSyncInfo::DwgModelSyncInfoId const& modelSyncId, DwgSyncInfo::DwgObjectHash hash1, DwgSyncInfo::DwgObjectHash hash2)
    {
    m_stmt->Reset(); 
    m_stmt->BindInt (1, modelSyncId.GetValue()); 
    m_stmt->BindBlob (2, hash1.m_buffer, sizeof(hash1), BeSQLite::Statement::MakeCopy::No);
    if (hash2.IsNull())
        m_stmt->BindNull (3);
    else
        m_stmt->BindBlob (3, hash2.m_buffer, sizeof(hash2), BeSQLite::Statement::MakeCopy::No);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::ElementIterator::Entry DwgSyncInfo::ElementIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::Layer::Insert(Db& db) const
    {
    if (!m_id.IsValid())
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    Statement stmt;
    stmt.Prepare(db, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Layer) " (Id,DwgFile,DwgModel,DwgObjectId,DwgName) VALUES (?,?,?,?,?)");
    int col = 1;
    stmt.BindId(col++, m_id);       // NativeId
    stmt.BindInt(col++, m_fm.GetDwgFileId().GetValue());
    stmt.BindInt64(col++, m_fm.GetDwgModelId().GetValue());
    stmt.BindInt64(col++, m_dwgId); // DWG layer object handle
    stmt.BindText(col++, m_dwgName.c_str(), Statement::MakeCopy::No); // DWG layer Name

    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::LayerIterator::LayerIterator (DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere (where);
    auto sql = this->MakeSqlString ("SELECT Id,DwgFile,DwgModel,DwgObjectId,DwgName FROM " SYNCINFO_ATTACH(SYNC_TABLE_Layer));
    m_db->GetCachedStatement (m_stmt, sql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::Layer DwgSyncInfo::LayerIterator::LayerIterator::Entry::Get ()
    {
    DwgSyncInfo::Layer  layer;
    int col = 0;
    layer.m_id = m_sql->GetValueId <DgnSubCategoryId> (col++);
    layer.m_fm = DwgSyncInfo::DwgModelSource (DwgSyncInfo::DwgFileId(m_sql->GetValueInt(col++)), DwgSyncInfo::DwgModelId(m_sql->GetValueInt64(col++)));
    layer.m_dwgId = m_sql->GetValueInt64 (col++);
    layer.m_dwgName = m_sql->GetValueText (col++);
    return layer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::LayerIterator::Entry DwgSyncInfo::LayerIterator::begin () const
    {
    m_stmt->Reset ();
    return Entry (m_stmt.get(), m_stmt->Step() == BE_SQLITE_ROW);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::FindFirstSubCategory(DgnSubCategoryId& glid, DwgModelSource fm, uint64_t flid)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "SELECT Id FROM " SYNCINFO_ATTACH(SYNC_TABLE_Layer) " WHERE DwgFile=? AND DwgModel=? AND DwgObjectId=?");

    int col = 1;
    stmt->BindInt(col++, fm.GetDwgFileId().GetValue());
    stmt->BindInt64(col++, fm.GetDwgModelId().GetValue());
    stmt->BindInt64(col++, flid);

    if (stmt->Step() != BE_SQLITE_ROW)
        return BSIERROR;

    glid = stmt->GetValueId<DgnSubCategoryId>(0);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::Layer DwgSyncInfo::InsertLayer(DgnSubCategoryId gcategoryId, DwgModelSource fm, DwgDbLayerTableRecordCR layer)
    {
    uint64_t    id = layer.GetObjectId().ToUInt64 ();
    DwgString   name = layer.GetName ();
    Layer       layerprov (gcategoryId, fm, id, Utf8String(name.c_str()).c_str());

    if (BE_SQLITE_DONE != layerprov.Insert(*m_dgndb))
        {
        layerprov.m_id = DgnSubCategoryId();
        m_dwgImporter.ReportIssue(DwgImporter::IssueSeverity::Info, IssueCategory::InconsistentData(), Issue::InvalidLayer(),
                                  Utf8PrintfString("%s (%lld)", name.c_str(), id).c_str());
        }

    return layerprov;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DwgSyncInfo::FindSubCategory(DwgDbObjectIdCR lid, DwgFileId fid)
    {
    uint64_t            layerId = lid.ToUInt64 ();
    DwgModelSource      modelSource(fid, DwgModelId());
    DgnSubCategoryId    glid;
    return (FindFirstSubCategory(glid, modelSource, layerId) == BSISUCCESS)? glid: DgnSubCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DwgSyncInfo::FindCategory(DwgDbObjectIdCR layerId, DwgFileId fid)
    {
    DgnSubCategoryId    subcatid = FindSubCategory (layerId, fid);
    return DgnSubCategory::QueryCategoryId(*GetDgnDb(), subcatid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DwgSyncInfo::FindSubCategory(DwgDbObjectIdCR layerId, DwgModelSource fm)
    {
    DgnSubCategoryId glid;
    return (FindFirstSubCategory(glid, fm, layerId.ToUInt64()) == BSISUCCESS)? glid: DgnSubCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DwgSyncInfo::GetSubCategory(DwgDbObjectIdCR layerId, DwgModelSource fm)
    {
    uint64_t            lid = layerId.ToUInt64 ();
    DgnSubCategoryId    glid;
    if (FindFirstSubCategory(glid, fm, lid) != BSISUCCESS &&
        FindFirstSubCategory(glid, DwgModelSource(fm.GetDwgFileId()), lid) != BSISUCCESS)
        {
        return DgnCategory::GetDefaultSubCategoryId(GetDwgImporter().GetUncategorizedCategory()); // unable to categorize
        }

    return glid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbHandle     DwgSyncInfo::FindLayerHandle (DgnCategoryId categoryId, DwgFileId fid)
    {
    // reverse look up for DWG layer
    DgnSubCategoryId    subcategoryId = DgnCategory::GetDefaultSubCategoryId (categoryId);
    CachedStatementPtr  stmt;
    m_dgndb->GetCachedStatement (stmt, "SELECT DwgObjectId FROM " SYNCINFO_ATTACH(SYNC_TABLE_Layer) " WHERE Id=? AND DwgFile=?");

    stmt->BindId (1, subcategoryId);
    stmt->BindInt (2, fid.GetValue());

    if (stmt->Step() != BE_SQLITE_ROW)
        return DwgDbHandle();

    uint64_t    handleValue = stmt->GetValueInt64 (0);
    return  DwgDbHandle(handleValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId   DwgSyncInfo::GetCategory (DwgDbEntityCR ent)
    {
    // try categorizing the entity from the layer
    DwgDbObjectId   layerId = ent.GetLayerId ();
    DwgDbDatabaseP  dwg = ent.GetDatabase().get ();

    return  this->GetCategory (layerId, dwg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId   DwgSyncInfo::GetCategory (DwgDbObjectIdCR layerId, DwgDbDatabaseP dwg)
    {
    if (layerId.IsValid() && nullptr != dwg)
        return this->FindCategory (layerId, DwgSyncInfo::GetDwgFileId(*dwg));
        
     // unable to categorize the entity
    return GetDwgImporter().GetUncategorizedCategory();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId    DwgSyncInfo::GetSubCategory (DwgDbObjectIdCR layerId, DwgDbDatabaseP dwg)
    {
    if (layerId.IsValid() && nullptr != dwg)
        return this->FindSubCategory (layerId, DwgSyncInfo::GetDwgFileId(*dwg));
        
    return DgnCategory::GetDefaultSubCategoryId(GetDwgImporter().GetUncategorizedCategory());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyleId      DwgSyncInfo::FindLineStyle (DwgDbObjectIdCR linetypeId)
    {
    // Linetype in the master file and none-model:
    DwgFileId       fileId = DwgFileId::GetFrom (m_dwgImporter.GetDwgDb());
    DwgModelSource  modelSource (fileId);
    return this->FindLineStyle (linetypeId, modelSource);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyleId      DwgSyncInfo::FindLineStyle (DwgDbObjectIdCR linetypeId, DwgModelSource const& modelSource)
    {
    CachedStatementPtr  stmt;
    m_dgndb->GetCachedStatement(stmt, "SELECT Id FROM " SYNCINFO_ATTACH(SYNC_TABLE_Linetype) " WHERE DwgFile=? AND DwgModel=? AND DwgObjectId=?");

    uint64_t    dwgId = linetypeId.ToUInt64 ();
    int         col = 1;
    stmt->BindInt (col++, modelSource.GetDwgFileId().GetValue());
    stmt->BindInt64 (col++, modelSource.GetDwgModelId().GetValue());
    stmt->BindInt64 (col++, dwgId);

    DgnStyleId  dgnId;
    if (stmt->Step() == BE_SQLITE_ROW)
        dgnId = stmt->GetValueId<DgnStyleId>(0);

    return dgnId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::Linetype   DwgSyncInfo::InsertLinetype (DgnStyleId lstyleId, DwgDbLinetypeTableRecordCR ltype)
    {
    // Linetype in the master file and none model:
    DwgFileId       fileId = DwgFileId::GetFrom (m_dwgImporter.GetDwgDb());
    DwgModelSource  modelSource (fileId);
    return this->InsertLinetype (lstyleId, modelSource, ltype);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::Linetype   DwgSyncInfo::InsertLinetype (DgnStyleId lstyleId, DwgModelSource const& fm, DwgDbLinetypeTableRecordCR ltype)
    {
    uint64_t    id = ltype.GetObjectId().ToUInt64 ();
    Utf8String  name (ltype.GetName().c_str());
    Linetype    linetypeProv (lstyleId, fm, id, name.c_str());

    if (BE_SQLITE_DONE != linetypeProv.Insert(*m_dgndb))
        {
        linetypeProv.m_id = DgnStyleId ();
        m_dwgImporter.ReportIssue(DwgImporter::IssueSeverity::Info, IssueCategory::InconsistentData(), Issue::LinetypeError(),
                                  Utf8PrintfString("%s (%lld)", name.c_str(), id).c_str());
        }

    return linetypeProv;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::Linetype::Insert (BeSQLite::Db& db) const
    {
    if (!m_id.IsValid() || 0 == m_dwgId)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    Statement   stmt;
    stmt.Prepare (db, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Linetype) " (Id,DwgFile,DwgModel,DwgObjectId,DwgName) VALUES (?,?,?,?,?)");

    int col = 1;
    stmt.BindId (col++, m_id);       // NativeId
    stmt.BindInt (col++, m_fm.GetDwgFileId().GetValue());
    stmt.BindInt64 (col++, m_fm.GetDwgModelId().GetValue());
    stmt.BindInt64 (col++, m_dwgId); // DWG linetype object handle
    stmt.BindText (col++, m_name.c_str(), Statement::MakeCopy::No); // Linestyle name possibly with a scale suffix

    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::Linetype::Update (BeSQLite::Db& db) const
    {
    // NEEDSWORK - the column changed here does not seem to get written into the .syncinfo file!
    if (!m_id.IsValid() || 0 == m_dwgId)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    Statement   stmt;
    stmt.Prepare (db, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_Linetype) " SET DwgName=? WHERE Id=?");

    int col = 1;
    stmt.BindText (col++, m_name.c_str(), Statement::MakeCopy::No);
    stmt.BindId (col++, m_id);

    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::UpdateLinetype (DgnStyleId lstyleId, DwgDbLinetypeTableRecordCR ltype)
    {
    uint64_t        ltypeId = ltype.GetObjectId().ToUInt64 ();
    Utf8String      name (ltype.GetName().c_str());
    DwgFileId       fileId = DwgFileId::GetFrom (m_dwgImporter.GetDwgDb());
    DwgModelSource  modelSource (fileId);
    Linetype        ltypeProv (lstyleId, modelSource, ltypeId, name.c_str());

    return (ltypeProv.Update(*m_dgndb) == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::View DwgSyncInfo::InsertView (DgnViewId viewId, DwgDbObjectIdCR objId, View::Type type, Utf8StringCR name)
    {
    uint64_t    vportId = objId.ToUInt64 ();
    View        vportProv (viewId, vportId, type, name);

    if (BE_SQLITE_DONE != vportProv.Insert(*m_dgndb))
        {
        vportProv.m_id = DgnViewId ();
        m_dwgImporter.ReportIssueV(DwgImporter::IssueSeverity::Error, IssueCategory::Sync(), Issue::ViewportError(), Utf8PrintfString("inserting %s (%lld)", name.c_str(), vportId).c_str());
        }

    return vportProv;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::View::Insert (BeSQLite::Db& db) const
    {
    if (!m_id.IsValid() || 0 == m_dwgId)
        {
        BeAssert(false && "Null DwgObjectId!");
        return BE_SQLITE_ERROR;
        }

    Statement   stmt;
    stmt.Prepare (db, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_View) " (Id,DwgObjectId,ViewportType,DwgName) VALUES (?,?,?,?)");

    int col = 1;
    stmt.BindId (col++, m_id);       // NativeId
    stmt.BindInt64 (col++, m_dwgId); // DWG vport object handle
    stmt.BindInt (col++, static_cast<int>(m_type)); // Viewport type
    stmt.BindText (col++, m_name.c_str(), Statement::MakeCopy::No); // VPort name

    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::View::Update (BeSQLite::Db& db) const
    {
    if (!m_id.IsValid() || 0 == m_dwgId)
        {
        BeAssert(false && "Null DwgObjectId!");
        return BE_SQLITE_ERROR;
        }

    Statement   stmt;
    stmt.Prepare (db, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_View) " SET DwgObjectId=?,ViewportType=?,DwgName=? WHERE Id=?");

    int col = 1;
    stmt.BindInt64 (col++, m_dwgId);
    stmt.BindInt (col++, static_cast<int>(m_type));
    stmt.BindText (col++, m_name.c_str(), Statement::MakeCopy::No);
    stmt.BindId (col++, m_id);

    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::ViewIterator::ViewIterator (DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere (where);
    auto sql = this->MakeSqlString ("SELECT Id,DwgObjectId,ViewportType,DwgName FROM " SYNCINFO_ATTACH(SYNC_TABLE_View));
    m_db->GetCachedStatement (m_stmt, sql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::View DwgSyncInfo::ViewIterator::ViewIterator::Entry::Get ()
    {
    DwgSyncInfo::View  view;
    int col = 0;
    view.m_id = m_sql->GetValueId <DgnViewId> (col++);
    view.m_dwgId = m_sql->GetValueInt64 (col++);
    view.m_type = static_cast<DwgSyncInfo::View::Type> (m_sql->GetValueInt(col++));
    view.m_name = m_sql->GetValueText (col++);
    return view;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::ViewIterator::Entry DwgSyncInfo::ViewIterator::begin () const
    {
    m_stmt->Reset ();
    return Entry (m_stmt.get(), m_stmt->Step() == BE_SQLITE_ROW);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::UpdateView (DgnViewId viewId, DwgDbObjectIdCR objId, View::Type type, Utf8StringCR name)
    {
    uint64_t    vportId = objId.ToUInt64 ();
    View        vportProv (viewId, vportId, type, name);

    return vportProv.Update(*m_dgndb) == BE_SQLITE_DONE ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId   DwgSyncInfo::FindView (DwgDbObjectIdCR vportId, View::Type type)
    {
    CachedStatementPtr  stmt;
    m_dgndb->GetCachedStatement(stmt, "SELECT Id FROM " SYNCINFO_ATTACH(SYNC_TABLE_View) " WHERE DwgObjectId=? AND ViewportType=?");

    uint64_t    dwgId = vportId.ToUInt64 ();
    int         col = 1;
    stmt->BindInt64 (col++, dwgId);
    stmt->BindInt (col++, static_cast<int>(type));

    DgnViewId  dgnId;
    if (stmt->Step() == BE_SQLITE_ROW)
        dgnId = stmt->GetValueId<DgnViewId>(0);

    return dgnId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbHandle DwgSyncInfo::FindViewportHandle (DgnViewId viewId)
    {
    CachedStatementPtr  stmt;
    m_dgndb->GetCachedStatement(stmt, "SELECT DwgObjectId FROM " SYNCINFO_ATTACH(SYNC_TABLE_View) " WHERE Id=?");
    stmt->BindId (1, viewId);

    if (stmt->Step() == BE_SQLITE_ROW)
        return DwgDbHandle(stmt->GetValueInt64(0));

    return DwgDbHandle();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::DeleteView (DgnViewId viewId)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_View) " WHERE Id=?");
    stmt->BindId (1, viewId);
    return (stmt->Step() == BE_SQLITE_DONE)? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSyncInfo::FindMaterial (DwgSyncInfo::Material& out, DwgDbObjectIdCR materialId)
    {
    CachedStatementPtr  stmt;
    m_dgndb->GetCachedStatement(stmt, "SELECT Id,DwgName,ObjectHash FROM " SYNCINFO_ATTACH(SYNC_TABLE_Material) " WHERE DwgFileId=? AND DwgObjectId=?");

    DwgDbDatabaseP  dwg = materialId.GetDatabase ();
    DwgFileId   fileId = DwgFileId::GetFrom (nullptr == dwg ? m_dwgImporter.GetDwgDb() : *dwg);
    uint64_t    dwgId = materialId.ToUInt64 ();
    int col = 1;
    stmt->BindInt (col++, fileId.GetValue());
    stmt->BindInt64 (col++, dwgId);

    if (stmt->Step() != BE_SQLITE_ROW)
        return  false;

    out.m_id = stmt->GetValueId<RenderMaterialId>(0);
    out.m_fileId = fileId;
    out.m_objectId = dwgId;
    out.m_name.AssignOrClear (stmt->GetValueText(1));
    if (!stmt->IsColumnNull(2))
        memcpy (&out.m_hash, stmt->GetValueBlob(2), sizeof(out.m_hash));
    else
        memset (&out.m_hash, 0, sizeof(out.m_hash));

    return out.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::Material   DwgSyncInfo::InsertMaterial (RenderMaterialId id, DwgDbMaterialCR material)
    {
    DwgDbDatabasePtr    dwg = material.GetDatabase ();
    DwgFileId   fileId = DwgFileId::GetFrom (dwg.IsNull() ? m_dwgImporter.GetDwgDb() : *dwg);
    Material    prov (id, fileId, m_dwgImporter.GetCurrentIdPolicy(), material);

    if (BE_SQLITE_DONE != prov.Insert(*m_dgndb))
        {
        prov.m_id = RenderMaterialId ();
        m_dwgImporter.ReportSyncInfoIssue(DwgImporter::IssueSeverity::Info, IssueCategory::Sync(), Issue::MaterialError(),
                                  Utf8PrintfString("%s (%lld)", prov.m_name.c_str(), id.GetValue()).c_str());
        }

    return prov;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::Material::Insert (BeSQLite::Db& db) const
    {
    if (!m_id.IsValid() || 0 == m_objectId)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    Statement stmt;
    stmt.Prepare (db, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Material) " (Id,DwgFileId,DwgObjectId,DwgName,ObjectHash) VALUES (?,?,?,?,?)");

    int col = 1;
    stmt.BindId (col++, m_id);  // NativeId
    stmt.BindInt (col++, m_fileId.GetValue());   // DWG file ID
    if (m_idPolicy==StableIdPolicy::ById)
        stmt.BindInt64 (col++, m_objectId); // DWG material object handle
    else
        stmt.BindNull (col++);
    stmt.BindText (col++, m_name.c_str(), Statement::MakeCopy::No); // DWG material name
    stmt.BindBlob (col++, &m_hash, sizeof(m_hash), Statement::MakeCopy::No);

    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::Material::Material (RenderMaterialId id, DwgFileId fid, StableIdPolicy policy, DwgDbMaterialCR material)
    {
    m_id = id;
    m_fileId = fid;
    m_idPolicy = policy;
    m_objectId = material.GetObjectId().ToUInt64 ();
    m_name.Assign (material.GetName().c_str());

    memset (&m_hash, 0, sizeof(m_hash));
    m_hasher.Reset ();

    DwgObjectHash::HashFiler filer(m_hasher, *DwgDbObject::Cast(&material));
    if (filer.IsValid())
        {
        material.DxfOut (filer);
        m_hash = m_hasher.GetHashVal ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::Material::Update (BeSQLite::Db& db) const
    {
    if (!m_id.IsValid() || 0 == m_objectId)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    Statement   stmt;
    stmt.Prepare (db, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_Material) " SET DwgName=?,ObjectHash=? WHERE Id=?");

    int col = 1;
    stmt.BindText (col++, m_name.c_str(), Statement::MakeCopy::No);
    stmt.BindBlob (col++, &m_hash, sizeof(m_hash), Statement::MakeCopy::No);
    stmt.BindId (col++, m_id);

    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::UpdateMaterial (DwgSyncInfo::Material& prov)
    {
    return (prov.Update(*m_dgndb) == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::DeleteMaterial (RenderMaterialId id)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_Material) " WHERE Id=?");
    stmt->BindId (1, id);
    return (stmt->Step() == BE_SQLITE_DONE)? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::MaterialIterator::MaterialIterator(DgnDbCR db) : BeSQLite::DbTableIterator(db)
    {
    Utf8String sqlString = MakeSqlString ("SELECT Id,DwgFileId,DwgObjectId FROM " SYNCINFO_ATTACH(SYNC_TABLE_Material));
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::MaterialIterator::Entry DwgSyncInfo::MaterialIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

RenderMaterialId DwgSyncInfo::MaterialIterator::Entry::GetRenderMaterialId() { return m_sql->GetValueId<RenderMaterialId>(0); }
DwgSyncInfo::DwgFileId DwgSyncInfo::MaterialIterator::Entry::GetDwgFileId() { return DwgFileId(m_sql->GetValueInt(1)); }
uint64_t DwgSyncInfo::MaterialIterator::Entry::GetDwgObjectId() { return m_sql->GetValueInt64(2); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSyncInfo::FindGroup (Group& out, DwgDbObjectIdCR groupId)
    {
    // find group by ID (expecting ID policy to be ById).
    CachedStatementPtr  stmt;
    m_dgndb->GetCachedStatement(stmt, "SELECT Id,DwgName,ObjectHash FROM " SYNCINFO_ATTACH(SYNC_TABLE_Group) " WHERE DwgFileId=? AND DwgObjectId=?");

    auto dwg = groupId.GetDatabase ();
    auto fileId = DwgFileId::GetFrom (nullptr == dwg ? m_dwgImporter.GetDwgDb() : *dwg);
    auto dwgId = groupId.ToUInt64 ();
    int col = 1;
    stmt->BindInt (col++, fileId.GetValue());
    stmt->BindInt64 (col++, dwgId);
    if (stmt->Step() != BE_SQLITE_ROW)
        return  false;

    out.m_id = stmt->GetValueId<DgnElementId>(0);
    out.m_fileId = fileId;
    out.m_objectId = dwgId;
    out.m_idPolicy = StableIdPolicy::ById;
    out.m_name.AssignOrClear (stmt->GetValueText(1));
    memcpy (&out.m_hash, stmt->GetValueBlob(2), sizeof(out.m_hash));

    return out.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::Group   DwgSyncInfo::InsertGroup (DgnElementId id, DwgDbGroupCR group)
    {
    auto dwg = group.GetDatabase ();
    auto fileId = DwgFileId::GetFrom (dwg.IsNull() ? m_dwgImporter.GetDwgDb() : *dwg);
    auto addMembers = m_dwgImporter._ShouldSyncGroupWithMembers ();

    Group   prov (id, fileId, m_dwgImporter.GetCurrentIdPolicy(), group, addMembers);

    if (BE_SQLITE_DONE != prov.Insert(*m_dgndb))
        {
        prov.m_id = DgnElementId ();
        m_dwgImporter.ReportSyncInfoIssue(DwgImporter::IssueSeverity::Info, IssueCategory::Sync(), Issue::GroupError(),
                                  Utf8PrintfString("%s (%lld)", prov.m_name.c_str(), id.GetValue()).c_str());
        }

    return prov;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::Group::Insert (BeSQLite::Db& db) const
    {
    if (!m_id.IsValid() || 0 == m_objectId)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    Statement stmt;
    stmt.Prepare (db, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Group) " (Id,DwgFileId,DwgObjectId,DwgName,ObjectHash) VALUES (?,?,?,?,?)");

    int col = 1;
    stmt.BindId (col++, m_id);  // NativeId
    stmt.BindInt (col++, m_fileId.GetValue());   // DWG file ID
    if (m_idPolicy==StableIdPolicy::ById)
        stmt.BindInt64 (col++, m_objectId); // DWG group object handle
    else
        stmt.BindNull (col++);
    stmt.BindText (col++, m_name.c_str(), Statement::MakeCopy::No); // DWG group name
    stmt.BindBlob (col++, &m_hash, sizeof(m_hash), Statement::MakeCopy::No);  // DWG group (optionally members included) hash

    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::Group::Group (DgnElementId id, DwgFileId fid, StableIdPolicy policy, DwgDbGroupCR group, bool addMembers)
    {
    m_id = id;
    m_fileId = fid;
    m_idPolicy = policy;
    m_objectId = group.GetObjectId().ToUInt64 ();
    m_name.Assign (group.GetName().c_str());

    ::memset (&m_hash, 0, sizeof(m_hash));
    m_hasher.Reset ();

    DwgObjectHash::HashFiler filer(m_hasher, *DwgDbObject::Cast(&group));
    if (filer.IsValid())
        {
        group.DxfOut (filer);
        m_hasher.Add (m_name.c_str(), m_name.size());
        }

    DwgDbObjectIdArray  memberIds;
    if (addMembers && group.GetAllEntityIds(memberIds) > 0)
        {
        for (auto memberId : memberIds)
            {
            DwgDbObjectPtr  obj(memberId, DwgDbOpenMode::ForRead);
            if (obj.OpenStatus() == DwgDbStatus::Success)
                {
                DwgObjectHash::HashFiler memberFiler(m_hasher, *obj);
                if (memberFiler.IsValid())
                    obj->DxfOut (memberFiler);
                }
            }
        }

    m_hash = m_hasher.GetHashVal ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DwgSyncInfo::Group::Update (BeSQLite::Db& db) const
    {
    if (!m_id.IsValid() || 0 == m_objectId)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    Statement   stmt;
    stmt.Prepare (db, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_Group) " SET DwgName=?,ObjectHash=? WHERE Id=?");

    int col = 1;
    stmt.BindText (col++, m_name.c_str(), Statement::MakeCopy::No);
    stmt.BindBlob (col++, &m_hash, sizeof(m_hash), Statement::MakeCopy::No);
    stmt.BindId (col++, m_id);

    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::UpdateGroup (DwgSyncInfo::Group& prov)
    {
    return (prov.Update(*m_dgndb) == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::DeleteGroup (DgnElementId id)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_Group) " WHERE Id=?");
    stmt->BindId (1, id);
    return (stmt->Step() == BE_SQLITE_DONE)? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::GroupIterator::GroupIterator(DgnDbCR db) : BeSQLite::DbTableIterator(db)
    {
    Utf8String sqlString = MakeSqlString ("SELECT Id,DwgFileId,DwgObjectId FROM " SYNCINFO_ATTACH(SYNC_TABLE_Group));
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::GroupIterator::Entry DwgSyncInfo::GroupIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnElementId DwgSyncInfo::GroupIterator::Entry::GetDgnElementId() { return m_sql->GetValueId<DgnElementId>(0); }
DwgSyncInfo::DwgFileId DwgSyncInfo::GroupIterator::Entry::GetDwgFileId() { return DwgFileId(m_sql->GetValueInt(1)); }
uint64_t DwgSyncInfo::GroupIterator::Entry::GetDwgObjectId() { return m_sql->GetValueInt64(2); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSyncInfo::WasDwgObjectDiscarded (DwgDbObjectIdCR vid, DwgModelSyncInfoId const& modelSyncId)
    {
    CachedStatementPtr stmt; 
    m_dgndb->GetCachedStatement(stmt, "SELECT COUNT(*) FROM " SYNCINFO_ATTACH(SYNC_TABLE_Discards) " WHERE DwgModelSyncInfoId=? AND DwgObjectId=?");
    stmt->Reset();
    stmt->ClearBindings();
    stmt->BindInt64 (1, modelSyncId.GetValue());
    stmt->BindInt64 (2, vid.ToUInt64());
    return stmt->Step() == BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSyncInfo::InsertDiscardedDwgObject (DwgDbEntityCR ent, DwgModelSyncInfoId const& modelSyncId)
    {
    CachedStatementPtr stmt; 
    m_dgndb->GetCachedStatement(stmt, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Discards) "(DwgModelSyncInfoId,DwgObjectId) VALUES (?,?)");
    int col=1;
    stmt->BindInt64 (col++, modelSyncId.GetValue());
    stmt->BindInt64 (col++, ent.GetObjectId().ToUInt64());
    stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::DeleteDiscardedDwgObject (DwgDbObjectIdCR dwgid, DwgModelSyncInfoId const& modelSyncId)
    {
    CachedStatementPtr stmt; 
    m_dgndb->GetCachedStatement(stmt, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_Discards) " WHERE DwgModelSyncInfoId=? AND DwgObjectId=?");
    int col=1;
    stmt->BindInt64 (col++, modelSyncId.GetValue());
    stmt->BindInt64 (col++, dwgid.ToUInt64());
    return stmt->Step() == BE_SQLITE_DONE? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DwgSyncInfo::GeomPart::Insert (BeSQLite::Db& db) const
    {
    Statement stmt;
    stmt.Prepare(db, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_GeometryPart) "(PartId,PartTag) VALUES (?,?)");
    int col = 1;
    stmt.BindId (col++, m_id);
    stmt.BindText (col++, m_tag, Statement::MakeCopy::No);
    return stmt.Step ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  DwgSyncInfo::GeomPart::GetSelectSql ()
    {
    return "SELECT PartId,PartTag FROM " SYNCINFO_ATTACH(SYNC_TABLE_GeometryPart);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgSyncInfo::GeomPart::FromSelect (BeSQLite::Statement& selected)
    {
    int col = 0;
    m_id = selected.GetValueId <DgnGeometryPartId> (col++);
    m_tag = selected.GetValueText (col++);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::GeomPartIterator::GeomPartIterator (DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere (where);
    auto sql = this->MakeSqlString (GeomPart::GetSelectSql().c_str());
    m_db->GetCachedStatement (m_stmt, sql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::GeomPart DwgSyncInfo::GeomPartIterator::GeomPartIterator::Entry::Get ()
    {
    DwgSyncInfo::GeomPart   part;
    part.FromSelect (*m_sql);
    return part;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::GeomPartIterator::Entry DwgSyncInfo::GeomPartIterator::begin () const
    {
    m_stmt->Reset ();
    return Entry (m_stmt.get(), m_stmt->Step() == BE_SQLITE_ROW);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::GeomPart::FindById (GeomPart& part, DgnDbCR db, DgnGeometryPartId id)
    {
    if (db.TableExists(SYNCINFO_ATTACH(SYNC_TABLE_GeometryPart)))
        {
        GeomPartIterator iter (db, "PartId=?");
        iter.GetStatement()->BindId (1, id);

        auto found = iter.begin ();
        if (found != iter.end())
            {
            part.FromSelect (*iter.GetStatement());
            return  BSISUCCESS;
            }
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSyncInfo::GeomPart::FindByTag (GeomPart& part, DgnDbCR db, Utf8CP tag)
    {
    if (!Utf8String::IsNullOrEmpty(tag) && db.TableExists(SYNCINFO_ATTACH(SYNC_TABLE_GeometryPart)))
        {
        GeomPartIterator iter (db, "PartTag=?");
        iter.GetStatement()->BindText (1, tag, Statement::MakeCopy::No);

        auto found = iter.begin ();
        if (found != iter.end())
            {
            part.FromSelect (*iter.GetStatement());
            return BSISUCCESS;
            }
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DwgSyncInfo::GetDbFileName(DgnDb& project)
    {
    BeFileName name(Utf8String(project.GetDbFileName()));
    name.append(L".syncinfo");
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DwgSyncInfo::GetDbFileName (BeFileNameCR bimFileName)
    {
    BeFileName syncFileName(bimFileName);
    syncFileName.append (L".syncinfo");
    return syncFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::AttachToProject(DgnDb& targetProject, BeFileNameCR dbName)
    {
    DbResult rc = targetProject.AttachDb(Utf8String(dbName).c_str(), SYNCINFO_ATTACH_ALIAS);
    if (BE_SQLITE_OK != rc)
        return BSIERROR;
    return OnAttach(targetProject);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::OnAttach(DgnDb& project)
    {
    m_dgndb = &project;

    if (!m_dgndb->TableExists(SYNCINFO_ATTACH(SYNC_TABLE_File)))
        {
        // We are creating a new syncinfo file
        Utf8String currentDbProfileVersion;
        m_dgndb->QueryProperty(currentDbProfileVersion, Properties::ProfileVersion());

        MUSTBEOK(SavePropertyString(SyncInfoProperty::ProfileVersion(), s_currentVersion.ToJson().c_str()));
        MUSTBEOK(SavePropertyString(Properties::CreationDate(), DateTime::GetCurrentTimeUtc().ToString().c_str()));
        MUSTBEOK(SavePropertyString(SyncInfoProperty::DgnDbGuid(), m_dgndb->GetDbGuid().ToString().c_str()));
        MUSTBEOK(SavePropertyString(SyncInfoProperty::DbProfileVersion(), currentDbProfileVersion.c_str()));
        MUSTBEOK(SavePropertyString(SyncInfoProperty::DgnDbProfileVersion(), m_dgndb->GetProfileVersion().ToJson().c_str()));
        // *** WIP_CONVERTER - I'd like to save project's last save time

        CreateTables();
        SetValid(true);
        return BSISUCCESS;
        }

    //  We are opening an existing syncinfo file
    BentleyStatus   status = PerformVersionChecks ();
    if (status != BSISUCCESS)
        return status;

    //  Check that this syncinfo goes with this project
    Utf8String projguidstr;
    BeSQLite::BeGuid projguid;
    if (QueryProperty(projguidstr, SyncInfoProperty::DgnDbGuid()) != BE_SQLITE_ROW
        || projguid.FromString(projguidstr.c_str()) != BSISUCCESS
        || m_dgndb->GetDbGuid() != projguid)
        {
        LOG.errorv("GUID mismatch. syncinfo=%s projectguid=%s does not match project guid=%s",
                                m_dgndb->GetDbFileName(), projguidstr.c_str(), m_dgndb->GetDbGuid().ToString().c_str());
        return BSIERROR;
        }

    Utf8String savedProjectDbProfileVersion, currentProjectDbProfileVersion;
    if (QueryProperty(savedProjectDbProfileVersion, SyncInfoProperty::DbProfileVersion()) != BE_SQLITE_ROW
        || m_dgndb->QueryProperty(currentProjectDbProfileVersion, Properties::ProfileVersion()) !=  BE_SQLITE_ROW
        || !savedProjectDbProfileVersion.Equals(currentProjectDbProfileVersion))
        {
        LOG.warningv("DB schema version mismatch. syncinfo=%s ProjectDbProfileVersion=%s does not match project ProfileVersion=%s.",
                                m_dgndb->GetDbFileName(), savedProjectDbProfileVersion.c_str(), currentProjectDbProfileVersion.c_str());
        // *** WIP_CONVERTER - Do we really have to throw away project history whenever we make a trivial schema change?
        return BSISUCCESS;//BSIERROR; *** WIP_CONVERTER - support schema evolution 
        }

    Utf8String currentProjectProfileVersion = m_dgndb->GetProfileVersion().ToJson();
    Utf8String savedProjectProfileVersion;
    if (QueryProperty(savedProjectProfileVersion, SyncInfoProperty::DgnDbProfileVersion()) != BE_SQLITE_ROW
        || !savedProjectProfileVersion.Equals(currentProjectProfileVersion))
        {
        LOG.warningv("project schema version mismatch. syncinfo=%s ProjectProfileVersion=%s does not match project ProjectProfileVersion=%s.",
                                m_dgndb->GetDbFileName(), savedProjectProfileVersion.c_str(), currentProjectProfileVersion.c_str());
        // *** WIP_CONVERTER - Do we really have to throw away project history whenever we make a trivial schema change?
        return BSISUCCESS;//BSIERROR; *** WIP_CONVERTER - support schema evolution 
        }

    SetValid(true);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSyncInfo::CreateEmptyFile(BeFileNameCR fileName, bool deleteIfExists)
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
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSyncInfo::SetLastError(BeSQLite::DbResult rc)
    {
    m_lastError = rc;
    m_lastErrorDescription = m_dgndb->GetLastError();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSyncInfo::GetLastError(BeSQLite::DbResult& result, Utf8String& descr)
    {
    result = m_lastError;
    descr = m_lastErrorDescription;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      04/15
//---------------------------------------------------------------------------------------
bool DwgSyncInfo::DwgModelSource::operator<(DwgModelSource const& rhs) const 
    {
    if (m_fileId < rhs.m_fileId)
        return true;
    if (m_fileId > rhs.m_fileId)
        return false;
    return m_modelId.GetValue() < rhs.m_modelId.GetValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgSyncInfo (DwgImporter& importer) : m_dwgImporter(importer)
    {
    m_dgndb = nullptr;
    m_lastError = BE_SQLITE_ERROR;
    m_lastErrorDescription.clear ();
    m_isValid = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::~DwgSyncInfo ()
    {
    if (m_dgndb != nullptr)
        m_dgndb->DetachDb (SYNCINFO_ATTACH_ALIAS);
    }

END_DWG_NAMESPACE

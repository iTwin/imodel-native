/*--------------------------------------------------------------------------------------+
|
|     $Source: SyncInfo.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include "DgnV8/DynamicSchemaGenerator/ECConversion.h"
#include <GeomJsonWireFormat/JsonUtils.h>

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"DgnV8Converter.SyncInfo"))

#define MUSTBEDBRESULT(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {SetLastError(rc); return BSIERROR;}}
#define MUSTBEOK(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_OK)
#define MUSTBEROW(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_ROW)
#define MUSTBEDONE(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_DONE)

#define MUSTBEDBRESULTRC(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {SetLastError(rc); return rc;}}
#define MUSTBEOKRC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_OK)
#define MUSTBEROWRC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_ROW)
#define MUSTBEDONERC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_DONE)

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

static rapidjson::Value fixedArrayToJson(double const* darray, size_t count, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    rapidjson::Value jdbls;
    jdbls.SetArray();
    jdbls.Reserve(count, allocator);
    for (size_t i=0; i<count; ++i)
        jdbls.PushBack(darray[i], allocator);
    return jdbls;
    }

static void fixedArrayFromJson(double* darray, size_t count, rapidjson::Value const& jdbls)
    {
    if (jdbls.Size() != count)
        {
        BeAssert(false);
        return;
        }
    for (int i=0; i<count; ++i)
        {
        darray[i] = jdbls[i].GetDouble();
        }
    }

static ProfileVersion s_currentVersion(0, 1, 0, 0);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SyncInfo::SavePropertyString(PropertySpecCR spec, Utf8CP stringData, uint64_t id, uint64_t subId)
    {
    Statement stmt;
    auto rc = stmt.Prepare(*m_dgndb, "INSERT OR REPLACE INTO " SYNCINFO_ATTACH(BEDB_TABLE_Property) " (Namespace,Name,Id,SubId,TxnMode,StrData) VALUES(?,?,?,?,?,?)");
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
DbResult SyncInfo::QueryProperty(Utf8StringR value, PropertySpecCR spec, uint64_t id, uint64_t subId) const
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
BentleyStatus SyncInfo::CreateTables()
    {
    if (nullptr == m_dgndb)
        {
        BeAssert(false);
        return BSIERROR;
        }

    if (m_dgndb->TableExists(SYNCINFO_ATTACH(SYNC_TABLE_File)))
        {
        CreateECTables();
        ImportJob::CreateTable(*m_dgndb);
        GeomPart::CreateTable(*m_dgndb);
        m_dgndb->SaveChanges();
        return BSISUCCESS;
        }

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_File),
                         "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "UniqueName CHAR NOT NULL UNIQUE,"
                         "V8Name CHAR NOT NULL,"
                         "LastSaveTime REAL,"
                         "LastModified BIGINT,"
                         "FileSize BIGINT,"
                         "UseHash BOOL");

    // can be N:N
    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Model),
                         "ModelId BIGINT NOT NULL,"
                         "V8FileSyncInfoId INTEGER REFERENCES " SYNC_TABLE_File "(Id) ON DELETE CASCADE,"
                         "V8Id INT,"
                         "V8Name CHAR NOT NULL,"
                         "Transform BLOB,"
                         "CONSTRAINT FileModelId UNIQUE(ModelId,V8FileSyncInfoId,V8Id,Transform)"); // we can map the same v8 model to two different BIM models. 
                                                                                                    // for example, we may import a drawing to its own BIM DrawingModel, and we may 
                                                                                                    // merge that same drawing into the DrawingModel of its V8 parent.

    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Model) "NativeIdx ON "  SYNC_TABLE_Model "(ModelId)");
    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Model) "FileAndModel ON "  SYNC_TABLE_Model "(V8FileSyncInfoId,V8Id)");

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Level),
                         "Id INT,"
                         "Type INT,"
                         "V8FileSyncInfoId INTEGER REFERENCES " SYNC_TABLE_File "(Id) ON DELETE CASCADE,"
                         "V8Model INT,"
                         "V8Id INT,"
                         "V8Name CHAR NOT NULL,"
                         "CONSTRAINT FileModelId UNIQUE(V8FileSyncInfoId,V8Model,V8Id,Type)");

    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Level) "NativeIdx ON "  SYNC_TABLE_Level "(Id)");

    // can be N:N
    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Element),
                         "ElementId BIGINT NOT NULL,"
                         "v8ModelSyncInfoId BIGINT NOT NULL,"
                         "V8ElementId BIGINT,"
                         "LastModified REAL,"
                         "Hash BLOB");

    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Element) "ElementIdx ON " SYNC_TABLE_Element "(ElementId)");
    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Element) "V8Idx ON " SYNC_TABLE_Element "(V8ModelSyncInfoId,V8ElementId)");
    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Element) "HashIdx ON "  SYNC_TABLE_Element "(V8ModelSyncInfoId,Hash) WHERE V8ElementId IS NULL");

    // can be N:1
    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_ExtractedGraphic),
                         "DrawingV8ModelSyncInfoId BIGINT NOT NULL," // A V8 type-100 (DgnAttachment) element - represents the section as a whole
                         "AttachmentV8ElementId BIGINT NOT NULL,"       //              "
                         "OriginalV8ModelSyncInfoId BIGINT NOT NULL,"   // A V8 3D element that was sectioned
                         "OriginalV8ElementId BIGINT NOT NULL,"         //              "
                         "Category BIGINT NOT NULL,"                    // The BIM category of the graphic
                         "Graphic BIGINT NOT NULL,"                     // The BIM DrawingGraphic element that contains all of the section graphics derived from the above element (in this particular attachment's section)
                         "PRIMARY KEY(DrawingV8ModelSyncInfoId,AttachmentV8ElementId,"
                                     "OriginalV8ModelSyncInfoId,OriginalV8ElementId,"
                                     "Category)"
                         );

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Discards), "V8ModelSyncInfoId INT, V8Id BIGINT");

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_ECSchema),
                         "V8Id INTEGER PRIMARY KEY,"
                         "V8FileSyncInfoId INTEGER NOT NULL,"
                         "V8Name TEXT NOT NULL,"
                         "V8VersionMajor INTEGER NOT NULL,"
                         "V8VersionMinor INTEGER NOT NULL,"
                         "MappingType INTEGER NOT NULL,"
                         "LastModified TIMESTAMP,"
                         "Digest INTEGER");

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_View),
                         "ElementId BIGINT NOT NULL, "
                         "V8FileSyncInfoId INTEGER NOT NULL, "
                         "V8ElementId BIGINT, "
                         "V8ViewName TEXT, "
                         "LastModified REAL");

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Imagery),
                         "ElementId BIGINT PRIMARY KEY, "
                         "V8FileSyncInfoId INTEGER REFERENCES " SYNC_TABLE_File "(Id) ON DELETE CASCADE,"
                         "Filename TEXT NOT NULL,"
                         "LastModified BIGINT,"
                         "FileSize BIGINT,"
                         "ETag TEXT,"
                         "RDSId TEXT");
    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Imagery) "ElementIdx ON "  SYNC_TABLE_Imagery "(ElementId)");

    ImportJob::CreateTable(*m_dgndb);
    GeomPart::CreateTable(*m_dgndb);

    //need a unique index to ensure uniqueness for schemas based on checksum
    Utf8String ddl;
    ddl.Sprintf("CREATE UNIQUE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_ECSchema) "_variantxml_uix ON "  SYNC_TABLE_ECSchema "(V8Name, V8FileSyncInfoId, Digest);");
    MUSTBEOK(m_dgndb->ExecuteSql(ddl.c_str()));
    //need a index on the entire table for fast look ups
    MUSTBEOK(m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_ECSchema) "_ix ON "  SYNC_TABLE_ECSchema "(V8Name);"));

    MUSTBEOK(m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Discards) "V8Idx ON " SYNC_TABLE_Discards "(V8ModelSyncInfoId, V8Id)"));

    CreateNamedGroupTable(true);
    CreateECTables();

    m_dgndb->SaveChanges();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::CreateECTables()
    {
    V8ECClassInfo::CreateTable(*m_dgndb);
    ECInstanceInfo::CreateTable(*m_dgndb);
    V8ECSchemaXmlInfo::CreateTable(*m_dgndb);
    V8ElementSecondaryECClassInfo::CreateTable(*m_dgndb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
BentleyStatus SyncInfo::PerformVersionChecks()
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
DbResult SyncInfo::InsertFont(DgnFontId newId, V8FontId oldId)
    {
    m_font[oldId] = newId;

    // WIP_CONVERTER -- write syncinfo 
    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFontId SyncInfo::FindFont(V8FontId oldId)
    {
    // WIP_CONVERTER -- read syncInfo
    auto i = m_font.find(oldId);
    return i == m_font.end() ? DgnFontId() : i->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
RenderMaterialId SyncInfo::FindMaterialByV8Id(uint64_t id, DgnV8FileR v8File, DgnV8ModelR v8Model)
    {
    DgnV8Api::Material const*    material;

    if (NULL == (material = DgnV8Api::MaterialManager::GetManagerR().FindMaterial(NULL, DgnV8Api::MaterialId(id), v8File, v8Model, true)))
        return RenderMaterialId();

    return m_converter.GetRemappedMaterial(material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SyncInfo::InsertLineStyle(DgnStyleId newId, double componentScale, V8StyleId oldId)
    {
    MappedLineStyle mapEntry(newId, componentScale);
    m_lineStyle[oldId] = mapEntry;

    // WIP_CONVERTER -- write syncinfo 
    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------**//**
 _RemapLineStyle adds an entry with an invalid ID when it finds an entry that cannot
 be mapped (probably because the original definition could not be found).  When FindLineStyle
 returns an invalid DgnStyleId, _RemapLineStyle needs to know if it is because the entry was 
 never added or because the entry was added with an invalid DgnStyleId. It uses foundStyle
 to determine that.

* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyleId SyncInfo::FindLineStyle(double&unitsScale, bool& foundStyle, V8StyleId oldId)
    {
    // WIP_CONVERTER -- read syncInfo
    auto i = m_lineStyle.find(oldId);
    if (i == m_lineStyle.end())
        {
        unitsScale = 1;
        foundStyle = false;
        return DgnStyleId();
        }

    foundStyle = true;
    unitsScale = i->second.m_unitsScale;
    return i->second.m_id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::DiskFileInfo::GetInfo(BeFileNameCR fileName)
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
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SyncInfo::GetUniqueNameForFile(DgnV8FileCR file)
    {
    //  The unique name is the key into the syncinfo_file table. 
    //  Therefore, we must distinguish between like-named files in different directories.
    //  The unique name must also be stable. If the whole project is moved to a new directory or machine, 
    //  the unique names of the files must be unaffected.

    // If we have a DMS URN for the document corresponding to this file, that is the unique name.
    Utf8String urn = GetConverter().GetDocumentURNforFile(file);
    if (!urn.empty())
        {
        if (iModelBridge::IsNonFileURN(urn))
            return urn;

        // I would rather use the doc GUID than a file:// URL or a file path.
        iModelBridgeDocumentProperties docProps;
        GetConverter().GetDocumentProperties(docProps, BeFileName(file.GetFileName().c_str()));
        if (!docProps.m_docGuid.empty())
            return docProps.m_docGuid;

        return urn;
        }

    // If we do not have a DMS URN or a doc GUID, we try to compute a stable unique name from the filename.
    // The full path should be unique already. To get something that is stable, we use only as much of 
    // the full path as we need to distinguish between like-named files in different directories.
    BeFileName fullFileName(file.GetFileName().c_str());
    WString uniqueName(fullFileName);
    auto pdir = m_converter.GetParams().GetInputRootDir();
    if (!pdir.empty() && (pdir.size() < fullFileName.size()) && pdir.EqualsI(fullFileName.substr(0, pdir.size())))
        uniqueName = fullFileName.substr(pdir.size());

    uniqueName.ToLower();  // make sure we don't get fooled by case-changes in file system on Windows
    return Utf8String(uniqueName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::ImportJob::CreateTable (BeSQLite::Db& db)
    {
    if (db.TableExists(SYNCINFO_ATTACH(SYNC_TABLE_ImportJob)))
        return;
    db.CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_ImportJob),
                         "V8ModelSyncInfoId INTEGER PRIMARY KEY,"
                         "Transform BLOB,"
                         "Type INTEGER,"
                         "Prefix TEXT,"
                         "SubjectId BIGINT NOT NULL"
                         ); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult SyncInfo::ImportJob::Insert (BeSQLite::Db& db) const
    {
    Statement stmt;
    stmt.Prepare(db, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_ImportJob) "(V8ModelSyncInfoId,Transform,Type,Prefix,SubjectId) VALUES (?,?,?,?,?)");
    int col = 1;
    stmt.BindInt(col++, m_v8RootModel.GetValue());
    stmt.BindBlob(col++, &m_transform, sizeof(m_transform), Statement::MakeCopy::No);
    stmt.BindInt(col++, (int)m_type);
    stmt.BindText(col++, m_prefix, Statement::MakeCopy::No);
    stmt.BindId(col++, m_subjectId);
    auto res = stmt.Step();
    m_ROWID = db.GetLastInsertRowId();
    return res;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult SyncInfo::ImportJob::Update (BeSQLite::Db& db) const
    {
    Statement stmt;
    stmt.Prepare(db, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_ImportJob) " SET Transform=?,Prefix=?,SubjectId=? WHERE(ROWID=?)");
    int col = 1;
    stmt.BindBlob(col++, &m_transform, sizeof(m_transform), Statement::MakeCopy::No);
    stmt.BindText(col++, m_prefix, Statement::MakeCopy::No);
    stmt.BindId(col++, m_subjectId);
    stmt.BindInt64(col++, m_ROWID);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SyncInfo::ImportJob::GetSelectSql()
    {
    return "SELECT ROWID,V8ModelSyncInfoId,Transform,Type,Prefix,SubjectId FROM " SYNCINFO_ATTACH(SYNC_TABLE_ImportJob);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::ImportJob::FromSelect(BeSQLite::Statement& stmt)
    {
    int col = 0;
    m_ROWID = stmt.GetValueInt64(col++);
    m_v8RootModel = V8ModelSyncInfoId(stmt.GetValueInt(col++));
    memcpy(&m_transform, stmt.GetValueBlob(col++), sizeof(Transform));
    m_type = (Type)stmt.GetValueInt(col++);
    m_prefix = stmt.GetValueText(col++);
    m_subjectId = stmt.GetValueId<DgnElementId>(col++);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::ImportJobIterator::ImportJobIterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);
    Utf8String sqlString = MakeSqlString(ImportJob::GetSelectSql().c_str());
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::ImportJob SyncInfo::ImportJobIterator::ImportJobIterator::Entry::GetimportJob()
    {
    SyncInfo::ImportJob importJob;
    importJob.FromSelect(*m_sql);
    return importJob;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::ImportJobIterator::Entry SyncInfo::ImportJobIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::ImportJob::FindById(ImportJob& importJob, DgnDbCR db, V8ModelSyncInfoId modelsiid)
    {
    if (!db.TableExists(SYNCINFO_ATTACH(SYNC_TABLE_ImportJob)))
        return BSIERROR;

    if (!modelsiid.IsValid())
        return BSIERROR;

    ImportJobIterator iter(db, "V8ModelSyncInfoId=?");
    iter.GetStatement()->BindInt64(1, modelsiid.GetValue());
    auto i = iter.begin();
    if (i == iter.end())
        return BSIERROR;
    importJob.FromSelect(*iter.GetStatement());
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::FindImportJobByV8RootModelId(ImportJob& importJob, SyncInfo::V8ModelSyncInfoId modelsiid)
    {
    return ImportJob::FindById(importJob, *m_dgndb, modelsiid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedImportJob Converter::FindSoleImportJobForFile(DgnV8FileR rootFile)
    {
    SyncInfo::V8FileProvenance provenance = m_syncInfo.FindFile(rootFile);
    if (!provenance.IsValid())
        return ResolvedImportJob();

    auto fsid = GetV8FileSyncInfoId(rootFile); // (makes sure that syncinfo is cached in file's appdata)
    if (!fsid.IsValid())
        return ResolvedImportJob();

    Statement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT importJob.V8ModelSyncInfoId FROM " 
                 SYNCINFO_ATTACH(SYNC_TABLE_ImportJob) " importJob, "
                 SYNCINFO_ATTACH(SYNC_TABLE_Model) " model "
                 "WHERE model.V8FileSyncInfoId=? AND importJob.V8ModelSyncInfoId = model.ROWID");
    stmt.BindInt(1, fsid.GetValue());
    if (stmt.Step() != BE_SQLITE_ROW)
        return ResolvedImportJob();
        
    SyncInfo::V8ModelSyncInfoId msid = stmt.GetValueId<SyncInfo::V8ModelSyncInfoId>(0);

    SyncInfo::ImportJob importJob;
    GetSyncInfo().FindImportJobByV8RootModelId(importJob, msid);    // grab the data now, before we step again

    if (BE_SQLITE_ROW == stmt.Step())                               // check that there is only ONE ImportJob record for this file
        {
        OnFatalError(IssueCategory::CorruptData(), Issue::Error(), "Multiple ImportJobs are registered for the root file. You must specify a root model in order to select the one you want to use.");
        BeAssert(false);
        return ResolvedImportJob();
        }

    return GetResolvedImportJob(importJob);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedImportJob Converter::FindImportJobForModel(DgnV8ModelR rootModel)
    {
    auto fsid = GetV8FileSyncInfoId(*rootModel.GetDgnFileP());
    if (!fsid.IsValid())
        return ResolvedImportJob();

    Statement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT importJob.V8ModelSyncInfoId FROM " 
                 SYNCINFO_ATTACH(SYNC_TABLE_ImportJob) " importJob, "
                 SYNCINFO_ATTACH(SYNC_TABLE_Model) " model "
                 "WHERE model.V8FileSyncInfoId=? AND model.V8Id=? AND importJob.V8ModelSyncInfoId = model.ROWID");
    stmt.BindInt(1, fsid.GetValue());
    stmt.BindInt(2, rootModel.GetModelId());

    if (BE_SQLITE_ROW != stmt.Step())
        return ResolvedImportJob();

    SyncInfo::V8ModelSyncInfoId msid = stmt.GetValueId<SyncInfo::V8ModelSyncInfoId>(0);
    SyncInfo::ImportJob importJob;
    GetSyncInfo().FindImportJobByV8RootModelId(importJob, msid);
    return GetResolvedImportJob(importJob);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::GeomPart::CreateTable (BeSQLite::Db& db)
    {
    if (db.TableExists(SYNCINFO_ATTACH(SYNC_TABLE_GeomPart)))
        return;
    db.CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_GeomPart),
                         "Id INTEGER PRIMARY KEY,"
                         "Tag TEXT"
                         );
    db.ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_GeomPart) "TagIdx ON "  SYNC_TABLE_GeomPart "(Tag)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult SyncInfo::GeomPart::Insert (BeSQLite::Db& db) const
    {
    Statement stmt;
    stmt.Prepare(db, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_GeomPart) "(Id,Tag) VALUES (?,?)");
    int col = 1;
    stmt.BindId(col++, m_id);
    stmt.BindText(col++, m_tag, Statement::MakeCopy::No);
    auto res = stmt.Step();
    return res;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SyncInfo::GeomPart::GetSelectSql()
    {
    return "SELECT Id,Tag FROM " SYNCINFO_ATTACH(SYNC_TABLE_GeomPart);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::GeomPart::FromSelect(BeSQLite::Statement& stmt)
    {
    int col = 0;
    m_id = stmt.GetValueId<DgnGeometryPartId>(col++);
    m_tag = stmt.GetValueText(col++);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::GeomPartIterator::GeomPartIterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);
    Utf8String sqlString = MakeSqlString(GeomPart::GetSelectSql().c_str());
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::GeomPart SyncInfo::GeomPartIterator::GeomPartIterator::Entry::GetGeomPart()
    {
    SyncInfo::GeomPart GeomPart;
    GeomPart.FromSelect(*m_sql);
    return GeomPart;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::GeomPartIterator::Entry SyncInfo::GeomPartIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::GeomPart::FindByTag(GeomPart& GeomPart, DgnDbCR db, Utf8CP tag)
    {
    if (!db.TableExists(SYNCINFO_ATTACH(SYNC_TABLE_GeomPart)))
        return BSIERROR;

    if (Utf8String::IsNullOrEmpty(tag))
        return BSIERROR;

    GeomPartIterator iter(db, "Tag=?");
    iter.GetStatement()->BindText(1, tag, Statement::MakeCopy::No);
    auto i = iter.begin();
    if (i == iter.end())
        return BSIERROR;
    GeomPart.FromSelect(*iter.GetStatement());
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::GeomPart::FindById(GeomPart& GeomPart, DgnDbCR db, DgnGeometryPartId partId)
    {
    if (!db.TableExists(SYNCINFO_ATTACH(SYNC_TABLE_GeomPart)))
        return BSIERROR;

    GeomPartIterator iter(db, "Id=?");
    iter.GetStatement()->BindId(1, partId);
    auto i = iter.begin();
    if (i == iter.end())
        return BSIERROR;
    GeomPart.FromSelect(*iter.GetStatement());
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::ModelId SyncInfo::GetV8ModelIdFromV8ModelSyncInfoId(V8ModelSyncInfoId msiid)
    {
    SyncInfo::V8ModelMapping mapping;
    GetModelBySyncInfoId(mapping, msiid);
    return mapping.GetV8ModelId().GetValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::InsertImportJob(ImportJob const& importJob)
    {
    return (BE_SQLITE_DONE == importJob.Insert(*m_dgndb))? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::UpdateImportJob(ImportJob const& importJob)
    {
    return (BE_SQLITE_DONE == importJob.Update(*m_dgndb))? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8FileProvenance::V8FileProvenance(DgnV8FileCR file, SyncInfo& sync, StableIdPolicy policy) : m_syncInfo(&sync)
    {
    if (!file.IsEmbeddedFile())
        {
        GetInfo(BeFileName(file.GetFileName().c_str()));
        m_lastSaveTime = ((DgnV8FileR) file).GetLastSaveTime();
        }

    m_idPolicy = policy;
    WString fullFileName(file.GetFileName().c_str());
    m_v8Name = Utf8String(fullFileName);
    m_uniqueName = sync.GetUniqueNameForFile(file);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SyncInfo::V8FileProvenance::Insert()
    {
    Statement stmt;
    stmt.Prepare(*m_syncInfo->m_dgndb, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_File) "(UniqueName,V8Name,LastSaveTime,LastModified,FileSize,UseHash) VALUES (?,?,?,?,?,?)");

    int col = 1;
    stmt.BindText(col++, m_uniqueName, Statement::MakeCopy::No);
    stmt.BindText(col++, m_v8Name, Statement::MakeCopy::No);
    stmt.BindDouble(col++, m_lastSaveTime);
    stmt.BindInt64(col++, m_lastModifiedTime);
    stmt.BindInt64(col++, m_fileSize);
    stmt.BindInt(col++, m_idPolicy == StableIdPolicy::ByHash ? 1 : 0);

    DbResult rc = stmt.Step();
    BeAssert(rc == BE_SQLITE_DONE);

    auto rowid = m_syncInfo->m_dgndb->GetLastInsertRowId();
    BeAssert(rowid <= UINT32_MAX);
    m_syncId = V8FileSyncInfoId((uint32_t) rowid);

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8FileProvenance SyncInfo::InsertFile(DbResult* err, DgnV8FileCR file, StableIdPolicy policy)
    {
    V8FileProvenance prov(file, *this, policy);
    auto rc = prov.Insert();
    if (rc == BE_SQLITE_DONE)
        return prov;
    if (nullptr != err)
        *err = rc;
    return V8FileProvenance(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8FileProvenance SyncInfo::UpdateFile(DbResult* err, DgnV8FileCR file)
    {
    V8FileProvenance prov = FindFile(file);
    if (!prov.IsValid())
        {
        if (err)
            *err = BE_SQLITE_NOTFOUND;
        return prov;
        }

    V8FileProvenance currentStats(file, *this, prov.m_idPolicy); // Get the current time, etc.

    auto rc = prov.Update(prov.m_syncId, currentStats);
    if (rc == BE_SQLITE_DONE)
        return prov;
    if (nullptr != err)
        *err = rc;
    return V8FileProvenance(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SyncInfo::V8FileProvenance::Update(V8FileSyncInfoId id, FileInfo const& latest)
    {
    Statement stmt;
    stmt.Prepare(*m_syncInfo->m_dgndb, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_File) " SET LastSaveTime=?,LastModified=?,FileSize=? WHERE Id=?");
    int col = 1;
    stmt.BindDouble(col++, latest.m_lastSaveTime);
    stmt.BindInt64(col++, latest.m_lastModifiedTime);
    stmt.BindInt64(col++, latest.m_fileSize);
    stmt.BindInt(col++, id.GetValue());
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8FileProvenance SyncInfo::FindFileByUniqueName(Utf8StringCR uname)
    {
    FileIterator files(GetConverter().GetDgnDb(), "UniqueName=?");
    files.GetStatement()->BindText(1, uname, Statement::MakeCopy::No);
    SyncInfo::FileIterator::Entry entry = files.begin();
    return (entry == files.end())? V8FileProvenance(*this): entry.GetV8FileProvenance(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8FileProvenance SyncInfo::FindFile(DgnV8FileCR file)
    {
    return FindFileByUniqueName(GetUniqueNameForFile(file));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8FileProvenance SyncInfo::FindFileByFileName(BeFileNameCR fullFileName)
    {
    // Make the worst-case assumption that the local files have been moved to a new directory.
    // Consider only the relative path of the file.
    auto dirPrefix = m_converter.GetParams().GetInputRootDir();
    size_t prefixLen = dirPrefix.size();
    if (!fullFileName.StartsWithI(dirPrefix.c_str())) // ??!
        prefixLen = 0;

    Utf8String searchName(fullFileName.substr(prefixLen));
    
    FileIterator files(GetConverter().GetDgnDb(), "V8Name LIKE ?");
    files.GetStatement()->BindText(1, searchName, Statement::MakeCopy::Yes);
    for (auto entry : files)
        {
        if (entry.GetV8Name().EndsWithI(searchName.c_str()))
            return entry.GetV8FileProvenance(*this);
        }
    return V8FileProvenance(*this);;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
SyncInfo::V8FileProvenance SyncInfo::FileIterator::Entry::GetV8FileProvenance(SyncInfo& si)
    {
    V8FileProvenance fp(si);
    fp.m_syncId = GetV8FileSyncInfoId();
    fp.m_uniqueName = GetUniqueName();
    fp.m_v8Name = GetV8Name();
    fp.m_idPolicy = GetCannotUseElementIds()? StableIdPolicy::ByHash : StableIdPolicy::ById;
    fp.m_lastSaveTime = GetLastSaveTime();
    fp.m_lastModifiedTime = GetLastModifiedTime();
    fp.m_fileSize = GetFileSize();
    return fp;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
SyncInfo::V8FileProvenance SyncInfo::FindFileById(V8FileSyncInfoId sid)
    {
    if (!sid.IsValid())
        return V8FileProvenance(*this);
    FileIterator files(GetConverter().GetDgnDb(), "Id=?");
    files.GetStatement()->BindInt(1, sid.GetValue());
    SyncInfo::FileIterator::Entry entry = files.begin();
    return (entry == files.end())? V8FileProvenance(*this): entry.GetV8FileProvenance(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool SyncInfo::HasDiskFileChanged(BeFileNameCR fileName)
    {
    V8FileProvenance prov = FindFileByFileName(fileName);
    if (!prov.IsValid())
        return true;

    // This is an attempt to tell if a file has *not* changed, looking only at the file's time and size.
    // This is a dangerous test, since we don't look at the contents, but we think that we can narrow 
    // the odds of a mistake by:
    // 1. using times measured in hectonanoseconds (on Windows, at least),
    // 2. also using file size
    SyncInfo::DiskFileInfo df;
    df.GetInfo(fileName);
    return df.m_lastModifiedTime != prov.m_lastModifiedTime || df.m_fileSize != prov.m_fileSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool SyncInfo::HasLastSaveTimeChanged(DgnV8FileCR v8File)
    {
    if (v8File.IsEmbeddedFile())
        return false;

    V8FileProvenance previous = FindFile(v8File);
    if (!previous.IsValid())
        return true;

    auto lastSaveTime = ((DgnV8FileR) v8File).GetLastSaveTime ();

    // a non-DGN FileIO may not set the last saved time in the file header - resort to the last modified time in such a case:
    if (0.0 == lastSaveTime)
        {
        BeFileName  filename(v8File.GetFileName().c_str());
        SyncInfo::DiskFileInfo diskfile;
        diskfile.GetInfo (filename);
        return diskfile.m_lastModifiedTime != previous.m_lastModifiedTime;
        }

    return lastSaveTime != previous.m_lastSaveTime;
    }

SyncInfo::V8FileSyncInfoId SyncInfo::FileIterator::Entry::GetV8FileSyncInfoId() { return SyncInfo::V8FileSyncInfoId(m_sql->GetValueInt(0)); }
Utf8String SyncInfo::FileIterator::Entry::GetUniqueName() { return m_sql->GetValueText(1); }
Utf8String SyncInfo::FileIterator::Entry::GetV8Name() { return m_sql->GetValueText(2); }
bool SyncInfo::FileIterator::Entry::GetCannotUseElementIds() { return 0 != m_sql->GetValueInt(3); }
double SyncInfo::FileIterator::Entry::GetLastSaveTime() { return m_sql->GetValueDouble(4); }
uint64_t SyncInfo::FileIterator::Entry::GetLastModifiedTime() { return m_sql->GetValueInt64(5); }
uint64_t SyncInfo::FileIterator::Entry::GetFileSize() { return m_sql->GetValueInt64(6); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::FileIterator::FileIterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);
    Utf8String sqlString = MakeSqlString("SELECT Id,UniqueName,V8Name,UseHash,LastSaveTime,LastModified,FileSize FROM " SYNCINFO_ATTACH(SYNC_TABLE_File));
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::FileIterator::Entry SyncInfo::FileIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

SyncInfo::V8ModelSyncInfoId SyncInfo::ModelIterator::Entry::GetV8ModelSyncInfoId() { return V8ModelSyncInfoId(m_sql->GetValueInt64(0)); }
DgnModelId SyncInfo::ModelIterator::Entry::GetModelId() { return m_sql->GetValueId<DgnModelId>(1); }
SyncInfo::V8FileSyncInfoId SyncInfo::ModelIterator::Entry::GetV8FileSyncInfoId() { return V8FileSyncInfoId(m_sql->GetValueInt(2)); }
SyncInfo::V8ModelId SyncInfo::ModelIterator::Entry::GetV8ModelId() { return V8ModelId(m_sql->GetValueInt(3)); }
Utf8CP SyncInfo::ModelIterator::Entry::GetV8Name() { return m_sql->GetValueText(4); }
Transform SyncInfo::ModelIterator::Entry::GetTransform()
    {
    if (m_sql->IsColumnNull(5))
        return Transform::FromIdentity();

    Transform t;
    memcpy(&t, m_sql->GetValueBlob(5), sizeof(t));
    return t;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::DeleteFile(V8FileSyncInfoId filesiid)
    {
    Statement stmt;
    stmt.Prepare(*m_dgndb, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_File) " WHERE ROWID=?");
    stmt.BindInt64(1, filesiid.GetValue());
    return stmt.Step() == BE_SQLITE_DONE ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::ModelIterator::ModelIterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);
    Utf8String sqlString = MakeSqlString("SELECT ROWID,ModelId,V8FileSyncInfoId,V8Id,V8Name,Transform FROM " SYNCINFO_ATTACH(SYNC_TABLE_Model));
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::ModelIterator::Entry SyncInfo::ModelIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SyncInfo::V8ModelMapping::Insert(Db& db) const
    {
    if (!m_modelId.IsValid())
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    Statement stmt;
    stmt.Prepare(db, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Model) " (ModelId,V8FileSyncInfoId,V8Id,V8Name,Transform) VALUES (?,?,?,?,?)");
    int col = 1;
    stmt.BindId(col++, m_modelId);
    stmt.BindInt(col++, m_source.m_v8FileSyncInfoId.GetValue());
    stmt.BindInt(col++, m_source.m_modelId.GetValue());
    stmt.BindText(col++, m_v8Name, Statement::MakeCopy::No);
    if (m_transform.IsIdentity())
        stmt.BindNull(col++);
    else
        stmt.BindBlob(col++, &m_transform, sizeof(m_transform), Statement::MakeCopy::No);

    auto rc = stmt.Step();
    if (BE_SQLITE_DONE == rc)
        m_syncInfoId = V8ModelSyncInfoId(db.GetLastInsertRowId());

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SyncInfo::V8ModelMapping::Update(Db& db) const
    {
    if (!m_syncInfoId.IsValid() || !m_modelId.IsValid())
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    Statement stmt;
    stmt.Prepare(db, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_Model) " SET ModelId=?,V8FileSyncInfoId=?,V8Id=?,V8Name=?,Transform=? WHERE (ROWID=?)");
    int col = 1;
    stmt.BindId(col++, m_modelId);
    stmt.BindInt(col++, m_source.m_v8FileSyncInfoId.GetValue());
    stmt.BindInt(col++, m_source.m_modelId.GetValue());
    stmt.BindText(col++, m_v8Name, Statement::MakeCopy::No);
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
SyncInfo::V8ModelMapping::V8ModelMapping()
    {
    m_transform.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ModelMapping::V8ModelMapping(DgnModelId mid, DgnV8ModelCR v8Model, TransformCR trans)
    {
    m_v8Name = Utf8String(v8Model.GetModelNameCP());
    m_source = V8ModelSource(v8Model);
    m_transform = trans;
    m_modelId = mid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::InsertModel(V8ModelMapping& modelMap, DgnModelId mid, DgnV8ModelCR v8Model, TransformCR trans)
    {
    modelMap = V8ModelMapping(mid, v8Model, trans);

    auto rc = modelMap.Insert(*m_dgndb);
    return (BE_SQLITE_DONE == rc) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::GetModelBySyncInfoId(V8ModelMapping& mapping, V8ModelSyncInfoId modelsiid)
    {
    SyncInfo::ModelIterator it(*GetDgnDb(), "ROWID=?");
    it.GetStatement()->BindInt64(1, modelsiid.GetValue());
    for (auto entry = it.begin(); entry != it.end(); ++entry)
        {
        if (entry.GetV8ModelSyncInfoId() == modelsiid)
            {
            mapping = entry.GetMapping();
            return BSISUCCESS;
            }
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::FindModel(V8ModelMapping* mapping, DgnV8ModelCR v8Model, TransformCP modelTrans, StableIdPolicy idPolicy)
    {
    // Note: We can't call Converter::GetModelFromSyncInfo at this stage, because it hasn't set up the m_v8Files array yet
    SyncInfo::V8FileProvenance provenance = FindFile(*v8Model.GetDgnFileP());
    if (!provenance.IsValid())
        return BSIERROR;

    SyncInfo::ModelIterator it(*GetDgnDb(), "V8FileSyncInfoId=? AND V8Id=?");
    it.GetStatement()->BindInt(1, provenance.m_syncId.GetValue());
    it.GetStatement()->BindInt(2, v8Model.GetModelId());

    for (auto entry=it.begin(); entry!=it.end(); ++entry)
        {
        if (nullptr == modelTrans || Converter::IsTransformEqualWithTolerance(entry.GetTransform(),*modelTrans))
            {
            if (nullptr != mapping)
                {
                *mapping = entry.GetMapping();
                }
            return BentleyApi::SUCCESS;
            }
        }

    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::DeleteModel(V8ModelSyncInfoId modelsiid)
    {
    Statement stmt;
    stmt.Prepare(*m_dgndb, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_Model) " WHERE ROWID=?");
    stmt.BindInt64(1, modelsiid.GetValue());
    return stmt.Step() == BE_SQLITE_DONE ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::UpdateElement(V8ElementMapping const& mapping)
    {
    if (!mapping.m_elementId.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_Element) " SET LastModified=?,Hash=? WHERE ElementId=?");
    int col = 1;
    stmt->BindDouble(col++, mapping.m_provenance.m_lastModified);
    stmt->BindBlob(col++, &mapping.m_provenance.m_hash, sizeof(mapping.m_provenance.m_hash), Statement::MakeCopy::No);
    stmt->BindId(col++, mapping.m_elementId);
    return (stmt->Step() == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::InsertElement(V8ElementMapping const& mapping)
    {
    if (!mapping.IsValid())
        {
        BeAssert(false);
        return BSIERROR;
        }

    if (m_converter.IsUpdating())
        DeleteDiscardedElement(mapping.m_v8ElementId, mapping.m_v8ModelSyncInfoId); // just in case it was previously recorded as a discard

    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Element) " (V8ModelSyncInfoId,V8ElementId,ElementId,LastModified,Hash) VALUES (?,?,?,?,?)");

    int col = 1;
    stmt->BindInt(col++, mapping.m_v8ModelSyncInfoId.GetValue());

    if (mapping.m_provenance.m_idPolicy == StableIdPolicy::ById)
        stmt->BindInt64(col++, mapping.m_v8ElementId);
    else
        stmt->BindNull(col++);

    stmt->BindId(col++, mapping.m_elementId);
    stmt->BindDouble(col++, mapping.m_provenance.m_lastModified);
    stmt->BindBlob(col++, &mapping.m_provenance.m_hash, sizeof(mapping.m_provenance.m_hash), Statement::MakeCopy::No);
    return (stmt->Step() == BE_SQLITE_DONE)? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::DeleteElement(DgnElementId gid)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_Element) " WHERE ElementId=?");
    stmt->BindId(1, gid);
    return (stmt->Step() == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle      03/15
//+---------------+---------------+---------------+---------------+---------------+------
bool SyncInfo::TryFindElement(DgnElementId& elementId, DgnV8EhCR eh) const
    {
    elementId = DgnElementId();
    CachedStatementPtr stmt = nullptr;
    m_dgndb->GetCachedStatement(stmt, "SELECT ElementId FROM " SYNCINFO_ATTACH(SYNC_TABLE_Element) " as element " 
                                " INNER JOIN " SYNCINFO_ATTACH(SYNC_TABLE_Model) " as model "
                                " ON element.V8ModelSyncInfoId=model.ROWID "
                                " WHERE model.V8FileSyncInfoId=? AND V8ElementId=?");

    V8ModelSource source(*eh.GetDgnModelP());

    // ***
    // *** NEEDS WORK: This check for a type-100 doesn't make sense. Are we trying to handle case of a V8 "far reference" to a V8 element? It won't be a type-100.
    // ***
    DgnV8FileP dgnV8File = eh.GetDgnFileP();
    if (eh.GetElementType() == DgnV8Api::REFERENCE_ATTACH_ELM)
        {
        DgnV8Api::DgnModelRef* model = eh.GetDgnModelP();
        model = DgnV8Api::DependencyManager::ResolveReferenceAttachment(model, eh.GetElementId());
        if (nullptr != model)
            dgnV8File = model->GetDgnFileP();
        }
    SyncInfo::V8FileSyncInfoId fileId = Converter::GetV8FileSyncInfoIdFromAppData(*dgnV8File);

    stmt->BindInt(1, fileId.GetValue());
    stmt->BindInt64(2, eh.GetElementId());

    const DbResult stat = stmt->Step();
    if (BE_SQLITE_ROW == stat)
        {
        if (!stmt->IsColumnNull(0))
            {
            elementId = DgnElementId(stmt->GetValueUInt64(0));
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool SyncInfo::IsMappedToSameV8Element(DgnElementId elementId, DgnElementIdSet const& known) const
    {
    ElementIterator findByBimId(*m_dgndb, "ElementId=?");
    findByBimId.GetStatement()->BindId(1, elementId);
    auto iThisElement = findByBimId.begin();
    if (iThisElement == findByBimId.end())
        return false;

    if (iThisElement.GetV8ElementId() == 0)
        {
        // when StableIdPolicy==ByHash
        ByHashIter  othersMappedToV8Hash(*m_dgndb);
        othersMappedToV8Hash.Bind(iThisElement.GetV8ModelSyncInfoId(), iThisElement.GetProvenance().m_hash);
        for (auto const& otherMappedToV8Hash : othersMappedToV8Hash)
            {
            if (known.find(otherMappedToV8Hash.GetElementId()) != known.end())
                return true;
            }
        }
    else
        {
        // when StableIdPolicy==ById
        ByV8ElementIdIter othersMappedToV8Id(*m_dgndb);
        othersMappedToV8Id.Bind(iThisElement.GetV8ModelSyncInfoId(), iThisElement.GetV8ElementId());
        for (auto const& otherMappedToV8Id : othersMappedToV8Id)
            {
            if (known.find(otherMappedToV8Id.GetElementId()) != known.end())
                return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::ElementProvenance::ElementProvenance(StatementP sql)
    {
    m_lastModified = sql->GetValueDouble(3);
    memcpy(&m_hash, sql->GetValueBlob(4), sizeof(m_hash));
    m_idPolicy = sql->IsColumnNull(1) ? StableIdPolicy::ByHash : StableIdPolicy::ById;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ModelSyncInfoId SyncInfo::ElementIterator::Entry::GetV8ModelSyncInfoId() const { return V8ModelSyncInfoId(m_sql->GetValueInt64(0)); }
uint64_t SyncInfo::ElementIterator::Entry::GetV8ElementId() const { return m_sql->GetValueInt64(1); }
DgnElementId SyncInfo::ElementIterator::Entry::GetElementId() const { return m_sql->GetValueId<DgnElementId>(2); }
SyncInfo::ElementProvenance SyncInfo::ElementIterator::Entry::GetProvenance() const { return SyncInfo::ElementProvenance(m_sql); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::ElementIterator::ElementIterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);
    Utf8String sqlString = MakeSqlString("SELECT V8ModelSyncInfoId,V8ElementId,ElementId,LastModified,Hash FROM " SYNCINFO_ATTACH(SYNC_TABLE_Element));
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::ElementIterator::Entry SyncInfo::ElementIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::InsertExtractedGraphic(V8ElementSource const& attachment, 
                                               V8ElementSource const& originalElement, 
                                               DgnCategoryId categoryId, DgnElementId extractedGraphic)
    {
    if (!originalElement.IsValid() || !categoryId.IsValid() || !extractedGraphic.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_ExtractedGraphic) 
                              " (DrawingV8ModelSyncInfoId,AttachmentV8ElementId,"
                                "OriginalV8ModelSyncInfoId,OriginalV8ElementId,"
                                "Category,Graphic)    VALUES (?,?,?,?,?,?)");

    int col = 1;
    stmt->BindInt64(col++, attachment.m_v8ModelSyncInfoId.GetValue());
    stmt->BindInt64(col++, attachment.m_v8ElementId);
    stmt->BindInt64(col++, originalElement.m_v8ModelSyncInfoId.GetValue());
    stmt->BindInt64(col++, originalElement.m_v8ElementId);
    stmt->BindId(col++, categoryId);
    stmt->BindId(col++, extractedGraphic);
    return (stmt->Step() == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::DeleteExtractedGraphics(V8ElementSource const& attachment, 
                                                V8ElementSource const& originalElement)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_ExtractedGraphic) 
                                " WHERE (DrawingV8ModelSyncInfoId=? AND AttachmentV8ElementId=?"
                                "    AND OriginalV8ModelSyncInfoId=?   AND OriginalV8ElementId=?)");
    int col = 1;
    stmt->BindInt64(col++, attachment.m_v8ModelSyncInfoId.GetValue());
    stmt->BindInt64(col++, attachment.m_v8ElementId);
    stmt->BindInt64(col++, originalElement.m_v8ModelSyncInfoId.GetValue());
    stmt->BindInt64(col++, originalElement.m_v8ElementId);
    return (stmt->Step() == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::DeleteExtractedGraphicsCategory(V8ElementSource const& attachment, 
                                                        V8ElementSource const& originalElement,
                                                        DgnCategoryId categoryId)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_ExtractedGraphic) 
                                " WHERE (DrawingV8ModelSyncInfoId=? AND AttachmentV8ElementId=?"
                                "    AND OriginalV8ModelSyncInfoId=?   AND OriginalV8ElementId=?"
                                "    AND Category=?)");
    int col = 1;
    stmt->BindInt64(col++, attachment.m_v8ModelSyncInfoId.GetValue());
    stmt->BindInt64(col++, attachment.m_v8ElementId);
    stmt->BindInt64(col++, originalElement.m_v8ModelSyncInfoId.GetValue());
    stmt->BindInt64(col++, originalElement.m_v8ElementId);
    stmt->BindId(col++, categoryId);
    return (stmt->Step() == BE_SQLITE_DONE) ? BSISUCCESS : BSIERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Sam.Wilson      09/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnElementId SyncInfo::FindExtractedGraphic(V8ElementSource const& attachment, 
                                            V8ElementSource const& originalElement, 
                                            DgnCategoryId categoryId)
    {
    CachedStatementPtr stmt = nullptr;
    m_dgndb->GetCachedStatement(stmt, "SELECT Graphic FROM " SYNCINFO_ATTACH(SYNC_TABLE_ExtractedGraphic) 
                                " WHERE (DrawingV8ModelSyncInfoId=? AND AttachmentV8ElementId=?"
                                "    AND OriginalV8ModelSyncInfoId=?   AND OriginalV8ElementId=?"
                                "    AND Category=?)");
    int col = 1;
    stmt->BindInt64(col++, attachment.m_v8ModelSyncInfoId.GetValue());
    stmt->BindInt64(col++, attachment.m_v8ElementId);
    stmt->BindInt64(col++, originalElement.m_v8ModelSyncInfoId.GetValue());
    stmt->BindInt64(col++, originalElement.m_v8ElementId);
    stmt->BindId(col++, categoryId);

    bvector<DgnElementId> graphics;
    if (BE_SQLITE_ROW == stmt->Step())
        {
        return stmt->GetValueId<DgnElementId>(0);
        }
    
    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SyncInfo::Level::Insert(Db& db) const
    {
    if (!m_id.IsValid())
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    Statement stmt;
    stmt.Prepare(db, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Level) " (Id,V8FileSyncInfoId,V8Model,V8Id,V8Name,Type) VALUES (?,?,?,?,?,?)");
    int col = 1;
    stmt.BindId(col++, m_id);
    stmt.BindInt(col++, m_fm.GetV8FileSyncInfoId().GetValue());
    stmt.BindInt(col++, m_fm.GetV8ModelId().GetValue());
    stmt.BindInt(col++, m_v8Id);
    stmt.BindText(col++, m_v8Name.c_str(), Statement::MakeCopy::No); // V8Name
    stmt.BindInt(col++, (int)m_type);

    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::FindFirstSubCategory(DgnSubCategoryId& glid, BeSQLite::Db& db, V8ModelSource fm, uint32_t flid, Level::Type ltype)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "SELECT Id FROM " SYNCINFO_ATTACH(SYNC_TABLE_Level) " WHERE V8FileSyncInfoId=? AND V8Model=? AND V8Id=? AND Type=?");

    int col = 1;
    stmt->BindInt(col++, fm.GetV8FileSyncInfoId().GetValue());
    stmt->BindInt(col++, fm.GetV8ModelId().GetValue());
    stmt->BindInt(col++, flid);
    stmt->BindInt(col++, (int)ltype);

    if (stmt->Step() != BE_SQLITE_ROW)
        return BSIERROR;

    glid = stmt->GetValueId<DgnSubCategoryId>(0);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::Level SyncInfo::InsertLevel(DgnSubCategoryId subcategoryid, V8ModelSource fm, DgnV8Api::LevelHandle const& vlevel)
    {
    auto catid = DgnSubCategory::QueryCategoryId(*GetDgnDb(), subcategoryid);
    Level::Type ltype = m_converter.IsSpatialCategory(catid)? Level::Type::Spatial: Level::Type::Drawing;

    Level levelprov(subcategoryid, ltype, fm, vlevel.GetLevelId(), Utf8String(vlevel.GetName()).c_str());

    if (LOG_LEVEL_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
        LOG_LEVEL.tracev("InsertLevel %lld - f:%d m:%d id:%d n:%s", 
                         levelprov.m_id.GetValue(), 
                         levelprov.m_fm.m_v8FileSyncInfoId.GetValue(), levelprov.m_fm.m_modelId.GetValue(), levelprov.m_v8Id, 
                         levelprov.m_v8Name.c_str());

    auto rc = levelprov.Insert(*m_dgndb);
    if (BE_SQLITE_DONE != rc)
        {
        levelprov.m_id = DgnSubCategoryId();

        if (BeSQLiteLib::IsConstraintDbResult(rc))
            {
            //BeAssert(false);
            }
        else
            {
            m_converter.ReportIssue(Converter::IssueSeverity::Info, Converter::IssueCategory::InconsistentData(), Converter::Issue::InvalidLevel(),
                                    Utf8PrintfString("%s (%lld)", Utf8String(vlevel.GetName()).c_str(), vlevel.GetLevelId()).c_str());
            }
        }

    return levelprov;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId SyncInfo::FindSubCategory(uint32_t v8levelId, V8FileSyncInfoId fid, Level::Type ltype)
    {
    V8ModelSource modelSource(fid, V8ModelId());
    DgnSubCategoryId glid;
    return (FindFirstSubCategory(glid, *m_dgndb, modelSource, v8levelId, ltype) == BSISUCCESS) ? glid : DgnSubCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId SyncInfo::FindCategory(uint32_t v8levelId, V8FileSyncInfoId fid, Level::Type ltype)
    {
    DgnSubCategoryId subcatid = FindSubCategory(v8levelId, fid, ltype);
    return DgnSubCategory::QueryCategoryId(*GetDgnDb(), subcatid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId SyncInfo::FindSubCategory(uint32_t v8levelId, V8ModelSource fm, Level::Type ltype)
    {
    DgnSubCategoryId glid;
    return (FindFirstSubCategory(glid, *m_dgndb, fm, v8levelId, ltype) == BSISUCCESS) ? glid : DgnSubCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId SyncInfo::GetSubCategory(uint32_t v8levelId, V8ModelSource fm, Level::Type ltype)
    {
    DgnSubCategoryId glid;
    if (FindFirstSubCategory(glid, *m_dgndb, fm, v8levelId, ltype) != BSISUCCESS
        && FindFirstSubCategory(glid, *m_dgndb, V8ModelSource(fm.GetV8FileSyncInfoId(), V8ModelId()), v8levelId, ltype) != BSISUCCESS)
        {
        return DgnCategory::GetDefaultSubCategoryId(GetConverter().GetUncategorizedCategory()); // unable to categorize
        }

    return glid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t Converter::GetV8Level(DgnV8EhCR v8Eh)
    {
    uint32_t v8Level = v8Eh.GetElementCP()->ehdr.level;
    if (0 == v8Level && v8Eh.GetElementCP()->IsComplexHeader()) // level of cell header is not valid for category...
        {
        DgnV8Api::ElementHandle v8TemplateEh;
        if (DgnV8Api::ComplexHeaderDisplayHandler::GetComponentForDisplayParams(v8TemplateEh, v8Eh))
            v8Level = v8TemplateEh.GetElementCP()->ehdr.level; // find based on level of first component...
        }
    return v8Level;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId SyncInfo::GetCategory(DgnV8EhCR v8Eh, ResolvedModelMapping const& v8mm)
    {
    if (!v8Eh.GetElementCP()->ehdr.isGraphics)
        return v8mm.GetDgnModel().Is2dModel() ? GetConverter().GetUncategorizedDrawingCategory() : GetConverter().GetUncategorizedCategory(); // level of non-graphic element is not valid for category...

    uint32_t v8Level = Converter::GetV8Level(v8Eh);
    Level::Type ltype = v8mm.GetDgnModel().Is3d() ? Level::Type::Spatial : Level::Type::Drawing;
    DgnCategoryId categoryId;
    if (0 != v8Level)
        categoryId = FindCategory(v8Level, Converter::GetV8FileSyncInfoIdFromAppData(*v8Eh.GetDgnModelP()->GetDgnFileP()), ltype);

    return (categoryId.IsValid() ? categoryId : GetConverter().GetUncategorizedCategory()); // return uncategorized if we didn't find a valid category...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool SyncInfo::WasElementDiscarded(uint64_t vid, V8ModelSyncInfoId fm)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "SELECT 1 FROM " SYNCINFO_ATTACH(SYNC_TABLE_Discards) " WHERE V8ModelSyncInfoId=? AND V8Id=?");
    stmt->Reset();
    stmt->ClearBindings();
    stmt->BindInt64(1, fm.GetValue());
    stmt->BindInt64(2, vid);
    return stmt->Step() == BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::InsertDiscardedElement(DgnV8EhCR eeh, V8ModelSyncInfoId modelsiid)
    {
    DgnV8ModelP v8Model = eeh.GetDgnModelP();
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Discards) "(V8ModelSyncInfoId,V8Id) VALUES (?,?)");
    int col = 1;
    stmt->BindInt64(col++, modelsiid.GetValue());
    stmt->BindInt64(col++, eeh.GetElementId());
    stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::DeleteDiscardedElement(uint64_t vid, V8ModelSyncInfoId fm)
    {
    DiscardedElement deprov(fm, vid);

    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_Discards) " WHERE V8ModelSyncInfoId=? AND V8Id=?");
    int col = 1;
    stmt->BindInt64(col++, fm.GetValue());
    stmt->BindInt64(col++, deprov.m_v8Id);
    return stmt->Step() == BE_SQLITE_DONE ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName SyncInfo::GetDbFileName(BeFileNameCR dbname)
    {
    BeFileName name(dbname);
    name.append(L".syncinfo");
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName SyncInfo::GetDbFileName (DgnDb& db)
    {
    return GetDbFileName(BeFileName(db.GetDbFileName(), BentleyCharEncoding::Utf8));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::AttachToProject(DgnDb& targetProject, BeFileNameCR dbName)
    {
    DbResult rc = targetProject.AttachDb(Utf8String(dbName).c_str(), SYNCINFO_ATTACH_ALIAS);
    if (BE_SQLITE_OK != rc)
        return BSIERROR;
    return OnAttach(targetProject);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   importElementAspectSchema(DgnDbR db)
    {
    if (db.Schemas().ContainsSchema(SOURCEINFO_ECSCHEMA_NAME))
        return BSISUCCESS;

    BeFileName schemaPathname = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    schemaPathname.AppendToPath(L"ECSchemas/Application/SourceInfo.ecschema.xml");

    if (!schemaPathname.DoesPathExist())
        {
        LOG.errorv("Error reading schema %ls", schemaPathname.GetName());
        return BSIERROR;
        }

    ECN::ECSchemaPtr schema;
    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater(db.GetSchemaLocater());
    ECN::SchemaReadStatus status = ECN::ECSchema::ReadFromXmlFile(schema, schemaPathname.GetName(), *schemaContext);

    // CreateSearchPathSchemaFileLocater
    if (ECN::SchemaReadStatus::Success != status)
        {
        LOG.errorv("Error reading schema %ls", schemaPathname.GetName());
        return BSIERROR;
        }

    bvector<ECN::ECSchemaCP> schemas;
    schemas.push_back(schema.get());
    if (SchemaStatus::Success != db.ImportV8LegacySchemas(schemas))
        return BSIERROR;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef TEST_SYNC_INFO_ASPECT
void SyncInfo::AssertAspectMatchesSyncInfo(V8ElementMapping const& mapping)
    {
    auto el = m_converter.GetDgnDb().Elements().GetElement(mapping.GetElementId());
    if (!el.IsValid())
        return;

    auto props = V8ElementSyncInfoAspect::Get(*el, mapping.m_v8ElementId);
    if (!props.IsValid())
        {
        // BeAssert(!m_converter._GetParams().GetWantProvenanceInBim());    Can't assert this until I convert all of the places that create syncinfo records to also create aspects
        return;
        }

    props.AssertMatch(*el, mapping.m_v8ElementId, mapping.m_provenance);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef TEST_SYNC_INFO_ASPECT
void SyncInfo::V8ElementSyncInfoAspect::AssertMatch(DgnElementCR el, DgnV8Api::ElementId v8Id, ElementProvenance const& elprov)
    {
    BeAssert(GetV8ElementId() == v8Id);
    // TODO: Get and check the aspect corresponding to the original v8 model
    // BeAssert(GetScope().GetValue() == el.GetModelId().GetValue()); -- No. scope identifies the model in the bim that represents the v8 element's model. The v8 element itself might not have been added to that bim model. For example, when we encounter a NamedGroup definiton element in a model, we typically write it to the bim dictionary model.
    BeAssert(GetKind() == SyncInfoAspect::Kind::Element);
    iModelSyncInfoAspect::SourceState ss;
    BeAssert(GetSourceState(ss) == BSISUCCESS);
    BeAssert(0==memcmp(&ss.m_hash[0], elprov.m_hash.m_buffer, sizeof(elprov.m_hash.m_buffer)));
    BeAssert(ss.m_lastModifiedTime == elprov.m_lastModified);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<BeSQLite::EC::ECInstanceId> SyncInfo::GetSyncInfoAspectIds(DgnElementCR el, SyncInfoAspect::Kind kind, Utf8StringCR sourceId)
    {
    auto sel = el.GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId from " SOURCEINFO_ECSCHEMA_NAME "." SOURCEINFO_CLASS_SoureElementInfo " WHERE (Element.Id=? AND Kind=? AND SourceId=?)");
    sel->BindId(1, el.GetElementId());
    sel->BindText(2, SyncInfoAspect::KindToString(kind), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    sel->BindText(3, sourceId.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    bvector<BeSQLite::EC::ECInstanceId> ids;
    while (BE_SQLITE_ROW == sel->Step())
        ids.push_back(sel->GetValueId<BeSQLite::EC::ECInstanceId>(0));
    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ElementSyncInfoAspect SyncInfo::V8ElementSyncInfoAspect::Get(DgnElementR el, DgnV8Api::ElementId v8Id)
    {
    auto ids = SyncInfo::GetSyncInfoAspectIds(el, SyncInfoAspect::Element, FormatSourceId(v8Id));
    if (ids.size() == 0)
        return V8ElementSyncInfoAspect(nullptr);
    BeAssert(ids.size() == 1 && "Not supporting multiple element kind aspects on a single bim element from a given sourceId");
    return V8ElementSyncInfoAspect(GetAspect(el, ids.front()).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ElementSyncInfoAspect SyncInfo::V8ElementSyncInfoAspect::Get(DgnElementCR el, DgnV8Api::ElementId v8Id)
    {
    auto ids = SyncInfo::GetSyncInfoAspectIds(el, SyncInfoAspect::Element, FormatSourceId(v8Id));
    if (ids.size() == 0)
        return V8ElementSyncInfoAspect(nullptr);
    BeAssert(ids.size() == 1 && "Not supporting multiple element kind aspects on a single bim element from a given sourceId");
    return V8ElementSyncInfoAspect(GetAspect(el, ids.front()).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ModelSyncInfoAspect SyncInfo::V8ModelSyncInfoAspect::Get(DgnElementR el, DgnV8Api::ModelId v8Id)
    {
    auto ids = SyncInfo::GetSyncInfoAspectIds(el, SyncInfoAspect::Model, FormatSourceId(v8Id));
    if (ids.size() == 0)
        return V8ModelSyncInfoAspect(nullptr);
    BeAssert(ids.size() == 1 && "Not supporting multiple model kind aspects on a single bim element from a given sourceId");
    return V8ModelSyncInfoAspect(GetAspect(el, ids.front()).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ModelSyncInfoAspect SyncInfo::V8ModelSyncInfoAspect::Get(DgnElementCR el, DgnV8Api::ModelId v8Id)
    {
    auto ids = SyncInfo::GetSyncInfoAspectIds(el, SyncInfoAspect::Model, FormatSourceId(v8Id));
    if (ids.size() == 0)
        return V8ModelSyncInfoAspect(nullptr);
    BeAssert(ids.size() == 1 && "Not supporting multiple model kind aspects on a single bim element from a given sourceId");
    return V8ModelSyncInfoAspect(GetAspect(el, ids.front()).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ElementSyncInfoAspect SyncInfo::V8ElementSyncInfoAspect::Make(V8ElementSyncInfoAspectData const& provdata, DgnDbR db) 
    {
    auto aspectClass = GetAspectClass(db);
    if (nullptr == aspectClass)
        return V8ElementSyncInfoAspect(nullptr);

    iModelSyncInfoAspect::SourceState ss;
    unsigned arraySize = sizeof(provdata.m_prov.m_hash.m_buffer) / sizeof(unsigned char);
    ss.m_hash.insert(ss.m_hash.end(), provdata.m_prov.m_hash.m_buffer, &provdata.m_prov.m_hash.m_buffer[arraySize]);

    ss.m_lastModifiedTime = provdata.m_prov.m_lastModified;
    auto instance = MakeInstance(DgnElementId(provdata.m_scope.GetValue()), KindToString(Kind::Element), FormatSourceId(provdata.m_v8Id), &ss, *aspectClass);
    return V8ElementSyncInfoAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::ElementId SyncInfo::V8ElementSyncInfoAspect::GetV8ElementId() const
    {
    int64_t id = 0;
    sscanf(GetSourceId(), "%lld", &id);
    return id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ModelSyncInfoAspect SyncInfo::V8ModelSyncInfoAspect::Make(DgnV8ModelCR v8Model, TransformCR transform, Converter& converter) 
    {
    auto aspectClass = GetAspectClass(converter.GetDgnDb());
    if (nullptr == aspectClass)
        return V8ModelSyncInfoAspect(nullptr);
    
    DgnElementId repositoryLinkId = converter.GetRepositoryLinkFromAppData(*v8Model.GetDgnFileP());
    auto instance = MakeInstance(repositoryLinkId, KindToString(Kind::Model), FormatSourceId(v8Model), nullptr, *aspectClass);
    
    V8ModelSyncInfoAspect aspect(instance.get());
    
    Utf8String v8ModelName(v8Model.GetModelName());

    rapidjson::Document json(rapidjson::kObjectType);
    auto& allocator = json.GetAllocator();
    json.AddMember("transform", fixedArrayToJson((double*)&transform, 12, allocator), allocator);
    json.AddMember("v8ModelName", rapidjson::Value(v8ModelName.c_str(), allocator), allocator);
    aspect.SetProperties(json);

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Transform SyncInfo::V8ModelSyncInfoAspect::GetTransform() const
    {
    auto json = GetProperties();
    Transform transform;
    fixedArrayFromJson((double*)&transform, 12, json["transform"].GetArray());
    return transform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SyncInfo::V8ModelSyncInfoAspect::GetV8ModelName() const
    {
    auto json = GetProperties();
    return json["v8ModelName"].GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::ModelId SyncInfo::V8ModelSyncInfoAspect::GetV8ModelId() const
    {
    return atoi(GetSourceId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef TEST_SYNC_INFO_ASPECT
void SyncInfo::V8ModelSyncInfoAspect::AssertMatch(V8ModelMapping const& mapping)
    {
    // BeAssert(GetScope().GetValue() == ... TODO: must be a repository link element
    BeAssert(GetV8ModelId() == mapping.GetV8ModelId().GetValue());
    BeAssert(GetKind() == SyncInfoAspect::Kind::Model);
    BeAssert(GetTransform().IsEqual(mapping.GetTransform(), Angle::SmallAngle(), 1.0e-5));
    BeAssert(GetV8ModelName().Equals(mapping.GetV8Name()));
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::GeomPartSyncInfoAspect SyncInfo::GeomPartSyncInfoAspect::Make(Utf8StringCR tag, DgnGeometryPartId partId, DgnDbR db)
    {
    auto aspectClass = GetAspectClass(db);
    if (nullptr == aspectClass)
        return GeomPartSyncInfoAspect(nullptr);
                
    auto instance = iModelSyncInfoAspect::MakeInstance(partId, KindToString(Kind::GeomPart), tag, nullptr, *aspectClass);
    return GeomPartSyncInfoAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::GeomPartSyncInfoAspect SyncInfo::GeomPartSyncInfoAspect::GetByTag(DgnElementCR el, Utf8StringCR tag)
    {
    auto ids = SyncInfo::GetSyncInfoAspectIds(el, SyncInfoAspect::Kind::GeomPart, tag);
    if (ids.size() == 0)
        return GeomPartSyncInfoAspect(nullptr);
    BeAssert(ids.size() == 1 && "Not supporting multiple GeomPart kind aspects on a single bim element for a given tag");
    return GeomPartSyncInfoAspect(GetAspect(el, ids.front()).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::GeomPartSyncInfoAspect SyncInfo::GeomPartSyncInfoAspect::GetByPartId(DgnElementCR el, DgnGeometryPartId partId)
    {
    auto sel = el.GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId from " SOURCEINFO_ECSCHEMA_NAME "." SOURCEINFO_CLASS_SoureElementInfo " WHERE (Element.Id=? AND Kind=? AND Scope.Id=?)");
    sel->BindId(1, el.GetElementId());
    sel->BindText(2, SyncInfoAspect::KindToString(SyncInfoAspect::Kind::GeomPart), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    sel->BindId(3, partId);
    if (BE_SQLITE_ROW != sel->Step())
        return GeomPartSyncInfoAspect(nullptr);
    
    auto aspectId = sel->GetValueId<BeSQLite::EC::ECInstanceId>(0);
    
    BeAssert((BE_SQLITE_ROW != sel->Step()) && "Not supporting multiple GeomPart kind aspects on a single bim element for a given partId");

    return GeomPartSyncInfoAspect(GetAspect(el, aspectId).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::SyncInfoAspect::Kind SyncInfo::SyncInfoAspect::GetKind() const 
    {
    return ParseKind(iModelSyncInfoAspect::GetKind());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::OnAttach(DgnDb& project)
    {
    m_dgndb = &project;

    importElementAspectSchema(*m_dgndb);

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
    if (PerformVersionChecks() != BSISUCCESS)
        return BSIERROR;

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
        || m_dgndb->QueryProperty(currentProjectDbProfileVersion, Properties::ProfileVersion()) != BE_SQLITE_ROW
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

    CreateTables();  // We STILL call CreateTables. That gives EC a chance to create its TEMP tables.
    ValidateViewTable();

    SetValid(true);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::CreateEmptyFile(BeFileNameCR fileName, bool deleteIfExists)
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
void SyncInfo::SetLastError(BeSQLite::DbResult rc)
    {
    m_lastError = rc;
    m_lastErrorDescription = m_dgndb->GetLastError();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::GetLastError(BeSQLite::DbResult& result, Utf8String& descr)
    {
    result = m_lastError;
    descr = m_lastErrorDescription;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2014
//---------------------------------------------------------------------------------------
DbResult SyncInfo::InsertECSchema(BentleyApi::ECN::ECSchemaId& insertedSchemaId, DgnV8FileR v8File,
                                  Utf8CP v8SchemaName, uint32_t v8ProfileVersionMajor, uint32_t v8ProfileVersionMinor,
                                  bool isDynamic, uint32_t checksum) const
    {
    insertedSchemaId;

    BeAssert(checksum != 0);

    V8FileSyncInfoId v8FileId = Converter::GetV8FileSyncInfoIdFromAppData(v8File);
    if (!v8FileId.IsValid() || Utf8String::IsNullOrEmpty(v8SchemaName))
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_dgndb->GetCachedStatement(stmt, "INSERT OR REPLACE INTO " SYNCINFO_ATTACH(SYNC_TABLE_ECSchema)
                                                    " (V8FileSyncInfoId,V8Name,V8VersionMajor,V8VersionMinor,MappingType,LastModified,Digest) VALUES (?,?,?,?,?,?,?)"))
        {
        BeAssert(false && "Could not retrieve cached SyncInfo statement.");
        return BE_SQLITE_ERROR;
        }

    stmt->BindInt(1, v8FileId.GetValue());
    stmt->BindText(2, v8SchemaName, Statement::MakeCopy::No);
    stmt->BindInt(3, v8ProfileVersionMajor);
    stmt->BindInt(4, v8ProfileVersionMinor);
    stmt->BindInt(5, (int) (isDynamic ? ECSchemaMappingType::Dynamic : ECSchemaMappingType::Identity));

    double nowJd = -1.0;
    if (DateTime::GetCurrentTimeUtc().ToJulianDay(nowJd) != SUCCESS)
        {
        BeAssert(false && "Failed to convert current date time to Julian Day.");
        return BE_SQLITE_ERROR;
        }

    stmt->BindDouble(6, nowJd);
    stmt->BindInt(7, checksum);

    if (BE_SQLITE_DONE != stmt->Step())
        return BE_SQLITE_ERROR;

    insertedSchemaId = BECN::ECSchemaId((uint64_t) m_dgndb->GetLastInsertRowId());
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
bool SyncInfo::TryGetECSchema(ECObjectsV8::SchemaKey& schemaKey, ECSchemaMappingType& mappingType, Utf8CP v8SchemaName, V8FileSyncInfoId fileId) const
    {
    //first check whether we need to capture this schema or not
    CachedStatementPtr stmt = nullptr;
    Utf8String sql("SELECT V8VersionMajor, V8VersionMinor, Digest, MappingType FROM "
                  SYNCINFO_ATTACH(SYNC_TABLE_ECSchema)
                  " WHERE V8Name=?");
    if (fileId.IsValid())
        sql.append("and V8FileSyncInfoId = ? ");

    if (BE_SQLITE_OK != m_dgndb->GetCachedStatement(stmt, sql.c_str()))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindText(1, v8SchemaName, Statement::MakeCopy::No);
    if (fileId.IsValid())
        stmt->BindInt(2, fileId.GetValue());
    if (BE_SQLITE_ROW != stmt->Step())
        return false;

    schemaKey.m_schemaName = WString(v8SchemaName).c_str();
    schemaKey.m_versionMajor = (uint32_t) stmt->GetValueInt(0);
    schemaKey.m_versionMinor = (uint32_t) stmt->GetValueInt(1);
    schemaKey.m_checkSum = (uint32_t) stmt->GetValueInt(2);
    mappingType = (ECSchemaMappingType) stmt->GetValueInt(3);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2014
//---------------------------------------------------------------------------------------
bool SyncInfo::ContainsECSchema(Utf8CP v8SchemaName) const
    {
    CachedStatementPtr stmt = nullptr;
    auto stat = m_dgndb->GetCachedStatement(stmt, "SELECT NULL FROM " SYNCINFO_ATTACH(SYNC_TABLE_ECSchema) " WHERE V8Name=?");
    if (BE_SQLITE_OK != stat)
        {
        BeAssert(false && "Could not retrieve cached SyncInfo statement.");
        return BE_SQLITE_ERROR;
        }

    stmt->BindText(1, v8SchemaName, Statement::MakeCopy::No);
    return stmt->Step() == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
DbResult SyncInfo::RetrieveECSchemaChecksums(bmap<Utf8String, uint32_t>& syncInfoChecksums, V8FileSyncInfoId fileId) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_dgndb->GetCachedStatement(stmt, "SELECT V8Name, Digest FROM " SYNCINFO_ATTACH(SYNC_TABLE_ECSchema) " WHERE V8FileSyncInfoId=?"))
        {
        BeAssert(false && "Could not retrieve cached SyncInfo statement.");
        return BE_SQLITE_ERROR;
        }

    stmt->BindInt(1, fileId.GetValue());
    while (BE_SQLITE_ROW == stmt->Step())
        {
        syncInfoChecksums[stmt->GetValueText(0)] = (uint32_t) stmt->GetValueInt(1);
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      04/15
//---------------------------------------------------------------------------------------
bool SyncInfo::V8ModelSource::operator<(V8ModelSource const& rhs) const
    {
    if (m_v8FileSyncInfoId < rhs.m_v8FileSyncInfoId)
        return true;
    if (m_v8FileSyncInfoId > rhs.m_v8FileSyncInfoId)
        return false;
    return m_modelId.GetValue() < rhs.m_modelId.GetValue();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::CreateNamedGroupTable(bool createIndex)
    {
    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups), "SourceId INTEGER NOT NULL, TargetId INTEGER NOT NULL");
    if (createIndex)
        MUSTBEOK(m_dgndb->ExecuteSql("CREATE UNIQUE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) "_ng_uix ON " SYNC_TABLE_NamedGroups "(SourceId, TargetId);"));

    return BentleyApi::SUCCESS;
    }

#define SYNC_TABLE_master SYNCINFO_ATTACH("sqlite_master")
#define TEMPTABLE_ATTACH(name) "temp." name

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::CheckNamedGroupTable()
    {
    Statement stmt;
    Utf8PrintfString query("SELECT name, tbl_name FROM %s WHERE type='table' AND name='%s'", SYNC_TABLE_master, SYNC_TABLE_NamedGroups);

    if (BE_SQLITE_OK != stmt.Prepare(*m_dgndb, query.c_str()))
        return BentleyApi::ERROR;

    if (BE_SQLITE_ROW == stmt.Step())
        {
        // create the temp table for storing new entries
        MUSTBEOK(m_dgndb->ExecuteSql("CREATE TABLE " TEMPTABLE_ATTACH(SYNC_TABLE_NamedGroups) " (SourceId INTEGER NOT NULL, TargetId INTEGER NOT NULL);"));
        return SUCCESS;
        }

    // If we didn't find an existing NamedGroups table, that means this is an update using an older syncinfo database.  We need to create a new NamedGroups table and populate it
    // with existing named group members.
    if (BentleyApi::SUCCESS != CreateNamedGroupTable(false))
        return BentleyApi::ERROR;

    Utf8CP sql = "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) "(SourceId, TargetId) SELECT SourceId, TargetId from bis_ElementRefersToElements b, ec_Class e, ec_Schema s where b.ECClassId = e.Id and e.Name='ElementGroupsMembers' and e.SchemaId = s.Id and s.Name='BisCore'";
    Statement groups;
    if (BE_SQLITE_OK != groups.Prepare(*m_dgndb, sql))
        return BentleyApi::ERROR;

    if (BE_SQLITE_DONE != groups.Step())
        {
        return ERROR;
        }
    MUSTBEOK(m_dgndb->ExecuteSql("CREATE UNIQUE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) "_ng_uix ON " SYNC_TABLE_NamedGroups "(SourceId, TargetId);"));

    // create the temp table for storing new entries
    MUSTBEOK(m_dgndb->ExecuteSql("CREATE TABLE " TEMPTABLE_ATTACH(SYNC_TABLE_NamedGroups) " (SourceId INTEGER NOT NULL, TargetId INTEGER NOT NULL);"));

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool SyncInfo::IsElementInNamedGroup(DgnElementId sourceId, DgnElementId targetId)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "SELECT 1 FROM " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) " WHERE SourceId=? AND TargetId=?");
    if (!stmt.IsValid())
        return BentleyApi::ERROR;

    stmt->BindId(1, sourceId);
    stmt->BindId(2, targetId);

    return (BE_SQLITE_ROW == stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::AddNamedGroupEntry(DgnElementId sourceId, DgnElementId targetId)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "INSERT INTO " TEMPTABLE_ATTACH(SYNC_TABLE_NamedGroups) " (SourceId, TargetId) VALUES(?, ?)");
    if (!stmt.IsValid())
        return BentleyApi::ERROR;

    stmt->BindId(1, sourceId);
    stmt->BindId(2, targetId);
    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;
    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::FinalizeNamedGroups()
    {
    MUSTBEOK(m_dgndb->ExecuteSql("DROP INDEX " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) "_ng_uix"));

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(*m_dgndb, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) " (SourceId, TargetId) SELECT SourceId, TargetId FROM " TEMPTABLE_ATTACH(SYNC_TABLE_NamedGroups)))
        return BentleyApi::ERROR;

    if (BE_SQLITE_DONE != stmt.Step())
        return ERROR;

    MUSTBEOK(m_dgndb->ExecuteSql("CREATE UNIQUE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) "_ng_uix ON " SYNC_TABLE_NamedGroups "(SourceId, TargetId);"));
    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
void SyncInfo::ValidateViewTable()
    {
    if (!m_dgndb->TableExists(SYNCINFO_ATTACH(SYNC_TABLE_View)))
        {
        m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_View),
                             "ElementId BIGINT NOT NULL, "
                             "V8FileSyncInfoId INTEGER NOT NULL, "
                             "V8ElementId BIGINT, "
                             "V8ViewName TEXT, "
                             "LastModified REAL");
        return;
        }

    // Since it was created late, and then a new column was added need to ensure both that the table exists and the column exists
    if (m_dgndb->ColumnExists(SYNCINFO_ATTACH(SYNC_TABLE_View), "V8ViewName"))
        return;

    m_dgndb->AddColumnToTable(SYNCINFO_ATTACH(SYNC_TABLE_View), "V8ViewName", "TEXT");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
BeSQLite::DbResult SyncInfo::InsertView(DgnViewId viewId, DgnV8ViewInfoCR viewInfo, Utf8CP viewName)
    {
    Statement stmt;
    stmt.Prepare(*m_dgndb, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_View) "(ElementId, V8FileSyncInfoId, V8ElementId, V8ViewName, LastModified) VALUES (?, ?,?,?,?)");
    int col = 1;
    stmt.BindId(col++, viewId);

    ElementRefP      viewElemRef = viewInfo.GetElementRef();
    if (nullptr == viewElemRef)
        return DbResult::BE_SQLITE_NOTFOUND;

    V8FileSyncInfoId v8FileId = Converter::GetV8FileSyncInfoIdFromAppData(*viewElemRef->GetDgnModelP()->GetDgnFileP());
    if (!v8FileId.IsValid())
        return BeSQLite::DbResult::BE_SQLITE_ERROR_FileNotFound;

    stmt.BindInt(col++, v8FileId.GetValue());
    stmt.BindInt64(col++, viewElemRef->GetElementId());
    stmt.BindText(col++, viewName, Statement::MakeCopy::Yes);
    stmt.BindDouble(col++, viewElemRef->GetLastModified());
    auto res = stmt.Step();
    return res;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool SyncInfo::TryFindView(DgnViewId& viewId, double& lastModified, Utf8StringR v8ViewName, DgnV8ViewInfoCR viewInfo) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_dgndb->GetCachedStatement(stmt, "SELECT ElementId, V8ViewName, LastModified FROM "
                                                    SYNCINFO_ATTACH(SYNC_TABLE_View)
                                                    " WHERE V8FileSyncInfoId=? AND V8ElementId=?"))
        {
        BeAssert(false);
        return false;
        }

    ElementRefP      viewElemRef = viewInfo.GetElementRef();
    V8FileSyncInfoId v8FileId = Converter::GetV8FileSyncInfoIdFromAppData(*viewElemRef->GetDgnModelP()->GetDgnFileP());
    if (!v8FileId.IsValid())
        return false;
    stmt->BindInt(1, v8FileId.GetValue());
    stmt->BindInt64(2, viewElemRef->GetElementId());
    DbResult rc = stmt->Step();
    if (BE_SQLITE_ROW != rc)
        return false;

    viewId = stmt->GetValueId<DgnViewId>(0);
    v8ViewName = stmt->GetValueText(1);
    lastModified = stmt->GetValueDouble(2);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
BeSQLite::DbResult SyncInfo::DeleteView(DgnViewId viewId)
    {
    Statement stmt;
    stmt.Prepare(*m_dgndb, "DELETE FROM " SYNCINFO_ATTACH(SYNC_TABLE_View) " WHERE ElementId=?");
    stmt.BindId(1, viewId);
    return stmt.Step();

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
BeSQLite::DbResult SyncInfo::UpdateView(DgnViewId viewId, Utf8CP v8ViewName, DgnV8ViewInfoCR viewInfo)
    {
    Statement stmt;
    stmt.Prepare(*m_dgndb, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_View) " SET LastModified=?, V8ViewName=? WHERE(ElementId=?)");
    int col = 1;
    stmt.BindDouble(col++, viewInfo.GetElementRef()->GetLastModified());
    stmt.BindText(col++, v8ViewName, Statement::MakeCopy::Yes);
    stmt.BindId(col++, viewId);
    return stmt.Step();
    }

DgnViewId SyncInfo::ViewIterator::Entry::GetId() { return m_sql->GetValueId<DgnViewId>(0); }
SyncInfo::V8FileSyncInfoId SyncInfo::ViewIterator::Entry::GetV8FileSyncInfoId() { return SyncInfo::V8FileSyncInfoId(m_sql->GetValueInt(1)); }
uint64_t SyncInfo::ViewIterator::Entry::GetV8ElementId() { return m_sql->GetValueInt64(2); }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::ViewIterator::ViewIterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);
    Utf8String sqlString = MakeSqlString("SELECT ElementId, V8FileSyncInfoId, V8ElementId, LastModified FROM " SYNCINFO_ATTACH(SYNC_TABLE_View));
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::ViewIterator::Entry SyncInfo::ViewIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool SyncInfo::EnsureImageryTableExists()
    {
    if (m_dgndb->TableExists(SYNCINFO_ATTACH(SYNC_TABLE_Imagery)))
        return true;

    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Imagery),
                         "ElementId BIGINT PRIMARY KEY, "
                         "Filename TEXT NOT NULL,"
                         "LastModified BIGINT,"
                         "FileSize BIGINT,"
                         "ETag TEXT,"
                         "RDSId TEXT");
    m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Imagery) "ElementIdx ON "  SYNC_TABLE_Level "(ElementId)");

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2018
//---------------+---------------+---------------+---------------+---------------+-------
BeSQLite::DbResult SyncInfo::InsertImageryFile(DgnElementId modeledElementId, V8FileSyncInfoId filesiid, Utf8CP filename, uint64_t lastModifiedTime, uint64_t fileSize, Utf8CP etag, Utf8CP rdsId)
    {
    EnsureImageryTableExists();

    Statement stmt;
    stmt.Prepare(*m_dgndb, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Imagery) "(ElementId, V8FileSyncInfoId, Filename, LastModified, FileSize, ETag, RDSId) VALUES (?,?,?,?,?,?,?)");
    int col = 1;
    stmt.BindId(col++, modeledElementId);
    stmt.BindInt(col++, filesiid.GetValue());
    stmt.BindText(col++, filename, Statement::MakeCopy::No);
    stmt.BindUInt64(col++, lastModifiedTime);
    stmt.BindUInt64(col++, fileSize);
    stmt.BindText(col++, etag, Statement::MakeCopy::No);
    stmt.BindText(col++, rdsId, Statement::MakeCopy::No);
    auto res = stmt.Step();
    return res;
    }

static bool isHttp(Utf8CP str) { return (0 == strncmp("http:", str, 5) || 0 == strncmp("https:", str, 6)); }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2018
//---------------+---------------+---------------+---------------+---------------+-------
void SyncInfo::GetCurrentImageryInfo(Utf8StringCR fileName, uint64_t& currentLastModifiedTime, uint64_t& currentFileSize, Utf8StringR currentEtag)
    {
    BeFileName beFile(fileName.c_str());
    if (beFile.DoesPathExist())
        {
        time_t mtime = 0;
        uint64_t tempFileSize;

        if (BeFileName::GetFileSize(tempFileSize, beFile) != BeFileNameStatus::Success
            || BeFileName::GetFileTime(nullptr, nullptr, &mtime, beFile) != BeFileNameStatus::Success)
            {
            Utf8PrintfString msg("Unable to get file info for '%s'", fileName.c_str());
            //ReportIssue(Converter::IssueSeverity::Info, Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), msg.c_str());
            }
        currentLastModifiedTime = mtime;
        currentFileSize = tempFileSize;
        }
    else if (isHttp(fileName.c_str()))
        {
        BentleyApi::Http::Request request(fileName, "HEAD");
        folly::Future<BentleyApi::Http::Response> response = request.Perform().wait();
        if (BentleyApi::Http::HttpStatus::OK == response.value().GetHttpStatus())
            {
            auto headers = response.value().GetHeaders();
            Utf8String etag = headers.GetETag();
            currentEtag = etag;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool SyncInfo::ModelHasChangedImagery(V8FileSyncInfoId filesiid)
    {
    EnsureImageryTableExists();
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_dgndb->GetCachedStatement(stmt, "SELECT Filename, LastModified, FileSize, ETag FROM "
                                                    SYNCINFO_ATTACH(SYNC_TABLE_Imagery)
                                                    " WHERE V8FileSyncInfoId=?"))
        {
        BeAssert(false);
        return false;
        }

    if (!filesiid.IsValid())
        return false;

    stmt->BindInt(1, filesiid.GetValue());
    while (BE_SQLITE_ROW == stmt->Step())
        {
        Utf8String fileName = stmt->GetValueText(0);
        uint64_t lastModifiedTime = stmt->GetValueUInt64(1);
        uint64_t fileSize = stmt->GetValueUInt64(2);
        Utf8String etag = stmt->GetValueText(3);

        uint64_t currentModifiedTime = 0;
        uint64_t currentFileSize = 0;
        Utf8String currentEtag;
        GetCurrentImageryInfo(fileName, currentModifiedTime, currentFileSize, currentEtag);

        if (currentModifiedTime != lastModifiedTime || currentFileSize != fileSize || !currentEtag.Equals(etag))
            return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool SyncInfo::TryFindImageryFile(DgnElementId modeledElementId, Utf8StringR fileName, uint64_t& lastModifiedTime, uint64_t &fileSize, Utf8StringR etag, Utf8StringR rdsId)
    {
    EnsureImageryTableExists();
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_dgndb->GetCachedStatement(stmt, "SELECT Filename, LastModified, FileSize, ETag, RDSId FROM "
                                                    SYNCINFO_ATTACH(SYNC_TABLE_Imagery)
                                                    " WHERE ElementId=?"))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindId(1, modeledElementId);
    DbResult rc = stmt->Step();
    if (BE_SQLITE_ROW != rc)
        return false;

    int col = 0;
    fileName = stmt->GetValueText(col++);
    lastModifiedTime = stmt->GetValueUInt64(col++);
    fileSize = stmt->GetValueUInt64(col++);
    etag = stmt->GetValueText(col++);
    rdsId = stmt->GetValueText(col++);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2018
//---------------+---------------+---------------+---------------+---------------+-------
BeSQLite::DbResult SyncInfo::UpdateImageryFile(DgnElementId modeledElementId, uint64_t lastModifiedTime, uint64_t fileSize, Utf8CP etag, Utf8CP rdsId)
    {
    EnsureImageryTableExists();

    Statement stmt;
    stmt.Prepare(*m_dgndb, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_Imagery) " SET LastModified=?, FileSize=?, ETag=?, RDSId=? WHERE(ElementId=?)");
    int col = 1;
    stmt.BindUInt64(col++, lastModifiedTime);
    stmt.BindUInt64(col++, fileSize);
    stmt.BindText(col++, etag, Statement::MakeCopy::No);
    stmt.BindText(col++, rdsId, Statement::MakeCopy::No);
    stmt.BindId(col++, modeledElementId);
    return stmt.Step();

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getUrnFromFirstSource(Utf8String& urn, BeXmlNodeP file)
    {
    for (BeXmlNodeP sources = file->GetFirstChild(); sources != nullptr; sources = sources->GetNextSibling())
        {
        if (0 != strcmp(sources->GetName(), "Sources"))
            continue;

        for (BeXmlNodeP source = sources->GetFirstChild(); source != nullptr; source = source->GetNextSibling())
            {
            if (0 != strcmp(source->GetName(), "Source"))
                continue;

            Utf8String refId;
            if (BeXmlStatus::BEXML_Success == source->GetAttributeStringValue(refId, "RefId"))
                continue;

            Utf8String content;
            source->GetContent(content);
            if (iModelBridge::IsPwUrn(content))
                {
                urn = content;
                return BSISUCCESS;       // <<== we only want the first source
                }
            }
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getUrnFromLastTarget(Utf8String& urn, BeXmlNodeP file)
    {
    bool foundAny = false;
    for (BeXmlNodeP target = file->GetFirstChild(); target != nullptr; target = target->GetNextSibling())
        {
        if (0 == strcmp(target->GetName(), "Target"))
            {
            Utf8String content;
            target->GetContent(content);
            if (iModelBridge::IsPwUrn(content))
                {
                urn = content;
                // keep looking. we want the last target
                }
            }
        }
    return foundAny? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isFileExtraction(BeXmlNodeP file)
    {
    for (BeXmlNodeP type = file->GetFirstChild(); type != nullptr; type = type->GetNextSibling())
        {
        if (0 == strcmp(type->GetName(), "Type"))
            {
            Utf8String content;
            type->GetContent(content);
            return content.EqualsI("Extract");
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getPwUrn(Bentley::DgnPlatform::ProvenanceBlobR blob)
    {
    Utf8String urn;

    WCharCP provData = (WCharCP)blob.GetData();

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, provData);
    if (xmlDom == nullptr)
        return urn;

    // First, get doc urns from *sources*
    BeXmlNodeP root = xmlDom->GetRootElement();
    for (BeXmlNodeP files = root->GetFirstChild(); files != nullptr; files = files->GetNextSibling())
        {
        if (0 != strcmp(files->GetName(), "Files"))
            continue;
        for (BeXmlNodeP file = files->GetFirstChild(); file != nullptr; file = file->GetNextSibling())
            {
            if (0 != strcmp(file->GetName(), "File"))
                continue;

            getUrnFromFirstSource(urn, file);

            if (!isFileExtraction(file))
                getUrnFromLastTarget(urn, file);
            }
        }

    return urn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::GetPwUrnFromFileProvenance(DgnV8FileCR file)
    {
    auto provData = const_cast<DgnV8FileR>(file).ReadFileProvenance();
    if (!provData.IsValid())
        return "";
    return getPwUrn(*provData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::GetDocumentURNforFile(DgnV8FileCR file)
    {
    // We prefer a PW URN. 

    auto const& moniker = file.GetDocument().GetMoniker();  
    Utf8String monikerURN(moniker.ResolveURI().c_str());
    if (iModelBridge::IsPwUrn(monikerURN))
        return monikerURN;

    Utf8String docURN(GetParams().QueryDocumentURN(BeFileName(file.GetFileName().c_str())));
    if (iModelBridge::IsPwUrn(docURN))
        return docURN;

    auto provURN = Converter::GetPwUrnFromFileProvenance(file);
    if (iModelBridge::IsPwUrn(provURN))
        return provURN;

    // We fall back on the first URN that is available.
    return !monikerURN.empty()? monikerURN:
           !docURN.empty()?     docURN:
                                provURN;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid Converter::GetDocumentGUIDforFile(DgnV8FileCR file)
    {
    auto const& moniker = file.GetDocument().GetMoniker();  
    Utf8String monikerURN(moniker.ResolveURI().c_str());
    if (iModelBridge::IsPwUrn(monikerURN))
        return iModelBridge::ParseDocGuidFromPwUri(monikerURN);

    iModelBridgeDocumentProperties docProps;
    GetDocumentProperties(docProps, BeFileName(file.GetFileName().c_str()));
    if (!docProps.m_docGuid.empty())
        {
        BeGuid guid;
        if (BSISUCCESS == guid.FromString(docProps.m_docGuid.c_str()))
            return guid;
        }

    auto provURN = Converter::GetPwUrnFromFileProvenance(file);
    if (iModelBridge::IsPwUrn(provURN))
        return iModelBridge::ParseDocGuidFromPwUri(provURN);

    return BeGuid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::V8ElementSyncInfoAspect::Update(ElementProvenance const& prov)
    {
    SourceState ss;
    unsigned arraySize = sizeof(prov.m_hash.m_buffer) / sizeof(unsigned char);
    ss.m_hash.insert(ss.m_hash.end(), prov.m_hash.m_buffer, &prov.m_hash.m_buffer[arraySize]);
    ss.m_lastModifiedTime = prov.m_lastModified; 
    SetSourceState(ss); 
    }
END_DGNDBSYNC_DGNV8_NAMESPACE

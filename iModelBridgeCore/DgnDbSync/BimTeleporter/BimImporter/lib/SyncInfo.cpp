/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimImporter/lib/SyncInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Logging/bentleylogging.h>
#include <DgnPlatform/DgnPlatformApi.h>

#include "SyncInfo.h"
#include "BisJson1ImporterImpl.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

#undef LOG
#define LOG (*NativeLogging::LoggingManager::GetLogger(L"BimTeleporter"))

#define MUSTBEDBRESULT(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {SetLastError(rc); return BSIERROR;}}
#define MUSTBEOK(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_OK)
#define MUSTBEROW(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_ROW)
#define MUSTBEDONE(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_DONE)

#define MUSTBEDBRESULTRC(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {SetLastError(rc); return rc;}}
#define MUSTBEOKRC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_OK)
#define MUSTBEROWRC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_ROW)
#define MUSTBEDONERC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_DONE)

BEGIN_BIM_TELEPORTER_NAMESPACE

static ProfileVersion s_currentVersion(0, 1, 0, 0);

//---------------------------------------------------------------------------------------
// @bsimethod                                    Keith.Bentley                   03/11
//---------------+---------------+---------------+---------------+---------------+-------
DbResult SyncInfo::SavePropertyString(PropertySpecCR spec, Utf8CP stringData, uint64_t id, uint64_t subId)
    {
    Statement stmt;
    auto rc = stmt.Prepare(*m_dgnDb, "INSERT OR REPLACE INTO " SYNCINFO_ATTACH(BEDB_TABLE_Property) " (Namespace,Name,Id,SubId,TxnMode,StrData) VALUES(?,?,?,?,?,?)");
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Keith.Bentley                   12/10
//---------------+---------------+---------------+---------------+---------------+-------
DbResult SyncInfo::QueryProperty(Utf8StringR value, PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    Statement stmt;
    DbResult rc = stmt.Prepare(*m_dgnDb, "SELECT StrData FROM " SYNCINFO_ATTACH(BEDB_TABLE_Property) " WHERE Namespace=? AND Name=? AND Id=? AND SubId=?");
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
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::CreateTables()
    {
    if (nullptr == m_dgnDb)
        {
        BeAssert(false);
        return ERROR;
        }

    if (m_dgnDb->TableExists(SYNCINFO_ATTACH(SYNC_TABLE_File)))
        {
        return SUCCESS;
        }

    m_dgnDb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_File),
                         "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "FileName CHAR NOT NULL UNIQUE,"
                         "DbVersion CHAR NOT NULL,"
                         "ECVersion CHAR NOT NULL,"
                         "DgnPrjVersion CHAR NOT NULL,"
                         "LastModified BIGINT,"
                         "FileSize BIGINT");

    m_dgnDb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Model),
                         "TargetId INTEGER NOT NULL,"
                         "SourceId INTEGER PRIMARY KEY NOT NULL,"
                         "Name CHAR NOT NULL");
    m_dgnDb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Model) "SourceIdx ON "  SYNC_TABLE_Model "(SourceId)");


    m_dgnDb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_ECSchema),
                         "Id INTEGER PRIMARY KEY,"
                         "Name TEXT NOT NULL,"
                         "VersionMajor INTEGER NOT NULL,"
                         "VersionMinor INTEGER NOT NULL,"
                         "LastModified TIMESTAMP,"
                         "Checksum INTEGER");

    m_dgnDb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_Element),
                         "TargetId INTEGER NOT NULL,"
                         "SourceId INTEGER PRIMARY KEY NOT NULL");
    m_dgnDb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_Element) "SourceIdx ON "  SYNC_TABLE_Element "(SourceId)");

    m_dgnDb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_SubCategory),
                         "TargetId INTEGER NOT NULL,"
                         "SourceId INTEGER PRIMARY KEY NOT NULL,"
                         "CategoryId INTEGER");
    m_dgnDb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_SubCategory) "SourceIdx ON "  SYNC_TABLE_SubCategory "(SourceId,CategoryId)");

    //need a unique index to ensure uniqueness for schemas based on checksum
    Utf8String ddl;
    ddl.Sprintf("CREATE UNIQUE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_ECSchema) "_variantxml_uix ON "  SYNC_TABLE_ECSchema "(Name, Checksum);");
    MUSTBEOK(m_dgnDb->ExecuteSql(ddl.c_str()));
    //need a index on the entire table for fast look ups
    MUSTBEOK(m_dgnDb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_ECSchema) "_ix ON "  SYNC_TABLE_ECSchema "(Name);"));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::PerformVersionChecks()
    {
    Utf8String versionString;
    MUSTBEROW(QueryProperty(versionString, SyncInfoProperty::ProfileVersion()));

    ProfileVersion storedVersion(0, 0, 0, 0);
    storedVersion.FromJson(versionString.c_str());
    if (storedVersion.CompareTo(s_currentVersion) == 0)
        return SUCCESS;

    if (storedVersion.CompareTo(s_currentVersion) > 0)
        {
        LOG.errorv("compatibility error - storedVersion=%s > currentVersion=%s", versionString.c_str(), s_currentVersion.ToJson().c_str());
        return ERROR;
        }

    // Upgrade - when we change the syncInfo schema, add upgrade steps here ...

    // Upgraded.  Update the stored version
    MUSTBEOK(SavePropertyString(SyncInfoProperty::ProfileVersion(), s_currentVersion.ToString().c_str()));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson      07/14
//---------------+---------------+---------------+---------------+---------------+-------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::DbFileProvenance::DbFileProvenance(BentleyApi::BeFileNameCR dbFileName, Json::Value& fileInfo, SyncInfo& syncInfo) : m_syncInfo(syncInfo)
    {
    m_fileName = Utf8String(dbFileName.GetName());
    m_ecVersion.FromJson(fileInfo["ECSchemaVersion"].asString().c_str());
    m_dbVersion.FromJson(fileInfo["DbSchemaVersion"].asString().c_str());
    m_dgnPrjVersion.FromJson(fileInfo["DgnProjVersion"].asString().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
DbResult SyncInfo::DbFileProvenance::Insert()
    {
    Statement stmt;
    stmt.Prepare(*m_syncInfo.m_dgnDb, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_File) "(FileName, DbSchemaVersion, ECSchemaVersion, DgnPrjVersion, LastModified, FileSize) VALUES (?,?,?,?,?,?)");

    int col = 1;
    stmt.BindText(col++, m_fileName, Statement::MakeCopy::No);
    stmt.BindText(col++, m_dbVersion.ToJson(), Statement::MakeCopy::No);
    stmt.BindText(col++, m_ecVersion.ToJson(), Statement::MakeCopy::No);
    stmt.BindText(col++, m_dgnPrjVersion.ToJson(), Statement::MakeCopy::No);
    stmt.BindInt64(col++, m_lastModifiedTime);
    stmt.BindInt64(col++, m_fileSize);

    DbResult rc = stmt.Step();
    BeAssert(BE_SQLITE_DONE == rc);

    auto rowid = m_syncInfo.m_dgnDb->GetLastInsertRowId();
    BeAssert(rowid <= UINT32_MAX);
    m_syncId = DbFileId((uint32_t) rowid);

    return rc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
DbResult SyncInfo::DbFileProvenance::Update()
    {
    if (!FindByName(false))
        return BE_SQLITE_ERROR;

    Statement stmt;
    stmt.Prepare(*m_syncInfo.m_dgnDb, "UPDATE " SYNCINFO_ATTACH(SYNC_TABLE_File) " SET LastModified=?,FileSize=? WHERE Id=?");
    int col = 1;
    stmt.BindInt64(col++, m_lastModifiedTime);
    stmt.BindInt64(col++, m_fileSize);
    stmt.BindInt(col++, m_syncId.GetValue());
    return stmt.Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
bool SyncInfo::DbFileProvenance::FindByName(bool fillLastModData)
    {
    CachedStatementPtr stmt;
    m_syncInfo.m_dgnDb->GetCachedStatement(stmt, "SELECT Id,LastModified,FileSize FROM " SYNCINFO_ATTACH(SYNC_TABLE_File) " WHERE FileName=? ");
    stmt->BindText(1, m_fileName, Statement::MakeCopy::No);

    auto result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        {
        m_syncId = DbFileId();
        return false;
        }

    int col = 0;
    m_syncId = DbFileId(stmt->GetValueInt(col++));
    if (fillLastModData)
        {
        m_lastModifiedTime = stmt->GetValueInt64(col++);
        m_fileSize = stmt->GetValueInt64(col++);
        }
    return true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson      07/14
//---------------+---------------+---------------+---------------+---------------+-------
bool SyncInfo::HasDiskFileChanged(BeFileNameCR fileName, Json::Value& jsonFile)
    {
    SyncInfo::DiskFileInfo df;
    df.GetInfo(fileName);

    DbFileProvenance prov(fileName, jsonFile, *this);
    if (!prov.FindByName(true))
        return true;

    // This is an attempt to tell if a file has *not* changed, looking only at the file's time and size.
    // This is a dangerous test, since we don't look at the contents, but we think that we can narrow 
    // the odds of a mistake by:
    // 1. using times measured in hectonanoseconds (on Windows, at least),
    // 2. also using file size
    return df.m_lastModifiedTime != prov.m_lastModifiedTime || df.m_fileSize != prov.m_fileSize;
    }

SyncInfo::DbFileId SyncInfo::FileIterator::Entry::GetV8FileSyncInfoId() { return SyncInfo::DbFileId(m_sql->GetValueInt(0)); }
Utf8String SyncInfo::FileIterator::Entry::GetFileName() { return m_sql->GetValueText(1); }
ProfileVersion SyncInfo::FileIterator::Entry::GetDbVersion() { return ProfileVersion(m_sql->GetValueText(2)); }
ProfileVersion SyncInfo::FileIterator::Entry::GetECVersion() { return ProfileVersion(m_sql->GetValueText(3)); }
ProfileVersion SyncInfo::FileIterator::Entry::GetDgnPrjVersion() { return ProfileVersion(m_sql->GetValueText(4)); }
uint64_t SyncInfo::FileIterator::Entry::GetLastModifiedTime() { return m_sql->GetValueInt64(5); }
uint64_t SyncInfo::FileIterator::Entry::GetFileSize() { return m_sql->GetValueInt64(6); }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::FileIterator::FileIterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);
    Utf8String sqlString = MakeSqlString("SELECT Id,FileName,LastModified,FileSize FROM " SYNCINFO_ATTACH(SYNC_TABLE_File));
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::FileIterator::Entry SyncInfo::FileIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

int64_t SyncInfo::ModelIterator::Entry::GetRowId() { return m_sql->GetValueInt64(0); }
DgnModelId SyncInfo::ModelIterator::Entry::GetTargetId() { return m_sql->GetValueId<DgnModelId>(1); }
DgnModelId SyncInfo::ModelIterator::Entry::GetSourceId() { return m_sql->GetValueId<DgnModelId>(2); }
Utf8CP SyncInfo::ModelIterator::Entry::GetName() { return m_sql->GetValueText(3); }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::ModelIterator::ModelIterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);
    Utf8String sqlString = MakeSqlString("SELECT ROWId, TargetId, SourceId, Name FROM " SYNCINFO_ATTACH(SYNC_TABLE_Model));
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::ModelIterator::Entry SyncInfo::ModelIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
DbResult SyncInfo::ModelMapping::Insert(BeSQLite::Db& db) const
    {
    if (!m_targetId.IsValid())
        return BE_SQLITE_ERROR;

    Statement stmt;
    stmt.Prepare(db, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Model) " (TargetId, SourceId, Name) VALUES (?, ?, ?)");
    int col = 1;
    stmt.BindId(col++, m_targetId);
    stmt.BindId(col++, m_sourceId);
    stmt.BindText(col++, m_name, Statement::MakeCopy::No);

    auto rc = stmt.Step();
    if (BE_SQLITE_DONE == rc)
        m_rowId = db.GetLastInsertRowId();
    return rc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::ModelMapping::ModelMapping() : m_rowId(0)
    { }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::ModelMapping::ModelMapping(DgnModelId source, DgnModelId target, Utf8String name) : m_rowId(0), m_sourceId(source), m_targetId(target), m_name(name)
    { }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::InsertModel(ModelMapping& minfo, DgnModelId target, DgnModelId source, Utf8String name)
    {
    minfo = ModelMapping(source, target, name);

    auto rc = minfo.Insert(*m_dgnDb);
    return (BE_SQLITE_ROW == rc) ? SUCCESS : ERROR;
    }

int64_t SyncInfo::ElementIterator::Entry::GetRowId() { return m_sql->GetValueInt64(0); }
DgnElementId SyncInfo::ElementIterator::Entry::GetTargetId() { return m_sql->GetValueId<DgnElementId>(1); }
DgnElementId SyncInfo::ElementIterator::Entry::GetSourceId() { return m_sql->GetValueId<DgnElementId>(2); }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::ElementIterator::ElementIterator(DgnDbCR db, Utf8CP where) : BeSQLite::DbTableIterator(db)
    {
    m_params.SetWhere(where);
    Utf8String sqlString = MakeSqlString("SELECT ROWId, TargetId, SourceId FROM " SYNCINFO_ATTACH(SYNC_TABLE_Element));
    m_db->GetCachedStatement(m_stmt, sqlString.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::ElementIterator::Entry SyncInfo::ElementIterator::begin() const
    {
    m_stmt->Reset();
    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
DbResult SyncInfo::ElementMapping::Insert(BeSQLite::Db& db) const
    {
    if (!m_targetId.IsValid())
        return BE_SQLITE_ERROR;

    Utf8String sqlString("INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_Element) " (TargetId, SourceId) VALUES (?, ?)");
    CachedStatementPtr stmt = db.GetCachedStatement(sqlString.c_str());

    if (!stmt.IsValid())
        return BE_SQLITE_ERROR;

    int col = 1;
    stmt->BindId(col++, m_targetId);
    stmt->BindId(col++, m_sourceId);

    auto rc = stmt->Step();
    if (BE_SQLITE_DONE == rc)
        m_rowId = db.GetLastInsertRowId();
    return rc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::ElementMapping::ElementMapping() : m_rowId(0)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::ElementMapping::ElementMapping(DgnElementId source, DgnElementId target) : m_rowId(0), m_sourceId(source), m_targetId(target)
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::InsertCodeSpec(CodeSpecId oldId, CodeSpecId newId)
    {
    m_codeSpecMap[oldId] = newId;
    // WIP_CONVERTER -- write syncinfo 

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
CodeSpecId SyncInfo::LookupCodeSpec(CodeSpecId oldId)
    {
    // WIP_CONVERTER -- read syncInfo
    auto i = m_codeSpecMap.find(oldId);
    return i == m_codeSpecMap.end() ? CodeSpecId() : i->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::InsertFont(DgnFontId oldId, DgnFontId newId)
    {
    m_fontMap[oldId] = newId;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
DgnFontId SyncInfo::LookupFont(DgnFontId oldId)
    {
    auto fontId = m_fontMap.find(oldId);
    return fontId == m_fontMap.end() ? DgnFontId() : fontId->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::InsertStyle(DgnStyleId oldId, DgnStyleId newId)
    {
    m_styleMap[oldId] = newId;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
DgnStyleId SyncInfo::LookupStyle(DgnStyleId oldId)
    {
    auto styleId = m_styleMap.find(oldId);
    return styleId == m_styleMap.end() ? DgnStyleId() : styleId->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::InsertLsComponent(LsComponentId oldId, LsComponentId newId)
    {
    m_componentMap[oldId] = newId;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
LsComponentId SyncInfo::LookupLsComponent(LsComponentId oldId)
    {
    auto componentId = m_componentMap.find(oldId);
    return componentId == m_componentMap.end() ? LsComponentId() : componentId->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::InsertCategory(DgnCategoryId oldId, DgnCategoryId newId)
    {
    SyncInfo::ElementMapping map(oldId, newId);
    map.Insert(*GetDgnDb());

    m_categoryMap[oldId] = newId;
    // WIP_IMPORTER -- write syncinfo
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnCategoryId SyncInfo::LookupCategory(DgnCategoryId oldId)
    {
    // WIP_IMPORTER -- read syncinfo
    auto i = m_categoryMap.find(oldId);
    return i == m_categoryMap.end() ? DgnCategoryId() : i->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::InsertSubCategory(DgnCategoryId catId, DgnSubCategoryId oldId, DgnSubCategoryId newId)
    {
    Utf8String sqlString("INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_SubCategory) " (TargetId, SourceId, CategoryId) VALUES (?, ?, ?)");
    CachedStatementPtr stmt = m_dgnDb->GetCachedStatement(sqlString.c_str());

    if (!stmt.IsValid())
        return ERROR;

    int col = 1;
    stmt->BindId(col++, newId);
    stmt->BindId(col++, oldId);
    stmt->BindId(col++, catId);

    return stmt->Step() == BE_SQLITE_DONE ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnSubCategoryId SyncInfo::LookupSubCategory(DgnCategoryId catId, DgnSubCategoryId oldId)
    {
    Utf8String sqlString("SELECT TargetId FROM " SYNCINFO_ATTACH(SYNC_TABLE_SubCategory));
    sqlString.append(" WHERE SourceId=? AND CategoryId=?");
    CachedStatementPtr stmt = m_dgnDb->GetCachedStatement(sqlString.c_str());
    if (!stmt.IsValid())
        return DgnSubCategoryId();

    int col = 1;
    stmt->BindId(col++, oldId);
    stmt->BindId(col++, catId);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        return stmt->GetValueId<DgnSubCategoryId>(0);
        }
    return DgnSubCategoryId();

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId SyncInfo::LookupElement(DgnElementId oldId)
    {
    Utf8String sqlString("SELECT TargetId, SourceId FROM " SYNCINFO_ATTACH(SYNC_TABLE_Element));
    sqlString.append(" WHERE SourceId=?");
    CachedStatementPtr stmt = m_dgnDb->GetCachedStatement(sqlString.c_str());
    if (!stmt.IsValid())
        return DgnElementId();
    stmt->BindId(1, oldId);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (stmt->GetValueId<DgnElementId>(1) == oldId)
            return stmt->GetValueId<DgnElementId>(0);
        }
    return DgnElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnModelId SyncInfo::LookupModel(DgnModelId oldId)
    {
    Utf8String sqlString("SELECT TargetId, SourceId FROM " SYNCINFO_ATTACH(SYNC_TABLE_Model));
    sqlString.append(" WHERE SourceId=?");
    CachedStatementPtr stmt = m_dgnDb->GetCachedStatement(sqlString.c_str());
    if (!stmt.IsValid())
        return DgnModelId();
    stmt->BindId(1, oldId);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (stmt->GetValueId<DgnModelId>(1) == oldId)
            return stmt->GetValueId<DgnModelId>(0);
        }
    return DgnModelId();
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::AttachToProject(DgnDb& targetProject, BeFileNameCR dbName)
    {
    DbResult rc = targetProject.AttachDb(Utf8String(dbName).c_str(), SYNCINFO_ATTACH_ALIAS);
    if (BE_SQLITE_OK != rc)
        return ERROR;
    return OnAttach(targetProject);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::OnAttach(DgnDb& project)
    {
    m_dgnDb = &project;

    if (!m_dgnDb->TableExists(SYNCINFO_ATTACH(SYNC_TABLE_File)))
        {
        // We are creating a new syncinfo file
        Utf8String currentDbProfileVersion;
        m_dgnDb->QueryProperty(currentDbProfileVersion, Properties::ProfileVersion());

        MUSTBEOK(SavePropertyString(SyncInfoProperty::ProfileVersion(), s_currentVersion.ToJson().c_str()));
        MUSTBEOK(SavePropertyString(Properties::CreationDate(), DateTime::GetCurrentTimeUtc().ToString().c_str()));
        MUSTBEOK(SavePropertyString(SyncInfoProperty::DgnDbGuid(), m_dgnDb->GetDbGuid().ToString().c_str()));
        MUSTBEOK(SavePropertyString(SyncInfoProperty::DbProfileVersion(), currentDbProfileVersion.c_str()));
        MUSTBEOK(SavePropertyString(SyncInfoProperty::DgnDbProfileVersion(), m_dgnDb->GetProfileVersion().ToJson().c_str()));

        CreateTables();
        SetValid(true);
        return SUCCESS;
        }

    //  We are opening an existing syncinfo file
    if (SUCCESS != PerformVersionChecks())
        return ERROR;

    //  Check that this syncinfo goes with this project
    Utf8String projguidstr;
    BeSQLite::BeGuid projguid;
    if (QueryProperty(projguidstr, SyncInfoProperty::DgnDbGuid()) != BE_SQLITE_ROW
        || projguid.FromString(projguidstr.c_str()) != BSISUCCESS
        || m_dgnDb->GetDbGuid() != projguid)
        {
        LOG.errorv("GUID mismatch. syncinfo=%s projectguid=%s does not match project guid=%s",
                   m_dgnDb->GetDbFileName(), projguidstr.c_str(), m_dgnDb->GetDbGuid().ToString().c_str());
        return BSIERROR;
        }
    Utf8String savedProjectDbProfileVersion, currentProjectDbProfileVersion;
    if (QueryProperty(savedProjectDbProfileVersion, SyncInfoProperty::DbProfileVersion()) != BE_SQLITE_ROW
        || m_dgnDb->QueryProperty(currentProjectDbProfileVersion, Properties::ProfileVersion()) != BE_SQLITE_ROW
        || !savedProjectDbProfileVersion.Equals(currentProjectDbProfileVersion))
        {
        LOG.warningv("DB schema version mismatch. syncinfo=%s ProjectDbProfileVersion=%s does not match project ProfileVersion=%s.",
                     m_dgnDb->GetDbFileName(), savedProjectDbProfileVersion.c_str(), currentProjectDbProfileVersion.c_str());
        // *** WIP_CONVERTER - Do we really have to throw away project history whenever we make a trivial schema change?
        return BSISUCCESS;//BSIERROR; *** WIP_CONVERTER - support schema evolution 
        }

    Utf8String currentProjectProfileVersion = m_dgnDb->GetProfileVersion().ToJson();
    Utf8String savedProjectProfileVersion;
    if (QueryProperty(savedProjectProfileVersion, SyncInfoProperty::DgnDbProfileVersion()) != BE_SQLITE_ROW
        || !savedProjectProfileVersion.Equals(currentProjectProfileVersion))
        {
        LOG.warningv("project schema version mismatch. syncinfo=%s ProjectProfileVersion=%s does not match project ProjectProfileVersion=%s.",
                     m_dgnDb->GetDbFileName(), savedProjectProfileVersion.c_str(), currentProjectProfileVersion.c_str());
        // *** WIP_CONVERTER - Do we really have to throw away project history whenever we make a trivial schema change?
        return BSISUCCESS;//BSIERROR; *** WIP_CONVERTER - support schema evolution 
        }

    CreateTables();  // We STILL call CreateTables. That gives EC a chance to create its TEMP tables.

    SetValid(true);
    return SUCCESS;
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
    m_lastErrorDescription = m_dgnDb->GetLastError();
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
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::SyncInfo(BisJson1ImporterImpl& importer) : m_importer(importer), m_dgnDb(nullptr)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo::~SyncInfo()
    {
    if (m_dgnDb != nullptr)
        m_dgnDb->DetachDb(SYNCINFO_ATTACH_ALIAS);
    }

END_BIM_TELEPORTER_NAMESPACE
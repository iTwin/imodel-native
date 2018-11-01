/*--------------------------------------------------------------------------------------+
|
|     $Source: BimFromDgnDb/BimImporter/lib/SyncInfo.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Logging/bentleylogging.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <ECPresentation/RulesDriven/RuleSetEmbedder.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

#include "SyncInfo.h"
#include "BimFromJsonImpl.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_ECPRESENTATION

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

BEGIN_BIM_FROM_DGNDB_NAMESPACE

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

    ElementClassToAspectClassMapping::CreateTable(*m_dgnDb);

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
SyncInfo::SyncInfo(BimFromJsonImpl& importer) : m_importer(importer), m_dgnDb(nullptr)
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

#define ELEMENT_TO_ASPECT_MAPPING_TABLE SYNCINFO_TABLE("ElementClassToAspectClass")

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementClassToAspectClassMapping::CreateTable(DgnDbR db)
    {
    if (db.TableExists(SYNCINFO_ATTACH(ELEMENT_TO_ASPECT_MAPPING_TABLE)))
        return BSISUCCESS;
    if (BE_SQLITE_OK != db.ExecuteSql("CREATE TABLE " SYNCINFO_ATTACH(ELEMENT_TO_ASPECT_MAPPING_TABLE) " (ElementClassId INTEGER NOT NULL, ElementSchemaName TEXT NOT NULL, ElementClassName TEXT NOT NULL, AspectClassId INTEGER NOT NULL, AspectSchemaName TEXT NOT NULL, AspectClassName TEXT NOT NULL)"))
        return BSIERROR;
    db.ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(ELEMENT_TO_ASPECT_MAPPING_TABLE) "ElementClassIdx ON " ELEMENT_TO_ASPECT_MAPPING_TABLE "(ElementClassId)");
    db.ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(ELEMENT_TO_ASPECT_MAPPING_TABLE) "AspectClassIdx ON " ELEMENT_TO_ASPECT_MAPPING_TABLE "(AspectClassId)");
    db.ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(ELEMENT_TO_ASPECT_MAPPING_TABLE) "AspectSchemaNameIdx ON " ELEMENT_TO_ASPECT_MAPPING_TABLE "(AspectSchemaName)");
    db.ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(ELEMENT_TO_ASPECT_MAPPING_TABLE) "ElementSchemaNameIdx ON " ELEMENT_TO_ASPECT_MAPPING_TABLE "(ElementSchemaName)");
    db.ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(ELEMENT_TO_ASPECT_MAPPING_TABLE) "ElementClassNameIdx ON " ELEMENT_TO_ASPECT_MAPPING_TABLE "(ElementClassName)");
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool ElementClassToAspectClassMapping::TryFind(DgnDbR db, DgnClassId elementId, ECN::ECClassId aspectId)
    {
    CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT ElementSchemaName FROM " SYNCINFO_ATTACH(ELEMENT_TO_ASPECT_MAPPING_TABLE) " WHERE ElementClassId = ? AND AspectClassId=?");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement for ElementClassToAspectClassMapping::Find.");
        return false;
        }

    stmt->BindId(1, elementId);
    stmt->BindId(2, aspectId);
    if (stmt->Step() != BE_SQLITE_ROW)
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementClassToAspectClassMapping::Insert(DgnDbR db, DgnClassId elementId, Utf8CP elementSchemaName, Utf8CP elementClassName, ECN::ECClassId aspectId, Utf8CP aspectSchemaName, Utf8CP aspectClassName)
    {
    if (ElementClassToAspectClassMapping::TryFind(db, elementId, aspectId))
        return BSISUCCESS;

    CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "INSERT INTO " SYNCINFO_ATTACH(ELEMENT_TO_ASPECT_MAPPING_TABLE) " (ElementClassId, ElementSchemaName, ElementClassName, AspectClassId, AspectSchemaName, AspectClassName) VALUES (?, ?, ?, ?, ?, ?)");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement for ElementClassToAspectClassMapping::Insert");
        return BSIERROR;
        }

    stmt->BindId(1, elementId);
    stmt->BindText(2, elementSchemaName, Statement::MakeCopy::No);
    stmt->BindText(3, elementClassName, Statement::MakeCopy::No);
    stmt->BindId(4, aspectId);
    stmt->BindText(5, aspectSchemaName, Statement::MakeCopy::No);
    stmt->BindText(6, aspectClassName, Statement::MakeCopy::No);
    stat = stmt->Step();

    return stat == BE_SQLITE_DONE ? BSISUCCESS : BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2017
//---------------+---------------+---------------+---------------+---------------+-------
void ElementClassToAspectClassMapping::CreatePresentationRules(DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(db, "SELECT DISTINCT AspectSchemaName FROM " SYNCINFO_ATTACH(ELEMENT_TO_ASPECT_MAPPING_TABLE));
    Utf8String fileName(db.GetFileName().GetFileNameWithoutExtension().c_str());
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        Utf8CP aspectSchemaName(stmt.GetValueText(0));
        bvector<Utf8String> targets;
        {
        CachedStatementPtr stmt2 = nullptr;
        auto stat = db.GetCachedStatement(stmt2, "SELECT DISTINCT ElementSchemaName FROM " SYNCINFO_ATTACH(ELEMENT_TO_ASPECT_MAPPING_TABLE) " WHERE AspectSchemaName = ?");
        if (stat != BE_SQLITE_OK)
            {
            }
        stmt2->BindText(1, aspectSchemaName, Statement::MakeCopy::No);
        targets.push_back(Utf8String(aspectSchemaName));
        while (stmt2->Step() == BE_SQLITE_ROW)
            {
            targets.push_back(stmt2->GetValueText(0));
            }
        }
        Utf8String supported = BeStringUtilities::Join(targets, ",");
        Utf8PrintfString purpose("%s specific", aspectSchemaName);
        PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(fileName, 1, 0, true, purpose, supported, "", false);
        {
        CachedStatementPtr stmt3 = nullptr;
        auto stat = db.GetCachedStatement(stmt3, "SELECT DISTINCT ElementSchemaName, ElementClassName FROM " SYNCINFO_ATTACH(ELEMENT_TO_ASPECT_MAPPING_TABLE) " WHERE AspectSchemaName = ?");
        if (stat != BE_SQLITE_OK)
            {
            }
        stmt3->BindText(1, aspectSchemaName, Statement::MakeCopy::No);
        while (stmt3->Step() == BE_SQLITE_ROW)
            {
            Utf8CP elementSchemaName = stmt3->GetValueText(0);
            Utf8CP elementClassName = stmt3->GetValueText(1);
            ContentModifierP modifier = new ContentModifier(elementSchemaName, elementClassName);
            ruleset->AddPresentationRule(*modifier);
            CachedStatementPtr stmt4 = nullptr;
            db.GetCachedStatement(stmt4, "SELECT DISTINCT AspectClassName FROM " SYNCINFO_ATTACH(ELEMENT_TO_ASPECT_MAPPING_TABLE) " WHERE ElementSchemaName = ? AND ElementClassName = ? AND AspectSchemaName = ?");
            stmt4->BindText(1, elementSchemaName, Statement::MakeCopy::No);
            stmt4->BindText(2, elementClassName, Statement::MakeCopy::No);
            stmt4->BindText(3, aspectSchemaName, Statement::MakeCopy::No);
            while (stmt4->Step() == BE_SQLITE_ROW)
                {
                Utf8PrintfString fullName("%s:%s", aspectSchemaName, stmt4->GetValueText(0));
                modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "BisCore:ElementOwnsMultiAspects", fullName, "", RelationshipMeaning::SameInstance));
                }
            }

        }
        RuleSetEmbedder embedder(db);
        embedder.Embed(*ruleset);
        }
    }
END_BIM_FROM_DGNDB_NAMESPACE
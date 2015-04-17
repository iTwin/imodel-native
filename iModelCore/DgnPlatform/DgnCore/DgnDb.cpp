/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnDb.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

static WCharCP s_dgndbExt   = L".dgndb";

/*---------------------------------------------------------------------------------**//**
* used to check names saved in categories, models, etc.
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDbTable::IsValidName(Utf8StringCR name, Utf8CP invalidChars)
    {
    // empty names, names that start or end with space, or contain an invalid character are illegal.
    // NOTE: don't use isspace for test below - it is locale specific and finds the non-breaking-space (0xA0) when using Latin-8 locale.
    return !name.empty() && ' ' != *name.begin() && ' ' != *name.rbegin() && (Utf8String::npos == name.find_first_of(invalidChars));
    }

/*---------------------------------------------------------------------------------**//**
* replace invalid characters in a string with a substitute character
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTable::ReplaceInvalidCharacters (Utf8StringR str, Utf8CP invalidChars, Utf8Char r)
    {
    size_t i, iprev = 0;
    while ((i = str.find_first_of (invalidChars, iprev)) != Utf8String::npos)
        {
        str[i] = r;
        iprev = i+1;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDb::DgnDb() : m_schemaVersion(0,0,0,0), m_fonts(*this), m_colors(*this), m_categories(*this), m_domains(*this), m_styles(*this), m_views(*this),
                 m_geomParts(*this), m_units(*this), m_models(*this), m_items(*this), m_elements(*this), 
                 m_materials(*this), m_links(*this), m_ecsqlCache(50, "DgnDb")
    {
    m_txnManager = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::Destroy()
    {
    // delete dgnfile, causes all models, etc. to be freed.
    m_models.Empty();

    DELETE_AND_CLEAR(m_txnManager);

    m_ecsqlCache.Empty ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDb::~DgnDb()
    {
    Destroy();
    }

void DgnDb::_OnDbClose() {Destroy(); T_Super::_OnDbClose();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::_OnDbOpened()
    {
    DbResult rc = T_Super::_OnDbOpened();
    if (BE_SQLITE_OK != rc)
        return rc;

    rc = Domains().SyncWithSchemas();
    if (BE_SQLITE_OK != rc)
        return rc;

    m_units.Load();
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
ITxnManagerR DgnDb::GetTxnManager()
    {
    if (NULL == m_txnManager)
        m_txnManager = new ITxnManager(*this);

    return *m_txnManager;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   02/15
//+---------------+---------------+---------------+---------------+---------------+------
CachedECSqlStatementPtr DgnDb::GetPreparedECSqlStatement (Utf8CP ecsql) const
    {
    return m_ecsqlCache.GetPreparedStatement (*this, ecsql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::DoOpenDgnDb (BeFileNameCR projectNameIn, OpenParams const& params)
    {
    m_fileName.SetName(projectNameIn);
    m_fileName.SupplyDefaultNameParts (s_dgndbExt);

    DbResult stat = OpenBeSQLiteDb (m_fileName, params);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("Error %s opening [%s]", Db::InterpretDbResult(stat), m_fileName.GetNameUtf8().c_str());
        }

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr DgnDb::OpenDgnDb(DbResult* outResult, BeFileNameCR fileName, OpenParams const& openParams)
    {
    DbResult ALLOW_NULL_OUTPUT (status, outResult);
    bool wantReadonly = openParams.IsReadonly();

    BeFileName dbFileName(fileName);
    dbFileName.SupplyDefaultNameParts(s_dgndbExt);

    DgnDbPtr dgnDb = new DgnDb();

    status = dgnDb->DoOpenDgnDb(dbFileName, openParams);
    if (status != BE_SQLITE_OK)
        return nullptr;

    // SchemaUpgrade logic may call OpenParams::_ReopenForSchemaUpgrade changing the file
    // from Readonly to ReadWrite.  This changes it back to what the caller requested.
    if (!wantReadonly || openParams.IsReadonly())
        return dgnDb;

    dgnDb = new DgnDb(); // release old and create a new DgnDb
    OpenParams readonlyParams(openParams);
    readonlyParams.SetOpenMode(Db::OPEN_Readonly);
    status = dgnDb->DoOpenDgnDb(dbFileName, readonlyParams);
    return (status != BE_SQLITE_OK) ? nullptr : dgnDb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateNewDgnDb(BeFileNameCR boundFileName, CreateDgnDbParams const& params)
    {
    BeFileName projectFile(boundFileName);

    if (boundFileName.IsEmpty())
        {
        projectFile.SetNameUtf8 (BEDB_MemoryDb);
        }
    else
        {
        projectFile.SupplyDefaultNameParts (s_dgndbExt);
        if (params.m_overwriteExisting && BeFileName::DoesPathExist(projectFile))
            {
            if (BeFileNameStatus::Success != BeFileName::BeDeleteFile (projectFile))
                {
                LOG.errorv(L"Unable to create DgnDb because '%ls' cannot be deleted.", projectFile.GetName());
                return BE_SQLITE_ERROR_FileExists;
                }
            }
        }

    bool useSeedDb = !params.m_seedDb.empty();

    if (useSeedDb)
        {
        BeFileNameStatus status = BeFileName::BeCopyFile (params.m_seedDb.c_str(), projectFile);
        if (BeFileNameStatus::Success != status)
            return BE_SQLITE_ERROR_FileExists;
        }

    DbResult rc = CreateNewDb (projectFile, params.GetGuid(), params);
    if (BE_SQLITE_OK != rc)
        return rc;

    m_fileName.SetName (projectFile);

    rc = CreateProjectTables();
    if (BE_SQLITE_OK != rc)
        return rc;

    InitializeDgnDb(params);

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr DgnDb::CreateDgnDb(DbResult* result, BeFileNameCR projectFileName, CreateDgnDbParams const& params)
    {
    DbResult ALLOW_NULL_OUTPUT (stat, result);

    DgnDbPtr project = new DgnDb();
    stat = project->CreateNewDgnDb(projectFileName, params);

    return  (BE_SQLITE_OK==stat) ? project : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* Reclaims disk space by vacuuming the database
* @bsimethod                                                    KeithBentley    12/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFileStatus DgnDb::CompactFile()
    {
    if (1 < GetCurrentSavepointDepth())
        return  DGNFILE_ERROR_TransactionActive;

    Savepoint* savepoint = GetSavepoint(0);
    if (savepoint)
        savepoint->Commit();

    if (BE_SQLITE_OK != TryExecuteSql("VACUUM"))
        return  DGNFILE_ERROR_SQLiteError;
    
    if (savepoint)
        savepoint->Begin();

    return DGNFILE_STATUS_Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::GetNextServerIssuedId (BeServerIssuedId& value, Utf8CP tableName, Utf8CP colName, uint32_t minimumId)
    {
    // WIP - In the future, this should coordinate with a central authority to compute an ID.
    Statement stmt;
    stmt.Prepare (*this, SqlPrintfString ("SELECT max(%s) FROM %s", colName, tableName));

    DbResult result = stmt.Step();
    if (BE_SQLITE_ROW != result)
        {
        value.Invalidate();
        BeAssert (false);
        return result;
        }

    uint32_t newId = stmt.GetValueInt(0) + 1;
    if (newId < minimumId)
        newId = minimumId;

    value = BeServerIssuedId (newId);
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyles::DgnStyles(DgnDbR project) : DgnDbTable(project), m_lineStyles(nullptr), m_annotationTextStyles(nullptr), m_annotationFrameStyles(nullptr),
            m_annotationLeaderStyles(nullptr), m_textAnnotationSeeds(nullptr)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyles::~DgnStyles()
    {
    DELETE_AND_CLEAR(m_lineStyles);
    DELETE_AND_CLEAR(m_annotationTextStyles);
    DELETE_AND_CLEAR(m_annotationFrameStyles);
    DELETE_AND_CLEAR(m_annotationLeaderStyles);
    DELETE_AND_CLEAR(m_textAnnotationSeeds);
    }

DgnLineStyles& DgnStyles::LineStyles() {if (NULL == m_lineStyles) m_lineStyles = new DgnLineStyles(m_dgndb); return *m_lineStyles;}
DgnAnnotationTextStyles& DgnStyles::AnnotationTextStyles() {if (NULL == m_annotationTextStyles) m_annotationTextStyles = new DgnAnnotationTextStyles(m_dgndb); return *m_annotationTextStyles;}
DgnAnnotationFrameStyles& DgnStyles::AnnotationFrameStyles() {if (NULL == m_annotationFrameStyles) m_annotationFrameStyles = new DgnAnnotationFrameStyles(m_dgndb); return *m_annotationFrameStyles;}
DgnAnnotationLeaderStyles& DgnStyles::AnnotationLeaderStyles() {if (NULL == m_annotationLeaderStyles) m_annotationLeaderStyles = new DgnAnnotationLeaderStyles(m_dgndb); return *m_annotationLeaderStyles;}
DgnTextAnnotationSeeds& DgnStyles::TextAnnotationSeeds() {if (NULL == m_textAnnotationSeeds) m_textAnnotationSeeds = new DgnTextAnnotationSeeds(m_dgndb); return *m_textAnnotationSeeds;}



/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnProject.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include "DgnCoreLog.h"
#include <DgnPlatform/DgnCore/DgnMarkupProject.h>
#include <DgnPlatform/DgnCore/DgnFile.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationFrameStylePersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationLeaderStylePersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationTextStylePersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/TextAnnotationSeedPersistence.h>

static WCharCP s_dgndbExt   = L".dgndb";

/*---------------------------------------------------------------------------------**//**
* used to check names saved in levels, models, etc.
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnProjectTable::IsValidName(Utf8StringCR name, Utf8CP invalidChars)
    {
    // empty names, names that start or end with space, or contain an invalid character are illegal.
    // NOTE: don't use isspace for test below - it is locale specific and finds the non-breaking-space (0xA0) when using Latin-8 locale.
    return !name.empty() && ' ' != *name.begin() && ' ' != *name.rbegin() && (Utf8String::npos == name.find_first_of(invalidChars));
    }

/*---------------------------------------------------------------------------------**//**
* replace invalid characters in a string with a substitute character
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectTable::ReplaceInvalidCharacters (Utf8StringR str, Utf8CP invalidChars, Utf8Char r)
    {
    size_t i, iprev = 0;
    while ((i = str.find_first_of (invalidChars, iprev)) != Utf8String::npos)
        {
        str[i] = r;
        iprev = i+1;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnUnits::DgnUnits(DgnProjectR project) : m_project(project)
    {
    m_projectOrigin.Zero();
    m_extent = DRange3d::NullRange();
    m_azimuth = m_latitude = m_longitude = 0.0;
    m_geoOriginBasis.Zero();
    m_hasGeoOriginBasis = false;
    m_geoCoordWorkingAreaInMetersSquared = 16093.4 * 16093.4; // 10 miles square
    m_geoCoordWorkingAreaInDegrees = 0.01; // fudged at 1/10 of degree now (doesn't account for longitude change)
    m_hasCheckedForGCS = false;
    m_geoServices = NULL;
    m_gcs = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinates::DgnGCS* DgnUnits::GetDgnGCS()
    {
    if (!m_hasCheckedForGCS)
        {
        m_geoServices = T_HOST.GetGeoCoordinationAdmin()._GetServices();
        m_gcs = m_geoServices? m_geoServices->GetGCSFromProject (m_project): NULL;
        m_hasCheckedForGCS = true;
        }
    return m_gcs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnUnits::UorsFromLatLong (DPoint3dR outUors, GeoPointCR inLatLong)
    {
    if (NULL == GetDgnGCS())
        return BSIERROR;
    return m_geoServices->UorsFromLatLong (outUors, inLatLong, *m_gcs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnUnits::LatLongFromUors (GeoPointR outLatLong, DPoint3dCR inUors)
    {
    if (NULL == GetDgnGCS())
        return BSIERROR;
    return m_geoServices->LatLongFromUors (outLatLong, inUors, *m_gcs);
    }

static Utf8CP DGNPROPERTYJSON_ProjectOrigin = "projectOrigin";
static Utf8CP DGNPROPERTYJSON_OriginLatitude  = "originLatitude";
static Utf8CP DGNPROPERTYJSON_OriginLongitude = "originLongitude";
static Utf8CP DGNPROPERTYJSON_Azimuth         = "azimuth";
static Utf8CP DGNPROPERTYJSON_GeoOriginBasis = "geoOriginBasis";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFileStatus DgnUnits::Load()
    {
    Json::Value  jsonObj;
    Utf8String value;

    DbResult  result = m_project.QueryProperty (value, DgnProjectProperty::Units());
    if (BE_SQLITE_ROW != result || !Json::Reader::Parse(value, jsonObj))
        {
        BeAssert(false);
        return DGNPROJECT_ERROR_UnitsMissing;
        }

    JsonUtils::DPoint3dFromJson(m_projectOrigin, jsonObj[DGNPROPERTYJSON_ProjectOrigin]);

    m_latitude  = jsonObj[DGNPROPERTYJSON_OriginLatitude].asDouble();
    m_longitude = jsonObj[DGNPROPERTYJSON_OriginLongitude].asDouble();
    m_azimuth = jsonObj[DGNPROPERTYJSON_Azimuth].asDouble();
    if (jsonObj.isMember(DGNPROPERTYJSON_GeoOriginBasis))
        {
        JsonUtils::DPoint2dFromJson (m_geoOriginBasis, jsonObj[DGNPROPERTYJSON_GeoOriginBasis]);
        m_hasGeoOriginBasis = true;
        }


    if (BE_SQLITE_ROW != m_project.QueryProperty (&m_extent, sizeof(m_extent), DgnProjectProperty::Extents()))
        m_extent = DRange3d::NullRange();

    return  DGNFILE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnUnits::Save()
    {
    Json::Value jsonObj;

    JsonUtils::DPoint3dToJson(jsonObj[DGNPROPERTYJSON_ProjectOrigin], m_projectOrigin);

    jsonObj[DGNPROPERTYJSON_OriginLatitude]  = m_latitude;
    jsonObj[DGNPROPERTYJSON_OriginLongitude] = m_longitude;
    jsonObj[DGNPROPERTYJSON_Azimuth] = m_azimuth;

    if (m_hasGeoOriginBasis)
        JsonUtils::DPoint2dToJson (jsonObj[DGNPROPERTYJSON_GeoOriginBasis], m_geoOriginBasis);

    m_project.SavePropertyString(DgnProjectProperty::Units(), Json::FastWriter::ToString(jsonObj));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnUnits::SaveProjectExtents (DRange3dCR newExtents)
    {
    m_extent = newExtents;
    Json::Value jsonObj;
    JsonUtils::DRange3dToJson(jsonObj, m_extent);
    return m_project.SavePropertyString(DgnProjectProperty::Extents(), Json::FastWriter::ToString(jsonObj));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnUnits::LoadProjectExtents()
    {
    Json::Value  jsonObj;
    Utf8String value;

    if (BE_SQLITE_ROW == m_project.QueryProperty(value, DgnProjectProperty::Extents()) && Json::Reader::Parse(value, jsonObj))
        {
        JsonUtils::DRange3dFromJson (m_extent, jsonObj);
        return;
        }

    // we don't have or can't read the extents from the property, load from range tree.
    RTree3dBoundsTest bounds (m_project);
    Statement stmt;
    DbResult rc = stmt.Prepare (m_project, "SELECT 1 FROM " DGN_VTABLE_PrjRTree " WHERE ElementId MATCH rTreeMatch(1)");
    bounds.m_bounds.Invalidate();
    rc=bounds.StepRTree(stmt);
    BeAssert(rc==BE_SQLITE_DONE);
    bounds.m_bounds.ToRange(m_extent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d DgnUnits::GetProjectExtents()
    {
    if (m_extent.IsNull())
        LoadProjectExtents();

    return m_extent;
    }

#if defined (_MSC_VER)
    #pragma warning (disable:4355)
#endif // defined (_MSC_VER)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProject::DgnProject() : m_schemaVersion(0,0,0,0), m_fonts(*this), m_colors(*this), m_levels(*this), m_domains(this), m_styles(*this),
                           m_stamps(*this), m_units(*this), m_models(*this), m_stampQvElemMap(NULL)
    {
    m_txnManager = NULL;
    m_elementLoadedListeners = NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProject::Destroy()
    {
    ViewContext::DeleteSymbolStampMap(m_stampQvElemMap);
    m_stampQvElemMap = NULL;

    // delete dgnfile, causes all models, etc. to be freed.
    m_models.Empty();

    BeAssert(NULL == m_stampQvElemMap);
    DELETE_AND_CLEAR(m_txnManager);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProject::~DgnProject()
    {
    Destroy();
    }

void DgnProject::_OnDbClose() {Destroy(); T_Super::_OnDbClose();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnProject::_OnDbCreated(CreateParams const& params)
    {
    DbResult rc = T_Super::_OnDbCreated(params);
    return (BE_SQLITE_OK != rc) ? rc : CreateVirtualDbTables();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnProject::_OnDbOpened()
    {
    DbResult rc = T_Super::_OnDbOpened();
    Models().OnDbOpened();
    return (BE_SQLITE_OK != rc) ? rc : CreateVirtualDbTables();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult  DgnProject::CreateVirtualDbTables()
    {
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
ITxnManagerR DgnProject::GetTxnManager()
    {
    if (NULL == m_txnManager)
        m_txnManager = new ITxnManager(*this);

    return  *m_txnManager;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2014
//---------------------------------------------------------------------------------------
DgnProjectPtr DgnProject::OpenProjectUnchecked (DgnFileStatus* outResult, BeFileNameCR fileName, OpenParams const& openParams)
    {
    DgnFileStatus ALLOW_NULL_OUTPUT (status, outResult);

    BeFileName projectFileName(fileName);
    projectFileName.SupplyDefaultNameParts (s_dgndbExt);

    DgnProjectPtr boundProject = new DgnProject();

    status = boundProject->DoOpenProject(projectFileName, openParams);
    if ((status == DGNFILE_STATUS_Success))
        status = boundProject->LoadMyDgnFile(openParams, true);

    return (status == DGNFILE_STATUS_Success) ? boundProject : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectPtr DgnProject::OpenProject (DgnFileStatus* outResult, BeFileNameCR fileName, OpenParams const& openParams)
    {
    bool wantReadonly = openParams.IsReadonly();
    DgnProjectPtr boundProject = OpenProjectUnchecked(outResult, fileName, openParams);

    if (!boundProject.IsValid())
        return NULL;

    if (wantReadonly && !openParams.IsReadonly())
        {
        //  SchemaUpgrade logic may call OpenParams::_ReopenForSchemaUpgrade changing the file
        //  from Readonly to ReadWrite.  This changes it back to what the caller requested.
        boundProject = NULL;
        OpenParams& ncOpenParams = const_cast<OpenParams&>(openParams);
        ncOpenParams.SetOpenMode(Db::OPEN_Readonly);
        boundProject = OpenProjectUnchecked(outResult, fileName, ncOpenParams);
        BeAssert(openParams.IsReadonly());
        }

    return boundProject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFileStatus DgnProject::LoadMyDgnFile(OpenParams const& openParams, bool upgradeIfNecessary)
    {
    Utf8String versionString;
    DbResult rc = QueryProperty (versionString, DgnFileProperty::SchemaVersion());

    DgnVersion dgnFileVersion (10,0,0,0);
    if (BE_SQLITE_ROW == rc)
        dgnFileVersion.FromJson(versionString.c_str());

    DgnVersion currDgnFileVersion(DGNFILE_CURRENT_VERSION_Major, DGNFILE_CURRENT_VERSION_Minor, DGNFILE_CURRENT_VERSION_Sub1, DGNFILE_CURRENT_VERSION_Sub2);

    // see whether we're going to need to upgrade this DgnFile. If so, we must reopen the r/w Db BEFORE creating the dgnFile
    if (upgradeIfNecessary && (dgnFileVersion < currDgnFileVersion))
        {
        // this will reopen the file read/write if necessary
        if (!openParams._ReopenForSchemaUpgrade(*this))
            return DGNSCHEMA_STATUS_MismatchCantOpenForWrite;
        }

    // continue upgrading until we get to the current version.
    while (upgradeIfNecessary && (dgnFileVersion < currDgnFileVersion))
        {
        // at this point we know that the project file is opened r/w
        DgnFileStatus stat = openParams._DoDgnFileUpgrade(*this, dgnFileVersion);
        if (DGNFILE_STATUS_Success != stat)
            return  stat;

        SavePropertyString (DgnFileProperty::SchemaVersion(), dgnFileVersion.ToJson());
        SaveChanges();
        }

    m_units.Load();
    return  DGNFILE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFileStatus DgnProject::DoOpenProject (BeFileNameCR projectNameIn, OpenParams const& params)
    {
    m_fileName.SetName(projectNameIn);
    m_fileName.SupplyDefaultNameParts (s_dgndbExt);

    DbResult stat = OpenBeSQLiteDb (m_fileName, params);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv(L"Error %ls opening [%ls]", WString(Db::InterpretDbResult(stat), BentleyCharEncoding::Utf8).c_str(), m_fileName.GetName());
        switch (stat)
            {
            case BE_SQLITE_READONLY:                return DGNOPEN_STATUS_FileIsReadonly;
            case BE_SQLITE_NOTADB:                  return DGNPROJECT_ERROR_CorruptDatabase;
            case BE_SQLITE_SCHEMA:                  return DGNOPEN_STATUS_CorruptFile;
            case BE_SQLITE_ERROR_FileNotFound:      return DGNOPEN_STATUS_FileNotFound;
            case BE_SQLITE_ERROR_AlreadyOpen:       return DGNOPEN_STATUS_AlreadyOpen;
            case BE_SQLITE_BUSY:                    return DGNOPEN_STATUS_FileInUse;
            case BE_SQLITE_ERROR_NoPropertyTable:   return DGNPROJECT_ERROR_MissingPropertyTable;
            case BE_SQLITE_ERROR_BadDbSchema:       return DGNPROJECT_ERROR_WrongBeSQLiteSchemaVersion;
            case BE_SQLITE_ERROR_ProfileUpgradeFailed:return DGNSCHEMA_STATUS_UpgradeFailed;
            case BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite:
            case BE_SQLITE_ERROR_ProfileTooNewForReadWrite:return DGNSCHEMA_STATUS_MismatchCantOpenForWrite;
            case BE_SQLITE_ERROR_ProfileTooNew:     return DGNSCHEMA_STATUS_VersionTooNew;
            case BE_SQLITE_ERROR_ProfileTooOld:     return DGNSCHEMA_STATUS_VersionTooOld;
            }

        return  DGNFILE_STATUS_UnknownError;
        }

    Statement stmt;
    stmt.Prepare (*this, "SELECT Name,Version FROM " DGN_TABLE_Domain);
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnDomainCP domain = DgnDomainLoader::FindDomain (stmt.GetValueText(0));
        if (NULL == domain)
            {
            LOG.errorv(L"Error Missing Domain [%s]", stmt.GetValueText(0));
            continue;
            }
        Domains().AddLoadedDomain (*domain);
        }

    return  DGNFILE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFileStatus DgnProject::CreateNewProject (BeFileNameCR boundFileName, CreateProjectParams const& params)
    {
    BeFileName projectFile(boundFileName);

    if (NULL == boundFileName)
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
                LOG.errorv(L"Unable to create DgnProject because '%ls' cannot be deleted.", projectFile.GetName());
                return DGNPROJECT_ERROR_CantCreateProjectFile;
                }
            }
        }

    bool useSeedDb = !params.m_seedDb.empty();

    if (useSeedDb)
        {
        BeFileNameStatus status = BeFileName::BeCopyFile (params.m_seedDb.c_str(), projectFile);
        if (BeFileNameStatus::Success != status)
            return ConvertFileNameStatusToPathNameStatus(status);
        }

    if (BE_SQLITE_OK != CreateNewDb (projectFile, params.GetGuid(), params))
        return DGNPROJECT_ERROR_CantCreateProjectFile;

    m_fileName.SetName (projectFile);

    if (BE_SQLITE_OK != CreateProjectTables())
        return DGNPROJECT_ERROR_CorruptDatabase;

    InitializeProject (params);

    return DGNFILE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectPtr DgnProject::CreateProject (DgnFileStatus* result, BeFileNameCR projectFileName, CreateProjectParams const& params)
    {
    DgnFileStatus ALLOW_NULL_OUTPUT (stat, result);

    DgnProjectPtr project = new DgnProject();
    stat = project->CreateNewProject(projectFileName, params);

    return  (DGNFILE_STATUS_Success==stat) ? project : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDomains::AddElementHandler (Utf8CP domain, HandlerR handler)
    {
    WString name, descr;
    handler.GetTypeName(name,255);
    handler.GetTypeDescription(descr,255);

    Statement stmt;
    stmt.Prepare (*m_project, "INSERT INTO " DGN_TABLE_Handler " (Id,Domain,Name,Descr,Type) VALUES(?,?,?,?,\"elem\")");
    stmt.BindInt64 (1, handler.GetHandlerId().GetId());
    stmt.BindText(2, domain, Statement::MAKE_COPY_No);
    stmt.BindText(3, Utf8String(name), Statement::MAKE_COPY_Yes);   // string conversion + string copy
    stmt.BindText(4, Utf8String(descr), Statement::MAKE_COPY_Yes);  // string conversion + string copy

    DbResult status = stmt.Step();
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDomains::AddDomainToProject (DgnDomainCR domain)
    {
    Statement stmt;
    stmt.Prepare (*m_project, SqlPrintfString ("INSERT INTO " DGN_TABLE_Domain " (Name,Descr,Version) VALUES(?,?,?)"));
    stmt.BindText (1, domain.GetName(), Statement::MAKE_COPY_No);
    stmt.BindText (2, domain.GetDescription(), Statement::MAKE_COPY_No);
    stmt.BindInt (3, domain.GetVersion());

    DbResult status = stmt.Step();
    if (BE_SQLITE_DONE != status)
        {
        BeAssert (false && L"Problem with stmt.Step()");
        }

    for (auto handler : domain.GetHandlers())
        AddElementHandler (domain.GetName(), *handler.second);

    AddLoadedDomain(domain);
    return BE_SQLITE_OK;
    }


DgnColors::~DgnColors() {DELETE_AND_CLEAR(m_colorMap);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnColors::SaveDgnColorMap()
    {
    DgnColorMapP colorMap = GetDgnColorMapP(); // make sure it's valid

    RgbColorDef colors[DgnColorMap::INDEX_ColorCount];
    colorMap->GetRgbColors(colors);

    return m_project.SaveProperty (DgnProjectProperty::ColorTable(), colors, DgnColorMap::INDEX_ColorCount*3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnColorMapCP DgnColors::GetDgnColorMap() const
    {
    if (NULL == m_colorMap)
        {
        m_colorMap = new DgnColorMap();

        RgbColorDef rgbColors[DgnColorMap::INDEX_ColorCount];
        if (BE_SQLITE_ROW == m_project.QueryProperty (rgbColors, DgnColorMap::INDEX_ColorCount*3, DgnProjectProperty::ColorTable()))
            m_colorMap->SetupFromRgbColors(rgbColors);
        else
            m_colorMap->SetupDefaultColors();
        }

    return m_colorMap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTrueColorId DgnColors::QueryTrueColorId (RgbColorDef const& color) const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT Id FROM " DGN_TABLE_Color " WHERE RGB=?");
    stmt.BindInt (1, IntColorDef(color));

    return  (BE_SQLITE_ROW == stmt.Step()) ? DgnTrueColorId(stmt.GetValueInt(0)) : DgnTrueColorId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnColors::QueryTrueColorInfo (RgbColorDef* color, Utf8StringP name, Utf8StringP book, DgnTrueColorId id) const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT RGB,Name,Book FROM " DGN_TABLE_Color " WHERE Id=?");
    stmt.BindId (1, id);
    if (BE_SQLITE_ROW != stmt.Step())
        return  ERROR;

    if (color)
        *color = IntColorDef(stmt.GetValueInt(0)).m_rgb;

    if (name)
        name->AssignOrClear(stmt.GetValueText(1));

    if (book)
        book->AssignOrClear(stmt.GetValueText(2));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTrueColorId DgnColors::InsertTrueColor (RgbColorDef const& color, Utf8CP name, Utf8CP book)
    {
    DgnTrueColorId newId = GetDgnColorMap()->UseNextTrueColorId(m_project);

    BeAssert (0 < newId.GetValue());

    Statement stmt;
    stmt.Prepare (m_project, "INSERT INTO " DGN_TABLE_Color " (Id,RGB,Name,Book) VALUES(?,?,?,?)");

    stmt.BindId (1, newId);
    stmt.BindInt (2, IntColorDef(color));
    if (name && *name)
        stmt.BindText(3, name, Statement::MAKE_COPY_No);

    if (book && *book)
        stmt.BindText(4, book, Statement::MAKE_COPY_No);

    DbResult status = stmt.Step();
    BeAssert (BE_SQLITE_DONE==status);
    return (BE_SQLITE_DONE==status) ? newId : DgnTrueColorId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/11
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnColors::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE_Color);

    Statement sql;
    sql.Prepare (*m_db, sqlString.c_str());

    return (BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnColors::Iterator::const_iterator DgnColors::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,RGB,Name,Book FROM " DGN_TABLE_Color);

        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnTrueColorId DgnColors::Iterator::Entry::GetId() const {Verify(); return DgnTrueColorId(m_sql->GetValueInt(0));}
RgbColorDef DgnColors::Iterator::Entry::GetColorValue () const {Verify(); return IntColorDef(m_sql->GetValueInt(1)).m_rgb;}
Utf8CP DgnColors::Iterator::Entry::GetName() const {Verify(); return m_sql->GetValueText(2);}
Utf8CP DgnColors::Iterator::Entry::GetBookName() const {Verify(); return m_sql->GetValueText(3);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModels::~DgnModels()
    {
    Empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModels::DgnModels(DgnProjectR project) : DgnProjectTable(project), m_elements(project)
    {
    m_qvCache= NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::Empty()
    {
    ClearLoaded();
    FreeQvCache (); // free the quickvision cache, if present
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnModels::InsertModel (Model& row)
    {
    DbResult status;
    if (!row.m_id.IsValid())
        {
        status = m_project.GetNextRepositoryBasedId ((BeRepositoryBasedId&) row.m_id, DGN_TABLE_Model, "Id");
        BeAssert (status == BE_SQLITE_OK);
        }

    Statement stmt;
    stmt.Prepare (m_project, "INSERT INTO " DGN_TABLE_Model " (Id,Name,Descr,Type,SubType,Space,Visibility) VALUES(?,?,?,?,?,?,?)");

    stmt.BindId (1, row.GetId());
    stmt.BindText (2, row.GetNameCP(), Statement::MAKE_COPY_No);
    stmt.BindText (3, row.GetDescription(), Statement::MAKE_COPY_No);
    stmt.BindInt (4, (int)row.GetModelType());
    stmt.BindText(5, row.GetModelSubType(), Statement::MAKE_COPY_No);
    stmt.BindInt (6, (int) row.GetCoordinateSpace());
    stmt.BindInt (7, row.GetVisibility());

    status = stmt.Step();
    BeAssert (BE_SQLITE_DONE==status);
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::DeleteModel (DgnModelId modelId)
    {
    Statement stmt;
    stmt.Prepare (m_project, "DELETE FROM " DGN_TABLE_Model " WHERE Id=?");
    stmt.BindId (1, modelId);
    return BE_SQLITE_DONE == stmt.Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnModels::QueryModelId(Utf8CP name) const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT Id FROM " DGN_TABLE_Model " WHERE Name=?");
    stmt.BindText (1, name, Statement::MAKE_COPY_No);
    return (BE_SQLITE_ROW != stmt.Step()) ? DgnModelId() : stmt.GetValueId<DgnModelId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::QueryModelById (Model* out, DgnModelId id) const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT Name,Descr,Type,SubType,Space,Visibility FROM " DGN_TABLE_Model " WHERE Id=?");
    stmt.BindId(1, id);

    if (BE_SQLITE_ROW != stmt.Step())
        return  ERROR;

    if (out) // this can be null to just test for the existence of a model by id
        {
        out->m_id         = id;
        out->m_name.AssignOrClear(stmt.GetValueText(0));
        out->m_description.AssignOrClear(stmt.GetValueText(1));
        out->m_modelType  = (DgnModelType) stmt.GetValueInt(2);
        out->m_subType.AssignOrClear(stmt.GetValueText(3));
        out->m_space = (Model::CoordinateSpace) stmt.GetValueInt(4);
        out->m_visibility = stmt.GetValueInt(5);
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::GetModelName (Utf8StringR name, DgnModelId id) const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT Name FROM " DGN_TABLE_Model " WHERE Id=?");
    stmt.BindId(1, id);

    if (BE_SQLITE_ROW != stmt.Step())
        return  ERROR;

    name.AssignOrClear(stmt.GetValueText(0));
    return  SUCCESS;
    }

T_DgnModelMap const& DgnModels::GetLoadedModels() const {return m_loadedModels;}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::ClearLoaded()
    {
    // Has to be called before model pointers are invalidated.
    m_elements.OnDestroying();

    for (auto& it : m_loadedModels)
        delete it.second;

    m_loadedModels.clear();
    m_elements.Destroy();
    }

static bvector<DgnModels::Factory*> s_modelFactories;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::Register (Factory& factory)
    {
    UnRegister(factory);       // make sure no factory is registered twice.
    s_modelFactories.push_back(&factory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::UnRegister (Factory& factory)
    {
    for (auto it=s_modelFactories.begin(); it!=s_modelFactories.end(); ++it)
        {
        if (*it == &factory)
            {
            s_modelFactories.erase(it);
            return; // we know it can only be in the list once
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnModelP getDgnModelObject(DgnProjectR project, DgnModels::Model const& model)
    {
    // first allow any registered factories create the DgnModel
    for (auto it=s_modelFactories.rbegin(); it!=s_modelFactories.rend(); ++it)
        {
        auto dgnModel = (*it)->_SupplyDgnModel(project, model);
        if (nullptr != dgnModel)
            return dgnModel;
        }

    // none of the factories were able to make a DgnModel, use the default DgnModel classes.
    auto modelName= model.GetNameCP();
    auto modelId  = model.GetId();
    switch (model.GetModelType())
        {
        case DgnModelType::Physical:
        case DgnModelType::TEMP_V8IMPORT_ONLY_3dSheet:
            return new PhysicalModel (project, modelId, modelName);

        case DgnModelType::Component:
            return new ComponentModel (project, modelId, modelName);

        case DgnModelType::Drawing:
            return new DrawingModel (project, modelId, modelName);

        case DgnModelType::Sheet:
            return new SheetModel (project, modelId, modelName);

        case DgnModelType::Redline:
            return new RedlineModel (project, modelId, modelName);
    
        case DgnModelType::PhysicalRedline:
            return new PhysicalRedlineModel (project, modelId, modelName);
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP DgnModels::NewModelObject (DgnModelId modelId)
    {
    DgnModels::Model model;
    if (SUCCESS != QueryModelById (&model, modelId))
        return nullptr;

    DgnModelP dgnModel = getDgnModelObject(m_project, model);
    if (nullptr == dgnModel)
        return nullptr;

    dgnModel->SetReadOnly (m_project.IsReadonly());
    m_loadedModels[modelId] = dgnModel;
    return  dgnModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP DgnModels::FindModelById (DgnModelId modelId)
    {
    auto const& it=m_loadedModels.find(modelId);
    return  it!=m_loadedModels.end() ? it->second : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP DgnModels::GetModelById (DgnModelId modelId)
    {
    if (!modelId.IsValid())
        return  NULL;

    DgnModelP dgnModel = FindModelById (modelId);
    return (NULL != dgnModel) ? dgnModel : NewModelObject(modelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/11
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnModels::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE_Model " WHERE (?1 = (Visibility & ?1))", true);

    Statement sql;
    sql.Prepare (*m_db, sqlString.c_str());
    sql.BindInt (1, (int) m_itType);

    return (BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModels::Iterator::const_iterator DgnModels::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,Name,Descr,Type,Space,Visibility FROM " DGN_TABLE_Model " WHERE (?1 = (Visibility & ?1))", true);

        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        m_stmt->BindInt (1, (int) m_itType);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnModelId   DgnModels::Iterator::Entry::GetModelId() const {Verify(); return m_sql->GetValueId<DgnModelId>(0);}
Utf8CP       DgnModels::Iterator::Entry::GetName() const {Verify();return m_sql->GetValueText(1);}
Utf8CP       DgnModels::Iterator::Entry::GetDescription() const {Verify();return m_sql->GetValueText(2);}
DgnModelType DgnModels::Iterator::Entry::GetModelType() const {Verify();return (DgnModelType) m_sql->GetValueInt(3);}
DgnModels::Model::CoordinateSpace DgnModels::Iterator::Entry::GetCoordinateSpace() const {Verify();return (Model::CoordinateSpace) m_sql->GetValueInt(4);}
UInt32       DgnModels::Iterator::Entry::GetVisibility() const {Verify();return m_sql->GetValueInt(5);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnLevels::InsertLevel (DgnLevels::Level const& row, DgnLevels::SubLevel::Appearance const& appearance)
    {
    if (!row.IsValid() || !IsValidName(row.GetName()))
        {
        // caller must have already received a valid id from server
        BeAssert (false);
        return  BE_SQLITE_ERROR;
        }

    Statement stmt;
    stmt.Prepare (m_project, "INSERT INTO " DGN_TABLE_Level " (Id,Name,Descr,Rank,Scope) VALUES(?,?,?,?,?)");

    stmt.BindInt  (1, row.GetLevelId().GetValue());
    stmt.BindText (2, row.GetName(), Statement::MAKE_COPY_No);
    stmt.BindText (3, row.GetDescription(), Statement::MAKE_COPY_No);
    stmt.BindInt  (4, (int) row.GetRank());
    stmt.BindInt  (5, (int) row.GetScope());

    DbResult status = stmt.Step();
    if (BE_SQLITE_DONE != status)
        return status;

    SubLevel subLevel (SubLevelId(row.GetLevelId()), "", appearance);
    return InsertSubLevel (subLevel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnLevels::UpdateLevel (Level const& row)
    {
    if (!row.IsValid() || !IsValidName(row.GetName()))
        return  BE_SQLITE_ERROR;

    Statement stmt;
    stmt.Prepare (m_project, "UPDATE " DGN_TABLE_Level " SET Name=?,Descr=?,Rank=?,Scope=? WHERE Id=?");

    stmt.BindText (1, row.GetName(), Statement::MAKE_COPY_No);
    stmt.BindText (2, row.GetDescription(), Statement::MAKE_COPY_No);
    stmt.BindInt  (3, (int) row.GetRank());
    stmt.BindInt  (4, (int) row.GetScope());
    stmt.BindInt  (5, row.GetLevelId().GetValue());

    DbResult status = stmt.Step();
    BeDataAssert (BE_SQLITE_DONE==status);
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnLevels::DeleteLevel (LevelId levelId)
    {
    // don't allow anyone to delete the default level.
    if (levelId == LEVEL_DEFAULT_LEVEL_ID)
        return BE_SQLITE_ERROR;
    Statement stmt;
    stmt.Prepare (m_project, "DELETE FROM " DGN_TABLE_Level " WHERE Id=?");
    stmt.BindInt (1, levelId.GetValue());
    const auto status = stmt.Step ();
    return (BE_SQLITE_DONE == status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
LevelId DgnLevels::QueryLevelId(Utf8CP name) const
    {
    CachedStatementPtr stmt;
    m_project.GetCachedStatement (stmt, "SELECT Id FROM " DGN_TABLE_Level " WHERE Name=?");
    stmt->BindText (1, name, Statement::MAKE_COPY_No);
    return (BE_SQLITE_ROW != stmt->Step()) ? LevelId() : LevelId(stmt->GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
LevelId DgnLevels::QueryHighestId() const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT max(Id) FROM " DGN_TABLE_Level);
    return (BE_SQLITE_ROW != stmt.Step()) ? LevelId() : LevelId(stmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLevels::Level DgnLevels::QueryLevelById (LevelId id) const
    {
    if (!id.IsValid())
        return Level();

    //  This has no effect unless there is a range tree query occurring during update dynamics.  See comments
    //  on HighPriorityOperationBlock for more information.
    BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;
    CachedStatementPtr stmt;
    m_project.GetCachedStatement (stmt, "SELECT Name,Descr,Rank,Scope FROM " DGN_TABLE_Level " WHERE Id=?");
    stmt->BindInt(1, id.GetValue());

    Level level;
    if (BE_SQLITE_ROW == stmt->Step())
        {
        level.m_levelId = id;
        level.m_name.AssignOrClear (stmt->GetValueText(0));
        level.m_description.AssignOrClear (stmt->GetValueText(1));
        level.m_rank = (Rank) stmt->GetValueInt(2);
        level.m_scope = (Scope) stmt->GetValueInt(3);
        }

    return level;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLevels::Iterator::const_iterator DgnLevels::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,Name,Descr,Rank,Scope FROM " DGN_TABLE_Level);

        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
size_t DgnLevels::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE_Level);

    Statement sql;
    sql.Prepare (*m_db, sqlString.c_str());

    return ((BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt (0));
    }

LevelId   DgnLevels::Iterator::Entry::GetLevelId() const  {Verify(); return LevelId(m_sql->GetValueInt(0));}
Utf8CP    DgnLevels::Iterator::Entry::GetName() const {Verify(); return m_sql->GetValueText(1);}
Utf8CP    DgnLevels::Iterator::Entry::GetDescription() const {Verify(); return m_sql->GetValueText(2);}
DgnLevels::Rank DgnLevels::Iterator::Entry::GetRank() const {Verify(); return (Rank) m_sql->GetValueInt(3);}
DgnLevels::Scope DgnLevels::Iterator::Entry::GetScope() const {Verify();return (Scope) m_sql->GetValueInt(4);}

static Utf8CP APPEARANCE_Invisible = "invisible";
static Utf8CP APPEARANCE_Color = "color";
static Utf8CP APPEARANCE_Weight = "weight";
static Utf8CP APPEARANCE_Style = "style";
static Utf8CP APPEARANCE_Priority = "priority";
static Utf8CP APPEARANCE_Material= "material";
static Utf8CP APPEARANCE_Transparency = "transp";
static Utf8CP SubLevelId_LevelId = "levelid";
static Utf8CP SubLevelId_SubLevelId = "sublevelid";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnLevels::InsertSubLevel (SubLevel& subLevel)
    {
    if (!subLevel.GetId().GetLevel().IsValid())
        {
        BeAssert (false);
        return BE_SQLITE_ERROR;
        }

    // if the supplied DgnSubLevelId is invalid, get the first available one
    if (!subLevel.GetId().GetSubLevel().IsValid())
        {
        DgnSubLevelId highestId = QueryHighestSubLevelId(subLevel.GetId().GetLevel());
        BeAssert (highestId.IsValid()); // should always have default SubLevel
        subLevel.m_id = SubLevelId(subLevel.GetId().GetLevel(), DgnSubLevelId(highestId.GetValue() + 1));
        }

    Statement stmt;
    stmt.Prepare (m_project, "INSERT INTO " DGN_TABLE_SubLevel " (LevelId,Id,Name,Descr,Props) VALUES(?,?,?,?,?)");

    stmt.BindInt (1, subLevel.GetId().GetLevel().GetValue());
    stmt.BindInt (2, subLevel.GetId().GetSubLevel().GetValue());
    if (!subLevel.IsDefaultSubLevel()) // default SubLevels don't have a name/descr
        {
        if (!IsValidName(subLevel.GetName()))
            return BE_SQLITE_ERROR;

        stmt.BindText (3, subLevel.GetName(), Statement::MAKE_COPY_No);
        stmt.BindText (4, subLevel.GetDescription(), Statement::MAKE_COPY_No);
        }
    stmt.BindText (5, subLevel.GetAppearance().ToJson(), Statement::MAKE_COPY_Yes);

    DbResult status = stmt.Step();
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnLevels::UpdateSubLevel (SubLevel const& subLevel)
    {
    Statement stmt;
    stmt.Prepare (m_project, "UPDATE " DGN_TABLE_SubLevel " SET Name=?,Descr=?,Props=? WHERE LevelId=? AND Id=?");

    if (!subLevel.IsDefaultSubLevel()) // default SubLevels don't have a name/descr
        {
        if (!IsValidName(subLevel.GetName()))
            return BE_SQLITE_ERROR;

        stmt.BindText (1, subLevel.GetName(), Statement::MAKE_COPY_No);
        stmt.BindText (2, subLevel.GetDescription(), Statement::MAKE_COPY_No);
        }
    stmt.BindText (3, subLevel.GetAppearance().ToJson(), Statement::MAKE_COPY_Yes);
    stmt.BindInt (4, subLevel.GetId().GetLevel().GetValue());
    stmt.BindInt (5, subLevel.GetId().GetSubLevel().GetValue());

    DbResult status = stmt.Step();
    BeDataAssert (BE_SQLITE_DONE==status);
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnLevels::DeleteSubLevel(SubLevelId subLevelId)
    {
    if (DgnSubLevelId(0) == subLevelId.GetSubLevel()) // don't allow the default SubLevel to be deleted
        return BE_SQLITE_ERROR;

    Statement stmt;
    stmt.Prepare (m_project, "DELETE FROM " DGN_TABLE_SubLevel " WHERE LevelId=? AND Id=?");
    stmt.BindInt (1, subLevelId.GetLevel().GetValue());
    stmt.BindInt (2, subLevelId.GetSubLevel().GetValue());
    return BE_SQLITE_DONE == stmt.Step() ? BE_SQLITE_OK : BE_SQLITE_ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubLevelId DgnLevels::QuerySubLevelId (LevelId levelId, Utf8CP subLevelName) const
    {
    CachedStatementPtr stmt;
    m_project.GetCachedStatement (stmt, "SELECT Id FROM " DGN_TABLE_SubLevel " WHERE LevelId=? AND Name=?");
    stmt->BindInt (1, levelId.GetValue());
    stmt->BindText (2, subLevelName, Statement::MAKE_COPY_No);
    return (BE_SQLITE_ROW != stmt->Step()) ? DgnSubLevelId() : DgnSubLevelId(stmt->GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubLevelId DgnLevels::QueryHighestSubLevelId(LevelId levelId) const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT max(Id) FROM " DGN_TABLE_SubLevel " WHERE LevelId=?");
    stmt.BindInt (1, levelId.GetValue());
    return (BE_SQLITE_ROW != stmt.Step()) ? DgnSubLevelId() : DgnSubLevelId(stmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLevels::SubLevel DgnLevels::QuerySubLevelById (SubLevelId subLevelId) const
    {
    if (!subLevelId.IsValid())
        return SubLevel();

    BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock; // See comments on HighPriorityOperationBlock

    CachedStatementPtr stmt;
    m_project.GetCachedStatement (stmt, "SELECT Name,Descr,Props FROM " DGN_TABLE_SubLevel " WHERE LevelId=? AND Id=?");
    stmt->BindInt(1, subLevelId.GetLevel().GetValue());
    stmt->BindInt(2, subLevelId.GetSubLevel().GetValue());

    SubLevel subLevel;
    if (BE_SQLITE_ROW == stmt->Step())
        {
        subLevel.m_id = subLevelId;
        subLevel.m_name.AssignOrClear(stmt->GetValueText(0));
        subLevel.m_description.AssignOrClear(stmt->GetValueText(1));
        subLevel.m_appearance.FromJson(stmt->GetValueText(2));
        }

    return subLevel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLevels::SubLevelIterator::const_iterator DgnLevels::SubLevelIterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        bool whereLevel = m_levelId.IsValid();

        Utf8String selectSql = "SELECT LevelId,Id,Name,Descr,Props FROM " DGN_TABLE_SubLevel;
        if (whereLevel)
            selectSql.append(" WHERE LevelId=?");

        Utf8String sqlString = MakeSqlString(selectSql.c_str(), whereLevel);

        m_db->GetCachedStatement (m_stmt, sqlString.c_str());

        if (whereLevel)
            m_stmt->BindInt(1, m_levelId.GetValue());

        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

LevelId DgnLevels::SubLevelIterator::Entry::GetLevelId() const {Verify(); return LevelId(m_sql->GetValueInt(0));}
DgnSubLevelId DgnLevels::SubLevelIterator::Entry::GetSubLevelId() const {Verify(); return DgnSubLevelId(m_sql->GetValueInt(1));}
Utf8CP DgnLevels::SubLevelIterator::Entry::GetName() const {Verify(); return m_sql->GetValueText(2);}
Utf8CP DgnLevels::SubLevelIterator::Entry::GetDescription() const {Verify(); return m_sql->GetValueText(3);}
DgnLevels::SubLevel::Appearance DgnLevels::SubLevelIterator::Entry::GetAppearance() const {Verify(); return DgnLevels::SubLevel::Appearance(m_sql->GetValueText(4));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SubLevelId::ToJson (JsonValueR val) const
    {
    val[SubLevelId_LevelId] = m_level.GetValue();
    val[SubLevelId_SubLevelId] = m_subLevel.GetValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SubLevelId::FromJson (JsonValueCR val)
    {
    m_level = LevelId(val[SubLevelId_LevelId].asUInt());
    m_subLevel = DgnSubLevelId(val[SubLevelId_SubLevelId].asUInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLevels::SubLevel::Appearance::FromJson(Utf8StringCR jsonStr)
    {
    Init();

    Json::Value val (Json::objectValue);
    if (!Json::Reader::Parse(jsonStr, val))
        return;

    m_invisible = val.get(APPEARANCE_Invisible, false).asBool();
    m_color  = val[APPEARANCE_Color].asUInt();
    m_weight = val[APPEARANCE_Weight].asUInt();
    m_style  = val[APPEARANCE_Style].asInt();
    m_displayPriority = val[APPEARANCE_Priority].asInt();
    m_transparency = val[APPEARANCE_Transparency].asDouble();

    if (val.isMember(APPEARANCE_Material))
        m_material = DgnMaterialId(val[APPEARANCE_Material].asUInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnLevels::SubLevel::Appearance::ToJson() const
    {
    Json::Value val;

    if (m_invisible)            val[APPEARANCE_Invisible] = true;
    if (0 != m_color)           val[APPEARANCE_Color]  = m_color;
    if (0 != m_weight)          val[APPEARANCE_Weight] = m_weight;
    if (0 != m_style)           val[APPEARANCE_Style]  = m_style;
    if (0 != m_displayPriority) val[APPEARANCE_Priority] = m_displayPriority;
    if (m_material.IsValid())   val[APPEARANCE_Material] = m_material.GetValue();
    if (0.0 != m_transparency)  val[APPEARANCE_Transparency] = m_transparency;

    return Json::FastWriter::ToString(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnLevels::SubLevel::Appearance::operator==(Appearance const& other) const
    {
    return m_invisible==other.m_invisible &&
           m_color==other.m_color && 
           m_weight==other.m_weight && 
           m_style==other.m_style && 
           m_displayPriority==other.m_displayPriority && 
           m_material==other.m_material && 
           m_transparency==other.m_transparency;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLevels::SubLevel::Override::ToJson(JsonValueR outValue) const
    {
    if (m_flags.m_invisible)    outValue[APPEARANCE_Invisible] = m_value.IsInvisible();
    if (m_flags.m_color)        outValue[APPEARANCE_Color] = m_value.GetColor();
    if (m_flags.m_weight)       outValue[APPEARANCE_Weight] = m_value.GetWeight();
    if (m_flags.m_style)        outValue[APPEARANCE_Style] = m_value.GetStyle();
    if (m_flags.m_material)     outValue[APPEARANCE_Material] = m_value.GetMaterial().GetValue();
    if (m_flags.m_priority)     outValue[APPEARANCE_Priority] = m_value.GetDisplayPriority();
    if (m_flags.m_transparency) outValue[APPEARANCE_Transparency] = m_value.GetTransparency();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLevels::SubLevel::Override::FromJson(JsonValueCR val)
    {
    Init();

    if (val.isMember(APPEARANCE_Invisible))    SetInvisible(val[APPEARANCE_Invisible].asBool());
    if (val.isMember(APPEARANCE_Color))        SetColor(val[APPEARANCE_Color].asUInt());
    if (val.isMember(APPEARANCE_Weight))       SetWeight(val[APPEARANCE_Weight].asUInt());
    if (val.isMember(APPEARANCE_Style))        SetStyle(val[APPEARANCE_Style].asInt());
    if (val.isMember(APPEARANCE_Material))     SetMaterial(DgnMaterialId(val[APPEARANCE_Material].asUInt64()));
    if (val.isMember(APPEARANCE_Priority))     SetDisplayPriority(val[APPEARANCE_Priority].asInt());
    if (val.isMember(APPEARANCE_Transparency)) SetTransparency(val[APPEARANCE_Transparency].asDouble());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLevels::SubLevel::Override::ApplyTo(Appearance& appear) const
    {
    if (m_flags.m_invisible)    appear.SetInvisible(m_value.IsInvisible());
    if (m_flags.m_color)        appear.SetColor(m_value.GetColor());
    if (m_flags.m_weight)       appear.SetWeight(m_value.GetWeight());
    if (m_flags.m_style)        appear.SetStyle(m_value.GetStyle());
    if (m_flags.m_material)     appear.SetMaterial(m_value.GetMaterial());
    if (m_flags.m_priority)     appear.SetDisplayPriority(m_value.GetDisplayPriority());
    if (m_flags.m_transparency) appear.SetTransparency(m_value.GetTransparency());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnKeyStringId DgnKeyStrings::QueryId (Utf8CP name)
    {
    Statement stmt;
    stmt.Prepare(m_project, "SELECT Id FROM " DGN_TABLE_KeyStr " WHERE Name=?");
    stmt.BindText(1, name, Statement::MAKE_COPY_No);
    return (BE_SQLITE_ROW==stmt.Step()) ?  stmt.GetValueId<DgnKeyStringId>(0) : DgnKeyStringId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnKeyStrings::Query (DgnKeyStringId id, Utf8StringP name, Utf8StringP value)
    {
    Statement stmt;
    stmt.Prepare(m_project, "SELECT Name,Def FROM " DGN_TABLE_KeyStr " WHERE Id=?");
    stmt.BindId (1, id);
    DbResult rc = stmt.Step();
    if (BE_SQLITE_ROW != rc)
        return  BE_SQLITE_ERROR;

    if (name)
        name->AssignOrClear (stmt.GetValueText(0));
    if (value)
        value->AssignOrClear (stmt.GetValueText(1));

    return  BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnKeyStringId DgnKeyStrings::Insert (Utf8CP name, Utf8CP value)
    {
    DgnKeyStringId id;
    DbResult rc = m_project.GetNextRepositoryBasedId (id, DGN_TABLE_KeyStr, "Id");
    if (BE_SQLITE_OK != rc)
        return  id;

    Statement stmt;
    stmt.Prepare(m_project, "INSERT INTO " DGN_TABLE_KeyStr " (Id,Name,Def) VALUES (?,?,?)");
    stmt.BindId(1, id);
    stmt.BindText(2, name, Statement::MAKE_COPY_No);
    stmt.BindText(3, value, Statement::MAKE_COPY_No);
    return  (stmt.Step() == BE_SQLITE_DONE) ? id : DgnKeyStringId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnKeyStrings::Update (Utf8CP name, Utf8CP value)
    {
    Statement stmt;
    stmt.Prepare(m_project, "UPDATE " DGN_TABLE_KeyStr " SET Def=?2 WHERE Name=?1");
    stmt.BindText(1, name, Statement::MAKE_COPY_No);
    stmt.BindText(2, value, Statement::MAKE_COPY_No);
    return  stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnKeyStrings::Delete (Utf8CP name)
    {
    Statement stmt;
    stmt.Prepare(m_project, "DELETE FROM " DGN_TABLE_KeyStr " WHERE Name=?");
    stmt.BindText(1, name, Statement::MAKE_COPY_No);
    return  stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFonts::DgnFonts(DgnProjectR project) : DgnProjectTable(project)
    {
    m_fontNumberMapLoaded = m_embeddedFontsLoaded = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFontCP DgnFonts::GetMissingFont(DgnFontType fontType, Utf8CP fontName) const
    {
    T_FontCatalogMap::const_iterator existingMissingFont = m_missingFonts.find(DgnFontKey(fontType, fontName));
    return (m_missingFonts.end() != existingMissingFont) ? existingMissingFont->second.get() : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jeff.Marker                     10/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFontCP DgnFonts::GetOrCreateMissingFont(DgnFontType fontType, Utf8CP fontName, UInt32 v8FontNumber) const
    {
    DgnFontCP missing = GetMissingFont(fontType, fontName);
    if (NULL != missing)
        return  missing;

    switch (fontType)
        {
        case DgnFontType::Rsc:
            {
            T_HOST.GetFontAdmin()._OnMissingRscFont(fontName);
            DgnFontPtr missingFont = DgnRscFont::CreateMissingFont(fontName, v8FontNumber);
            m_missingFonts.Insert(DgnFontKey(DgnFontType::Rsc, fontName), missingFont);
            return missingFont.get();
            }

        case DgnFontType::Shx:
            {
            T_HOST.GetFontAdmin()._OnMissingShxFont(fontName);
            DgnFontPtr missingFont = DgnShxFont::CreateMissingFont(fontName);
            m_missingFonts.Insert(DgnFontKey(DgnFontType::Shx, fontName), missingFont);
            return missingFont.get();
            }

        case DgnFontType::TrueType:
            {
            T_HOST.GetFontAdmin()._OnMissingTrueTypeFont(fontName);
            DgnFontPtr missingFont = DgnTrueTypeFont::CreateMissingFont(fontName);
            m_missingFonts.Insert(DgnFontKey(DgnFontType::TrueType, fontName), missingFont);
            return missingFont.get();
            }
        }

    BeAssert(false && L"Unknown/unexpected DgnFontType.");
    return &DgnFontManager::GetDefaultTrueTypeFont();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/11
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnFonts::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE_Font);

    Statement sql;
    sql.Prepare (*m_db, sqlString.c_str());

    return (BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFonts::Iterator::Entry DgnFonts::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,Name,Type FROM " DGN_TABLE_Font);

        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

UInt32 DgnFonts::Iterator::Entry::GetFontId() const   {Verify(); return m_sql->GetValueInt(0);}
Utf8CP DgnFonts::Iterator::Entry::GetName() const     {Verify(); return m_sql->GetValueText(1);}
DgnFontType DgnFonts::Iterator::Entry::GetFontType() const {Verify(); return (DgnFontType) m_sql->GetValueInt(2);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnFonts::InsertFontNumberMappingDirect (UInt32 id, DgnFontType fontType, Utf8CP name)
    {
    // Basic sanity check... primarily to prevent -1, but catches other corrupt cases.
    if (id > 5000000)
        {
        BeAssert (false);
        return ERROR;
        }

    // Existing elements' internal data may depend on the font they're using; you cannot simply replace a font mapping.
    if (m_fontNumberMap.end() != m_fontNumberMap.find (id))
        {
        BeDataAssert (false && L"A font with this ID already exists, and the mapping cannot be replaced.");
        return ERROR;
        }

    Statement stmt;
    stmt.Prepare (m_project, "INSERT INTO " DGN_TABLE_Font " (Id,Name,Type) VALUES(?,?,?)");
    stmt.BindInt (1, (int)id);
    stmt.BindText (2, name, Statement::MAKE_COPY_No);
    stmt.BindInt (3, (int)fontType);

    if (BE_SQLITE_DONE != stmt.Step())
        return ERROR;

    DgnFontCP font = DgnFontManager::FindFont (name, fontType, &m_project);
    if (NULL == font)
        font = GetOrCreateMissingFont(fontType, name);

    m_fontNumberMap[id] = font;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2014
//---------------------------------------------------------------------------------------
bool DgnFonts::IsFontNumberMapLoaded() const
    {
    return m_fontNumberMapLoaded;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
BentleyStatus DgnFonts::AcquireFontNumber (UInt32& acquiredID, DgnFontCR font)
    {
    acquiredID = (UInt32)-1;

    if (!font.IsAccessibleByProject(m_project))
        {
        BeAssert(false);
        return ERROR;
        }

    // Attempt to re-use an existing mapping first.
    if (SUCCESS == FindFontNumber(&acquiredID, font))
        return SUCCESS;

    // Attempt to maintain RSC font numbers, unless it's a system fallback font. These are last-resort options that are fine getting re-mapped because they don't reflect real-world RSC font libraries.
    if ((DgnFontType::Rsc == font.GetType()) && !font.IsLastResort())
        {
        auto const& rscFont = (DgnRscFont const&)font;
        auto rscFontNumber = rscFont.GetV8FontNumber();

        // If it's already in the map by-instance, we've returned above.
        // Therefore, if the V8 ID doesn't already have a mapping use it (otherwise it's a collision).
        // In the case of RSC font number collisions (now possible in DgnDb), the first one in gets its preferred ID; others are re-mapped like all other fonts.
        if (m_fontNumberMap.end() == m_fontNumberMap.find(rscFontNumber))
            {
            if (SUCCESS != InsertFontNumberMappingDirect(rscFontNumber, font.GetType(), font.GetName().c_str()))
                return ERROR;

            acquiredID = rscFontNumber;

            return SUCCESS;
            }

        // Otherwise fall through and give it the next available number (e.g. there was a collision above).
        }

    // To attempt to keep RSC font number mappings, don't attempt to assign anything but RSC fonts (tested above) less than 256.
    UInt32 tryFontNumber = 256;
    while (m_fontNumberMap.end() != m_fontNumberMap.find(tryFontNumber))
        ++tryFontNumber;

    if (SUCCESS != InsertFontNumberMappingDirect(tryFontNumber, font.GetType(), font.GetName().c_str()))
        return ERROR;

    acquiredID = tryFontNumber;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnFontCP DgnFonts::FindFont (UInt32 fontNumber) const
    {
    auto fontIter = FontNumberMap().find (fontNumber);
    return (FontNumberMap().end() == fontIter) ? NULL : fontIter->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
BentleyStatus DgnFonts::FindFontNumber (UInt32* foundID, DgnFontCR font) const
    {
    if (NULL != foundID)
        *foundID = (UInt32)-1;

    for (auto fontIter : FontNumberMap())
        {
        if (!fontIter.second->Equals (font))
            continue;

        if (NULL != foundID)
            *foundID = fontIter.first;

        return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2012
//---------------------------------------------------------------------------------------
DgnFontCP DgnFonts::EmbedFont (DgnFontCR font)
    {
    // Can't embed missing fonts; don't bother embedding last resort fonts.
    if (font.IsLastResort() || font.IsMissing())
        return NULL;

    for (auto embeddedFontIter : EmbeddedFonts())
        {
        if ((font.GetType() != embeddedFontIter.first.m_type) || !font.GetName().EqualsI (embeddedFontIter.first.m_name.c_str()))
            continue;

        return embeddedFontIter.second.get();
        }

    DgnFontPtr embeddedFont = font.Embed (m_project);
    if (!embeddedFont.IsValid())
        return NULL;

    m_embeddedFonts.Insert (DgnFontKey (embeddedFont->GetType(), embeddedFont->GetName()), embeddedFont);

    return embeddedFont.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
T_FontCatalogMap const& DgnFonts::EmbeddedFonts() const
    {
    if (m_embeddedFontsLoaded)
        return m_embeddedFonts;

    m_embeddedFontsLoaded = true;

    Statement embeddedFontNumberStmt;
    embeddedFontNumberStmt.Prepare (m_project, "SELECT DISTINCT(Id) FROM " BEDB_TABLE_Property " WHERE Namespace=?");
    embeddedFontNumberStmt.BindText (1, PROPERTY_APPNAME_DgnFont, Statement::MAKE_COPY_No);

    while (BE_SQLITE_ROW == embeddedFontNumberStmt.Step())
        {
        UInt32 fontNumber = static_cast<UInt32>(embeddedFontNumberStmt.GetValueInt(0));

        Statement fontTypeStmt;
        fontTypeStmt.Prepare (m_project, "SELECT Type,Name FROM " DGN_TABLE_Font " WHERE Id=?");
        fontTypeStmt.BindInt (1, fontNumber);

        if (BE_SQLITE_ROW != fontTypeStmt.Step())
            {
            BeAssert (false);
            continue;
            }

        DgnFontType fontType = static_cast<DgnFontType>(fontTypeStmt.GetValueInt (0));
        Utf8String  fontName(fontTypeStmt.GetValueText (1));
        DgnFontPtr  embeddedFont;

        BeAssert (BE_SQLITE_DONE == fontTypeStmt.Step());

        switch (fontType)
            {
            case DgnFontType::Rsc:
                embeddedFont = DgnRscFont::CreateFromEmbeddedFont (fontName.c_str(), fontNumber, m_project);
                break;
            case DgnFontType::Shx:
                embeddedFont = DgnShxFont::CreateFromEmbeddedFont (fontName.c_str(), fontNumber, m_project);
                break;
            case DgnFontType::TrueType:
                embeddedFont = DgnTrueTypeFont::CreateFromEmbeddedFont (fontName.c_str(), fontNumber, m_project);
                break;
            }

        if (!embeddedFont.IsValid())
            {
            BeAssert (false);
            continue;
            }

        m_embeddedFonts.Insert(DgnFontKey (fontType, embeddedFont->GetName()), embeddedFont);
        }

    return m_embeddedFonts;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
T_FontNumberMap const& DgnFonts::FontNumberMap() const
    {
    if (!m_fontNumberMapLoaded)
        {
        // See TFS#78396, scheduled for Graphite06.
        HighPriorityOperationBlock  highPriority;
        m_fontNumberMapLoaded = true;

        Statement stmt;
        stmt.Prepare (m_project, "SELECT Id,Type,Name FROM " DGN_TABLE_Font);

        while (BE_SQLITE_ROW == stmt.Step())
            {
            UInt32      fontID      = (UInt32)stmt.GetValueInt (0);
            DgnFontType fontType    = (DgnFontType)stmt.GetValueInt (1);
            Utf8String  fontName    (stmt.GetValueText (2));
            DgnFontCP   font        = DgnFontManager::FindFont (fontName.c_str(), fontType, &m_project);

            if (NULL == font)
                font = GetOrCreateMissingFont(fontType, fontName.c_str());

            m_fontNumberMap[fontID] = font;
            }
        }

    return m_fontNumberMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DgnStyleId DgnStyles::Iterator::Entry::GetId() const{Verify(); return DgnStyleId(m_sql->GetValueInt(0));}
DgnStyleType DgnStyles::Iterator::Entry::GetType() const{Verify(); return(DgnStyleType)m_sql->GetValueInt(1);}
Utf8CP DgnStyles::Iterator::Entry::GetName() const{Verify(); return m_sql->GetValueText(2);}
Utf8CP DgnStyles::Iterator::Entry::GetDescription() const{Verify(); return m_sql->GetValueText(3);}
void const* DgnStyles::Iterator::Entry::GetData() const{Verify(); return m_sql->GetValueBlob(4);}
int DgnStyles::Iterator::Entry::GetDataSize() const{Verify(); return m_sql->GetColumnBytes(4);}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DgnStyles::Iterator::Entry DgnStyles::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,Type,Name,Descr,Data FROM " DGN_TABLE_Style);

        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
size_t DgnStyles::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE_Style);

    Statement sql;
    sql.Prepare(*m_db, sqlString.c_str());

    return((BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DbResult DgnStyles::InsertStyle(Style& style, Int32 minimumStyleId)
    {
    // Acquire the next ID for this style type.
    Statement stmt;
    stmt.Prepare(m_project, "SELECT max(Id) FROM " DGN_TABLE_Style " WHERE Type=?");
    stmt.BindInt(1, (int) style.GetType());

    DbResult result = stmt.Step();
    if (BE_SQLITE_ROW != result)
        {
        BeAssert(false); // Could not determine next highest ID for style type.
        return result;
        }

    Int32 styleId = (stmt.GetValueInt(0) + 1);
    if (styleId < minimumStyleId)
        styleId = minimumStyleId;

    style.m_id = DgnStyleId(styleId);

    return InsertStyleWithId(style);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DbResult DgnStyles::InsertStyleWithId(Style& style)
    {
    if (!style.m_id.IsValid())
        return BE_SQLITE_ERROR;

    // Rely on constraints to restrict duplicate by-type and by-name.
    Statement stmt;
    stmt.Prepare(m_project, "INSERT INTO " DGN_TABLE_Style " (Id,Type,Name,Descr,Data) VALUES(?,?,?,?,?)");
    stmt.BindInt(1, style.m_id.GetValue());
    stmt.BindInt(2, (int) style.m_type);
    stmt.BindText(3, style.m_name, Statement::MAKE_COPY_No);
    stmt.BindText(4, style.m_description, Statement::MAKE_COPY_No);
    stmt.BindBlob(5, &style.m_data[0],(int)style.m_data.size(), Statement::MAKE_COPY_No);

    DbResult dbStatus = stmt.Step();
    return dbStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DbResult DgnStyles::DeleteStyle(DgnStyleType type, DgnStyleId id)
    {
    if (!id.IsValid())
        return  BE_SQLITE_ERROR;
    Statement stmt;
    stmt.Prepare(m_project, "DELETE FROM " DGN_TABLE_Style " WHERE Type=? AND Id=?");
    stmt.BindInt(1, (int) type);
    stmt.BindInt(2, id.GetValue());

    return stmt.Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DbResult DgnStyles::UpdateStyle(Style const& style)
    {
    Statement stmt;
    stmt.Prepare(m_project, "UPDATE " DGN_TABLE_Style " SET Name=?,Descr=?,Data=? WHERE Type=? AND Id=?");
    stmt.BindText(1, style.m_name, Statement::MAKE_COPY_No);
    stmt.BindText(2, style.m_description, Statement::MAKE_COPY_No);
    stmt.BindBlob(3, &style.m_data[0],(int)style.m_data.size(), Statement::MAKE_COPY_No);
    stmt.BindInt(4, (int) style.m_type);

    if (style.m_id.IsValid())
        stmt.BindInt(5, style.m_id.GetValue());

    return stmt.Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DgnStyleId DgnStyles::QueryStyleId(DgnStyleType type, Utf8CP name) const
    {
    Statement stmt;
    stmt.Prepare(m_project, "SELECT Id FROM " DGN_TABLE_Style " WHERE Type=? AND Name=?");
    stmt.BindInt(1, (int) type);
    stmt.BindText(2, name, Statement::MAKE_COPY_No);

    return((BE_SQLITE_ROW == stmt.Step()) ? DgnStyleId(stmt.GetValueInt(0)) : DgnStyleId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DgnStyles::Style DgnStyles::QueryStyleById(DgnStyleType type, DgnStyleId id) const
    {
    if (!id.IsValid())
        return DgnStyles::Style();

    BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;
    Statement stmt;
    stmt.Prepare(m_project, "SELECT Name,Descr,Data FROM " DGN_TABLE_Style " WHERE Type=? AND Id=?");
    stmt.BindInt(1, (int) type);
    stmt.BindInt(2, id.GetValue());

    DgnStyles::Style row;
    if(BE_SQLITE_ROW != stmt.Step())
        return row;

    row.m_id = id;
    row.m_type = type;

    row.m_name.AssignOrClear(stmt.GetValueText(0));
    row.m_description.AssignOrClear(stmt.GetValueText(1));

    size_t dataSize = (size_t)stmt.GetColumnBytes(2);
    if (dataSize > 0)
        {
        row.m_data.resize(dataSize);
        memcpy(&row.m_data[0], stmt.GetValueBlob(2), dataSize);
        }

    return row;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnProject::GetNextServerIssuedId (BeServerIssuedId& value, Utf8CP tableName, Utf8CP colName, UInt32 minimumId)
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

    UInt32 newId = stmt.GetValueInt(0) + 1;
    if (newId < minimumId)
        newId = minimumId;

    value = BeServerIssuedId (newId);
    return BE_SQLITE_OK;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     10/12
//---------------------------------------------------------------------------------------
DgnMaterialId   DgnMaterials::Iterator::Entry::GetId() const        {Verify(); return m_sql->GetValueId<DgnMaterialId>(0);}
Utf8CP          DgnMaterials::Iterator::Entry::GetName() const      {Verify(); return m_sql->GetValueText(1);}
Utf8CP          DgnMaterials::Iterator::Entry::GetPalette () const  {Verify(); return m_sql->GetValueText(2);}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     10/12
//---------------------------------------------------------------------------------------
size_t DgnMaterials::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE_Material);

    Statement sql;
    sql.Prepare (*m_db, sqlString.c_str());

    return (BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     10/12
//---------------------------------------------------------------------------------------
DgnMaterials::Iterator::Entry DgnMaterials::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString ("SELECT Id,Name,Palette FROM " DGN_TABLE_Material);
        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     10/12
//---------------------------------------------------------------------------------------
DbResult DgnMaterials::InsertMaterial (Row& row)
    {
    DbResult status;
    if (!row.m_materialId.IsValid())
        {
        status = m_project.GetNextRepositoryBasedId (row.m_materialId, DGN_TABLE_Material, "Id");
        BeAssert (status == BE_SQLITE_OK);
        }

    Statement stmt;
    stmt.Prepare (m_project, "INSERT INTO " DGN_TABLE_Material "(Id,Name,Palette) VALUES(?,?,?)");
    stmt.BindId (1, row.m_materialId);
    stmt.BindText (2, row.m_name, Statement::MAKE_COPY_No);
    stmt.BindText (3, row.m_palette, Statement::MAKE_COPY_No);

    status = stmt.Step();
    return (BE_SQLITE_DONE == status) ? BE_SQLITE_OK : status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     10/12
//---------------------------------------------------------------------------------------
DbResult DgnMaterials::UpdateMaterial (DgnMaterials::Row const& entry)
    {
    Statement stmt;
    stmt.Prepare (m_project, "UPDATE " DGN_TABLE_Material " SET Name=?,Palette=? WHERE Id=?");
    stmt.BindText (1, entry.m_name, Statement::MAKE_COPY_No);
    stmt.BindText (2, entry.m_palette, Statement::MAKE_COPY_No);
    stmt.BindId (3, entry.m_materialId);
    return stmt.Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     10/12
//---------------------------------------------------------------------------------------
DbResult DgnMaterials::DeleteMaterial (DgnMaterialId materialId)
    {
    Statement stmt;
    stmt.Prepare (m_project, "DELETE FROM " DGN_TABLE_Material " WHERE Id=?");
    stmt.BindId (1, materialId);
    return stmt.Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     10/12
//---------------------------------------------------------------------------------------
DgnMaterials::Row DgnMaterials::QueryMaterialById (DgnMaterialId id) const
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT Name,Palette FROM " DGN_TABLE_Material " WHERE Id=?");
    stmt.BindId (1, id);

    DgnMaterials::Row entry;
    if (BE_SQLITE_ROW == stmt.Step())
        {
        entry.m_materialId = id;
        entry.m_name.AssignOrClear (stmt.GetValueText (0));
        entry.m_palette.AssignOrClear (stmt.GetValueText (1));
        }

    return entry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyles::DgnStyles(DgnProjectR project) : DgnProjectTable(project)
    {
    m_displaySytles = nullptr;
    m_textStyles = nullptr;
    m_lineStyles = nullptr;
    m_annotationTextStyles = nullptr;
    m_annotationFrameStyles = nullptr;
    m_annotationLeaderStyles = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyles::~DgnStyles()
    {
    DELETE_AND_CLEAR(m_textStyles);
    DELETE_AND_CLEAR(m_lineStyles);
    DELETE_AND_CLEAR(m_annotationTextStyles);
    DELETE_AND_CLEAR(m_annotationFrameStyles);
    DELETE_AND_CLEAR(m_annotationLeaderStyles);
    }

DgnTextStyles& DgnStyles::TextStyles() {if (NULL==m_textStyles) m_textStyles = new DgnTextStyles(m_project); return *m_textStyles;}
DgnLineStyles& DgnStyles::LineStyles() {if (NULL==m_lineStyles) m_lineStyles = new DgnLineStyles(m_project); return *m_lineStyles;}
DgnAnnotationTextStyles& DgnStyles::AnnotationTextStyles() {if (NULL==m_annotationTextStyles) m_annotationTextStyles = new DgnAnnotationTextStyles(m_project); return *m_annotationTextStyles;}
DgnAnnotationFrameStyles& DgnStyles::AnnotationFrameStyles() {if (NULL==m_annotationFrameStyles) m_annotationFrameStyles = new DgnAnnotationFrameStyles(m_project); return *m_annotationFrameStyles;}
DgnAnnotationLeaderStyles& DgnStyles::AnnotationLeaderStyles() {if (NULL==m_annotationLeaderStyles) m_annotationLeaderStyles = new DgnAnnotationLeaderStyles(m_project); return *m_annotationLeaderStyles;}
DgnTextAnnotationSeeds& DgnStyles::TextAnnotationSeeds() {if (NULL==m_textAnnotationSeeds) m_textAnnotationSeeds = new DgnTextAnnotationSeeds(m_project); return *m_textAnnotationSeeds;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
static DgnFileStatus performEmbeddedProjectVersionChecks(DbResult& dbResult, Db& db, BeRepositoryBasedId id)
    {
    Utf8String versionString;
    dbResult = db.QueryProperty (versionString, DgnEmbeddedProjectProperty::SchemaVersion(), id.GetValue());
    if (BE_SQLITE_ROW != dbResult)
        return DGNSCHEMA_STATUS_VersionTooOld;

    DgnVersion actualDgnProfileVersion(0,0,0,0);
    actualDgnProfileVersion.FromJson(versionString.c_str());
    DgnVersion expectedDgnProfileVersion (PROJECT_CURRENT_VERSION_Major, PROJECT_CURRENT_VERSION_Minor, PROJECT_CURRENT_VERSION_Sub1, PROJECT_CURRENT_VERSION_Sub2);
    DgnVersion minimumAutoUpgradableDgnProfileVersion (PROJECT_SUPPORTED_VERSION_Major, PROJECT_SUPPORTED_VERSION_Minor, 0, 0);

    bool isProfileAutoUpgradable = false; //unused as this method is not attempting to auto-upgrade
    dbResult = Db::CheckProfileVersion (isProfileAutoUpgradable, expectedDgnProfileVersion, actualDgnProfileVersion, minimumAutoUpgradableDgnProfileVersion, db.IsReadonly(), "DgnDb");
    switch (dbResult)
        {
        case BE_SQLITE_ERROR_ProfileTooOld:
            return DGNSCHEMA_STATUS_VersionTooOld;
        case BE_SQLITE_ERROR_ProfileTooNew:
            return DGNSCHEMA_STATUS_VersionTooNew;
        default:
            return DGNFILE_STATUS_Success;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
static DgnFileStatus performPackageVersionChecks(DbResult& dbResult, Db& db)
    {
    Utf8String versionString;
    dbResult = db.QueryProperty (versionString, PackageProperty::SchemaVersion());
    if (BE_SQLITE_ROW != dbResult)
        return DGNSCHEMA_STATUS_VersionTooOld;

    PackageSchemaVersion actualPackageSchemaVersion(0,0,0,0);
    actualPackageSchemaVersion.FromJson(versionString.c_str());
    PackageSchemaVersion expectedPackageVersion (PACKAGE_CURRENT_VERSION_Major, PACKAGE_CURRENT_VERSION_Minor, PACKAGE_CURRENT_VERSION_Sub1, PACKAGE_CURRENT_VERSION_Sub2);
    PackageSchemaVersion minimumAutoUpgradablePackageVersion (PACKAGE_SUPPORTED_VERSION_Major, PACKAGE_SUPPORTED_VERSION_Minor, PACKAGE_SUPPORTED_VERSION_Sub1, PACKAGE_SUPPORTED_VERSION_Sub2);

    bool isProfileAutoUpgradable = false; //unused as this method is not attempting to auto-upgrade
    dbResult = Db::CheckProfileVersion (isProfileAutoUpgradable, expectedPackageVersion, actualPackageSchemaVersion, minimumAutoUpgradablePackageVersion, db.IsReadonly(), "Package");
    switch (dbResult)
        {
        case BE_SQLITE_ERROR_ProfileTooOld:
            return DGNSCHEMA_STATUS_VersionTooOld;
        case BE_SQLITE_ERROR_ProfileTooNew:
            return DGNSCHEMA_STATUS_VersionTooNew;
        default:
            return DGNFILE_STATUS_Success;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
DgnFileStatus DgnProjectPackage::ExtractUsingDefaults(DbResult& dbResult, BeFileNameCR dgndbFile, BeFileNameCR packageFile, bool overwriteExisting, ICompressProgressTracker* progress)
    {
    if (!BeFileName::DoesPathExist(packageFile))
        return DGNOPEN_STATUS_FileNotFound;

    Utf8CP dbName = NULL;
    Db  db;
    Db::OpenParams openParams(Db::OPEN_Readonly);

    Utf8String   utf8Name;
    //  Make dbName match the name + extension of the package, but with the trailing z removed.
    WString   temp;
    WString   tempExt;
    packageFile.ParseName(NULL, NULL, &temp, &tempExt);
    utf8Name.Sprintf("%s.%s", Utf8String(temp.c_str()).c_str(), Utf8String(tempExt.c_str()).c_str());
    utf8Name.resize(utf8Name.size()-1);
    dbName = utf8Name.c_str();

    if ((dbResult = db.OpenBeSQLiteDb(packageFile, openParams)) != BE_SQLITE_OK)
        return DGNFILE_ERROR_SQLiteError;

    DgnFileStatus schemaStatus = performPackageVersionChecks(dbResult, db);
    if (DGNFILE_STATUS_Success != schemaStatus)
        return DGNFILE_ERROR_InvalidFileSchema;

    DbEmbeddedFileTable& embeddedFiles = db.EmbeddedFiles();
    Utf8String  fileType;
    BeRepositoryBasedId id = embeddedFiles.QueryFile(dbName, NULL, NULL, NULL, &fileType);

    if (!id.IsValid())
        {
        DbEmbeddedFileTable::Iterator iterator = embeddedFiles.MakeIterator();
        //  If there is only 1 use it regardless of name.
        if (iterator.QueryCount() != 1)
            {
            dbResult = BE_SQLITE_NOTFOUND;
            return DGNFILE_ERROR_SQLiteError;
            }

        DbEmbeddedFileTable::Iterator::Entry entry = iterator.begin();
        utf8Name = entry.GetNameUtf8();
        dbName = utf8Name.c_str();

        id = embeddedFiles.QueryFile(dbName, NULL, NULL, NULL, &fileType);
        }

    if (!id.IsValid() || strcmp("dgndb", fileType.c_str()))
        {
        //  If there is only 1 use it regardless of name.
        dbResult = BE_SQLITE_NOTFOUND;
        return DGNFILE_ERROR_SQLiteError;
        }

    if (DGNFILE_STATUS_Success != (schemaStatus = performEmbeddedProjectVersionChecks(dbResult, db, id)))
        return schemaStatus;

    if (BeFileName::DoesPathExist(dgndbFile.GetName()))
        {
        if (!overwriteExisting)
            return DGNOPEN_STATUS_FileAlreadyExists;

        if (BeFileNameStatus::Success != BeFileName::BeDeleteFile (dgndbFile))
            return DGNOPEN_STATUS_FileAlreadyExists;
        }

    Utf8String  u8outputName(dgndbFile.GetName());
    if ((dbResult = embeddedFiles.Export (u8outputName.c_str(), dbName, progress)) != BE_SQLITE_OK)
        return DGNFILE_ERROR_SQLiteError;

    return DGNFILE_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
DgnFileStatus DgnProjectPackage::Extract (BeSQLite::DbResult& dbResult, Utf8CP outputDirectory, Utf8CP dbName, BeFileNameCR packageFile, bool overwriteExisting, ICompressProgressTracker* progress)
    {
    if (!BeFileName::DoesPathExist(packageFile))
        return DGNOPEN_STATUS_FileNotFound;

    BeSQLite::Db        db;
    Db::OpenParams      openParams(Db::OPEN_Readonly);

    Utf8String   utf8Name;
    if (NULL == dbName)
        {
        //  Make dbName match the name + extension of the package, but with the trailing z removed.
        WString   temp;
        WString   tempExt;
        packageFile.ParseName(NULL, NULL, &temp, &tempExt);
        utf8Name.Sprintf("%ls.%ls", temp.c_str(), tempExt.c_str());
        utf8Name.resize(utf8Name.size()-1);
        dbName = utf8Name.c_str();
        }

    if ((dbResult = db.OpenBeSQLiteDb(packageFile, openParams)) != BE_SQLITE_OK)
        return DGNFILE_ERROR_SQLiteError;

    Utf8String versionString;
    dbResult = db.QueryProperty (versionString, PackageProperty::SchemaVersion());
    if (BE_SQLITE_ROW != dbResult)
        return DGNPROJECT_ERROR_InvalidSchemaVersion;

    PackageSchemaVersion packageSchemaVersion(0,0,0,0);
    packageSchemaVersion.FromJson(versionString.c_str());
    PackageSchemaVersion currentPackageVersion (PACKAGE_CURRENT_VERSION_Major, PACKAGE_CURRENT_VERSION_Minor, PACKAGE_CURRENT_VERSION_Sub1, PACKAGE_CURRENT_VERSION_Sub2);
    PackageSchemaVersion supportedPackageVersion (PACKAGE_SUPPORTED_VERSION_Major, PACKAGE_SUPPORTED_VERSION_Minor, PACKAGE_SUPPORTED_VERSION_Sub1, PACKAGE_SUPPORTED_VERSION_Sub2);

    if (packageSchemaVersion.CompareTo(supportedPackageVersion, PackageSchemaVersion::VERSION_MajorMinor) < 0)
        return DGNSCHEMA_STATUS_VersionTooOld;

    if (packageSchemaVersion.CompareTo(currentPackageVersion,   PackageSchemaVersion::VERSION_MajorMinor) > 0)
        return DGNSCHEMA_STATUS_VersionTooNew;

    DbEmbeddedFileTable& embeddedFiles = db.EmbeddedFiles();
    Utf8String  fileType;
    BeRepositoryBasedId id = embeddedFiles.QueryFile(dbName, NULL, NULL, NULL, &fileType);
    if (!id.IsValid() || strcmp("dgndb", fileType.c_str()))
        {
        dbResult = BE_SQLITE_NOTFOUND;
        return DGNFILE_ERROR_SQLiteError;
        }

    dbResult = db.QueryProperty (versionString, DgnEmbeddedProjectProperty::SchemaVersion(), id.GetValue());
    if (BE_SQLITE_ROW != dbResult)
        return DGNPROJECT_ERROR_InvalidSchemaVersion;

    DgnVersion dgnSchemaVersion(0,0,0,0);
    dgnSchemaVersion.FromJson(versionString.c_str());
    DgnVersion currentVersion (PROJECT_CURRENT_VERSION_Major, PROJECT_CURRENT_VERSION_Minor, PROJECT_CURRENT_VERSION_Sub1, PROJECT_CURRENT_VERSION_Sub2);
    DgnVersion supportedVersion (PROJECT_SUPPORTED_VERSION_Major, PROJECT_SUPPORTED_VERSION_Minor, 0, 0);

    if (dgnSchemaVersion.CompareTo(supportedVersion, DgnVersion::VERSION_MajorMinor) < 0)
        return DGNSCHEMA_STATUS_VersionTooOld;

    if (dgnSchemaVersion.CompareTo(currentVersion,   DgnVersion::VERSION_MajorMinor) > 0)
        return DGNSCHEMA_STATUS_VersionTooNew;

    WString wcOutputDirectory;
    WString wcDbName;

    wcOutputDirectory.AssignUtf8(outputDirectory);
    BeFileName::AppendSeparator(wcOutputDirectory);
    wcDbName.AssignUtf8(dbName);

    BeFileName  outputFileName;
    WString dev, dir, name, ext;
    BeFileName::ParseName(&dev, &dir, NULL, NULL, wcOutputDirectory.c_str());
    BeFileName::ParseName(NULL, NULL, &name, &ext, wcDbName.c_str());

    outputFileName.BuildName(dev.c_str(), dir.c_str(), name.c_str(), ext.c_str());

    if (BeFileName::DoesPathExist(outputFileName))
        {
        if (!overwriteExisting)
            return DGNOPEN_STATUS_FileAlreadyExists;

        if (BeFileNameStatus::Success != BeFileName::BeDeleteFile (outputFileName))
            return DGNOPEN_STATUS_FileAlreadyExists;
        }

    Utf8String  u8outputName(outputFileName.GetName());

    if ((dbResult = embeddedFiles.Export (u8outputName.c_str(), dbName, progress)) != BE_SQLITE_OK)
        return DGNFILE_ERROR_SQLiteError;

    return DGNFILE_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
static void copyProjectPropertyToPackage(BeSQLite::Db& package, DgnProject& sourceProject, PropertySpecCR sourceSpec, PropertySpecCR targetSpec, BeRepositoryBasedId&embeddedProjectId)
    {
    Utf8String  propertyStr;

    if (sourceProject.QueryProperty(propertyStr, sourceSpec) != BE_SQLITE_ROW)
        return;  //  Nothing to copy

    package.SavePropertyString(targetSpec, propertyStr, embeddedProjectId.GetValue());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
DgnFileStatus DgnProjectPackage::CreatePackage (BeFileNameCR packageFile, BeFileNameCR dgndbFile, CreatePackageParams createParams)
    {
    DgnFileStatus       fileStatus;
    DgnProjectPtr  sourceProject = DgnProject::OpenProject(&fileStatus, dgndbFile, DgnProject::OpenParams(Db::OPEN_Readonly));
    if (!sourceProject.IsValid())
        return fileStatus;

    Utf8String  dgndbFileUtf8 (dgndbFile.GetName());

    WString     base;
    WString     ext;
    dgndbFile.ParseName(NULL, NULL, &base, &ext);

    WString     embeddedName;
    BeFileName::BuildName(embeddedName, NULL, NULL, base.c_str(), ext.c_str());
    Utf8String  embeddedUtf8(embeddedName.c_str());

    createParams.SetStartDefaultTxn (DefaultTxn_Exclusive);

    if (createParams.m_overwriteExisting && BeFileName::DoesPathExist(packageFile))
        {
        if (BeFileNameStatus::Success != BeFileName::BeDeleteFile (packageFile))
            {
            LOG.errorv(L"Unable to create DgnPackage because '%ls' cannot be deleted.", packageFile.GetName());
            return DGNOPEN_STATUS_FileAlreadyExists;
            }
        }

    BeSQLite::Db    db;
    if (BE_SQLITE_OK !=  db.CreateNewDb (packageFile, createParams.GetGuid(), createParams))
        {
        return DGNFILE_ERROR_SQLiteError;
        }

    DbEmbeddedFileTable& embeddedFiles = db.EmbeddedFiles();
    if (BE_SQLITE_OK != embeddedFiles.Import(embeddedUtf8.c_str(), dgndbFileUtf8.c_str(), "dgndb", NULL, createParams.m_chunkSize))
        return DGNFILE_ERROR_SQLiteError;

    //  Imported the file into a compressed format.  Now copy properties.
    BeRepositoryBasedId id = embeddedFiles.QueryFile (embeddedUtf8.c_str(), NULL, NULL, NULL, NULL);

    DgnViewId defaultViewID;
    if (BE_SQLITE_ROW != sourceProject->QueryProperty(&defaultViewID, (UInt32)sizeof(defaultViewID), DgnViewProperty::DefaultView()))
        {
        Statement firstViewStatement;
        firstViewStatement.Prepare(*sourceProject, "SELECT Id FROM " DGN_TABLE_View " LIMIT 1");

        if (BE_SQLITE_ROW == firstViewStatement.Step())
            defaultViewID = firstViewStatement.GetValueId<DgnViewId>(0);
        }

    DbResult result;
    if (defaultViewID.IsValid())  //  This should be valid unless the project has no views
        {
        Statement stmt;
        result = stmt.Prepare (*sourceProject, "SELECT SubId from be_Prop where Namespace = 'dgn_View' AND Name = 'Thumbnail' AND Id=? LIMIT 1");
        BeAssert(BE_SQLITE_OK == result);
        stmt.BindInt64(1, defaultViewID.GetValue());
        result = stmt.Step();
        if (BE_SQLITE_ROW == result)
            {
            UInt32 imageSize;
            UInt32  resolution = (UInt32)stmt.GetValueInt(0);
            result = sourceProject->QueryPropertySize(imageSize, DgnViewProperty::Thumbnail(), defaultViewID.GetValue(), resolution);
            BeAssert(BE_SQLITE_ROW == result);

            ScopedArray<byte>   thumbnail(imageSize);
            result = sourceProject->QueryProperty(thumbnail.GetData(), imageSize, DgnViewProperty::Thumbnail(), defaultViewID.GetValue(), resolution);
            if (BE_SQLITE_ROW != result)
                { BeAssert(false); }

            result = db.SaveProperty(DgnEmbeddedProjectProperty::Thumbnail(), thumbnail.GetData(), imageSize, id.GetValue(), resolution);
            BeAssert (BE_SQLITE_OK == result);
            }
        }

    PackageSchemaVersion  schemaVersion(PACKAGE_CURRENT_VERSION_Major, PACKAGE_CURRENT_VERSION_Minor, PACKAGE_CURRENT_VERSION_Sub1, PACKAGE_CURRENT_VERSION_Sub2);
    db.SavePropertyString (PackageProperty::SchemaVersion(), schemaVersion.ToJson());
    db.SaveCreationDate();

    copyProjectPropertyToPackage(db, *sourceProject, DgnProjectProperty::SchemaVersion(), DgnEmbeddedProjectProperty::SchemaVersion(), id);
    copyProjectPropertyToPackage(db, *sourceProject, DgnProjectProperty::Name(), DgnEmbeddedProjectProperty::Name(), id);
    copyProjectPropertyToPackage(db, *sourceProject, DgnProjectProperty::Description(), DgnEmbeddedProjectProperty::Description(), id);
    copyProjectPropertyToPackage(db, *sourceProject, DgnProjectProperty::Client(), DgnEmbeddedProjectProperty::Client(), id);
    copyProjectPropertyToPackage(db, *sourceProject, DgnProjectProperty::LastEditor(), DgnEmbeddedProjectProperty::LastEditor(), id);
    copyProjectPropertyToPackage(db, *sourceProject, Properties::CreationDate(), DgnEmbeddedProjectProperty::CreationDate(), id);
    copyProjectPropertyToPackage(db, *sourceProject, Properties::ExpirationDate(), DgnEmbeddedProjectProperty::ExpirationDate(), id);

    return DGNFILE_STATUS_Success;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Sam.Wilson      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ElementProvenance DgnProvenances::QueryProvenance (ElementId newElementId)
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT SourceFile,SourceId FROM " DGN_TABLE_ProvElem " WHERE Id=?");
    stmt.BindInt64 (1, newElementId.GetValue());
    return (BE_SQLITE_ROW != stmt.Step()) ? ElementProvenance(0,0) : ElementProvenance (stmt.GetValueInt64(0), stmt.GetValueInt64(1));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Sam.Wilson      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnProvenances::QueryFileName (Utf8StringR oldFileName, Int64 foreignFileId)
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT OrigName,EmbedId,EmbeddedIn FROM " DGN_TABLE_ProvFile " WHERE Id=?");
    stmt.BindInt64 (1, foreignFileId);
    if (BE_SQLITE_ROW != stmt.Step())
        return ERROR;

    oldFileName = stmt.GetValueText(0);
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Sam.Wilson      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnProvenances::QueryFileId (UInt64& oldFileId, Utf8CP origName)
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT Id FROM " DGN_TABLE_ProvFile " WHERE OrigName=?");
    stmt.BindText (1, origName, Statement::MAKE_COPY_No);
    if (BE_SQLITE_ROW != stmt.Step())
        return ERROR;

    oldFileId = stmt.GetValueInt64(0);
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Sam.Wilson      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ElementId DgnProvenances::GetElement (ElementProvenance const& provenance)
    {
    Statement stmt;
    stmt.Prepare (m_project, "SELECT Id FROM " DGN_TABLE_ProvElem " WHERE SourceFile=? AND SourceId=?");
    stmt.BindInt64 (1, provenance.GetOriginalFileId());
    stmt.BindInt64 (2, provenance.GetOriginalElementId());

    return (BE_SQLITE_ROW != stmt.Step()) ? ElementId() : stmt.GetValueId<ElementId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
void DgnUnits::GeoTransform::Init (DgnUnits const& units)
    {
    if (m_isValid)
        return;

    static double s_wgs84Major = 6371837.0;

    m_xRadius = s_wgs84Major * cos (units.GetOriginLatitude() * msGeomConst_radiansPerDegree);
    m_yRadius = s_wgs84Major * cos (units.GetOriginLatitude() * msGeomConst_radiansPerDegree);

    m_matrix = RotMatrix::FromAxisAndRotationAngle (2, -1.0 * units.GetAzimuth() * msGeomConst_radiansPerDegree);
    m_inverseMatrix.InverseOf (m_matrix);
    m_isValid = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool DgnUnits::CanConvertBetweenGeoAndWorld() const
    {
    return m_hasGeoOriginBasis;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     12/13
//---------------------------------------------------------------------------------------
bool DgnUnits::IsGeoPointWithinGeoCoordinateSystemWorkingArea (GeoPointCR point) const
    {
    return (fabs (point.longitude - m_longitude) > m_geoCoordWorkingAreaInDegrees
         || fabs (point.latitude  - m_latitude)  > m_geoCoordWorkingAreaInDegrees);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     12/13
//---------------------------------------------------------------------------------------
bool DgnUnits::IsWorldPointWithinGeoCoordinateSystemWorkingArea (DPoint3dCR point) const
    {
    DPoint2d deltaFromOriginInMeters;
    deltaFromOriginInMeters.DifferenceOf (m_geoOriginBasis, DPoint2d::From (point.x, point.y));
    deltaFromOriginInMeters.Scale (1000.); // meters
    return deltaFromOriginInMeters.MagnitudeSquared() < m_geoCoordWorkingAreaInMetersSquared;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnUnits::ConvertToWorldPoint (DPoint3dR worldPoint, GeoPointCR geoPoint) const
    {
    if (!CanConvertBetweenGeoAndWorld() || !IsGeoPointWithinGeoCoordinateSystemWorkingArea(geoPoint))
        return ERROR;

    m_geoTransform.Init (*this);

    DPoint3d delta;
    delta.x = (geoPoint.longitude - m_longitude) * m_geoTransform.m_xRadius * msGeomConst_radiansPerDegree;
    delta.y = (geoPoint.latitude -  m_latitude)  * m_geoTransform.m_yRadius * msGeomConst_radiansPerDegree;
    delta.z = (geoPoint.elevation);

    m_geoTransform.m_inverseMatrix.Multiply (delta);
    delta.Scale (1000.); // meters
    worldPoint.SumOf (DPoint3d::From (m_geoOriginBasis), delta);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnUnits::ConvertToGeoPoint (GeoPointR geoPoint, DPoint3dCR worldPoint) const
    {
    if (!CanConvertBetweenGeoAndWorld() || !IsWorldPointWithinGeoCoordinateSystemWorkingArea(worldPoint))
        return ERROR;

    m_geoTransform.Init (*this);

    DPoint3d delta;
    delta.DifferenceOf (worldPoint, DPoint3d::From (m_geoOriginBasis));
    delta.Scale (1.0 / 1000.);
    m_geoTransform.m_matrix.Multiply (delta);

    geoPoint.longitude = m_longitude + msGeomConst_degreesPerRadian * delta.x / m_geoTransform.m_xRadius;
    geoPoint.latitude  = m_latitude  + msGeomConst_degreesPerRadian * delta.y / m_geoTransform.m_yRadius;
    geoPoint.elevation = delta.z;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
void DgnUnits::SetGeoOriginBasis (DPoint2dCR basis)
    {
    m_geoOriginBasis = basis;
    m_hasGeoOriginBasis = true;
    m_geoTransform.m_isValid = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
void DgnUnits::SetOriginLatitude (double originLat)
    {
    m_latitude = originLat;
    m_geoTransform.m_isValid = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
void DgnUnits::SetOriginLongitude (double originLong)
    {
    m_longitude = originLong;
    m_geoTransform.m_isValid = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
void DgnUnits::SetAzimuth (double azimuth)
    {
    m_azimuth = azimuth;
    m_geoTransform.m_isValid = false;
    }

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnStamps::InsertStamp(StampName& stampName, StampData const& data)
    {
    if (!stampName.IsNameValid())
        return BE_SQLITE_ERROR;

    // if the stampname.id1 is invalid, assign it the first available id
    if (!stampName.m_id1.IsValid())
        {
        NamedParams whereParam("Namespace=@ns AND Name=@n");
        whereParam.AddStringParameter ("@ns", stampName.GetNamespace().c_str());
        whereParam.AddStringParameter ("@n", stampName.GetName().c_str());
        m_project.GetNextRepositoryBasedId (stampName.m_id1, DGN_TABLE_Stamp, "Id1", &whereParam);
        }

    if (stampName.m_id2 < 0)
        stampName.m_id2 = 0; // don't allow negative id2 values

    Statement stmt;
    stmt.Prepare(m_project, "INSERT INTO " DGN_TABLE_Stamp "(Namespace,Name,Id1,Id2,Data) VALUES(?,?,?,?,?)");
    stmt.BindText(1, stampName.GetNamespace(), Statement::MAKE_COPY_No);
    stmt.BindText(2, stampName.GetName(), Statement::MAKE_COPY_No);
    stmt.BindId(3, stampName.GetId1());
    stmt.BindInt64(4, stampName.GetId2());
    stmt.BindBlob(5, data.begin(), (int) data.size(), Statement::MAKE_COPY_No);

    DbResult status = stmt.Step();
    return (BE_SQLITE_DONE == status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnStamps::UpdateStamp(StampName const& stampName, StampData const& data)
    {
    if (!stampName.IsValid())
        return BE_SQLITE_ERROR;

    m_cache.Drop(stampName);

    Statement stmt;
    stmt.Prepare(m_project, "UPDATE " DGN_TABLE_Stamp " SET Data=?5 WHERE Namespace=?1 AND Name=?2 AND Id1=?3 AND Id2=?4");
    stmt.BindText(1, stampName.GetNamespace(), Statement::MAKE_COPY_No);
    stmt.BindText(2, stampName.GetName(), Statement::MAKE_COPY_No);
    stmt.BindId(3, stampName.GetId1());
    stmt.BindInt64(4, stampName.GetId2());
    stmt.BindBlob(5, data.begin(), (int) data.size(), Statement::MAKE_COPY_No);

    DbResult status = stmt.Step();
    return (BE_SQLITE_DONE == status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnStamps::DeleteStamp(StampName const& stampName)
    {
    if (stampName.GetNamespace().empty() || stampName.GetName().empty())
        return BE_SQLITE_ERROR;

    if (stampName.IsValid())
        m_cache.Drop(stampName);
    else
        m_cache.Clear(); // potentially dropping more than one entry. Pretty extreme, but ok for now

    Utf8String sql = "DELETE FROM " DGN_TABLE_Stamp " WHERE Namespace=?1 AND Name=?2";
    if (stampName.GetId1().IsValid())
        {
        sql += " AND Id1=?3";
        if (stampName.GetId2() >= 0)
            sql += " AND Id2=?4";
        }

    Statement stmt;
    stmt.Prepare(m_project, sql.c_str());
    stmt.BindText(1, stampName.GetNamespace(), Statement::MAKE_COPY_No);
    stmt.BindText(2, stampName.GetName(), Statement::MAKE_COPY_No);

    if (stampName.GetId1().IsValid())
        {
        stmt.BindId(3, stampName.GetId1());
        if (stampName.GetId2() >= 0)
            stmt.BindInt64(4, stampName.GetId2());
        }

    DbResult status = stmt.Step();
    return (BE_SQLITE_DONE == status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStamps::StampDataPtr DgnStamps::FindStamp(StampName const& stampName)
    {
    if (!stampName.IsValid())
        return NULL;

    auto stampData = m_cache.Find(stampName);
    if (!stampData.IsValid())
        {
        HighPriorityOperationBlock __v;
        CachedStatementPtr stmt;
        m_project.GetCachedStatement (stmt, "SELECT Data FROM " DGN_TABLE_Stamp " WHERE Namespace=?1 AND Name=?2 AND Id1=?3 AND Id2=?4");
        stmt->BindText(1, stampName.GetNamespace(), Statement::MAKE_COPY_No);
        stmt->BindText(2, stampName.GetName(), Statement::MAKE_COPY_No);
        stmt->BindId(3, stampName.GetId1());
        stmt->BindInt64(4, stampName.GetId2());

        // save an entry in the cache for misses too, so we don't keep looking for them
        stampData = (BE_SQLITE_ROW != stmt->Step()) ? new StampData() : new StampData((UInt8 const*) stmt->GetValueBlob(0), stmt->GetColumnBytes(0));
        m_cache.Add(stampName, *stampData);
        }

    return stampData->size()>0 ? stampData : NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
StampQvElemMapR DgnProject::GetStampQvElemMap()
    {
    if (NULL == m_stampQvElemMap)
        m_stampQvElemMap = ViewContext::CreateSymbolStampMap(*this);

    return *m_stampQvElemMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
StampQvElemMapP DgnProject::GetStampQvElemMapP()
    {
    return m_stampQvElemMap;
    }

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  07/2014
//=======================================================================================
struct ElementLoadedEventCaller
{
PersistentElementRefR m_elRef;

ElementLoadedEventCaller (PersistentElementRefR elRef) : m_elRef (elRef) {}
void CallHandler (IElementLoadedEvent& handler) const {handler._OnElementLoaded (m_elRef);}

}; // ElementLoadedEventCaller

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProject::SendElementLoadedEvent (PersistentElementRefR elRef) const
    {
    if (NULL == m_elementLoadedListeners)
        return;

    m_elementLoadedListeners->CallAllHandlers (ElementLoadedEventCaller (elRef));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings   07/2014
//---------------------------------------------------------------------------------------
void DgnProject::AddListener (IElementLoadedEvent* listener)
    {
    if (NULL == m_elementLoadedListeners)
        m_elementLoadedListeners = new EventHandlerList<IElementLoadedEvent>;

    m_elementLoadedListeners->AddHandler (listener);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings   07/2014
//---------------------------------------------------------------------------------------
void DgnProject::DropListener (IElementLoadedEvent* listener)
    {
    if (NULL != m_elementLoadedListeners)
        m_elementLoadedListeners->DropHandler (listener);
    }

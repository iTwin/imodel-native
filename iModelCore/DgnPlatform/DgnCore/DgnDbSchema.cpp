/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnDbSchema.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnVersion getCurrentSchemaVerion()
    {
    return DgnVersion(DGNDB_CURRENT_VERSION_Major, DGNDB_CURRENT_VERSION_Minor, DGNDB_CURRENT_VERSION_Sub1, DGNDB_CURRENT_VERSION_Sub2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static void importDgnSchema (DgnDbR project, bool updateExisting)
    {
    ECN::ECSchemaReadContextPtr ecSchemaContext = ECN::ECSchemaReadContext::CreateContext();
    ecSchemaContext->AddSchemaLocater (project.GetSchemaLocater());

    BeFileName ecSchemaPath = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    ecSchemaPath.AppendToPath (L"ECSchemas");

    BeFileName dgnSchemaPath = ecSchemaPath;
    dgnSchemaPath.AppendToPath (L"Dgn");
    ecSchemaContext->AddSchemaPath (dgnSchemaPath);

    BeFileName standardSchemaPath = ecSchemaPath;
    standardSchemaPath.AppendToPath (L"Standard");
    ecSchemaContext->AddSchemaPath (standardSchemaPath);

    SchemaKey dgnschemaKey (L"dgn", 2, 0);
    ECSchemaPtr dgnschema = ECSchema::LocateSchema (dgnschemaKey, *ecSchemaContext);
    BeAssert (dgnschema != NULL);

    BentleyStatus status = project.Schemas().ImportECSchemas (ecSchemaContext->GetCache(),
        ECDbSchemaManager::ImportOptions (false, updateExisting));

    BeAssert (status == SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateProjectTables()
    {
    CreateTable(DGN_TABLE_Domain,    "Name CHAR UNIQUE NOT NULL PRIMARY KEY,"
                                     "Descr CHAR,"
                                     "Version INTEGER");

    CreateTable(DGN_TABLE_Handler,   "ClassId INTEGER PRIMARY KEY,"
                                     "Domain CHAR NOT NULL REFERENCES " DGN_TABLE_Domain "(Name),"
                                     "Name CHAR NOT NULL COLLATE NOCASE,"
                                     "Permissions INT,"
                                     "CONSTRAINT names UNIQUE(Domain,Name)");

    CreateTable(DGN_TABLE_Font,      "Id INTEGER PRIMARY KEY,"
                                     "Type INT,"
                                     "Name CHAR NOT NULL COLLATE NOCASE,"
                                     "CONSTRAINT names UNIQUE(Type,Name)");

    importDgnSchema(*this, false);

    CreateTable(DGN_TABLE_Session,   "Id INTEGER PRIMARY KEY,"
                                     "Name CHAR NOT NULL COLLATE NOCASE UNIQUE,"
                                     "Descr CHAR,"
                                     "Data BLOB");

    ExecuteSql("CREATE TRIGGER delete_viewProps AFTER DELETE ON " DGN_TABLE(DGN_CLASSNAME_View) " BEGIN DELETE FROM " BEDB_TABLE_Property
                    " WHERE Namespace=\"" PROPERTY_APPNAME_DgnView "\" AND Id=OLD.Id; END");

    ExecuteSql("CREATE VIRTUAL TABLE " DGN_VTABLE_PrjRTree " USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)");

    ExecuteSql("CREATE TRIGGER dgn_prjrange_del AFTER DELETE ON " DGN_TABLE(DGN_CLASSNAME_ElementGeom)
               " BEGIN DELETE FROM " DGN_VTABLE_PrjRTree " WHERE ElementId=old.ElementId; END");

    CreateTable(DGN_TABLE_RasterFile,"RasterId INTEGER PRIMARY KEY,"
                                            "Name CHAR NOT NULL,"
                                            "Header BLOB");

    CreateTable(DGN_TABLE_RasterData,"RasterId INTEGER,"
                                            "ResolutionLevel INT,"
                                            "RowBlockId INT,"
                                            "ColumnBlockId INT,"
                                            "Data BLOB");

    ExecuteSql("CREATE INDEX dgn_rasterDataIdx ON " DGN_TABLE_RasterData "(RasterId,ResolutionLevel,RowBlockId,ColumnBlockId)");
    ExecuteSql("CREATE TRIGGER delete_rasterData AFTER DELETE ON " DGN_TABLE_RasterFile " BEGIN DELETE FROM " DGN_TABLE_RasterData " WHERE RasterId=OLD.RasterId; END");

    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::SaveDgnDbSchemaVersion(DgnVersion version)
    {
    m_schemaVersion = version;
    return  SavePropertyString (DgnProjectProperty::SchemaVersion(), m_schemaVersion.ToJson());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::InitializeDgnDb (CreateDgnDbParams const& params)
    {
    if (params.GetGuid().IsValid())
        ChangeDbGuid(params.GetGuid());

    SaveDgnDbSchemaVersion();
    SaveCreationDate();
    SaveCreationBeSqliteBuildVersion();

    Domains().OnDbOpened();

    SavePropertyString(DgnProjectProperty::LastEditor(), Utf8String(T_HOST.GetProductName()));
    SavePropertyString(DgnProjectProperty::Name(), params.m_name);
    SavePropertyString(DgnProjectProperty::Description(), params.m_description);
    SavePropertyString(DgnProjectProperty::Client(), params.m_client);

    m_units.Save();

    SaveChanges();

    return  BE_SQLITE_OK;
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/13
//=======================================================================================
struct ProjectSchemaUpgrader
{
    virtual DgnVersion _GetVersion() = 0;
    virtual DbResult _Upgrade (DgnDbR project, DgnVersion version) = 0;
};


#if defined (WHEN_FIRST_UPGRADER)
static ProjectSchemaUpgrader* s_upgraders[] =
    {
    // NOTE: entries in this list *must* be sorted in ascending version order.
    // Add a new version here

    };
#endif

/*---------------------------------------------------------------------------------**//**
* Each call to _DoUpgrade will upgrade the schema from its stored version to the immediately succeeding version.
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::OpenParams::_DoUpgrade (DgnDbR project, DgnVersion& version) const
    {
#if defined (WHEN_FIRST_UPGRADER)
    for (auto upgrader : s_upgraders)
        {
        if (version < upgrader->_GetVersion())
            {
            DbResult stat = upgrader->_Upgrade (project, version);
            if (stat == BE_SQLITE_OK)
                version = upgrader->_GetVersion();

            return stat;
            }
        }

    BeAssert (false); // if no upgrade code necessary, add an upgrader that does nothing
    version = getCurrentSchemaVerion();
#endif
    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* The schema stored in the newly opened project is too old. Perform an upgrade, if possible.
* @bsimethod                                    Keith.Bentley                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::OpenParams::UpgradeSchema(DgnDbR project) const
    {
    if (!_ReopenForSchemaUpgrade (project))
        return BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite;

    DgnVersion version = project.GetSchemaVersion();
    for (;;)
        {
        DbResult stat = _DoUpgrade(project, version);
        if (BE_SQLITE_OK != stat)
            return stat;

        project.SaveDgnDbSchemaVersion(version);

        // Stop when we get to the current version.
        if (getCurrentSchemaVerion() == version)
            break;
        }

    return project.SaveChanges();
    }

DgnVersion DgnDb::GetSchemaVersion() {return m_schemaVersion;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::_VerifySchemaVersion (Db::OpenParams const& params)
    {
    DbResult stat = T_Super::_VerifySchemaVersion(params);
    if (BE_SQLITE_OK != stat)
        return stat;

    Utf8String versionString;
    stat = QueryProperty (versionString, DgnProjectProperty::SchemaVersion());
    if (BE_SQLITE_ROW != stat)
        return BE_SQLITE_ERROR_InvalidProfileVersion;

    m_schemaVersion.FromJson(versionString.c_str());
    DgnVersion expectedVersion = getCurrentSchemaVerion();
    DgnVersion minimumAutoUpgradableVersion (DGNDB_SUPPORTED_VERSION_Major, DGNDB_SUPPORTED_VERSION_Minor, 0, 0);

    bool profileIsAutoUpgradable = false;
    stat = CheckProfileVersion (profileIsAutoUpgradable, expectedVersion, m_schemaVersion, minimumAutoUpgradableVersion, params.IsReadonly(), "DgnDb");

    return profileIsAutoUpgradable ? ((DgnDb::OpenParams&)params).UpgradeSchema(*this) : stat;
    }


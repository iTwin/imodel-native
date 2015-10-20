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
static void importDgnSchema(DgnDbR db, bool updateExisting)
    {
    ECSchemaReadContextPtr ecSchemaContext = ECN::ECSchemaReadContext::CreateContext();
    ecSchemaContext->AddSchemaLocater(db.GetSchemaLocater());

    BeFileName ecSchemaPath = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    ecSchemaPath.AppendToPath(L"ECSchemas");

    BeFileName dgnSchemaPath = ecSchemaPath;
    dgnSchemaPath.AppendToPath(L"Dgn");
    ecSchemaContext->AddSchemaPath(dgnSchemaPath);

    BeFileName standardSchemaPath = ecSchemaPath;
    standardSchemaPath.AppendToPath(L"Standard");
    ecSchemaContext->AddSchemaPath(standardSchemaPath);

    SchemaKey dgnschemaKey("dgn", 2, 0);
    ECSchemaPtr dgnschema = ECSchema::LocateSchema(dgnschemaKey, *ecSchemaContext);
    BeAssert(dgnschema != NULL);

    BentleyStatus status = db.Schemas().ImportECSchemas(ecSchemaContext->GetCache(), ECDbSchemaManager::ImportOptions(false, updateExisting));
    BeAssert(status == SUCCESS);
    }

#define GEOM_IN_PHYSICAL_SPACE_CLAUSE " 1 = new.InPhysicalSpace "

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateDictionaryModel()
    {
    Utf8String dictionaryName = DgnCoreL10N::GetString(DgnCoreL10N::MODELNAME_Dictionary());
    DgnModel::Properties props;
    Json::Value propsValue;
    props.ToJson(propsValue);
    Utf8String propsJson = Json::FastWriter::ToString(propsValue);

    DgnModel::Code modelCode = DgnModel::CreateModelCode(dictionaryName);
    Statement stmt(*this, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Model) " (Id,Code,Descr,ECClassId,Visibility,Props,CodeAuthorityId,CodeNameSpace) VALUES(?,?,'',?,0,?,?,?)");
    stmt.BindId(1, DgnModel::DictionaryId());
    stmt.BindText(2, modelCode.GetValueCP(), Statement::MakeCopy::No);
    stmt.BindId(3, Domains().GetClassId(dgn_ModelHandler::Dictionary::GetHandler()));
    stmt.BindText(4, propsJson.c_str(), Statement::MakeCopy::No);
    stmt.BindId(5, modelCode.GetAuthority());
    stmt.BindText(6, modelCode.GetNameSpace().c_str(), Statement::MakeCopy::No);

    auto result = stmt.Step();
    BeAssert(BE_SQLITE_DONE == result && "Failed to create dictionary model");
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateDgnDbTables()
    {
    CreateTable(DGN_TABLE_Domain,    "Name CHAR UNIQUE NOT NULL PRIMARY KEY,"
                                     "Descr CHAR,"
                                     "Version INTEGER");

    CreateTable(DGN_TABLE_Handler,   "ClassId INTEGER PRIMARY KEY,"
                                     "Domain CHAR NOT NULL REFERENCES " DGN_TABLE_Domain "(Name),"
                                     "Name CHAR NOT NULL COLLATE NOCASE,"
                                     "Permissions INT,"
                                     "CONSTRAINT names UNIQUE(Domain,Name)");

    CreateTable(DGN_TABLE_Txns, "Id INTEGER PRIMARY KEY NOT NULL," 
                           "Deleted BOOL,"
                           "Grouped BOOL,"
                           "Operation CHAR,"
                           "Time TIMESTAMP DEFAULT(julianday('now')),"
                           "Change BLOB");

    Fonts().DbFontMap().CreateFontTable();

    ExecuteSql("CREATE VIRTUAL TABLE " DGN_VTABLE_RTree3d " USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"); // Define this before importing dgn schema!

    importDgnSchema(*this, false);

    // Every DgnDb has a few built-in authorities for element codes
    CreateAuthorities();

    // Every DgnDb has a dictionary model
    CreateDictionaryModel();

    ExecuteSql("CREATE TRIGGER dgn_prjrange_del AFTER DELETE ON " DGN_TABLE(DGN_CLASSNAME_ElementGeom)
               " BEGIN DELETE FROM " DGN_VTABLE_RTree3d " WHERE ElementId=old.ElementId;END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_upd AFTER UPDATE ON " DGN_TABLE(DGN_CLASSNAME_ElementGeom) 
               " WHEN new.Placement IS NOT NULL AND " GEOM_IN_PHYSICAL_SPACE_CLAUSE
               "BEGIN INSERT OR REPLACE INTO " DGN_VTABLE_RTree3d "(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,"
               "DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5)"
               " FROM (SELECT DGN_placement_aabb(NEW.Placement) as bb);END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_upd1 AFTER UPDATE ON " DGN_TABLE(DGN_CLASSNAME_ElementGeom) 
                " WHEN OLD.Placement IS NOT NULL AND NEW.Placement IS NULL"
                " BEGIN DELETE FROM " DGN_VTABLE_RTree3d " WHERE ElementId=OLD.ElementId;END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_ins AFTER INSERT ON " DGN_TABLE(DGN_CLASSNAME_ElementGeom) 
               " WHEN new.Placement IS NOT NULL AND " GEOM_IN_PHYSICAL_SPACE_CLAUSE
               "BEGIN INSERT INTO " DGN_VTABLE_RTree3d "(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,"
               "DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5)"
               " FROM (SELECT DGN_placement_aabb(NEW.Placement) as bb);END");

    ExecuteSql("CREATE TRIGGER delete_viewProps AFTER DELETE ON " DGN_TABLE(DGN_CLASSNAME_View) " BEGIN DELETE FROM " BEDB_TABLE_Property
               " WHERE Namespace=\"" PROPERTY_APPNAME_DgnView "\" AND Id=OLD.Id;END");

    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::SaveDgnDbSchemaVersion(DgnVersion version)
    {
    m_schemaVersion = version;
    return  SavePropertyString(DgnProjectProperty::SchemaVersion(), m_schemaVersion.ToJson());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::InitializeDgnDb(CreateDgnDbParams const& params)
    {
    if (params.GetGuid().IsValid())
        ChangeDbGuid(params.GetGuid());

    SaveDgnDbSchemaVersion();
    SaveCreationDate();

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
    virtual DbResult _Upgrade(DgnDbR project, DgnVersion version) = 0;
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
DbResult DgnDb::OpenParams::_DoUpgrade(DgnDbR project, DgnVersion& version) const
    {
#if defined (WHEN_FIRST_UPGRADER)
    for (auto upgrader : s_upgraders)
        {
        if (version < upgrader->_GetVersion())
            {
            DbResult stat = upgrader->_Upgrade(project, version);
            if (stat == BE_SQLITE_OK)
                version = upgrader->_GetVersion();

            return stat;
            }
        }

    BeAssert(false); // if no upgrade code necessary, add an upgrader that does nothing
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
    if (!_ReopenForSchemaUpgrade(project))
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
DbResult DgnDb::_VerifySchemaVersion(Db::OpenParams const& params)
    {
    DbResult stat = T_Super::_VerifySchemaVersion(params);
    if (BE_SQLITE_OK != stat)
        return stat;

    Utf8String versionString;
    stat = QueryProperty(versionString, DgnProjectProperty::SchemaVersion());
    if (BE_SQLITE_ROW != stat)
        return BE_SQLITE_ERROR_InvalidProfileVersion;

    m_schemaVersion.FromJson(versionString.c_str());
    DgnVersion expectedVersion = getCurrentSchemaVerion();
    DgnVersion minimumAutoUpgradableVersion(DGNDB_SUPPORTED_VERSION_Major, DGNDB_SUPPORTED_VERSION_Minor, 0, 0);

    bool profileIsAutoUpgradable = false;
    stat = CheckProfileVersion(profileIsAutoUpgradable, expectedVersion, m_schemaVersion, minimumAutoUpgradableVersion, params.IsReadonly(), "DgnDb");

    return profileIsAutoUpgradable ?((DgnDb::OpenParams&)params).UpgradeSchema(*this) : stat;
    }

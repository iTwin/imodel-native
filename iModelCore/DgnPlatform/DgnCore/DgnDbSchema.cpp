/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnDbSchema.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::IsCustomHandledProperty(ECN::ECPropertyCR prop) const
    {
    auto customHandledProperty = GetDgnDb().Schemas().GetECClass(BIS_ECSCHEMA_NAME, "CustomHandledProperty");
    if (nullptr == customHandledProperty)
        return false;

    return prop.IsDefined(*customHandledProperty);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::IsCustomHandledProperty(Utf8CP propName) const
    {
    auto eclass = GetElementClass();
    auto prop = eclass->GetPropertyP(propName, true);
    if (nullptr == prop)
        return false;

    return IsCustomHandledProperty(*prop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
AutoHandledPropertiesCollection::Iterator::Iterator(ECN::ECPropertyIterable::const_iterator it, AutoHandledPropertiesCollection const& coll)
    : m_i(it), m_coll(coll) 
    {
    ToNextValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
AutoHandledPropertiesCollection::Iterator& AutoHandledPropertiesCollection::Iterator::operator++()
    {
    BeAssert(m_i != m_coll.m_end);
    ++m_i;
    ToNextValid();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
AutoHandledPropertiesCollection::AutoHandledPropertiesCollection(ECN::ECClassCR eclass, DgnDbR db, ECSqlClassParams::StatementType stype, bool wantCustomHandledProps)
    : m_props(eclass.GetProperties(true)), m_end(m_props.end()), m_stype(stype), m_wantCustomHandledProps(wantCustomHandledProps)
    {
    #ifdef DEBUG_AUTO_HANDLED_PROPERTIES
        printf("%s\n", eclass.GetName().c_str());
        printf("---------------------------\n");
    #endif
    m_customHandledProperty = db.Schemas().GetECClass(BIS_ECSCHEMA_NAME, "CustomHandledProperty");
    m_autoHandledProperty = db.Schemas().GetECClass(BIS_ECSCHEMA_NAME, "AutoHandledProperty");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void AutoHandledPropertiesCollection::Iterator::ToNextValid()
    {
    while (m_i != m_coll.m_end)
        {
        auto prop = *m_i;

        #ifdef DEBUG_AUTO_HANDLED_PROPERTIES
            auto propName = prop->GetName();
            printf ("%s\n", propName.c_str());
            for (auto ca : prop->GetCustomAttributes(true))
                {
                printf ("\t%s\n", ca->GetClass().GetName().c_str());
                for (auto caprop : ca->GetClass().GetProperties())
                    {
                    ECN::ECValue v;
                    if (ECN::ECObjectsStatus::Success == ca->GetValue(v, caprop->GetName().c_str()) && !v.IsNull())
                        printf("\t\t%s %s\n", caprop->GetName().c_str(), v.ToString().c_str());
                    else
                        printf ("\t\t%s (missing)\n", caprop->GetName().c_str());
                    }
                }
        #endif
        
        // Auto-handling is the default. Custom-handling is opt-in. A property must have the CustomHandledProperty CA in order to be custom-handled.
        ECN::IECInstancePtr ca = prop->GetCustomAttribute(*m_coll.m_customHandledProperty);

        bool isCustom = ca.IsValid();

        if (isCustom != m_coll.m_wantCustomHandledProps)
            {
            ++m_i;
            continue;
            }

        if (!m_coll.m_wantCustomHandledProps)
            {
            // A property *may* have an AutoHandledProperty CA, if it wants to specify a statement type.
            ca = prop->GetCustomAttribute(*m_coll.m_autoHandledProperty);
            }

        int32_t stype = (int32_t)ECSqlClassParams::StatementType::All;
        if (ca.IsValid())
            {
            ECN::ECValue v;
            if (ECN::ECObjectsStatus::Success == ca->GetValue(v, "StatementTypes") && !v.IsNull())
                stype = v.GetInteger();
            }

        m_stype = (ECSqlClassParams::StatementType)(stype);

        if (0 != (stype & (int32_t)m_coll.m_stype))
            {
            return; // yes, this property is for the requested statement type
            }

        ++m_i;
        }
    }

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
static void importBisCoreSchema(DgnDbR db)
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

    SchemaKey bisCoreSchemaKey("BisCore", 1, 0);
    ECSchemaPtr bisCoreSchema = ECSchema::LocateSchema(bisCoreSchemaKey, *ecSchemaContext);
    BeAssert(bisCoreSchema != NULL);

    BentleyStatus status = db.Schemas().ImportECSchemas(ecSchemaContext->GetCache());
    BeAssert(status == SUCCESS);
    }

#define GEOM_IN_SPATIAL_INDEX_CLAUSE " 1 = new.InSpatialIndex "
#define ORIGIN_FROM_PLACEMENT "DGN_point(NEW.Origin_X,NEW.Origin_Y,NEW.Origin_Z)"
#define ANGLES_FROM_PLACEMENT "DGN_angles(NEW.Yaw,NEW.Pitch,NEW.Roll)"
#define BBOX_FROM_PLACEMENT "DGN_bbox(NEW.BBoxLow_X,NEW.BBoxLow_Y,NEW.BBoxLow_Z,NEW.BBoxHigh_X,NEW.BBoxHigh_Y,NEW.BBoxHigh_Z)"
#define PLACEMENT_FROM_GEOM "DGN_placement(" ORIGIN_FROM_PLACEMENT "," ANGLES_FROM_PLACEMENT "," BBOX_FROM_PLACEMENT ")"
#define AABB_FROM_PLACEMENT "DGN_placement_aabb(" PLACEMENT_FROM_GEOM ")"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult insertIntoDgnModel(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnElementId modeledElementId, DgnCodeCR modelCode)
    {
    Statement stmt(db, "INSERT INTO " BIS_TABLE(BIS_CLASS_Model) " (Id,ECClassId,ModeledElementId,CodeAuthorityId,CodeNamespace,CodeValue,UserLabel,Visibility) VALUES(?,?,?,?,?,?,'',0)");
    stmt.BindId(1, modelId);
    stmt.BindId(2, classId);
    stmt.BindId(3, modeledElementId);
    stmt.BindId(4, modelCode.GetAuthority());
    stmt.BindText(5, modelCode.GetNamespace().c_str(), Statement::MakeCopy::No);
    stmt.BindText(6, modelCode.GetValueCP(), Statement::MakeCopy::No);

    auto result = stmt.Step();
    BeAssert(BE_SQLITE_DONE == result && "Failed to create model");
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateDictionaryModel()
    {
    DgnModelId modelId = DgnModel::DictionaryId();
    DgnClassId classId = Domains().GetClassId(dgn_ModelHandler::Dictionary::GetHandler());
    DgnCode modelCode = DgnModel::CreateModelCode("Dictionary", BIS_ECSCHEMA_NAME);
    return insertIntoDgnModel(*this, modelId, classId, Elements().GetRootSubjectId(), modelCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateRepositoryModel()
    {
    DgnModelId modelId = DgnModel::RepositoryModelId();
    DgnClassId classId = Domains().GetClassId(dgn_ModelHandler::Repository::GetHandler());
    DgnCode modelCode = DgnModel::CreateModelCode(BIS_CLASS_RepositoryModel, BIS_ECSCHEMA_NAME);
    return insertIntoDgnModel(*this, modelId, classId, Elements().GetRootSubjectId(), modelCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateRootSubject(CreateDgnDbParams const& params)
    {
    DgnElementId elementId = Elements().GetRootSubjectId();
    DgnModelId modelId = DgnModel::RepositoryModelId();
    DgnCode elementCode = DgnCode::CreateEmpty();

    // element handlers are not initialized yet, so insert root Subject directly
    ECSqlStatement statement;
    if (ECSqlStatus::Success != statement.Prepare(*this, "INSERT INTO " BIS_SCHEMA(BIS_CLASS_Subject) " (ECInstanceId,ModelId,CodeAuthorityId,CodeNamespace,CodeValue,UserLabel,Descr) VALUES(?,?,?,?,?,?,?)"))
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    statement.BindId(1, elementId);
    statement.BindId(2, modelId);
    statement.BindId(3, elementCode.GetAuthority());
    statement.BindText(4, elementCode.GetNamespace().c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindText(5, elementCode.GetValueCP(), IECSqlBinder::MakeCopy::No);
    statement.BindText(6, params.m_rootSubjectLabel.c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindText(7, params.m_rootSubjectDescription.c_str(), IECSqlBinder::MakeCopy::No);

    DbResult result = statement.Step();
    BeAssert(BE_SQLITE_DONE == result);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateRepositoryLink(CreateDgnDbParams const& params)
    {
    DgnElementId elementId = Elements().GetRepositoryLinkId();
    DgnModelId modelId = DgnModel::RepositoryModelId();
    DgnCode elementCode = DgnCode::CreateEmpty();
    Utf8String repositoryName(GetFileName().GetFileNameAndExtension());

    // element handlers are not initialized yet, so insert RepositoryLink directly
    ECSqlStatement statement;
    if (ECSqlStatus::Success != statement.Prepare(*this, "INSERT INTO " BIS_SCHEMA(BIS_CLASS_RepositoryLink) " (ECInstanceId,ModelId,CodeAuthorityId,CodeNamespace,CodeValue,UserLabel) VALUES(?,?,?,?,?,?)"))
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    statement.BindId(1, elementId);
    statement.BindId(2, modelId);
    statement.BindId(3, elementCode.GetAuthority());
    statement.BindText(4, elementCode.GetNamespace().c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindText(5, elementCode.GetValueCP(), IECSqlBinder::MakeCopy::No);
    statement.BindText(6, repositoryName.c_str(), IECSqlBinder::MakeCopy::No);

    DbResult result = statement.Step();
    BeAssert(BE_SQLITE_DONE == result);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateDgnDbTables(CreateDgnDbParams const& params)
    {
    CreateTable(DGN_TABLE_Domain,   "[Name] TEXT NOT NULL UNIQUE COLLATE NoCase PRIMARY KEY,"
                                    "[Descr] TEXT,"
                                    "[Version] INTEGER");

    CreateTable(DGN_TABLE_Handler,  "[ClassId] INTEGER PRIMARY KEY,"
                                    "[Domain] TEXT NOT NULL COLLATE NoCase REFERENCES " DGN_TABLE_Domain "([Name]),"
                                    "[Name] TEXT NOT NULL COLLATE NoCase,"
                                    "[Permissions] INTEGER,"
                                    "CONSTRAINT names UNIQUE([Domain],[Name])");

    CreateTable(DGN_TABLE_Txns, "[Id] INTEGER PRIMARY KEY NOT NULL," 
                                "[Deleted] BOOLEAN,"
                                "[Grouped] BOOLEAN,"
                                "[Operation] TEXT,"
                                "[Time] TIMESTAMP DEFAULT(julianday('now')),"
                                "[Change] BLOB");

    Fonts().DbFontMap().CreateFontTable();

    ExecuteSql("CREATE VIRTUAL TABLE " DGN_VTABLE_SpatialIndex " USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"); // Define this before importing dgn schema!

    importBisCoreSchema(*this);

    // Every DgnDb has a few built-in authorities for element codes
    CreateAuthorities();

    // Every DgnDb has a RepositoryModel and a DictionaryModel
    ExecuteSql("PRAGMA defer_foreign_keys = true;"); // the RepositoryModel and primary Subject have foreign keys to each other
    CreateRepositoryModel();
    CreateRootSubject(params);
    ExecuteSql("PRAGMA defer_foreign_keys = false;");
    CreateRepositoryLink(params);
    CreateDictionaryModel();

    // The Generic domain is used when a conversion process doesn't have enough information to pick something better
    if (DgnDbStatus::Success != GenericDomain::ImportSchema(*this))
        {
        BeAssert(false);
        return BE_SQLITE_NOTFOUND;
        }

    ExecuteSql("CREATE TRIGGER dgn_prjrange_del AFTER DELETE ON " BIS_TABLE(BIS_CLASS_GeometricElement3d)
               " BEGIN DELETE FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId=old.ElementId;END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_upd AFTER UPDATE ON " BIS_TABLE(BIS_CLASS_GeometricElement3d) 
               " WHEN new.Origin_X IS NOT NULL AND " GEOM_IN_SPATIAL_INDEX_CLAUSE
               "BEGIN INSERT OR REPLACE INTO " DGN_VTABLE_SpatialIndex "(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,"
               "DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5)"
               " FROM (SELECT " AABB_FROM_PLACEMENT " as bb);END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_upd1 AFTER UPDATE ON " BIS_TABLE(BIS_CLASS_GeometricElement3d) 
                " WHEN OLD.Origin_X IS NOT NULL AND NEW.Origin_X IS NULL"
                " BEGIN DELETE FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId=OLD.ElementId;END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_ins AFTER INSERT ON " BIS_TABLE(BIS_CLASS_GeometricElement3d) 
               " WHEN new.Origin_X IS NOT NULL AND " GEOM_IN_SPATIAL_INDEX_CLAUSE
               "BEGIN INSERT INTO " DGN_VTABLE_SpatialIndex "(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,"
               "DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5)"
               " FROM (SELECT " AABB_FROM_PLACEMENT " as bb);END");

#ifdef NEEDSWORK_VIEW_SETTINGS_TRIGGER
    ExecuteSql("CREATE TRIGGER delete_viewProps AFTER DELETE ON " BIS_TABLE(BIS_CLASS_View) " BEGIN DELETE FROM " BEDB_TABLE_Property
               " WHERE Namespace=\"" PROPERTY_APPNAME_DgnView "\" AND Id=OLD.Id;END");
#endif

    DbResult result = DgnSearchableText::CreateTable(*this);
    BeAssert(BE_SQLITE_OK == result && "Failed to create FTS5 tables");

    return result;
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
    SavePropertyString(DgnProjectProperty::Client(), params.m_client);

    m_units.Save();

    DbResult result = params.m_createStandalone ? Txns().InitializeTableHandlers() : BE_SQLITE_OK;

    SaveChanges();

    return result;
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

#endif
    version = getCurrentSchemaVerion();
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

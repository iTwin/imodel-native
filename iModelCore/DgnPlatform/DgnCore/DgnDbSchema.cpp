/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnDbSchema.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
        printf("%s\n", eclass.GetFullName());
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
                        printf("\t\t%s=%s\n", caprop->GetName().c_str(), v.ToString().c_str());
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
* @bsimethod                                    Ramanujam.Raman                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static void importBisCoreSchema(DgnDbCR db)
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

    BentleyStatus status = db.Schemas().ImportECSchemas(ecSchemaContext->GetCache().GetSchemas(), db.GetECSchemaImportToken());
    BeAssert(status == SUCCESS);
    }

#define GEOM_IN_SPATIAL_INDEX_CLAUSE " 1 = new.InSpatialIndex "
#define ORIGIN_FROM_PLACEMENT "DGN_point(NEW.Origin_X,NEW.Origin_Y,NEW.Origin_Z)"
#define ANGLES_FROM_PLACEMENT "DGN_angles(NEW.Yaw,NEW.Pitch,NEW.Roll)"
#define BBOX_FROM_PLACEMENT "DGN_bbox(NEW.BBoxLow_X,NEW.BBoxLow_Y,NEW.BBoxLow_Z,NEW.BBoxHigh_X,NEW.BBoxHigh_Y,NEW.BBoxHigh_Z)"
#define PLACEMENT_FROM_GEOM "DGN_placement(" ORIGIN_FROM_PLACEMENT "," ANGLES_FROM_PLACEMENT "," BBOX_FROM_PLACEMENT ")"
#define AABB_FROM_PLACEMENT "DGN_placement_aabb(" PLACEMENT_FROM_GEOM ")"
#define OF_SPATIAL_DATA "OF Origin_X,Origin_Y,Origin_Z,Yaw,Pitch,Roll,BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult insertIntoDgnModel(DgnDbR db, DgnElementId modeledElementId, DgnClassId classId)
    {
    Statement stmt(db, "INSERT INTO " BIS_TABLE(BIS_CLASS_Model) " (Id,ECClassId,ModeledElementId,ModeledElementRelECClassId,Visibility) VALUES(?,?,?,?,0)");
    stmt.BindId(1, DgnModelId(modeledElementId.GetValue())); // DgnModelId is the same as the element that it is modeling
    stmt.BindId(2, classId);
    stmt.BindId(3, modeledElementId);
    stmt.BindId(4, db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_REL_ModelModelsElement));

    DbResult result = stmt.Step();
    BeAssert(BE_SQLITE_DONE == result && "Failed to create model");
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreatePartitionElement(Utf8CP className, DgnElementId partitionId, Utf8CP partitionName)
    {
    DgnCode partitionCode(CodeSpecs().QueryCodeSpecId(BIS_CODESPEC_InformationPartitionElement), partitionName, Elements().GetRootSubjectId());

    // element handlers are not initialized yet, so insert DefinitionPartition directly
    Utf8PrintfString sql("INSERT INTO %s (ECInstanceId,Model.Id,Parent.Id,Parent.RelECClassId,CodeSpec.Id,CodeScope,CodeValue) VALUES(?,?,?,?,?,?,?)", className);
    ECSqlStatement statement;
    if (ECSqlStatus::Success != statement.Prepare(*this, sql.c_str(), GetECCrudWriteToken()))
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    statement.BindId(1, partitionId);
    statement.BindId(2, DgnModel::RepositoryModelId());
    statement.BindId(3, Elements().GetRootSubjectId());
    statement.BindId(4, Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_REL_SubjectOwnsPartitionElements));
    statement.BindId(5, partitionCode.GetCodeSpecId());
    statement.BindText(6, partitionCode.GetScope().c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindText(7, partitionCode.GetValueCP(), IECSqlBinder::MakeCopy::No);

    DbResult result = statement.Step();
    BeAssert(BE_SQLITE_DONE == result);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateDictionaryModel()
    {
    DgnElementId modeledElementId = Elements().GetDictionaryPartitionId();
    BeAssert(modeledElementId.GetValue() == DgnModel::DictionaryId().GetValue());

    DbResult result = CreatePartitionElement(BIS_SCHEMA(BIS_CLASS_DefinitionPartition), modeledElementId, BIS_SCHEMA(BIS_CLASS_DictionaryModel));
    if (BE_SQLITE_DONE != result)
        return result;

    DgnClassId classId = Domains().GetClassId(dgn_ModelHandler::Dictionary::GetHandler());
    BeAssert(classId.IsValid());
    return insertIntoDgnModel(*this, modeledElementId, classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateSessionModel()
    {
    DgnElementId modeledElementId = Elements().GetSessionPartitionId();
    DbResult result = CreatePartitionElement(BIS_SCHEMA(BIS_CLASS_DefinitionPartition), modeledElementId, BIS_SCHEMA(BIS_CLASS_SessionModel));
    if (BE_SQLITE_DONE != result)
        return result;

    DgnClassId classId = Domains().GetClassId(dgn_ModelHandler::Session::GetHandler());
    BeAssert(classId.IsValid());
    return insertIntoDgnModel(*this, modeledElementId, classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateRealityDataSourcesModel()
    {
    DgnElementId modeledElementId = Elements().GetRealityDataSourcesPartitionId();
    DbResult result = CreatePartitionElement(BIS_SCHEMA(BIS_CLASS_LinkPartition), modeledElementId, BIS_SCHEMA("RealityDataSources"));
    if (BE_SQLITE_DONE != result)
        return result;

    DgnClassId classId = Domains().GetClassId(dgn_ModelHandler::Link::GetHandler());
    BeAssert(classId.IsValid());
    return insertIntoDgnModel(*this, modeledElementId, classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateRepositoryModel()
    {
    DgnClassId classId = Domains().GetClassId(dgn_ModelHandler::Repository::GetHandler());
    BeAssert(DgnModel::RepositoryModelId().GetValue() == Elements().GetRootSubjectId().GetValue());
    return insertIntoDgnModel(*this, Elements().GetRootSubjectId(), classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateRootSubject(CreateDgnDbParams const& params)
    {
    DgnElementId elementId = Elements().GetRootSubjectId();
    DgnModelId modelId = DgnModel::RepositoryModelId();
    CodeSpecId codeSpecId = CodeSpecs().QueryCodeSpecId(BIS_CODESPEC_Subject);
    DgnCode elementCode = DgnCode(codeSpecId, params.m_rootSubjectName, elementId);

    // element handlers are not initialized yet, so insert root Subject directly
    ECSqlStatement statement;
    if (ECSqlStatus::Success != statement.Prepare(*this, "INSERT INTO " BIS_SCHEMA(BIS_CLASS_Subject) " (ECInstanceId,Model.Id,CodeSpec.Id,CodeScope,CodeValue,Description) VALUES(?,?,?,?,?,?)", GetECCrudWriteToken()))
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    statement.BindId(1, elementId);
    statement.BindId(2, modelId);
    statement.BindId(3, elementCode.GetCodeSpecId());
    statement.BindText(4, elementCode.GetScope().c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindText(5, elementCode.GetValueCP(), IECSqlBinder::MakeCopy::No);
    statement.BindText(6, params.m_rootSubjectDescription.c_str(), IECSqlBinder::MakeCopy::No);

    DbResult result = statement.Step();
    BeAssert(BE_SQLITE_DONE == result);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateDgnDbTables(CreateDgnDbParams const& params)
    {
    CreateTable(DGN_TABLE_Domain,   "Name TEXT NOT NULL UNIQUE COLLATE NoCase PRIMARY KEY,"
                                    "Description TEXT,"
                                    "Version INTEGER");

    CreateTable(DGN_TABLE_Handler,  "ClassId INTEGER PRIMARY KEY,"
                                    "Domain TEXT NOT NULL COLLATE NoCase REFERENCES " DGN_TABLE_Domain "(Name),"
                                    "Name TEXT NOT NULL COLLATE NoCase,"
                                    "Permissions INTEGER,"
                                    "CONSTRAINT names UNIQUE(Domain,Name)");

    CreateTable(DGN_TABLE_Txns, "Id INTEGER PRIMARY KEY NOT NULL," 
                                "Deleted BOOLEAN,"
                                "Grouped BOOLEAN,"
                                "Operation TEXT,"
                                "Time TIMESTAMP DEFAULT(julianday('now')),"
                                "Change BLOB");

    Fonts().DbFontMap().CreateFontTable();

    ExecuteSql("CREATE VIRTUAL TABLE " DGN_VTABLE_SpatialIndex " USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"); // Define this before importing dgn schema!

    importBisCoreSchema(*this);

    // Every DgnDb has a few built-in CodeSpec for element codes
    CreateCodeSpecs();

    // Every DgnDb has a RepositoryModel and a DictionaryModel
    ExecuteSql("PRAGMA defer_foreign_keys = true;"); // the RepositoryModel and root Subject have foreign keys to each other
    CreateRepositoryModel();
    CreateRootSubject(params);
    ExecuteSql("PRAGMA defer_foreign_keys = false;");
    CreateDictionaryModel();
    CreateSessionModel();
    CreateRealityDataSourcesModel();

    // The Generic domain is used when a conversion process doesn't have enough information to pick something better
    if (DgnDbStatus::Success != GenericDomain::ImportSchema(*this))
        {
        BeAssert(false);
        return BE_SQLITE_NOTFOUND;
        }

    ExecuteSql("CREATE TRIGGER dgn_prjrange_del AFTER DELETE ON " BIS_TABLE(BIS_CLASS_GeometricElement3d)
               " BEGIN DELETE FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId=old.ElementId;END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_upd AFTER UPDATE " OF_SPATIAL_DATA " ON " BIS_TABLE(BIS_CLASS_GeometricElement3d) 
               " WHEN new.Origin_X IS NOT NULL AND " GEOM_IN_SPATIAL_INDEX_CLAUSE 
               "BEGIN INSERT OR REPLACE INTO " DGN_VTABLE_SpatialIndex "(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,"
               "DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5)"
               " FROM (SELECT " AABB_FROM_PLACEMENT " as bb);END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_upd1 AFTER UPDATE " OF_SPATIAL_DATA " ON " BIS_TABLE(BIS_CLASS_GeometricElement3d) 
                " WHEN OLD.Origin_X IS NOT NULL AND NEW.Origin_X IS NULL"
                " BEGIN DELETE FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId=OLD.ElementId;END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_ins AFTER INSERT ON " BIS_TABLE(BIS_CLASS_GeometricElement3d) 
               " WHEN new.Origin_X IS NOT NULL AND " GEOM_IN_SPATIAL_INDEX_CLAUSE
               "BEGIN INSERT INTO " DGN_VTABLE_SpatialIndex "(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,"
               "DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5)"
               " FROM (SELECT " AABB_FROM_PLACEMENT " as bb);END");

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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    12/2016
//---------------------------------------------------------------------------------------
DgnDbSchemaVersion DgnDbSchemaVersion::FromLegacy(Utf8CP legacyVersionString)
    {
    SchemaVersion legacyVersion(legacyVersionString);

    if (5 == legacyVersion.GetMajor())
        return DgnDbSchemaVersion::Version_1_5(); // Graphite05

    if ((6 == legacyVersion.GetMajor()) && (1 == legacyVersion.GetMinor()))
        return DgnDbSchemaVersion::Version_1_6(); // DgnDb0601

    return DgnDbSchemaVersion(); // Unknown/Invalid
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    12/2016
//---------------------------------------------------------------------------------------
DgnDbSchemaVersion DgnDbSchemaVersion::Extract(BeFileNameCR fileName)
    {
    BeSQLite::Db db;
    if (BE_SQLITE_OK != db.OpenBeSQLiteDb(fileName, Db::OpenParams (Db::OpenMode::Readonly)))
        return DgnDbSchemaVersion(); // not a BeSQLite database

    Utf8String packageSchemaVersion;
    if (BE_SQLITE_ROW == db.QueryProperty(packageSchemaVersion, PackageProperty::SchemaVersion()))
        {
        // is a package, query DgnDbSchemaVersion from embedded DgnDb (use current PropertySpec)
        Utf8String schemaVersion;
        if (BE_SQLITE_ROW == db.QueryProperty(schemaVersion, DgnEmbeddedProjectProperty::SchemaVersion(), 1 /* first embedded file */))
            return DgnDbSchemaVersion(schemaVersion.c_str());

        // is a package, query DgnDbSchemaVersion from embedded DgnDb (use legacy PropertySpec)
        Utf8String legacySchemaVersion;
        if (BE_SQLITE_ROW == db.QueryProperty(legacySchemaVersion, LegacyEmbeddedDbSchemaVersionProperty(), 1 /* first embedded file */))
            return DgnDbSchemaVersion::FromLegacy(legacySchemaVersion.c_str());

        return DgnDbSchemaVersion(); // valid package, but invalid or non-existent payload
        }

    // not a package, query DgnDbSchemaVersion directly (use current PropertySpec)
    Utf8String schemaVersion;
    if (BE_SQLITE_ROW == db.QueryProperty(schemaVersion, DgnProjectProperty::SchemaVersion()))
        return DgnDbSchemaVersion(schemaVersion.c_str());

    // not a package, query DgnDbSchemaVersion directly (use legacy PropertySpec)
    Utf8String legacySchemaVersion;
    if (BE_SQLITE_ROW == db.QueryProperty(legacySchemaVersion, LegacyDbSchemaVersionProperty()))
        return DgnDbSchemaVersion::FromLegacy(legacySchemaVersion.c_str());

    return DgnDbSchemaVersion(); // unknown BeSQLite database type
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

    m_geoLocation.Save();

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
    version = DgnDbSchemaVersion::GetCurrent();
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
        if (DgnDbSchemaVersion::GetCurrent() == version)
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
        {
        if (BE_SQLITE_ROW == QueryProperty(versionString, DgnDbSchemaVersion::LegacyDbSchemaVersionProperty()))
            return BE_SQLITE_ERROR_ProfileTooOld; // report Graphite05 and DgnDb0601 as too old rather than invalid

        return BE_SQLITE_ERROR_InvalidProfileVersion;
        }

    m_schemaVersion.FromJson(versionString.c_str());
    DgnVersion expectedVersion = DgnDbSchemaVersion::GetCurrent();
    DgnVersion minimumAutoUpgradableVersion(DGNDB_SUPPORTED_VERSION_Major, DGNDB_SUPPORTED_VERSION_Minor, 0, 0);

    bool profileIsAutoUpgradable = false;
    stat = CheckProfileVersion(profileIsAutoUpgradable, expectedVersion, m_schemaVersion, minimumAutoUpgradableVersion, params.IsReadonly(), "DgnDb");

    return profileIsAutoUpgradable ?((DgnDb::OpenParams&)params).UpgradeSchema(*this) : stat;
    }

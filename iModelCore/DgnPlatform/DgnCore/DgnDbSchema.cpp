/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnDbSchema.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"


ECN::ECSchemaPtr s_fakeSchema;
ECN::ECCustomAttributeClassP s_fakeCustomClass;
ECN::ECCustomAttributeClassP s_fakeStypeClass;
static std::map<Utf8String, bmap<Utf8String, bvector<IECInstancePtr>>> s_fakeCAs;

/*---------------------------------------------------------------------------------**//**
* Create a temporary, in-memory schema that defines a couple of CustomAttributes classes.
* It's much faster to do it this way than to re-import the entire dgn schema.
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
static ECN::ECSchemaP getFakeSchema()
    {
    if (s_fakeSchema.IsValid())
        return s_fakeSchema.get();

    if (ECN::ECObjectsStatus::Success != ECN::ECSchema::CreateSchema(s_fakeSchema, "", 0, 0))
        {
        BeAssert(false);
        return nullptr;
        }
    auto status = s_fakeSchema->CreateCustomAttributeClass(s_fakeCustomClass, "CustomHandledProperty");
    if (ECN::ECObjectsStatus::Success != status)
        {
        BeAssert(false);
        s_fakeSchema = nullptr;
        return nullptr;
        }

    status = s_fakeSchema->CreateCustomAttributeClass(s_fakeStypeClass, "PropertyStatementType");
    if (ECN::ECObjectsStatus::Success != status)
        {
        BeAssert(false);
        s_fakeSchema = nullptr;
        s_fakeStypeClass = nullptr;
        return nullptr;
        }

    ECN::PrimitiveECPropertyP stypeProp;
    status = s_fakeStypeClass->CreatePrimitiveProperty(stypeProp, "StatementTypes");
    if (ECN::ECObjectsStatus::Success != status)
        {
        BeAssert(false);
        s_fakeSchema = nullptr;
        s_fakeStypeClass = nullptr;
        s_fakeStypeClass = nullptr;
        return nullptr;
        }
    stypeProp->SetType(ECN::PrimitiveType::PRIMITIVETYPE_Integer);

    return s_fakeSchema.get();
    }

/*---------------------------------------------------------------------------------**//**
* See if the specified (fake) CustomAttribute was registered for the specified property. 
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
static ECN::IECInstancePtr getFakeCA(ECN::ECPropertyCR prop, ECN::ECClassCR desiredClass)
    {
    if (!s_fakeSchema.IsValid())
        return nullptr;

    auto iclass = s_fakeCAs.find(prop.GetClass().GetECSqlName());
    if (iclass == s_fakeCAs.end())
        return nullptr;

    for (auto const& props: iclass->second)
        {
        if (props.first.Equals(prop.GetName()))
            {
            for (auto ca: props.second)
                {
                if (&ca->GetClass() == &desiredClass)
                    return ca;
                }
            }
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomPropertyRegistry::HasOldDgnSchema(DgnDbR db)
    {
    return nullptr == db.Schemas().GetECClass(DGN_ECSCHEMA_NAME, "CustomHandledProperty");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomPropertyRegistry::Register(Utf8CP propName, ECSqlClassParams::StatementType stype)
    {
    if (nullptr == getFakeSchema())  // make sure fake schema is created
        return;
    BeAssert(nullptr != m_eclass->GetPropertyP(propName, false));
    auto& cls = s_fakeCAs[m_eclass->GetName()];
    auto& prop = cls[propName];
    prop.push_back(s_fakeCustomClass->GetDefaultStandaloneEnabler()->CreateInstance());
    if (ECSqlClassParams::StatementType::All != stype)
        {
        auto stypePropInstance = s_fakeStypeClass->GetDefaultStandaloneEnabler()->CreateInstance();
        stypePropInstance->SetValue("StatementTypes", ECN::ECValue((int32_t)stype));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomPropertyRegistry::SetClass(DgnDbR db, Utf8CP schemaName, Utf8CP className) 
    { 
    SetClass(db.Schemas().GetECClass(schemaName, className));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool AutoHandledPropertiesCollection::HasCustomHandledProperty(ECN::ECPropertyCR prop) const
    {
    if (nullptr != m_customHandledProperty)
        return prop.IsDefined(*m_customHandledProperty);

    return getFakeCA(prop, *s_fakeCustomClass).IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlClassParams::StatementType AutoHandledPropertiesCollection::GetStatementType(ECN::ECPropertyCR prop) const
    {
    ECN::IECInstancePtr stype;
    if (m_propertyStatementType != nullptr)
        stype = prop.GetCustomAttribute(*m_propertyStatementType);
    else
        stype = getFakeCA(prop, *s_fakeStypeClass);

    if (!stype.IsValid())
        return ECSqlClassParams::StatementType::All;    // default = all statement types

    ECN::ECValue v;
    stype->GetValue(v, "StatementTypes");
    return (ECSqlClassParams::StatementType)(v.GetInteger());
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
static void importDgnSchema(DgnDbR db)
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
static DbResult insertIntoDgnModel(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnCode const& modelCode)
    {
    Statement stmt(db, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Model) " (Id,Code_Value,Label,ECClassId,Visibility,Code_AuthorityId,Code_Namespace) VALUES(?,?,'',?,0,?,?)");
    stmt.BindId(1, modelId);
    stmt.BindText(2, modelCode.GetValueCP(), Statement::MakeCopy::No);
    stmt.BindId(3, classId);
    stmt.BindId(4, modelCode.GetAuthority());
    stmt.BindText(5, modelCode.GetNamespace().c_str(), Statement::MakeCopy::No);

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
    DgnCode modelCode = DgnModel::CreateModelCode("Dictionary", DGN_ECSCHEMA_NAME);
    return insertIntoDgnModel(*this, modelId, classId, modelCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateGroupInformationModel()
    {
    DgnModelId modelId = DgnModel::GroupInformationId();
    DgnClassId classId = Domains().GetClassId(dgn_ModelHandler::GroupInformation::GetHandler());
    DgnCode modelCode = DgnModel::CreateModelCode("GroupInformation", DGN_ECSCHEMA_NAME);
    return insertIntoDgnModel(*this, modelId, classId, modelCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateDgnDbTables()
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

    importDgnSchema(*this);

    // Every DgnDb has a few built-in authorities for element codes
    CreateAuthorities();

    // Every DgnDb has a DictionaryModel and a default GroupInformationModel
    CreateDictionaryModel();
    CreateGroupInformationModel();

    // The Generic domain is used when a conversion process doesn't have enough information to pick something better
    if (DgnDbStatus::Success != GenericDomain::ImportSchema(*this, DgnDomain::ImportSchemaOptions::ImportOnly)) // Let an upper layer decide whether or not to create ECClassViews
        {
        BeAssert(false);
        return BE_SQLITE_NOTFOUND;
        }

    ExecuteSql("CREATE TRIGGER dgn_prjrange_del AFTER DELETE ON " DGN_TABLE(DGN_CLASSNAME_GeometricElement3d)
               " BEGIN DELETE FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId=old.ElementId;END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_upd AFTER UPDATE ON " DGN_TABLE(DGN_CLASSNAME_GeometricElement3d) 
               " WHEN new.Origin_X IS NOT NULL AND " GEOM_IN_SPATIAL_INDEX_CLAUSE
               "BEGIN INSERT OR REPLACE INTO " DGN_VTABLE_SpatialIndex "(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,"
               "DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5)"
               " FROM (SELECT " AABB_FROM_PLACEMENT " as bb);END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_upd1 AFTER UPDATE ON " DGN_TABLE(DGN_CLASSNAME_GeometricElement3d) 
                " WHEN OLD.Origin_X IS NOT NULL AND NEW.Origin_X IS NULL"
                " BEGIN DELETE FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId=OLD.ElementId;END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_ins AFTER INSERT ON " DGN_TABLE(DGN_CLASSNAME_GeometricElement3d) 
               " WHEN new.Origin_X IS NOT NULL AND " GEOM_IN_SPATIAL_INDEX_CLAUSE
               "BEGIN INSERT INTO " DGN_VTABLE_SpatialIndex "(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,"
               "DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5)"
               " FROM (SELECT " AABB_FROM_PLACEMENT " as bb);END");

#ifdef NEEDSWORK_VIEW_SETTINGS_TRIGGER
    ExecuteSql("CREATE TRIGGER delete_viewProps AFTER DELETE ON " DGN_TABLE(DGN_CLASSNAME_View) " BEGIN DELETE FROM " BEDB_TABLE_Property
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
    SavePropertyString(DgnProjectProperty::Name(), params.m_name);
    SavePropertyString(DgnProjectProperty::Description(), params.m_description);
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

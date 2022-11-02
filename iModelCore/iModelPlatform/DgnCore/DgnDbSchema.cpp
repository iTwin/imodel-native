/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

bmap<ECN::ECClassCP, bvector<ECN::ECPropertyCP>> AutoHandledPropertiesCollection::s_orphanCustomHandledProperties;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AutoHandledPropertiesCollection::DetectOrphanCustomHandledProperty(DgnDbR db, ECN::ECClassCR eclass)
    {
    auto caClass = db.Schemas().GetClass(BIS_ECSCHEMA_NAME, "CustomHandledProperty");
    for (auto ecp : eclass.GetProperties(false))
        {
        if (ecp->IsDefined(*caClass))
            s_orphanCustomHandledProperties[&ecp->GetClass()].push_back(ecp);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AutoHandledPropertiesCollection::IsOrphanCustomHandledProperty(ECN::ECPropertyCR ecp)
    {
    auto const& co = s_orphanCustomHandledProperties.find(&ecp.GetClass());
    if (co == s_orphanCustomHandledProperties.end())
        return false;
    auto const& orphans = co->second;
    return std::find(orphans.begin(), orphans.end(), &ecp) != orphans.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::IsCustomHandledProperty(ECN::ECPropertyCR prop) const
    {
    auto customHandledProperty = GetDgnDb().Schemas().GetClass(BIS_ECSCHEMA_NAME, "CustomHandledProperty");
    if (nullptr == customHandledProperty)
        return false;

    return prop.IsDefined(*customHandledProperty);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AutoHandledPropertiesCollection::Iterator::Iterator(ECN::ECPropertyIterable::const_iterator it, AutoHandledPropertiesCollection const& coll)
    : m_i(it), m_coll(coll)
    {
    ToNextValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AutoHandledPropertiesCollection::Iterator& AutoHandledPropertiesCollection::Iterator::operator++()
    {
    BeAssert(m_i != m_coll.m_end);
    ++m_i;
    ToNextValid();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AutoHandledPropertiesCollection::AutoHandledPropertiesCollection(ECN::ECClassCR eclass, DgnDbR db, ECSqlClassParams::StatementType stype, bool wantCustomHandledProps)
    : m_props(eclass.GetProperties(true)), m_end(m_props.end()), m_stype(stype), m_wantCustomHandledProps(wantCustomHandledProps)
    {
    #ifdef DEBUG_AUTO_HANDLED_PROPERTIES
        printf("%s\n", eclass.GetFullName());
        printf("---------------------------\n");
    #endif
    m_customHandledProperty = db.Schemas().GetClass(BIS_ECSCHEMA_NAME, "CustomHandledProperty");
    m_autoHandledProperty = db.Schemas().GetClass(BIS_ECSCHEMA_NAME, "AutoHandledProperty");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

        bool isCustom = ca.IsValid() && !IsOrphanCustomHandledProperty(**m_i);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AutoHandledPropertiesCollection::ForEach(ECN::ECClassCR ecClass, DgnDbR db, ECSqlClassParams::StatementType stype, bool wantCustomHandledProps, std::function<void(ECN::ECPropertyCP)> cb)
    {
    for (auto prop : AutoHandledPropertiesCollection(ecClass, db, stype, wantCustomHandledProps))
        cb(prop);
    }


#define GEOM_IN_SPATIAL_INDEX_CLAUSE " 1 = new.InSpatialIndex "
#define ORIGIN_FROM_PLACEMENT "DGN_point(NEW.Origin_X,NEW.Origin_Y,NEW.Origin_Z)"
#define ANGLES_FROM_PLACEMENT "DGN_angles(NEW.Yaw,NEW.Pitch,NEW.Roll)"
#define BBOX_FROM_PLACEMENT "DGN_bbox(NEW.BBoxLow_X,NEW.BBoxLow_Y,NEW.BBoxLow_Z,NEW.BBoxHigh_X,NEW.BBoxHigh_Y,NEW.BBoxHigh_Z)"
#define PLACEMENT_FROM_GEOM "DGN_placement(" ORIGIN_FROM_PLACEMENT "," ANGLES_FROM_PLACEMENT "," BBOX_FROM_PLACEMENT ")"
#define AABB_FROM_PLACEMENT "DGN_placement_aabb(" PLACEMENT_FROM_GEOM ")"
#define OF_SPATIAL_DATA "OF Origin_X,Origin_Y,Origin_Z,Yaw,Pitch,Roll,BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult insertIntoDgnModel(DgnDbR db, DgnClassId classId, DgnElementId modeledElementId, DgnModelId parentModelId, bool isPrivate)
    {
    Statement stmt(db, "INSERT INTO " BIS_TABLE(BIS_CLASS_Model) " (Id,ECClassId,ParentModelId,ModeledElementId,ModeledElementRelECClassId,IsPrivate,IsTemplate) VALUES(?,?,?,?,?,?,0)");
    stmt.BindId(1, DgnModelId(modeledElementId.GetValue())); // DgnModelId is the same as the element that it is modeling
    stmt.BindId(2, classId);
    stmt.BindId(3, parentModelId);
    stmt.BindId(4, modeledElementId);
    stmt.BindId(5, db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ModelModelsElement));
    stmt.BindBoolean(6, isPrivate);

    DbResult result = stmt.Step();
    BeAssert(BE_SQLITE_DONE == result && "Failed to create model");
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreatePartitionElement(Utf8CP className, DgnElementId partitionId, Utf8CP partitionName)
    {
    DgnCode partitionCode(CodeSpecs().QueryCodeSpecId(BIS_CODESPEC_InformationPartitionElement), Elements().GetRootSubjectId(), partitionName);

    // element handlers are not initialized yet, so insert DefinitionPartition directly
    Utf8PrintfString sql("INSERT INTO %s (ECInstanceId,Model.Id,Parent.Id,Parent.RelECClassId,CodeSpec.Id,CodeScope.Id,CodeValue) VALUES(?,?,?,?,?,?,?)", className);
    ECSqlStatement statement;
    if (ECSqlStatus::Success != statement.Prepare(*this, sql.c_str(), GetECCrudWriteToken()))
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    statement.BindId(1, partitionId);
    statement.BindId(2, DgnModel::RepositoryModelId());
    statement.BindId(3, Elements().GetRootSubjectId());
    statement.BindId(4, Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_SubjectOwnsPartitionElements));
    statement.BindId(5, partitionCode.GetCodeSpecId());
    statement.BindId(6, partitionCode.GetScopeElementId(*this));
    statement.BindText(7, partitionCode.GetValue().GetUtf8CP(), IECSqlBinder::MakeCopy::No);

    DbResult result = statement.Step();
    BeAssert(BE_SQLITE_DONE == result);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    return insertIntoDgnModel(*this, classId, modeledElementId, DgnModel::RepositoryModelId(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateRealityDataSourcesModel()
    {
    DgnElementId modeledElementId = Elements().GetRealityDataSourcesPartitionId();
    DbResult result = CreatePartitionElement(BIS_SCHEMA(BIS_CLASS_LinkPartition), modeledElementId, BIS_SCHEMA("RealityDataSources"));
    if (BE_SQLITE_DONE != result)
        return result;

    DgnClassId classId = Domains().GetClassId(dgn_ModelHandler::Link::GetHandler());
    BeAssert(classId.IsValid());
    return insertIntoDgnModel(*this, classId, modeledElementId, DgnModel::RepositoryModelId(), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateRepositoryModel()
    {
    DgnClassId classId = Domains().GetClassId(dgn_ModelHandler::Repository::GetHandler());
    BeAssert(DgnModel::RepositoryModelId().GetValue() == Elements().GetRootSubjectId().GetValue());
    return insertIntoDgnModel(*this, classId, Elements().GetRootSubjectId(), DgnModelId(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateRootSubject(CreateDgnDbParams const& params)
    {
    DgnElementId elementId = Elements().GetRootSubjectId();
    DgnModelId modelId = DgnModel::RepositoryModelId();
    CodeSpecId codeSpecId = CodeSpecs().QueryCodeSpecId(BIS_CODESPEC_Subject);
    DgnCode elementCode = DgnCode(codeSpecId, elementId, params.m_rootSubjectName);

    // element handlers are not initialized yet, so insert root Subject directly
    ECSqlStatement statement;
    if (ECSqlStatus::Success != statement.Prepare(*this, "INSERT INTO " BIS_SCHEMA(BIS_CLASS_Subject) " (ECInstanceId,Model.Id,CodeSpec.Id,CodeScope.Id,CodeValue,Description) VALUES(?,?,?,?,?,?)", GetECCrudWriteToken()))
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    statement.BindId(1, elementId);
    statement.BindId(2, modelId);
    statement.BindId(3, elementCode.GetCodeSpecId());
    statement.BindId(4, elementCode.GetScopeElementId(*this));
    statement.BindText(5, elementCode.GetValueUtf8CP(), IECSqlBinder::MakeCopy::No);
    statement.BindText(6, params.m_rootSubjectDescription.c_str(), IECSqlBinder::MakeCopy::No);

    DbResult result = statement.Step();
    BeAssert(BE_SQLITE_DONE == result);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateRebaseTable()
    {
    return CreateTable(DGN_TABLE_Rebase, "Id INTEGER PRIMARY KEY AUTOINCREMENT, Rebase BLOB");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateDgnDbTables(CreateDgnDbParams const& params) {
    CreateTable(DGN_TABLE_Domain, "Name TEXT NOT NULL UNIQUE COLLATE NoCase PRIMARY KEY,"
                                    "Description TEXT,"
                                    "Version INTEGER");

    CreateTable(DGN_TABLE_Handler, "ClassId INTEGER PRIMARY KEY,"
                                    "Domain TEXT NOT NULL COLLATE NoCase REFERENCES " DGN_TABLE_Domain "(Name),"
                                    "Name TEXT NOT NULL COLLATE NoCase,"
                                    "Permissions INTEGER,"
                                    "CONSTRAINT names UNIQUE(Domain,Name)");

    CreateTable(DGN_TABLE_Txns, "Id INTEGER PRIMARY KEY NOT NULL,"
                                "Deleted BOOLEAN,"
                                "Grouped BOOLEAN,"
                                "Operation TEXT,"
                                "IsSchemaChange BOOLEAN,"
                                "Time TIMESTAMP DEFAULT(julianday('now')),"
                                "Change BLOB");

    CreateTable(DGN_TABLE_Font, "Id INTEGER PRIMARY KEY,"
                                "Type INT,"
                                "Name CHAR NOT NULL COLLATE NOCASE,"
                                "Metadata BLOB,"
                                "CONSTRAINT names UNIQUE(Type,Name)");

    CreateRebaseTable();

    ExecuteSql("CREATE VIRTUAL TABLE " DGN_VTABLE_SpatialIndex " USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"); // Define this before importing dgn schema!

    BisCoreDomain::GetDomain().SetCreateParams(params);
    // BisCoreDomain is the only domain that requires the create params. They are passed through BisCoreDomain::_OnSchemaImported() -> DgnDb::OnBisCoreSchemaImported()

    SchemaStatus status = Domains().ImportSchemas();
    if (SchemaStatus::Success != status) {
        BeAssert(false);
        return SchemaStatusToDbResult(status, false /*=isUpgrade*/);
    }

    ExecuteSql("CREATE TRIGGER dgn_prjrange_del AFTER DELETE ON " BIS_TABLE(BIS_CLASS_GeometricElement3d) " BEGIN DELETE FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId=old.ElementId;END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_upd AFTER UPDATE " OF_SPATIAL_DATA " ON " BIS_TABLE(BIS_CLASS_GeometricElement3d) " WHEN new.Origin_X IS NOT NULL AND " GEOM_IN_SPATIAL_INDEX_CLAUSE
                                                                                                                            "BEGIN INSERT OR REPLACE INTO " DGN_VTABLE_SpatialIndex "(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,"
                                                                                                                            "DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5)"
                                                                                                                            " FROM (SELECT " AABB_FROM_PLACEMENT " as bb);END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_upd1 AFTER UPDATE " OF_SPATIAL_DATA " ON " BIS_TABLE(BIS_CLASS_GeometricElement3d) " WHEN OLD.Origin_X IS NOT NULL AND NEW.Origin_X IS NULL"
                                                                                                                            " BEGIN DELETE FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId=OLD.ElementId;END");

    ExecuteSql("CREATE TRIGGER dgn_rtree_ins AFTER INSERT ON " BIS_TABLE(BIS_CLASS_GeometricElement3d) " WHEN new.Origin_X IS NOT NULL AND " GEOM_IN_SPATIAL_INDEX_CLAUSE
                                                                                                        "BEGIN INSERT INTO " DGN_VTABLE_SpatialIndex "(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,"
                                                                                                        "DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5)"
                                                                                                        " FROM (SELECT " AABB_FROM_PLACEMENT " as bb);END");

    DbResult result = DgnSearchableText::CreateTable(*this);
    BeAssert(BE_SQLITE_OK == result && "Failed to create FTS5 tables");

    return result;
}

//---------------------------------------------------------------------------------------
// Called from BisCoreDomain::_OnSchemaImported to setup a newly created DgnDb
// @bsimethod
//---------------------------------------------------------------------------------------
void DgnDb::OnBisCoreSchemaImported(CreateDgnDbParams const& params)
    {
    // Every DgnDb has a few built-in CodeSpec for element codes
    CreateCodeSpecs();

    // Every DgnDb has a RepositoryModel and a DictionaryModel
    ExecuteSql("PRAGMA defer_foreign_keys = true;"); // the RepositoryModel and root Subject have foreign keys to each other
    CreateRepositoryModel();
    CreateRootSubject(params);
    ExecuteSql("PRAGMA defer_foreign_keys = false;");
    CreateDictionaryModel();
    CreateRealityDataSourcesModel();

    DbResult result = InitializeElementIdSequence();
    UNUSED_VARIABLE(result);
    BeAssert(result == BE_SQLITE_OK);
    result = ResetElementIdSequence(GetBriefcaseId());
    BeAssert(result == BE_SQLITE_OK);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::SaveDgnDbProfileVersion(DgnDbProfileVersion version)
    {
    m_profileVersion = version;
    return  SavePropertyString(DgnProjectProperty::ProfileVersion(), m_profileVersion.ToJson());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbProfileVersion DgnDbProfileVersion::FromLegacy(Utf8CP legacyVersionString)
    {
    DgnDbProfileVersion legacyVersion(legacyVersionString);

    if (5 == legacyVersion.GetMajor())
        return DgnDbProfileVersion::Version_1_5(); // Graphite05

    if ((6 == legacyVersion.GetMajor()) && (1 == legacyVersion.GetMinor()))
        return DgnDbProfileVersion::Version_1_6(); // DgnDb0601

    return DgnDbProfileVersion(); // Unknown/Invalid
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbProfileVersion DgnDbProfileVersion::Extract(BeFileNameCR fileName)
    {
    BeSQLite::Db db;
    if (BE_SQLITE_OK != db.OpenBeSQLiteDb(fileName, Db::OpenParams (Db::OpenMode::Readonly)))
        return DgnDbProfileVersion(); // not a BeSQLite database

    Utf8String packageVersion;
    if (BE_SQLITE_ROW == db.QueryProperty(packageVersion, PackageProperty::ProfileVersion()))
        {
        // is a package, query DgnDbProfileVersion from embedded DgnDb (use current PropertySpec)
        Utf8String profileVersion;
        if (BE_SQLITE_ROW == db.QueryProperty(profileVersion, DgnEmbeddedProjectProperty::ProfileVersion(), 1 /* first embedded file */))
            return DgnDbProfileVersion(profileVersion.c_str());

        // is a package, query DgnDbProfileVersion from embedded DgnDb (use legacy PropertySpec)
        Utf8String legacyProfileVersion;
        if (BE_SQLITE_ROW == db.QueryProperty(legacyProfileVersion, LegacyEmbeddedDbProfileVersionProperty(), 1 /* first embedded file */))
            return DgnDbProfileVersion::FromLegacy(legacyProfileVersion.c_str());

        return DgnDbProfileVersion(); // valid package, but invalid or non-existent payload
        }

    // not a package, query DgnDbProfileVersion directly (use current PropertySpec)
    Utf8String profileVersion;
    if (BE_SQLITE_ROW == db.QueryProperty(profileVersion, DgnProjectProperty::ProfileVersion()))
        return DgnDbProfileVersion(profileVersion.c_str());

    // not a package, query DgnDbProfileVersion directly (use legacy PropertySpec)
    Utf8String legacyProfileVersion;
    if (BE_SQLITE_ROW == db.QueryProperty(legacyProfileVersion, LegacyDbProfileVersionProperty()))
        return DgnDbProfileVersion::FromLegacy(legacyProfileVersion.c_str());

    return DgnDbProfileVersion(); // unknown BeSQLite database type
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::InitializeDgnDb(CreateDgnDbParams const& params) {
    if (params.GetGuid().IsValid())
        ChangeDbGuid(params.GetGuid());

    SaveDgnDbProfileVersion();
    SaveCreationDate();

    SavePropertyString(DgnProjectProperty::LastEditor(), "iTwin.js");
    SavePropertyString(DgnProjectProperty::Client(), params.m_client);

    AxisAlignedBox3d extents = params.m_projectExtents;
    if (extents.IsNull())
        extents = m_geoLocation.GetDefaultProjectExtents();

    m_geoLocation.SetProjectExtents(extents);
    m_geoLocation.SetGlobalOrigin(params.m_globalOrigin);
    m_geoLocation.Save();

    Domains().OnDbOpened();
    return SaveChanges();
}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ProjectSchemaUpgrader
    {
    virtual DgnDbProfileVersion _GetVersion() = 0;
    virtual DbResult _Upgrade(DgnDbR project, DgnDbProfileVersion version) = 0;
    };

#if defined (WIP_RebaseSupportUpgrader)
struct RebaseSupportUpgrader : ProjectSchemaUpgrader
    {
    DgnDbProfileVersion _GetVersion() override {return DgnDbProfileVersion(2, 1);}
    DbResult _Upgrade(DgnDbR db, DgnDbProfileVersion version)
        {
        // DGN_TABLE_Rebase was introduced in 2.1
        if ((version.GetMajor() != 2) || (version.GetMinor() >= 1))
            return BE_SQLITE_OK;

        if (db.TableExists(DGN_TABLE_Rebase))
            return BE_SQLITE_OK;

        return db.CreateRebaseTable();
        }
    };

static RebaseSupportUpgrader s_rebaseSupportUpgrader;

static ProjectSchemaUpgrader* s_upgraders[] =
    {
    // NOTE: entries in this list *must* be sorted in ascending version order.
    // Add a new version here
    &s_rebaseSupportUpgrader
    };
#endif

#if defined (WHEN_FIRST_UPGRADER)
static ProjectSchemaUpgrader* s_upgraders[] =
    {
    // NOTE: entries in this list *must* be sorted in ascending version order.
    // Add a new version here

    };
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbProfileVersion DgnDb::GetProfileVersion() { return m_profileVersion; }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ProfileState DgnDb::_CheckProfileVersion() const
    {
    ProfileState ecdbProfileState = T_Super::_CheckProfileVersion();

    Utf8String versionString;
    DbResult stat = QueryProperty(versionString, DgnProjectProperty::ProfileVersion());
    if (BE_SQLITE_ROW != stat)
        {
        if (BE_SQLITE_ROW == QueryProperty(versionString, DgnDbProfileVersion::LegacyDbProfileVersionProperty()))
            return ProfileState::Older(ProfileState::CanOpen::No, false); // report Graphite05 and DgnDb0601 as too old rather than invalid

        return ProfileState::Error();
        }

    m_profileVersion.FromJson(versionString.c_str());
    ProfileState dgndbProfileState = CheckProfileVersion(DgnDbProfileVersion::GetCurrent(), m_profileVersion, DgnDbProfileVersion(DGNDB_SUPPORTED_VERSION_Major, DGNDB_SUPPORTED_VERSION_Minor, 0, 0), "DgnDb");
    return ecdbProfileState.Merge(dgndbProfileState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnDb::_OnBeforeProfileUpgrade()
    {
    if (AreTxnsEnabled())
        Txns().EnableTracking(true);

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnDb::_OnAfterProfileUpgrade()
    {
    if (!AreTxnsEnabled())
        return BE_SQLITE_OK;

    if (Txns().HasChanges()) {
        BeAssert(!Txns().m_inProfileUpgrade);
        AutoRestore<bool> _v(&Txns().m_inProfileUpgrade, true);
        return SaveChanges("Automatic Profile Upgrade");
    }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::_UpgradeProfile()
    {
    DbResult result = T_Super::_UpgradeProfile();
    if (BE_SQLITE_OK != result)
        return result;

#if defined (WHEN_FIRST_UPGRADER)
    for (auto upgrader : s_upgraders)
        {
        if (m_profileVersion < upgrader->_GetVersion())
            {
            DbResult stat = upgrader->_Upgrade(project, m_profileVersion);
            if (BE_SQLITE_OK != stat)
                return stat;

            m_profileVersion = upgrader->_GetVersion();
            }
        }
#else
    m_profileVersion = DgnDbProfileVersion::GetCurrent();
#endif

    return SaveDgnDbProfileVersion(m_profileVersion);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::DropSchemaResult DgnDb::DropSchema(Utf8StringCR name, bool logIssue) {
    return Domains().DoDropSchema(name, logIssue);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaStatus DgnDb::ImportSchemas(bvector<ECSchemaCP> const& schemas)
    {
    bvector<ECN::ECSchemaCP> schemasToImport;
    SchemaStatus status = PickSchemasToImport(schemasToImport, schemas, false /*=isImportingFromV8*/);
    if (status != SchemaStatus::Success)
        {
        BeAssert(false && "One or more schemas are incompatible.");
        return status;
        }

    return Domains().DoImportSchemas(schemasToImport, SchemaManager::SchemaImportOptions::None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaStatus DgnDb::ImportV8LegacySchemas(bvector<ECSchemaCP> const& schemas, size_t* numImported)
    {
    bvector<ECN::ECSchemaCP> schemasToImport;
    SchemaStatus status = PickSchemasToImport(schemasToImport, schemas, true /*=isImportingFromV8*/);
    if (status != SchemaStatus::Success)
        {
        BeAssert(false && "One or more schemas are incompatible.");
        return status;
        }

    if (nullptr != numImported)
        *numImported = schemasToImport.size();
    if (schemasToImport.empty())
        return SchemaStatus::Success;

    SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues | SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications;
    return Domains().DoImportSchemas(schemasToImport, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaStatus DgnDb::PickSchemasToImport(bvector<ECSchemaCP>& importSchemas, bvector<ECSchemaCP> const& schemas, bool isImportingFromV8) const
    {
    importSchemas.clear();

    for (ECSchemaCP appSchema : schemas)
        {
        if (appSchema->GetName().EqualsIAscii("Units_Schema"))
            continue;

        SchemaStatus status = DgnDomains::DoValidateSchema(*appSchema, false /*=isReadonly*/, *this);

        if (status == SchemaStatus::Success)
            continue;

        if (status == SchemaStatus::SchemaTooNew || status == SchemaStatus::SchemaTooOld)
            return status;

        BeAssert(status == SchemaStatus::SchemaNotFound || status == SchemaStatus::SchemaUpgradeRequired || status == SchemaStatus::SchemaUpgradeRecommended || status == SchemaStatus::SchemaIsDynamic);
        importSchemas.push_back(appSchema);
        }

    return SchemaStatus::Success;
    }


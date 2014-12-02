/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnProjectSchema.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnVersion getCurrentSchemaVerion()
    {
    return DgnVersion(PROJECT_CURRENT_VERSION_Major, PROJECT_CURRENT_VERSION_Minor, PROJECT_CURRENT_VERSION_Sub1, PROJECT_CURRENT_VERSION_Sub2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnVersion getCurrentDgnFileVersion()
    {
    return  DgnVersion(DGNFILE_CURRENT_VERSION_Major, DGNFILE_CURRENT_VERSION_Minor, DGNFILE_CURRENT_VERSION_Sub1, DGNFILE_CURRENT_VERSION_Sub2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProject::Create3dRTree ()
    {
    ExecuteSql("CREATE VIRTUAL TABLE " DGN_VTABLE_PrjRTree " USING rtree(ElementId,Min_X,Max_X,Min_Y,Max_Y,Min_Z,Max_Z)");
    ExecuteSql("CREATE TRIGGER dgn_prjrange_add AFTER INSERT ON " DGN_TABLE_Element 
               " WHEN (SELECT 1 from " DGN_TABLE_Model " WHERE new.ModelId=Id AND Type=0 AND Space=1 AND new.[Min.X] IS NOT NULL) IS NOT NULL"
               " BEGIN INSERT OR IGNORE INTO " DGN_VTABLE_PrjRTree " VALUES(NEW.Id,new.[Min.X],new.[Max.X],new.[Min.Y],new.[Max.Y],new.[Min.Z],new.[Max.Z]); END");
    ExecuteSql("CREATE TRIGGER dgn_prjrange_upd AFTER UPDATE ON " DGN_TABLE_Element 
               " WHEN (SELECT 1 from " DGN_TABLE_Model " WHERE new.ModelId=Id AND Type=0 AND Space=1 AND new.[Min.X] IS NOT NULL) IS NOT NULL"
               " AND (old.[Min.X]!=new.[Min.X] OR old.[Max.X]!=new.[Max.X] OR old.[Min.Y]!=new.[Min.Y] OR old.[Max.Y]!=new.[Max.Y] OR old.[Min.Z]!=new.[Min.Z] OR old.[Max.Z]!=new.[Max.Z])" 
               " BEGIN UPDATE OR IGNORE " DGN_VTABLE_PrjRTree " SET "
               "Min_X=new.[Min.X],Max_X=new.[Max.X],Min_Y=new.[Min.Y],Max_Y=new.[Max.Y],Min_Z=new.[Min.Z],Max_Z=new.[Max.Z] WHERE ElementId=new.Id; END");

    ExecuteSql("CREATE TRIGGER dgn_prjrange_del AFTER DELETE ON " DGN_TABLE_Element 
               " WHEN old.Symb IS NOT NULL AND old.[Min.Z] IS NOT NULL"
               " BEGIN DELETE FROM " DGN_VTABLE_PrjRTree " WHERE ElementId=old.Id; END");

    ExecuteSql("INSERT INTO " DGN_VTABLE_PrjRTree " SELECT a.Id,a.[Min.X],a.[Max.X],a.[Min.Y],a.[Max.Y],a.[Min.Z],a.[Max.Z] FROM " DGN_TABLE_Element " as a,"
               DGN_TABLE_Model " as b WHERE a.ModelId=b.Id AND b.Type=0 AND b.Space=1 AND a.[Max.X] IS NOT NULL");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void createSubLevelTable(DgnProjectR project)
    {
    project.CreateTable(DGN_TABLE_SubLevel, 
                                            "LevelId INTEGER REFERENCES " DGN_TABLE_Level "(Id) ON DELETE CASCADE,"
                                            "Id INT,"
                                            "Name CHAR COLLATE NOCASE,"
                                            "Descr CHAR,"
                                            "Props CHAR,"
                                            "PRIMARY KEY(LevelId,Id),"
                                            "CONSTRAINT subLevelName UNIQUE(LevelId,Name)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void createModelTable (DgnProjectR project)
    {
#if defined (NEEDS_WORK_DGNITEM)
    project.CreateTable(DGN_TABLE_Model, 
                                            "Id INTEGER PRIMARY KEY,"
                                            "Name CHAR NOT NULL COLLATE NOCASE UNIQUE,"
                                            "Descr CHAR,"
                                            "Type INT,"
                                            "SubType CHAR,"
                                            "Space INT,"
                                            "Visibility INT");
#endif

    project.ExecuteSql("CREATE TRIGGER delete_modelfromsel AFTER DELETE ON " DGN_TABLE_Model " BEGIN DELETE FROM " DGN_TABLE_SetEntry
                    " WHERE SetType=\"" DGN_SETTYPE_ModelSelector "\" AND EntryId=OLD.Id; END");
    project.ExecuteSql("CREATE TRIGGER delete_modelProps AFTER DELETE ON " DGN_TABLE_Model " BEGIN DELETE FROM " BEDB_TABLE_Property
                    " WHERE Namespace=\"" PROPERTY_APPNAME_DgnModel "\" AND Id=OLD.Id; END");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void createStampTable(DgnProjectR project)
    {
    project.CreateTable(DGN_TABLE_Stamp, 
                                            "Namespace CHAR NOT NULL,"
                                            "Name CHAR NOT NULL,"
                                            "Id1 INT NOT NULL,"
                                            "Id2 INT,"
                                            "Props CHAR,"
                                            "Data BLOB,"
                                            "PRIMARY KEY(Namespace,Name,Id1,Id2)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static void importDgnSchema (DgnProjectR project, bool updateExisting)
    {
    ECN::ECSchemaReadContextPtr ecSchemaContext = ECN::ECSchemaReadContext::CreateContext();
    ecSchemaContext->AddSchemaLocater (project.GetEC().GetSchemaLocater());

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
    BeAssert (dgnschema != NULL && "Could not find the dgn schema");

    BentleyStatus status = project.GetEC().GetSchemaManager().ImportECSchemas (ecSchemaContext->GetCache(), 
        ECDbSchemaManager::ImportOptions (false/*supplementation*/, updateExisting));

    BeAssert (status == SUCCESS && "Could not import dgn schema");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnProject::CreateProjectTables()
    {
    CreateTable(DGN_TABLE_Domain,    "Name CHAR UNIQUE NOT NULL PRIMARY KEY,"
                                            "Descr CHAR,"
                                            "Version INTEGER");

    CreateTable(DGN_TABLE_Handler,   "Type CHAR NOT NULL,"
                                            "Id BIGINT NOT NULL,"
                                            "Domain CHAR NOT NULL,"
                                            "Name CHAR NOT NULL,"
                                            "Descr CHAR,"
                                            "PRIMARY KEY (Type,Id)");

    CreateTable(DGN_TABLE_Font,      "Id INTEGER PRIMARY KEY,"
                                            "Type INT,"
                                            "Name CHAR NOT NULL COLLATE NOCASE,"
                                            "CONSTRAINT names UNIQUE(Type,Name)");

    CreateTable(DGN_TABLE_Color,     "Id INTEGER PRIMARY KEY,"
                                            "RGB INT NOT NULL,"
                                            "Name CHAR COLLATE NOCASE,"
                                            "Book CHAR COLLATE NOCASE,"
                                            "CONSTRAINT names UNIQUE(Name,Book)");

    CreateTable(DGN_TABLE_Style,     "Type INT NOT NULL,"
                                            "Id INTEGER NOT NULL,"
                                            "Name CHAR NOT NULL COLLATE NOCASE,"
                                            "Descr CHAR,"
                                            "Data BLOB,"
                                            "PRIMARY KEY(Type,Id),"
                                            "CONSTRAINT names UNIQUE(Type,Name)");

    CreateTable(DGN_TABLE_Material,
                                            "Id INTEGER PRIMARY KEY,"
                                            "Name CHAR NOT NULL COLLATE NOCASE,"
                                            "Palette CHAR");

    importDgnSchema(*this, false);

#if defined (NEEDS_WORK_DGNITEM)
    CreateTable(DGN_TABLE_Level,     "Id INTEGER PRIMARY KEY,"
                                            "Name CHAR NOT NULL COLLATE NOCASE UNIQUE,"
                                            "Descr CHAR,"
                                            "Scope INT,"
                                            "Rank INT,"
                                            "Props CHAR");
#endif

    CreateTable(DGN_TABLE_GeneratedModel, 
                                            "Id INTEGER PRIMARY KEY,"
                                            "ViewId INTEGER,"
                                            "ModelId INTEGER");

    createSubLevelTable(*this);
    createModelTable(*this);
    createStampTable(*this);

#if defined (NEEDS_WORK_DGNITEM)
    CreateTable(DGN_TABLE_View,      "Id INTEGER PRIMARY KEY,"
                                            "ViewType INT,"
                                            "SubType CHAR,"
                                            "Name CHAR NOT NULL COLLATE NOCASE UNIQUE,"
                                            "Descr CHAR,"
                                            "SelectorId INTEGER,"
                                            "Source INT,"
                                            "BaseModel INTEGER REFERENCES " DGN_TABLE_Model " ON DELETE CASCADE");
#endif

    CreateTable(DGN_TABLE_Session,   "Id INTEGER PRIMARY KEY,"
                                            "Name CHAR NOT NULL COLLATE NOCASE UNIQUE,"
                                            "Descr CHAR,"
                                            "Data BLOB");

    CreateTable(DGN_TABLE_ModelSelector, 
                                            "Id INTEGER PRIMARY KEY,"
                                            "Name CHAR NOT NULL COLLATE NOCASE UNIQUE,"
                                            "Descr CHAR,"
                                            "Flags INT,"
                                            "Data BLOB");

    CreateTable(DGN_TABLE_SetEntry,  "SetType INTEGER REFERENCES " DGN_TABLE_KeyStr ","
                                            "SetId BIGINT,"
                                            "EntryId BIGINT,"
                                            "Data BLOB,"
                                            "PRIMARY KEY (SetType,SetId,EntryId)");

    CreateTable(DGN_TABLE_KeyStr, "Id INTEGER PRIMARY KEY,"
                                            "Name CHAR NOT NULL UNIQUE,"
                                            "Def CHAR");

    ExecuteSql("CREATE TRIGGER delete_selector AFTER DELETE ON " DGN_TABLE_ModelSelector " BEGIN DELETE FROM " DGN_TABLE_SetEntry
                    " WHERE SetType=\"" DGN_SETTYPE_ModelSelector "\" AND SetId=OLD.Id; END");

    ExecuteSql("CREATE TRIGGER delete_viewProps AFTER DELETE ON " DGN_TABLE_View " BEGIN DELETE FROM " BEDB_TABLE_Property
                    " WHERE Namespace=\"" PROPERTY_APPNAME_DgnView "\" AND Id=OLD.Id; END");

    ExecuteSql("CREATE TRIGGER delete_materialProps AFTER DELETE ON " DGN_TABLE_Material " BEGIN DELETE FROM " BEDB_TABLE_Property
                    " WHERE Namespace=\"" PROPERTY_APPNAME_DgnMaterial "\" AND Id=OLD.Id; END");

    ExecuteSql("CREATE TRIGGER delete_sessnProps AFTER DELETE ON " DGN_TABLE_Session " BEGIN DELETE FROM " BEDB_TABLE_Property
                    " WHERE Namespace=\"" PROPERTY_APPNAME_DgnSession "\" AND Id=OLD.Id; END");

    ExecuteSql("CREATE TRIGGER delete_levelProps AFTER DELETE ON " DGN_TABLE_Level " BEGIN DELETE FROM " BEDB_TABLE_Property
                    " WHERE Namespace=\"" PROPERTY_APPNAME_DgnLevel "\" AND Id=OLD.Id; END");

    DgnSystemDomain::GetInstance()._OnProjectCreated(*this);
    DgnDraftingDomain::GetInstance()._OnProjectCreated(*this);

#if defined (NEEDS_WORK_DGNITEM)
    CreateTable(DGN_TABLE_Tag,              "Id INTEGER PRIMARY KEY,"
                                            "ECClassId INTEGER,"
                                            "ParentTag INTEGER REFERENCES " DGN_TABLE_Tag " ON DELETE CASCADE,"
                                            "Code CHAR NOT NULL");

    CreateTable(DGN_TABLE_Item,             "Id INTEGER PRIMARY KEY,"
                                            "ECClassId INTEGER,"
                                            "TagId INTEGER REFERENCES " DGN_TABLE_Tag " ON DELETE CASCADE,"
                                            "ModelId INTEGER REFERENCES " DGN_TABLE_Model " ON DELETE CASCADE,"
                                            "[Origin.X] REAL,"
                                            "[Origin.Y] REAL,"
                                            "[Origin.Z] REAL,"
                                            "Yaw REAL,"
                                            "Pitch REAL,"
                                            "Roll REAL,"
                                            "Left REAL,"
                                            "Right REAL,"
                                            "Bottom REAL,"
                                            "Top REAL,"
                                            "Front REAL,"
                                            "Back REAL,"
                                            "LastMod TIMESTAMP DEFAULT(julianday('now')),"
                                            "Geom BLOB");


    CreateTable(DGN_TABLE_Element,          "Id INTEGER PRIMARY KEY,"
                                            "ItemId INTEGER REFERENCES " DGN_TABLE_Item " ON DELETE CASCADE,"
                                            "HandlerId INTEGER,"
                                            "ModelId INTEGER REFERENCES " DGN_TABLE_Model " ON DELETE CASCADE,"
                                            "LevelId INT,"
                                            "[Min.X] REAL,"
                                            "[Max.X] REAL,"
                                            "[Min.Y] REAL,"
                                            "[Max.Y] REAL,"
                                            "[Min.Z] REAL,"
                                            "[Max.Z] REAL,"
                                            "Props INT,"
                                            "Symb BLOB,"
                                            "Priority INT,"
                                            "LastMod TIMESTAMP DEFAULT(julianday('now')),"
                                            "Size INT,"
                                            "Data BLOB");
#endif

    CreateTable(DGN_TABLE_ElmXAtt, 
                                            "ElementId INTEGER,"
                                            "HandlerId INTEGER NOT NULL,"
                                            "Id INTEGER NOT NULL,"
                                            "Flags INT,"
                                            "Size INT NOT NULL,"
                                            "Data BLOB,"
                                            "PRIMARY KEY(ElementId,HandlerId,Id)");

    ExecuteSql("CREATE TRIGGER delete_xatt AFTER DELETE ON " DGN_TABLE_Element " BEGIN DELETE FROM " DGN_TABLE_ElmXAtt
               " WHERE ElementId=OLD.Id; END");

    CreateTable(DGN_TABLE_RasterFile,"RasterId INTEGER PRIMARY KEY,"
                                            "Name CHAR NOT NULL,"
                                            "Header BLOB");

    CreateTable(DGN_TABLE_RasterData,"RasterId INTEGER,"
                                            "ResolutionLevel INT,"
                                            "RowBlockId INT,"
                                            "ColumnBlockId INT,"
                                            "Data BLOB");

    ExecuteSql("CREATE INDEX dgn_elemModelLevel ON " DGN_TABLE_Element "(ModelId,LevelId)");
    ExecuteSql("CREATE INDEX dgn_elemItem ON " DGN_TABLE_Element "(ItemId) WHERE ItemId IS NOT NULL");
    ExecuteSql("CREATE INDEX dgn_itemModel ON " DGN_TABLE_Item "(ModelId)");
    ExecuteSql("CREATE INDEX dgn_rasterDataIdx ON " DGN_TABLE_RasterData "(RasterId,ResolutionLevel,RowBlockId,ColumnBlockId)");
    ExecuteSql("CREATE TRIGGER delete_rasterData AFTER DELETE ON " DGN_TABLE_RasterFile " BEGIN DELETE FROM " DGN_TABLE_RasterData " WHERE RasterId=OLD.RasterId; END");

    // the old.LastMod=new.LastMod is to make it possible to set the LastMod explicitly, which is necessary for change merging.
    ExecuteSql("CREATE TRIGGER lastModElm AFTER UPDATE ON " DGN_TABLE_Element " WHEN old.LastMod=new.LastMod "
                "BEGIN UPDATE " DGN_TABLE_Element " SET LastMod=julianday('now') WHERE Id=new.Id; END");
    ExecuteSql("CREATE TRIGGER lastModItm AFTER UPDATE ON " DGN_TABLE_Item " WHEN old.LastMod=new.LastMod "
                "BEGIN UPDATE " DGN_TABLE_Item " SET LastMod=julianday('now') WHERE Id=new.Id; END");

    // save the DgnFile version property.
    SavePropertyString (DgnFileProperty::SchemaVersion(), getCurrentDgnFileVersion().ToJson());
    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnProject::SaveProjectSchemaVersion(DgnVersion version)
    {
    m_schemaVersion = version;
    return  SavePropertyString (DgnProjectProperty::SchemaVersion(), m_schemaVersion.ToJson());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnProject::InitializeProject (CreateProjectParams const& params)
    {
    if (params.GetGuid().IsValid())
        ChangeDbGuid(params.GetGuid());

    SaveMyProjectGuid (params.GetGuid());

    SaveProjectSchemaVersion();
    SaveCreationDate();

    SavePropertyString(DgnProjectProperty::LastEditor(), Utf8String(T_HOST.GetProductName()));
    SavePropertyString(DgnProjectProperty::Name(), params.m_name);
    SavePropertyString(DgnProjectProperty::Description(), params.m_description);
    SavePropertyString(DgnProjectProperty::Client(), params.m_client);

    KeyStrings().Insert (DGN_SETTYPE_ModelSelector);

    DgnModelSelectors::Selector selector ("Default", "default model selector");
    DbResult rc = ModelSelectors().InsertSelector (selector);
    BeAssert (rc==BE_SQLITE_OK);

    DgnSessions::Row session ("Default", "default session");
    rc = Sessions().InsertSession(session);
    BeAssert (rc==BE_SQLITE_OK);

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
    virtual DbResult _Upgrade (DgnProjectR project, DgnVersion version) = 0;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/13
//=======================================================================================
struct Upgrader_5202  : ProjectSchemaUpgrader
    {
    virtual DgnVersion _GetVersion() override  {return DgnVersion(5,2,0,2);}
    virtual DbResult _Upgrade (DgnProjectR project, DgnVersion version) override
        {
        if (version == DgnVersion(5,2,0,1))
            return BE_SQLITE_ERROR_ProfileTooOld; // this version is now hornswaggled

        // we also added the "Palette" column to the material table
        DbResult rc = project.ExecuteSql ("ALTER TABLE " DGN_TABLE_Material " ADD Palette CHAR");
        return (BE_SQLITE_OK != rc) ? BE_SQLITE_ERROR_ProfileUpgradeFailed : BE_SQLITE_OK;
        }
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/14
//=======================================================================================
struct Upgrader_5205  : ProjectSchemaUpgrader
{
    virtual DgnVersion _GetVersion() override  {return DgnVersion(5,2,0,5);}
    virtual DbResult _Upgrade (DgnProjectR project, DgnVersion version) override
        {
#if defined (NEEDS_WORK_DGNITEM)
        createSubLevelTable(project);
        project.ExecuteSql ("ALTER TABLE " DGN_TABLE_Level " ADD Scope INT");
        project.ExecuteSql ("ALTER TABLE " DGN_TABLE_Level " ADD Rank INT");
        project.ExecuteSql ("ALTER TABLE " DGN_TABLE_Level " ADD Props CHAR");
        
        if (SUCCESS != DgnECPersistence::UpgradeTables (project))
            return BE_SQLITE_ERROR;
#endif

        return BE_SQLITE_OK;
        }
};

//=======================================================================================
// data from the 5.2.0.5 and prior ViewFlags blob.
// @bsiclass                                                    Keith.Bentley   01/14
//=======================================================================================
struct ViewFlags5205
    {
    UInt32      fast_text:1;
    UInt32      line_wghts:1;
    UInt32      patterns:1;
    UInt32      text_nodes:1;
    UInt32      ed_fields:1;
    UInt32      grid:1;
    UInt32      lev_symb:1;         // removed
    UInt32      constructs:1;
    UInt32      dimens:1;
    UInt32      fast_cell:1;        // removed
    UInt32      fill:1;
    UInt32      auxDisplay:1;
    UInt32      renderMode:6;
    UInt32      textureMaps:1;
    UInt32      transparency:1;
    UInt32      inhibitLineStyles:1;
    UInt32      patternDynamics:1;
    UInt32      tagsOff:1;          // removed
    UInt32      renderDisplayEdges:1;
    UInt32      renderDisplayHidden:1;
    UInt32      overrideBackground:1;
    UInt32      noFrontClip:1;
    UInt32      noBackClip:1;
    UInt32      noClipVolume:1;
    UInt32      useDisplaySet:1;    // removed
    UInt32      associativeClip:1;  // removed
    UInt32      renderDisplayShadows:1;
    UInt32      hiddenLineStyle:3;
    UInt32      inhibitRenderMaterials:1;
    UInt32      ignoreSceneLights:1;
    UInt32      reserved;           // removed

    ViewFlags5205 (){memset (this, 0, sizeof(*this));}
    ViewFlags Convert() 
        {
        ViewFlags flags;
        flags.fast_text              = fast_text;
        flags.line_wghts             = line_wghts;
        flags.patterns               = patterns;
        flags.text_nodes             = text_nodes;
        flags.ed_fields              = ed_fields;
        flags.grid                   = grid;
        flags.constructs             = constructs;
        flags.dimens                 = dimens;
        flags.fill                   = fill;
        flags.auxDisplay             = auxDisplay;
        flags.renderMode             = renderMode;
        flags.textureMaps            = textureMaps;
        flags.transparency           = transparency;
        flags.inhibitLineStyles      = inhibitLineStyles;
        flags.patternDynamics        = patternDynamics;
        flags.renderDisplayEdges     = renderDisplayEdges;
        flags.renderDisplayHidden    = renderDisplayHidden;
        flags.overrideBackground        = overrideBackground;
        flags.noFrontClip            = noFrontClip;
        flags.noBackClip             = noBackClip;
        flags.noClipVolume           = noClipVolume;
        flags.renderDisplayShadows   = renderDisplayShadows;
        flags.hiddenLineStyle        = hiddenLineStyle;
        flags.inhibitRenderMaterials = inhibitRenderMaterials;
        flags.ignoreSceneLights      = ignoreSceneLights;
        return flags;
        }
    };

/*---------------------------------------------------------------------------------**//**
* We used to write the view settings to multiple properties. In 5.4.0.0 we converted it to a single Json string (and removed a few members).
* This function pulls the data out of the old properties and creates and saves the Json string.
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void upgradeViewSettings5400 (DgnProjectR project)
    {
    DgnViewProperty::ViewSetting flagsSpec("Flags");
    DgnViewProperty::ViewSetting physicalViewSpec("PhysicalView");
    DgnViewProperty::ViewSetting viewArea2dSpec("ViewArea2d");
    DgnViewProperty::ViewSetting backgroundColorSpec("BgColor");
    DgnViewProperty::ViewSetting levelMaskSpec("LevelMask");

    auto views = project.Views();
    for (auto view : views.MakeIterator())
        {
        auto viewId = view.GetDgnViewId();
        Json::Value val;
        
        ViewFlags5205 oldFlags;
        views.QueryProperty (&oldFlags, sizeof(oldFlags), viewId, flagsSpec);

        Utf8String oldView;
        if (BE_SQLITE_ROW == views.QueryProperty (oldView, viewId, physicalViewSpec))
            {
            Json::Reader::Parse (oldView, val);
            oldFlags.Convert().To3dJson(val["flags"]);
            }
        else if (BE_SQLITE_ROW == views.QueryProperty (oldView, viewId, viewArea2dSpec))
            {
            Json::Reader::Parse (oldView, val["area2d"]);
            }

        oldFlags.Convert().ToBaseJson(val["flags"]);

        RgbColorDef backgroundColor;
        if (BE_SQLITE_ROW == views.QueryProperty (&backgroundColor, sizeof(backgroundColor), viewId, backgroundColorSpec))
            {
            val["bgColor"] = IntColorDef(backgroundColor).AsUInt32();
            }

        Utf8String levelMask;
        if (BE_SQLITE_ROW == views.QueryProperty (levelMask, viewId, levelMaskSpec))
            val["levels"] = levelMask;

        views.SavePropertyString(viewId, DgnViewProperty::Settings(), Json::FastWriter::ToString(val));

        // delete old properties
        views.DeleteProperty(viewId, flagsSpec);
        views.DeleteProperty(viewId, physicalViewSpec);
        views.DeleteProperty(viewId, viewArea2dSpec);
        views.DeleteProperty(viewId, backgroundColorSpec);
        views.DeleteProperty(viewId, levelMaskSpec);
        }

    project.SaveSettings();
    }

/*---------------------------------------------------------------------------------**//**
* In 5.4.0.0 we changed the units to always store coordinates in Millimeters,
* So from now on UOR == StorageUnits == Millimeters. Therefore "uorPerStorage" is always 1.0 and has therefore been eliminated.
* We also renamed the property from "DgnFile:StorageUnits" to "DgnProject:Units" and consolidated all of the Geo-location 
* properties into the units property. Since we can't (really don't want to) transform the data from old published
* files, we leave it alone and save the units with the numerator and "uorPerStorage" combined so that coordinates
* are peserved, even though that means that old upgraded files have units other than millimeters.
* Note that this change only affects storage units. Each model still has MasterUnits and SubUnits, so
* coordinate data is unchanged from the user's perspective. 2d models still each have a global origin, but all physical
* models share the Project's global origin.
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void upgradeUnits5400(DgnProjectR project)
    {
    DgnUnits& units = project.Units();
    Utf8String value;
    Json::Value jsonObj;

    DgnFileProperty::FileProperty storageunitProp ("StorageUnits");
    if (BE_SQLITE_ROW == project.QueryProperty (value, storageunitProp))
        {
        if (Json::Reader::Parse(value, jsonObj))
            {
            DPoint3d globalOrigin;
            JsonUtils::DPoint3dFromJson(globalOrigin, jsonObj["globalOrigin"]);

            units.SetGlobalOrigin(globalOrigin);
            }
        else
            {
            BeAssert(false); // what happened??
            }
        project.DeleteProperty(storageunitProp);
        }

    DgnProjectProperty::ProjectProperty latProp("OriginLatitude");
    if (BE_SQLITE_ROW == project.QueryProperty (value, latProp))
        {
        if (Json::Reader::Parse(value, jsonObj))
            units.SetOriginLatitude(jsonObj.asDouble());
        project.DeleteProperty(latProp);
        }

    DgnProjectProperty::ProjectProperty longProp("OriginLongitude");
    if (BE_SQLITE_ROW == project.QueryProperty (value, longProp))
        {
        if (Json::Reader::Parse(value, jsonObj))
            units.SetOriginLongitude(jsonObj.asDouble());
        project.DeleteProperty(longProp);
        }

    DgnProjectProperty::ProjectProperty azProp("Azimuth");
    if (BE_SQLITE_ROW == project.QueryProperty (value, azProp))
        {
        if (Json::Reader::Parse(value, jsonObj))
            units.SetAzimuth(jsonObj.asDouble());
        project.DeleteProperty(azProp);
        }

    DgnProjectProperty::ProjectProperty geoOrignProp("GeoOriginBasis");
    if (BE_SQLITE_ROW == project.QueryProperty (value, geoOrignProp))
        {
        Json::Value arrayJsonObj (Json::arrayValue);
        if (Json::Reader::Parse(value, arrayJsonObj))
            {
            DPoint2d basis;
            JsonUtils::DPoint2dFromJson (basis, arrayJsonObj);
            units.SetGeoOriginBasis(basis);
            }
        project.DeleteProperty(geoOrignProp);
        }

    units.Save();

    DRange3d extent;
    if (BE_SQLITE_ROW != project.QueryProperty (&extent, sizeof(extent), DgnProjectProperty::ProjectProperty("Extents")))
        extent = DRange3d::NullRange();

    units.SaveProjectExtents(extent);
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/14
//=======================================================================================
struct Upgrader_5400 : ProjectSchemaUpgrader
{
    virtual DgnVersion _GetVersion() override  {return DgnVersion(5,4,0,0);}
    virtual DbResult _Upgrade (DgnProjectR project, DgnVersion version) override
        {
        // in this version, we changed the format of ranges from integer to double. We continue to read the old data,
        // but old versions can't read the new data. Therefore, no upgrading of the element data actually happens.

        // added the SubType column to both view and model tables.
        project.ExecuteSql ("ALTER TABLE " DGN_TABLE_View " ADD SubType CHAR");
        project.ExecuteSql ("ALTER TABLE " DGN_TABLE_Model " ADD SubType CHAR");

        createStampTable(project);

        // In version 5.4.0.0 we added the facet table. Pull the appearance data from the level table and put it in the facet table.
        // We have to leave the old columns, since SQLite doesn't support removing them.
        Statement stmt;
        stmt.Prepare (project, "SELECT Id,Color,Weight,Style,Material,Transp,DispPri FROM " DGN_TABLE_Level);

        while (BE_SQLITE_ROW == stmt.Step())
            {
            DgnLevels::SubLevel::Appearance appear;
            appear.SetColor(stmt.GetValueInt(1));
            appear.SetWeight(stmt.GetValueInt(2));
            appear.SetStyle(stmt.GetValueInt(3));
            appear.SetMaterial(stmt.GetValueId<DgnMaterialId>(4));
            appear.SetTransparency(stmt.GetValueDouble(5));
            appear.SetDisplayPriority(stmt.GetValueInt(6));
            DgnLevels::SubLevel subLevel(SubLevelId(LevelId(stmt.GetValueInt(0))), "", appear);
            project.Levels().InsertSubLevel(subLevel);
            }
        // there isn't anything in the level flags worth keeping - just throw them away.
        project.ExecuteSql ("UPDATE " DGN_TABLE_Level " SET Flags=Null");

        upgradeUnits5400(project);
        upgradeViewSettings5400(project);
        return BE_SQLITE_OK;
        }
};

static ProjectSchemaUpgrader* s_upgraders[] =
    {
    // NOTE: entries in this list *must* be sorted in ascending version order.
    new Upgrader_5202(),
    new Upgrader_5205(),
    new Upgrader_5400(),
    // Add a new version here
    };

/*---------------------------------------------------------------------------------**//**
* Each call to _DoUpgrade will upgrade the schema from its stored version to the immediately succeeding version.
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnProject::OpenParams::_DoUpgrade (DgnProjectR project, DgnVersion& version) const
    {
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
    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* The schema stored in the newly opened project is too old. Perform an upgrade, if possible.
* @bsimethod                                    Keith.Bentley                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnProject::OpenParams::UpgradeSchema(DgnProjectR project) const
    {
    if (!_ReopenForSchemaUpgrade (project))
        return BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite;

    DgnVersion version = project.GetSchemaVersion();
    for (;;)
        {
        DbResult stat = _DoUpgrade(project, version);
        if (BE_SQLITE_OK != stat)
            return stat;

        project.SaveProjectSchemaVersion(version);

        // Stop when we get to the current version.
        if (getCurrentSchemaVerion() == version)
            break;
        }

    return project.SaveChanges();
    }

DgnVersion DgnProject::GetSchemaVersion() {return m_schemaVersion;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnProject::_VerifySchemaVersion (Db::OpenParams const& params)
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
    DgnVersion minimumAutoUpgradableVersion (PROJECT_SUPPORTED_VERSION_Major, PROJECT_SUPPORTED_VERSION_Minor, 0, 0);

    bool profileIsAutoUpgradable = false;
    stat = CheckProfileVersion (profileIsAutoUpgradable, expectedVersion, m_schemaVersion, minimumAutoUpgradableVersion, params.IsReadonly(), "DgnDb");

    return profileIsAutoUpgradable ? ((DgnProject::OpenParams&)params).UpgradeSchema(*this) : stat;
    }

/*---------------------------------------------------------------------------------**//**
* Upgrade the DgnFile from an older version to the immediately succeeeding version.
* This method should upgrade the DgnFile for a SINGLE CHANGE ONLY and then return. That's so subclasses can particpate
* in the upgrade process one step at a time looking only for the upgrade from one specific version. It will be called
* repeatedly until the file is upgraded to the current verions.
* @bsimethod                                    Keith.Bentley                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFileStatus DgnProject::OpenParams::_DoDgnFileUpgrade(DgnProjectR project, DgnVersion& from) const
    {
    from = getCurrentDgnFileVersion();
    return  DGNFILE_STATUS_Success;
    }

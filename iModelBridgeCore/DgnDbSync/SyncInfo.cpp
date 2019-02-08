/*--------------------------------------------------------------------------------------+
|
|     $Source: SyncInfo.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include "DgnV8/DynamicSchemaGenerator/ECConversion.h"
#include <GeomJsonWireFormat/JsonUtils.h>

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"DgnV8Converter.SyncInfo"))

#define MUSTBEDBRESULT(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {SetLastError(rc); return BSIERROR;}}
#define MUSTBEOK(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_OK)
#define MUSTBEROW(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_ROW)
#define MUSTBEDONE(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_DONE)

#define MUSTBEDBRESULTRC(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {SetLastError(rc); return rc;}}
#define MUSTBEOKRC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_OK)
#define MUSTBEROWRC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_ROW)
#define MUSTBEDONERC(stmt) MUSTBEDBRESULTRC(stmt,BE_SQLITE_DONE)

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

static rapidjson::Value fixedArrayToJson(double const* darray, size_t count, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    rapidjson::Value jdbls;
    jdbls.SetArray();
    jdbls.Reserve(count, allocator);
    for (size_t i=0; i<count; ++i)
        jdbls.PushBack(darray[i], allocator);
    return jdbls;
    }

static void fixedArrayFromJson(double* darray, size_t count, rapidjson::Value const& jdbls)
    {
    if (jdbls.Size() != count)
        {
        BeAssert(false);
        return;
        }
    for (int i=0; i<count; ++i)
        {
        darray[i] = jdbls[i].GetDouble();
        }
    }

static ProfileVersion s_currentVersion(0, 1, 0, 0);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SyncInfo::SavePropertyString(PropertySpecCR spec, Utf8CP stringData, uint64_t id, uint64_t subId)
    {
    Statement stmt;
    auto rc = stmt.Prepare(*m_dgndb, "INSERT OR REPLACE INTO " SYNCINFO_ATTACH(BEDB_TABLE_Property) " (Namespace,Name,Id,SubId,TxnMode,StrData) VALUES(?,?,?,?,?,?)");
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SyncInfo::QueryProperty(Utf8StringR value, PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    Statement stmt;
    DbResult rc = stmt.Prepare(*m_dgndb, "SELECT StrData FROM " SYNCINFO_ATTACH(BEDB_TABLE_Property) " WHERE Namespace=? AND Name=? AND Id=? AND SubId=?");
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::CreateTables()
    {
    if (nullptr == m_dgndb)
        {
        BeAssert(false);
        return BSIERROR;
        }


    if (!m_dgndb->TableExists(SYNCINFO_ATTACH(SYNC_TABLE_ECSchema)))
        {
        m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_ECSchema),
                         "V8Id INTEGER PRIMARY KEY,"
                         "RepositoryLinkId BIGINT NOT NULL,"
                         "V8Name TEXT NOT NULL,"
                         "V8VersionMajor INTEGER NOT NULL,"
                         "V8VersionMinor INTEGER NOT NULL,"
                         "MappingType INTEGER NOT NULL,"
                         "LastModified TIMESTAMP,"
                         "Digest INTEGER");

        //need a unique index to ensure uniqueness for schemas based on checksum
        Utf8String ddl;
        ddl.Sprintf("CREATE UNIQUE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_ECSchema) "_variantxml_uix ON "  SYNC_TABLE_ECSchema "(V8Name, RepositoryLinkId, Digest);");
        MUSTBEOK(m_dgndb->ExecuteSql(ddl.c_str()));
        //need a index on the entire table for fast look ups
        MUSTBEOK(m_dgndb->ExecuteSql("CREATE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_ECSchema) "_ix ON "  SYNC_TABLE_ECSchema "(V8Name);"));

        CreateNamedGroupTable(true);
        }

    CreateECTables();

    m_dgndb->SaveChanges();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::CreateECTables()
    {
    V8ECClassInfo::CreateTable(*m_dgndb);
    ECInstanceInfo::CreateTable(*m_dgndb);
    V8ECSchemaXmlInfo::CreateTable(*m_dgndb);
    V8ElementSecondaryECClassInfo::CreateTable(*m_dgndb);
    }

// TOOD: Get rid of this
struct SyncInfoProperty
{
    struct Spec : BeSQLite::PropertySpec
        {
        Spec(BentleyApi::Utf8CP name) : PropertySpec(name, "SyncInfo", PropertySpec::Mode::Normal, PropertySpec::Compress::No) {}
        };

    static Spec ProfileVersion()       {return Spec("SchemaVersion");}
    static Spec DgnDbGuid()            {return Spec("DgnDbGuid");}
    static Spec DbProfileVersion()     {return Spec("DbSchemaVersion");}
    static Spec DgnDbProfileVersion()  {return Spec("DgnDbSchemaVersion");}
};

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
BentleyStatus SyncInfo::PerformVersionChecks()
    {
    //  Look at the stored version and see if we have to upgrade
    Utf8String versionString;
    MUSTBEROW(QueryProperty(versionString, SyncInfoProperty::ProfileVersion()));

    ProfileVersion storedVersion(0, 0, 0, 0);
    storedVersion.FromJson(versionString.c_str());

    if (storedVersion.CompareTo(s_currentVersion) == 0)
        return BSISUCCESS;

    if (storedVersion.CompareTo(s_currentVersion) > 0)
        { // version is too new!
        LOG.errorv("compatibility error - storedVersion=%s > currentVersion=%s", versionString.c_str(), s_currentVersion.ToJson().c_str());
        return BSIERROR;
        }

    //  Upgrade - when we change the syncInfo schema, add upgrade steps here ...

    //  Upgraded. Update the stored version.
    MUSTBEOK(SavePropertyString(SyncInfoProperty::ProfileVersion(), s_currentVersion.ToJson().c_str()));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SyncInfo::InsertFont(DgnFontId newId, V8FontId oldId)
    {
    m_font[oldId] = newId;

    // WIP_CONVERTER -- write syncinfo 
    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFontId SyncInfo::FindFont(V8FontId oldId)
    {
    // WIP_CONVERTER -- read syncInfo
    auto i = m_font.find(oldId);
    return i == m_font.end() ? DgnFontId() : i->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
RenderMaterialId SyncInfo::FindMaterialByV8Id(uint64_t id, DgnV8FileR v8File, DgnV8ModelR v8Model)
    {
    DgnV8Api::Material const*    material;

    if (NULL == (material = DgnV8Api::MaterialManager::GetManagerR().FindMaterial(NULL, DgnV8Api::MaterialId(id), v8File, v8Model, true)))
        return RenderMaterialId();

    return m_converter.GetRemappedMaterial(material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SyncInfo::InsertLineStyle(DgnStyleId newId, double componentScale, V8StyleId oldId)
    {
    MappedLineStyle mapEntry(newId, componentScale);
    m_lineStyle[oldId] = mapEntry;

    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------**//**
 _RemapLineStyle adds an entry with an invalid ID when it finds an entry that cannot
 be mapped (probably because the original definition could not be found).  When FindLineStyle
 returns an invalid DgnStyleId, _RemapLineStyle needs to know if it is because the entry was 
 never added or because the entry was added with an invalid DgnStyleId. It uses foundStyle
 to determine that.

* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyleId SyncInfo::FindLineStyle(double&unitsScale, bool& foundStyle, V8StyleId oldId)
    {
    // WIP_CONVERTER -- read syncInfo
    auto i = m_lineStyle.find(oldId);
    if (i == m_lineStyle.end())
        {
        unitsScale = 1;
        foundStyle = false;
        return DgnStyleId();
        }

    foundStyle = true;
    unitsScale = i->second.m_unitsScale;
    return i->second.m_id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::DiskFileInfo::GetInfo(BeFileNameCR fileName)
    {
    time_t mtime;
    if (BeFileName::GetFileSize(m_fileSize, fileName.c_str()) != BeFileNameStatus::Success
        || BeFileName::GetFileTime(nullptr, nullptr, &mtime, fileName.c_str()) != BeFileNameStatus::Success)
        return BSIERROR;

    m_lastModifiedTime = mtime;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SyncInfo::GetUniqueNameForFile(DgnV8FileCR file)
    {
    //  The unique name is the key into the syncinfo_file table. 
    //  Therefore, we must distinguish between like-named files in different directories.
    //  The unique name must also be stable. If the whole project is moved to a new directory or machine, 
    //  the unique names of the files must be unaffected.

    // If we have a DMS URN for the document corresponding to this file, that is the unique name.
    Utf8String urn = GetConverter().GetDocumentURNforFile(file);
    if (!urn.empty())
        {
        if (iModelBridge::IsNonFileURN(urn))
            return urn;

        // I would rather use the doc GUID than a file:// URL or a file path.
        iModelBridgeDocumentProperties docProps;
        GetConverter().GetDocumentProperties(docProps, BeFileName(file.GetFileName().c_str()));
        if (!docProps.m_docGuid.empty())
            return docProps.m_docGuid;

        return urn;
        }

    // If we do not have a DMS URN or a doc GUID, we try to compute a stable unique name from the filename.
    // The full path should be unique already. To get something that is stable, we use only as much of 
    // the full path as we need to distinguish between like-named files in different directories.
    BeFileName fullFileName(file.GetFileName().c_str());
    WString uniqueName(fullFileName);
    auto pdir = m_converter.GetParams().GetInputRootDir();
    if (!pdir.empty() && (pdir.size() < fullFileName.size()) && pdir.EqualsI(fullFileName.substr(0, pdir.size())))
        uniqueName = fullFileName.substr(pdir.size());

    uniqueName.ToLower();  // make sure we don't get fooled by case-changes in file system on Windows
    return Utf8String(uniqueName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::RepositoryLinkExternalSourceAspect SyncInfo::FindFileByFileName(BeFileNameCR fullFileName)
    {
    // Make the worst-case assumption that the local files have been moved to a new directory.
    // Consider only the relative path of the file.
    auto dirPrefix = m_converter.GetParams().GetInputRootDir();
    size_t prefixLen = dirPrefix.size();
    if (!fullFileName.StartsWithI(dirPrefix.c_str())) // ??!
        prefixLen = 0;

    Utf8String searchName(fullFileName.substr(prefixLen));

#ifdef DOES_NOT_WORK_IF_NAME_IS_FILE_URL
    RepositoryLinkExternalSourceAspectIterator rlinkIter(GetConverter().GetDgnDb(), "(json_extract(JsonProperties, '$.fileName') LIKE :searchName)");
    rlinkIter.GetStatement()->BindText(rlinkIter.GetParameterIndex("searchName"), searchName.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    for (auto aspect : rlinkIter)
        {
        if (aspect.GetFileName().EndsWithI(searchName.c_str()))
            return aspect;
        }
#else
    RepositoryLinkExternalSourceAspectIterator rlinkIter(GetConverter().GetDgnDb());
    for (auto aspect : rlinkIter)
        {
        if (aspect.GetFileName().EndsWithI(searchName.c_str()))
            return aspect;
        }
#endif
    return RepositoryLinkExternalSourceAspect(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool SyncInfo::HasDiskFileChanged(BeFileNameCR fileName)
    {
    RepositoryLinkExternalSourceAspect prov = FindFileByFileName(fileName);
    if (!prov.IsValid())
        return true;

    // This is an attempt to tell if a file has *not* changed, looking only at the file's time and size.
    // This is a dangerous test, since we don't look at the contents, but we think that we can narrow 
    // the odds of a mistake by:
    // 1. using times measured in hectonanoseconds (on Windows, at least),
    // 2. also using file size
    SyncInfo::DiskFileInfo df;
    df.GetInfo(fileName);
    return df.m_lastModifiedTime != prov.GetLastModifiedTime() || df.m_fileSize != prov.GetFileSize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool SyncInfo::HasLastSaveTimeChanged(DgnV8FileCR v8File)
    {
    if (v8File.IsEmbeddedFile())
        return false;

    V8FileInfo finfo = ComputeFileInfo(v8File);
    auto previous = RepositoryLinkExternalSourceAspect::FindAspectByIdentifier(*GetDgnDb(), finfo.m_uniqueName);
    if (!previous.IsValid())
        return true;

    auto lastSaveTime = const_cast<DgnV8FileR>(v8File).GetLastSaveTime();

    // a non-DGN FileIO may not set the last saved time in the file header - resort to the last modified time in such a case:
    if (0.0 == lastSaveTime)
        {
        BeFileName  filename(v8File.GetFileName().c_str());
        SyncInfo::DiskFileInfo diskfile;
        diskfile.GetInfo (filename);
        return diskfile.m_lastModifiedTime != previous.GetLastModifiedTime();
        }

    return lastSaveTime != previous.GetLastSaveTime();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::ProxyGraphicExternalSourceAspect SyncInfo::ProxyGraphicExternalSourceAspect::CreateAspect(DgnModelCR bimDrawingModel, Utf8StringCR idPath, Utf8StringCR propsJson, DgnDbR db) 
    {
    auto aspectClass = GetAspectClass(db);
    if (nullptr == aspectClass)
        return ProxyGraphicExternalSourceAspect(nullptr);

    auto instance = CreateInstance(bimDrawingModel.GetModeledElementId(), Kind::ProxyGraphic, idPath, nullptr, *aspectClass);
    auto aspect = ProxyGraphicExternalSourceAspect(instance.get());

    if (!propsJson.empty())
        {
        rapidjson::Document json(rapidjson::kObjectType);
        json.Parse(propsJson.c_str());
        aspect.SetProperties(json);
        }

    return aspect;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Sam.Wilson      1/19
//+---------------+---------------+---------------+---------------+---------------+------
DgnElementId SyncInfo::ProxyGraphicExternalSourceAspect::FindDrawingGraphic (DgnModelCR proxyGraphicScope, Utf8StringCR sectionedV8ElementPath, DgnCategoryId drawingGraphicCategory, DgnClassId drawingGraphicClassId, DgnDbR db)
    {
    auto drawingGraphicClass = db.Schemas().GetClass(drawingGraphicClassId);
    Utf8PrintfString ecsql(
        "SELECT dg.ECInstanceId FROM %s dg, " XTRN_SRC_ASPCT_FULLCLASSNAME " x"
        " WHERE x.Scope.Id=? AND dg.Category.Id=? AND x.Element.Id=dg.ECInstanceId AND x.Kind='ProxyGraphic' AND x.Identifier=?",
        drawingGraphicClass? drawingGraphicClass->GetFullName(): BIS_SCHEMA(BIS_CLASS_DrawingGraphic));
    auto stmt = db.GetPreparedECSqlStatement(ecsql.c_str());
    int col=1;
    stmt->BindId(col++, proxyGraphicScope.GetModelId());
    stmt->BindId(col++, drawingGraphicCategory);
    stmt->BindText(col++, sectionedV8ElementPath.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW == stmt->Step())
        {
        auto eid = stmt->GetValueId<DgnElementId>(0);
        BeAssert((BE_SQLITE_DONE == stmt->Step()) && "There should be only one ProxyGraphic XSA on a DrawingGraphic element with a given category");
        return eid;
        }
    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::LevelExternalSourceAspect::FindFirstSubCategory(DgnSubCategoryId& subCatId, DgnV8ModelCR v8Model, uint32_t levelId, Type ltype, Converter& converter)
    {
    DgnElementId repositoryLinkId = converter.GetRepositoryLinkId(*v8Model.GetDgnFileP());
    BeAssert(repositoryLinkId.IsValid());
    Utf8String v8LevelId = FormatSourceId(levelId);
    Utf8String v8ModelId = V8ModelExternalSourceAspect::FormatSourceId(v8Model);
    auto desiredCategoryClassId = converter.GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, (Type::Spatial == ltype)? BIS_CLASS_SpatialCategory: BIS_CLASS_DrawingCategory);
    
    auto aspectStmt = converter.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT x.Element.Id, x.JsonProperties FROM " XTRN_SRC_ASPCT_FULLCLASSNAME " x, " BIS_SCHEMA(BIS_CLASS_SubCategory) " e"
        " WHERE (x.Element.Id=e.ECInstanceId AND x.Scope.Id=? AND x.Kind=? AND x.Identifier=? AND json_extract(x.JsonProperties, '$.v8ModelId') = ?)");
    aspectStmt->BindId(1, repositoryLinkId);
    aspectStmt->BindText(2, Kind::Level, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    aspectStmt->BindText(3, v8LevelId.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    aspectStmt->BindText(4, v8ModelId.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);

    BeSQLite::EC::CachedECSqlStatementPtr catClassStmt;

    while (BE_SQLITE_ROW == aspectStmt->Step())
        {
        subCatId = aspectStmt->GetValueId<DgnSubCategoryId>(0);

        // Must be a SubCategory of the requested type of Category (spatial or drawing)
        if (!catClassStmt.IsValid())
            catClassStmt = converter.GetDgnDb().GetPreparedECSqlStatement(
                "SELECT c.ECClassId FROM " BIS_SCHEMA(BIS_CLASS_Category) " c JOIN " BIS_SCHEMA(BIS_CLASS_SubCategory) " s ON c.ECInstanceId=s.Parent.Id"
                " WHERE s.ECInstanceId=?");

        catClassStmt->Reset();
        catClassStmt->BindId(1, subCatId);
        if (BE_SQLITE_ROW != catClassStmt->Step())
            continue;
        if (catClassStmt->GetValueId<ECN::ECClassId>(0) != desiredCategoryClassId)
            continue;

        return BSISUCCESS;
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::LevelExternalSourceAspect SyncInfo::LevelExternalSourceAspect::CreateAspect(DgnElementId scopeId, DgnV8Api::LevelHandle const& vlevel, DgnV8ModelCR v8Model, Converter& converter)
    {
    BeAssert(scopeId.IsValid());

    auto aspectClass = GetAspectClass(converter.GetDgnDb());
    if (nullptr == aspectClass)
        return LevelExternalSourceAspect(nullptr);
    
    DgnElementId repositoryLinkId = converter.GetRepositoryLinkId(*v8Model.GetDgnFileP());
    BeAssert(repositoryLinkId.IsValid());
    auto instance = CreateInstance(repositoryLinkId, Kind::Level, FormatSourceId(vlevel.GetLevelId()), nullptr, *aspectClass);
    
    LevelExternalSourceAspect aspect(instance.get());
    
    Utf8String v8LevelName(vlevel.GetName());
    Utf8String v8ModelId = V8ModelExternalSourceAspect::FormatSourceId(v8Model);

    rapidjson::Document json(rapidjson::kObjectType);
    auto& allocator = json.GetAllocator();
    json.AddMember("v8ModelId", rapidjson::Value(v8ModelId.c_str(), allocator), allocator);
    json.AddMember("v8LevelName", rapidjson::Value(v8LevelName.c_str(), allocator), allocator);
    aspect.SetProperties(json);

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::LevelExternalSourceAspect SyncInfo::LevelExternalSourceAspect::CreateAspect(DgnV8Api::LevelHandle const& vlevel, DgnV8ModelCR v8Model, Converter& converter)
    {
    return CreateAspect(converter.GetRepositoryLinkId(*v8Model.GetDgnFileP()), vlevel, v8Model, converter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::LevelExternalSourceAspect SyncInfo::InsertLevel(DgnSubCategoryId subcategoryid, DgnV8ModelCR v8model, DgnV8Api::LevelHandle const& vlevel)
    {
    auto catid = DgnSubCategory::QueryCategoryId(*GetDgnDb(), subcategoryid);
    LevelExternalSourceAspect::Type ltype = m_converter.IsSpatialCategory(catid)? LevelExternalSourceAspect::Type::Spatial: LevelExternalSourceAspect::Type::Drawing;

    auto subCatEl = GetDgnDb()->Elements().GetForEdit<DgnSubCategory>(subcategoryid);

    auto existingSubCatId = FindSubCategory(vlevel.GetLevelId(), v8model, ltype);
    if (existingSubCatId.IsValid())
        {
        // Make sure this is not a dup. We store only 1 aspect per V8Model. On the other hand, many levels from different V8Models
        // may be mapped to the same SubCategory. We check here to see if this level from this V8Model has already been added.
        // Don't know why that happens ... multiple self refs??
        auto aspect = LevelExternalSourceAspect::FindAspectByV8Model(*GetDgnDb()->Elements().Get<DgnSubCategory>(existingSubCatId), v8model);
        if (aspect.IsValid())
            return aspect;
        }

    auto aspect = LevelExternalSourceAspect::CreateAspect(vlevel, v8model, m_converter);
    aspect.AddAspect(*subCatEl);
    subCatEl->Update();
    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::LevelExternalSourceAspect SyncInfo::LevelExternalSourceAspect::FindAspectByV8Model(DgnSubCategoryCR el, DgnV8ModelCR v8Model)
    {
    auto stmt = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT ECInstanceId, JsonProperties FROM " XTRN_SRC_ASPCT_FULLCLASSNAME
        " WHERE (Element.Id=? AND Kind=? AND json_extract(JsonProperties, '$.v8ModelId') = ?)");
    stmt->BindId(1, el.GetElementId());
    stmt->BindText(2, Kind::Level, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindText(3, FormatV8ModelId(v8Model.GetModelId()).c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;
    return LevelExternalSourceAspect(ExternalSourceAspect::GetAspect(el, stmt->GetValueId<BeSQLite::EC::ECInstanceId>(0)).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId SyncInfo::FindSubCategory(uint32_t v8levelId, DgnV8FileR v8File, LevelExternalSourceAspect::Type ltype)
    {
    RepositoryLinkId fid = m_converter.GetRepositoryLinkId(v8File);
    DgnSubCategoryId glid;
    return (FindFirstSubCategory(glid, *m_dgndb, v8File.GetDictionaryModel(), v8levelId, ltype) == BSISUCCESS) ? glid : DgnSubCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId SyncInfo::FindCategory(uint32_t v8levelId, DgnV8FileR v8File, LevelExternalSourceAspect::Type ltype)
    {
    DgnSubCategoryId subcatid = FindSubCategory(v8levelId, v8File, ltype);
    return DgnSubCategory::QueryCategoryId(*GetDgnDb(), subcatid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId SyncInfo::FindSubCategory(uint32_t v8levelId, DgnV8ModelCR v8Model, LevelExternalSourceAspect::Type ltype)
    {
    DgnSubCategoryId glid;
    return (FindFirstSubCategory(glid, *m_dgndb, v8Model, v8levelId, ltype) == BSISUCCESS) ? glid : DgnSubCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId SyncInfo::GetSubCategory(uint32_t v8levelId, DgnV8ModelCR v8Model, LevelExternalSourceAspect::Type ltype)
    {
    DgnSubCategoryId glid;
    if (FindFirstSubCategory(glid, *m_dgndb, v8Model, v8levelId, ltype) != BSISUCCESS
        && FindFirstSubCategory(glid, *m_dgndb, v8Model.GetDgnFileP()->GetDictionaryModel(), v8levelId, ltype) != BSISUCCESS)
        {
        return DgnCategory::GetDefaultSubCategoryId(GetConverter().GetUncategorizedCategory()); // unable to categorize
        }

    return glid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t Converter::GetV8Level(DgnV8EhCR v8Eh)
    {
    uint32_t v8Level = v8Eh.GetElementCP()->ehdr.level;
    if (0 == v8Level && v8Eh.GetElementCP()->IsComplexHeader()) // level of cell header is not valid for category...
        {
        DgnV8Api::ElementHandle v8TemplateEh;
        if (DgnV8Api::ComplexHeaderDisplayHandler::GetComponentForDisplayParams(v8TemplateEh, v8Eh))
            v8Level = v8TemplateEh.GetElementCP()->ehdr.level; // find based on level of first component...
        }
    return v8Level;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId SyncInfo::GetCategory(DgnV8EhCR v8Eh, ResolvedModelMapping const& v8mm)
    {
    if (!v8Eh.GetElementCP()->ehdr.isGraphics)
        return v8mm.GetDgnModel().Is2dModel() ? GetConverter().GetUncategorizedDrawingCategory() : GetConverter().GetUncategorizedCategory(); // level of non-graphic element is not valid for category...

    uint32_t v8Level = Converter::GetV8Level(v8Eh);
    LevelExternalSourceAspect::Type ltype = v8mm.GetDgnModel().Is3d() ? LevelExternalSourceAspect::Type::Spatial : LevelExternalSourceAspect::Type::Drawing;
    DgnCategoryId categoryId;
    if (0 != v8Level)
        categoryId = FindCategory(v8Level, *v8Eh.GetDgnModelP()->GetDgnFileP(), ltype);

    return (categoryId.IsValid() ? categoryId : GetConverter().GetUncategorizedCategory()); // return uncategorized if we didn't find a valid category...
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::AttachToProject(DgnDb& targetProject, BeFileNameCR dbName)
    {
    DbResult rc = targetProject.AttachDb(Utf8String(dbName).c_str(), SYNCINFO_ATTACH_ALIAS);
    if (BE_SQLITE_OK != rc)
        return BSIERROR;
    return OnAttach(targetProject);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceId SyncInfo::GetSoleAspectIdByKind(DgnElementCR el, Utf8CP kind)
    {
    auto sel = el.GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId from " XTRN_SRC_ASPCT_FULLCLASSNAME " WHERE (Element.Id=? AND Kind=?)");
    sel->BindId(1, el.GetElementId());
    sel->BindText(2, kind, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW != sel->Step())
        return BeSQLite::EC::ECInstanceId();
    auto id = sel->GetValueId<BeSQLite::EC::ECInstanceId>(0);
    BeAssert(BE_SQLITE_ROW != sel->Step());
    return id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<BeSQLite::EC::ECInstanceId> SyncInfo::GetExternalSourceAspectIds(DgnElementCR el, Utf8CP kind, Utf8StringCR sourceId)
    {
    auto sel = el.GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId from " XTRN_SRC_ASPCT_FULLCLASSNAME " WHERE (Element.Id=? AND Kind=? AND Identifier=?)");
    sel->BindId(1, el.GetElementId());
    sel->BindText(2, kind, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    sel->BindText(3, sourceId.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    bvector<BeSQLite::EC::ECInstanceId> ids;
    while (BE_SQLITE_ROW == sel->Step())
        ids.push_back(sel->GetValueId<BeSQLite::EC::ECInstanceId>(0));
    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ElementExternalSourceAspect SyncInfo::V8ElementExternalSourceAspect::GetAspectForEdit(DgnElementR el, Utf8StringCR sourceId)
    {
    auto ids = SyncInfo::GetExternalSourceAspectIds(el, Kind::Element, sourceId);
    if (ids.size() == 0)
        return V8ElementExternalSourceAspect(nullptr);
    BeAssert(ids.size() == 1 && "Not supporting multiple element kind aspects on a single bim element from a given sourceId");
    return V8ElementExternalSourceAspect(ExternalSourceAspect::GetAspectForEdit(el, ids.front()).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ElementExternalSourceAspect SyncInfo::V8ElementExternalSourceAspect::GetAspect(DgnElementCR el, Utf8StringCR sourceId)
    {
    auto ids = SyncInfo::GetExternalSourceAspectIds(el, Kind::Element, sourceId);
    if (ids.size() == 0)
        return V8ElementExternalSourceAspect(nullptr);
    BeAssert(ids.size() == 1 && "Not supporting multiple element kind aspects on a single bim element from a given sourceId");
    return V8ElementExternalSourceAspect(ExternalSourceAspect::GetAspect(el, ids.front()).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ElementExternalSourceAspect SyncInfo::V8ElementExternalSourceAspect::CreateAspect(V8ElementExternalSourceAspectData const& provdata, DgnDbR db) 
    {
    auto aspectClass = GetAspectClass(db);
    if (nullptr == aspectClass)
        return V8ElementExternalSourceAspect(nullptr);

    auto sourceId = !provdata.m_v8IdPath.empty()? provdata.m_v8IdPath: FormatSourceId(provdata.m_v8Id); 

    auto instance = CreateInstance(DgnElementId(provdata.m_scope.GetValue()), Kind::Element, sourceId, nullptr, *aspectClass);
    auto aspect = V8ElementExternalSourceAspect(instance.get());

    if (!provdata.m_propsJson.empty())
        {
        rapidjson::Document propsData(rapidjson::kObjectType);
        propsData.Parse(provdata.m_propsJson.c_str());
        aspect.SetProperties(propsData);
        }

    aspect.Update(provdata.m_prov);

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool SyncInfo::V8ElementExternalSourceAspect::DoesProvenanceMatch(ElementProvenance const& elprov) const
    {
    auto ss = GetSourceState();

    Utf8String provLastMod;
    iModelExternalSourceAspect::DoubleToString(provLastMod, elprov.m_lastModified);
    if (ss.m_version != provLastMod)
        return false;

    Utf8String provHash;
    iModelExternalSourceAspect::HexStrFromBytes(provHash, elprov.m_hash.m_buffer);
    return ss.m_checksum.Equals(provHash);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::V8ElementExternalSourceAspect::Update(ElementProvenance const& prov)
    {
    SourceState ss;
    iModelExternalSourceAspect::HexStrFromBytes(ss.m_checksum, prov.m_hash.m_buffer);
    DoubleToString(ss.m_version, prov.m_lastModified);
    SetSourceState(ss); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::ElementId SyncInfo::V8ElementExternalSourceAspect::GetV8ElementId() const
    {
    int64_t id = 0;
    sscanf(GetIdentifier().c_str(), "%lld", &id);
    return id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ModelExternalSourceAspect SyncInfo::V8ModelExternalSourceAspect::CreateAspect(DgnV8ModelCR v8Model, TransformCR transform, Converter& converter) 
    {
    auto aspectClass = GetAspectClass(converter.GetDgnDb());
    if (nullptr == aspectClass)
        return V8ModelExternalSourceAspect(nullptr);
    
    DgnElementId repositoryLinkId = converter.GetRepositoryLinkId(*v8Model.GetDgnFileP());
    auto instance = CreateInstance(repositoryLinkId, Kind::Model, FormatSourceId(v8Model), nullptr, *aspectClass);
    
    V8ModelExternalSourceAspect aspect(instance.get());
    
    Utf8String v8ModelName(v8Model.GetModelName());

    rapidjson::Document json(rapidjson::kObjectType);
    auto& allocator = json.GetAllocator();
    json.AddMember("transform", fixedArrayToJson((double*)&transform, 12, allocator), allocator);
    json.AddMember("v8ModelName", rapidjson::Value(v8ModelName.c_str(), allocator), allocator);
    aspect.SetProperties(json);

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
std::tuple<DgnElementPtr, SyncInfo::V8ModelExternalSourceAspect> SyncInfo::V8ModelExternalSourceAspect::GetAspectForEdit(DgnModelR model)
    {
    auto el = model.GetModeledElement()->CopyForEdit();
    auto aspectId = SyncInfo::GetSoleAspectIdByKind(*el, Kind::Model);
    return std::make_tuple(el, V8ModelExternalSourceAspect(iModelExternalSourceAspect::GetAspectForEdit(*el, aspectId).m_instance.get()));
    }

std::tuple<DgnElementCPtr, SyncInfo::V8ModelExternalSourceAspect> SyncInfo::V8ModelExternalSourceAspect::GetAspect(DgnModelCR model)
    {
    auto el = model.GetModeledElement();
    auto aspectId = SyncInfo::GetSoleAspectIdByKind(*el, Kind::Model);
    return std::make_tuple(el, V8ModelExternalSourceAspect(iModelExternalSourceAspect::GetAspect(*el, aspectId).m_instance.get()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ModelExternalSourceAspect SyncInfo::V8ModelExternalSourceAspect::GetAspectByAspectId(DgnDbR db, BeSQLite::EC::ECInstanceId aspectId)
    {
    auto stmt = db.GetPreparedECSqlStatement("SELECT Element.Id FROM " XTRN_SRC_ASPCT_FULLCLASSNAME " WHERE (ECInstanceId=?)");
    stmt->BindId(1, aspectId);
    if (BE_SQLITE_ROW != stmt->Step())
        {
        BeAssert(false && "invalid aspectId");
        return V8ModelExternalSourceAspect();
        }
    auto el = db.Elements().GetElement(stmt->GetValueId<DgnElementId>(0));
    if (!el.IsValid())
        {
        BeAssert(false && "How could I find an aspect of an element and not be able to get the element itself?");
        return V8ModelExternalSourceAspect();
        }

    auto aspect = iModelExternalSourceAspect::GetAspect(*el, aspectId);
    if (aspect.GetKind() != Kind::Model)
        {
        BeAssert(false && "Not a model aspect");
        return V8ModelExternalSourceAspect();
        }

    return V8ModelExternalSourceAspect(aspect.m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Transform SyncInfo::V8ModelExternalSourceAspect::GetTransform() const
    {
    auto json = GetProperties();
    Transform transform;
    fixedArrayFromJson((double*)&transform, 12, json["transform"].GetArray());
    return transform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::V8ModelExternalSourceAspect::SetTransform(TransformCR t)
    {
    auto json = GetProperties();
    json["transform"] = fixedArrayToJson((double*)&t, 12, json.GetAllocator());
    SetProperties(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SyncInfo::V8ModelExternalSourceAspect::GetV8ModelName() const
    {
    auto json = GetProperties();
    return json["v8ModelName"].GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::ModelId SyncInfo::V8ModelExternalSourceAspect::GetV8ModelId() const
    {
    return atoi(GetIdentifier().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8FileInfo SyncInfo::ComputeFileInfo(DgnV8FileCR file)
    {
    V8FileInfo info;

    if (!file.IsEmbeddedFile())
        {
        info.GetInfo(BeFileName(file.GetFileName().c_str()));
        info.m_lastSaveTime = ((DgnV8FileR) file).GetLastSaveTime();
        }

    WString fullFileName(file.GetFileName().c_str());
    info.m_v8Name = Utf8String(fullFileName);
    info.m_uniqueName = GetUniqueNameForFile(file);
    return info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::RepositoryLinkExternalSourceAspect SyncInfo::RepositoryLinkExternalSourceAspect::CreateAspect(DgnDbR db, V8FileInfo const& fileInfo, StableIdPolicy idPolicy) 
    {
    auto aspectClass = GetAspectClass(db);
    if (nullptr == aspectClass)
        return RepositoryLinkExternalSourceAspect(nullptr);
    
    SourceState ss;
    iModelExternalSourceAspect::UInt64ToString(ss.m_version, fileInfo.m_lastModifiedTime);
    auto instance = CreateInstance(db.Elements().GetRootSubjectId(), Kind::RepositoryLink, fileInfo.m_uniqueName, &ss, *aspectClass);
    
    RepositoryLinkExternalSourceAspect aspect(instance.get());
    
    rapidjson::Document json(rapidjson::kObjectType);
    auto& allocator = json.GetAllocator();
    json.AddMember("lastSaveTime", fileInfo.m_lastSaveTime, allocator);
    json.AddMember("fileSize", rapidjson::Value(iModelExternalSourceAspect::UInt64ToString(fileInfo.m_fileSize).c_str(), allocator), allocator);
    json.AddMember("fileName", rapidjson::Value(fileInfo.m_v8Name.c_str(), allocator), allocator);
    json.AddMember("idPolicy", (int)idPolicy, allocator);
    aspect.SetProperties(json);

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::RepositoryLinkExternalSourceAspect::Update(V8FileInfo const& fileInfo)
    {
    iModelExternalSourceAspect::SourceState ss;
    iModelExternalSourceAspect::UInt64ToString(ss.m_version, fileInfo.m_lastModifiedTime);
    SetSourceState(ss);

    auto json = GetProperties();
    json["fileSize"] = rapidjson::Value(iModelExternalSourceAspect::UInt64ToString(fileInfo.m_fileSize).c_str(), json.GetAllocator());
    json["lastSaveTime"] = fileInfo.m_lastSaveTime;
    SetProperties(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::RepositoryLinkExternalSourceAspect SyncInfo::RepositoryLinkExternalSourceAspect::FindAspectByIdentifier(DgnDbR db, Utf8StringCR uniqueName)
    {
    auto ei = FindElementBySourceId(db, db.Elements().GetRootSubjectId(), Kind::RepositoryLink, uniqueName);
    auto rlink = db.Elements().Get<RepositoryLink>(ei.elementId);
    if (!rlink.IsValid())
        return RepositoryLinkExternalSourceAspect(nullptr);
    auto id = SyncInfo::GetSoleAspectIdByKind(*rlink, Kind::RepositoryLink);
    if (!id.IsValid())
        return nullptr;
    return RepositoryLinkExternalSourceAspect(ExternalSourceAspect::GetAspect(*rlink, id).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::RepositoryLinkExternalSourceAspect SyncInfo::RepositoryLinkExternalSourceAspect::GetAspectForEdit(RepositoryLinkR el)
    {
    auto id = SyncInfo::GetSoleAspectIdByKind(el, Kind::RepositoryLink);
    if (!id.IsValid())
        return nullptr;
    return RepositoryLinkExternalSourceAspect(ExternalSourceAspect::GetAspectForEdit(el, id).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::RepositoryLinkExternalSourceAspect SyncInfo::RepositoryLinkExternalSourceAspect::GetAspect(RepositoryLinkCR el)
    {
    auto id = SyncInfo::GetSoleAspectIdByKind(el, Kind::RepositoryLink);
    if (!id.IsValid())
        return nullptr;
    return RepositoryLinkExternalSourceAspect(ExternalSourceAspect::GetAspect(el, id).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t SyncInfo::RepositoryLinkExternalSourceAspect::GetLastModifiedTime() const
    {
    return iModelExternalSourceAspect::UInt64FromString(GetSourceState().m_version.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
double SyncInfo::RepositoryLinkExternalSourceAspect::GetLastSaveTime() const
    {
    auto json = GetProperties();
    uint64_t v;
    return json["lastSaveTime"].GetDouble();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t SyncInfo::RepositoryLinkExternalSourceAspect::GetFileSize() const
    {
    auto json = GetProperties();
    return iModelExternalSourceAspect::UInt64FromString(json["fileSize"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SyncInfo::RepositoryLinkExternalSourceAspect::GetFileName() const
    {
    auto json = GetProperties();
    return json["fileName"].GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
StableIdPolicy SyncInfo::RepositoryLinkExternalSourceAspect::GetStableIdPolicy() const
    {
    auto json = GetProperties();
    return (StableIdPolicy)(json["idPolicy"].GetInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::GeomPartExternalSourceAspect SyncInfo::GeomPartExternalSourceAspect::CreateAspect(DgnElementId scopeId, Utf8StringCR tag, DgnDbR db)
    {
    auto aspectClass = GetAspectClass(db);
    if (nullptr == aspectClass)
        return GeomPartExternalSourceAspect(nullptr);
                
    auto instance = iModelExternalSourceAspect::CreateInstance(scopeId, Kind::GeomPart, tag, nullptr, *aspectClass);
    return GeomPartExternalSourceAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::GeomPartExternalSourceAspect SyncInfo::GeomPartExternalSourceAspect::GetAspect(DgnGeometryPartCR el)
    {
    auto id = SyncInfo::GetSoleAspectIdByKind(el, Kind::Level);
    if (!id.IsValid())
        return nullptr;
    return GeomPartExternalSourceAspect(ExternalSourceAspect::GetAspect(el, id).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::ViewDefinitionExternalSourceAspect SyncInfo::ViewDefinitionExternalSourceAspect::CreateAspect(DgnElementId scopeId, Utf8StringCR viewName, DgnV8ViewInfoCR viewInfo, DgnDbR db)
    {
    if (nullptr == viewInfo.GetElementRef())
        {
        BeAssert(false);
        return ViewDefinitionExternalSourceAspect(nullptr);
        }

    auto aspectClass = GetAspectClass(db);
    if (nullptr == aspectClass)
        return ViewDefinitionExternalSourceAspect(nullptr);

    auto instance = iModelExternalSourceAspect::CreateInstance(scopeId, Kind::ViewDefinition, FormatSourceId(viewInfo.GetElementId()), nullptr, *aspectClass);
    auto aspect = ViewDefinitionExternalSourceAspect(instance.get());

    aspect.Update(viewInfo, viewName);

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::ViewDefinitionExternalSourceAspect::Update(DgnV8ViewInfoCR viewInfo, Utf8StringCR viewName)
    {
    ElementRefP viewElemRef = viewInfo.GetElementRef();
    if (nullptr == viewElemRef)
        {
        BeAssert(false);
        return;
        }

    iModelExternalSourceAspect::SourceState ss;
    DoubleToString(ss.m_version, viewInfo.GetElementRef()->GetLastModified());
    SetSourceState(ss);

    auto json = GetProperties();
    auto& allocator = json.GetAllocator();
    if (!json.HasMember(json_v8ViewName))
        json.AddMember("v8ViewName", rapidjson::Value(viewName.c_str(), allocator), allocator);
    else
        json[json_v8ViewName] = rapidjson::Value(viewName.c_str(), allocator);

    SetProperties(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SyncInfo::ViewDefinitionExternalSourceAspect::GetV8ViewName() const
    {
    auto json = GetProperties();
    return json[json_v8ViewName].GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::ViewDefinitionExternalSourceAspect SyncInfo::ViewDefinitionExternalSourceAspect::GetAspect(ViewDefinitionCR el)
    {
    auto id = SyncInfo::GetSoleAspectIdByKind(el, Kind::ViewDefinition);
    if (!id.IsValid())
        return nullptr;
    return ViewDefinitionExternalSourceAspect(ExternalSourceAspect::GetAspect(el, id).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::ViewDefinitionExternalSourceAspect SyncInfo::ViewDefinitionExternalSourceAspect::GetAspectForEdit(ViewDefinitionR el)    // non-const version, used for editing
    {
    auto id = SyncInfo::GetSoleAspectIdByKind(el, Kind::ViewDefinition);
    if (!id.IsValid())
        return nullptr;
    return ViewDefinitionExternalSourceAspect(ExternalSourceAspect::GetAspectForEdit(el, id).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
std::tuple<SyncInfo::ViewDefinitionExternalSourceAspect,DgnViewId> SyncInfo::ViewDefinitionExternalSourceAspect::GetAspectBySourceId(DgnElementId scopeId, DgnV8ViewInfoCR viewInfo, DgnDbR db)
    {
    DgnElementId elId = FindElementBySourceId(db, scopeId, Kind::ViewDefinition, FormatSourceId(viewInfo.GetElementId())).elementId;
    if (!elId.IsValid())
        return std::make_tuple(ViewDefinitionExternalSourceAspect(nullptr), DgnViewId());

    auto el = db.Elements().Get<ViewDefinition>(elId);
    if (!el.IsValid())
        {
        BeAssert(false);
        return std::make_tuple(ViewDefinitionExternalSourceAspect(nullptr), DgnViewId());
        }
    return std::make_tuple(GetAspect(*el), DgnViewId(elId.GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::CachedECSqlStatementPtr SyncInfo::ViewDefinitionExternalSourceAspect::GetIteratorForScope(DgnDbR db, DgnElementId scope)
    {
    auto stmt = db.GetPreparedECSqlStatement("SELECT Element.Id, ECInstanceId from " XTRN_SRC_ASPCT_FULLCLASSNAME " WHERE (Scope.Id=? AND Kind=?)");
    stmt->BindId(1, scope);
    stmt->BindText(2, Kind::ViewDefinition, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncInfo::OnAttach(DgnDb& project)
    {
    m_dgndb = &project;

    if (!m_dgndb->TableExists(SYNCINFO_ATTACH(SYNC_TABLE_ECSchema)))
        {
        // We are creating a new syncinfo file
        Utf8String currentDbProfileVersion;
        m_dgndb->QueryProperty(currentDbProfileVersion, Properties::ProfileVersion());

        MUSTBEOK(SavePropertyString(SyncInfoProperty::ProfileVersion(), s_currentVersion.ToJson().c_str()));
        MUSTBEOK(SavePropertyString(Properties::CreationDate(), DateTime::GetCurrentTimeUtc().ToString().c_str()));
        MUSTBEOK(SavePropertyString(SyncInfoProperty::DgnDbGuid(), m_dgndb->GetDbGuid().ToString().c_str()));
        MUSTBEOK(SavePropertyString(SyncInfoProperty::DbProfileVersion(), currentDbProfileVersion.c_str()));
        MUSTBEOK(SavePropertyString(SyncInfoProperty::DgnDbProfileVersion(), m_dgndb->GetProfileVersion().ToJson().c_str()));
        // *** WIP_CONVERTER - I'd like to save project's last save time

        CreateTables();
        SetValid(true);
        return BSISUCCESS;
        }

    //  We are opening an existing syncinfo file
    if (PerformVersionChecks() != BSISUCCESS)
        return BSIERROR;

    //  Check that this syncinfo goes with this project
    Utf8String projguidstr;
    BeSQLite::BeGuid projguid;
    if (QueryProperty(projguidstr, SyncInfoProperty::DgnDbGuid()) != BE_SQLITE_ROW
        || projguid.FromString(projguidstr.c_str()) != BSISUCCESS
        || m_dgndb->GetDbGuid() != projguid)
        {
        LOG.errorv("GUID mismatch. syncinfo=%s projectguid=%s does not match project guid=%s",
                   m_dgndb->GetDbFileName(), projguidstr.c_str(), m_dgndb->GetDbGuid().ToString().c_str());
        return BSIERROR;
        }

    Utf8String savedProjectDbProfileVersion, currentProjectDbProfileVersion;
    if (QueryProperty(savedProjectDbProfileVersion, SyncInfoProperty::DbProfileVersion()) != BE_SQLITE_ROW
        || m_dgndb->QueryProperty(currentProjectDbProfileVersion, Properties::ProfileVersion()) != BE_SQLITE_ROW
        || !savedProjectDbProfileVersion.Equals(currentProjectDbProfileVersion))
        {
        LOG.warningv("DB schema version mismatch. syncinfo=%s ProjectDbProfileVersion=%s does not match project ProfileVersion=%s.",
                     m_dgndb->GetDbFileName(), savedProjectDbProfileVersion.c_str(), currentProjectDbProfileVersion.c_str());
        // *** WIP_CONVERTER - Do we really have to throw away project history whenever we make a trivial schema change?
        return BSISUCCESS;//BSIERROR; *** WIP_CONVERTER - support schema evolution 
        }

    Utf8String currentProjectProfileVersion = m_dgndb->GetProfileVersion().ToJson();
    Utf8String savedProjectProfileVersion;
    if (QueryProperty(savedProjectProfileVersion, SyncInfoProperty::DgnDbProfileVersion()) != BE_SQLITE_ROW
        || !savedProjectProfileVersion.Equals(currentProjectProfileVersion))
        {
        LOG.warningv("project schema version mismatch. syncinfo=%s ProjectProfileVersion=%s does not match project ProjectProfileVersion=%s.",
                     m_dgndb->GetDbFileName(), savedProjectProfileVersion.c_str(), currentProjectProfileVersion.c_str());
        // *** WIP_CONVERTER - Do we really have to throw away project history whenever we make a trivial schema change?
        return BSISUCCESS;//BSIERROR; *** WIP_CONVERTER - support schema evolution 
        }

    CreateTables();  // We STILL call CreateTables. That gives EC a chance to create its TEMP tables.

    SetValid(true);
    return BSISUCCESS;
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
    m_lastErrorDescription = m_dgndb->GetLastError();
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
// @bsimethod                                                   Krischan.Eberle   11/2014
//---------------------------------------------------------------------------------------
DbResult SyncInfo::InsertECSchema(BentleyApi::ECN::ECSchemaId& insertedSchemaId, DgnV8FileR v8File,
                                  Utf8CP v8SchemaName, uint32_t v8ProfileVersionMajor, uint32_t v8ProfileVersionMinor,
                                  bool isDynamic, uint32_t checksum) const
    {
    insertedSchemaId;

    BeAssert(checksum != 0);

    RepositoryLinkId v8FileId = m_converter.GetRepositoryLinkId(v8File);
    if (!v8FileId.IsValid() || Utf8String::IsNullOrEmpty(v8SchemaName))
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_dgndb->GetCachedStatement(stmt, "INSERT OR REPLACE INTO " SYNCINFO_ATTACH(SYNC_TABLE_ECSchema)
                                                    " (RepositoryLinkId,V8Name,V8VersionMajor,V8VersionMinor,MappingType,LastModified,Digest) VALUES (?,?,?,?,?,?,?)"))
        {
        BeAssert(false && "Could not retrieve cached SyncInfo statement.");
        return BE_SQLITE_ERROR;
        }

    stmt->BindId(1, v8FileId);
    stmt->BindText(2, v8SchemaName, Statement::MakeCopy::No);
    stmt->BindInt(3, v8ProfileVersionMajor);
    stmt->BindInt(4, v8ProfileVersionMinor);
    stmt->BindInt(5, (int) (isDynamic ? ECSchemaMappingType::Dynamic : ECSchemaMappingType::Identity));

    double nowJd = -1.0;
    if (DateTime::GetCurrentTimeUtc().ToJulianDay(nowJd) != SUCCESS)
        {
        BeAssert(false && "Failed to convert current date time to Julian Day.");
        return BE_SQLITE_ERROR;
        }

    stmt->BindDouble(6, nowJd);
    stmt->BindInt(7, checksum);

    if (BE_SQLITE_DONE != stmt->Step())
        return BE_SQLITE_ERROR;

    insertedSchemaId = BECN::ECSchemaId((uint64_t) m_dgndb->GetLastInsertRowId());
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
bool SyncInfo::TryGetECSchema(ECObjectsV8::SchemaKey& schemaKey, ECSchemaMappingType& mappingType, Utf8CP v8SchemaName, RepositoryLinkId fileId) const
    {
    //first check whether we need to capture this schema or not
    CachedStatementPtr stmt = nullptr;
    Utf8String sql("SELECT V8VersionMajor, V8VersionMinor, Digest, MappingType FROM "
                  SYNCINFO_ATTACH(SYNC_TABLE_ECSchema)
                  " WHERE V8Name=?");
    if (fileId.IsValid())
        sql.append("and RepositoryLinkId = ? ");

    if (BE_SQLITE_OK != m_dgndb->GetCachedStatement(stmt, sql.c_str()))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindText(1, v8SchemaName, Statement::MakeCopy::No);
    if (fileId.IsValid())
        stmt->BindId(2, fileId);
    if (BE_SQLITE_ROW != stmt->Step())
        return false;

    schemaKey.m_schemaName = WString(v8SchemaName).c_str();
    schemaKey.m_versionMajor = (uint32_t) stmt->GetValueInt(0);
    schemaKey.m_versionMinor = (uint32_t) stmt->GetValueInt(1);
    schemaKey.m_checkSum = (uint32_t) stmt->GetValueInt(2);
    mappingType = (ECSchemaMappingType) stmt->GetValueInt(3);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2014
//---------------------------------------------------------------------------------------
bool SyncInfo::ContainsECSchema(Utf8CP v8SchemaName) const
    {
    CachedStatementPtr stmt = nullptr;
    auto stat = m_dgndb->GetCachedStatement(stmt, "SELECT NULL FROM " SYNCINFO_ATTACH(SYNC_TABLE_ECSchema) " WHERE V8Name=?");
    if (BE_SQLITE_OK != stat)
        {
        BeAssert(false && "Could not retrieve cached SyncInfo statement.");
        return BE_SQLITE_ERROR;
        }

    stmt->BindText(1, v8SchemaName, Statement::MakeCopy::No);
    return stmt->Step() == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
DbResult SyncInfo::RetrieveECSchemaChecksums(bmap<Utf8String, uint32_t>& syncInfoChecksums, RepositoryLinkId fileId) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_dgndb->GetCachedStatement(stmt, "SELECT V8Name, Digest FROM " SYNCINFO_ATTACH(SYNC_TABLE_ECSchema) " WHERE RepositoryLinkId=?"))
        {
        BeAssert(false && "Could not retrieve cached SyncInfo statement.");
        return BE_SQLITE_ERROR;
        }

    stmt->BindId(1, fileId);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        syncInfoChecksums[stmt->GetValueText(0)] = (uint32_t) stmt->GetValueInt(1);
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::CreateNamedGroupTable(bool createIndex)
    {
    m_dgndb->CreateTable(SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups), "SourceId INTEGER NOT NULL, TargetId INTEGER NOT NULL");
    if (createIndex)
        MUSTBEOK(m_dgndb->ExecuteSql("CREATE UNIQUE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) "_ng_uix ON " SYNC_TABLE_NamedGroups "(SourceId, TargetId);"));

    return BentleyApi::SUCCESS;
    }

#define SYNC_TABLE_master SYNCINFO_ATTACH("sqlite_master")
#define TEMPTABLE_ATTACH(name) "temp." name

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::CheckNamedGroupTable()
    {
    Statement stmt;
    Utf8PrintfString query("SELECT name, tbl_name FROM %s WHERE type='table' AND name='%s'", SYNC_TABLE_master, SYNC_TABLE_NamedGroups);

    if (BE_SQLITE_OK != stmt.Prepare(*m_dgndb, query.c_str()))
        return BentleyApi::ERROR;

    if (BE_SQLITE_ROW == stmt.Step())
        {
        // create the temp table for storing new entries
        MUSTBEOK(m_dgndb->ExecuteSql("CREATE TABLE " TEMPTABLE_ATTACH(SYNC_TABLE_NamedGroups) " (SourceId INTEGER NOT NULL, TargetId INTEGER NOT NULL);"));
        return SUCCESS;
        }

    // If we didn't find an existing NamedGroups table, that means this is an update using an older syncinfo database.  We need to create a new NamedGroups table and populate it
    // with existing named group members.
    if (BentleyApi::SUCCESS != CreateNamedGroupTable(false))
        return BentleyApi::ERROR;

    Utf8CP sql = "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) "(SourceId, TargetId) SELECT SourceId, TargetId from bis_ElementRefersToElements b, ec_Class e, ec_Schema s where b.ECClassId = e.Id and e.Name='ElementGroupsMembers' and e.SchemaId = s.Id and s.Name='BisCore'";
    Statement groups;
    if (BE_SQLITE_OK != groups.Prepare(*m_dgndb, sql))
        return BentleyApi::ERROR;

    if (BE_SQLITE_DONE != groups.Step())
        {
        return ERROR;
        }
    MUSTBEOK(m_dgndb->ExecuteSql("CREATE UNIQUE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) "_ng_uix ON " SYNC_TABLE_NamedGroups "(SourceId, TargetId);"));

    // create the temp table for storing new entries
    MUSTBEOK(m_dgndb->ExecuteSql("CREATE TABLE " TEMPTABLE_ATTACH(SYNC_TABLE_NamedGroups) " (SourceId INTEGER NOT NULL, TargetId INTEGER NOT NULL);"));

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool SyncInfo::IsElementInNamedGroup(DgnElementId sourceId, DgnElementId targetId)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "SELECT 1 FROM " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) " WHERE SourceId=? AND TargetId=?");
    if (!stmt.IsValid())
        return BentleyApi::ERROR;

    stmt->BindId(1, sourceId);
    stmt->BindId(2, targetId);

    return (BE_SQLITE_ROW == stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::AddNamedGroupEntry(DgnElementId sourceId, DgnElementId targetId)
    {
    CachedStatementPtr stmt;
    m_dgndb->GetCachedStatement(stmt, "INSERT INTO " TEMPTABLE_ATTACH(SYNC_TABLE_NamedGroups) " (SourceId, TargetId) VALUES(?, ?)");
    if (!stmt.IsValid())
        return BentleyApi::ERROR;

    stmt->BindId(1, sourceId);
    stmt->BindId(2, targetId);
    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;
    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::FinalizeNamedGroups()
    {
    MUSTBEOK(m_dgndb->ExecuteSql("DROP INDEX " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) "_ng_uix"));

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(*m_dgndb, "INSERT INTO " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) " (SourceId, TargetId) SELECT SourceId, TargetId FROM " TEMPTABLE_ATTACH(SYNC_TABLE_NamedGroups)))
        return BentleyApi::ERROR;

    if (BE_SQLITE_DONE != stmt.Step())
        return ERROR;

    MUSTBEOK(m_dgndb->ExecuteSql("CREATE UNIQUE INDEX " SYNCINFO_ATTACH(SYNC_TABLE_NamedGroups) "_ng_uix ON " SYNC_TABLE_NamedGroups "(SourceId, TargetId);"));
    return BentleyApi::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::UriExternalSourceAspect SyncInfo::UriExternalSourceAspect::CreateAspect(DgnElementId repositoryLinkId, Utf8CP filename, UriContentInfo const& info, Utf8CP rdsId, Converter& converter)
    {
    auto aspectClass = GetAspectClass(converter.GetDgnDb());

    SourceState ss;
    iModelExternalSourceAspect::UInt64ToString(ss.m_version, info.m_lastModifiedTime);
    auto instance = CreateInstance(repositoryLinkId, Kind::URI, filename, &ss, *aspectClass);
    
    UriExternalSourceAspect aspect(instance.get());
    
    rapidjson::Document json(rapidjson::kObjectType);
    auto& allocator = json.GetAllocator();
    json.AddMember("fileSize", rapidjson::Value(iModelExternalSourceAspect::UInt64ToString(info.m_fileSize).c_str(), allocator), allocator);
    json.AddMember("eTag", rapidjson::Value(info.m_eTag.c_str(), allocator), allocator);
    json.AddMember("rdsId", rapidjson::Value(rdsId? rdsId: "", allocator), allocator);
    aspect.SetProperties(json);

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::UriExternalSourceAspect SyncInfo::UriExternalSourceAspect::GetAspectForEdit(DgnElementR el)
    {
    auto id = SyncInfo::GetSoleAspectIdByKind(el, Kind::URI);
    if (!id.IsValid())
        return nullptr;
    return UriExternalSourceAspect(ExternalSourceAspect::GetAspectForEdit(el, id).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/18
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::UriExternalSourceAspect SyncInfo::UriExternalSourceAspect::GetAspect(DgnElementCR el)
    {
    auto id = SyncInfo::GetSoleAspectIdByKind(el, Kind::URI);
    if (!id.IsValid())
        return nullptr;
    return UriExternalSourceAspect(ExternalSourceAspect::GetAspect(el, id).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::UriExternalSourceAspect::GetInfo(UriContentInfo& info) const
    {
    auto ss = GetSourceState();
    info.m_lastModifiedTime = iModelExternalSourceAspect::UInt64FromString(ss.m_version.c_str());

    auto props = GetProperties();
    info.m_fileSize = iModelExternalSourceAspect::UInt64FromString(props["fileSize"].GetString());
    info.m_eTag = props["eTag"].GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::UriExternalSourceAspect::SetInfo(UriContentInfo const& info)
    {
    auto ss = GetSourceState();
    iModelExternalSourceAspect::UInt64ToString(ss.m_version, info.m_lastModifiedTime);
    SetSourceState(ss);

    auto props = GetProperties();
    props["fileSize"] = rapidjson::Value(iModelExternalSourceAspect::UInt64ToString(info.m_fileSize).c_str(), props.GetAllocator());
    props["eTag"] = rapidjson::Value(info.m_eTag.c_str(), props.GetAllocator());
    SetProperties(props);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SyncInfo::UriExternalSourceAspect::GetSourceGuid() const
    {
    auto props = GetProperties();
    return props["rdsId"].GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::UriExternalSourceAspect::SetSourceGuid(Utf8StringCR rdsid)
    {
    auto props = GetProperties();
    props["rdsId"] = rapidjson::Value(rdsid.c_str(), props.GetAllocator());
    SetProperties(props);
    }

static bool isHttp(Utf8CP str) { return (0 == strncmp("http:", str, 5) || 0 == strncmp("https:", str, 6)); }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      1/19
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SyncInfo::UriContentInfo::GetInfo(Utf8StringCR uri)
    {
    m_eTag.clear();
    
    if (!isHttp(uri.c_str()))
        return DiskFileInfo::GetInfo(BeFileName(uri.c_str(), true));

    BentleyApi::Http::Request request(uri, "HEAD");
    folly::Future<BentleyApi::Http::Response> response = request.Perform().wait();
    if (BentleyApi::Http::HttpStatus::OK != response.value().GetHttpStatus())
        return BSIERROR;

    auto headers = response.value().GetHeaders();
    m_eTag = headers.GetETag();
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      1/19
//---------------+---------------+---------------+---------------+---------------+-------
bool SyncInfo::FileHasChangedUriContent(RepositoryLinkId repositoryLinkId)
    {
    // TODO: Select all ImageXSAs where scope.id = repositoryLinkId
    if (!repositoryLinkId.IsValid())
        return false;

    auto stmt = GetDgnDb()->GetPreparedECSqlStatement("SELECT Element.Id FROM " XTRN_SRC_ASPCT_FULLCLASSNAME " WHERE Scope.Id=? AND Kind=?");
    stmt->BindId(1, repositoryLinkId);
    stmt->BindText(2, UriExternalSourceAspect::Kind::URI, BeSQLite::EC::IECSqlBinder::MakeCopy::No);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (HasUriContentChanged(stmt->GetValueId<DgnElementId>(0)))
            return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      1/19
//---------------+---------------+---------------+---------------+---------------+-------
bool SyncInfo::HasUriContentChanged(DgnElementId eid)
    {
    auto aspect = UriExternalSourceAspect::GetAspect(eid, *GetDgnDb());
    if (!aspect.IsValid())
        return false;

    printf("%s\n", aspect.FormatForDump(*GetDgnDb(), true, true).c_str());

    UriContentInfo storedInfo;
    aspect.GetInfo(storedInfo);

    UriContentInfo currentInfo;
    if (BSISUCCESS != currentInfo.GetInfo(aspect.GetFilenameOrUrl()))
        return false;

    return !currentInfo.IsEqual(storedInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getUrnFromFirstSource(Utf8String& urn, BeXmlNodeP file)
    {
    for (BeXmlNodeP sources = file->GetFirstChild(); sources != nullptr; sources = sources->GetNextSibling())
        {
        if (0 != strcmp(sources->GetName(), "Sources"))
            continue;

        for (BeXmlNodeP source = sources->GetFirstChild(); source != nullptr; source = source->GetNextSibling())
            {
            if (0 != strcmp(source->GetName(), "Source"))
                continue;

            Utf8String refId;
            if (BeXmlStatus::BEXML_Success == source->GetAttributeStringValue(refId, "RefId"))
                continue;

            Utf8String content;
            source->GetContent(content);
            if (iModelBridge::IsPwUrn(content))
                {
                urn = content;
                return BSISUCCESS;       // <<== we only want the first source
                }
            }
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getUrnFromLastTarget(Utf8String& urn, BeXmlNodeP file)
    {
    bool foundAny = false;
    for (BeXmlNodeP target = file->GetFirstChild(); target != nullptr; target = target->GetNextSibling())
        {
        if (0 == strcmp(target->GetName(), "Target"))
            {
            Utf8String content;
            target->GetContent(content);
            if (iModelBridge::IsPwUrn(content))
                {
                urn = content;
                // keep looking. we want the last target
                }
            }
        }
    return foundAny? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isFileExtraction(BeXmlNodeP file)
    {
    for (BeXmlNodeP type = file->GetFirstChild(); type != nullptr; type = type->GetNextSibling())
        {
        if (0 == strcmp(type->GetName(), "Type"))
            {
            Utf8String content;
            type->GetContent(content);
            return content.EqualsI("Extract");
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getPwUrn(Bentley::DgnPlatform::ProvenanceBlobR blob)
    {
    Utf8String urn;

    WCharCP provData = (WCharCP)blob.GetData();

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, provData);
    if (xmlDom == nullptr)
        return urn;

    // First, get doc urns from *sources*
    BeXmlNodeP root = xmlDom->GetRootElement();
    for (BeXmlNodeP files = root->GetFirstChild(); files != nullptr; files = files->GetNextSibling())
        {
        if (0 != strcmp(files->GetName(), "Files"))
            continue;
        for (BeXmlNodeP file = files->GetFirstChild(); file != nullptr; file = file->GetNextSibling())
            {
            if (0 != strcmp(file->GetName(), "File"))
                continue;

            getUrnFromFirstSource(urn, file);

            if (!isFileExtraction(file))
                getUrnFromLastTarget(urn, file);
            }
        }

    return urn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::GetPwUrnFromFileProvenance(DgnV8FileCR file)
    {
    auto provData = const_cast<DgnV8FileR>(file).ReadFileProvenance();
    if (!provData.IsValid())
        return "";
    return getPwUrn(*provData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::GetDocumentURNforFile(DgnV8FileCR file)
    {
    // We prefer a PW URN. 

    auto const& moniker = file.GetDocument().GetMoniker();  
    Utf8String monikerURN(moniker.ResolveURI().c_str());
    if (iModelBridge::IsPwUrn(monikerURN))
        return monikerURN;

    Utf8String docURN(GetParams().QueryDocumentURN(BeFileName(file.GetFileName().c_str())));
    if (iModelBridge::IsPwUrn(docURN))
        return docURN;

    auto provURN = Converter::GetPwUrnFromFileProvenance(file);
    if (iModelBridge::IsPwUrn(provURN))
        return provURN;

    // We fall back on the first URN that is available.
    return !monikerURN.empty()? monikerURN:
           !docURN.empty()?     docURN:
                                provURN;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid Converter::GetDocumentGUIDforFile(DgnV8FileCR file)
    {
    auto const& moniker = file.GetDocument().GetMoniker();  
    Utf8String monikerURN(moniker.ResolveURI().c_str());
    if (iModelBridge::IsPwUrn(monikerURN))
        return iModelBridge::ParseDocGuidFromPwUri(monikerURN);

    iModelBridgeDocumentProperties docProps;
    GetDocumentProperties(docProps, BeFileName(file.GetFileName().c_str()));
    if (!docProps.m_docGuid.empty())
        {
        BeGuid guid;
        if (BSISUCCESS == guid.FromString(docProps.m_docGuid.c_str()))
            return guid;
        }

    auto provURN = Converter::GetPwUrnFromFileProvenance(file);
    if (iModelBridge::IsPwUrn(provURN))
        return iModelBridge::ParseDocGuidFromPwUri(provURN);

    return BeGuid();
    }

END_DGNDBSYNC_DGNV8_NAMESPACE

/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgSyncInfo.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DwgImportInternal.h"

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"DwgImporter.SyncInfo"))

BEGIN_DWG_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/19
+===============+===============+===============+===============+===============+======*/
struct FileProperty
    {
    static constexpr char UniqueName[] = "UniqueName";
    static constexpr char DwgName[] = "DwgName";
    static constexpr char VersionGuid[] = "VersionGuid";
    static constexpr char LastSaveTime[] = "LastSaveTime";
    static constexpr char FileSize[] = "FileSize";
    static constexpr char IdPolicy[] = "idPolicy";
    static constexpr char RootRepositoryLink[] = "RootRepositoryLink";
    };  // FileProperty

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/19
+===============+===============+===============+===============+===============+======*/
struct UriProperty
    {
    static constexpr char FileSize[] = "FileSize";
    static constexpr char ETag[] = "eTag";
    static constexpr char RdsId[] = "rdsId";
    };  // UriProperty

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/19
+===============+===============+===============+===============+===============+======*/
struct ModelProperty
    {
    struct SourceType
        {
        static constexpr Utf8CP ModelSpace = "ModelSpace";
        static constexpr Utf8CP PaperSpace = "PaperSpace";
        static constexpr Utf8CP XRefAttachment = "XrefAttachment";
        static constexpr Utf8CP RasterAttachment = "RasterAttachment";
        // get the json string value from enum class value
        static Utf8CP FromEnum(DwgSourceAspects::ModelAspect::SourceType enumValue)
            {
            switch (enumValue)
                {
                case DwgSourceAspects::ModelAspect::SourceType::ModelSpace: return SourceType::ModelSpace;
                case DwgSourceAspects::ModelAspect::SourceType::PaperSpace: return SourceType::PaperSpace;
                case DwgSourceAspects::ModelAspect::SourceType::XRefAttachment: return SourceType::XRefAttachment;
                case DwgSourceAspects::ModelAspect::SourceType::RasterAttachment: return SourceType::RasterAttachment;
                }
            return  "";
            }
        // get the enum class value from json string
        static DwgSourceAspects::ModelAspect::SourceType ToEnum(Utf8CP stringValue)
            {
            if (::_stricmp(stringValue, SourceType::ModelSpace) == 0)
                return DwgSourceAspects::ModelAspect::SourceType::ModelSpace;
            else if (::_stricmp(stringValue, SourceType::PaperSpace) == 0)
                return DwgSourceAspects::ModelAspect::SourceType::PaperSpace;
            else if (::_stricmp(stringValue, SourceType::XRefAttachment) == 0)
                return DwgSourceAspects::ModelAspect::SourceType::XRefAttachment;
            else if (::_stricmp(stringValue, SourceType::RasterAttachment) == 0)
                return DwgSourceAspects::ModelAspect::SourceType::RasterAttachment;
            return  DwgSourceAspects::ModelAspect::SourceType::Unknown;
            }
        };  // SourceType
    static constexpr char Transform[] = "Transform";
    static constexpr char DwgName[] = "DwgName";
    static constexpr char DwgSourceType[] = "SourceType";
    };  // ModelProperty

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/19
+===============+===============+===============+===============+===============+======*/
struct LayerProperty
    {
    static constexpr char LayerType[] = "LayerType";
    static constexpr char LayerName[] = "LayerName";
    }; // LayerProperty

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/19
+===============+===============+===============+===============+===============+======*/
struct LinetypeProperty
    {
    static constexpr char LinetypeName[] = "LinetypeName";
    }; // LinetypeProperty

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/19
+===============+===============+===============+===============+===============+======*/
struct MaterialProperty
    {
    static constexpr char MaterialName[] = "MaterialName";
    }; // MaterialProperty

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/19
+===============+===============+===============+===============+===============+======*/
struct GroupProperty
    {
    static constexpr char GroupName[] = "GroupName";
    }; // GroupProperty

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/19
+===============+===============+===============+===============+===============+======*/
struct ViewProperty
    {
    struct SourceType
        {
        static constexpr Utf8CP ModelSpaceViewport = "ModelSpaceViewport";
        static constexpr Utf8CP PaperSpaceViewport = "PaperSpaceViewport";
        static constexpr Utf8CP XRefAttachment = "XrefAttachment";
        static constexpr Utf8CP ViewportEntity = "ViewportEntity";
        // get json string value from enum class value
        static Utf8CP FromEnum(DwgSourceAspects::ViewAspect::SourceType enumValue)
            {
            switch(enumValue)
                {
                case DwgSourceAspects::ViewAspect::SourceType::ModelSpaceViewport: return SourceType::ModelSpaceViewport;
                case DwgSourceAspects::ViewAspect::SourceType::PaperSpaceViewport: return SourceType::PaperSpaceViewport;
                case DwgSourceAspects::ViewAspect::SourceType::XRefAttachment: return SourceType::XRefAttachment;
                case DwgSourceAspects::ViewAspect::SourceType::ViewportEntity: return SourceType::ViewportEntity;
                }
            return "";
            }
        // get enum class value from json string value
        static DwgSourceAspects::ViewAspect::SourceType ToEnum(Utf8CP stringValue)
            {
            if (::_stricmp(stringValue, SourceType::ModelSpaceViewport) == 0)
                return DwgSourceAspects::ViewAspect::SourceType::ModelSpaceViewport;
            else if (::_stricmp(stringValue, SourceType::PaperSpaceViewport) == 0)
                return DwgSourceAspects::ViewAspect::SourceType::PaperSpaceViewport;
            else if (::_stricmp(stringValue, SourceType::XRefAttachment) == 0)
                return DwgSourceAspects::ViewAspect::SourceType::XRefAttachment;
            else if (::_stricmp(stringValue, SourceType::ViewportEntity) == 0)
                return DwgSourceAspects::ViewAspect::SourceType::ViewportEntity;
            return DwgSourceAspects::ViewAspect::SourceType::ModelSpaceViewport;
            }
        };  // SourceType
    static constexpr char DwgSourceType[] = "SourceType";
    static constexpr char DwgName[] = "DwgName";
    }; // LayerProperty


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/19
+===============+===============+===============+===============+===============+======*/
struct HashFiler : IDxfFiler
{
private:
    BentleyApi::MD5&        m_hasher;
    DwgDbObjectCR           m_object;
    DwgDbDatabasePtr        m_dwg;
    bset<DwgDbObjectId>     m_collectedIds;

public:
explicit HashFiler (BentleyApi::MD5& hasher, DwgDbObjectCR obj) : m_hasher(hasher), m_object(obj), m_dwg(obj.GetDatabase()) {}

bool    IsValid () { return m_dwg.IsValid(); }
BentleyApi::MD5::HashVal  GetHashValue () { return m_hasher.GetHashVal(); }
Utf8String  GetHashString () { return m_hasher.GetHashString(); }
bset<DwgDbObjectId> GetCollectedIds () { return m_collectedIds; }
void ResetCollectedIds () { m_collectedIds.clear(); }

DwgFilerType    _GetFilerType () const override { return DwgFilerType::BagFiler; }
DwgDbDatabaseP  _GetDatabase () const override { return const_cast<DwgDbDatabaseP>(m_dwg.get()); }
DwgDbStatus _Write (DxfGroupCode code, int8_t v) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, int16_t v) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, int32_t v) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, int64_t v) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, uint8_t v) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, uint16_t v) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, uint32_t v) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, uint64_t v) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, bool v) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, double v, DoublePrecision prec) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, DPoint2dCR v, DoublePrecision prec) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, DPoint3dCR v, DoublePrecision prec) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, DVec2dCR v, DoublePrecision prec) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, DVec3dCR v, DoublePrecision prec) override { return Add(&v, sizeof(v)); }
DwgDbStatus _Write (DxfGroupCode code, DwgStringCR v) override { return Add(v.AsBufferPtr(), v.GetBufferSize()); }
DwgDbStatus _Write (DxfGroupCode code, DwgBinaryDataCR v) override { return Add(v.GetBuffer(), v.GetSize()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus _Write (DxfGroupCode code, DwgDbHandleCR v) override
    {
    uint64_t    handle = v.AsUInt64 ();
    m_hasher.Add (&handle, sizeof(handle));
    return DwgDbStatus::Success;
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus _Write (DxfGroupCode code, DwgDbObjectIdCR v) override
    {
    uint64_t    handle = v.ToUInt64 ();
    m_hasher.Add (&handle, sizeof(handle));
    m_collectedIds.insert (v);
    return DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus _Write (DxfGroupCode code, double x, double y, double z, DoublePrecision prec) override
    {
    m_hasher.Add (&x, sizeof(x));
    m_hasher.Add (&y, sizeof(y));
    m_hasher.Add (&z, sizeof(z));
    return DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus Add (void const* buf, size_t nBytes)
    {
    m_hasher.Add (buf, nBytes);
    return DwgDbStatus::Success;    
    }
};  // HashFiler


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/19
+===============+===============+===============+===============+===============+======*/
struct HashFactory
{
    struct SyncOptions
        {
        bool    m_syncBlockChanges;
        bool    m_syncAsmBodyInFull;
        bool    m_syncDependentObjects;
        SyncOptions () : m_syncBlockChanges(false), m_syncAsmBodyInFull(true), m_syncDependentObjects(false) {}
        }; // SyncOptions

private:
    BentleyApi::MD5& m_hasher;
    DwgDbObjectCR   m_object;
    SyncOptions     m_syncOptions;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void AppendComplexEntityHash (HashFiler& filer)
    {
    DwgDbEntityCP   entity = DwgDbEntity::Cast(&m_object);
    if (nullptr == entity)
        return;

    // handle complex objects which require additional primary hash:
    if (entity->GetDwgClassName().StartsWithI(L"AecDb"))
        {
        // Aec objects do not dxfOut actual data - add range for now - TFS 853852:
        DRange3d    range;
        if (entity->GetRange(range) == DwgDbStatus::Success)
            m_hasher.Add (&range, sizeof(range));
        return;
        }

    if (m_object.IsAProxy())
        {
        // a proxy entity does not file out DXF group code 310 - need more data, TFS 933725.
        DwgDbObjectPArray   proxy;
        if (DwgDbStatus::Success == entity->Explode(proxy))
            {
            for (auto child : proxy)
                {
                try
                    {
                    child->DxfOut (filer);
                    }
                catch (...)
                    {
                    // OpenDWG may throw exception on writing TTF file
                    }
                // operator delete is hidden by Teigha!
                ::free (child);
                }
            }
        return;
        }

    // 2D/3D polyline and polyface/polygon mesh entities have vertex entities to follow.
    DwgDb2dPolylineCP   pline2d = nullptr;
    DwgDb3dPolylineCP   pline3d = nullptr;
    DwgDbPolyFaceMeshCP pfmesh = nullptr;
    DwgDbPolygonMeshCP  mesh = nullptr;
    DwgDbObjectIteratorPtr  vertexIter;
    if ((pline2d = DwgDb2dPolyline::Cast(entity)) != nullptr)
        vertexIter = pline2d->GetVertexIterator ();
    else if ((pline3d = DwgDb3dPolyline::Cast(entity)) != nullptr)
        vertexIter = pline3d->GetVertexIterator ();
    else if ((pfmesh = DwgDbPolyFaceMesh::Cast(entity)) != nullptr)
        vertexIter = pfmesh->GetVertexIterator ();
    else if ((mesh = DwgDbPolygonMesh::Cast(entity)) != nullptr)
        vertexIter = mesh->GetVertexIterator ();

    if (vertexIter.IsValid() && vertexIter->IsValid())
        {
        for (vertexIter->Start(); !vertexIter->Done(); vertexIter->Next())
            {
            DwgDbEntityPtr vertex(vertexIter->GetObjectId(), DwgDbOpenMode::ForRead);
            if (vertex.OpenStatus() == DwgDbStatus::Success)
                vertex->DxfOut (filer);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AppendBlockHash (DwgDbObjectIdCR blockId)
    {
    DwgDbBlockTableRecordPtr    block(blockId, DwgDbOpenMode::ForRead);
    if (block.IsNull() || block->IsExternalReference() || block->IsLayout())
        return  BSIERROR;

    // this is a performance dragger but unfortunately we cannot use existing hashed blocks as they won't match!
    DwgDbBlockChildIteratorPtr  iter = block->GetBlockChildIterator ();
    if (!iter.IsValid() || !iter->IsValid())
        return  BSIERROR;

    for (iter->Start(); !iter->Done(); iter->Step())
        {
        DwgDbEntityPtr  entity(iter->GetEntityId(), DwgDbOpenMode::ForRead);
        if (entity.IsNull())
            continue;

        DwgDbObjectP    child = DwgDbObject::Cast(entity.get());
        if (nullptr == child)
            continue;

        // optionally single out ASM objects for the sake of performance
        if (!m_syncOptions.m_syncAsmBodyInFull && BSISUCCESS == this->CreateAsmObjectHash(*child))
            continue;

        // hash current child entity itself and add to output hasher:
        HashFiler filer(m_hasher, *child);
        if (filer.IsValid())
            {
            try
                {
                child->DxfOut (filer);
                }
            catch (...)
                {
                // OpenDWG may throw exception on writing TTF file
                }
            }

        // if this is an INSERT entity, recurse into the block and hash all nested blocks as well:
        DwgDbBlockReferenceP    insert = DwgDbBlockReference::Cast (child);
        if (nullptr != insert)
            this->AppendBlockHash (insert->GetBlockTableRecordId());
        }

    return BSISUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CreateAsmObjectHash (DwgDbObjectCR object)
    {
    /*-----------------------------------------------------------------------------------
    DxfOut from an ASM entity outputs the whole Brep data and is unnecessarily expensive.
    This method selects & hash essential Brep properties for the sake of performance.
    -----------------------------------------------------------------------------------*/
    auto entity = DwgDbEntity::Cast (&object);
    if (nullptr == entity)
        return  BSIERROR;

    auto solid3d = DwgDb3dSolid::Cast (entity);
    auto body = DwgDbBody::Cast (entity);
    auto region = DwgDbRegion::Cast (entity);
    if (nullptr == solid3d && nullptr == body && nullptr == region)
        return  BSIERROR;

    DRange3d    range;
    if (entity->GetRange(range) != DwgDbStatus::Success)
        return  BSIERROR;

    auto ecs = Transform::FromIdentity ();
    entity->GetEcs (ecs);

    auto color = entity->GetColor().GetMRGB ();
    auto layer = entity->GetLayerId().ToUInt64 ();
    auto material = entity->GetMaterialId().ToUInt64 ();
    auto transparency = entity->GetTransparency().SerializeOut ();
    bool visibility = entity->GetVisibility() == DwgDbVisibility::Visible;

    // hash entity properties
    m_hasher.Add (&range, sizeof(range));
    m_hasher.Add (&ecs, sizeof(ecs));
    m_hasher.Add (&color, sizeof(color));
    m_hasher.Add (&layer, sizeof(layer));
    m_hasher.Add (&material, sizeof(material));
    m_hasher.Add (&transparency, sizeof(transparency));
    m_hasher.Add (&visibility, sizeof(visibility));

    // hash solid properties
    if (nullptr != solid3d)
        {
        auto numChanges = solid3d->GetNumChanges ();
        m_hasher.Add (&numChanges, sizeof(numChanges));
        
        DwgDb3dSolid::MassProperties    massProps;
        if (DwgDbStatus::Success == solid3d->GetMassProperties(massProps))
            m_hasher.Add (&massProps, sizeof(DwgDb3dSolid::MassProperties));

        double area = 0.0;
        if (DwgDbStatus::Success == solid3d->GetArea(area))
            m_hasher.Add (&area, sizeof(area));
        }
    else if (nullptr != body)
        {
        auto numChanges = body->GetNumChanges ();
        m_hasher.Add (&numChanges, sizeof(numChanges));
        }
    else if (nullptr != region)
        {
        auto numChanges = region->GetNumChanges ();
        m_hasher.Add (&numChanges, sizeof(numChanges));
        double area = 0.0;
        if (DwgDbStatus::Success == region->GetArea(area))
            m_hasher.Add (&area, sizeof(area));
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void    AppendDependentObjectHash (HashFiler& filer)
    {
    // copy then remove the ID collection from the filer to avoid nesting
    auto collectedIds = filer.GetCollectedIds ();
    filer.ResetCollectedIds ();

    // check extended dictionary
    for (auto id : collectedIds)
        {
        DwgDbDictionaryPtr  extDictionary(id, DwgDbOpenMode::ForRead);
        if (extDictionary.OpenStatus() == DwgDbStatus::Success)
            {
            auto iter = extDictionary->GetIterator ();
            if (!iter.IsValid() || !iter->IsValid())
                continue;
            for (; !iter->Done(); iter->Next())
                {
                // add the child object provenance into current object provenance
                DwgDbObjectPtr child(iter->GetObjectId(), DwgDbOpenMode::ForRead);
                if (child.OpenStatus() == DwgDbStatus::Success)
                    child->DxfOut (filer);
                }
            }
        }
    // don't recurse into nested objects until we have a real need for it!
    }


public:
// the constructor
HashFactory (DwgDbObjectCR obj, BentleyApi::MD5& md5) : m_object(obj), m_hasher(md5) {}

void SetSyncBlockChanges (bool syncBlock) { m_syncOptions.m_syncBlockChanges = syncBlock; }
void SetSyncAsmBodyInFull (bool syncAsm) { m_syncOptions.m_syncAsmBodyInFull = syncAsm; }
void SetSyncDependentObjects (bool syncDep) { m_syncOptions.m_syncDependentObjects = syncDep; }

void ResetHasher () { m_hasher.Reset(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CreateObjectHash (bool reset = true)
    {
    // the default dxfOut is expensive for ASM entities, honor an option to hash only Brep data:
    if (!m_syncOptions.m_syncAsmBodyInFull && BSISUCCESS == this->CreateAsmObjectHash(m_object))
        return  BSISUCCESS;

    // hash the DWG object via DxfFiler
    if (reset)
        m_hasher.Reset ();

    HashFiler filer(m_hasher, m_object);
    if (!filer.IsValid())
        return  BSIERROR;

    try
        {
        m_object.DxfOut (filer);
        }
    catch (...)
        {
        // OpenDWG may throw exception on writing TTF file
        }

    // default dxfOut skips enssenstial data required for some standard complex objects, add these now:
    this->AppendComplexEntityHash (filer);

    // an options to hash dependent objects under the extended dictionary:
    if (m_syncOptions.m_syncDependentObjects)
        this->AppendDependentObjectHash (filer);

    // an option to recursively add block definitions - a robust change detection but also a very expensive operation
    if (m_syncOptions.m_syncBlockChanges)
        {
        DwgDbBlockReferenceCP   insert = DwgDbBlockReference::Cast (&m_object);
        if (nullptr != insert)
            this->AppendBlockHash(insert->GetBlockTableRecordId());
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::MD5::HashVal GetHashValue ()
    {
    return m_hasher.GetHashVal ();
    }

};  // HashFactory



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbHandle DwgSourceAspects::BaseAspect::GetIdentifierAsHandle () const
    {
    auto idstr = this->GetIdentifier();
    return BaseAspect::ParseDbHandle(idstr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::RepositoryLinkAspect DwgSourceAspects::RepositoryLinkAspect::Create (DgnDbR db, DwgFileInfoCR finfo)
    {
    auto aspectClass = iModelExternalSourceAspect::GetAspectClass(db);
    if (nullptr == aspectClass)
        return nullptr;
    
    SourceState sourceState;
    iModelExternalSourceAspect::UInt64ToString(sourceState.m_version, finfo.GetLastModifiedTime());

    auto ecInstance = iModelExternalSourceAspect::CreateInstance(db.Elements().GetRootSubjectId(), Kind::RepositoryLink, finfo.GetUniqueName(), &sourceState, *aspectClass);
    if (!ecInstance.IsValid())
        return nullptr;
    
    auto fileSize = iModelExternalSourceAspect::UInt64ToString(finfo.GetFileSize());
    auto rlinkId = finfo.GetRootRepositoryLinkId();
    auto rootReplinkId = rlinkId.IsValid() ? iModelExternalSourceAspect::UInt64ToString(rlinkId.GetValue()) : Utf8String();

    RepositoryLinkAspect aspect(ecInstance.get());
    
    // add json properties to the aspect
    rapidjson::Document json(rapidjson::kObjectType);
    auto& allocator = json.GetAllocator();

    json.AddMember(FileProperty::LastSaveTime, finfo.GetLastSaveTime(), allocator);
    json.AddMember(FileProperty::FileSize, rapidjson::Value(fileSize.c_str(), allocator), allocator);
    json.AddMember(FileProperty::VersionGuid, rapidjson::Value(finfo.GetVersionGuid().c_str(), allocator), allocator);
    json.AddMember(FileProperty::UniqueName, rapidjson::Value(finfo.GetUniqueName().c_str(), allocator), allocator);
    json.AddMember(FileProperty::DwgName, rapidjson::Value(finfo.GetDwgName().c_str(), allocator), allocator);
    json.AddMember(FileProperty::IdPolicy, (int)finfo.GetIdPolicy(), allocator);
    json.AddMember(FileProperty::RootRepositoryLink, rapidjson::Value(rootReplinkId.c_str(), allocator), allocator);

    aspect.SetProperties(json);

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSourceAspects::RepositoryLinkAspect::Update (DwgFileInfo const& finfo)
    {
    iModelExternalSourceAspect::SourceState sourceState;
    iModelExternalSourceAspect::UInt64ToString(sourceState.m_version, finfo.GetLastModifiedTime());
    SetSourceState(sourceState);

    auto fileSize = iModelExternalSourceAspect::UInt64ToString(finfo.GetFileSize());
    auto rootReplinkId = iModelExternalSourceAspect::UInt64ToString(finfo.GetRootRepositoryLinkId().GetValue());

    // reset lastSaveTime & fileSize
    auto json = this->GetProperties();
    json[FileProperty::LastSaveTime] = finfo.GetLastSaveTime();
    json[FileProperty::FileSize] = rapidjson::Value(fileSize.c_str(), json.GetAllocator());
    json[FileProperty::RootRepositoryLink] = rapidjson::Value(rootReplinkId.c_str(), json.GetAllocator());

    this->SetProperties(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::RepositoryLinkAspect DwgSourceAspects::RepositoryLinkAspect::FindByIdentifier (DgnDbR db, Utf8StringCR identifier)
    {
    auto ids = iModelExternalSourceAspect::FindElementBySourceId(db, db.Elements().GetRootSubjectId(), Kind::RepositoryLink, identifier);
    auto link = db.Elements().Get<RepositoryLink>(ids.elementId);
    if (!link.IsValid())
        return nullptr;

    auto found = DwgSourceAspects::GetSoleAspectIdByKind(*link, Kind::RepositoryLink);
    if (!found.IsValid())
        return nullptr;

    auto aspect = iModelExternalSourceAspect::GetAspect(*link, found, nullptr);
    return RepositoryLinkAspect(aspect.GetInstanceP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::RepositoryLinkAspect DwgSourceAspects::RepositoryLinkAspect::Get (RepositoryLinkCR link)
    {
    auto found = DwgSourceAspects::GetSoleAspectIdByKind(link, Kind::RepositoryLink);
    if (!found.IsValid())
        return nullptr;
    
    auto aspect = iModelExternalSourceAspect::GetAspect(link, found, nullptr);
    return RepositoryLinkAspect(aspect.GetInstanceP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::RepositoryLinkAspect DwgSourceAspects::RepositoryLinkAspect::GetForEdit (RepositoryLinkR link)
    {
    auto found = DwgSourceAspects::GetSoleAspectIdByKind(link, Kind::RepositoryLink);
    if (!found.IsValid())
        return nullptr;
    
    auto aspect = iModelExternalSourceAspect::GetAspectForEdit(link, found, nullptr);
    return RepositoryLinkAspect(aspect.GetInstanceP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DwgSourceAspects::RepositoryLinkAspect::GetFileName () const
    {
    auto json = this->GetProperties();
    auto filename = json[FileProperty::DwgName].GetString();
    return  filename;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
StableIdPolicy DwgSourceAspects::RepositoryLinkAspect::GetStableIdPolicy () const
    {
    auto json = this->GetProperties();
    auto propVal = json[FileProperty::IdPolicy].GetInt();
    return static_cast<StableIdPolicy>(propVal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSourceAspects::RepositoryLinkAspect::GetDwgFileInfo (DwgSourceAspects::DwgFileInfoR finfo) const
    {
    auto json = this->GetProperties();
    finfo.SetUniqueName(json[FileProperty::UniqueName].GetString());
    finfo.SetDwgName (json[FileProperty::DwgName].GetString());
    finfo.SetVersionGuid (json[FileProperty::VersionGuid].GetString());
    finfo.SetIdPolicy (static_cast<StableIdPolicy>(json[FileProperty::IdPolicy].GetInt()));
    finfo.SetLastSaveTime (json[FileProperty::LastSaveTime].GetDouble());
    finfo.SetFileSize (::_atoi64(json[FileProperty::FileSize].GetString()));

    uint64_t rootReplinkId = ::_atoi64(json[FileProperty::RootRepositoryLink].GetString());
    finfo.SetRootRepositoryLinkId (RepositoryLinkId(rootReplinkId));
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DwgSourceAspects::RepositoryLinkAspect::GetDwgName () const
    {
    return this->GetProperties()[FileProperty::DwgName].GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DwgSourceAspects::RepositoryLinkAspect::GetUniqueName () const
    {
    return this->GetProperties()[FileProperty::UniqueName].GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryLinkId DwgSourceAspects::RepositoryLinkAspect::GetRootRepositoryLinkId () const
    {
    auto idstr = this->GetProperties()[FileProperty::RootRepositoryLink].GetString();
    uint64_t v = ::_atoi64(idstr);
    return  RepositoryLinkId(v);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSourceAspects::UriContentInfo::SetFrom (Utf8StringCR pathOrUri)
    {
    BentleyStatus   status = BSIERROR;
    m_eTag.clear();
    if (pathOrUri.StartsWithI("http:") || pathOrUri.StartsWithI("https:"))
        {
        BentleyApi::Http::Request request(pathOrUri, "HEAD");
        folly::Future<BentleyApi::Http::Response> response = request.Perform().wait();
        if (BentleyApi::Http::HttpStatus::OK == response.value().GetHttpStatus())
            {
            auto headers = response.value().GetHeaders();
            m_eTag = headers.GetETag();
            status = BSISUCCESS;
            }
        else
            {
            status = static_cast<BentleyStatus>(response.value().GetHttpStatus());
            }
        }
    else
        {
        status = DiskFileInfo::SetFrom(BeFileName(pathOrUri.c_str(), true));
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::UriAspect DwgSourceAspects::UriAspect::Create(DgnElementId linkId, Utf8CP fname, UriContentInfoCR info, Utf8CP rdsId, DgnDbR db)
    {
    auto aspectClass = iModelExternalSourceAspect::GetAspectClass(db);
    if (aspectClass == nullptr)
        return  nullptr;

    SourceState sourceState;
    iModelExternalSourceAspect::UInt64ToString(sourceState.m_version, info.GetLastModifiedTime());
    auto instance = iModelExternalSourceAspect::CreateInstance(linkId, Kind::URI, fname, &sourceState, *aspectClass);
    if (!instance.IsValid())
        return  nullptr;
    
    UriAspect aspect(instance.get());
    
    rapidjson::Document json(rapidjson::kObjectType);
    auto& allocator = json.GetAllocator();

    json.AddMember(UriProperty::FileSize, rapidjson::Value(iModelExternalSourceAspect::UInt64ToString(info.GetFileSize()).c_str(), allocator), allocator);
    json.AddMember(UriProperty::ETag, rapidjson::Value(info.m_eTag.c_str(), allocator), allocator);
    json.AddMember(UriProperty::RdsId, rapidjson::Value(rdsId ? rdsId : "", allocator), allocator);
    aspect.SetProperties(json);

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::UriAspect DwgSourceAspects::UriAspect::Get(DgnElementId id, DgnDbR db)
    {
    auto element = db.Elements().GetElement(id);
    return element.IsValid() ? Get(*element) : UriAspect(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::UriAspect DwgSourceAspects::UriAspect::Get(DgnElementCR element)
    {
    auto aspectId = DwgSourceAspects::GetSoleAspectIdByKind(element, Kind::URI);
    if (!aspectId.IsValid())
        return nullptr;
    return UriAspect(BaseAspect::GetAspect(element, aspectId).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::UriAspect DwgSourceAspects::UriAspect::GetForEdit(DgnElementR element)
    {
    auto aspectId = DwgSourceAspects::GetSoleAspectIdByKind(element, Kind::URI);
    if (!aspectId.IsValid())
        return nullptr;
    return UriAspect(BaseAspect::GetAspectForEdit(element, aspectId).m_instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DwgSourceAspects::UriAspect::GetSourceGuid() const
    {
    auto json = this->GetProperties();
    auto rdsid = json[UriProperty::RdsId].GetString();
    return  rdsid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSourceAspects::UriAspect::SetSourceGuid(Utf8StringCR rdsId)
    {
    auto json = this->GetProperties();
    json[UriProperty::RdsId] = rapidjson::Value(rdsId.c_str(), json.GetAllocator());
    this->SetProperties(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSourceAspects::UriAspect::GetInfo(UriContentInfoR outInfo) const
    {
    auto sourceState = this->GetSourceState();
    outInfo.SetLastModifiedTime(iModelExternalSourceAspect::UInt64FromString(sourceState.m_version.c_str()));

    auto json = this->GetProperties();
    auto fsize = json[UriProperty::FileSize].GetString();

    outInfo.SetFileSize(iModelExternalSourceAspect::UInt64FromString(fsize));
    outInfo.m_eTag = json[UriProperty::ETag].GetString();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSourceAspects::UriAspect::SetInfo(UriContentInfoCR inInfo)
    {
    auto sourceState = GetSourceState();
    iModelExternalSourceAspect::UInt64ToString(sourceState.m_version, inInfo.GetLastModifiedTime());
    this->SetSourceState(sourceState);

    auto json = this->GetProperties();
    auto fsize = iModelExternalSourceAspect::UInt64ToString(inInfo.GetFileSize());

    json[UriProperty::FileSize] = rapidjson::Value(fsize.c_str(), json.GetAllocator());
    json[UriProperty::ETag] = rapidjson::Value(inInfo.m_eTag.c_str(), json.GetAllocator());
    this->SetProperties(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ModelAspect   DwgSourceAspects::ModelAspect::CreateStub (DwgDbObjectIdCR objId, DwgDbDatabaseP dwg, TransformCR trans, DwgImporterR importer)
    {
    // create a model aspect ready to be set
    if (dwg == nullptr)
        return nullptr;

    // scope from input DWG file
    auto linkId = importer.GetRepositoryLink(dwg);
    if (!linkId.IsValid())
        return nullptr;

    auto aspectClass = iModelExternalSourceAspect::GetAspectClass(importer.GetDgnDb());
    if (nullptr == aspectClass)
        return nullptr;
    
    auto instance = iModelExternalSourceAspect::CreateInstance(linkId, Kind::Model, BaseAspect::FormatObjectId(objId), nullptr, *aspectClass);
    return  ModelAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgSourceAspects::ModelAspect::AddProperties (Utf8StringCR dwgName, TransformCR transform, Utf8CP sourceType)
    {
    // add json properties from input params for the aspect
    rapidjson::Document json(rapidjson::kObjectType);
    auto& allocator = json.GetAllocator();
    auto jsonArray = DwgHelper::GetJsonFromDoubleArray (reinterpret_cast<double const*>(&transform), 12, allocator);

    json.AddMember(ModelProperty::Transform, jsonArray, allocator);
    json.AddMember(ModelProperty::DwgName, rapidjson::Value(dwgName.c_str(), allocator), allocator);
    json.AddMember(ModelProperty::DwgSourceType, rapidjson::Value(sourceType, allocator), allocator);

    this->SetProperties(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ModelAspect   DwgSourceAspects::ModelAspect::Create (DwgDbBlockTableRecordCR block, TransformCR transform, DwgImporterR importer)
    {
    // create a modelspace or paperspace aspect, in master DWG file scope
    auto aspect = DwgSourceAspects::ModelAspect::CreateStub (block.GetObjectId(), block.GetDatabase().get(), transform, importer);
    if (aspect.IsValid())
        {
        // Use file name for modelspace and layout name for paperspaces
        Utf8String dwgName;
        if (block.IsModelspace())
            {
            auto dwg = block.GetDatabase();
            if (dwg == nullptr)
                dwgName.Assign(block.GetName().c_str());
            else
                dwgName.Assign(BeFileName::GetFileNameWithoutExtension(dwg->GetFileName().c_str()).c_str());
            }
        else
            {
            DwgDbLayoutPtr  layout(block.GetLayoutId(), DwgDbOpenMode::ForRead);
            if (layout.OpenStatus() == DwgDbStatus::Success)
                dwgName.Assign(layout->GetName().c_str());
            else
                dwgName.Assign(block.GetName().c_str());
            }
        Utf8CP sourceType = block.IsModelspace() ? ModelProperty::SourceType::ModelSpace : ModelProperty::SourceType::PaperSpace;
        aspect.AddProperties (dwgName.c_str(), transform, sourceType);
        }
    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ModelAspect   DwgSourceAspects::ModelAspect::Create (DwgDbBlockReferenceCR xinsert, DwgDbDatabaseR xref, TransformCR transform, DwgImporterR importer)
    {
    // create an xref model aspect, in xref DWG scope
    auto aspect = DwgSourceAspects::ModelAspect::CreateStub(xinsert.GetObjectId(), &xref, transform, importer);
    if (aspect.IsValid())
        {
        DwgDbBlockTableRecordPtr block(xinsert.GetBlockTableRecordId(), DwgDbOpenMode::ForRead);
        if (block.OpenStatus() != DwgDbStatus::Success)
            return  nullptr;

        Utf8String  dwgName(BeFileName::GetFileNameWithoutExtension(block->GetPath().c_str()).c_str());
        aspect.AddProperties (dwgName.c_str(), transform, ModelProperty::SourceType::XRefAttachment);
        }
    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ModelAspect   DwgSourceAspects::ModelAspect::Create (DwgDbRasterImageCR image, TransformCR transform, DwgImporterR importer)
    {
    // create an raster model aspect, in raster's owner scope
    auto aspect = DwgSourceAspects::ModelAspect::CreateStub (image.GetObjectId(), image.GetDatabase().get(), transform, importer);
    if (aspect.IsValid())
        {
        DwgString   sourceFile, activeFile;
        if (image.GetFileName(sourceFile, &activeFile) != DwgDbStatus::Success)
            return  nullptr;

        Utf8String  dwgName(BeFileName::GetFileNameWithoutExtension(sourceFile.IsEmpty() ? activeFile.c_str() : sourceFile.c_str()).c_str());
        aspect.AddProperties (dwgName.c_str(), transform, ModelProperty::SourceType::RasterAttachment);
        }
    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  DwgSourceAspects::ModelAspect::GetDwgModelName () const
    {
    auto json = this->GetProperties();
    return json[ModelProperty::DwgName].GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void     DwgSourceAspects::ModelAspect::SetDwgModelName (Utf8StringCR dwgName)
    {
    auto json = this->GetProperties();
    json[ModelProperty::DwgName] = rapidjson::Value(dwgName.c_str(), json.GetAllocator());
    this->SetProperties(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
Transform   DwgSourceAspects::ModelAspect::GetTransform () const
    {
    auto json = this->GetProperties();
    Transform transform;
    DwgHelper::GetDoubleArrayFromJson(reinterpret_cast<double*>(&transform), 12, json[ModelProperty::Transform].GetArray());
    return transform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void     DwgSourceAspects::ModelAspect::SetTransform (TransformCR transform)
    {
    auto json = this->GetProperties();
    json[ModelProperty::Transform] = DwgHelper::GetJsonFromDoubleArray(reinterpret_cast<double const*>(&transform), 12, json.GetAllocator());
    this->SetProperties(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ModelAspect::SourceType DwgSourceAspects::ModelAspect::GetSourceType () const
    {
    auto json = this->GetProperties();
    auto str = json[ModelProperty::DwgSourceType].GetString();
    return  ModelProperty::SourceType::ToEnum(str);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ModelAspect   DwgSourceAspects::ModelAspect::GetByAspectId (DgnDbR db, BeSQLite::EC::ECInstanceId id)
    {
    auto stmt = db.GetPreparedECSqlStatement("SELECT Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE (ECInstanceId=?)");
    stmt->BindId(1, id);
    if (BE_SQLITE_ROW == stmt->Step())
        {
        auto element = db.Elements().GetElement(stmt->GetValueId<DgnElementId>(0));
        if (element.IsValid())
            {
            auto aspect = iModelExternalSourceAspect::GetAspect(*element, id);
            if (aspect.IsValid() && aspect.GetKind() == Kind::Model)
                return ModelAspect(aspect.m_instance.get());
            }
        }
    BeAssert (false && "ModelAspect not found by ID!");
    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DwgSourceAspects::ModelAspectIterator::Count () const
    {
    size_t  count = 0;
    for (auto iter = this->begin(); iter != this->end(); ++iter)
        count++;
    return  count;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::LayerAspect DwgSourceAspects::LayerAspect::Create(DwgDbObjectIdCR layerId, Type type, DgnDbR db, DwgImporterR importer)
    {
    auto dwg = layerId.GetDatabase();
    if (dwg == nullptr)
        return nullptr;

    auto rlinkId = importer.GetRepositoryLink(dwg);
    if (!rlinkId.IsValid())
        return nullptr;

    auto aspectClass = iModelExternalSourceAspect::GetAspectClass(db);
    if (nullptr == aspectClass)
        return nullptr;
    
    auto idstr = BaseAspect::FormatObjectId(layerId);
    auto instance = iModelExternalSourceAspect::CreateInstance(rlinkId, Kind::Layer, idstr, nullptr, *aspectClass);

    LayerAspect aspect(instance.get());
    if (aspect.IsValid())
        {
        DwgDbLayerTableRecordPtr    layer(layerId, DwgDbOpenMode::ForRead);
        if (layer.OpenStatus() != DwgDbStatus::Success)
            return  nullptr;

        Utf8String layerName(layer->GetName().c_str());

        rapidjson::Document json(rapidjson::kObjectType);
        auto& allocator = json.GetAllocator();

        json.AddMember(LayerProperty::LayerType, static_cast<int>(type), allocator);
        json.AddMember(LayerProperty::LayerName, rapidjson::Value(layerName.c_str(), allocator), allocator);
        aspect.SetProperties(json);
        }
    return  aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::LayerAspect DwgSourceAspects::LayerAspect::FindBy(DgnSubCategoryCR subcat, DgnElementId scope)
    {
    auto sql = subcat.GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE (Element.Id=? AND Scope.Id=? AND Kind=?)");
    if (sql == nullptr)
        return  nullptr;

    sql->BindId(1, subcat.GetElementId());
    sql->BindId(2, scope);
    sql->BindText(3, Kind::Layer, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW != sql->Step())
        return nullptr;
    auto instance = BaseAspect::GetAspect(subcat, sql->GetValueId<BeSQLite::EC::ECInstanceId>(0)).m_instance.get();
    return LayerAspect(instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSourceAspects::LayerAspect::FindFirstSubCategory(DgnSubCategoryId& subcatId, DgnDbR db, DwgDbObjectIdCR layerId, Type const* type, DwgImporterR importer)
    {
    auto dwg = layerId.GetDatabase();
    if (dwg == nullptr)
        return BSIERROR;

    auto rlinkId = importer.GetRepositoryLink(dwg);
    if (!rlinkId.IsValid())
        return BSIERROR;

    auto idstr = BaseAspect::FormatObjectId(layerId);
    BeSQLite::EC::CachedECSqlStatementPtr sql;

    if (type == nullptr)
        {
        sql = db.GetPreparedECSqlStatement("SELECT Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE (Scope.Id=? AND Kind=? AND Identifier=?)");
        sql->BindId(1, rlinkId);
        sql->BindText(2, Kind::Layer, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        sql->BindText(3, idstr.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        }
    else
        {
        Utf8PrintfString typestr("%d", static_cast<int>(*type));

        sql = db.GetPreparedECSqlStatement(
            "SELECT x.Element.Id, x.JsonProperties FROM " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " x "
            " WHERE (x.Scope.Id=? AND x.Kind=? AND x.Identifier=? AND json_extract(x.JsonProperties, '$.LayerType') = ?)");
        sql->BindId(1, rlinkId);
        sql->BindText(2, Kind::Layer, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        sql->BindText(3, idstr.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        sql->BindText(4, typestr.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        }

    while (BE_SQLITE_ROW == sql->Step())
        {
        subcatId = sql->GetValueId<DgnSubCategoryId>(0);
        return BSISUCCESS;
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DwgSourceAspects::LayerAspect::GetLayerName () const
    {
    auto json = this->GetProperties();
    return  json[LayerProperty::LayerName].GetString();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ViewAspect DwgSourceAspects::ViewAspect::Create(DgnElementId scopeId, DwgDbObjectIdCR sourceId, SourceType sourceType, Utf8StringCR viewName, DgnDbR db)
    {
    auto aspectClass = iModelExternalSourceAspect::GetAspectClass(db);
    if (nullptr == aspectClass)
        return nullptr;
    
    auto idstr = BaseAspect::FormatObjectId(sourceId);
    auto instance = iModelExternalSourceAspect::CreateInstance(scopeId, Kind::ViewDefinition, idstr, nullptr, *aspectClass);

    ViewAspect aspect(instance.get());
    if (aspect.IsValid())
        {
        rapidjson::Document json(rapidjson::kObjectType);
        auto& allocator = json.GetAllocator();
        json.AddMember(ViewProperty::DwgSourceType, rapidjson::Value(ViewProperty::SourceType::FromEnum(sourceType), allocator), allocator);
        json.AddMember(ViewProperty::DwgName, rapidjson::Value(viewName.c_str(), allocator), allocator);
        aspect.SetProperties(json);
        }
    return  aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSourceAspects::ViewAspect::Update(SourceType sourceType, Utf8StringCR viewName)
    {
    auto json = this->GetProperties();
    auto& allocator = json.GetAllocator();

    auto stchars = ViewProperty::SourceType::FromEnum(sourceType);
    if (!json.HasMember(ViewProperty::DwgSourceType))
        json.AddMember(ViewProperty::DwgSourceType, rapidjson::Value(stchars, allocator), allocator);
    else
        json[ViewProperty::DwgSourceType] = rapidjson::Value(stchars, allocator);

    if (!json.HasMember(ViewProperty::DwgName))
        json.AddMember(ViewProperty::DwgName, rapidjson::Value(viewName.c_str(), allocator), allocator);
    else
        json[ViewProperty::DwgSourceType] = rapidjson::Value(viewName.c_str(), allocator);

    this->SetProperties(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ViewAspect DwgSourceAspects::ViewAspect::GetForEdit(ViewDefinitionR view)
    {
    auto aspectId = DwgSourceAspects::GetSoleAspectIdByKind(view, Kind::ViewDefinition);
    if (!aspectId.IsValid())
        return nullptr;
    auto instance = iModelExternalSourceAspect::GetAspectForEdit(view, aspectId).m_instance;
    return ViewAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ViewAspect DwgSourceAspects::ViewAspect::Get(ViewDefinitionCR view)
    {
    auto aspectId = DwgSourceAspects::GetSoleAspectIdByKind(view, Kind::ViewDefinition);
    if (!aspectId.IsValid())
        return nullptr;
    auto instance = iModelExternalSourceAspect::GetAspect(view, aspectId).m_instance;
    return ViewAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
std::tuple<DwgSourceAspects::ViewAspect,DgnViewId> DwgSourceAspects::ViewAspect::Get(DgnElementId scope, DwgDbObjectIdCR sourceId, DgnDbR db)
    {
    auto id = iModelExternalSourceAspect::FindElementBySourceId(db, scope, Kind::ViewDefinition, FormatObjectId(sourceId)).elementId;
    if (!id.IsValid())
        return std::make_tuple(ViewAspect(), DgnViewId());

    auto view = db.Elements().Get<ViewDefinition>(id);
    if (!view.IsValid())
        return std::make_tuple(ViewAspect(), DgnViewId());

    return std::make_tuple(Get(*view), DgnViewId(id.GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ViewAspect::SourceType DwgSourceAspects::ViewAspect::GetSourceType() const
    {
    auto json = this->GetProperties();
    auto str = json[ViewProperty::DwgSourceType].GetString();
    return  ViewProperty::SourceType::ToEnum(str);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::LinetypeAspect DwgSourceAspects::LinetypeAspect::Create(DgnElementId scopeId, DwgDbObjectIdCR sourceId, Utf8StringCR ltypeName, DgnDbR db)
    {
    auto aspectClass = iModelExternalSourceAspect::GetAspectClass(db);
    if (nullptr == aspectClass)
        return nullptr;
    
    auto idstr = BaseAspect::FormatObjectId(sourceId);
    auto instance = iModelExternalSourceAspect::CreateInstance(scopeId, Kind::Linetype, idstr, nullptr, *aspectClass);

    LinetypeAspect aspect(instance.get());
    if (aspect.IsValid())
        {
        rapidjson::Document json(rapidjson::kObjectType);
        auto& allocator = json.GetAllocator();
        json.AddMember(LinetypeProperty::LinetypeName, rapidjson::Value(ltypeName.c_str(), allocator), allocator);
        aspect.SetProperties(json);
        }
    return  aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::LinetypeAspect DwgSourceAspects::LinetypeAspect::GetForEdit(LineStyleElementR linestyle)
    {
    auto aspectId = DwgSourceAspects::GetSoleAspectIdByKind(linestyle, Kind::Linetype);
    if (!aspectId.IsValid())
        return nullptr;
    auto instance = iModelExternalSourceAspect::GetAspectForEdit(linestyle, aspectId).m_instance;
    return LinetypeAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::LinetypeAspect DwgSourceAspects::LinetypeAspect::Get(LineStyleElementCR linestyle)
    {
    auto aspectId = DwgSourceAspects::GetSoleAspectIdByKind(linestyle, Kind::Linetype);
    if (!aspectId.IsValid())
        return nullptr;
    auto instance = iModelExternalSourceAspect::GetAspect(linestyle, aspectId).m_instance;
    return LinetypeAspect(instance.get());
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSourceAspects::LinetypeAspect::Update(Utf8StringCR ltypeName)
    {
    auto json = this->GetProperties();
    auto& allocator = json.GetAllocator();

    if (!json.HasMember(LinetypeProperty::LinetypeName))
        json.AddMember(LinetypeProperty::LinetypeName, rapidjson::Value(ltypeName.c_str(), allocator), allocator);
    else
        json[LinetypeProperty::LinetypeName] = rapidjson::Value(ltypeName.c_str(), allocator);

    this->SetProperties(json);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectProvenance DwgSourceAspects::MaterialAspect::CreateProvenance(DwgDbMaterialCR material, DwgImporterR importer)
    {
    auto object = DwgDbObject::Cast(&material);
    if (object == nullptr)
        {
        BeAssert (false && "DwgDbMaterial is not a DwgDbObject!");
        return  DwgSourceAspects::ObjectProvenance();
        }
    // create a full object provenance from the materil
    return DwgSourceAspects::ObjectProvenance(*object, importer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::MaterialAspect DwgSourceAspects::MaterialAspect::Create(DwgDbMaterialCR material, DwgImporterR importer)
    {
    auto aspectClass = iModelExternalSourceAspect::GetAspectClass(importer.GetDgnDb());
    if (nullptr == aspectClass)
        return nullptr;
    
    // scope all materials per master file
    auto scopeId = importer.GetRepositoryLink(&importer.GetDwgDb());
    auto idstr = BaseAspect::FormatObjectId(material.GetObjectId());
    auto instance = iModelExternalSourceAspect::CreateInstance(scopeId, Kind::Material, idstr, nullptr, *aspectClass);

    MaterialAspect aspect(instance.get());
    if (aspect.IsValid())
        {
        Utf8String  name(material.GetName().c_str());
        auto prov = DwgSourceAspects::MaterialAspect::CreateProvenance(material, importer);
        if (!prov.IsNull())
            aspect.Update(prov, name);
        else
            aspect.Invalidate();
        }
    return  aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSourceAspects::MaterialAspect::Update(DwgSourceAspects::ObjectProvenanceCR prov, Utf8StringCR name)
    {
    rapidjson::Document json(rapidjson::kObjectType);
    auto& allocator = json.GetAllocator();
    json.AddMember(MaterialProperty::MaterialName, rapidjson::Value(name.c_str(), allocator), allocator);
    this->SetProperties(json);

    SourceState sourceState;
    if (prov.GetHash().AsHexString(sourceState.m_checksum) > 0)
        this->SetSourceState(sourceState); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::MaterialAspect DwgSourceAspects::MaterialAspect::GetForEdit(RenderMaterialR material)
    {
    auto aspectId = DwgSourceAspects::GetSoleAspectIdByKind(material, Kind::Material);
    if (!aspectId.IsValid())
        return nullptr;
    auto instance = iModelExternalSourceAspect::GetAspectForEdit(material, aspectId).m_instance;
    return MaterialAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::MaterialAspect DwgSourceAspects::MaterialAspect::Get(RenderMaterialCR material)
    {
    auto aspectId = DwgSourceAspects::GetSoleAspectIdByKind(material, Kind::Material);
    if (!aspectId.IsValid())
        return nullptr;
    auto instance = iModelExternalSourceAspect::GetAspect(material, aspectId).m_instance;
    return MaterialAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSourceAspects::MaterialAspect::IsSameMaterial(ObjectProvenanceCR prov, Utf8StringCR name) const
    {
    // match name
    auto json = this->GetProperties();
    Utf8String  oldName = json[MaterialProperty::MaterialName].GetString();
    if (!oldName.EqualsI(name))
        return  false;
    
    // match provenance
    auto sourceState = this->GetSourceState();
    Utf8String hashstr;
    if (prov.GetHash().AsHexString(hashstr) > 0)
        return hashstr.Equals(sourceState.m_checksum);
    BeAssert (false && "ObjectProvenance not yet hashed!");
    return  false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::GroupAspect DwgSourceAspects::GroupAspect::Create(DwgDbGroupCR group, DwgImporterR importer)
    {
    auto aspectClass = iModelExternalSourceAspect::GetAspectClass(importer.GetDgnDb());
    if (nullptr == aspectClass)
        return nullptr;
    
    // scope group per DWG file
    auto dwg = group.GetDatabase().get();
    if (dwg == nullptr)
        return  nullptr;

    auto scopeId = importer.GetRepositoryLink(dwg);
    auto idstr = BaseAspect::FormatObjectId(group.GetObjectId());
    auto instance = iModelExternalSourceAspect::CreateInstance(scopeId, Kind::Group, idstr, nullptr, *aspectClass);

    GroupAspect aspect(instance.get());
    if (aspect.IsValid())
        {
        Utf8String  name(group.GetName().c_str());
        auto prov = DwgSourceAspects::GroupAspect::CreateProvenance(group, importer);
        if (!prov.IsNull())
            aspect.Update (prov, name);
        else
            aspect.Invalidate();
        }
    return  aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSourceAspects::GroupAspect::Update(DwgSourceAspects::ObjectProvenanceCR prov, Utf8StringCR name)
    {
    rapidjson::Document json(rapidjson::kObjectType);
    auto& allocator = json.GetAllocator();
    json.AddMember(GroupProperty::GroupName, rapidjson::Value(name.c_str(), allocator), allocator);
    this->SetProperties(json);

    SourceState sourceState;
    if (prov.GetHash().AsHexString(sourceState.m_checksum) > 0)
        this->SetSourceState(sourceState); 
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectProvenance DwgSourceAspects::GroupAspect::CreateProvenance(DwgDbGroupCR group, DwgImporterR importer)
    {
    auto object = DwgDbObject::Cast(&group);
    if (object == nullptr)
        {
        BeAssert (false && "DwgDbGroup is not seen as a DwgDbObject!");
        return  DwgSourceAspects::ObjectProvenance();
        }

    // create a full object provenance from the group object
    DwgSourceAspects::ObjectProvenance  prov(*object, importer);
    
    if (importer._ShouldSyncGroupWithMembers())
        {
        // add member objects into the provenance
        DwgDbObjectIdArray  memberIds;
        if (group.GetAllEntityIds(memberIds) > 0)
            {
            for (auto memberId : memberIds)
                {
                DwgDbObjectPtr  memberObject(memberId, DwgDbOpenMode::ForRead);
                if (memberObject.OpenStatus() == DwgDbStatus::Success)
                    prov.AddHash (*memberObject);
                }
            }
        }
    return  prov;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::GroupAspect DwgSourceAspects::GroupAspect::GetForEdit(DgnElementR dgnGroup)
    {
    auto aspectId = DwgSourceAspects::GetSoleAspectIdByKind(dgnGroup, Kind::Group);
    if (!aspectId.IsValid())
        return nullptr;
    auto instance = iModelExternalSourceAspect::GetAspectForEdit(dgnGroup, aspectId).m_instance;
    return GroupAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::GroupAspect DwgSourceAspects::GroupAspect::Get(DgnElementCR dgnGroup)
    {
    auto aspectId = DwgSourceAspects::GetSoleAspectIdByKind(dgnGroup, Kind::Group);
    if (!aspectId.IsValid())
        return nullptr;
    auto instance = iModelExternalSourceAspect::GetAspect(dgnGroup, aspectId).m_instance;
    return GroupAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSourceAspects::GroupAspect::IsSameGroup(ObjectProvenanceCR prov, Utf8StringCR name) const
    {
    // match name
    auto json = this->GetProperties();
    Utf8String  oldName = json[GroupProperty::GroupName].GetString();
    if (!oldName.EqualsI(name))
        return  false;
    
    // match provenance
    auto sourceState = this->GetSourceState();
    Utf8String hashstr;
    if (prov.GetHash().AsHexString(hashstr) > 0)
        return hashstr.Equals(sourceState.m_checksum);
    BeAssert(false && "ObjectProvenance not yet hashed!");
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::GeomPartAspect DwgSourceAspects::GeomPartAspect::Create(DgnElementId scopeId, Utf8StringCR tag, DgnDbR db)
    {
    auto aspectClass = iModelExternalSourceAspect::GetAspectClass(db);
    if (nullptr == aspectClass)
        return nullptr;
    auto instance = iModelExternalSourceAspect::CreateInstance(scopeId, Kind::GeometryPart, tag, nullptr, *aspectClass);
    return GeomPartAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::GeomPartAspect DwgSourceAspects::GeomPartAspect::Get(DgnGeometryPartCR geomPart)
    {
    auto aspectId = DwgSourceAspects::GetSoleAspectIdByKind(geomPart, Kind::GeometryPart);
    if (!aspectId.IsValid())
        return  nullptr;
    auto instance = iModelExternalSourceAspect::GetAspect(geomPart, aspectId).m_instance;
    return GeomPartAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeometryPartId DwgSourceAspects::GeomPartAspect::FindGeometryPart(DgnElementId scopeId, Utf8StringCR tag, DgnDbR db)
    {
    auto entry = iModelExternalSourceAspect::FindElementBySourceId(db, scopeId, Kind::GeometryPart, tag);
    return DgnGeometryPartId(entry.elementId.GetValueUnchecked());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectProvenance::ObjectProvenance(DwgDbObjectCR object, DwgImporterR importer)
    {
    ::memset (&m_hash, 0, sizeof(m_hash));
    m_hasher.Reset ();

    HashFactory factory(object, m_hasher);
    factory.SetSyncBlockChanges(importer.GetOptions().GetSyncBlockChanges());
    factory.SetSyncAsmBodyInFull(importer.GetOptions().GetSyncAsmBodyInFull());
    factory.SetSyncDependentObjects(importer.GetOptions().GetSyncDependentObjects());

    factory.CreateObjectHash(true);

    m_hash = factory.GetHashValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSourceAspects::ObjectProvenance::AddHash(DwgDbObjectCR object)
    {
    // do not reset hasher - add it to existing hasher
    HashFactory factory(object, m_hasher);
    factory.CreateObjectHash(false);
    m_hash = factory.GetHashValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSourceAspects::ObjectProvenance::AddHash(const void* binaryData, size_t numBytes)
    {
    // do not reset hasher - add it to existing hasher
    if (numBytes == 0 || binaryData == nullptr)
        {
        m_hasher.Add(binaryData, numBytes);
        m_hash = m_hasher.GetHashVal();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSourceAspects::ObjectProvenance::Hash::IsNull () const
    {
    for (size_t i = 0; i < BentleyApi::MD5::BlockSize; i++)
        {
        if (m_buffer[i] != 0)
            return  false;
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  DwgSourceAspects::ObjectProvenance::Hash::AsHexString(Utf8StringR outString) const
    {
    static constexpr Utf8Char   s_hexMaskChars[] = "0123456789ABCDEF";
    outString.clear ();

    size_t  nBytes = sizeof (m_buffer);
    for (size_t i = 0; i < nBytes; i++)
        {
        outString.push_back (s_hexMaskChars[(m_buffer[i] >> 4) & 0x0F]);
        outString.push_back (s_hexMaskChars[m_buffer[i] & 0x0F]);
        }
    return  nBytes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectAspect::SourceData::SourceData(DwgDbObjectCR obj, DgnModelId modelId, DwgImporterR importer)
    {
    m_modelId = modelId,
    m_objectHandle = obj.GetObjectId().GetHandle();
    m_provenance = ObjectProvenance (obj, importer);
    m_jsonProperties.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectAspect DwgSourceAspects::ObjectAspect::Create(SourceDataCR data, DgnDbR db)
    {
    auto aspectClass = iModelExternalSourceAspect::GetAspectClass(db);
    if (nullptr == aspectClass)
        return nullptr;

    DgnElementId    scope(data.GetModelId().GetValue());
    Utf8String  idstr = BaseAspect::FormatDbHandle(data.GetObjectHandle());
    if (idstr.empty())
        return nullptr;

    auto instance = iModelExternalSourceAspect::CreateInstance(scope, Kind::Element, idstr, nullptr, *aspectClass);
    auto aspect = ObjectAspect(instance.get());
    if (!aspect.IsValid())
        return nullptr;

    auto props = data.GetJsonProperties();
    if (!props.empty())
        {
        rapidjson::Document json(rapidjson::kObjectType);
        json.Parse(props.c_str());
        aspect.SetProperties(json);
        }
    aspect.Update(data.GetProvenance());

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectAspect DwgSourceAspects::ObjectAspect::GetForEdit(DgnElementR element, Utf8StringCR idstr)
    {
    auto ids = DwgSourceAspects::GetExternalSourceAspectIds(element, Kind::Element, idstr);
    if (ids.size() == 0)
        return nullptr;
    BeAssert(ids.size() == 1 && "Multiple aspects of Element kind on a single DgnElement from a given sourceId are not supported!");
    auto instance = iModelExternalSourceAspect::GetAspectForEdit(element, ids.front()).m_instance;
    return ObjectAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectAspect DwgSourceAspects::ObjectAspect::GetForEdit(DgnElementR element, DwgDbObjectIdCR id)
    {
    return DwgSourceAspects::ObjectAspect::GetForEdit(element, BaseAspect::FormatObjectId(id));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectAspect DwgSourceAspects::ObjectAspect::Get(DgnElementCR element, Utf8StringCR idstr)
    {
    auto ids = DwgSourceAspects::GetExternalSourceAspectIds(element, Kind::Element, idstr);
    if (ids.size() == 0)
        return nullptr;
    BeAssert(ids.size()==1 && "Multiple aspects of Element kind on a single DgnElement from a given sourceId are not supported");
    auto instance = iModelExternalSourceAspect::GetAspect(element, ids.front()).m_instance;
    return ObjectAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectAspect DwgSourceAspects::ObjectAspect::Get(DgnElementCR element, DwgDbObjectIdCR id)
    {
    return DwgSourceAspects::ObjectAspect::Get(element, BaseAspect::FormatObjectId(id));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectAspect DwgSourceAspects::ObjectAspect::Get(DgnElementCR el, BeSQLite::EC::ECInstanceId aspectId)
    {
    auto instance = iModelExternalSourceAspect::GetAspect(el, aspectId).m_instance;
    return ObjectAspect(instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DwgSourceAspects::ObjectAspect::GetModelId () const
    {
    return  DgnModelId(this->GetScope().GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSourceAspects::ObjectAspect::Update(ObjectProvenanceCR prov)
    {
    SourceState sourceState;
    if (prov.GetHash().AsHexString(sourceState.m_checksum) > 0)
        this->SetSourceState(sourceState); 
    else
        BeAssert(false && "Uinitialized object provenance!");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  DwgSourceAspects::ObjectAspect::GetHashString() const
    {
    if (!this->IsValid())
        return  Utf8String();
    auto sourceState = this->GetSourceState();
    return  sourceState.m_checksum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSourceAspects::ObjectAspect::IsProvenanceEqual(ObjectProvenanceCR prov) const
    {
    auto sourceState = this->GetSourceState();
    Utf8String hashstr;
    if (prov.GetHash().AsHexString(hashstr) > 0)
        return hashstr.Equals(sourceState.m_checksum);
    BeAssert(false && "ObjectProvenance not yet hashed!");
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgSourceAspects::ObjectAspectIterator::Bind(uint64_t sourceId)
    {
    auto idstr = BaseAspect::FormatHexUInt64(sourceId);
    auto index = GetParameterIndex("idparm");
    m_stmt->Reset();
    m_stmt->BindText(index, idstr.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  DwgSourceAspects::DiskFileInfo::SetFrom (BeFileNameCR filename)
    {
    BeFileNameStatus status = BeFileName::GetFileSize(m_fileSize, filename.c_str());
    if (status == BeFileNameStatus::Success)
        {
        time_t mtime;
        status = BeFileName::GetFileTime(nullptr, nullptr, &mtime, filename.c_str());
        if (status == BeFileNameStatus::Success)
            m_lastModifiedTime = mtime;
        }
    return  static_cast<BentleyStatus>(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgSourceAspects::DwgFileInfo::SetFrom (DwgDbDatabaseCR dwg, DwgImporterR importer)
    {
    // set super info
    BeFileName  fullFilename(dwg.GetFileName().c_str());
    auto status = DiskFileInfo::SetFrom(fullFilename);

    // set DWG specific info
    m_versionGuid.Assign (dwg.GetVersionGuid().c_str());

    // get universial date as last edited time stamp
    DwgDbDate   tduupdate = dwg.GetTDUUPDATE ();
    m_lastSaveTime = tduupdate.GetJulianFraction ();

    m_dwgName.Assign (fullFilename.c_str());
    m_uniqueName = importer._GetUniqueNameForFile (fullFilename);
    m_idPolicy = importer.GetOptions().GetStableIdPolicy ();

    auto& rootDwg = importer.GetDwgDb ();
    m_rootRepositoryLinkId = DwgSourceAspects::GetRepositoryLinkId(rootDwg);

    return status;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSourceAspects::HasDiskFileChanged(BeFileNameCR fileName)
    {
    DwgSourceAspects::DiskFileInfo discInfo(fileName);
    auto prov = this->FindFileByFileName(fileName);
    if (!prov.IsValid())
        return true;

    DwgFileInfo dwgInfo;
    if (prov.GetDwgFileInfo(dwgInfo) != BSISUCCESS)
        return  true;

    // This is an attempt to tell if a file has *not* changed, looking only at the file's time and size.
    // This is a dangerous test, since we don't look at the contents, but we think that we can narrow 
    // the odds of a mistake by:
    // 1. using times measured in hectonanoseconds (on Windows, at least),
    // 2. also using file size
    return discInfo.IsEqual(dwgInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSourceAspects::HasLastSaveTimeChanged(DwgDbDatabaseCR dwg)
    {
    BeFileName  filename(dwg.GetFileName().c_str());

    // unique name as file identifier
    auto uniqueName = this->GetImporter()._GetUniqueNameForFile(filename);
    auto previous = RepositoryLinkAspect::FindByIdentifier(*m_dgndb, uniqueName);
    if (!previous.IsValid())
        return true;

    DwgFileInfo previousInfo;
    if (previous.GetDwgFileInfo(previousInfo) != BSISUCCESS)
        return  true;

    // last save time by TDUUPDATE is ideal, but a non-ACAD based product may not update it, so check both:
    double lastSaveTime = dwg.GetTDUUPDATE().GetJulianFraction();
    if (lastSaveTime != previousInfo.GetLastSaveTime())
        return  true;
    // TDUUPDATE unchanged - also check disk time stamp:
    return  this->HasDiskFileChanged(filename);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgSourceAspects::HasVersionGuidChanged (DwgDbDatabaseCR dwg)
    {
    BeFileName  filename(dwg.GetFileName().c_str());

    // unique name as file identifier
    auto uniqueName = this->GetImporter()._GetUniqueNameForFile(filename);
    auto previous = RepositoryLinkAspect::FindByIdentifier(*m_dgndb, uniqueName);
    if (!previous.IsValid())
        return true;

    DwgFileInfo previousInfo;
    if (previous.GetDwgFileInfo(previousInfo) != BSISUCCESS)
        return  true;

    Utf8String  versionGuid(dwg.GetVersionGuid().c_str());
    return 0 != versionGuid.CompareTo(previousInfo.GetVersionGuid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceId DwgSourceAspects::GetSoleAspectIdByKind (DgnElementCR el, Utf8CP kind)
    {
    auto sel = el.GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE (Element.Id=? AND Kind=?)");
    sel->BindId(1, el.GetElementId());
    sel->BindText(2, kind, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW != sel->Step())
        return BeSQLite::EC::ECInstanceId();
    auto id = sel->GetValueId<BeSQLite::EC::ECInstanceId>(0);
    BeAssert(BE_SQLITE_ROW != sel->Step());
    return id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::RepositoryLinkAspect  DwgSourceAspects::FindFileByFileName (BeFileNameCR path)
    {
    auto dirPrefix = m_importer.GetOptions().GetInputRootDir();
    size_t prefixLen = dirPrefix.size();
    if (!path.StartsWithI(dirPrefix.c_str()))
        prefixLen = 0;

    Utf8String searchName(path.substr(prefixLen));
    RepositoryLinkAspectIterator iter(*m_dgndb);

    for (auto aspect : iter)
        {
        if (aspect.GetFileName().EndsWithI(searchName.c_str()))
            return aspect;
        }
    return RepositoryLinkAspect();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryLinkId  DwgSourceAspects::GetRepositoryLinkId(DwgDbDatabaseR dwg)
    {
    uint64_t    linkId = 0;
    if (DwgDbStatus::Success != dwg.GetRepositoryLinkId(linkId))
        linkId = 0;
    return  RepositoryLinkId(linkId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::_GetUniqueNameForFile(WStringCR fullFileName) const
    {
    //  The unique name is the key into the syncinfo_file table. 
    //  Therefore, we must distinguish between like-named files in different directories.
    //  The unique name must also be stable. If the whole project is moved to a new directory or machine, 
    //  the unique names of the files must be unaffected.

    // If we have a DMS GUID for the document corresponding to this file, that is the unique name.
    BeGuid docGuid = this->GetOptions().QueryDocumentGuid(BeFileName(fullFileName));
    if (docGuid.IsValid())
        {
        Utf8String lguid = docGuid.ToString();
        lguid.ToLower();
        return lguid;
        }

    // If we do not have a GUID, we try to compute a stable unique name from the filename.
    // The full path should be unique already. To get something that is stable, we use only as much of 
    // the full path as we need to distinguish between like-named files in different directories.
    WString uniqueName(fullFileName);
    auto pdir = this->GetOptions().GetInputRootDir();
    if (!pdir.empty() && (pdir.size() < fullFileName.size()) && pdir.EqualsI(fullFileName.substr(0, pdir.size())))
        uniqueName = fullFileName.substr(pdir.size());

    uniqueName.ToLower();  // make sure we don't get fooled by case-changes in file system on Windows
    return Utf8String(uniqueName);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectAspect DwgSourceAspects::AddOrUpdateObjectAspect(DgnElementR element, ObjectAspect::SourceDataCR source)
    {
    if (!source.IsValid())
        return  ObjectAspect();

    auto aspect = DwgSourceAspects::ObjectAspect::GetForEdit(element, BaseAspect::FormatDbHandle(source.GetObjectHandle()));
    if (aspect.IsValid())
        {
        aspect.Update(source.GetProvenance());
        return aspect;
        }

    aspect = DwgSourceAspects::ObjectAspect::Create(source, *m_dgndb);
    if (aspect.AddAspect(element) != DgnDbStatus::Success)
        aspect.Invalidate();

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectAspect DwgSourceAspects::AddOrUpdateObjectAspect(DgnElementR element, DwgDbHandleCR handle, ObjectProvenanceCR provenance)
    {
    // use the pre-calculated provenance
    ObjectAspect::SourceData source(handle, element.GetModelId(), provenance, Utf8String());
    return  this->AddOrUpdateObjectAspect(element, source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectAspect DwgSourceAspects::AddOrUpdateObjectAspect(DgnElementR element, DwgDbObjectCR object)
    {
    // calculate provenance from input object
    ObjectAspect::SourceData source(object, element.GetModelId(), m_importer);
    return  this->AddOrUpdateObjectAspect(element, source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectAspect DwgSourceAspects::FindObjectAspect (DwgDbHandleCR objectHandle, DgnModelCR model, T_ObjectAspectFilter* filter)
    {
    struct ObjectAspectIteratorByHandle : ObjectAspectIterator 
        {
    private:
        void Bind(DwgDbHandleCR handle) 
            {
            auto index = this->GetParameterIndex("idparm");
            m_stmt->Reset();
            m_stmt->BindText(index, BaseAspect::FormatDbHandle(handle).c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
            }
    public:
        ObjectAspectIteratorByHandle(DgnModelCR model, DwgDbHandleCR handle) : ObjectAspectIterator(model, "Identifier=:idparm")
            {
            Bind(handle);
            }
        };  // ObjectAspectIteratorByHandle

    ObjectAspectIteratorByHandle    iter(model, objectHandle);
    auto found = iter.begin();
    // apply filters
    if (nullptr != filter)
        {
        while ((found != iter.end()) && !(*filter)(found, m_importer))
            ++found;
        }
    if (found != iter.end())
        return  *found;

    return ObjectAspect();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ObjectAspect DwgSourceAspects::FindObjectAspect (ObjectProvenanceCR prov, DgnModelCR model, T_ObjectAspectFilter* filter)
    {
    struct ObjectAspectIteratorByHash : ObjectAspectIterator 
        {
    private:
        void Bind(ObjectProvenanceCR prov)
            {
            Utf8String  hashstr;
            if (prov.GetHash().AsHexString(hashstr) == 0)
                BeAssert(false && "ObjectProvenance not yet hashed!");
            auto index = this->GetParameterIndex("checksumparm");
            m_stmt->Reset();
            m_stmt->BindText(index, hashstr.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
            }
    public:
        ObjectAspectIteratorByHash(DgnModelCR model, ObjectProvenanceCR prov) : ObjectAspectIterator(model, "Checksum=:checksumparm")
            {
            Bind(prov);
            }
        };  // ObjectAspectIteratorByHash

    ObjectAspectIteratorByHash  iter(model, prov);
    auto found = iter.begin();
    // apply filters
    if (nullptr != filter)
        {
        while ((found != iter.end()) && !(*filter)(found, m_importer))
            ++found;
        }
    if (found != iter.end())
        return  *found;

    return ObjectAspect();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ModelAspect DwgSourceAspects::AddModelAspect(DgnModelR model, DwgDbBlockTableRecordCR block, TransformCR trans, DwgDbBlockReferenceCP xrefInsert, DwgDbDatabaseP xrefDwg)
    {
    auto modelElement = model.GetModeledElement();
    if (!modelElement.IsValid())
        return  ModelAspect();

    // will create an aspect for either a modelspace/paperspace or an xRef insert
    bool isXref = nullptr != xrefInsert && nullptr != xrefDwg;

    auto aspect = isXref ? ModelAspect::Create(*xrefInsert, *xrefDwg, trans, m_importer) : ModelAspect::Create(block, trans, m_importer);
    if (aspect.IsValid())
        {
        auto writeElement = modelElement->CopyForEdit();
        if (writeElement.IsValid())
            {
            aspect.AddAspect(*writeElement);
            if (!writeElement->Update().IsValid())
                aspect.Invalidate ();
            }
        else
            {
            aspect.Invalidate ();
            }
        }

    return  aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ModelAspect DwgSourceAspects::AddModelAspect(DgnModelR model, DwgDbRasterImageCR raster, TransformCR trans)
    {
    auto modelElement = model.GetModeledElement();
    if (!modelElement.IsValid())
        return  ModelAspect();

    auto aspect = DwgSourceAspects::ModelAspect::Create(raster, trans, m_importer);
    if (aspect.IsValid())
        {
        auto writeElement = modelElement->CopyForEdit();
        if (writeElement.IsValid())
            {
            aspect.AddAspect (*writeElement);
            if (!writeElement->Update().IsValid())
                aspect.Invalidate ();
            }
        else
            {
            aspect.Invalidate ();
            }
        }

    return  aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ModelAspect DwgSourceAspects::FindModelAspect(DwgDbObjectIdCR id, DwgDbDatabaseR dwg, TransformCR trans)
    {
    auto rlinkId = m_importer.GetRepositoryLink(&dwg);
    auto scope = m_dgndb->Elements().Get<RepositoryLink>(rlinkId);
    if (!scope.IsValid())
        return  ModelAspect();

    auto idstr = BaseAspect::FormatObjectId(id);

    ModelAspectIterator iter(*scope, "Identifier=:identifier");
    iter.GetStatement()->BindText(iter.GetParameterIndex("Identifier"), idstr.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    for (auto entry : iter)
        {
        if (entry.GetTransform().IsEqual(trans))
            return  entry;
        }
    return ModelAspect();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::LayerAspect DwgSourceAspects::AddLayerAspect(DgnSubCategoryId subcatId, DwgDbObjectIdCR layerId) const
    {
    LayerAspect aspect;
    // ensure only one aspect per model
    DgnSubCategoryId    foundId;
    if (DwgSourceAspects::LayerAspect::FindFirstSubCategory(foundId, *m_dgndb, layerId, nullptr, m_importer) == BSISUCCESS)
        {
        auto existingSubcategory = DgnSubCategory::Get(*m_dgndb, foundId);
        if (existingSubcategory.IsValid())
            {
            auto rlinkId = m_importer.GetRepositoryLink (layerId.GetDatabase());
            aspect = LayerAspect::FindBy(*existingSubcategory, rlinkId);
            if (aspect.IsValid())
                return  aspect;
            }
        }

    // open the sub category element for write
    auto subcategory = m_dgndb->Elements().GetForEdit<DgnSubCategory>(subcatId);
    if (!subcategory.IsValid())
        return  aspect;

    // get the category by sub category ID
    auto catid = DgnSubCategory::QueryCategoryId(*m_dgndb, subcatId);
    auto category = DgnCategory::Get(*m_dgndb, catid);
    if (!category.IsValid())
        return  aspect;

    // Spatial or Drawing category
    LayerAspect::Type type = nullptr != dynamic_cast<SpatialCategory const*>(category.get()) ? LayerAspect::Type::Spatial : LayerAspect::Type::Drawing;

    // create a LayerAspect, add to the sub category, and update the element
    aspect = DwgSourceAspects::LayerAspect::Create(layerId, type, *m_dgndb, m_importer);
    aspect.AddAspect(*subcategory);
    subcategory->Update();

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId   DwgSourceAspects::FindCategory(DgnSubCategoryId* subcatIdOut, DwgDbObjectIdCR layerId) const
    {
    // find and return ID's for both category and sub category
    DgnCategoryId   catId;
    DgnSubCategoryId    subcatId;
    if (DwgSourceAspects::LayerAspect::FindFirstSubCategory(subcatId, *m_dgndb, layerId, nullptr, m_importer) == BSISUCCESS)
        {
        if (subcatIdOut != nullptr)
            *subcatIdOut = subcatId;
        catId = DgnSubCategory::QueryCategoryId(*m_dgndb, subcatId);
        }
    return  catId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbHandle DwgSourceAspects::FindLayerHandle(DgnCategoryId categoryId, DgnElementId scopeId) const
    {
    DwgDbHandle found;
    auto subCategory = DgnSubCategory::Get(*m_dgndb, DgnCategory::GetDefaultSubCategoryId(categoryId));
    if (!subCategory.IsValid())
        return  found;

    auto aspect = DwgSourceAspects::LayerAspect::FindBy(*subCategory, scopeId);
    if (aspect.IsValid())
        found = aspect.GetLayerHandle ();
    return  found;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ViewAspect DwgSourceAspects::AddOrUpdateViewAspect(ViewDefinitionR view, DwgDbObjectIdCR sourceId, ViewAspect::SourceType sourceType, Utf8StringCR viewName) const
    {
    if (!sourceId.IsValid())
        return  ViewAspect();

    auto aspect = DwgSourceAspects::ViewAspect::GetForEdit(view);
    if (aspect.IsValid())
        {
        aspect.Update(sourceType, viewName);
        return aspect;
        }

    // scope all ViewAspects per root master file
    auto scopeId = m_importer.GetRepositoryLink(&m_importer.GetDwgDb());
    if (scopeId.IsValid())
        {
        aspect = DwgSourceAspects::ViewAspect::Create(scopeId, sourceId, sourceType, viewName, *m_dgndb);
        if (aspect.AddAspect(view) != DgnDbStatus::Success)
            aspect.Invalidate();
        }

    if (aspect.IsValid())
        view.Update();

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId DwgSourceAspects::FindView (DwgDbObjectIdCR sourceId, ViewAspect::SourceType sourceType) const
    {
    DgnViewId   viewId;

    // scope all ViewAspects per root master file
    auto scopeId = m_importer.GetRepositoryLink(&m_importer.GetDwgDb());
    if (!scopeId.IsValid())
        return  viewId;

    auto idstr = BaseAspect::FormatObjectId(sourceId);

    ViewAspectIterator iter(*m_dgndb, scopeId, "Identifier=:identifier");
    iter.GetStatement()->BindText(iter.GetParameterIndex("Identifier"), idstr.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    for (auto entry : iter)
        {
        if (entry.GetSourceType() == sourceType)
            {
            viewId = entry.GetViewId();
            break;
            }
        }
    return viewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbHandle DwgSourceAspects::FindViewportHandle(DgnViewId viewId) const
    {
    DwgDbHandle found;
    auto view = ViewDefinition::Get(*m_dgndb, viewId);
    if (!view.IsValid())
        return  found;
    auto aspect = DwgSourceAspects::ViewAspect::Get(*view);
    if (aspect.IsValid())
        found = aspect.GetSourceHandle();
    return  found;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::LinetypeAspect DwgSourceAspects::AddOrUpdateLinetypeAspect(DgnStyleId styleId, DwgDbObjectIdCR ltypeId, Utf8StringCR ltypeName)
    {
    auto dwg = ltypeId.GetDatabase();
    if (dwg == nullptr)
        return  LinetypeAspect();

    auto linestyle = LineStyleElement::GetForEdit(*m_dgndb, styleId);
    if (!linestyle.IsValid())
        return  LinetypeAspect();

    auto aspect = DwgSourceAspects::LinetypeAspect::GetForEdit(*linestyle);
    if (aspect.IsValid())
        {
        aspect.Update(ltypeName);
        return aspect;
        }

    // scope LinetypeAspect per DWG file
    auto scopeId = m_importer.GetRepositoryLink(dwg);
    if (scopeId.IsValid())
        {
        aspect = DwgSourceAspects::LinetypeAspect::Create(scopeId, ltypeId, ltypeName, *m_dgndb);
        if (aspect.AddAspect(*linestyle) != DgnDbStatus::Success)
            aspect.Invalidate();
        }

    if (aspect.IsValid())
        linestyle->Update();

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyleId DwgSourceAspects::FindLineStyle(DwgDbObjectIdCR ltypeId) const
    {
    auto dwg = ltypeId.GetDatabase();
    if (dwg == nullptr)
        return  DgnStyleId();

    // scope LinetypeAspect per DWG file
    auto scopeId = m_importer.GetRepositoryLink(dwg);
    auto idstr = BaseAspect::FormatObjectId(ltypeId);
    auto aspect = iModelExternalSourceAspect::GetAspectBySourceId(m_importer.GetDgnDb(), scopeId, BaseAspect::Kind::Linetype, idstr);
    if (aspect.IsValid())
        {
        LinetypeAspectP lstyleAspect = static_cast<LinetypeAspectP>(&aspect);
        if (lstyleAspect != nullptr)
            return  lstyleAspect->GetLinestyleId();
        }
    return  DgnStyleId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::MaterialAspect DwgSourceAspects::AddOrUpdateMaterialAspect(RenderMaterialId materialId, DwgDbMaterialCR dwgMaterial)
    {
    Utf8String  materialName(dwgMaterial.GetName().c_str());
    auto dgnMaterial = m_dgndb->Elements().GetForEdit<RenderMaterial>(materialId);
    if (!dgnMaterial.IsValid())
        return  MaterialAspect();
    
    auto aspect = DwgSourceAspects::MaterialAspect::GetForEdit(*dgnMaterial);
    if (aspect.IsValid())
        {
        // update existing aspect from input material
        auto prov = DwgSourceAspects::MaterialAspect::CreateProvenance(dwgMaterial, m_importer);
        if (!prov.IsNull())
            aspect.Update(prov, materialName);
        return aspect;
        }

    aspect = DwgSourceAspects::MaterialAspect::Create(dwgMaterial, m_importer);
    if (aspect.AddAspect(*dgnMaterial) != DgnDbStatus::Success)
        aspect.Invalidate();

    if (aspect.IsValid())
        dgnMaterial->Update();

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::MaterialAspect DwgSourceAspects::FindMaterialAspect(DwgDbObjectIdCR materialId) const
    {
    // scope all MaterialAspects per root master file
    auto scopeId = m_importer.GetRepositoryLink(&m_importer.GetDwgDb());
    auto idstr = BaseAspect::FormatObjectId(materialId);
    auto aspect = iModelExternalSourceAspect::GetAspectBySourceId(*m_dgndb, scopeId, BaseAspect::Kind::Material, idstr);
    if (aspect.IsValid())
        {
        MaterialAspectP materialAspect = static_cast<MaterialAspectP>(&aspect);
        if (materialAspect != nullptr)
            return  *materialAspect;
        }
    return  MaterialAspect();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::GroupAspect DwgSourceAspects::AddOrUpdateGroupAspect(DgnElementR element, DwgDbGroupCR group)
    {
    Utf8String  groupName(group.GetName().c_str());
    auto aspect = DwgSourceAspects::GroupAspect::GetForEdit(element);
    if (aspect.IsValid())
        {
        // update existing aspect from input group
        auto prov = DwgSourceAspects::GroupAspect::CreateProvenance(group, m_importer);
        if (!prov.IsNull())
            aspect.Update(prov, groupName);
        return aspect;
        }

    // create a new aspect for the group
    aspect = DwgSourceAspects::GroupAspect::Create(group, m_importer);
    if (aspect.AddAspect(element) != DgnDbStatus::Success)
        aspect.Invalidate();

    if (aspect.IsValid())
        element.Update();

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::GroupAspect DwgSourceAspects::FindGroupAspect(DwgDbObjectIdCR groupId) const
    {
    // scope GroupAspects per file
    auto dwg = groupId.GetDatabase();
    if (dwg == nullptr)
        return  GroupAspect();

    auto scopeId = m_importer.GetRepositoryLink(dwg);
    auto idstr = BaseAspect::FormatObjectId(groupId);
    auto aspect = iModelExternalSourceAspect::GetAspectBySourceId(*m_dgndb, scopeId, BaseAspect::Kind::Group, idstr);
    if (aspect.IsValid())
        {
        GroupAspectP groupAspect = static_cast<GroupAspectP>(&aspect);
        if (groupAspect != nullptr)
            return  *groupAspect;
        }
    return  GroupAspect();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
T_ECInstanceIdArray DwgSourceAspects::GetExternalSourceAspectIds(DgnElementCR element, Utf8CP kind, Utf8StringCR sourceId)
    {
    auto sql = element.GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE (Element.Id=? AND Kind=? AND Identifier=?)");
    sql->BindId(1, element.GetElementId());
    sql->BindText(2, kind, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    sql->BindText(3, sourceId.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);

    T_ECInstanceIdArray idArray;
    while (BE_SQLITE_ROW == sql->Step())
        idArray.push_back(sql->GetValueId<BeSQLite::EC::ECInstanceId>(0));
    return idArray;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet DwgSourceAspects::FindElementsBy(Utf8CP kind, DwgDbHandleCR sourceHandle) const
    {
    auto idstr = BaseAspect::FormatDbHandle(sourceHandle);
    DgnElementIdSet idset;

    auto sql = m_dgndb->GetPreparedECSqlStatement("SELECT Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE (Kind=? AND Identifier=?)");
    if (sql != nullptr)
        {
        sql->BindText(1, kind, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        sql->BindText(2, idstr.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        while (BE_SQLITE_ROW == sql->Step())
            {
            auto elementId = sql->GetValueId<DgnElementId>(0);
            idset.insert(elementId);
            }
        }
    return idset;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet DwgSourceAspects::FindElementsBy(DwgDbHandleCR sourceHandle) const
    {
    return  this->FindElementsBy(DwgSourceAspects::BaseAspect::Kind::Element, sourceHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgSourceAspects::Initialize (DgnDbR db)
    {
    m_dgndb = &db;
    return (db.IsDbOpen() && !db.IsReadonly()) ? BentleyStatus::BSISUCCESS : BentleyStatus::BSIERROR;
    }

END_DWG_NAMESPACE

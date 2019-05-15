/*--------------------------------------------------------------------------------------+
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Dwg/DwgImporter.h>

#include <DgnPlatform/DgnPlatform.h>
#include <iModelBridge/iModelBridgeSyncInfoFile.h>
#include <Bentley/MD5.h>
#include <iterator>                         // std::iterator, std::input_iterator_tag

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_DWGDB

BEGIN_DWG_NAMESPACE

typedef bvector<BeSQLite::EC::ECInstanceId> T_ECInstanceIdArray;

struct DwgImporter;


//=======================================================================================
// @bsiclass                                                    Sam.Wilson   09/14
//=======================================================================================
enum class StableIdPolicy : bool
    {
    ById = 0,
    ByHash = 1
    };

/*=================================================================================**//**
* DwgSourceAspects to track imported DWG data which may be optionally used for update.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DwgSourceAspects
    {
public:
    // Base ExternalSourceAspect for DWG
    struct BaseAspect : iModelExternalSourceAspect
        {
    protected:
        BaseAspect(ECN::IECInstance* instance) : iModelExternalSourceAspect(instance) {}
        //! Valid only for source kinds using unique handles as identifiers
        DwgDbHandle GetIdentifierAsHandle() const;
    public:
        struct Kind 
            {
            static constexpr Utf8CP Element = "Element";
            static constexpr Utf8CP Model = "Model";
            static constexpr Utf8CP Layer = "Layer";
            static constexpr Utf8CP Linetype = "LineType";
            static constexpr Utf8CP Material = "Material";
            static constexpr Utf8CP Group = "Group";
            static constexpr Utf8CP ViewDefinition = "ViewDefinition";
            static constexpr Utf8CP RepositoryLink = "RepositoryLink";
            static constexpr Utf8CP URI = "URI";
            static constexpr Utf8CP GeometryPart = "GeometryPart";
            };  // Kind
        static Utf8String FormatDbHandle(DwgDbHandleCR hd) { return "0x"+Utf8String(hd.AsAscii().c_str()); }
        static Utf8String FormatObjectId(DwgDbObjectIdCR id) { return "0x"+Utf8String(id.ToAscii().c_str()); }
        static Utf8String FormatUInt64(uint64_t i) { Utf8Char str[32]; BeStringUtilities::FormatUInt64(str, i); return str; }
        static DwgDbHandle ParseDbHandle(Utf8StringCR h) { return DwgDbHandle(BeStringUtilities::ParseHex(h.c_str())); }
        };  // BaseAspect

    //! Information about a file on disk. This struct captures the information that can be extracted from the dwg disk file itself.
    struct DiskFileInfo
        {
        uint64_t    m_lastModifiedTime; // (Unix time in seconds)
        uint64_t    m_fileSize;

        DiskFileInfo() {m_lastModifiedTime = m_fileSize = 0;}
        DiskFileInfo(BeFileNameCR filename) { SetFrom(filename); }

        BentleyStatus SetFrom(BeFileNameCR);
        void SetLastModifiedTime(uint64_t t) { m_lastModifiedTime = t; }
        void SetFileSize(uint64_t s) { m_fileSize = s; }
        uint64_t GetLastModifiedTime() const { return m_lastModifiedTime; }
        uint64_t GetFileSize() const { return m_fileSize; }
        bool IsEqual(DiskFileInfo const& o) {return m_lastModifiedTime == o.m_lastModifiedTime && m_fileSize == o.m_fileSize;}
        };  // DiskFileInfo
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(DiskFileInfo)

    //! Information about a file. This struct captures the information that can be extracted from the dwg file itself.
    struct DwgFileInfo : DiskFileInfo
        {
        Utf8String      m_uniqueName;
        Utf8String      m_dwgName;
        Utf8String      m_versionGuid;
        double          m_lastSaveTime;
        StableIdPolicy  m_idPolicy;

        DwgFileInfo() : m_lastSaveTime(0.0), m_idPolicy(StableIdPolicy::ById) {}
        DwgFileInfo(DwgDbDatabaseCR dwg, DwgImporter const& importer) { SetFrom(dwg, importer); }

        BentleyStatus SetFrom(DwgDbDatabaseCR, DwgImporter const&);
        void SetUniqueName(Utf8CP name) { m_uniqueName.assign(name); }
        void SetDwgName(Utf8CP name) { m_dwgName.assign(name); }
        void SetVersionGuid(Utf8CP guid) { m_versionGuid.assign(guid); }
        void SetLastSaveTime(double t) { m_lastSaveTime = t; }
        void SetIdPolicy(StableIdPolicy policy) { m_idPolicy = policy; }
        Utf8StringCR GetUniqueName() const { return m_uniqueName; }
        Utf8StringCR GetDwgName() const { return m_dwgName; }
        Utf8StringCR GetVersionGuid() const { return m_versionGuid; }
        double GetLastSaveTime () const { return m_lastSaveTime; }
        StableIdPolicy GetIdPolicy() const { return m_idPolicy; }
        };  // DwgFileInfo
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(DwgFileInfo)

    //! Information about a path that might be a disk file or might be a URL
    struct UriContentInfo : DiskFileInfo
        {
        Utf8String m_eTag;    // Set if path is a URI
        
        BentleyStatus SetFrom(Utf8StringCR pathOrUrl);
        bool IsEqual(UriContentInfo const& o) {return DiskFileInfo::IsEqual(o) && m_eTag == o.m_eTag;}
        };
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(UriContentInfo)

    //! Aspect stored on a RepositoryLink to capture additional info about the source file
    struct RepositoryLinkAspect : BaseAspect
        {
    private:
        RepositoryLinkAspect(ECN::IECInstance* instance) : BaseAspect(instance) {} 
    public:
        RepositoryLinkAspect() : BaseAspect(nullptr) {} 

        DWG_EXPORT static RepositoryLinkAspect Create(DgnDbR, DwgFileInfo const&, StableIdPolicy);
        DWG_EXPORT void Update(DwgFileInfoCR);

        DWG_EXPORT static RepositoryLinkAspect FindByIdentifier(DgnDbR, Utf8StringCR identifier);
        DWG_EXPORT static RepositoryLinkAspect GetForEdit(RepositoryLinkR);
        DWG_EXPORT static RepositoryLinkAspect Get(RepositoryLinkCR);

        RepositoryLinkId GetRepositoryLinkId() const {return RepositoryLinkId(GetElementId().GetValueUnchecked());}
        DWG_EXPORT Utf8String GetFileName() const;
        DWG_EXPORT StableIdPolicy GetStableIdPolicy() const;
        DWG_EXPORT BentleyStatus GetDwgFileInfo(DwgFileInfoR) const;
        DWG_EXPORT Utf8String GetUniqueName() const;
        DWG_EXPORT Utf8String GetDwgName() const;
        };  // RepositoryLinkAspect
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(RepositoryLinkAspect)

    struct RepositoryLinkAspectIterator : ExternalSourceAspectIterator<RepositoryLinkAspect>
        {
        RepositoryLinkAspectIterator(DgnDbR db, Utf8CP wh = nullptr) : ExternalSourceAspectIterator(db, db.Elements().GetRootSubjectId(), BaseAspect::Kind::RepositoryLink, wh) {}
        };

    //! Aspect that tracks the content of a URI. Will also work with a filepath.
    //! This is a lot like RepositoryLinkAspect. We make it separate just to keep things clear, as apps tend to think of RepostoryLinks as
    //! corresponding directly to documents in ProjectWise or another DMS. In the case of PW, the document is expected to have document properties, RBAC, etc. 
    //! UriExternalSourceAspect is for the general/generic case of sourcing data from a URI with nothing implied about what the resource is or if it has metadata or not.
    struct UriAspect : BaseAspect
        {
    private:
        UriAspect(ECN::IECInstance* instance) : BaseAspect(instance) {}
    public:
        static UriAspect Create(DgnElementId, Utf8CP, UriContentInfo const&, Utf8CP, DgnDbR db);
        static UriAspect Get(DgnElementCR);
        static UriAspect Get(DgnElementId eid, DgnDbR db);
        static UriAspect GetForEdit(DgnElementR);

        Utf8String GetFilenameOrUrl() const {return GetIdentifier();}
        Utf8String GetSourceGuid() const;
        void SetSourceGuid(Utf8StringCR rdsId);
        void GetInfo(UriContentInfoR out) const;
        void SetInfo(UriContentInfoCR in);
        };  // UriAspect
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(UriAspect)

    //! Aspect about the content of a source model
    struct ModelAspect : BaseAspect
        {
        friend struct ModelAspectIterator;
    private:
        ModelAspect(ECN::IECInstance* instance) : BaseAspect(instance) {}
        void AddProperties (Utf8StringCR dwgName, TransformCR trans, Utf8CP sourceType);
        //! Create an empty model aspect ready to be added with model specific properties
        static ModelAspect CreateStub(DwgDbObjectIdCR, DwgDbDatabaseP, TransformCR, DwgImporter&);
    public:
        //! DWG Model source type
        enum class SourceType
            {
            Unknown             = 0,    // Unknown/unsupported
            ModelSpace          = 1,    // modelspace
            PaperSpace          = 2,    // paperspace
            XRefAttachment      = 3,    // xRef insert
            RasterAttachment    = 4,    // raster image
            };  // SourceType
        ModelAspect() : BaseAspect(nullptr) {}

        //! Methods to create a new aspect in memory. Caller must call AddAspect, passing in the model element that is to have this aspect.
        //! Create an aspect from a Modelspace or Paperspace block
        DWG_EXPORT static ModelAspect Create(DwgDbBlockTableRecordCR, TransformCR, DwgImporter&);
        //! Create an aspect for an Xreference entity
        DWG_EXPORT static ModelAspect Create(DwgDbBlockReferenceCR xinsert, DwgDbDatabaseR xref, TransformCR, DwgImporter&);
        //! Create an aspect from a raster attachment
        DWG_EXPORT static ModelAspect Create(DwgDbRasterImageCR, TransformCR, DwgImporter&);
        DWG_EXPORT static std::tuple<DgnElementCPtr, ModelAspect> Get(DgnModelCR);
        DWG_EXPORT static std::tuple<DgnElementPtr, ModelAspect> GetForEdit(DgnModelR);
        DWG_EXPORT static ModelAspect GetByAspectId(DgnDbR, BeSQLite::EC::ECInstanceId);

        RepositoryLinkId GetRepositoryLinkId() const {return RepositoryLinkId(GetScope().GetValueUnchecked());}
        DgnModelId GetModelId() const {return DgnModelId(GetElementId().GetValueUnchecked());}
        //! Query the source object handle of a block, an xrefInsert or a raster attachment
        DwgDbHandle GetDwgModelHandle() const { return BaseAspect::GetIdentifierAsHandle(); }
        DWG_EXPORT Utf8String GetDwgModelName() const;
        DWG_EXPORT void SetDwgModelName(Utf8StringCR);
        DWG_EXPORT Transform GetTransform() const;
        DWG_EXPORT void SetTransform(TransformCR);
        DWG_EXPORT SourceType GetSourceType() const;
        };  // ModelAspect
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ModelAspect)

    struct ModelAspectIterator : ExternalSourceAspectIterator<ModelAspect>
        {
        ModelAspectIterator(DgnDbR db, DgnElementId scopeId, Utf8CP where=nullptr) : ExternalSourceAspectIterator(db, scopeId, BaseAspect::Kind::Model, where) {}
        ModelAspectIterator(RepositoryLinkCR scope, Utf8CP where=nullptr) : ExternalSourceAspectIterator(scope.GetDgnDb(), scope.GetElementId(), BaseAspect::Kind::Model, where) {}
        };  // ModelAspectIterator
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ModelAspectIterator)

    //! Information about a layer.  A LayerAspect is scoped per source file, i.e. a lyer in an xRef should have its ID from that xRef, not the one in the master file.
    struct LayerAspect : BaseAspect
        {
        enum class Type {Spatial, Drawing}; // WARNING: Persistent values - do not change
    private:
        LayerAspect(ECN::IECInstance* instance) : BaseAspect(instance) {}
    public:
        LayerAspect() : BaseAspect(nullptr) {}

        //! Create a new aspect in memory. The scope will be the RepositoryLink element that stands for the source file, not the root file. Caller must call AddAspect, passing in the Category element.
        DWG_EXPORT static LayerAspect Create(DwgDbObjectIdCR layerId, Type, DgnDbR, DwgImporter&);
        //! Find a LayerAspect by a sub-category and a RespositoryLink ID
        DWG_EXPORT static LayerAspect FindBy(DgnSubCategoryCR subcat, DgnElementId scope);
        //! Find a sub-category from a source layer ID, and an optional category type(physical or drawing)
        DWG_EXPORT static BentleyStatus FindFirstSubCategory(DgnSubCategoryId& out, DgnDbR, DwgDbObjectIdCR layerId, Type const* type, DwgImporter&);
        DgnSubCategoryId GetSubCategoryId () const { return DgnSubCategoryId(GetElementId().GetValue()); }
        DgnCategoryId GetCategoryId (DgnDbR db) const { return DgnSubCategory::QueryCategoryId(db, GetSubCategoryId()); }
        DwgDbHandle GetLayerHandle() const { return BaseAspect::GetIdentifierAsHandle(); }
        DWG_EXPORT Utf8String GetLayerName() const;
        };
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(LayerAspect)

    struct LayerAspectIterator : ExternalSourceAspectIterator<LayerAspect>
        {
        LayerAspectIterator(DgnDbR db, DgnElementId scopeId, Utf8CP wh="") : ExternalSourceAspectIterator(db, scopeId, BaseAspect::Kind::Layer, wh) {}
        };
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(LayerAspectIterator)

    //! Information about a view, created from a viewport table record, a layout viewport, or a viewport entity.  All ViewAspects are scoped per root file.
    struct ViewAspect : BaseAspect
        {
        enum class SourceType
            {
            ModelSpaceViewport,     // vport table record
            PaperSpaceViewport,     // the "overall" layout viewport
            ViewportEntity,         // viewport entity in a layout
            XRefAttachment,         // xRef inserted in a layout
            };  // SourceType
    private:
        ViewAspect(ECN::IECInstance* instance) : BaseAspect(instance) {}
    public:
        ViewAspect() : BaseAspect(nullptr) {}

        //! Create a new aspect in memory. The scope will be the RepositoryLink element that stands for the source file. Caller must call AddAspect, passing in the Category element.
        DWG_EXPORT static ViewAspect Create(DgnElementId scopeId, DwgDbObjectIdCR sourceId, SourceType, Utf8StringCR name, DgnDbR db);
        DWG_EXPORT static ViewAspect GetForEdit(ViewDefinitionR);
        DWG_EXPORT static ViewAspect Get(ViewDefinitionCR);
        DWG_EXPORT static std::tuple<ViewAspect,DgnViewId> Get(DgnElementId scope, DwgDbObjectIdCR sourceId, DgnDbR db);
        DWG_EXPORT void Update(SourceType, Utf8StringCR viewName); 
        DgnViewId GetViewId() const { return DgnViewId(GetElementId().GetValueUnchecked()); }
        DWG_EXPORT SourceType GetSourceType() const;
        DwgDbHandle GetSourceHandle() const { return BaseAspect::GetIdentifierAsHandle(); }
        };
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ViewAspect)

    struct ViewAspectIterator : ExternalSourceAspectIterator<ViewAspect>
        {
        ViewAspectIterator(DgnDbR db, DgnElementId scope, Utf8CP where="") : ExternalSourceAspectIterator(db, scope, BaseAspect::Kind::ViewDefinition, where) {}
        };
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ViewAspectIterator)

    //! Information about a linetype
    struct LinetypeAspect : BaseAspect
        {
    private:
        LinetypeAspect(ECN::IECInstance* instance) : BaseAspect(instance) {}
    public:
        LinetypeAspect() : BaseAspect(nullptr) {}
        DWG_EXPORT static LinetypeAspect Create(DgnElementId scopeId, DwgDbObjectIdCR ltypeId, Utf8StringCR name, DgnDbR db);
        DWG_EXPORT static LinetypeAspect GetForEdit(LineStyleElementR);
        DWG_EXPORT static LinetypeAspect Get(LineStyleElementCR);
        DWG_EXPORT void Update(Utf8StringCR name); 
        DgnStyleId GetLinestyleId() const { return DgnStyleId(GetElementId().GetValueUnchecked()); }
        DwgDbHandle GetLinetypeHandle() const { return BaseAspect::GetIdentifierAsHandle(); }
        };  // LinetypeAspect
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(LinetypeAspect)

    //! A generic object provenance
    struct ObjectProvenance
        {
    public:
        struct Hash : BentleyApi::MD5::HashVal
            {
            Hash() {::memset(m_buffer, 0, sizeof BentleyApi::MD5::BlockSize);}
            Hash(BentleyApi::MD5::HashVal const& from) : BentleyApi::MD5::HashVal(from) {}
            bool IsSame(Hash const& other) const {return 0==memcmp(m_buffer, other.m_buffer, sizeof(m_buffer));}
            bool IsNull() const;
            size_t AsHexString(Utf8StringR outString) const;
            };  // Hash
        DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(Hash)

    private:
        Hash    m_hash;
        BentleyApi::MD5 m_hasher;

    public:
        //! Constructor from a pre-calculated hash. AddHash calls will have no effect on the hash constructed this way.
        ObjectProvenance(HashCR hash) : m_hash(hash) {}
        //! Constructor to calculate hash from a DWG specific object.  May add more data via AddHash calls.
        DWG_EXPORT ObjectProvenance(DwgDbObjectCR, DwgImporter& importer);
        //! Default constructor ready to add new hash.  May add data via AddHash calls.
        ObjectProvenance() {}
        //! Directly hash input generic data and add it to to existing hash
        DWG_EXPORT void AddHash(const void* binaryData, size_t numBytes);
        //! Calculate hash from input DWG object and add it to existing hash
        DWG_EXPORT void AddHash(DwgDbObjectCR object);
        bool IsNull() const { return m_hash.IsNull(); }
        bool IsSame(ObjectProvenance const& other) const { return m_hash.IsSame(other.m_hash); }
        HashCR GetHash() const { return m_hash; }
        void SetHash(HashCR hash) { m_hash = hash; }
        };  // ObjectProvenance
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ObjectProvenance)

    //! Information about a generic object
    struct ObjectAspect : BaseAspect
        {
    public:
        //! Source data to persist in ObjectAspect
        struct SourceData
            {
        protected:
            DwgDbHandle m_objectHandle;
            DgnModelId  m_modelId;
            Utf8String  m_jsonProperties;
            ObjectProvenance m_provenance;

        public:
            //! Constructor from pre-calculated source data from a generic source object
            SourceData(DwgDbHandleCR h, DgnModelId m, ObjectProvenanceCR v, Utf8StringCR j) : m_objectHandle(h), m_modelId(m), m_provenance(v), m_jsonProperties(j) {}
            //! Constructor to build provenance from a DWG specific source object
            DWG_EXPORT SourceData(DwgDbObjectCR, DgnModelId, DwgImporter&);
            // the default constructor
            SourceData() {}
            bool IsValid() const { return m_modelId.IsValid() && !m_objectHandle.IsNull(); }
            DwgDbHandle GetObjectHandle() const { return m_objectHandle; }
            void SetObjectHandle(DwgDbHandleCR handle) { m_objectHandle = handle; }
            DgnModelId GetModelId() const { return m_modelId; }
            void SetModelId(DgnModelId id) { m_modelId = id; }
            Utf8StringCR GetJsonProperties() const { return m_jsonProperties; }
            void SetJsonProperties(Utf8StringCR str) { m_jsonProperties = str; }
            ObjectProvenanceCR GetProvenance() const { return m_provenance; }
            void SetProvenance (ObjectProvenanceCR prov) { m_provenance = prov; }
            };  // SourceData
        DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(SourceData)

    protected:
        ObjectAspect(ECN::IECInstance* instance) : BaseAspect(instance) {}
    public:
        ObjectAspect() : BaseAspect(nullptr) {}

        //! Create a new aspect in memory. Caller must call AddAspect, passing in the element that is to have this aspect.
        DWG_EXPORT static ObjectAspect Create(SourceDataCR, DgnDbR);
        DWG_EXPORT static ObjectAspect GetForEdit(DgnElementR, Utf8StringCR idstr);
        DWG_EXPORT static ObjectAspect GetForEdit(DgnElementR, DwgDbObjectIdCR id);
        DWG_EXPORT static ObjectAspect Get(DgnElementCR, Utf8StringCR idstr);
        DWG_EXPORT static ObjectAspect Get(DgnElementCR el, DwgDbObjectIdCR id);
        DWG_EXPORT static ObjectAspect Get(DgnElementCR el, BeSQLite::EC::ECInstanceId aspectId);
        //! The scope of an ObjectAspect is a model (element). This convenience method returns the scope as a DgnModelId
        DWG_EXPORT DgnModelId GetModelId() const;
        //! The ID of the source object
        DwgDbHandle GetObjectHandle() const { return BaseAspect::GetIdentifierAsHandle(); }
        DWG_EXPORT void Update(ObjectProvenanceCR); 
        DWG_EXPORT Utf8String GetHashString() const;
        DWG_EXPORT bool IsProvenanceEqual(ObjectProvenanceCR) const;
        };
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ObjectAspect)

    struct ObjectAspectIterator : ExternalSourceAspectIterator<ObjectAspect>
        {
        ObjectAspectIterator(DgnModelCR model, Utf8CP where=nullptr) : ExternalSourceAspectIterator(model.GetDgnDb(), model.GetModeledElementId(), BaseAspect::Kind::Element, where) {}
        };  // ObjectAspectIterator
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ObjectAspectIterator)

    //! Used to choose one of many existing entries in DwgSourceAspects
    typedef std::function<bool(ObjectAspectIterator::Entry const&, DwgImporter& importer)> T_ObjectAspectFilter;

    //! Information about a material
    struct MaterialAspect : BaseAspect
        {
    private:
        MaterialAspect(ECN::IECInstance* instance) : BaseAspect(instance) {}
    public:
        MaterialAspect() : BaseAspect(nullptr) {}
        DWG_EXPORT static MaterialAspect Create(DwgDbMaterialCR material, DwgImporter& importer);
        DWG_EXPORT static MaterialAspect GetForEdit(RenderMaterialR);
        DWG_EXPORT static MaterialAspect Get(RenderMaterialCR);
        DWG_EXPORT static ObjectProvenance CreateProvenance(DwgDbMaterialCR material, DwgImporter& importer);
        DWG_EXPORT void Update(DwgSourceAspects::ObjectProvenanceCR prov, Utf8StringCR name);
        DWG_EXPORT bool IsSameMaterial(ObjectProvenanceCR, Utf8StringCR) const;
        RenderMaterialId GetRenderMaterialId () const { return RenderMaterialId(GetElementId().GetValueUnchecked()); }
        DwgDbHandle GetMaterialHandle() const { return BaseAspect::GetIdentifierAsHandle(); }
        };  // MaterialAspect
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(MaterialAspect)

    struct MaterialAspectIterator : ExternalSourceAspectIterator<MaterialAspect>
        {
        MaterialAspectIterator(DgnDbR db, DgnElementId scope, Utf8CP where=nullptr) : ExternalSourceAspectIterator(db, scope, BaseAspect::Kind::Material, where) {}
        };  // MaterialAspectIterator
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(MaterialAspectIterator)

    //! Information about a group
    struct GroupAspect : ObjectAspect
        {
    private:
        GroupAspect(ECN::IECInstance* instance) : ObjectAspect(instance) {}
    public:
        GroupAspect() : ObjectAspect(nullptr) {}
        DWG_EXPORT static GroupAspect Create(DwgDbGroupCR group, DwgImporter&);
        DWG_EXPORT static GroupAspect GetForEdit(DgnElementR);
        DWG_EXPORT static GroupAspect Get(DgnElementCR);
        DWG_EXPORT static ObjectProvenance CreateProvenance(DwgDbGroupCR group, DwgImporter& importer);
        DWG_EXPORT void Update(DwgSourceAspects::ObjectProvenanceCR prov, Utf8StringCR name);
        DWG_EXPORT bool IsSameGroup(ObjectProvenanceCR, Utf8StringCR) const;
        DwgDbHandle GetGroupHandle() const { return BaseAspect::GetIdentifierAsHandle(); }
        };  // GroupAspect
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(GroupAspect)

    struct GroupAspectIterator : ExternalSourceAspectIterator<GroupAspect>
        {
        GroupAspectIterator(DgnModelCR model, Utf8CP where=nullptr) : ExternalSourceAspectIterator(model.GetDgnDb(), model.GetModeledElementId(), BaseAspect::Kind::Element, where) {}
        };  // GroupAspectIterator
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(GroupAspectIterator)

    //! GeomPart to track & retrieve parts created as shared geometry in db
    struct GeomPartAspect : BaseAspect
        {
    private:
        GeomPartAspect(ECN::IECInstance* instance) : BaseAspect(instance) {}
    public:
        //! Create a new aspect in memory. scopeId should be the bridge's job definition model. Caller must call AddAspect, passing in the DgnGeometryPart element.
        DWG_EXPORT static GeomPartAspect Create(DgnElementId scopeId, Utf8StringCR tag, DgnDbR);
        //! Get an existing GeomPart aspect from the specified DgnGeometryPart
        DWG_EXPORT static GeomPartAspect Get(DgnGeometryPartCR geomPart);
        //! Look up the element that has the GeomPart aspect with the specified tag. Note that this is based on the assumption that GeometryPart "tags" are unique within the specified scope!
        DWG_EXPORT static DgnGeometryPartId FindGeometryPart(DgnElementId scopeId, Utf8StringCR tag, DgnDbR);
        };  // GeomPartAspect

    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(GeomPartAspect)

    struct GeomPartAspectIterator : ExternalSourceAspectIterator<GeomPartAspect>
        {
        GeomPartAspectIterator(DgnModelCR model, Utf8CP where=nullptr) : ExternalSourceAspectIterator(model.GetDgnDb(), model.GetModeledElementId(), BaseAspect::Kind::Element, where) {}
        };  // GeomPartAspectIterator
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(GeomPartAspectIterator)


private:
    DwgImporter&        m_importer;
    DgnDbP              m_dgndb;

protected:
    bool ComputeHash(BentleyApi::MD5&, DwgDbObjectIdCR, DwgDbObjectCP);
    BentleyStatus CreateTables();

public:
    DwgSourceAspects(DwgImporter& importer) : m_importer(importer), m_dgndb(nullptr) {}
    ~DwgSourceAspects() {}
    DwgImporter& GetImporter() { return m_importer; }
    DgnDbP  GetDgnDb() {return m_dgndb;}
    //! Set the project before use
    DWG_EXPORT BentleyStatus Initialize(DgnDbR db);

    //! @name DWG Files
    //! @{
    //! Query if the specified file appears to have changed, compared to save data in syncinfo
    //! @return true if the file is not found in syncinfo or if it is found and its last-save time is different
    DWG_EXPORT bool HasLastSaveTimeChanged(DwgDbDatabaseCR);
    //! Query if a dwg file's disk file might have changed since syncinfo was created or last updated
    //! @param[in] dwgFileName  the name of the file on disk
    DWG_EXPORT bool HasDiskFileChanged(BeFileNameCR dwgFileName);
    //! Query if a dwg file's version GUID might have changed since syncinfo was created or last updated
    //! @param[in] dwg  DWG database
    DWG_EXPORT bool HasVersionGuidChanged(DwgDbDatabaseCR dwg);
    //! Query ExternalSourceAspect of RespositoryLink by file name
    DWG_EXPORT RepositoryLinkAspect FindFileByFileName(BeFileNameCR);
    //! Query RepositoryLinkId previsouly saved on DWG database
    DWG_EXPORT static DgnElementId GetRepositoryLinkId(DwgDbDatabaseR dwg);
    //! @}

    //! @name Models
    //! @{
    //! Create a ModelAspect for a modelspace/paperspace or an xRef insert.
    //! @param[in] model Target model for which a ModelAspect is to be added
    //! @param[in] block Source DWG block
    //! @param[in] trans A transformation matrix applied to the model
    //! @param[in] xrefInsert A source xRef instance (nullptr for non-xRef source type)
    //! @param[in] xrefDwg A source DWG database for an xRef (nullptr for non-xRef source type)
    DWG_EXPORT ModelAspect AddModelAspect (DgnModelR model, DwgDbBlockTableRecordCR block, TransformCR trans, DwgDbBlockReferenceCP xrefInsert, DwgDbDatabaseP xrefDwg);
    DWG_EXPORT ModelAspect AddModelAspect (DgnModelR model, DwgDbRasterImageCR raster, TransformCR trans);
    //! Get ModelAspect
    DWG_EXPORT ModelAspect FindModelAspect (DwgDbObjectIdCR id, DwgDbDatabaseR dwg, TransformCR trans);
    //! @}

    //! @name Layers
    //! @{
    DWG_EXPORT LayerAspect AddLayerAspect (DgnSubCategoryId subcatId, DwgDbObjectIdCR layerId) const;
    DWG_EXPORT DgnCategoryId FindCategory (DgnSubCategoryId* subcatId, DwgDbObjectIdCR layerId) const;
    DWG_EXPORT DwgDbHandle FindLayerHandle (DgnCategoryId catId, DgnElementId scopeId) const;
    //! @}

    //! @name Views
    //! @{
    DWG_EXPORT ViewAspect AddOrUpdateViewAspect(ViewDefinitionR view, DwgDbObjectIdCR sourceId, ViewAspect::SourceType sourceType, Utf8StringCR viewName) const;
    DWG_EXPORT DgnViewId FindView (DwgDbObjectIdCR sourceId, ViewAspect::SourceType sourceType) const;
    DWG_EXPORT DwgDbHandle FindViewportHandle (DgnViewId viewId) const;
    //! @}

    //! @name Linetypes
    //! @{
    DWG_EXPORT LinetypeAspect AddOrUpdateLinetypeAspect(DgnStyleId, DwgDbObjectIdCR ltypeId, Utf8StringCR ltypeName);
    DWG_EXPORT DgnStyleId FindLineStyle (DwgDbObjectIdCR ltypeId) const;
    //! @}

    //! @name Materials
    //! @{
    DWG_EXPORT MaterialAspect AddOrUpdateMaterialAspect(RenderMaterialId dgnMaterialId, DwgDbMaterialCR dwgMaterial);
    DWG_EXPORT MaterialAspect FindMaterialAspect(DwgDbObjectIdCR materialId) const;
    //! @}

    //! @name Groups
    //! @{
    DWG_EXPORT GroupAspect AddOrUpdateGroupAspect(DgnElementR dgnGroup, DwgDbGroupCR dwgGroup);
    DWG_EXPORT GroupAspect FindGroupAspect(DwgDbObjectIdCR grouplId) const;
    //! @}

    //! @name Objects
    //! @{
    //! Create a new or update existing ObjectAspect from pre-calculated source data
    //! @param[in] element Existing element along with its aspect to be updated. If the aspect does not exist, a new one is created from the input source data.
    //! @param[in] source Source data which will be saved in or updated on the ExternalSourceAspect of the element
    DWG_EXPORT ObjectAspect AddOrUpdateObjectAspect (DgnElementR element, ObjectAspect::SourceDataCR source);
    //! Create a new or update existing ObjectAspect from a source object ID and pre-calculated provenance
    //! @param[in] element Existing element along with its aspect to be updated
    //! @param[in] sourceId A unique ID from the source object, the value is used as Identifier for the ObjectAspect
    //! @param[in] sourceProvenance Pre-calculated provenance for the source object
    DWG_EXPORT ObjectAspect AddOrUpdateObjectAspect (DgnElementR element, DwgDbHandleCR sourceId, ObjectProvenanceCR sourceProvenance);
    //! Create a new or update existing ObjectAspect from a DWG object from which the provenance will be calculated
    //! @param[in] element Existing element along with its aspect to be updated
    //! @param[in] object Source DWG object
    DWG_EXPORT ObjectAspect AddOrUpdateObjectAspect (DgnElementR element, DwgDbObjectCR object);
    //! Find the ExternalSourceApsect for an object by object handle
    DWG_EXPORT ObjectAspect FindObjectAspect (DwgDbHandleCR objecthandle, DgnModelCR model, T_ObjectAspectFilter* filter=nullptr);
    //! Find the ExternalSourceApsect for an object by object provenance
    DWG_EXPORT ObjectAspect FindObjectAspect (ObjectProvenanceCR prov, DgnModelCR model, T_ObjectAspectFilter* filter=nullptr);
    //! Find all elements which have been mapped from this source object handle as an Element Kind
    DWG_EXPORT DgnElementIdSet FindElementsBy(DwgDbHandleCR sourceHandle) const;
    //! @}
    
    //! @name Misc
    //! @{
    DWG_EXPORT DgnElementIdSet FindElementsBy(Utf8CP kind, DwgDbHandleCR sourceHandle) const;
    DWG_EXPORT static T_ECInstanceIdArray GetExternalSourceAspectIds(DgnElementCR el, Utf8CP kind, Utf8StringCR sourceId);
    DWG_EXPORT static BeSQLite::EC::ECInstanceId GetSoleAspectIdByKind(DgnElementCR el, Utf8CP kind);
    //! @}
    };  // DwgSourceAspects

END_DWG_NAMESPACE

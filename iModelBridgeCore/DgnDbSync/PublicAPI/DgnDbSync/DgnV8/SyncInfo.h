/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/BeId.h>

// ===Namespace Warning===
// DgnV8 puts does not build unless there is a "using namespace Bentley" in effect.  
// ===Namespace Warning===

#include "DgnV8.h"

#include <DgnPlatform/DgnPlatform.h>
#include <iterator>     // std::iterator, std::input_iterator_tag
#include <Bentley/md5.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnDbSync/DgnDbSync.h>
#include <iModelBridge/iModelBridgeSyncInfoFile.h>

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

#define SYNCINFO_ATTACH_ALIAS "SYNCINFO"
#define SYNCINFO_TABLE(name)  "v8sync_" name
#define SYNCINFO_ATTACH(name) SYNCINFO_ATTACH_ALIAS "." name
struct Converter;
struct SyncInfo;
struct ResolvedModelMapping;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson   09/14
//=======================================================================================
enum class StableIdPolicy : bool
    {
    ById = 0,
    ByHash = 1
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SyncInfo
    {
    //! The V8 provenance of an element in an iModel. May refer to an element, model, or other object in the v8 source files.
    struct ExternalSourceAspect : iModelExternalSourceAspect
        {
      protected:
        friend struct SyncInfo;
        ExternalSourceAspect(ECN::IECInstance* i) : iModelExternalSourceAspect(i) {}
        static Utf8String FormatV8ElementId(DgnV8Api::ElementId v8Id) {char buf[32]; BeStringUtilities::FormatUInt64(buf, v8Id); return buf;}  // DgnV8Api::ElementId is a UInt64
        static Utf8String FormatV8ModelId(DgnV8Api::ModelId v8Id) {char buf[32]; return itoa(v8Id, buf, 10);}       // DgnV8Api::ModelId is a Int32
        BeSQLite::EC::ECInstanceId GetSoleAspectIdByKind(DgnElementCR el, Utf8CP kind);

      public:
        struct Kind 
            {
            static constexpr Utf8CP RepositoryLink = "DocumentWithBeGuid";  // Keep this consistent with iModelBridgeSyncInfoFile::RecordDocument
            static constexpr Utf8CP Element = "Element";
            static constexpr Utf8CP Model = "Model";
            static constexpr Utf8CP ProxyGraphic = "ProxyGraphic";
            static constexpr Utf8CP Level = "Level";
            static constexpr Utf8CP GeomPart = "GeomPart";
            static constexpr Utf8CP ViewDefinition = "ViewDefinition";
            static constexpr Utf8CP URI = "URI";
            };
        };

    struct V8ModelExternalSourceAspectIterator;

    //! Information about a file on disk. This struct captures the information that can be extracted from the disk file itself.
    struct DiskFileInfo
        {
        uint64_t m_lastModifiedTime{}; // (Unix time in seconds)
        uint64_t m_fileSize{};
        BentleyStatus GetInfo (BentleyApi::BeFileNameCR);
        DiskFileInfo() {m_lastModifiedTime = m_fileSize = 0;}
        bool IsEqual(DiskFileInfo const& o) {return m_lastModifiedTime == o.m_lastModifiedTime && m_fileSize == o.m_fileSize;}
        };

    //! Information about a path that might be a disk file or might be a URL
    struct UriContentInfo : DiskFileInfo
        {
        Utf8String m_eTag;    // Set if path is a URI
        
        BentleyStatus GetInfo(Utf8StringCR pathOrUrl);
        bool IsEqual(UriContentInfo const& o) {return DiskFileInfo::IsEqual(o) && m_eTag == o.m_eTag;}
        };

    //! Information about a file. This struct captures the information that can be extracted from the v8 file itself.
    struct V8FileInfo : DiskFileInfo
    {
        BentleyApi::Utf8String  m_uniqueName;
        BentleyApi::Utf8String  m_v8Name;
        double      m_lastSaveTime;
        V8FileInfo() : m_lastSaveTime(0.0) {}
    };

    //! Aspect stored on a RepositoryLink to capture additional info about the source file
    struct RepositoryLinkExternalSourceAspect : ExternalSourceAspect
        {
        RepositoryLinkExternalSourceAspect(ECN::IECInstance* i) : ExternalSourceAspect(i) {} 
        DGNDBSYNC_EXPORT static RepositoryLinkExternalSourceAspect CreateAspect(DgnDbR, V8FileInfo const&, StableIdPolicy);
        DGNDBSYNC_EXPORT bool Update(V8FileInfo const&);

        DGNDBSYNC_EXPORT static RepositoryLinkExternalSourceAspect FindAspectByIdentifier(DgnDbR, Utf8StringCR identifier);
        DGNDBSYNC_EXPORT static RepositoryLinkExternalSourceAspect GetAspectForEdit(RepositoryLinkR);
        DGNDBSYNC_EXPORT static RepositoryLinkExternalSourceAspect GetAspect(RepositoryLinkCR);

        RepositoryLinkId GetRepositoryLinkId() const {return RepositoryLinkId(GetElementId().GetValueUnchecked());}
        DGNDBSYNC_EXPORT Utf8String GetFileName() const;
        DGNDBSYNC_EXPORT StableIdPolicy GetStableIdPolicy() const;
        // WIP_EXTERNAL_SOURCE_ASPECT - combine the following methods into one that fills in a V8FileInfo object
        DGNDBSYNC_EXPORT uint64_t GetFileSize() const;
        DGNDBSYNC_EXPORT uint64_t GetLastModifiedTime() const;
        DGNDBSYNC_EXPORT double GetLastSaveTime() const;
        };

    struct RepositoryLinkExternalSourceAspectIterator : ExternalSourceAspectIterator<RepositoryLinkExternalSourceAspect>
        {
        RepositoryLinkExternalSourceAspectIterator(DgnDbR db, Utf8CP wh = nullptr) : ExternalSourceAspectIterator(db, db.Elements().GetRootSubjectId(), ExternalSourceAspect::Kind::RepositoryLink, wh) {}
        };

    //! Aspect that tracks the content of a URI. Will also work with a filepath.
    //! This is a lot like RepositoryLinkExternalSourceAspect. We make it separate just to keep things clear, as apps tend to think of RepostoryLinks as
    //! corresponding directly to documents in ProjectWise or another DMS. In the case of PW, the document is expected to have document properties, RBAC, etc. 
    //! UriExternalSourceAspect is for the general/generic case of sourcing data from a URI with nothing implied about what the resource is or if it has metadata or not.
    struct UriExternalSourceAspect : ExternalSourceAspect
        {
        private:
        UriExternalSourceAspect(ECN::IECInstance* i) : ExternalSourceAspect(i) {}
        public:
        static UriExternalSourceAspect CreateAspect(DgnElementId repositoryLinkId, Utf8CP filename, UriContentInfo const&, Utf8CP rdsId, Converter&);
        static UriExternalSourceAspect GetAspect(DgnElementCR);
        static UriExternalSourceAspect GetAspect(DgnElementId eid, DgnDbR db) {auto el = db.Elements().GetElement(eid); return el.IsValid()? GetAspect(*el): UriExternalSourceAspect(nullptr);}
        static UriExternalSourceAspect GetAspectForEdit(DgnElementR);

        Utf8String GetFilenameOrUrl() const {return GetIdentifier();}

        Utf8String GetSourceGuid() const; // The Source GUID may be empty and can change over time. That is why it is not the Identifier (or incorporated in it).
        void SetSourceGuid(Utf8StringCR);

        void GetInfo(UriContentInfo&) const;
        void SetInfo(UriContentInfo const&);
        };

    //! Aspect stored on a Model element to a) identify the source V8 model and b) store additional qualifying information, such as a transform, that is applied during conversion
    struct V8ModelExternalSourceAspect : ExternalSourceAspect
        {
        friend struct V8ModelExternalSourceAspectIterator;

      private:
        V8ModelExternalSourceAspect(ECN::IECInstance* i) : ExternalSourceAspect(i) {}
      public:
        V8ModelExternalSourceAspect() : ExternalSourceAspect(nullptr) {}

        static Utf8String FormatSourceId(DgnV8Api::ModelId v8Id) {return FormatV8ModelId(v8Id);}
        static Utf8String FormatSourceId(DgnV8ModelCR model) {return FormatSourceId(model.GetModelId());}

        //! Create a new aspect in memory. Caller must call AddAspect, passing in the model element that is to have this aspect.
        DGNDBSYNC_EXPORT static V8ModelExternalSourceAspect CreateAspect(DgnV8ModelCR, TransformCR, Converter&);
        
        DGNDBSYNC_EXPORT static std::tuple<DgnElementPtr, V8ModelExternalSourceAspect> GetAspectForEdit(DgnModelR);
        DGNDBSYNC_EXPORT static std::tuple<DgnElementCPtr, V8ModelExternalSourceAspect> GetAspect(DgnModelCR);
        DGNDBSYNC_EXPORT static V8ModelExternalSourceAspect GetAspectByAspectId(DgnDbR, BeSQLite::EC::ECInstanceId aspectId);

        RepositoryLinkId GetRepositoryLinkId() const {return RepositoryLinkId(GetScope().GetValueUnchecked());}
        DgnModelId GetModelId() const {return DgnModelId(GetElementId().GetValueUnchecked());}
        DGNDBSYNC_EXPORT DgnV8Api::ModelId GetV8ModelId() const;
        DGNDBSYNC_EXPORT Transform GetTransform() const;
        DGNDBSYNC_EXPORT void SetTransform(TransformCR);
        DGNDBSYNC_EXPORT Utf8String GetV8ModelName() const;
        };

    struct V8ModelExternalSourceAspectIterator : ExternalSourceAspectIterator<V8ModelExternalSourceAspect>
        {
        V8ModelExternalSourceAspectIterator(RepositoryLinkCR scope, Utf8CP wh = nullptr) : ExternalSourceAspectIterator(scope.GetDgnDb(), scope.GetElementId(), ExternalSourceAspect::Kind::Model, wh) {}
        V8ModelExternalSourceAspectIterator(DgnDbR db, RepositoryLinkExternalSourceAspect const& rlxsa, Utf8CP wh)  : ExternalSourceAspectIterator(db, rlxsa.GetElementId(), ExternalSourceAspect::Kind::Model, wh) {}
        };

    struct V8ModelExternalSourceAspectIteratorByV8Id : V8ModelExternalSourceAspectIterator
        {
        V8ModelExternalSourceAspectIteratorByV8Id(RepositoryLinkCR scope, DgnV8ModelCR v8Model) : V8ModelExternalSourceAspectIterator(scope, "Identifier = :identifier")
            {
            GetStatement()->BindText(GetParameterIndex("identifier"), V8ModelExternalSourceAspect::FormatSourceId(v8Model).c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
            }
        V8ModelExternalSourceAspectIteratorByV8Id(RepositoryLinkCR scope, DgnV8Api::ModelId v8ModelId) : V8ModelExternalSourceAspectIterator(scope, "Identifier = :identifier")
            {
            GetStatement()->BindText(GetParameterIndex("identifier"), V8ModelExternalSourceAspect::FormatSourceId(v8ModelId).c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
            }
        };

    struct ElementHash : BentleyApi::MD5::HashVal
    {
        ElementHash() {}
        ElementHash(BentleyApi::MD5::HashVal const& from) : BentleyApi::MD5::HashVal(from) {}
        bool IsSame(ElementHash const& other){return 0==memcmp(m_buffer, other.m_buffer, sizeof(m_buffer));}
    };

    struct ElementProvenance
    {
        double      m_lastModified;
        StableIdPolicy m_idPolicy;
        ElementHash m_hash;

        explicit ElementProvenance(BeSQLite::StatementP);
        ElementProvenance(DgnV8EhCR, SyncInfo&, StableIdPolicy);
        ElementProvenance() {}
        bool IsSame(ElementProvenance const& other){return (m_idPolicy==StableIdPolicy::ByHash || m_lastModified==other.m_lastModified) && m_hash.IsSame(other.m_hash);}
    };

    //! Data that uniquely identifies a V8 *element*. This data is used as part of the input when constructing a ExternalSourceAspect.
    //! This corresponds to V8ElementSource, which it will someday replace.
    struct V8ElementExternalSourceAspectData
        {
        DgnModelId m_scope;         // The model that was created (earlier) in the iModel from the V8 model that contains this V8 element.
        DgnV8Api::ElementId m_v8Id {}; // The V8 element's ID
        Utf8String m_v8IdPath;      // Or, a path of V8 element IDs, in case of elements in nested references.
        ElementProvenance m_prov;   // The V8 element's state
        Utf8String m_propsJson;
        V8ElementExternalSourceAspectData(DgnModelId scope, DgnV8Api::ElementId v8Id, ElementProvenance const& prov, Utf8StringCR propsJson) : m_scope(scope), m_v8Id(v8Id), m_prov(prov), m_propsJson(propsJson) {}
        V8ElementExternalSourceAspectData(DgnModelId scope, Utf8StringCR idPath, ElementProvenance const& prov, Utf8StringCR propsJson) : m_scope(scope), m_v8IdPath(idPath), m_prov(prov), m_propsJson(propsJson) {}
        };
    
    struct V8ElementExternalSourceAspectIterator;

    //! Identifies the source of an element in an iModel that was created from an element in a V8 model.
    struct V8ElementExternalSourceAspect : ExternalSourceAspect
        {
        friend struct V8ElementExternalSourceAspectIterator;
      private:
        V8ElementExternalSourceAspect(ECN::IECInstance* i) : ExternalSourceAspect(i) {}
      public:
        V8ElementExternalSourceAspect() : ExternalSourceAspect(nullptr) {}
        static Utf8String FormatSourceId(DgnV8Api::ElementId v8Id) {return FormatV8ElementId(v8Id);}
        static Utf8String FormatSourceId(DgnV8EhCR el) {return FormatSourceId(el.GetElementId());}

        //! Create a new aspect in memory. Caller must call AddAspect, passing in the element that is to have this aspect.
        DGNDBSYNC_EXPORT static V8ElementExternalSourceAspect CreateAspect(V8ElementExternalSourceAspectData const&, DgnDbR);
        
        //! Get an existing syncinfo aspect from the specified element in the case where we know that it was derived from a V8 *element*.
        //! Use this method only in the case where the element is known to have only a single element kind aspect.
        static V8ElementExternalSourceAspect GetAspectForEdit(DgnElementR, Utf8StringCR);
        static V8ElementExternalSourceAspect GetAspectForEdit(DgnElementR el, DgnV8Api::ElementId eid) {return GetAspectForEdit(el, FormatSourceId(eid));}
        //! Get an existing syncinfo aspect from the specified element in the case where we know that it was derived from a V8 *element*.
        //! Use this method only in the case where the element is known to have only a single element kind aspect.
        static V8ElementExternalSourceAspect GetAspect(DgnElementCR, Utf8StringCR);
        static V8ElementExternalSourceAspect GetAspect(DgnElementCR el, DgnV8Api::ElementId eid) {return GetAspect(el, FormatSourceId(eid));}

        static V8ElementExternalSourceAspect GetAspect(DgnElementCR el, BeSQLite::EC::ECInstanceId aspectId) {return V8ElementExternalSourceAspect(iModelExternalSourceAspect::GetAspect(el, aspectId).m_instance.get());}

        //! The scope of an V8ElementExternalSourceAspect is a model (element). This convenience method returns the scope as a DgnModelId
        DgnModelId GetModelId() const {return DgnModelId(GetScope().GetValueUnchecked());}

        DGNDBSYNC_EXPORT void Update(ElementProvenance const& prov); 

        DGNDBSYNC_EXPORT DgnV8Api::ElementId GetV8ElementId() const;

        DGNDBSYNC_EXPORT bool DoesProvenanceMatch(ElementProvenance const&) const;
        };

    struct V8ElementExternalSourceAspectIterator : ExternalSourceAspectIterator<V8ElementExternalSourceAspect>
    {
        V8ElementExternalSourceAspectIterator(DgnModelCR model, Utf8CP wh = nullptr) : ExternalSourceAspectIterator(model.GetDgnDb(), model.GetModeledElementId(), ExternalSourceAspect::Kind::Element, wh) {}
    };

    struct V8ElementExternalSourceAspectIteratorByV8Id : V8ElementExternalSourceAspectIterator 
    {
    private:
        void Bind(uint64_t elId) {m_stmt->Reset(); m_stmt->BindText(GetParameterIndex("idparm"), V8ElementExternalSourceAspect::FormatSourceId(elId).c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);}
    public:
        V8ElementExternalSourceAspectIteratorByV8Id(DgnModelCR model, DgnV8Api::ElementId eid) : V8ElementExternalSourceAspectIterator(model, "Identifier=:idparm") {Bind(eid);}
        V8ElementExternalSourceAspectIteratorByV8Id(DgnModelCR model, DgnV8EhCR eh) : V8ElementExternalSourceAspectIterator(model, "Identifier=:idparm") {Bind(eh.GetElementId());}
    };

    struct V8ElementExternalSourceAspectIteratorByChecksum : SyncInfo::V8ElementExternalSourceAspectIterator
    {
        private:
        void Bind(SyncInfo::ElementHash const& hash) {
            Utf8String provHash;
            iModelExternalSourceAspect::HexStrFromBytes(provHash, hash.m_buffer);
            m_stmt->Reset(); 
            m_stmt->BindText(GetParameterIndex("checksumparm"), provHash.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
            }
    public:
        V8ElementExternalSourceAspectIteratorByChecksum(DgnModelCR model, SyncInfo::ElementHash const& hash) : V8ElementExternalSourceAspectIterator(model, "Checksum=:checksumparm") {Bind(hash);}
    };

    struct LevelExternalSourceAspect : ExternalSourceAspect
        {
        enum class Type {Spatial, Drawing}; // WARNING: Persistent values - do not change

        private:
        static Utf8String FormatSourceId(DgnV8Api::LevelId v8Id) {return FormatV8ElementId(v8Id);}
        LevelExternalSourceAspect(ECN::IECInstance* i) : ExternalSourceAspect(i) {}
        DGNDBSYNC_EXPORT static LevelExternalSourceAspect CreateAspect(DgnElementId scopeId, DgnV8Api::LevelHandle const&, DgnV8ModelCR, Type, Converter&);

        public:
        //! Create a new aspect in memory. The scope will be the RepositoryLink element that stands for the source file. Caller must call AddAspect, passing in the Category element.
        DGNDBSYNC_EXPORT static LevelExternalSourceAspect CreateAspect(DgnV8Api::LevelHandle const&, DgnV8ModelCR, Type, Converter&);
        DGNDBSYNC_EXPORT static LevelExternalSourceAspect LevelExternalSourceAspect::FindAspectByV8Model(DgnSubCategoryCR, DgnV8ModelCR v8Model);

        DGNDBSYNC_EXPORT static BentleyStatus FindFirstSubCategory(DgnSubCategoryId&, DgnV8ModelCR v8Model, uint32_t flid, Type ltype, Converter& converter);
        };

    //! Identifies a drawing graphic in the V8 source
    struct ProxyGraphicExternalSourceAspect : ExternalSourceAspect
        {
        private:
        ProxyGraphicExternalSourceAspect(ECN::IECInstance* i) : ExternalSourceAspect(i) {}
        public:
        static Utf8String FormatSourceId(DgnV8Api::ElementId v8Id) {return FormatV8ElementId(v8Id);}
        DGNDBSYNC_EXPORT static ProxyGraphicExternalSourceAspect CreateAspect(DgnModelCR bimDrawingModel, Utf8StringCR idPath, Utf8StringCR propsJson, DgnDbR db);
        DGNDBSYNC_EXPORT static DgnElementId FindDrawingGraphic(DgnModelCR proxyGraphicScope, Utf8StringCR sectionedV8ElementPath, DgnCategoryId drawingGraphicCategory, DgnClassId drawingGraphicClassId, DgnDbR db);
        };

    //! Identifies the source of a ViewDefinition. The source is identified by the ElementId of the V8 view element. 
    //! This aspect *also* stores a view name as data, not as the primary identifier.
    struct ViewDefinitionExternalSourceAspect : ExternalSourceAspect
        {
        private:
        static Utf8String FormatSourceId(DgnV8Api::ElementId v8Id) {return FormatV8ElementId(v8Id);}

        public:
        static constexpr Utf8CP json_v8ViewName = "v8ViewName";

        ViewDefinitionExternalSourceAspect(ECN::IECInstance* i) : ExternalSourceAspect(i) {}

        //! Create a new aspect in memory. scopeId should be the RepositoryLink element that stands for the source file. Caller must call AddAspect, passing in the ViewDefinition element.
        DGNDBSYNC_EXPORT static ViewDefinitionExternalSourceAspect CreateAspect(DgnElementId scopeId, Utf8StringCR viewName, DgnV8ViewInfoCR viewInfo, DgnDbR db);

        DGNDBSYNC_EXPORT static ViewDefinitionExternalSourceAspect GetAspectForEdit(ViewDefinitionR el);
        DGNDBSYNC_EXPORT static ViewDefinitionExternalSourceAspect GetAspect(ViewDefinitionCR el);

        DgnViewId GetViewId() const {return DgnViewId(GetElementId().GetValueUnchecked());}

        //! Look up an existing ViewDefinition, given the scope and V8 ViewId
        DGNDBSYNC_EXPORT static std::tuple<ViewDefinitionExternalSourceAspect,DgnViewId> GetAspectBySourceId(DgnElementId scopeId, DgnV8ViewInfoCR, DgnDbR db);

        DGNDBSYNC_EXPORT void Update(DgnV8ViewInfoCR const&, Utf8StringCR viewName); 

        //! Get an iterator over all ViewDefinitionExternalSourceAspects with the specified scope (which should be a RepositoryLink element ID).
        //! The iterator will select Element.Id and ECInstanceId
        DGNDBSYNC_EXPORT static BeSQLite::EC::CachedECSqlStatementPtr GetIteratorForScope(DgnDbR, DgnElementId scope);
    
        // DGNDBSYNC_EXPORT DgnV8Api::ElementId GetV8ViewId() const;
        DGNDBSYNC_EXPORT Utf8String GetV8ViewName() const;
        };

    struct ViewDefinitionExternalSourceAspectIterator : ExternalSourceAspectIterator<ViewDefinitionExternalSourceAspect>
        {
        ViewDefinitionExternalSourceAspectIterator(DgnDbR db, DgnElementId scope, Utf8CP wh = "") : ExternalSourceAspectIterator(db, scope, ExternalSourceAspect::Kind::ViewDefinition, wh) {}
        };

    //! A GeomPart mapping
    struct GeomPartExternalSourceAspect : ExternalSourceAspect
        {
        private:
        GeomPartExternalSourceAspect(ECN::IECInstance* i) : ExternalSourceAspect(i) {}
        public:
        //! Create a new aspect in memory. scopeId should be the bridge's job definition model. Caller must call AddAspect, passing in the DgnGeometryPart element.
        DGNDBSYNC_EXPORT static GeomPartExternalSourceAspect CreateAspect(DgnElementId scopeId, Utf8StringCR tag, DgnDbR);
        //! Look up the element that has the GeomPart aspect with the specified tag. Note that this is based on the assumption that GeometryPart "tags" are unique within the specified scope!
        static DgnGeometryPartId GetAspectByTag(DgnDbR db, DgnElementId scopeId, Utf8StringCR tag) {return DgnGeometryPartId(FindElementBySourceId(db, scopeId, Kind::GeomPart, tag).elementId.GetValueUnchecked());}
        //! Get an existing GeomPart aspect from the specified DgnGeometryPart
        DGNDBSYNC_EXPORT static GeomPartExternalSourceAspect GetAspect(DgnGeometryPartCR el);
        };

    //! An Id that is per-file
    template<typename IDTYPE> struct FileBasedId
        {
        RepositoryLinkId m_repositoryLinkId;
        IDTYPE m_id;
        FileBasedId () : m_id(0) {}
        FileBasedId (RepositoryLinkId ffid, IDTYPE id) : m_repositoryLinkId(ffid), m_id(id) {}
        bool operator<(FileBasedId const& rhs) const
            {
            if (m_repositoryLinkId < rhs.m_repositoryLinkId) return true;
            if (m_repositoryLinkId > rhs.m_repositoryLinkId) return false;
            return m_id < rhs.m_id;
            }
        };

    typedef FileBasedId<int32_t>  V8StyleId;
    typedef FileBasedId<int32_t>  V8FontId;

    struct MappedLineStyle
        {
        DgnStyleId      m_id;
        double          m_unitsScale;   //  may be needed to adjust LineStyleParams since they expressed in Line Style units.

        MappedLineStyle() : m_unitsScale(1) {}
        MappedLineStyle(DgnStyleId id, double unitsScale) : m_id(id), m_unitsScale(unitsScale) {}
        };

    bmap<V8StyleId,MappedLineStyle> m_lineStyle;  // Temporary
    bmap<V8FontId,DgnFontId>        m_font;       // Temporary

    Converter&          m_converter;
    DgnDb*              m_dgndb;

protected:
    bool ComputeHash(BentleyApi::MD5&, DgnV8ModelR, DgnV8Api::MSElement const&, uint32_t offsetToStartOfData);
    void DumpElement (DgnV8Api::ElementHandle const&);
    void DumpXAttribute (DgnV8Api::ElementHandle::XAttributeIter const& ix);
public:
    static bvector<BeSQLite::EC::ECInstanceId> GetExternalSourceAspectIds(DgnElementCR el, Utf8CP kind, Utf8StringCR sourceId);
    static BeSQLite::EC::ECInstanceId GetSoleAspectIdByKind(DgnElementCR el, Utf8CP kind);

    BentleyStatus CreateTables();
    BentleyStatus CreateNamedGroupTable();

    DGNDBSYNC_EXPORT BeSQLite::DbResult InsertFont (DgnFontId newId, V8FontId oldId);
    DGNDBSYNC_EXPORT BeSQLite::DbResult InsertLineStyle (DgnStyleId, double unitsScale, V8StyleId oldId);

    //! Construct a SyncInfo object to use for the specified project.
    SyncInfo(Converter&);

    DgnDb* GetDgnDb() {return m_dgndb;}

    //! Creates the temporary tables used during a single conversion.  None of this information is necessary on updates.
    //! @return non-zero if initialization or verification failed.
    //! @param[in] project The project.
    DGNDBSYNC_EXPORT BentleyStatus Initialize(DgnDb& project);

    //! @name V8 Files
    //! @{

    //! Get a name for the specified file that not used by any other file registered in SyncInfo
    Utf8String GetUniqueNameForFile(DgnV8FileCR file);

    //! Query if the specified file appears to have changed, compared to save data in syncinfo
    //! @return true if the file is not found in syncinfo or if it is found and its last-save time is different
    DGNDBSYNC_EXPORT bool HasLastSaveTimeChanged(DgnV8FileCR file);

    //! Query if a v8 file's disk file might have changed since syncinfo was created or last updated
    //! @param[in] v8FileName  the name of the file on disk
    //! @param[in] uniqueName       the key that identifies the file in sync info
    DGNDBSYNC_EXPORT bool HasDiskFileChanged(BeFileNameCR v8FileName);

    DGNDBSYNC_EXPORT V8FileInfo ComputeFileInfo(DgnV8FileCR);

    DGNDBSYNC_EXPORT RepositoryLinkExternalSourceAspect FindFileByFileName(BeFileNameCR);
    
    //! @}


    //! @name Elements
    //! @{

    DGNDBSYNC_EXPORT static void SetDumpHash(FILE*);

    DGNDBSYNC_EXPORT void ComputeHash(BentleyApi::MD5& hasher, DgnV8EhCR v8eh);

    //! @}

    //! @name Levels, Fonts, Styles, Materials
    //! @{

    DGNDBSYNC_EXPORT LevelExternalSourceAspect InsertLevel(DgnSubCategoryId glevelId, DgnV8ModelCR v8model, DgnV8Api::LevelHandle const& vlevel);

    //! Lookup the first native level mapped to the specified v8 level id. Elements using this level are assumed to be in the current v8 model in the current v8 file.
    //! This function checks first for a model-specific version of the level and then falls back to a file-wide version.
    //! This function returns the default category if all else fails
    DGNDBSYNC_EXPORT DgnSubCategoryId GetSubCategory(uint32_t v8levelid, DgnV8ModelCR, LevelExternalSourceAspect::Type ltype);

    //! Find the category to use for the specified DgnV8 element
    //! This function returns the default category if all else fails
    DGNDBSYNC_EXPORT DgnCategoryId GetCategory(DgnV8EhCR, ResolvedModelMapping const&);

    DGNDBSYNC_EXPORT DgnSubCategoryId FindSubCategory(uint32_t v8levelid, DgnV8ModelCR, LevelExternalSourceAspect::Type ltype);
    DGNDBSYNC_EXPORT DgnSubCategoryId FindSubCategory(uint32_t v8levelId, DgnV8FileR, LevelExternalSourceAspect::Type ltype);
    DGNDBSYNC_EXPORT DgnCategoryId FindCategory(uint32_t v8levelId, DgnV8FileR, LevelExternalSourceAspect::Type ltype);

    BentleyStatus FindFirstSubCategory (DgnSubCategoryId& glid, BeSQLite::Db&, DgnV8ModelCR v8Model, uint32_t flid, LevelExternalSourceAspect::Type ltype)
        {
        return LevelExternalSourceAspect::FindFirstSubCategory(glid, v8Model, flid, ltype, m_converter);
        }

    //! Query sync info for a v8 font in the current v8 file.
    DGNDBSYNC_EXPORT DgnFontId FindFont(V8FontId oldId);

    //! Query sync info for a v8 linestyle in the current v8 file.
    DGNDBSYNC_EXPORT DgnStyleId FindLineStyle(double& unitsScale, bool& foundStyle, V8StyleId oldId);

    //! Query sync info for a v8 material in the current v8 file.
    DGNDBSYNC_EXPORT RenderMaterialId FindMaterialByV8Id (uint64_t v, DgnV8FileR v8File, DgnV8ModelR v8Model);

    //! @}

    //! @name ECSchemas
    //! @{

    enum class ECSchemaMappingType
        {
        Identity = 1, //!< Mapped as is
        Dynamic = 2 //!< if multiple dynamic schemas exist, they will be merged during conversion
        };

    //! Adds an entry for this schema
    //! @param[in] repositoryId - ElementId of the repository (DgnV8 file) that this schema was found in
    //! @param[in] schemaName - Name of the schema
    //! @param[in] v8SchemaVersionMajor - major version of the schema
    //! @param[in] v8SchemaVersionMinor - minor version of the schema
    //! @param[in] isDynamic - Whether this is a dynamic schema or not
    //! @param[in] checksum - (V8) calculated checksum
    DGNDBSYNC_EXPORT BeSQLite::DbResult InsertSchema(BentleyApi::ECN::ECSchemaId& insertedSchemaId, DgnElementId repositoryId, Utf8StringCR schemaName, uint32_t v8SchemaVersionMajor, uint32_t v8SchemaVersionMinor,
                                                     bool isDynamic, uint32_t checksum);

    //! Returns information about the given schema.  If a scope (repositoryId representing the V8 file) is given, will only look in that repository.  Otherwise, will just check to see if 
    //! that schema has been synced and return information about the first found
    //! @param[out] versionMajor - major schema version of the found schema
    //! @param[out] versionMinor - minor schema version of the found schema
    //! @param[out] isDynamic - whether the found schema is dynamic or not
    //! @param[out] checksum - (V8) calculated checksum of the found schema
    //! @param[in] schemaName - Schema to look for
    //! @param[in] scope - limit query to only this scope (optional)
    DGNDBSYNC_EXPORT bool TryGetECSchema(ECObjectsV8::SchemaKey&, ECSchemaMappingType&, Utf8StringCR schemaName, DgnElementId scope = DgnElementId());

    DGNDBSYNC_EXPORT bool ContainsECSchema(Utf8CP v8SchemaName) const;

    //! For a give file, will return all of the schema names and checksums in that file
    //! @param[out] syncInfoChecksums - map of schemaname:checksum pairs
    //! @param[in]  fileId - The file to query
    DGNDBSYNC_EXPORT BeSQLite::DbResult RetrieveECSchemaChecksums(bmap<Utf8String, uint32_t>& syncInfoChecksums, RepositoryLinkId fileId);
    //! @}

    //! @name NamedGroups - The index is dropped on the ElementRefersToElements table while inserting named group members.  This was to allow for fast inserts, but as a result, lookups are slow and so
    //! the converter can no longer check for an existing entry before calling ElementGroupsElement::Insert.  Therefore, we use a temporary indexed table for checking whether an element is already a
    //! member of the given group.
    //! @{

    //! Check to see if the given element is already a member of the group
    //! @param[in] sourceId - ElementId of the group owner
    //! @param[in] targetId - ElementId of the potential new member element
    //! @return true if the element is already a member of the group
    DGNDBSYNC_EXPORT bool IsElementInNamedGroup(DgnElementId sourceId, DgnElementId targetId);
    //! @}

    BentleyStatus AddElementToNamedGroup(DgnElementId sourceId, DgnElementId targetId);

    //! Checks to see if the given V8File has any associated image files and if so, checks each one to see if it has changed
    //! @param[in] fileId - SyncInfo id of the V8File
    DGNDBSYNC_EXPORT bool FileHasChangedUriContent(RepositoryLinkId fileId);

    //! Checks to see if the given element a) is based on remote imagery data, and if so b) has the remote data changed.
    //! Returns false if the element is not based on remote imagery data.
    DGNDBSYNC_EXPORT bool HasUriContentChanged(DgnElementId eid);

    Converter& GetConverter() const {return m_converter;}
    };

END_DGNDBSYNC_DGNV8_NAMESPACE

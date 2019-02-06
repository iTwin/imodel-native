/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/DgnV8/SyncInfo.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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

#define SYNC_TABLE_ECSchema     SYNCINFO_TABLE("ECSchema")
#define SYNC_TABLE_NamedGroups  SYNCINFO_TABLE("NamedGroups")
#define SYNC_TABLE_Imagery      SYNCINFO_TABLE("Imagery")

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
        static Utf8String FormatV8ElementId(DgnV8Api::ElementId v8Id) {return Utf8PrintfString("%llu", v8Id);}  // DgnV8Api::ElementId is a UInt64
        static Utf8String FormatV8ModelId(DgnV8Api::ModelId v8Id) {return Utf8PrintfString("%ld", v8Id);}       // DgnV8Api::ModelId is a Int32
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
            };
        };

    struct V8ModelExternalSourceAspectIterator;

    //! Information about a file on disk. This struct captures the information that can be extracted from the disk file itself.
    struct DiskFileInfo
        {
        uint64_t m_lastModifiedTime; // (Unix time in seconds)
        uint64_t m_fileSize;
        BentleyApi::BentleyStatus GetInfo (BentleyApi::BeFileNameCR);
        DiskFileInfo() {m_lastModifiedTime = m_fileSize = 0;}
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
        DGNDBSYNC_EXPORT void Update(V8FileInfo const&);

        DGNDBSYNC_EXPORT static RepositoryLinkExternalSourceAspect FindAspectByIdentifier(DgnDbR, Utf8StringCR identifier);
        DGNDBSYNC_EXPORT static RepositoryLinkExternalSourceAspect GetAspectForEdit(RepositoryLinkR);
        DGNDBSYNC_EXPORT static RepositoryLinkExternalSourceAspect GetAspect(RepositoryLinkCR);

        RepositoryLinkId GetRepositoryLinkId() const {return RepositoryLinkId(GetElementId().GetValueUnchecked());}
        DGNDBSYNC_EXPORT Utf8String GetFileName() const;
        DGNDBSYNC_EXPORT uint64_t GetFileSize() const;
        DGNDBSYNC_EXPORT uint64_t GetLastModifiedTime() const;
        DGNDBSYNC_EXPORT double GetLastSaveTime() const;
        DGNDBSYNC_EXPORT StableIdPolicy GetStableIdPolicy() const;
        };

    struct RepositoryLinkExternalSourceAspectIterator : ExternalSourceAspectIterator<RepositoryLinkExternalSourceAspect>
        {
        RepositoryLinkExternalSourceAspectIterator(DgnDbR db, Utf8CP wh = nullptr) : ExternalSourceAspectIterator(db, db.Elements().GetRootSubjectId(), ExternalSourceAspect::Kind::RepositoryLink, wh) {}
        };

    //! Aspect stored on a Model element to a) identify the source V8 model and b) store additional qualifying information, such as a transform, that is applied during conversion
    struct V8ModelExternalSourceAspect : ExternalSourceAspect
        {
        friend struct V8ModelExternalSourceAspectIterator;

      private:
        V8ModelExternalSourceAspect(ECN::IECInstance* i) : ExternalSourceAspect(i) {}
      public:
        V8ModelExternalSourceAspect() : ExternalSourceAspect(nullptr) {}

        static Utf8String FormatSourceId(DgnV8Api::ModelId v8Id) {return FormatV8ElementId(v8Id);}
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
        private:
        static Utf8String FormatSourceId(DgnV8Api::LevelId v8Id) {return FormatV8ElementId(v8Id);}
        LevelExternalSourceAspect(ECN::IECInstance* i) : ExternalSourceAspect(i) {}
        DGNDBSYNC_EXPORT static LevelExternalSourceAspect CreateAspect(DgnElementId scopeId, DgnV8Api::LevelHandle const&, DgnV8ModelCR, Converter&);

        public:
        static constexpr Utf8CP json_v8LevelName = "v8LevelName";
        static constexpr Utf8CP json_v8ModelId = "v8ModelId";

        enum class Type {Spatial, Drawing}; // WARNING: Persistent values - do not change

        //! Create a new aspect in memory. The scope will be the RepositoryLink element that stands for the source file. Caller must call AddAspect, passing in the Category element.
        DGNDBSYNC_EXPORT static LevelExternalSourceAspect CreateAspect(DgnV8Api::LevelHandle const&, DgnV8ModelCR, Converter&);

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
        DGNDBSYNC_EXPORT static DgnElementId FindDrawingGraphicBySource(DgnModelCR drawingModel, Utf8StringCR idPath, DgnCategoryId drawingGraphicCategory, DgnClassId elementClassId, DgnDbR db);
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

    enum class ECSchemaMappingType
        {
        Identity = 1, //!< Mapped as is
        Dynamic = 2 //!< if multiple dynamic schemas exist, they will be merged during conversion
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
    BeSQLite::DbResult  m_lastError;
    Utf8String          m_lastErrorDescription;
    bool                m_isValid;

protected:
    bool ComputeHash(BentleyApi::MD5&, DgnV8ModelR, DgnV8Api::MSElement const&, uint32_t offsetToStartOfData);
    void DumpElement (DgnV8Api::ElementHandle const&);
    void DumpXAttribute (DgnV8Api::ElementHandle::XAttributeIter const& ix);

    BentleyStatus PerformVersionChecks();

public:
    static bvector<BeSQLite::EC::ECInstanceId> GetExternalSourceAspectIds(DgnElementCR el, Utf8CP kind, Utf8StringCR sourceId);
    static BeSQLite::EC::ECInstanceId GetSoleAspectIdByKind(DgnElementCR el, Utf8CP kind);

    BentleyStatus CreateTables();
    BentleyStatus CreateNamedGroupTable(bool createIndex);
    void CreateECTables();

    DGNDBSYNC_EXPORT BeSQLite::DbResult InsertFont (DgnFontId newId, V8FontId oldId);
    DGNDBSYNC_EXPORT BeSQLite::DbResult InsertLineStyle (DgnStyleId, double unitsScale, V8StyleId oldId);

    DGNDBSYNC_EXPORT BeSQLite::DbResult SavePropertyString (BeSQLite::PropertySpec const& spec, Utf8CP stringData, uint64_t majorId=0, uint64_t subId=0);
    DGNDBSYNC_EXPORT BeSQLite::DbResult QueryProperty (Utf8StringR str, BeSQLite::PropertySpec const& spec, uint64_t id=0, uint64_t subId=0) const;

    //! Construct a SyncInfo object to use for the specified project.
    SyncInfo(Converter&);
    DGNDBSYNC_EXPORT ~SyncInfo();

    DgnDb* GetDgnDb() {return m_dgndb;}

    //! Get the name of the .syncinfo file
    DGNDBSYNC_EXPORT static BeFileName GetDbFileName (DgnDb&);
    DGNDBSYNC_EXPORT static BeFileName GetDbFileName (BeFileName const&);

    //! Call this to attach a synchinfo file to a project.
    DGNDBSYNC_EXPORT BentleyStatus AttachToProject (DgnDb& targetProject, BeFileNameCR dbName);

    //! This function associates this SyncInfoBase object with the specified project.
    //! Call this after attaching an existing .syncinfo file to the specified project.
    //! This function either finishes creating a new syncInfo or it validates an existing syncinfo.
    //! @return non-zero if initialization or verification failed.
    //! @param[in] project The project.
    BentleyStatus OnAttach(DgnDb& project);

    //! Create an empty .syncinfo file. Then attach it to the project.
    //! @param[in] dbName The name of the syncinfo file
    //! @param[in] deleteIfExists If true, any existing .syncinfo file with the same name is deleted
    DGNDBSYNC_EXPORT static BentleyStatus CreateEmptyFile (BeFileNameCR dbName, bool deleteIfExists=true);

    //! Query if the SyncInfo is valid (created or read)
    bool IsValid() const {return m_isValid;}

    //! Mark the SyncInfo as valid or not.
    void SetValid (bool b) {m_isValid=b;}

    DGNDBSYNC_EXPORT void SetLastError (BeSQLite::DbResult);
    DGNDBSYNC_EXPORT void GetLastError (BeSQLite::DbResult&, Utf8String&);

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

    //! Record sync info for a level.
    //! @param[out] info        Sync info for the level
    //! @param[in]  glevelId    The level's ID in the DgnDb
    //! @param[in]  v8model     If the level is to be used only by elements in a single model, then \a v8model should identify the model. If the level is to be
    //! used by elements in any model in the current v8 file, then pass the dictionary model.
    //! @param[in]  vlevel      The V8 level that was converted
    //! @return non-zero error status if the level could not be inserted in sync info. This would probably be caused by a non-unique name.
    //! @note reports an issue in insertion fails.
    DGNDBSYNC_EXPORT BentleyStatus InsertLevel(DgnSubCategoryId glevelId, DgnV8ModelCR v8model, DgnV8Api::LevelHandle const& vlevel);

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
    DGNDBSYNC_EXPORT BeSQLite::DbResult InsertECSchema(ECN::ECSchemaId&, DgnV8FileR, Utf8CP v8SchemaName, uint32_t v8SchemaVersionMajor, uint32_t v8SchemaVersionMinor, bool isDynamic, uint32_t checksum) const;
    DGNDBSYNC_EXPORT bool TryGetECSchema(ECObjectsV8::SchemaKey&, ECSchemaMappingType&, Utf8CP v8SchemaName, RepositoryLinkId fileId) const;
    DGNDBSYNC_EXPORT bool ContainsECSchema(Utf8CP v8SchemaName) const;
    DGNDBSYNC_EXPORT BeSQLite::DbResult RetrieveECSchemaChecksums(bmap<Utf8String, uint32_t>& syncInfoChecksums, RepositoryLinkId fileId) const;
    //! @}

    //! @name NamedGroups - The index is dropped on the ElementRefersToElements table while inserting named group members.  This was to allow for fast inserts, but as a result, lookups are slow and so
    //! the converter can no longer check for an existing entry before calling ElementGroupsElement::Insert.  Therefore, we use an indexed table in SyncInfo for checking whether an element is already a
    //! member of the given group.
    //! @{

    //! If doing an update against an older syncinfo table, the NamedGroup table won't exist.  This will check for its existence.  If it doesn't exist, it will create it and populate it from the
    //! ElementRefersToElements table in the dgndb
    DGNDBSYNC_EXPORT BentleyStatus CheckNamedGroupTable();

    //! Check to see if the given element is already a member of the group
    //! @param[in] sourceId - ElementId of the group owner
    //! @param[in] targetId - ElementId of the potential new member element
    //! @return true if the element is already a member of the group
    DGNDBSYNC_EXPORT bool IsElementInNamedGroup(DgnElementId sourceId, DgnElementId targetId);

    //! Adds the given element to the group.
    //! @param[in] sourceId - ElementId of the group owner
    //! @param[in] targetId - ElementId of the new member element
    DGNDBSYNC_EXPORT BentleyStatus AddNamedGroupEntry(DgnElementId sourceId, DgnElementId targetId);

    //! Insertions are stored in a temporary table.  Upon completion of processing the named groups, the temp table must be merged into the SyncInfo table.
    DGNDBSYNC_EXPORT BentleyStatus FinalizeNamedGroups();
    //! @}

    //! Checks to see if the View syncinfo table exists.  This is only necessary when updating imodels created early during the EAP process.  Will create the table if it doesn't exist
    bool EnsureImageryTableExists();

    //! Record the provenance for reality data (raster, PointCloud, ThreeMx, ScalableMesh, etc.  
    //! @param[in] modeledElementId - ElementId that models this element.  Only one entry per elementId is allowed
    //! @param[in] filename - Filename (or URL) to the imagery
    //! @param[in] lastModifiedTime - Time the file was last modified
    //! @param[in] fileSize - Size of the image file
    //! @param[in] etag - Unique marker for a file that is changed by the webserver whenever the file is changed
    //! @param[in] rdsId - Guid from the reality data server
    DGNDBSYNC_EXPORT BeSQLite::DbResult InsertImageryFile(DgnElementId modeledElementId, RepositoryLinkId filesiid, Utf8CP filename, uint64_t lastModifiedTime, uint64_t fileSize, Utf8CP etag, Utf8CP rdsId);

    //! Checks to see if the given V8File has any associated image files and if so, checks each one to see if it has changed
    //! @param[in] fileId - SyncInfo id of the V8File
    DGNDBSYNC_EXPORT bool ModelHasChangedImagery(RepositoryLinkId fileId);

    //! Looks for an entry for the given modeledElementId
    //! @param[in] modeledElementId - ElementId of the model to look for
    //! @param[out] lastModifiedTime - last modified time of the imagery
    //! @param[out] fileSize - size of the imagery file
    //! @param[out] etag - Unique marker for a file that is changed by the webserver whenever the file is changed
    //! @param[out] rdsId - Guid from the reality data server
    DGNDBSYNC_EXPORT bool TryFindImageryFile(DgnElementId modeledElementId, Utf8StringR filename, uint64_t& lastModifiedTime, uint64_t &fileSize, Utf8StringR etag, Utf8StringR rdsId);

    //! Updates the entry for an imagery file
    //! @param[in] modeledElementId - ElementId of the model to look for
    //! @param[in] lastModifiedTime - last modified time of the imagery
    //! @param[in] fileSize - size of the imagery file
    //! @param[in] etag - Unique marker for a file that is changed by the webserver whenever the file is changed
    //! @param[in] rdsId - Guid from the reality data server
    DGNDBSYNC_EXPORT BeSQLite::DbResult UpdateImageryFile(DgnElementId modeledElementId, uint64_t lastModifiedTime, uint64_t fileSize, Utf8CP etag, Utf8CP rdsId);

    //! Given a filename, will get the current info about the file
    //! @param[in] filename - Path to either local file or the URL of the imagery
    //! @param[in] currentModifiedTime - last modified time if a local file
    //! @param[in] currentFileSize - file size of a local file
    //! @param[in] currentEtag - Web server's unique marker for the given URL
    DGNDBSYNC_EXPORT void GetCurrentImageryInfo(Utf8StringCR filename, uint64_t& currentLastModifiedTime, uint64_t& currentFileSize, Utf8StringR currentEtag);

    Converter& GetConverter() const {return m_converter;}
    };

END_DGNDBSYNC_DGNV8_NAMESPACE

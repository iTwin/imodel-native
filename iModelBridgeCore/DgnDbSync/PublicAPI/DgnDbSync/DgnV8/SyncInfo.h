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

#define SYNC_TABLE_File         SYNCINFO_TABLE("File")
#define SYNC_TABLE_Model        SYNCINFO_TABLE("Model")
#define SYNC_TABLE_Element      SYNCINFO_TABLE("Element")
#define SYNC_TABLE_ExtractedGraphic SYNCINFO_TABLE("ExtractedGraphic")
#define SYNC_TABLE_Level        SYNCINFO_TABLE("Level")
#define SYNC_TABLE_View         SYNCINFO_TABLE("View")
#define SYNC_TABLE_ECSchema     SYNCINFO_TABLE("ECSchema")
#define SYNC_TABLE_Discards     SYNCINFO_TABLE("Discards")
#define SYNC_TABLE_ImportJob    SYNCINFO_TABLE("ImportJob")
#define SYNC_TABLE_NamedGroups  SYNCINFO_TABLE("NamedGroups")
#define SYNC_TABLE_Imagery      SYNCINFO_TABLE("Imagery")
#define SYNC_TABLE_GeomPart     SYNCINFO_TABLE("GeomPart")

struct Converter;
struct SyncInfo;
struct ResolvedModelMapping;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson  07/14
//=======================================================================================
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

//=======================================================================================
// @bsiclass                                                    Sam.Wilson   09/14
//=======================================================================================
enum class StableIdPolicy : bool
    {
    ById = 0,
    ByHash = 1
    };

/*=================================================================================**//**
* Stores information needed to synchronize data that is created in a DgnDb from source DgnV8 file(s).
*
* Converter notes.
*
* Syncinfo is set up to support converters, *from* v8 elements *to* DgnDb elements.
* Thus, the primary key of most syncinfo tables is based on the v8 ids, not the DgnDb identifiers (if any).
*
* You might think of syncinfo as a converter's notes to itself. The converter has many options for how it maps v8
* elements to DgnDb elements. The converter can convert a v8 element to a single corresponding DgnDb entity, it could map many v8 elements
* to one DgnDb entity, or it could distribute the data from a single v8 element across multiple DgnDb elements. The converter could
* also decide to discard a v8 element and not create any corresponding data in the DgnDb.
* Syncinfo is where it keeps a record of what it did.
*
* Syncinfo can be used by DgnDb apps to get "provenance" information about DgnDb elements, but it should be understood that
* the mapping backward from DgnDb elements to v8 elements may be complex.
*
* -- File  - 1:1        - a converter should create a File sync info entry for each v8 file that it processes.
*                           The primary key of the File table in sync info is just an autoincrement integer. It
*                           does not identify anything in the DgnDb project itself.
*
* -- Model              - a converter should create a Model sync info entry for each v8 model that it processes. The converter may:
*            1:1            -- create a DgnDb model to represent a single v8 model.
*            0:1            -- decide not to create a DgnDb model for a given v8 model.
*            1:n            -- "map" many v8 models to a single DgnDb model.
*            n:1            -- make several (transformed) copies in the DgnDb from a single v8 model.
*                           Thus, neither the v8 Id (V8FileSyncInfoId,V8Id) nor the DgnDb DgnModelId (NativeId) can be the primary key.
*                           We index both, and the index can contain dups.
*
* -- Element            - the mapping between DgnDb and v8 elements is very loose. A converter may:
*            1:1            -- convert a v8 element to a DgnDb entity,
*            0:1            -- reject and discard the v8 element,
*            0:1            -- convert the v8 element to non-element data in the DgnDb (e.g., convert a 'view' element to a DgnDb view).
*            n:1            -- distribute the v8 element's data across multiple DgnDb elements
*            1:n            -- combine the data from multiple v8 elements into a single DgnDb entity (e.g., stitch lines together into a linestring).
*            1:n            -- map many v8 elements to a single DgnDb entity (e.g., cells with matching name and definition).
*                           Thus, neither v8 id (V8FileSyncInfoId,V8Model,V8Id nor the DgnDb DgnElementId column can be the primary key.
*                           We index both, and the index can contain dups.
*                           We also index the Digest column.
*
* -- Level              - the mapping between DgnDb and v8 levels is very loose. A converter may:
*            1:1            -- convert a v8 level to a new DgnDb level,
*            0:1            -- reject and discard the v8 level (e.g., because it's unused),
*            1:n            -- map several v8 levels to a single DgnDb level.
*            n:1            -- make several (renamed) copies in the DgnDb from a single v8 level. For example, the convert may create model-specific copies of a given level in a file.
*                           Thus, nether the v8 id (V8FileSyncInfoId,V8Model,V8Id) nor the DgnDb DgnSubCategoryId (Id) column can be the primary key.
*                           We index both, and the index can contain dups.
*                           Note: If the V8Model column is not nullptr, then the level is model-specific.
*                           Note: If the V8Model column is nullptr, then the level is file-wide.
*
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SyncInfo
    {
    struct ModelIterator;

    //! Information about a file on disk. This struct captures the information that can be extracted from the v8 disk file itself.
    struct DiskFileInfo
        {
        uint64_t m_lastModifiedTime; // (Unix time in seconds)
        uint64_t m_fileSize;
        BentleyApi::BentleyStatus GetInfo (BentleyApi::BeFileNameCR);
        DiskFileInfo() {m_lastModifiedTime = m_fileSize = 0;}
        };

    //! Information about a file. This struct captures the information that can be extracted from the v8 file itself.
    struct FileInfo : DiskFileInfo
    {
        BentleyApi::Utf8String  m_uniqueName;
        BentleyApi::Utf8String  m_v8Name;
        StableIdPolicy m_idPolicy;
        double      m_lastSaveTime;
        FileInfo() {m_lastSaveTime = 0.0; m_idPolicy=StableIdPolicy::ById;}
    };

    struct V8FileSyncInfoId : BeUInt32Id<V8FileSyncInfoId,UINT32_MAX>
    {
        V8FileSyncInfoId() {Invalidate();}
        explicit V8FileSyncInfoId(uint32_t u) : BeUInt32Id(u) {}
        void CheckValue() const {BeAssert(IsValid());}
    };

    //! Sync info for a v8 file. The struct includes information assigned to the v8 file by the converter.
    //! There should be 1 File entry in syncinfo for each v8 file processed by the converter.
    struct V8FileProvenance : FileInfo
        {
        friend SyncInfo;
        SyncInfo*           m_syncInfo;
        V8FileSyncInfoId    m_syncId;

        private:
        BeSQLite::DbResult Insert();
        BeSQLite::DbResult Update(V8FileSyncInfoId, FileInfo const&);

        V8FileProvenance(SyncInfo& s) : m_syncInfo(&s) {} 
        V8FileProvenance(DgnV8FileCR, SyncInfo&, StableIdPolicy);

        public:
        bool IsValid() const {return m_syncId.IsValid();}
        };

    struct FileIterator : BeSQLite::DbTableIterator
        {
        DGNDBSYNC_EXPORT FileIterator(DgnDbCR db, Utf8CP where);
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct FileIterator;
            Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}
        public:
            DGNDBSYNC_EXPORT V8FileSyncInfoId GetV8FileSyncInfoId();
            DGNDBSYNC_EXPORT Utf8String GetUniqueName();
            DGNDBSYNC_EXPORT Utf8String GetV8Name();
            DGNDBSYNC_EXPORT bool GetCannotUseElementIds();
            DGNDBSYNC_EXPORT uint64_t GetLastModifiedTime(); // (Unix time in seconds)
            DGNDBSYNC_EXPORT uint64_t GetFileSize();
            DGNDBSYNC_EXPORT double GetLastSaveTime(); // (Unix time in seconds)
            Entry const& operator* () const {return *this;}
            V8FileProvenance GetV8FileProvenance(SyncInfo& si);
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNDBSYNC_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry (NULL, false);}
        };

    //! Find the file with the specified syncinfo id. Use it like this:
    //! <code>
    //! FileById theFile(db, theFileId);
    //! auto i = theFile.begin();
    //! if (i == theFile.end())
    //!     return not-found;
    //! auto entry = *i;
    //! entry.GetUniqueName(); ...
    //! </code>
    //!     
    struct FileById : FileIterator
        {
        FileById(DgnDbCR db, V8FileSyncInfoId sid) : FileIterator(db, "Id=?")
            {
            GetStatement()->BindInt(1, sid.GetValue());
            }
        };

    // A wrapper around the V8 ModelId. This is unique only within a V8 file.
    struct V8ModelId
        {
    private:
        int m_id;
        bool m_isValid;

    public:
        V8ModelId() : m_id(-1), m_isValid(false) {}
        explicit V8ModelId(int id) : m_id(id), m_isValid(true) {}

        int GetValue() const { return m_id; }
        bool IsValid() const { return m_isValid; }
        };

    // A V8 model syncinfo ID is the ROWID in the syncinfo models table for a particular V8 model ATTACHMENT that we have mapped to a distinct model in the BIM.
    // The same V8 model can show up multiple times in the syncinfo models table. Each occurrence will have a unqiue ROWID/V8ModelSyncInfoId.
    struct V8ModelSyncInfoId
        {
        private:
            uint64_t m_rowid;
            bool m_isValid;

        public:
            V8ModelSyncInfoId() : m_rowid(0), m_isValid(false) {}
            explicit V8ModelSyncInfoId(uint64_t id) : m_rowid(id), m_isValid(true) {BeAssert(id != 0);}

            uint64_t GetValue() const { return m_rowid; }
            bool IsValid() const { return m_isValid; }

            bool operator==(V8ModelSyncInfoId const& rhs) const {return m_rowid == rhs.m_rowid;}
            bool operator!=(V8ModelSyncInfoId const& rhs) const {return !(*this == rhs);}
            bool operator <(V8ModelSyncInfoId const& rhs) const {return m_rowid < rhs.m_rowid;}
        };

    // Identifies a model in a V8 file. Note that this identifies a MODEL, not a V8 attachment of a model.
    struct V8ModelSource
        {
        V8FileSyncInfoId    m_v8FileSyncInfoId;
        V8ModelId   m_modelId;
        V8ModelSource() {}
        V8ModelSource(V8FileSyncInfoId id, V8ModelId model) : m_v8FileSyncInfoId(id), m_modelId(model) {}
        explicit V8ModelSource(DgnV8ModelCR);
        bool IsValid() const {return m_v8FileSyncInfoId.IsValid();}
        V8ModelId GetV8ModelId() const { return m_modelId; }
        V8FileSyncInfoId GetV8FileSyncInfoId() const { return m_v8FileSyncInfoId; }
        DGNDBSYNC_EXPORT bool operator<(V8ModelSource const& rhs) const;
        bool operator==(V8ModelSource const& rhs) const { return m_v8FileSyncInfoId == rhs.m_v8FileSyncInfoId && m_modelId.GetValue() == rhs.m_modelId.GetValue(); }
        bool operator!=(V8ModelSource const& rhs) const { return !(*this == rhs); }
        };

    //! Sync info for a particular attachment of a model. This struct includes all information needed to map a v8 model to a DgnDb model.
    //! Note that many v8 models may be mapped into a single DgnDb model, and 1 v8 model may be mapped to multiple DgnDb models.
    struct V8ModelMapping
        {
        private:
        friend struct ModelIterator;
        friend struct Converter;
        mutable V8ModelSyncInfoId m_syncInfoId; //!< The id assigned to this attachment by syncinfo
        Utf8String  m_v8Name;           //!< The name of the V8 model.
        V8ModelSource m_source;         //!< The V8 ModelId of the model in the source V8 file
        Transform   m_transform;        //!< How the contents of the v8 model are transformed to elements in the DgnDb model
        DgnModelId  m_modelId;          //!< The Id of the DgnDb model to which this v8 model is mapped

        public:
        DGNDBSYNC_EXPORT V8ModelMapping();
        DGNDBSYNC_EXPORT V8ModelMapping(DgnModelId mid, DgnV8ModelCR v8model, TransformCR trans);
        V8ModelMapping(V8ModelMapping const& rhs, TransformCR trans) {*this = rhs; m_transform = trans;}

        bool IsValid() const {return m_syncInfoId.IsValid();}
        BeSQLite::DbResult Insert (BeSQLite::Db&) const;
        BeSQLite::DbResult Update (BeSQLite::Db&) const;
        V8FileSyncInfoId GetV8FileSyncInfoId() const { return m_source.m_v8FileSyncInfoId; }
        V8ModelSyncInfoId GetV8ModelSyncInfoId() const {return m_syncInfoId;}
        DgnModelId GetModelId() const { return m_modelId; }
        V8ModelId GetV8ModelId() const {return m_source.m_modelId;}
        TransformCR GetTransform() const { return m_transform; }
        void SetTransform(TransformCR t) { m_transform = t; }
        V8ModelSource const& GetV8ModelSource() const {return m_source;}
        Utf8StringCR GetV8Name() const {return m_v8Name;}
        };

    struct ModelIterator : BeSQLite::DbTableIterator
        {
        DGNDBSYNC_EXPORT ModelIterator(DgnDbCR db, Utf8CP where);
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct ModelIterator;
            Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}

        public:
            DGNDBSYNC_EXPORT V8ModelSyncInfoId GetV8ModelSyncInfoId(); // aka ROWID
            DGNDBSYNC_EXPORT DgnModelId GetModelId();
            DGNDBSYNC_EXPORT V8FileSyncInfoId GetV8FileSyncInfoId();
            DGNDBSYNC_EXPORT V8ModelId GetV8ModelId();
            DGNDBSYNC_EXPORT Utf8CP GetV8Name();
            DGNDBSYNC_EXPORT Transform GetTransform();
            Entry const& operator* () const {return *this;}

            V8ModelMapping GetMapping()
                {
                V8ModelMapping m;
                m.m_syncInfoId = GetV8ModelSyncInfoId();
                m.m_v8Name = GetV8Name();
                m.m_source = V8ModelSource(GetV8FileSyncInfoId(), GetV8ModelId());
                m.m_transform = GetTransform();
                m.m_modelId = GetModelId();
                return m;
                }
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNDBSYNC_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry (NULL, false);}
        };


    //! Sync info for a level
    struct Level
        {
        enum Type {Spatial=0, Drawing=1}; // *** THESE VALUES ARE PERSISTED IN SYNCINFO. DO NOT CHANGE ***

        DgnSubCategoryId   m_id;
        V8ModelSource   m_fm;
        uint32_t        m_v8Id;
        Type            m_type;
        Utf8String      m_v8Name;

        Level(DgnSubCategoryId id, Type ltype, V8ModelSource fm, uint32_t fid, Utf8CP name) : m_id(id), m_type(ltype), m_fm(fm), m_v8Id(fid), m_v8Name(name) {}
        BeSQLite::DbResult Insert (BeSQLite::Db&) const;

        bool IsValid() const {return m_id.IsValid();}
        bool IsModelSpecific() const {return m_fm.GetV8ModelId().IsValid();}
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

    //! Data that uniquely identifies a V8 *element*. This data is used as part of the input when constructing a SyncInfoAspect.
    //! This corresponds to V8ElementSource, which it will someday replace.
    struct V8ElementSyncInfoAspectData
        {
        DgnModelId m_scope;         // The model that was created (earlier) in the iModel from the V8 model that contains this V8 element.
        DgnV8Api::ElementId m_v8Id; // The V8 element's ID
        ElementProvenance m_prov;   // The V8 element's state
        V8ElementSyncInfoAspectData(DgnModelId scope, DgnV8Api::ElementId v8Id, ElementProvenance const& prov) : m_scope(scope), m_v8Id(v8Id), m_prov(prov) {}
        };
    
    //! Uniquely identifies a V8 element
    struct V8ElementSource
        {
        V8ModelSyncInfoId m_v8ModelSyncInfoId;
        uint64_t m_v8ElementId;

        V8ElementSource(DgnV8EhCR el, V8ModelSyncInfoId modelsiid) :
            m_v8ModelSyncInfoId(modelsiid), m_v8ElementId(el.GetElementId())
            {}

        V8ElementSource(uint64_t v8elid, V8ModelSyncInfoId modelsiid) :
            m_v8ModelSyncInfoId(modelsiid), m_v8ElementId(v8elid)
            {}

        V8ElementSource() : m_v8ElementId(0) {}

        bool IsValid() const { return m_v8ModelSyncInfoId.IsValid() && m_v8ElementId != 0; }
        };

    struct CompareV8ElementSource
        {
        bool operator()(SyncInfo::V8ElementSource const& lhs, SyncInfo::V8ElementSource const& rhs) const 
            {
            if (lhs.m_v8ModelSyncInfoId < rhs.m_v8ModelSyncInfoId)
                return true;
            if (lhs.m_v8ModelSyncInfoId != rhs.m_v8ModelSyncInfoId)
                return false;
            return lhs.m_v8ElementId < rhs.m_v8ElementId;
            }
        };

    typedef BentleyApi::bset<V8ElementSource, CompareV8ElementSource> T_V8ElementSourceSet;
    typedef BentleyApi::bmap<V8ElementSource, T_V8ElementSourceSet, CompareV8ElementSource> T_V8ElementMapOfV8ElementSourceSet;

    //! Full details of how a V8 element is mapped to a BIM element.
    struct V8ElementMapping : V8ElementSource
        {
        DgnElementId m_elementId;
        ElementProvenance m_provenance;

        V8ElementMapping(DgnElementId id, DgnV8EhCR el, V8ModelSyncInfoId modelsiid, ElementProvenance const& lmt) :
            V8ElementSource(el, modelsiid),
            m_elementId(id), m_provenance(lmt)
            {
            }

        V8ElementMapping(DgnElementId id, uint64_t v8elid, V8ModelSyncInfoId modelsiid, ElementProvenance const& lmt) :
            V8ElementSource(v8elid, modelsiid),
            m_elementId(id), m_provenance(lmt)
            {}

        V8ElementMapping() {}

        DgnElementId GetElementId() const {return m_elementId;}
        bool IsValid() const {return V8ElementSource::IsValid() && m_elementId.IsValid();}
        };

    struct CompareV8ElementMappingByElementId
        {
        bool operator()(SyncInfo::V8ElementMapping const& lhs, SyncInfo::V8ElementMapping const& rhs) const { return lhs.m_elementId.GetValueUnchecked() < rhs.m_elementId.GetValueUnchecked(); }
        };

    //! Sync info for an element holds a mapping between and an entity in DgnDb and an element in a v8 model,
    //! plus information about the state of the v8 element's data.
    struct ElementIterator : BeSQLite::DbTableIterator
    {
        DGNDBSYNC_EXPORT ElementIterator(DgnDbCR db, Utf8CP where);

        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
            private:
            friend struct ElementIterator;
            Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}

            public:
            DGNDBSYNC_EXPORT DgnElementId GetElementId() const;
            DGNDBSYNC_EXPORT V8ModelSyncInfoId GetV8ModelSyncInfoId() const;
            DGNDBSYNC_EXPORT uint64_t GetV8ElementId() const;
            DGNDBSYNC_EXPORT ElementProvenance GetProvenance() const;

            V8ElementMapping GetV8ElementMapping() const
                {
                V8ElementMapping m;
                m.m_elementId = GetElementId();
                m.m_v8ModelSyncInfoId = GetV8ModelSyncInfoId();
                m.m_v8ElementId = GetV8ElementId();
                m.m_provenance = GetProvenance();
                return m;
                }

            Entry const& operator* () const {return *this;}
            };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNDBSYNC_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry (NULL, false);}
    };

    struct ByV8ElementIdIter : ElementIterator 
    {
        ByV8ElementIdIter(DgnDbCR db) : ElementIterator(db, "V8ModelSyncInfoId=? AND V8ElementId=?") {}
        void Bind(V8ModelSyncInfoId modelId, uint64_t elId) {m_stmt->Reset(); m_stmt->BindInt64(1, modelId.GetValue()); m_stmt->BindInt64(2, elId);}
    };

    struct ByHashIter : SyncInfo::ElementIterator
    {
        ByHashIter(DgnDbCR db) : ElementIterator(db, "V8ModelSyncInfoId=? AND V8ElementId IS NULL AND Hash=?") {}
        void Bind(V8ModelSyncInfoId modelId, SyncInfo::ElementHash hash) {m_stmt->Reset(); m_stmt->BindInt64(1, modelId.GetValue()); m_stmt->BindBlob(2, hash.m_buffer, sizeof(hash), BeSQLite::Statement::MakeCopy::No);}

    };

    //! The V8 provenance of an element in an iModel. May refer to an element, model, or other object in the v8 source files.
    struct SyncInfoAspect : iModelSyncInfoAspect
        {
      protected:
        friend struct SyncInfo;
        SyncInfoAspect(ECN::IECInstance* i) : iModelSyncInfoAspect(i) {}
      public:
        enum Kind
            {
            Element, Model, DrawingGraphic, Level, GeomPart
            };

        static Utf8CP KindToString(Kind kind)
            {
            switch (kind)
                {
                case Kind::Element: return "Element";
                case Kind::Model: return "Model";
                case Kind::DrawingGraphic: return "DrawingGraphic";
                case Kind::Level: return "Level";
                case Kind::GeomPart: return "GeomPart";
                }
            BeAssert(false);
            return "Element";
            }

        static Kind ParseKind(Utf8CP str)
            {
            if (0==strcmp(str,"Element")) return Kind::Element;
            if (0==strcmp(str,"Model")) return Kind::Model;
            if (0==strcmp(str,"DrawingGraphic")) return Kind::DrawingGraphic;
            if (0==strcmp(str,"Level")) return Kind::Level;
            if (0==strcmp(str,"GeomPart")) return Kind::GeomPart;
            BeAssert(false);
            return Kind::Element;
            }

        DGNDBSYNC_EXPORT SyncInfoAspect::Kind GetKind() const;
        };

    //! A GeomPart mapping
    struct GeomPartSyncInfoAspect : SyncInfoAspect
        {
        private:
        GeomPartSyncInfoAspect(ECN::IECInstance* i) : SyncInfoAspect(i) {}
        public:
        //! Create a new aspect in memory. scopeId should be the bridge's job definition model. Caller must call AddTo, passing in the DgnGeometryPart element.
        DGNDBSYNC_EXPORT static GeomPartSyncInfoAspect Make(DgnElementId scopeId, Utf8StringCR tag, DgnDbR);
        //! Look up the element that has the GeomPart aspect with the specified tag. Note that this is based on the assumption that GeometryPart "tags" are unique within the specified scope!
        DGNDBSYNC_EXPORT static DgnElementId FindElementByTag(DgnDbR db, DgnElementId scopeId, Utf8StringCR tag);
        //! Get an existing GeomPart aspect from the specified DgnGeometryPart
        //! Look up anp existing GeomPart aspect by its partId. el should be the job's definition model element.
        DGNDBSYNC_EXPORT static GeomPartSyncInfoAspect Get(DgnGeometryPartCR el);
        };

    //! Identifies the source of an element in an iModel that was created from an element in a V8 model.
    //! Replacement for V8ElementMapping
    struct V8ElementSyncInfoAspect : SyncInfoAspect
        {
      private:
        V8ElementSyncInfoAspect(ECN::IECInstance* i) : SyncInfoAspect(i) {}
      public:
        static Utf8String FormatSourceId(DgnV8Api::ElementId v8Id) {return Utf8PrintfString("%lld", v8Id);}
        static Utf8String FormatSourceId(DgnV8EhCR el) {return FormatSourceId(el.GetElementId());}

        //! Create a new aspect in memory. Caller must call AddTo.
        DGNDBSYNC_EXPORT static V8ElementSyncInfoAspect Make(V8ElementSyncInfoAspectData const&, DgnDbR);
        
        //! Get an existing syncinfo aspect from the specified element in the case where we know that it was derived from a V8 *element*.
        //! Use this method only in the case where the element is known to have only a single element kind aspect.
        static V8ElementSyncInfoAspect Get(DgnElementR, DgnV8Api::ElementId);
        //! Get an existing syncinfo aspect from the specified element in the case where we know that it was derived from a V8 *element*.
        //! Use this method only in the case where the element is known to have only a single element kind aspect.
        static V8ElementSyncInfoAspect Get(DgnElementCR, DgnV8Api::ElementId);

        DGNDBSYNC_EXPORT void Update(ElementProvenance const& prov); 

        DGNDBSYNC_EXPORT DgnV8Api::ElementId GetV8ElementId() const;

        #ifdef TEST_SYNC_INFO_ASPECT
        void AssertMatch(DgnElementCR, DgnV8Api::ElementId, ElementProvenance const&);
        #endif
        };

    //! Replacement for V8ModelMapping
    struct V8ModelSyncInfoAspect : SyncInfoAspect
        {
      private:
        V8ModelSyncInfoAspect(ECN::IECInstance* i) : SyncInfoAspect(i) {}
      public:
        static Utf8String FormatSourceId(DgnV8Api::ModelId v8Id) {return Utf8PrintfString("%lld", v8Id);}
        static Utf8String FormatSourceId(DgnV8ModelCR model) {return FormatSourceId(model.GetModelId());}

        //! Create a new aspect in memory. Caller must call AddTo.
        DGNDBSYNC_EXPORT static V8ModelSyncInfoAspect Make(DgnV8ModelCR, TransformCR, Converter&);
        
        //! Get an existing syncinfo aspect from the specified Model in the case where we know that it was derived from a V8 *Model*.
        //! Use this method only in the case where the element is known to have only a single model kind aspect.
        DGNDBSYNC_EXPORT static V8ModelSyncInfoAspect Get(DgnElementR, DgnV8Api::ModelId);
        //! Get an existing syncinfo aspect from the specified Model in the case where we know that it was derived from a V8 *Model*.
        //! Use this method only in the case where the element is known to have only a single model kind aspect.
        DGNDBSYNC_EXPORT static V8ModelSyncInfoAspect Get(DgnElementCR, DgnV8Api::ModelId);

        DGNDBSYNC_EXPORT DgnV8Api::ModelId GetV8ModelId() const;
        DGNDBSYNC_EXPORT Transform GetTransform() const;
        DGNDBSYNC_EXPORT Utf8String GetV8ModelName() const;


        #ifdef TEST_SYNC_INFO_ASPECT
        void AssertMatch(V8ModelMapping const&);
        #endif
        };

    //! Provenance info for an element that was \em not converted but was discarded instead.
    struct DiscardedElement
        {
        V8ModelSyncInfoId m_fm;
        uint64_t      m_v8Id;
        DiscardedElement(V8ModelSyncInfoId fm, uint64_t id) : m_fm(fm), m_v8Id(id){}
        };

    //! Import "job" definition
    struct ImportJob
        {
        //!< The type of converter used to create the bim. NB This is persistent data. Do not change.
        enum class Type {RootModels, TiledFile}; 

        private:
        mutable int64_t m_ROWID {};
        SyncInfo::V8ModelSyncInfoId m_v8RootModel;
        Type m_type;
        Utf8String m_prefix;
        DgnElementId m_subjectId;
        Transform m_transform;

        public:
        static Utf8String GetSelectSql();
        void FromSelect(BeSQLite::Statement&);

        BeSQLite::DbResult Insert (BeSQLite::Db&) const;
        BeSQLite::DbResult Update (BeSQLite::Db&) const;
        static BentleyStatus FindById(ImportJob&, DgnDbCR, SyncInfo::V8ModelSyncInfoId);
        static void CreateTable(BeSQLite::Db&);

        SyncInfo::V8ModelSyncInfoId GetV8ModelSyncInfoId() const { return m_v8RootModel; }
        void SetV8ModelSyncInfoId(SyncInfo::V8ModelSyncInfoId i) {m_v8RootModel=i;}
        Type GetType() const {return m_type;}
        void SetType(Type t) {m_type = t;}
        Utf8StringCR GetPrefix() const { return m_prefix; }
        void SetPrefix(Utf8StringCR p) {m_prefix = p;}
        DgnElementId GetSubjectId() const {return m_subjectId;}
        void SetSubjectId(DgnElementId s) {m_subjectId = s;}
        void SetTransform(TransformCR t) {m_transform = t;}
        Transform GetTransform() const {return m_transform;}
        };

    struct ImportJobIterator : BeSQLite::DbTableIterator
        {
        DGNDBSYNC_EXPORT ImportJobIterator(DgnDbCR db, Utf8CP where);

        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
            private:
            friend struct ImportJobIterator;
            Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}

            public:
            DGNDBSYNC_EXPORT ImportJob GetimportJob();
            Entry const& operator* () const {return *this;}
            };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNDBSYNC_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry (NULL, false);}
        };

    //! GeomPart with a converter-generated tag
    struct GeomPart
        {
        DgnGeometryPartId m_id;
        Utf8String m_tag;

        GeomPart() {}
        GeomPart(DgnGeometryPartId i, Utf8StringCR t) : m_id(i), m_tag(t) {}

        static Utf8String GetSelectSql();
        void FromSelect(BeSQLite::Statement&);

        BeSQLite::DbResult Insert (BeSQLite::Db&) const;
        static BentleyStatus FindByTag(GeomPart&, DgnDbCR, Utf8CP tag);
        static BentleyStatus FindById(GeomPart&, DgnDbCR db, DgnGeometryPartId);
        static void CreateTable(BeSQLite::Db&);
        };

    struct GeomPartIterator : BeSQLite::DbTableIterator
        {
        DGNDBSYNC_EXPORT GeomPartIterator(DgnDbCR db, Utf8CP where);

        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
            private:
            friend struct GeomPartIterator;
            Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}

            public:
            DGNDBSYNC_EXPORT GeomPart GetGeomPart();
            Entry const& operator* () const {return *this;}
            };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNDBSYNC_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry (NULL, false);}
        };

    struct ViewIterator : BeSQLite::DbTableIterator
        {
        DGNDBSYNC_EXPORT ViewIterator(DgnDbCR db, Utf8CP where);
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
            private:
                friend struct ViewIterator;
                Entry(BeSQLite::StatementP sql, bool IsValid) : DbTableIterator::Entry(sql, IsValid) {}

            public:
                DGNDBSYNC_EXPORT DgnViewId GetId();
                DGNDBSYNC_EXPORT V8FileSyncInfoId GetV8FileSyncInfoId();
                DGNDBSYNC_EXPORT uint64_t GetV8ElementId();

                Entry const& operator* () const { return *this; }
            };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNDBSYNC_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(NULL, false); }
        };

    enum class ECSchemaMappingType
        {
        Identity = 1, //!< Mapped as is
        Dynamic = 2 //!< if multiple dynamic schemas exist, they will be merged during conversion
        };


    //! An Id that is per-file
    template<typename IDTYPE> struct FileBasedId
        {
        V8FileSyncInfoId m_v8FileSyncInfoId;
        IDTYPE m_id;
        FileBasedId () : m_v8FileSyncInfoId(0), m_id(0) {}
        FileBasedId (V8FileSyncInfoId ffid, IDTYPE id) : m_v8FileSyncInfoId(ffid), m_id(id) {}
        bool operator<(FileBasedId const& rhs) const
            {
            if (m_v8FileSyncInfoId < rhs.m_v8FileSyncInfoId) return true;
            if (m_v8FileSyncInfoId > rhs.m_v8FileSyncInfoId) return false;
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
    //! Optimized for fast look-up
    BentleyStatus FindFirstSubCategory (DgnSubCategoryId&, BeSQLite::Db&, V8ModelSource, uint32_t flid, Level::Type ltype);

    BentleyStatus PerformVersionChecks();

public:
    static bvector<BeSQLite::EC::ECInstanceId> GetSyncInfoAspectIds(DgnElementCR el, SyncInfoAspect::Kind, Utf8StringCR sourceId);

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

    DGNDBSYNC_EXPORT V8FileProvenance InsertFile(BeSQLite::DbResult*, DgnV8FileCR, StableIdPolicy);
    DGNDBSYNC_EXPORT V8FileProvenance UpdateFile(BeSQLite::DbResult*, DgnV8FileCR);
    DGNDBSYNC_EXPORT V8FileProvenance FindFileById(V8FileSyncInfoId);
    DGNDBSYNC_EXPORT V8FileProvenance FindFile(DgnV8FileCR);
    DGNDBSYNC_EXPORT V8FileProvenance FindFileByFileName(BeFileNameCR);
    DGNDBSYNC_EXPORT V8FileProvenance FindFileByUniqueName(Utf8StringCR);
    
    //! @private
    BentleyStatus DeleteFile(V8FileSyncInfoId);

    //! @}

    //! @name V8 Models
    //! @{

    //! Delete all syncinfo for the specified V8 model.
    DGNDBSYNC_EXPORT BentleyStatus DeleteModel(V8ModelSyncInfoId);

    //! @}

    //! @name Elements
    //! @{

    DGNDBSYNC_EXPORT static void SetDumpHash(FILE*);

    DGNDBSYNC_EXPORT void ComputeHash(BentleyApi::MD5& hasher, DgnV8EhCR v8eh);

    //! Remove sync info for an element. The element is assumed to be in the current v8 file.
    //! @note This deletes all rows in sync info having the specified native element id in the current model within the current v8 model.
    //! @param[in] id Identfies the entity in the DgnDb
    //! @return non-zero error status if no sync info could be found for this element or if the syncinfo file could not be updated
    DGNDBSYNC_EXPORT BentleyStatus DeleteElement(DgnElementId id);

    //! Mark a v8 element as not converted.
    //! @param[in] el The element in the DgnV8 file
    //! @param[in] modelsiid Identifies the V8 attachment through which we found this V8 element
    DGNDBSYNC_EXPORT void InsertDiscardedElement(DgnV8EhCR el, V8ModelSyncInfoId modelsiid);

    //! Delete a discard record. The updater should call this when it successfully converts an element that was formerly discarded.
    //! @param[in] v8id  The ID of the element in the v8 repository
    //! @param[in] modelsiid Identifies the V8 attachment through which we found this V8 element
    //! @return non-zero error status if the element was NOT found in the discard table
    DGNDBSYNC_EXPORT BentleyStatus DeleteDiscardedElement (uint64_t v8id, V8ModelSyncInfoId modelsiid);

    //! Query if a v8 element was not converted. The v8 element is assumed to be in the current v8 model in the current v8 file.
    //! @param[in] v8id  The ID of the element in the v8 repository
    //! @param[in] modelsiid Identifies the V8 attachment through which we found this V8 element
    //! @return true if the element was found in the discard table
    DGNDBSYNC_EXPORT bool WasElementDiscarded (uint64_t v8id, V8ModelSyncInfoId modelsiid);

#ifdef TEST_SYNC_INFO_ASPECT
    void AssertAspectMatchesSyncInfo(V8ElementMapping const&);
#endif

    //! @}

    //! @name Levels, Fonts, Styles, Materials
    //! @{

    //! Lookup the first native level mapped to the specified v8 level id. Elements using this level are assumed to be in the current v8 model in the current v8 file.
    //! This function checks first for a model-specific version of the level and then falls back to a file-wide version.
    //! This function returns the default category if all else fails
    DGNDBSYNC_EXPORT DgnSubCategoryId GetSubCategory(uint32_t v8levelid, V8ModelSource, Level::Type ltype);

    //! Find the category to use for the specified DgnV8 element
    //! This function returns the default category if all else fails
    DGNDBSYNC_EXPORT DgnCategoryId GetCategory(DgnV8EhCR, ResolvedModelMapping const&);

    DGNDBSYNC_EXPORT DgnSubCategoryId FindSubCategory(uint32_t v8levelid, V8ModelSource, Level::Type ltype);
    DGNDBSYNC_EXPORT DgnSubCategoryId FindSubCategory(uint32_t v8levelId, V8FileSyncInfoId, Level::Type ltype);
    DGNDBSYNC_EXPORT DgnCategoryId FindCategory(uint32_t v8levelId, V8FileSyncInfoId, Level::Type ltype);

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
    DGNDBSYNC_EXPORT bool TryGetECSchema(ECObjectsV8::SchemaKey&, ECSchemaMappingType&, Utf8CP v8SchemaName, V8FileSyncInfoId fileId) const;
    DGNDBSYNC_EXPORT bool ContainsECSchema(Utf8CP v8SchemaName) const;
    DGNDBSYNC_EXPORT BeSQLite::DbResult RetrieveECSchemaChecksums(bmap<Utf8String, uint32_t>& syncInfoChecksums, V8FileSyncInfoId fileId) const;
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
    DGNDBSYNC_EXPORT BeSQLite::DbResult InsertImageryFile(DgnElementId modeledElementId, V8FileSyncInfoId filesiid, Utf8CP filename, uint64_t lastModifiedTime, uint64_t fileSize, Utf8CP etag, Utf8CP rdsId);

    //! Checks to see if the given V8File has any associated image files and if so, checks each one to see if it has changed
    //! @param[in] fileId - SyncInfo id of the V8File
    DGNDBSYNC_EXPORT bool ModelHasChangedImagery(V8FileSyncInfoId fileId);

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

    //! Create sync info for a DgnV8 model
    //! @param[out] minfo   The newly inserted sync info for the model
    //! @param[in] modelid The model in the DgnDb to which this v8 model is mapped
    //! @param[in] v8model The "v8" DgnV8 model
    //! @param[in] transform The transform from the v8 model to the DgnDb model
    //! @return non-zero error status if the new sync info could not be created
    //! @note that many DgnV8 models may be mapped into a single DgnDb model.
    DGNDBSYNC_EXPORT BentleyStatus InsertModel(V8ModelMapping& minfo, DgnModelId modelId, DgnV8ModelCR v8model, TransformCR transform);

    DGNDBSYNC_EXPORT BentleyStatus FindModel(V8ModelMapping* mapping, DgnV8ModelCR v8Model, TransformCP modelTrans, StableIdPolicy idPolicy);

    DGNDBSYNC_EXPORT BentleyStatus GetModelBySyncInfoId(V8ModelMapping& mapping, V8ModelSyncInfoId modelsiid);

    DGNDBSYNC_EXPORT DgnV8Api::ModelId GetV8ModelIdFromV8ModelSyncInfoId(V8ModelSyncInfoId msiid);

    //! Record a mapping between a DgnV8 element and a DgnDb entity
    //! @param[in] mapping  The mapping to insert
    //! @return Non-zero error status if the insert fails
    //! @note You cannot change the mapping between DgnV8 element and BIM element in an update. You must do delete and add.
    DGNDBSYNC_EXPORT BentleyStatus InsertElement(V8ElementMapping const& mapping);

    //! Update the provenance stored for the specified element in syncinfo
    //! @param[in] mapping  The element to update, including the new provenance info to write
    //! @return The V8ElementMapping if the insert to syncinfo succeeds or an invalid mapping if it failed.
    DGNDBSYNC_EXPORT BentleyStatus UpdateElement(V8ElementMapping const& mapping);

    //! Check if the specified BIM element is mapped to the same V8 element as any element in the specified set.
    DGNDBSYNC_EXPORT bool IsMappedToSameV8Element(DgnElementId, DgnElementIdSet const&) const;

    // *** NEEDS WORK
    DGNDBSYNC_EXPORT bool TryFindElement(DgnElementId&, DgnV8EhCR el) const;

    // ExtractedGraphics: 1 DrawingGraphic per (V8 element and CategoryId)
    //! Record a mapping between a DrawingGraphic and the V8 element that it was derived from. 
    DGNDBSYNC_EXPORT BentleyStatus InsertExtractedGraphic(V8ElementSource const& attachment,
                                                          V8ElementSource const& originalElement,
                                                          DgnCategoryId categoryId, DgnElementId extractedGraphic);
    //! Delete mappings for all extracted graphics associated with the specified V8 element.
    DGNDBSYNC_EXPORT BentleyStatus DeleteExtractedGraphics(V8ElementSource const& attachment,
                                                           V8ElementSource const& originalElement);
    //! Delete mappings for all extracted graphics associated with the specified V8 element and category.
    DGNDBSYNC_EXPORT BentleyStatus DeleteExtractedGraphicsCategory(V8ElementSource const& attachment,
                                                                   V8ElementSource const& originalElement,
                                                                   DgnCategoryId category);
    //! Return the DrawingGraphic that was derived from the specified V8 element. 
    DGNDBSYNC_EXPORT DgnElementId FindExtractedGraphic(V8ElementSource const& attachment,
                                                       V8ElementSource const& originalElement,
                                                       DgnCategoryId categoryId);

    //! Record a mapping between a DgnV8 View and a DgnDb ID
    DGNDBSYNC_EXPORT BeSQLite::DbResult InsertView(DgnViewId viewId, DgnV8ViewInfoCR viewInfo, Utf8CP viewName);

    //! Looks to see if this view has already been converted
    DGNDBSYNC_EXPORT bool TryFindView(DgnViewId&, double& lastModified, Utf8StringR v8ViewName, DgnV8ViewInfoCR viewInfo) const;

    //! Removes a mapping for a view that has been deleted
    DGNDBSYNC_EXPORT BeSQLite::DbResult DeleteView(DgnViewId viewId);

    //! Updates the information for a previously recorded view mapping
    DGNDBSYNC_EXPORT BeSQLite::DbResult UpdateView(DgnViewId viewId, Utf8CP v8ViewName, DgnV8ViewInfoCR viewInfo);

    //! Checks to see if the View syncinfo table exists.
    DGNDBSYNC_EXPORT void ValidateViewTable();

    //! Record sync info for a level.
    //! @param[out] info        Sync info for the level
    //! @param[in]  glevelId    The level's ID in the DgnDb
    //! @param[in]  fmid        If the level is to be used only by elements in a single model, then \a fmid should identify the model. If the level is to be
    //! used by elements in any model in the current v8 file, then pass (-1)
    //! @param[in]  vlevel      The V8 level that was converted
    //! @return non-zero error status if the level could not be inserted in sync info. This would probably be caused by a non-unique name.
    //! @note reports an issue in insertion fails.
    DGNDBSYNC_EXPORT Level InsertLevel(DgnSubCategoryId glevelId, V8ModelSource, DgnV8Api::LevelHandle const& vlevel);

    //! @name ImportJobs - When we convert a DgnV8-based project and store its contents in a DgnDb, that's an "import job".
    //! A given DgnDb can built up by many import jobs. We keep track of import jobs so that we can find them when doing an update from the source V8 files later.
    //! @see ImportJobIterator
    //! @{

    //! Look up a record in the ImportJob table
    //! @param[out] importJob       The import job data, if found
    //! @param[in] modelsiid    Identifies the V8 root model in syncinfo
    DGNDBSYNC_EXPORT BentleyStatus FindImportJobByV8RootModelId(ImportJob& importJob, V8ModelSyncInfoId modelsiid);

    //! Record the fact that the specified importJob has been converted and stored in the DgnDb
    //! @param importJob    The importJob info to insert
    DGNDBSYNC_EXPORT BentleyStatus InsertImportJob(ImportJob const& importJob);

    //! Update the import job's transform, prefix, and subjectid.
    //! @param importJob    The importJob info to update
    DGNDBSYNC_EXPORT BentleyStatus UpdateImportJob(ImportJob const& importJob);
    //! @}
    };

END_DGNDBSYNC_DGNV8_NAMESPACE

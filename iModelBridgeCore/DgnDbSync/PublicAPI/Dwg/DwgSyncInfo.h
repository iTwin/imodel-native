/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Dwg/DwgSyncInfo.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Dwg/DwgImporter.h>

#include <DgnPlatform/DgnPlatform.h>
#include <Bentley/MD5.h>
#include <iterator>                         // std::iterator, std::input_iterator_tag

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_DWGDB

BEGIN_DWG_NAMESPACE

#define SYNCINFO_ATTACH_ALIAS   "SYNCINFO"
#define SYNCINFO_TABLE(name)    "dwgsync_" name
#define SYNCINFO_ATTACH(name)   SYNCINFO_ATTACH_ALIAS "." name

#define SYNC_TABLE_File         SYNCINFO_TABLE("File")
#define SYNC_TABLE_Model        SYNCINFO_TABLE("Model")
#define SYNC_TABLE_Element      SYNCINFO_TABLE("Element")
#define SYNC_TABLE_Layer        SYNCINFO_TABLE("Layer")
#define SYNC_TABLE_Linetype     SYNCINFO_TABLE("Linetype")
#define SYNC_TABLE_Material     SYNCINFO_TABLE("Material")
#define SYNC_TABLE_View         SYNCINFO_TABLE("View")
#define SYNC_TABLE_Group        SYNCINFO_TABLE("Group")
#define SYNC_TABLE_ECSchema     SYNCINFO_TABLE("ECSchema")
#define SYNC_TABLE_Discards     SYNCINFO_TABLE("Discards")
#define SYNC_TABLE_ImportJob    SYNCINFO_TABLE("ImportJob")
#define SYNC_TABLE_Block        SYNCINFO_TABLE("Block")
#define SYNC_TABLE_GeometryPart SYNCINFO_TABLE("GeometryPart")

struct DwgImporter;


//=======================================================================================
// @bsiclass                                                    Sam.Wilson  07/14
//=======================================================================================
struct SyncInfoProperty
{
    struct Spec : BeSQLite::PropertySpec
        {
        Spec(BentleyApi::Utf8CP name) : PropertySpec(name, "DwgSyncInfo", PropertySpec::Mode::Normal, PropertySpec::Compress::No) {}
        };

    static Spec ProfileVersion()      {return Spec("SchemaVersion");}
    static Spec DgnDbGuid()           {return Spec("DgnDbGuid");}
    static Spec DbProfileVersion()    {return Spec("DbSchemaVersion");}
    static Spec DgnDbProfileVersion() {return Spec("DgnDbSchemaVersion");}
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
* SyncInfo to track imported DWG data which may be optionally used for update.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DwgSyncInfo
    {
    //! Information about a file on disk. This struct captures the information that can be extracted from the dwg disk file itself.
    struct DiskFileInfo
        {
        uint64_t                    m_lastModifiedTime; // (Unix time in seconds)
        uint64_t                    m_fileSize;
        BentleyApi::BentleyStatus   GetInfo (BentleyApi::BeFileNameCR);
        DiskFileInfo() {m_lastModifiedTime = m_fileSize = 0;}
        };

    //! Information about a file. This struct captures the information that can be extracted from the dwg file itself.
    struct FileInfo : DiskFileInfo
        {
        BentleyApi::Utf8String  m_uniqueName;
        BentleyApi::Utf8String  m_dwgName;
        BentleyApi::Utf8String  m_versionGuid;
        StableIdPolicy          m_idPolicy;
        double                  m_lastSaveTime;
        FileInfo() {m_lastSaveTime = 0.0; m_idPolicy=StableIdPolicy::ById;}
        };

    struct DwgFileId : BeUInt32Id<DwgFileId,UINT32_MAX>
        {
        DwgFileId() { Invalidate(); }
        explicit DwgFileId (uint32_t u) : BeUInt32Id(u) {}
        static DwgFileId    GetFrom (DwgDbDatabaseR);
        void                CheckValue () const {BeAssert(IsValid());}
        };

    //! Sync info for a dwg file. The struct includes information assigned to the dwg file by the importer.
    //! There should be 1 File entry in syncinfo for each dwg file processed by the importer.
    struct FileProvenance : FileInfo
        {
        DwgSyncInfo&    m_syncInfo;
        DwgFileId       m_syncId;

        FileProvenance(DwgDbDatabaseCR, DwgSyncInfo&, StableIdPolicy);
        FileProvenance(BentleyApi::BeFileNameCR, DwgSyncInfo&, StableIdPolicy);

        BeSQLite::DbResult Insert();
        BeSQLite::DbResult Update();
        bool            FindByName (bool fillLastModData);
        bool            IsValid () const {return m_syncId.IsValid();}
        DwgFileId       GetDwgFileId () const { return m_syncId; }
        StableIdPolicy  GetIdPolicy () const { return m_idPolicy; }
        };

    struct FileIterator : BeSQLite::DbTableIterator
        {
        DWG_EXPORT FileIterator(DgnDbCR db, Utf8CP where);
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct FileIterator;
            Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}
        public:
            DWG_EXPORT DwgFileId GetSyncId();
            DWG_EXPORT Utf8String GetUniqueName();
            DWG_EXPORT Utf8String GetDwgName();
            DWG_EXPORT Utf8String GetVersionGuid();   // changes on each DWG save
            DWG_EXPORT bool GetCannotUseElementIds();
            DWG_EXPORT uint64_t GetLastModifiedTime(); // (Unix time in seconds)
            DWG_EXPORT uint64_t GetFileSize();
            DWG_EXPORT double GetLastSaveTime(); // (Unix time in seconds)
            Entry const& operator* () const {return *this;}
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DWG_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry (NULL, false);}
        };

    struct FileById : FileIterator
        {
        FileById (DgnDbCR db, DwgFileId fileId) : FileIterator(db, "Id=?")
            {
            GetStatement()->BindInt(1, fileId.GetValue());
            }
        };

    struct DwgModelId
        {
        private:
            // a unique ID from DWG object ID, with 0 denotes the dictionary model.
            uint64_t    m_id;
            bool        m_isValid;

        public:
            DwgModelId() : m_id(0), m_isValid(false) {}
            explicit DwgModelId(uint64_t id) : m_id(id), m_isValid(true) {}
            explicit DwgModelId(DwgDbObjectId id) : m_id(id.ToUInt64()), m_isValid(id.IsValid()) {}

            uint64_t    GetValue() const { return m_id; }
            bool        IsValid() const { return m_isValid; }
            void        Invalidate() { m_isValid = false; }
        };

    //! Unique identifier of a DWG model - the row ID of the model syncInfo table
    struct DwgModelSyncInfoId
        {
        private:
            uint64_t    m_rowid;
            bool        m_isValid;

        public:
            DwgModelSyncInfoId() : m_rowid(0), m_isValid(false) {}
            explicit DwgModelSyncInfoId (uint64_t id) : m_rowid(id), m_isValid(true) {BeAssert(id != 0);}

            uint64_t    GetValue() const { return m_rowid; }
            bool        IsValid() const { return m_isValid; }
            void        Invalidate () { m_rowid = 0; m_isValid = false; }

            bool operator==(DwgModelSyncInfoId const& rhs) const {return m_rowid == rhs.m_rowid;}
            bool operator!=(DwgModelSyncInfoId const& rhs) const {return !(*this == rhs);}
            bool operator <(DwgModelSyncInfoId const& rhs) const {return m_rowid < rhs.m_rowid;}
        };  // DwgModelSyncInfoId

    //! A DWG model: a modelspace(including xref's modelspace), a paperspace or a raster image. Any model - not unique.
    struct DwgModelSource
        {
    private:
        DwgFileId    m_fileId;
        DwgModelId   m_modelId;

    public:
        //! constructor for an invalid model
        DwgModelSource() {}
        //! constructor by fileId for a dictionary model
        DwgModelSource(DwgFileId id) : m_fileId(id), m_modelId(0) {}
        //! constructor by explicit file & model ID's
        DwgModelSource(DwgFileId fileId, DwgModelId modelId) : m_fileId(fileId), m_modelId(modelId) {}
        //! constructor by a space model object ID (for an xref, it must be the modelspace ID of the xref).
        explicit DwgModelSource (DwgDbObjectIdCR spaceBlockId);
        //! constructor by a DWG model object. If its an xRef attachment, xrefDwg must be supplied.
        explicit DwgModelSource (DwgDbObjectCP modelObject, DwgDbDatabaseP xrefDwg = nullptr);
        DwgModelId  GetDwgModelId () const {return m_modelId;}
        void        SetDwgModelId (DwgModelId mid) { m_modelId = mid;}
        DwgFileId   GetDwgFileId () const {return m_fileId;}
        void        SetDwgFileId (DwgFileId fid) { m_fileId = fid;}
        DWG_EXPORT bool operator<(DwgModelSource const& rhs) const;
        DWG_EXPORT bool operator==(DwgModelSource const& rhs) const {return m_fileId == rhs.m_fileId && m_modelId.GetValue() == rhs.m_modelId.GetValue();}
        DWG_EXPORT bool operator!=(DwgModelSource const& rhs) const {return !(*this == rhs);}
        DWG_EXPORT bool IsValid () const { return m_fileId.IsValid() && m_modelId.IsValid(); }
        };  // DwgModelSource

    enum class ModelSourceType
        {
        ModelSpace              = 1,    // modelspace
        PaperSpace              = 2,    // paperspace
        XRefAttachment          = 3,    // xRef insert
        RasterAttachment        = 4,    // raster image
        };  // ModelSourceType

    //! Sync info for a unique model/attachment. This struct includes all information needed to map a dwg model to a DgnDb model.
    //! Note that many dwg models may be mapped into a single DgnDb model, and 1 dwg model may be mapped to multiple DgnDb models.
    struct DwgModelMapping
        {
    private:
        mutable DwgModelSyncInfoId  m_syncInfoId;   //!< The ROWID of a DWG model in sync info.
        Utf8String                  m_dwgName;      //!< The name of the dwgmodel.
        DwgModelSource              m_source;       //!< DWG block definition containing entities to be imported.
        Transform                   m_transform;    //!< How the contents of the dwg model are transformed to objects in the DgnDb model
        DgnModelId                  m_id;           //!< The Id of the DgnDb model to which this dwg model is mapped
        ModelSourceType             m_sourceType;   //!< type of a modelspace, paperpsace, xref or a raster
        uint64_t                    m_instanceId;   //!< DWG object ID of a block, an xref insert or a raster image from which this model was created

    public:
        DWG_EXPORT DwgModelMapping ();
        //! map the modelspace or a paperspace to a DgnModel
        DWG_EXPORT DwgModelMapping (DgnModelId mid, DwgDbBlockTableRecordCR block, TransformCR trans);
        //! map an xref insert entity to a DgnModel
        DWG_EXPORT DwgModelMapping (DgnModelId mid, DwgDbBlockReferenceCR xrefInsert, DwgDbDatabaseR xrefDwg, TransformCR trans);
        //! map a raster attachment to a DgnModel
        DWG_EXPORT DwgModelMapping (DgnModelId mid, DwgDbRasterImageCR raster, TransformCR trans);

        BeSQLite::DbResult  Insert (BeSQLite::Db&) const;
        BeSQLite::DbResult  Update (BeSQLite::Db&) const;
        bool                IsValid () const {return m_syncInfoId.IsValid();}
        DwgModelSyncInfoId  GetDwgModelSyncInfoId () const {return m_syncInfoId;}
        void                SetDwgModelSyncInfoId (DwgModelSyncInfoId const& id) {m_syncInfoId=id;}
        uint64_t            GetDwgModelInstanceId () const {return m_instanceId;}
        void                SetDwgModelInstanceId (uint64_t id) {m_instanceId=id;}
        Utf8StringCR        GetDwgName () const {return m_dwgName;}
        void                SetDwgName (Utf8StringCR n) {m_dwgName=n;}
        DwgFileId           GetDwgFileId () const {return m_source.GetDwgFileId();}
        DwgModelId          GetDwgModelId () const {return m_source.GetDwgModelId();}
        DgnModelId          GetModelId () const {return m_id;}
        void                SetModelId (DgnModelId id) {m_id=id;}
        DwgModelSource      GetSource () const {return m_source;}
        void                SetSource (DwgModelSource const& s) {m_source=s;}
        ModelSourceType     GetSourceType () const {return m_sourceType;}
        void                SetSourceType (ModelSourceType const& t) {m_sourceType=t;}
        TransformCR         GetTransform () const {return m_transform;}
        void                SetTransform (TransformCR t) {m_transform=t;}
        };

    struct ModelIterator : BeSQLite::DbTableIterator
        {
        DWG_EXPORT ModelIterator(DgnDbCR db, Utf8CP where);
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct ModelIterator;
            Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}

        public:
            DWG_EXPORT DwgModelSyncInfoId GetDwgModelSyncInfoId ();
            DWG_EXPORT DwgModelMapping GetMapping ();
            DWG_EXPORT DgnModelId GetModelId();
            DWG_EXPORT DwgFileId GetDwgFileId();
            DWG_EXPORT DwgModelId GetDwgModelId();
            DWG_EXPORT Utf8CP GetDwgName();
            DWG_EXPORT ModelSourceType GetSourceType();
            DWG_EXPORT uint64_t GetDwgModelInstanceId();
            DWG_EXPORT Transform GetTransform();
            Entry const& operator* () const {return *this;}
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DWG_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry (NULL, false);}
        };

    //! ImportJob record
    struct ImportJob
        {
        //!< The type of converter used to create the ibim. NB This is persistent data. Do not change.
        enum class Type
            {
            RootModels,
            TiledFile       // not implemented yet
            };

    private:
        DwgModelSyncInfoId  m_dwgRootModel;
        DgnElementId        m_subjectId;
        Transform           m_transform;
        Type                m_type;
        Utf8String          m_prefix;
        mutable int64_t     m_ROWID {};

    public:
        ImportJob () : m_type(Type::RootModels) { m_transform.InitIdentity(); }
        static Utf8String       GetSelectSql ();
        void                    FromSelect (BeSQLite::Statement&);
        BeSQLite::DbResult      Insert (BeSQLite::Db&) const;
        BeSQLite::DbResult      Update (BeSQLite::Db&) const;
        static BentleyStatus    FindById (ImportJob&, DgnDbCR, DwgModelSyncInfoId const&);
        static void             CreateTable (BeSQLite::Db&);
        Type    GetType () const { return m_type; }
        void    SetType (Type t) { m_type = t; }
        Utf8StringCR  GetPrefix () const { return m_prefix; }
        void  SetPrefix (Utf8StringCR p) { m_prefix = p; }
        DgnElementId GetSubjectId () const { return m_subjectId; }
        void SetSubjectId (DgnElementId id) { m_subjectId = id; }
        DwgModelSyncInfoId const& GetDwgModelSyncInfoId () const { return m_dwgRootModel; }
        void SetDwgModelSyncInfoId (DwgModelSyncInfoId const& m) { m_dwgRootModel = m; }
        void SetTransform(TransformCR t) { m_transform = t; }
        Transform GetTransform() const { return m_transform; }
        };  // ImportJob

    struct ImportJobIterator : BeSQLite::DbTableIterator
        {
        DWG_EXPORT ImportJobIterator(DgnDbCR db, Utf8CP where);
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
        private:
            friend struct ImportJobIterator;
            Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}

        public:
            DWG_EXPORT ImportJob GetimportJob();
            Entry const& operator* () const {return *this;}
            };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DWG_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry (NULL, false);}
        };  // ImportJobIterator

    //! Sync info for a layer
    struct Layer
        {
        DgnSubCategoryId    m_id;
        DwgModelSource      m_fm;
        uint64_t            m_dwgId;
        Utf8String          m_dwgName;

        Layer () : m_dwgId(0) {}
        Layer(DgnSubCategoryId id, DwgModelSource fm, uint64_t fid, Utf8CP name) : m_id(id), m_fm(fm), m_dwgId(fid), m_dwgName(name) {}
        BeSQLite::DbResult Insert (BeSQLite::Db&) const;
        DgnSubCategoryId   GetSubCategoryId () const { return m_id; }
        DwgDbHandle        GetLayerHandle () const { return DwgDbHandle(m_dwgId); }

        bool IsValid() const {return m_id.IsValid();}
        bool IsModelSpecific() const {return m_fm.GetDwgModelId().IsValid();}
        };

    struct LayerIterator : BeSQLite::DbTableIterator
        {
        DWG_EXPORT LayerIterator (DgnDbCR db, Utf8CP where);

        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
            private:
                friend struct LayerIterator;
                Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}

            public:
                DWG_EXPORT Layer Get ();
                Entry const& operator* () const { return *this; }
            };  // Entry

        typedef Entry const_iterator;
        typedef Entry iterator;
        DWG_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(nullptr, false); }
        };  // LayerIterator

    //! Sync info for a linetype
    struct Linetype
        {
        DgnStyleId          m_id;
        DwgModelSource      m_fm;
        uint64_t            m_dwgId;
        Utf8String          m_name;
        
        Linetype(DgnStyleId id, DwgModelSource fm, uint64_t fid, Utf8CP name) : m_id(id), m_fm(fm), m_dwgId(fid), m_name(name) {}
        BeSQLite::DbResult Insert (BeSQLite::Db&) const;
        BeSQLite::DbResult Update (BeSQLite::Db&) const;

        bool IsValid() const { return m_id.IsValid(); }
        bool IsModelSpecific() const { return m_fm.GetDwgModelId().IsValid(); }
        };  // Linetype

    //! Sync info for a view created from a viewport table record, a layout viewport, or a viewport entity
    struct View
        {
        enum class Type
            {
            ModelspaceViewport,     // vport table record
            PaperspaceViewport,     // the "overall" layout viewport
            ViewportEntity,         // viewport entity in a layout
            XrefAttachment,         // xRef inserted in a layout
            };  // Type
        DgnViewId   m_id;
        uint64_t    m_dwgId;
        Type        m_type;
        Utf8String  m_name;
        
        View () : m_dwgId(0), m_type(Type::ModelspaceViewport) {}
        View (DgnViewId id, uint64_t oid, Type t, Utf8StringCR n) : m_id(id), m_dwgId(oid), m_type(t), m_name(n) {}
        BeSQLite::DbResult Insert (BeSQLite::Db&) const;
        BeSQLite::DbResult Update (BeSQLite::Db&) const;

        bool IsValid() const { return m_id.IsValid(); }
        Type GetViewType () const { return m_type; }
        DgnViewId GetViewId () const { return m_id; }
        DwgDbHandle GetDwgId () const { return DwgDbHandle(m_dwgId); }
        };  // View

    struct ViewIterator : BeSQLite::DbTableIterator
        {
        DWG_EXPORT ViewIterator (DgnDbCR db, Utf8CP where);

        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
            private:
                friend struct ViewIterator;
                Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}

            public:
                DWG_EXPORT View Get ();
                Entry const& operator* () const { return *this; }
            };  // Entry

        typedef Entry const_iterator;
        typedef Entry iterator;
        DWG_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(nullptr, false); }
        };  // ViewIterator

    struct DwgObjectHash : BentleyApi::MD5::HashVal
        {
        struct HashFiler : IDxfFiler
            {
        private:
            BentleyApi::MD5&        m_hasher;
            DwgDbObjectCR           m_object;
            DwgDbDatabasePtr        m_dwg;

        public:
            explicit HashFiler (BentleyApi::MD5& hasher, DwgDbObjectCR obj) : m_hasher(hasher), m_object(obj), m_dwg(obj.GetDatabase()) {;}
            bool                    IsValid () { return m_dwg.IsValid(); }
            BentleyApi::MD5::HashVal  GetHashValue () { return m_hasher.GetHashVal(); }
            Utf8String              GetHashString () { return m_hasher.GetHashString(); }
            DwgDbStatus             Add (void const* p, size_t nBytes);

            virtual DwgFilerType    _GetFilerType () const override { return DwgFilerType::BagFiler; }
            virtual DwgDbDatabaseP  _GetDatabase () const override { return const_cast<DwgDbDatabaseP>(m_dwg.get()); }
            virtual DwgDbStatus     _Write (DxfGroupCode code, int8_t v) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, int16_t v) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, int32_t v) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, int64_t v) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, uint8_t v) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, uint16_t v) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, uint32_t v) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, uint64_t v) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, bool v) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, double v, DoublePrecision prec = DoublePrecision::Default) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, DwgStringCR v) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, DwgBinaryDataCR v) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, DwgDbHandleCR v) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, DwgDbObjectIdCR v) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, DPoint2dCR v, DoublePrecision prec = DoublePrecision::Default) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, DPoint3dCR v, DoublePrecision prec = DoublePrecision::Default) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, DVec2dCR v, DoublePrecision prec = DoublePrecision::Default) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, DVec3dCR v, DoublePrecision prec = DoublePrecision::Default) override;
            virtual DwgDbStatus     _Write (DxfGroupCode code, double x, double y, double z, DoublePrecision prec = DoublePrecision::Default) override;
            };  // HashFiler
        
        DwgObjectHash () { memset(m_buffer, 0, sizeof BentleyApi::MD5::BlockSize); }
        DwgObjectHash (BentleyApi::MD5::HashVal const& from) : BentleyApi::MD5::HashVal(from) {}
        bool    IsSame (DwgObjectHash const& other) const { return 0==memcmp(m_buffer, other.m_buffer, sizeof(m_buffer)); }
        bool    IsNull () const;
        size_t  AsHexString (Utf8StringR outString);
        };  // DwgObjectHash

    struct DwgObjectProvenance
        {
    private:
        BentleyApi::MD5 m_hasher;
        bool            m_syncAsmBodyInFull;

    public:
        StableIdPolicy  m_idPolicy;
        DwgObjectHash   m_primaryHash;
        DwgObjectHash   m_secondaryHash;

        explicit DwgObjectProvenance (BeSQLite::StatementP);
        DwgObjectProvenance (DwgDbObjectCR, DwgSyncInfo&, StableIdPolicy, bool hash2nd = false);
        DwgObjectProvenance () {}
        BentleyStatus  CreateBlockHash (DwgDbObjectIdCR blockId);
        BentleyStatus  CreateAsmObjectHash (DwgDbObjectCR obj);
        void AppendComplexObjectHash (DwgObjectHash::HashFiler& filer, DwgDbObjectCR obj);
        bool HasSecondaryHash () const { return !m_secondaryHash.IsNull(); }
        bool IsSame (DwgObjectProvenance const& other, bool check2nd = false) const { return m_primaryHash.IsSame(other.m_primaryHash) && (!check2nd || m_secondaryHash.IsSame(other.m_secondaryHash)); }
        DwgObjectHash const& GetPrimaryHash () const { return m_primaryHash; }
        void SetPrimaryHash (DwgObjectHash const& h) { m_primaryHash = h; }
        DwgObjectHash const& GetSecondaryHash () const { return m_secondaryHash; }
        void SetSecondaryHash (DwgObjectHash const& h) { m_secondaryHash = h; }
        };  // DwgObjectProvenance

    //! Uniquely identifies a DWG object
    struct DwgObjectSource
        {
        DwgModelSyncInfoId  m_modelSyncInfoId;
        uint64_t            m_dwgObjectHandle;

        DwgObjectSource (DwgDbObjectCR o, DwgModelSyncInfoId m) : m_modelSyncInfoId(m), m_dwgObjectHandle(o.GetObjectId().ToUInt64()) {}
        DwgObjectSource (uint64_t h, DwgModelSyncInfoId m) : m_modelSyncInfoId(m), m_dwgObjectHandle(h) {}
        DwgObjectSource() : m_dwgObjectHandle(0) {}
        bool IsValid() const { return m_modelSyncInfoId.IsValid() && m_dwgObjectHandle != 0; }
        };  // DwgObjectSource

    struct CompareDwgObjectSource
        {
        bool operator()(DwgSyncInfo::DwgObjectSource const& lhs, DwgSyncInfo::DwgObjectSource const& rhs) const 
            {
            if (lhs.m_modelSyncInfoId < rhs.m_modelSyncInfoId)
                return true;
            if (lhs.m_modelSyncInfoId != rhs.m_modelSyncInfoId)
                return false;
            return lhs.m_dwgObjectHandle < rhs.m_dwgObjectHandle;
            }
        };

    typedef BentleyApi::bset<DwgObjectSource, CompareDwgObjectSource> T_DwgObjectSourceSet;
    typedef BentleyApi::bmap<DwgObjectSource, T_DwgObjectSourceSet, CompareDwgObjectSource> T_DwgObjectMapOfDwgObjectSourceSet;

    //! Full details of how a DWG object is mapped to a BIM element.
    struct DwgObjectMapping : DwgObjectSource
        {
        DgnElementId        m_elementId;
        DwgObjectProvenance m_provenance;

        DwgObjectMapping (DgnElementId id, DwgDbObjectCR o, DwgModelSyncInfoId m, DwgObjectProvenance const& p) :
            DwgObjectSource(o, m), m_elementId(id), m_provenance(p) {}

        DwgObjectMapping (DgnElementId id, uint64_t h, DwgModelSyncInfoId m, DwgObjectProvenance const& p) :
            DwgObjectSource(h, m), m_elementId(id), m_provenance(p) {}

        DwgObjectMapping() {}

        DgnElementId GetElementId() const {return m_elementId;}
        bool IsValid() const {return DwgObjectSource::IsValid() && m_elementId.IsValid();}
        };  // DwgObjectMapping

    struct CompareDwgObjectMappingByElementId
        {
        bool operator()(DwgSyncInfo::DwgObjectMapping const& lhs, DwgSyncInfo::DwgObjectMapping const& rhs) const { return lhs.m_elementId.GetValueUnchecked() < rhs.m_elementId.GetValueUnchecked(); }
        };

    //! Sync info for an object holds a mapping between and an entity in DgnDb and an object in a dwg model,
    //! plus information about the state of the dwg object's data.
    struct ElementIterator : BeSQLite::DbTableIterator
        {
        DWG_EXPORT ElementIterator(DgnDbCR db, Utf8CP where);
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
            private:
                friend struct ElementIterator;
                Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}

            public:
                DWG_EXPORT DgnElementId GetElementId() const;
                DWG_EXPORT DwgFileId GetDwgFileId() const;
                DWG_EXPORT DwgModelSyncInfoId GetDwgModelSyncInfoId() const;
                DWG_EXPORT uint64_t GetDwgObjectId() const;
                DWG_EXPORT DwgObjectProvenance GetProvenance() const;
                DWG_EXPORT DwgObjectMapping GetObjectMapping() const;
                Entry const& operator* () const {return *this;}
            };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DWG_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry (NULL, false);}
        };

    struct ByDwgObjectIdIter : ElementIterator 
        {
        ByDwgObjectIdIter(DgnDbCR db) : ElementIterator(db, "DwgModelSyncInfoId=? AND DwgObjectId=?") {}
        void Bind(DwgModelSyncInfoId const& modelSyncId, uint64_t elId) {m_stmt->Reset(); m_stmt->BindInt64(1, modelSyncId.GetValue()); m_stmt->BindInt64(2, elId);}
        };

    struct ByHashIter : DwgSyncInfo::ElementIterator
        {
        ByHashIter(DgnDbCR db) : ElementIterator(db, "DwgModelSyncInfoId=? AND DwgObjectId IS NULL AND PrimaryHash=? AND SecondaryHash=?") {}
        void Bind (DwgModelSyncInfoId const& modelSyncId, DwgSyncInfo::DwgObjectHash hash1, DwgSyncInfo::DwgObjectHash hash2);
        };

    //! Provenance info for an object that was \em not converted but was discarded instead.
    struct DiscardedDwgObject
        {
        DwgModelSource  m_fm;
        uint64_t        m_dwgId;
        DiscardedDwgObject(DwgModelSource fm, uint64_t id) : m_fm(fm), m_dwgId(id){}
        };

    enum class ECSchemaMappingType
        {
        Identity = 1, //!< Mapped as is
        Dynamic = 2 //!< if multiple dynamic schemas exist, they will be merged during conversion
        };

    //! Sync info for Material
    struct Material
        {
    private:
        BentleyApi::MD5     m_hasher;

    public:
        StableIdPolicy      m_idPolicy;
        RenderMaterialId    m_id;
        DwgFileId           m_fileId;
        uint64_t            m_objectId;
        Utf8String          m_name;
        DwgObjectHash       m_hash;
        
        explicit Material () { m_id.Invalidate(); }
        explicit Material (RenderMaterialId id, DwgFileId fid, StableIdPolicy policy, DwgDbMaterialCR material);
        BeSQLite::DbResult Insert (BeSQLite::Db&) const;
        BeSQLite::DbResult Update (BeSQLite::Db&) const;
        bool IsValid() const { return m_id.IsValid(); }
        bool IsSame (Material const& other) const { return m_name.EqualsI(other.m_name) && m_hash.IsSame(other.m_hash); }
        };  // Material

    // Material iterator
    struct MaterialIterator : BeSQLite::DbTableIterator
        {
        DWG_EXPORT MaterialIterator(DgnDbCR db);
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
        private:
            friend struct MaterialIterator;
            Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}

        public:
            DWG_EXPORT RenderMaterialId GetRenderMaterialId();
            DWG_EXPORT DwgFileId GetDwgFileId();
            DWG_EXPORT uint64_t GetDwgObjectId();
            Entry const& operator* () const {return *this;}
            };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DWG_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry (nullptr, false); }
        };  // MaterialIterator

    //! Sync info for Group
    struct Group
        {
    private:
        BentleyApi::MD5     m_hasher;

    public:
        StableIdPolicy      m_idPolicy;
        DgnElementId        m_id;
        DwgFileId           m_fileId;
        uint64_t            m_objectId;
        Utf8String          m_name;
        DwgObjectHash       m_hash;
        
        DWG_EXPORT explicit Group () { m_id.Invalidate(); }
        DWG_EXPORT explicit Group (DgnElementId id, DwgFileId fid, StableIdPolicy policy, DwgDbGroupCR group, bool addMembers = false);
        DWG_EXPORT BeSQLite::DbResult Insert (BeSQLite::Db&) const;
        DWG_EXPORT BeSQLite::DbResult Update (BeSQLite::Db&) const;
        DWG_EXPORT DgnElementId GetDgnElementId () const { return m_id; }
        DWG_EXPORT uint64_t GetDwgHandleValue () const { return m_objectId; }
        DWG_EXPORT bool IsValid() const { return m_id.IsValid(); }
        bool IsSame (Group const& other) const { return m_hash.IsSame(other.m_hash); }
        };  // Group

    // Group iterator
    struct GroupIterator : BeSQLite::DbTableIterator
        {
        DWG_EXPORT GroupIterator(DgnDbCR db);
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
        private:
            friend struct GroupIterator;
            Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}

        public:
            DWG_EXPORT DgnElementId GetDgnElementId();
            DWG_EXPORT DwgFileId GetDwgFileId();
            DWG_EXPORT uint64_t GetDwgObjectId();
            Entry const& operator* () const {return *this;}
            };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DWG_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(nullptr, false); }
        };  // GroupIterator

    // GeomPart to track & retrieve parts created as shared geometry in db
    struct GeomPart
        {
    private:
        DgnGeometryPartId   m_id;
        Utf8String          m_tag;

    public:
        GeomPart () {}
        GeomPart (DgnGeometryPartId id, Utf8StringCR tag) : m_id(id), m_tag(tag) {}

        BeSQLite::DbResult Insert (BeSQLite::Db& db) const;
        void FromSelect (BeSQLite::Statement& db);
        bool IsValid () const { return m_id.IsValid(); }
        DgnGeometryPartId GetPartId () const { return m_id; }
        void SetPartId (DgnGeometryPartId id) { m_id = id; }
        Utf8StringCR GetPartTag () const { return m_tag; }
        void SetPartTag (Utf8StringCR tag) { m_tag = tag; }
        static Utf8String GetSelectSql ();
        DWG_EXPORT static BentleyStatus FindById (GeomPart& part, DgnDbCR db, DgnGeometryPartId id);
        DWG_EXPORT static BentleyStatus FindByTag (GeomPart& part, DgnDbCR db, Utf8CP tag);
        };  // GeomPart

    struct GeomPartIterator : BeSQLite::DbTableIterator
        {
        DWG_EXPORT GeomPartIterator (DgnDbCR db, Utf8CP where);

        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
            private:
                friend struct GeomPartIterator;
                Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}

            public:
                DWG_EXPORT GeomPart Get ();
                Entry const& operator* () const { return *this; }
            };  // Entry

        typedef Entry const_iterator;
        typedef Entry iterator;
        DWG_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(nullptr, false); }
        };  // GeomPartIterator


    DwgImporter&        m_dwgImporter;
    DgnDb*              m_dgndb;
    BeSQLite::DbResult  m_lastError;
    Utf8String          m_lastErrorDescription;
    bool                m_isValid;

protected:
    bool ComputeHash (BentleyApi::MD5&, DwgDbObjectIdCR, DwgDbObjectCP);
    //! Optimized for fast look-up
    BentleyStatus FindFirstSubCategory (DgnSubCategoryId&, DwgModelSource, uint64_t flid);

    DWG_EXPORT BeSQLite::DbResult InsertFile(FileProvenance&, FileInfo const&);

    BentleyStatus PerformVersionChecks();

public:
    BentleyStatus CreateTables();

    DWG_EXPORT BeSQLite::DbResult SavePropertyString (BeSQLite::PropertySpec const& spec, Utf8CP stringData, uint64_t majorId=0, uint64_t subId=0);
    DWG_EXPORT BeSQLite::DbResult QueryProperty (Utf8StringR str, BeSQLite::PropertySpec const& spec, uint64_t id=0, uint64_t subId=0) const;

    //! Construct a DwgSyncInfo object to use for the specified project.
    DwgSyncInfo (DwgImporter&);
    DWG_EXPORT ~DwgSyncInfo ();

    DgnDb* GetDgnDb () {return m_dgndb;}

    //! Get the name of the .syncinfo file
    DWG_EXPORT static BeFileName GetDbFileName (DgnDb&);
    DWG_EXPORT static BeFileName GetDbFileName (BeFileNameCR);

    //! Call this to attach a synchinfo file to a project.
    DWG_EXPORT BentleyStatus AttachToProject (DgnDb& targetProject, BeFileNameCR dbName);

    //! This function associates this DwgSyncInfoBase object with the specified project.
    //! Call this after attaching an existing .syncinfo file to the specified project.
    //! This function either finishes creating a new syncInfo or it validates an existing syncinfo.
    //! @return non-zero if initialization or verification failed.
    //! @param[in] project The project.
    BentleyStatus OnAttach(DgnDb& project);

    //! Create an empty .syncinfo file. Then attach it to the project.
    //! @param[in] dbName The name of the syncinfo file
    //! @param[in] deleteIfExists If true, any existing .syncinfo file with the same name is deleted
    DWG_EXPORT static BentleyStatus CreateEmptyFile (BeFileNameCR dbName, bool deleteIfExists=true);

    //! Query if the DwgSyncInfo is valid (created or read)
    bool IsValid() const {return m_isValid;}

    //! Mark the DwgSyncInfo as valid or not.
    void SetValid (bool b) {m_isValid=b;}

    DWG_EXPORT void SetLastError (BeSQLite::DbResult);
    DWG_EXPORT void GetLastError (BeSQLite::DbResult&, Utf8String&);

    //! @name DWG Files
    //! @{

    //! Get information about a disk file
    DWG_EXPORT static DwgFileId   GetDwgFileId (DwgDbDatabaseR);
    static StableIdPolicy   GetFileIdPolicy (DwgDbDatabaseR);
    Utf8String  GetUniqueName (WStringCR fullname);
    DWG_EXPORT BentleyStatus  DeleteFile (DwgFileId fileId);

    //! Query if the specified file appears to have changed, compared to save data in syncinfo
    //! @return true if the file is not found in syncinfo or if it is found and its last-save time is different
    DWG_EXPORT bool HasLastSaveTimeChanged (DwgDbDatabaseCR);

    //! Query if a dwg file's disk file might have changed since syncinfo was created or last updated
    //! @param[in] dwgFileName  the name of the file on disk
    DWG_EXPORT bool HasDiskFileChanged(BeFileNameCR dwgFileName);

    //! Query if a dwg file's version GUID might have changed since syncinfo was created or last updated
    //! @param[in] dwg  DWG database
    DWG_EXPORT bool HasVersionGuidChanged (DwgDbDatabaseCR dwg);
    //! @}

    //! @name DWG Models/Layouts
    //! @{

    //! Delete all syncinfo for the specified DWG model.
    DWG_EXPORT BentleyStatus DeleteModel (DwgSyncInfo::DwgModelSyncInfoId const&);

    //! Create sync info for a DWG model/layout/xref/raster
    //! @param[out] minfo   The newly inserted sync info for the model
    //! @param[in] modelId  The model in the DgnDb to which this dwg layout block model is mapped
    //! @param[in] block The dwg layout block model (a modelspace or a paperspace)
    //! @param[in] transform The transform from the dwg layout block model to the DgnDb model
    //! @return non-zero error status if the new sync info could not be created
    DWG_EXPORT BentleyStatus InsertModel (DwgModelMapping& minfo, DgnModelId modelId, DwgDbBlockTableRecordCR block, TransformCR transform);
    //! Create sync info for an xref model
    //! @param[out] minfo The newly inserted sync info for the model
    //! @param[in] modelId The model in the DgnDb to which this dwg layout block model is mapped
    //! @param[in] xrefInsert The dwg xref attachment
    //! @param[in] xrefDwg The dwg xref file/database
    //! @param[in] transform The transform the DWG xref to the DgnModel
    DWG_EXPORT BentleyStatus InsertModel (DwgModelMapping& minfo, DgnModelId modelId, DwgDbBlockReferenceCR xrefInsert, DwgDbDatabaseR xrefDwg, TransformCR transform);
    //! Create sync info for a DWG raster model
    //! @param[out] minfo The newly inserted sync info for the model
    //! @param[in] modelId The model in the DgnDb to which this dwg layout block model is mapped
    //! @param[in] image The dwg raster attachment
    //! @param[in] transform The transform from the dwg raster to the raster model
    DWG_EXPORT BentleyStatus InsertModel (DwgModelMapping& minfo, DgnModelId modelId, DwgDbRasterImageCR image, TransformCR transform);
    //! Find DgnModel, optionally matching a transformation
    //! @param[out] mapping The sync info found for the model
    //! @param[in] id The dwg object ID
    //! @param[in] trans The transformation for the model, optional
    DWG_EXPORT BentleyStatus FindModel (DwgModelMapping* mapping, DwgDbObjectIdCR id, TransformCP trans);
    //! Find DgnModel by DwgModelSyncInfoId
    //! @param[out] mapping The sync info found for the model
    //! @param[in] syncInfoId The syncInfo ID for the model
    DWG_EXPORT BentleyStatus FindModel (DwgModelMapping* mapping, DwgModelSyncInfoId syncInfoId);

    //! @}

    //! @name DwgObjects
    //! @{
    //! delete sync info for an object in the current dwg model.
    //! @note This deletes all rows in sync info having the specified dwg object id in the current dwg model.
    //! @param[in] dwgid  The ID of the object in the dwg repository
    //! @return non-zero error code if the object is not found in sync info for the current dwg model.
    DWG_EXPORT BentleyStatus DeleteElement (DwgDbObjectIdCR, DwgModelSyncInfoId const& modelSyncId);

    //! Remove sync info for a DWG object. The object is assumed to be in the current dwg file.
    //! @note This deletes all rows in sync info having the specified native object id in the current model within the current dwg model.
    //! @param[in] id Identfies the entity in the DgnDb
    //! @return non-zero error status if no sync info could be found for this object or if the syncinfo file could not be updated
    DWG_EXPORT BentleyStatus DeleteElement (DgnElementId id);

    //! Mark a dwg object as not converted.
    DWG_EXPORT void InsertDiscardedDwgObject (DwgDbEntityCR, DwgModelSyncInfoId const& modelSyncId);

    //! Delete a discard record. The updater should call this when it successfully converts an object that was formerly discarded.
    //! @param[in] dwgId  The ID of the object in the dwg repository
    //! @param[in] modelSyncId  The raw ID of the model in DwgSyncInfo table
    DWG_EXPORT BentleyStatus DeleteDiscardedDwgObject (DwgDbObjectIdCR dwgId, DwgModelSyncInfoId const& modelSyncId);

    //! Query if a dwg object was not converted. The dwg object is assumed to be in the current dwg model in the current dwg file.
    //! @param[in] dwgId  The ID of the object in the dwg repository
    //! @param[in] modelSyncId  The raw ID of the model in DwgSyncInfo table
    DWG_EXPORT bool WasDwgObjectDiscarded (DwgDbObjectIdCR dwgId, DwgModelSyncInfoId const& modelSyncId);
    //! @}

    //! @name Layers, Materials
    //! @{

    //! Lookup the first native layer mapped to the specified dwg layer id. Objects using this layer are assumed to be in the current dwg model in the current dwg file.
    //! This function checks first for a model-specific version of the layer and then falls back to a file-wide version.
    //! This function returns the default category if all else fails
    DWG_EXPORT DgnSubCategoryId GetSubCategory (DwgDbObjectIdCR layerId, DwgModelSource);

    //! Find the category to use for the specified DWG entity (i.e. a graphic object only)
    //! This function returns the default category if all else fails
    DWG_EXPORT DgnCategoryId GetCategory (DwgDbEntityCR);
    DWG_EXPORT DgnCategoryId GetCategory (DwgDbObjectIdCR layerId, DwgDbDatabaseP dwg);
    DWG_EXPORT DgnSubCategoryId GetSubCategory (DwgDbObjectIdCR layerId, DwgDbDatabaseP dwg);
    DWG_EXPORT DgnSubCategoryId FindSubCategory (DwgDbObjectIdCR layerId, DwgModelSource model);
    DWG_EXPORT DgnSubCategoryId FindSubCategory (DwgDbObjectIdCR layerId, DwgFileId fileId);
    DWG_EXPORT DgnCategoryId FindCategory (DwgDbObjectIdCR layerId, DwgFileId fileId);
    DWG_EXPORT DwgDbHandle FindLayerHandle (DgnCategoryId categoryId, DwgFileId fileId);

    //! Query sync info for a dwg material in the current dwg file (applies to StableIdPolicy::ById)..
    DWG_EXPORT bool FindMaterial (Material& out, DwgDbObjectIdCR id);
    DWG_EXPORT Material InsertMaterial (RenderMaterialId id, DwgDbMaterialCR material);
    DWG_EXPORT BentleyStatus UpdateMaterial (DwgSyncInfo::Material& prov);
    DWG_EXPORT BentleyStatus DeleteMaterial (RenderMaterialId id);

    //! Query sync info for a DWG group (applies to StableIdPolicy::ById).
    //! @param[out] out The sync info about the requested group
    //! @param[in] groupId The input DWG group object ID for which a sync info is queried
    DWG_EXPORT bool FindGroup (DwgSyncInfo::Group& out, DwgDbObjectIdCR groupId);
    //! Insert group into the syncInfo
    //! @param[in] id The ID of the DgnElement converted for the DWG group object
    //! @param[in] group DWG group object
    DWG_EXPORT Group InsertGroup (DgnElementId id, DwgDbGroupCR group);
    DWG_EXPORT BentleyStatus UpdateGroup (DwgSyncInfo::Group& syncGroup);
    DWG_EXPORT BentleyStatus DeleteGroup (DgnElementId id);

    //! @}

    DwgImporter& GetDwgImporter() const {return m_dwgImporter;}

    //! Record a mapping between a DWG object and a DgnDb entity
    //! @param[in] id  The ElementId in the DgnDb
    //! @param[in] en  The entity in the DWG file
    //! @param[in] lmt primary & data hash, etc.
    //! @param[in] modelSyncId The raw ID of the DWG Model in the syncinfo table
    //! @return non-zero error status if the insert to syncinfo failed.
    //! Note you cannot change the mapping between Dwg object and DgnDb element in an update. You must do delete and add.
    DWG_EXPORT BentleyStatus InsertElement (DgnElementId id, DwgDbEntityCR en, DwgObjectProvenance const& lmt, DwgModelSyncInfoId const& modelSyncId);
    DWG_EXPORT BentleyStatus UpdateElement (DgnElementId id, DwgDbEntityCR en, DwgObjectProvenance const& lmt);
    DWG_EXPORT bool TryFindElement (DgnElementId&, DwgDbObjectCP, DwgModelSyncInfoId const&) const;
    //! Find all elements mapped from the input object and in models mapped from the same DWG file.
    DWG_EXPORT bool FindElements (DgnElementIdSet&, DwgDbObjectIdCR) const;
    //! Check if the specified BIM element is mapped to the same DWG object as any element in the specified set.
    DWG_EXPORT bool IsMappedToSameDwgObject (DgnElementId elementId, DgnElementIdSet const& known) const;

    //! Record sync info for a layer.
    //! @param[in]  subId The layer's sub category ID in DgnDb
    //! @param[in]  ms The DWG model for the sub category
    //! @param[in]  layer The DWG layer
    //! @return non-zero error status if the layer could not be inserted in sync info. This would probably be caused by a non-unique name.
    //! @note reports an issue in insertion fails.
    DWG_EXPORT Layer InsertLayer (DgnSubCategoryId subId, DwgModelSource ms, DwgDbLayerTableRecordCR layer);

    //! Record sync info for a linetype.
    //! @param[in]  id          DgnDb linestyle ID
    //! @param[in]  model       DWG model source
    //! @param[in]  ltype       DWG linetype object
    DWG_EXPORT Linetype       InsertLinetype (DgnStyleId id, DwgModelSource const& model, DwgDbLinetypeTableRecordCR ltype);
    DWG_EXPORT Linetype       InsertLinetype (DgnStyleId id, DwgDbLinetypeTableRecordCR ltype);
    DWG_EXPORT BentleyStatus  UpdateLinetype (DgnStyleId id, DwgDbLinetypeTableRecordCR ltype);
    DWG_EXPORT DgnStyleId     FindLineStyle (DwgDbObjectIdCR linetypId, DwgModelSource const& model);
    DWG_EXPORT DgnStyleId     FindLineStyle (DwgDbObjectIdCR linetypId);
    //! @}
    
    //! Record sync info for a ViewDefinition created from a modelspace viewport, the paperspace viewport, or a viewport entity
    //! @param[in]  id          DgnDb ViewDefinition ID
    //! @param[in]  objId       object ID of a DWG viewport table record or a viewport entity
    //! @param[in]  type        Source viewport type - modelspace, paperspace, or viewport entity
    //! @param[in]  name        View name
    DWG_EXPORT View           InsertView (DgnViewId id, DwgDbObjectIdCR objId, View::Type type, Utf8StringCR name);
    DWG_EXPORT BentleyStatus  UpdateView (DgnViewId id, DwgDbObjectIdCR objId, View::Type type, Utf8StringCR name);
    //! Query syncInfo by DWG ID and source
    //! @param[in]  vportId     entityId of a viewport table record or a viewport entity
    //! @param[in]  type        viewport table record, paperspace viewport or viewport entity
    DWG_EXPORT DgnViewId      FindView (DwgDbObjectIdCR vportId, View::Type type);
    DWG_EXPORT DwgDbHandle    FindViewportHandle (DgnViewId viewId);
    DWG_EXPORT BentleyStatus  DeleteView (DgnViewId viewId);
    //! @}
    
    //! @name ImportJobs - When we convert a DgnV8-based project and store its contents in a DgnDb, that's an "import job".
    //! A given DgnDb can built up by many import jobs. We keep track of import jobs so that we can find them when doing an update from the source V8 files later.
    //! @see ImportJobIterator
    //! @{

    //! Look up a record in the ImportJob table
    //! @param[out] importJob   The gtest data, if found
    //! @param[in] dwgModel      Identifies the DWG root model in syncinfo
    DWG_EXPORT BentleyStatus FindImportJobById(ImportJob& importJob, DwgModelSyncInfoId const& dwgModel);

    //! Record the fact that the specified importJob has been converted and stored in the DgnDb
    //! @param importJob    The importJob info to insert
    DWG_EXPORT BentleyStatus InsertImportJob(ImportJob const& importJob);
    //! Update the import job's transform, prefix, and subjectid.
    //! @param importJob    The importJob info to update
    DWG_EXPORT BentleyStatus UpdateImportJob(ImportJob const& importJob);
    //! @}
    };  // DwgSyncInfo

END_DWG_NAMESPACE

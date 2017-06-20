/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimImporter/lib/SyncInfo.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/BeId.h>
#include <BimTeleporter/BimTeleporter.h>

BEGIN_BIM_TELEPORTER_NAMESPACE

#define SYNCINFO_ATTACH_ALIAS "SYNCINFO"
#define SYNCINFO_TABLE(name)  "v8sync_" name
#define SYNCINFO_ATTACH(name) SYNCINFO_ATTACH_ALIAS "." name

#define SYNC_TABLE_File         SYNCINFO_TABLE("File")
#define SYNC_TABLE_Model        SYNCINFO_TABLE("Model")
#define SYNC_TABLE_Element      SYNCINFO_TABLE("Element")
#define SYNC_TABLE_ECSchema     SYNCINFO_TABLE("ECSchema")
#define SYNC_TABLE_SubCategory  SYNCINFO_TABLE("SubCategory")

struct BisJson1ImporterImpl;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson  07/14
//=======================================================================================
struct SyncInfoProperty
    {
    struct Spec : BeSQLite::PropertySpec
        {
        Spec(BentleyApi::Utf8CP name) : PropertySpec(name, "SyncInfo", PropertySpec::Mode::Normal, PropertySpec::Compress::No) {}
        };

    static Spec ProfileVersion() { return Spec("SchemaVersion"); }
    static Spec DgnDbGuid() { return Spec("DgnDbGuid"); }
    static Spec DbProfileVersion() { return Spec("DbSchemaVersion"); }
    static Spec DgnDbProfileVersion() { return Spec("DgnDbSchemaVersion"); }
    };

//---------------------------------------------------------------------------------------
// Stores information needed to synchronize data that is created in a Bim from an earlier generation.
//---------------+---------------+---------------+---------------+---------------+-------

struct SyncInfo
    {
    public:
        //! Information about a file on disk. This struct captures the information that can be extracted from the db disk file itself.
        struct DiskFileInfo
            {
            uint64_t m_lastModifiedTime; // (Unix time in seconds)
            uint64_t m_fileSize;
            BentleyApi::BentleyStatus GetInfo(BentleyApi::BeFileNameCR);
            DiskFileInfo() { m_lastModifiedTime = m_fileSize = 0; }
            };

        //! Information about a file.  This struct captures the information that can be extracted from the db file itself.
        struct FileInfo : DiskFileInfo
            {
            BentleyApi::Utf8String m_fileName;
            BeSQLite::ProfileVersion m_dbVersion;
            BeSQLite::ProfileVersion m_ecVersion;
            BeSQLite::ProfileVersion m_dgnPrjVersion;
            FileInfo() : m_dbVersion(0, 0, 0, 0), m_ecVersion(0, 0, 0, 0), m_dgnPrjVersion(0, 0, 0, 0) {}
            };

        struct DbFileId : BeUInt32Id<DbFileId, UINT32_MAX>
            {
            DbFileId() { Invalidate(); }
            explicit DbFileId(uint32_t u) : BeUInt32Id(u) {}
            void CheckValue() const { BeAssert(IsValid()); }
            };

        //! Sync info for a previous generation of db
        struct DbFileProvenance : FileInfo
            {
            SyncInfo& m_syncInfo;
            DbFileId m_syncId;

            DbFileProvenance(BentleyApi::BeFileNameCR dbFileName, Json::Value& fileInfo, SyncInfo& syncInfo);

            BeSQLite::DbResult Insert();
            BeSQLite::DbResult Update();
            bool FindByName(bool fillLastModData);
            bool IsValid() const {return m_syncId.IsValid(); }
            };

        struct FileIterator : BeSQLite::DbTableIterator
            {
            FileIterator(DgnDbCR db, Utf8CP where);
            struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
                {
                private:
                    friend struct FileIterator;
                    Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) {}
                public:
                    DbFileId GetV8FileSyncInfoId();
                    Utf8String GetFileName();
                    uint64_t GetLastModifiedTime();
                    uint64_t GetFileSize();
                    BeSQLite::ProfileVersion GetDbVersion();
                    BeSQLite::ProfileVersion GetECVersion();
                    BeSQLite::ProfileVersion GetDgnPrjVersion();
                    Entry const& operator* () const { return *this; }
                };
            typedef Entry const_iterator;
            typedef Entry iterator;
            const_iterator begin() const;
            const_iterator end() const { return Entry(NULL, false); }
            };

        struct ModelMapping
            {
            private:
                mutable int64_t m_rowId;            //!< The ROWID of this sync info.  Note this is not the Id of either version of the Model
                Utf8String m_name;                  //!< The name of the model
                DgnModelId m_sourceId;                 //!< The Id of the Model in the source db
                DgnModelId m_targetId;                 //!< The id of the Model in the target db

            public:
                ModelMapping();
                ModelMapping(DgnModelId source, DgnModelId target, Utf8String name);

                BeSQLite::DbResult Insert(BeSQLite::Db& db) const;
                DgnModelId GetSourceId() { return m_sourceId; }
                DgnModelId GetTargetId() { return m_targetId; }
                Utf8String GetName() { return m_name; }
            };

        struct ModelIterator : BeSQLite::DbTableIterator
            {
            ModelIterator(DgnDbCR db, Utf8CP where);
            struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
                {
                private:
                    friend struct ModelIterator;
                    Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) {}

                public:
                    int64_t GetRowId();
                    DgnModelId GetSourceId();
                    DgnModelId GetTargetId();
                    Utf8CP GetName();
                    Entry const& operator* () const { return *this; }
                };
            typedef Entry const_iterator;
            typedef Entry iterator;
            const_iterator begin() const;
            const_iterator end() const { return Entry(NULL, false); }
            };

        struct ElementMapping
            {
            private:
                mutable int64_t m_rowId;            //!< The ROWID of this sync info.  Note this is not the Id of either version of the Element
                DgnElementId m_sourceId;                 //!< The Id of the Element in the source db
                DgnElementId m_targetId;                 //!< The id of the Element in the target db

            public:
                ElementMapping();
                ElementMapping(DgnElementId source, DgnElementId target);

                BeSQLite::DbResult Insert(BeSQLite::Db& db) const;
                DgnElementId GetSourceId() { return m_sourceId; }
                DgnElementId GetTargetId() { return m_targetId; }
            };

        struct ElementIterator : BeSQLite::DbTableIterator
            {
            ElementIterator(DgnDbCR db, Utf8CP where);
            struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
                {
                private:
                    friend struct ElementIterator;
                    Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) {}

                public:
                    int64_t GetRowId();
                    DgnElementId GetSourceId();
                    DgnElementId GetTargetId();
                    Utf8CP GetName();
                    Entry const& operator* () const { return *this; }
                };
            typedef Entry const_iterator;
            typedef Entry iterator;
            const_iterator begin() const;
            const_iterator end() const { return Entry(NULL, false); }
            };

    private:
        BisJson1ImporterImpl& m_importer;
        DgnDb* m_dgnDb;
        BeSQLite::DbResult m_lastError;
        Utf8String m_lastErrorDescription;
        bool m_isValid;
        bmap<CodeSpecId, CodeSpecId> m_codeSpecMap;
        bmap<DgnCategoryId, DgnCategoryId> m_categoryMap;

    protected:
        BeSQLite::DbResult InsertFile(DbFileProvenance& provenance, FileInfo const& info);
        BentleyStatus PerformVersionChecks();

    public:
        BentleyStatus CreateTables();

        BeSQLite::DbResult SavePropertyString(BeSQLite::PropertySpec const& spec, Utf8CP stringData, uint64_t majorId = 0, uint64_t subId = 0);
        BeSQLite::DbResult QueryProperty(Utf8StringR str, BeSQLite::PropertySpec const& spec, uint64_t id = 0, uint64_t subId = 0) const;

        //! Construct a SyncInfo to use for the specified project
        SyncInfo(BisJson1ImporterImpl& importer);
        ~SyncInfo();

        DgnDb* GetDgnDb() { return m_dgnDb; }

        //! Get the name of the .syncinfo file
        static BeFileName GetDbFileName(DgnDb&);
        static BeFileName GetDbFileName(BeFileName const&);

        //! Call this to attach a synchinfo file to a project.
        BentleyStatus AttachToProject(DgnDb& targetProject, BeFileNameCR dbName);

        //! This function associates this SyncInfoBase object with the specified project.
        //! Call this after attaching an existing .syncinfo file to the specified project.
        //! This function either finishes creating a new syncInfo or it validates an existing syncinfo.
        //! @return non-zero if initialization or verification failed.
        //! @param[in] project The project.
        BentleyStatus OnAttach(DgnDb& project);

        //! Create an empty .syncinfo file. Then attach it to the project.
        //! @param[in] dbName The name of the syncinfo file
        //! @param[in] deleteIfExists If true, any existing .syncinfo file with the same name is deleted
        static BentleyStatus CreateEmptyFile(BeFileNameCR dbName, bool deleteIfExists = true);

        //! Query if the SyncInfo is valid (created or read)
        bool IsValid() const { return m_isValid; }

        //! Mark the SyncInfo as valid or not.
        void SetValid(bool b) { m_isValid = b; }

        void SetLastError(BeSQLite::DbResult);
        void GetLastError(BeSQLite::DbResult&, Utf8String&);

        //! Query if a db file's disk file might have changed since syncinfo was created or last updated
        bool HasDiskFileChanged(BeFileNameCR dbFileName, Json::Value& jsonFile);

        BisJson1ImporterImpl& GetImporter() const { return m_importer; }

        //! Create sync info for a dgndb model
        //! @param[out] minfo   The newly inserted sync info for the model
        //! @param[in] targetId The model id of the target Bis model
        //! @param[in[ sourceId The model id of the source dgndb model
        BentleyStatus InsertModel(ModelMapping& minfo, DgnModelId target, DgnModelId source, Utf8String name);

        BentleyStatus InsertCodeSpec(CodeSpecId oldId, CodeSpecId newId);
        CodeSpecId LookupCodeSpec(CodeSpecId oldId);

        BentleyStatus InsertCategory(DgnCategoryId oldId, DgnCategoryId newId);
        DgnCategoryId LookupCategory(DgnCategoryId oldId);

        BentleyStatus InsertSubCategory(DgnCategoryId catId, DgnSubCategoryId oldId, DgnSubCategoryId newId);
        DgnSubCategoryId LookupSubCategory(DgnCategoryId catId, DgnSubCategoryId oldId);

        DgnElementId LookupElement(DgnElementId oldId);
        DgnModelId LookupModel(DgnModelId oldId);

    };
END_BIM_TELEPORTER_NAMESPACE
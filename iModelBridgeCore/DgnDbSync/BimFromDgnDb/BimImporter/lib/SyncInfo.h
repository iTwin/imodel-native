/*--------------------------------------------------------------------------------------+
|
|     $Source: BimFromDgnDb/BimImporter/lib/SyncInfo.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <Bentley/BeId.h>
#include <BimFromDgnDb/BimFromDgnDb.h>

BEGIN_BIM_FROM_DGNDB_NAMESPACE

#define SYNCINFO_ATTACH_ALIAS "SYNCINFO"
#define SYNCINFO_TABLE(name)  "G0601sync_" name
#define SYNCINFO_ATTACH(name) SYNCINFO_ATTACH_ALIAS "." name

#define SYNC_TABLE_File         SYNCINFO_TABLE("File")
#define SYNC_TABLE_Model        SYNCINFO_TABLE("Model")
#define SYNC_TABLE_Element      SYNCINFO_TABLE("Element")
#define SYNC_TABLE_ECSchema     SYNCINFO_TABLE("ECSchema")
#define SYNC_TABLE_SubCategory  SYNCINFO_TABLE("SubCategory")

struct BimFromJsonImpl;

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
        BimFromJsonImpl& m_importer;
        DgnDb* m_dgnDb;
        BeSQLite::DbResult m_lastError;
        Utf8String m_lastErrorDescription;
        bool m_isValid;
        bmap<CodeSpecId, CodeSpecId> m_codeSpecMap;
        bmap<DgnCategoryId, DgnCategoryId> m_categoryMap;
        bmap<DgnFontId, DgnFontId> m_fontMap;
        bmap<DgnStyleId, DgnStyleId> m_styleMap;
        bmap<LsComponentId, LsComponentId> m_componentMap;

    protected:
        BentleyStatus PerformVersionChecks();

    public:
        BentleyStatus CreateTables();

        BeSQLite::DbResult SavePropertyString(BeSQLite::PropertySpec const& spec, Utf8CP stringData, uint64_t majorId = 0, uint64_t subId = 0);
        BeSQLite::DbResult QueryProperty(Utf8StringR str, BeSQLite::PropertySpec const& spec, uint64_t id = 0, uint64_t subId = 0) const;

        //! Construct a SyncInfo to use for the specified project
        SyncInfo(BimFromJsonImpl& importer);
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

        BimFromJsonImpl& GetImporter() const { return m_importer; }

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

        BentleyStatus InsertFont(DgnFontId oldId, DgnFontId newId);
        DgnFontId LookupFont(DgnFontId oldId);

        BentleyStatus InsertStyle(DgnStyleId oldId, DgnStyleId newId);
        DgnStyleId LookupStyle(DgnStyleId oldId);

        BentleyStatus InsertLsComponent(LsComponentId oldId, LsComponentId newId);
        LsComponentId LookupLsComponent(LsComponentId);

        DgnElementId LookupElement(DgnElementId oldId);
        DgnModelId LookupModel(DgnModelId oldId);

    };

END_BIM_FROM_DGNDB_NAMESPACE
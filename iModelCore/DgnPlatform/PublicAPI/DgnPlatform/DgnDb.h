/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnDb.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDbTables.h"
#include "DgnModel.h"
#include "MemoryManager.h"
#include "RepositoryManager.h"
#include "UpdatePlan.h"
#include <Bentley/BeFileName.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/13
//=======================================================================================
enum DgnDbSchemaValues : int32_t
    {
    DGNDB_CURRENT_VERSION_Major = 6,
    DGNDB_CURRENT_VERSION_Minor = 1,
    DGNDB_CURRENT_VERSION_Sub1  = 0,
    DGNDB_CURRENT_VERSION_Sub2  = 0,
//__PUBLISH_SECTION_END__
    //------------------------------
    // Schema change history
    //------------------------------
    // 6.0.1.1 - Remove ON DELETE RESTRICT, ON UPDATE RESTRICT
    // 6.0.1.2 - Introduce MinimumSharedColumnCount for dgn_GeometricElement2d=8 and dgn_GeometricElement3d=16
    // 6.1.0.0 - BIS clean-up activities (move classes to Markup domain, shared colums for dgn:Model, dgn:GeometryPart --> element, etc.)
//__PUBLISH_SECTION_START__

    DGNDB_SUPPORTED_VERSION_Major = 6,  // oldest version of the schema supported by the current api
    DGNDB_SUPPORTED_VERSION_Minor = 1,
    };

//=======================================================================================
//! Supplies the parameters necessary to create new DgnDbs.
// @bsiclass
//=======================================================================================
struct CreateDgnDbParams : BeSQLite::Db::CreateParams
{
public:
    bool            m_overwriteExisting;
    Utf8String      m_name;
    Utf8String      m_description;
    Utf8String      m_client;
    BeFileName      m_seedDb;
    BeSQLite::BeGuid m_guid;

    //! @param[in] guid The BeSQLite::BeGuid to store in the newly created DgnDb. If not supplied, a new BeSQLite::BeGuid is created.
    //! @note The new BeSQLite::BeGuid can be obtained via GetGuid.
    CreateDgnDbParams(BeSQLite::BeGuid guid=BeSQLite::BeGuid()) : BeSQLite::Db::CreateParams(), m_guid(guid) {if (!m_guid.IsValid()) m_guid.Create(); }

    //! Set the value to be stored as the ProjectName property in the new DgnDb created using this CreateDgnDbParams/
    //! @note This value is stored as a property in the project. It does *not* refer to a file name.
    void SetProjectName(Utf8CP name) {m_name.AssignOrClear(name);}

    //! Set the value to be stored as the ProjectDescription property in the new DgnDb created using this CreateDgnDbParams.
    void SetProjectDescription(Utf8CP description) {m_description.AssignOrClear(description);}

    //! Set the value to be stored as the Client property in the new DgnDb created using this CreateDgnDbParams.
    void SetClient(Utf8CP client) {m_client = client;}

    //! Set the filename of an existing, valid, SQLite file to be used as the "seed database" for the new DgnDb created using this CreateDgnDbParams.
    //! If a SeedDb is specified, it is merely copied verbatim to the filename supplied to DgnDb::CreateDgnDb. Then, the DgnDb
    //! tables are added to the copy of the seed database.
    //! @note The default is to create a new database from scratch (in other words, no seed database).
    //! @note When using a seed database, it determines the page size, encoding, compression, user_version, etc, for the database. Make sure they are appropriate
    //! for your needs.
    void SetSeedDb(BeFileNameCR seedDb) {m_seedDb = seedDb;}

    //! Get the BeSQLite::BeGuid to be stored in the newly created DgnDb. This is only necessary if you don't supply a valid BeSQLite::BeGuid
    //! to the ctor.
    BeSQLite::BeGuid GetGuid() const {return m_guid;}

    //! Determine whether to overwrite an existing file in DgnDb::CreateDgnDb. The default is to fail if a file by the supplied name
    //! already exists.
    void SetOverwriteExisting(bool val) {m_overwriteExisting = val;}
};

//=======================================================================================
//! A 4-digit number that specifies a serializable version of something in a DgnDb.
// @bsiclass
//=======================================================================================
struct DgnVersion : BeSQLite::SchemaVersion
{
    DgnVersion(uint16_t major, uint16_t minor, uint16_t sub1, uint16_t sub2) : SchemaVersion(major, minor, sub1, sub2) {}
    DgnVersion(Utf8CP val) : SchemaVersion(val){}
};

//=======================================================================================
//! A DgnDb is an in-memory object to access the information in a DgnDb file.
// @bsiclass
//=======================================================================================
struct DgnDb : RefCounted<BeSQLite::EC::ECDb>
{
    DEFINE_T_SUPER(BeSQLite::EC::ECDb)

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   05/13
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE OpenParams : BeSQLite::Db::OpenParams
    {
        explicit OpenParams(OpenMode openMode, BeSQLite::DefaultTxn startDefaultTxn=BeSQLite::DefaultTxn::Yes) : Db::OpenParams(openMode, startDefaultTxn) {}
        virtual ~OpenParams() {}

        BeSQLite::DbResult UpgradeSchema(DgnDbR) const;
        DGNPLATFORM_EXPORT virtual BeSQLite::DbResult _DoUpgrade(DgnDbR, DgnVersion& from) const;
    };

private:
    void Destroy();

protected:
    friend struct Txns;
    friend struct DgnElement;

    Utf8String      m_fileName;
    DgnElements     m_elements;
    DgnModels       m_models;
    DgnVersion      m_schemaVersion;
    DgnDomains      m_domains;
    DgnFonts        m_fonts;
    DgnStyles       m_styles;
    DgnUnits        m_units;
    DgnLinks        m_links;
    DgnAuthorities  m_authorities;
    TxnManagerPtr   m_txnManager;
    MemoryManager   m_memoryManager;
    IBriefcaseManagerPtr m_briefcaseManager;
    DgnSearchableText   m_searchableText;
    mutable RevisionManagerP m_revisionManager;
    BeSQLite::EC::ECSqlStatementCache m_ecsqlCache;
    DgnQueryQueue m_queryQueue;

    DGNPLATFORM_EXPORT virtual BeSQLite::DbResult _VerifySchemaVersion(BeSQLite::Db::OpenParams const& params) override;
    DGNPLATFORM_EXPORT virtual void _OnDbClose() override;
    DGNPLATFORM_EXPORT virtual BeSQLite::DbResult _OnDbOpened() override;

    BeSQLite::DbResult CreateNewDgnDb(BeFileNameCR boundFileName, CreateDgnDbParams const& params);
    BeSQLite::DbResult CreateDgnDbTables();
    BeSQLite::DbResult CreateAuthorities();
    BeSQLite::DbResult CreateDictionaryModel();
    BeSQLite::DbResult CreateGroupInformationModel();
    BeSQLite::DbResult InitializeDgnDb(CreateDgnDbParams const& params);
    BeSQLite::DbResult SaveDgnDbSchemaVersion(DgnVersion version=DgnVersion(DGNDB_CURRENT_VERSION_Major,DGNDB_CURRENT_VERSION_Minor,DGNDB_CURRENT_VERSION_Sub1,DGNDB_CURRENT_VERSION_Sub2));
    BeSQLite::DbResult DoOpenDgnDb(BeFileNameCR projectNameIn, OpenParams const&);

public:
    DgnDb();
    virtual ~DgnDb();

    //! Get ae BeFileName for this DgnDb.
    //! @note The superclass method BeSQLite::Db::GetDbFileName may also be used to get the same value, as a Utf8CP.
    BeFileName GetFileName() const {return BeFileName(m_fileName);}

    //! Get the schema version of an opened DgnDb.
    DGNPLATFORM_EXPORT DgnVersion GetSchemaVersion();

    //! Open an existing DgnDb file.
    //! @param[out] status BE_SQLITE_OK if the DgnDb file was successfully opened, error code otherwise. May be NULL.
    //! @param[in] filename The name of the BeSQLite::Db file from which the DgnDb is to be opened. Must be a valid filename on the local
    //! file system. It is not legal to open a DgnDb over a network share.
    //! @param[in] openParams Parameters for opening the database file
    //! @return a reference counted pointer to the opened DgnDb. Its IsValid() method will be false if the open failed for any reason.
    //! @note If this method succeeds, it will return a valid DgnDbPtr. The project will be automatically closed when the last reference
    //! to it is released. There is no way to hold a pointer to a "closed project".
    //! @note A DgnDb can have an expiration date. See Db::IsExpired
    DGNPLATFORM_EXPORT static DgnDbPtr OpenDgnDb(BeSQLite::DbResult* status, BeFileNameCR filename, OpenParams const& openParams);

    //! Create and open a new DgnDb file.
    //! @param[out] status BE_SQLITE_OK if the DgnDb file was successfully created, error code otherwise. May be NULL.
    //! @param[in] filename The name of the file for the new DgnDb. Must be a valid filename on the local
    //! filesystem. It is not legal to create a DgnDb over a network share.
    //! @param[in] params Parameters that control aspects of the newly created DgnDb
    //! @return a reference counted pointer to the newly created DgnDb. Its IsValid() method will return false if the open failed for any reason.
    //! @note If this method succeeds, it will return a valid DgnDbPtr. The DgnDb will be automatically closed when the last reference
    //! to it is released. There is no way to hold a pointer to a "closed project".
    DGNPLATFORM_EXPORT static DgnDbPtr CreateDgnDb(BeSQLite::DbResult* status, BeFileNameCR filename, CreateDgnDbParams const& params);

    DgnModels& Models() const {return const_cast<DgnModels&>(m_models);}                 //!< The DgnModels of this DgnDb
    DgnElements& Elements() const{return const_cast<DgnElements&>(m_elements);}          //!< The DgnElements of this DgnDb
    DgnUnits& Units() const {return const_cast<DgnUnits&>(m_units);}                     //!< The units for this DgnDb
    DgnStyles& Styles() const {return const_cast<DgnStyles&>(m_styles);}                 //!< The styles for this DgnDb
    DgnFonts& Fonts() const {return const_cast<DgnFonts&>(m_fonts); }                    //!< The fonts for this DgnDb
    DgnLinks& Links() const{return const_cast<DgnLinks&>(m_links);}                      //!< The DgnLinks for this DgnDb
    DgnDomains& Domains() const {return const_cast<DgnDomains&>(m_domains);}             //!< The DgnDomains associated with this DgnDb.
    DgnAuthorities& Authorities() const {return const_cast<DgnAuthorities&>(m_authorities);} //!< The authorities associated with this DgnDb
    DgnSearchableText& SearchableText() const { return const_cast<DgnSearchableText&>(m_searchableText); } //!< The searchable text table for this DgnDb
    DGNPLATFORM_EXPORT TxnManagerR Txns();                    //!< The Txns for this DgnDb.
    DGNPLATFORM_EXPORT RevisionManagerR Revisions() const; //!< The Revisions for this DgnDb.
    MemoryManager& Memory() const { return const_cast<MemoryManager&>(m_memoryManager);} //!< Manages memory associated with this DgnDb.
    DGNPLATFORM_EXPORT IBriefcaseManager& BriefcaseManager(); //!< Manages this briefcase's held locks and codes
    DgnQueryQueue& GetQueryQueue() const {return const_cast<DgnQueryQueue&>(m_queryQueue);}

    //! Gets a cached and prepared ECSqlStatement.
    DGNPLATFORM_EXPORT BeSQLite::EC::CachedECSqlStatementPtr GetPreparedECSqlStatement(Utf8CP ecsql) const;

    //! Perform a SQLite VACUUM on this DgnDb. This potentially makes the file smaller and more efficient to access.
    DGNPLATFORM_EXPORT DgnDbStatus CompactFile();

    //! Determine whether this DgnDb is the master copy.
    bool IsMasterCopy() const {return GetBriefcaseId().IsMasterId();}

    //! Determine whether this DgnDb is a briefcase.
    bool IsBriefcase() const {return !IsMasterCopy();}

    //! Determine whether this DgnDb is a stand-alone briefcase; that is, a transactable briefcase not associated with any master copy.
    bool IsStandaloneBriefcase() const {return GetBriefcaseId().IsStandaloneId();}

    DGNPLATFORM_EXPORT DictionaryModelR GetDictionaryModel(); //!< Return the dictionary model for this DgnDb.

    //! Ids for DgnPlatform threads
    enum class ThreadId {Unknown=0, Client=100, Render=101, Query=102, RealityData=103};

    DGNPLATFORM_EXPORT static ThreadId GetThreadId();        //!< Get the ThreadId for the current thread
    DGNPLATFORM_EXPORT static WCharCP GetThreadIdName();     //!< For debugging purposes, get the current ThreadId as a string
    static void SetThreadId(ThreadId);    //!< Set the ThreadId for the current thread
    static void VerifyThread(ThreadId id) {BeAssert(id==GetThreadId());}   //!< assert that this is a specific thread
    static void VerifyClientThread() {VerifyThread(ThreadId::Client);}     //!< assert that this is the Client thread
    static void VerifyRenderThread() {VerifyThread(ThreadId::Render);}     //!< assert that this is the Render thread
    static void VerifyQueryThread()  {VerifyThread(ThreadId::Query);}      //!< assert that this is the Query thread
};

END_BENTLEY_DGN_NAMESPACE


/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnDb.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDbTables.h"
#include "DgnModel.h"
#include "MemoryManager.h"
#include "SessionManager.h"
#include "RepositoryManager.h"
#include "UpdatePlan.h"
#include <Bentley/BeFileName.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/13
//=======================================================================================
enum DgnDbSchemaValues : int32_t
{
    DGNDB_CURRENT_VERSION_Major = 1,    // WIP: Increment to 2.0 just prior to Bim02 release
    DGNDB_CURRENT_VERSION_Minor = 15,   // WIP: Increment this (1.x) for intermediate schema changes before Bim02 release
    DGNDB_CURRENT_VERSION_Sub1  = 0,
    DGNDB_CURRENT_VERSION_Sub2  = 0,

    DGNDB_SUPPORTED_VERSION_Major = 1,  // oldest version of the schema supported by the current api
    DGNDB_SUPPORTED_VERSION_Minor = 15,
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
//! Class designed for extracting the DgnDb schema version from a BeFileName
//! @see DgnDbSchemaVersion::Extract
// @bsiclass
//=======================================================================================
struct DgnDbSchemaVersion : DgnVersion
{
    DEFINE_T_SUPER(DgnVersion)
    friend struct DgnDb;

private:
    DgnDbSchemaVersion() : T_Super(0, 0, 0, 0) {}
    DgnDbSchemaVersion(uint16_t major, uint16_t minor) : T_Super(major, minor, 0, 0) {}
    DgnDbSchemaVersion(uint16_t major, uint16_t minor, uint16_t sub1, uint16_t sub2) : T_Super(major, minor, sub1, sub2) {}
    explicit DgnDbSchemaVersion(Utf8CP json) : T_Super(json) {}

    //! Map from legacy version range into current version range
    static DgnDbSchemaVersion FromLegacy(Utf8CP versionJson);
    //! Former PropertySpec values of DgnProjectProperty::SchemaVersion.  Used for inspecting legacy .dgndb, .idgndb files.
    static BeSQLite::PropertySpec LegacyDbSchemaVersionProperty() {return BeSQLite::PropertySpec("SchemaVersion", "dgn_Proj");}
    //! Former PropertySpec values of DgnEmbeddedProjectProperty::SchemaVersion.  Used for inspecting legacy .imodel files.
    static BeSQLite::PropertySpec LegacyEmbeddedDbSchemaVersionProperty() {return BeSQLite::PropertySpec("SchemaVersion", "pkge_dgnProj");}

public:
    bool IsValid() const {return !IsEmpty();}
    bool IsCurrent() const {return *this == GetCurrent();}
    bool IsPast() const {return *this < GetCurrent();}
    bool IsFuture() const {return *this > GetCurrent();}
    bool IsVersion_1_5() const {return *this == DgnDbSchemaVersion::Version_1_5();}
    bool IsVersion_1_6() const {return *this == DgnDbSchemaVersion::Version_1_6();}

    //! Extract the DgnDbSchemaVersion from the specfied file
    DGNPLATFORM_EXPORT static DgnDbSchemaVersion Extract(BeFileNameCR fileName);
    static DgnDbSchemaVersion GetCurrent() {return DgnDbSchemaVersion(DGNDB_CURRENT_VERSION_Major, DGNDB_CURRENT_VERSION_Minor, DGNDB_CURRENT_VERSION_Sub1, DGNDB_CURRENT_VERSION_Sub2);}
    static DgnDbSchemaVersion Version_1_0() {return DgnDbSchemaVersion(1, 0);} // DgnV8
    static DgnDbSchemaVersion Version_1_5() {return DgnDbSchemaVersion(1, 5);} // Graphite05
    static DgnDbSchemaVersion Version_1_6() {return DgnDbSchemaVersion(1, 6);} // DgnDb0601
};

//=======================================================================================
//! Supplies the parameters necessary to create new DgnDbs.
// @bsiclass
//=======================================================================================
struct CreateDgnDbParams : BeSQLite::Db::CreateParams
{
public:
    bool m_overwriteExisting;
    Utf8String m_rootSubjectName;
    Utf8String m_rootSubjectDescription;
    Utf8String m_client;
    BeFileName m_seedDb;
    BeSQLite::BeGuid m_guid;

    //! Default constructor for CreateDgnDbParams
    //! @param[in] guid The BeSQLite::BeGuid to store in the newly created DgnDb. If not supplied, a new BeSQLite::BeGuid value is created.
    //! @note The new BeSQLite::BeGuid can be obtained via GetGuid.
    CreateDgnDbParams(BeSQLite::BeGuid guid=BeSQLite::BeGuid(true)) : BeSQLite::Db::CreateParams(), m_guid(guid) {}

    //! Constructor for CreateDgnDbParams
    //! @param[in] name The (required) value to be stored as the CodeValue property of the root Subject
    //! @param[in] description The (optional) value to be stored as the Description property of the root Subject
    //! @param[in] guid The BeSQLite::BeGuid to store in the newly created DgnDb. If not supplied, a new BeSQLite::BeGuid value is created.
    //! @note The new BeSQLite::BeGuid can be obtained via GetGuid.
    CreateDgnDbParams(Utf8CP name, Utf8CP description=nullptr, BeSQLite::BeGuid guid=BeSQLite::BeGuid(true)) : BeSQLite::Db::CreateParams(), m_guid(guid)
        {
        SetRootSubjectName(name);
        SetRootSubjectDescription(description);
        }

    //! Set the name to be stored as the CodeValue property of the root Subject for the new DgnDb created using this CreateDgnDbParams.
    //! @note The (required) CodeValue of the root Subject should indicate what the DgnDb contains.
    //! @see DgnElements::GetRootSubject
    void SetRootSubjectName(Utf8CP name) {m_rootSubjectName.AssignOrClear(name);}

    //! Set the value to be stored as the Description property of the root Subject for the new DgnDb created using this CreateDgnDbParams.
    //! @note The (optional) Description property of the root Subject can be a longer description of what the DgnDb contains.
    //! @see DgnElements::GetRootSubject
    void SetRootSubjectDescription(Utf8CP description) {m_rootSubjectDescription.AssignOrClear(description);}

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
    bool ValidateSchemaForImport(bool& needsImport, ECN::ECSchemaCR appSchema) const;

protected:
    friend struct Txns;
    friend struct DgnElement;

    Utf8String m_fileName;
    DgnElements m_elements;
    DgnModels m_models;
    DgnVersion m_schemaVersion;
    DgnDomains m_domains;
    DgnFonts m_fonts;
    DgnLineStylesPtr m_lineStyles;
    DgnGeoLocation m_geoLocation;
    DgnCodeSpecs m_codeSpecs;
    TxnManagerPtr m_txnManager;
    SessionManager m_sessionManager;
    MemoryManager m_memoryManager;
    IBriefcaseManagerPtr m_briefcaseManager;
    DgnSearchableText m_searchableText;
    mutable std::unique_ptr<RevisionManager> m_revisionManager;
    mutable BeSQLite::EC::ECSqlStatementCache m_ecsqlCache;

    DGNPLATFORM_EXPORT BeSQLite::DbResult _VerifySchemaVersion(BeSQLite::Db::OpenParams const& params) override;
    DGNPLATFORM_EXPORT void _OnDbClose() override;
    DGNPLATFORM_EXPORT BeSQLite::DbResult _OnDbOpened() override;
    // *** WIP_SCHEMA_IMPORT - temporary work-around needed because ECClass objects are deleted when a schema is imported
    void _OnAfterECSchemaImport() const override {m_ecsqlCache.Empty(); Elements().ClearUpdaterCache();}

    BeSQLite::DbResult CreateNewDgnDb(BeFileNameCR boundFileName, CreateDgnDbParams const& params); //!< @private
    BeSQLite::DbResult CreateDgnDbTables(CreateDgnDbParams const& params); //!< @private
    BeSQLite::DbResult CreateCodeSpecs(); //!< @private
    BeSQLite::DbResult CreateRepositoryModel(); //!< @private
    BeSQLite::DbResult CreateRootSubject(CreateDgnDbParams const& params); //!< @private
    BeSQLite::DbResult CreatePartitionElement(Utf8CP, DgnElementId, Utf8CP); //!< @private
    BeSQLite::DbResult CreateDictionaryModel(); //!< @private
    BeSQLite::DbResult CreateSessionModel(); //!< @private
    BeSQLite::DbResult CreateRealityDataSourcesModel(); //!< @private
    BeSQLite::DbResult InitializeDgnDb(CreateDgnDbParams const& params); //!< @private
    BeSQLite::DbResult SaveDgnDbSchemaVersion(DgnVersion version=DgnDbSchemaVersion::GetCurrent()); //!< @private
    BeSQLite::DbResult DoOpenDgnDb(BeFileNameCR projectNameIn, OpenParams const&); //!< @private

public:
    DgnDb();
    virtual ~DgnDb();

    //! Get the BeFileName for this DgnDb.
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
    //! @param[in] params Parameters that control aspects of the newly created DgnDb. Must include a valid root Subject name.
    //! @return a reference counted pointer to the newly created DgnDb. Its IsValid() method will return false if the open failed for any reason.
    //! @note If this method succeeds, it will return a valid DgnDbPtr. The DgnDb will be automatically closed when the last reference
    //! to it is released. There is no way to hold a pointer to a "closed project".
    //! @see CreateDgnDbParams::SetRootSubjectName
    DGNPLATFORM_EXPORT static DgnDbPtr CreateDgnDb(BeSQLite::DbResult* status, BeFileNameCR filename, CreateDgnDbParams const& params);

    DgnModels& Models() const {return const_cast<DgnModels&>(m_models);}                 //!< The DgnModels of this DgnDb
    DgnElements& Elements() const{return const_cast<DgnElements&>(m_elements);}          //!< The DgnElements of this DgnDb
    DgnGeoLocation& GeoLocation() const {return const_cast<DgnGeoLocation&>(m_geoLocation);}  //!< The geolocation information for this DgnDb
    DgnLineStyles& LineStyles() const {return const_cast<DgnLineStyles&>(*m_lineStyles);}//!< The line styles for this DgnDb
    DgnFonts& Fonts() const {return const_cast<DgnFonts&>(m_fonts);}                    //!< The fonts for this DgnDb
    DgnDomains& Domains() const {return const_cast<DgnDomains&>(m_domains);}             //!< The DgnDomains associated with this DgnDb.
    DgnCodeSpecs& CodeSpecs() const {return const_cast<DgnCodeSpecs&>(m_codeSpecs);} //!< The codeSpecs associated with this DgnDb
    DgnSearchableText& SearchableText() const {return const_cast<DgnSearchableText&>(m_searchableText);} //!< The searchable text table for this DgnDb
    DGNPLATFORM_EXPORT TxnManagerR Txns();                 //!< The TxnManager for this DgnDb.
    DGNPLATFORM_EXPORT RevisionManagerR Revisions() const; //!< The RevisionManager for this DgnDb.
    MemoryManager& Memory() const {return const_cast<MemoryManager&>(m_memoryManager);} //!< Manages memory associated with this DgnDb.
    SessionManager& Sessions() const {return const_cast<SessionManager&>(m_sessionManager);} //!< Manages Sessions associated with this DgnDb.
    DGNPLATFORM_EXPORT IBriefcaseManager& BriefcaseManager(); //!< Manages this briefcase's held locks and codes

    //! Imports a set of EC Schemas into the Bim
    //! @param[in] schemas Schemas to be imported. 
    //! @remarks Only used for cases where the schemas are NOT paired with a domain. @see DgnDomain::ImportSchema().
    //! It's the caller's responsibility to start a new transaction before this call and commit it after a successful 
    //! import. If an eror happens during the import, the new transaction is abandoned in the call. 
    DGNPLATFORM_EXPORT DgnDbStatus ImportSchemas(bvector<ECN::ECSchemaCP> schemas);

    //! Inserts a new ECRelationship
    //! @param[out] relKey key of the new ECRelationship
    //! @param[in] relClass ECRelationshipClass to create an instance of
    //! @param[in] sourceId SourceECInstanceId.
    //! @param[in] targetId TargetECInstanceId.
    //! @param[in] relInstanceProperties If @p relClass has ECProperties, pass its values via this parameter. Note: In this
    //! case @ref ECN::IECRelationshipInstance::GetSource "IECRelationshipInstance::GetSource" and @ref ECN::IECRelationshipInstance::GetTarget "IECRelationshipInstance::GetTarget"
    //! don't have to be set in @p relInstanceProperties
    //! @return BE_SQLITE_OK in case of success. Error codes otherwise
    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertRelationship(BeSQLite::EC::ECInstanceKey& relKey, ECN::ECRelationshipClassCR relClass, BeSQLite::EC::ECInstanceId sourceId, BeSQLite::EC::ECInstanceId targetId, ECN::IECRelationshipInstanceCP relInstanceProperties = nullptr);

    //! Inserts a new ECRelationship between two elements.
    //! @param[out] relKey key of the new ECRelationship
    //! @param[in] relClass ECRelationshipClass to create an instance of
    //! @param[in] sourceId The "source" element.
    //! @param[in] targetId The "target" element.
    //! @param[in] relInstanceProperties If @p relClass has ECProperties, pass its values via this parameter. Note: In this
    //! case @ref ECN::IECRelationshipInstance::GetSource "IECRelationshipInstance::GetSource" and @ref ECN::IECRelationshipInstance::GetTarget "IECRelationshipInstance::GetTarget"
    //! don't have to be set in @p relInstanceProperties
    //! @return BE_SQLITE_OK in case of success. Error codes otherwise
    BeSQLite::DbResult InsertRelationship(BeSQLite::EC::ECInstanceKey& relKey, ECN::ECRelationshipClassCR relClass, DgnElementId sourceId, DgnElementId targetId, ECN::IECRelationshipInstanceCP relInstanceProperties = nullptr)
        {
        return InsertRelationship(relKey, relClass, BeSQLite::EC::ECInstanceId(sourceId.GetValue()), BeSQLite::EC::ECInstanceId(targetId.GetValue()));
        }
    
    //! Update one or more properties of an existing ECRelationship instance. Note that you cannot change the source or target. @note this function only makes sense if the relationship instance is stored in a link table.
    //! @param key Identifies the relationship instance.
    //! @param props Contains the properties to be written. Note that this functions updates props by setting its InstanceId.
    //! @return BE_SQLITE_OK in case of success. Error codes otherwise
    DGNPLATFORM_EXPORT BeSQLite::DbResult UpdateRelationshipProperties(BeSQLite::EC::ECInstanceKeyCR key, ECN::IECInstanceR props);

    //! Deletes ECRelationships which match the specified @p sourceId and @p targetId.
    //! @remarks @p sourceId and @p targetId are used to build the ECSQL where clause. So they are used to filter
    //! what to delete. If one of them is invalid, it will not be included in the filter. If both are invalid, it is an error.
    //! @param[in] relClassECSqlName ECRelationshipClass name in ECSQL format (schema name.class name)
    //! @param[in] sourceId SourceECInstanceId filter. If invalid, no SourceECInstanceId filter will be applied.
    //! @param[in] targetId TargetECInstanceId filter. If invalid, no TargetECInstanceId filter will be applied.
    //! @return BE_SQLITE_OK in case of success. Error codes otherwise
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteRelationships(Utf8CP relClassECSqlName, BeSQLite::EC::ECInstanceId sourceId, BeSQLite::EC::ECInstanceId targetId);

    BeSQLite::DbResult DeleteRelationships(Utf8CP relClassECSqlName, DgnElementId sourceId, DgnElementId targetId)
        {
        return DeleteRelationships(relClassECSqlName, BeSQLite::EC::ECInstanceId(sourceId.GetValue()), BeSQLite::EC::ECInstanceId(targetId.GetValue()));
        }

    //! Deletes a specific ECRelationship
    //! @param key Identifies the ECRelationship instance
    //! @return BE_SQLITE_OK in case of success. Error codes otherwise
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteRelationship(BeSQLite::EC::ECInstanceKeyCR key);

    //! Gets a cached and prepared ECSqlStatement.
    DGNPLATFORM_EXPORT BeSQLite::EC::CachedECSqlStatementPtr GetPreparedECSqlStatement(Utf8CP ecsql) const;
    DGNPLATFORM_EXPORT BeSQLite::EC::CachedECSqlStatementPtr GetNonSelectPreparedECSqlStatement(Utf8CP ecsql, BeSQLite::EC::ECCrudWriteToken const*) const;

    //! Perform a SQLite VACUUM on this DgnDb. This potentially makes the file smaller and more efficient to access.
    DGNPLATFORM_EXPORT DgnDbStatus CompactFile();

    //! Determine whether this DgnDb is the master copy.
    bool IsMasterCopy() const {return GetBriefcaseId().IsMasterId();}

    //! Determine whether this DgnDb is a briefcase.
    bool IsBriefcase() const {return !IsMasterCopy();}

    //! Determine whether this DgnDb is a stand-alone briefcase; that is, a transactable briefcase not associated with any master copy.
    bool IsStandaloneBriefcase() const {return GetBriefcaseId().IsStandaloneId();}

    DGNPLATFORM_EXPORT RepositoryModelPtr GetRepositoryModel(); //!< Return the RepositoryModel for this DgnDb.
    DGNPLATFORM_EXPORT DictionaryModelR GetDictionaryModel(); //!< Return the dictionary model for this DgnDb.
    DGNPLATFORM_EXPORT LinkModelPtr GetRealityDataSourcesModel(); //!< Return the LinkModel listing the reality data sources for this DgnDb.
    DGNPLATFORM_EXPORT SessionModelPtr GetSessionModel(); //!< Return the SessionModel for this DgnDb.

/** @name DgnPlatform Threads */
/** @{ */
    //! Ids for DgnPlatform threads
    enum class ThreadId {Unknown=0, Client=100, Render=101, IoPool=103, CpuPool=104};

    DGNPLATFORM_EXPORT static ThreadId GetThreadId();    //!< Get the ThreadId for the current thread
    DGNPLATFORM_EXPORT static WCharCP GetThreadIdName(); //!< For debugging purposes, get the current ThreadId as a string
    DGNPLATFORM_EXPORT static void SetThreadId(ThreadId);    //!< Set the ThreadId for the current thread
    static void VerifyThread(ThreadId id) {BeAssert(id==GetThreadId());}   //!< assert that this is a specific thread
    static void VerifyClientThread() {VerifyThread(ThreadId::Client);}     //!< assert that this is the Client thread
    static void VerifyRenderThread() {VerifyThread(ThreadId::Render);}     //!< assert that this is the Render thread
    static void VerifyIoPoolThread() {VerifyThread(ThreadId::IoPool);}     //!< assert that this is one of the IoPool threads
    static void VerifyCpuPoolThread() {VerifyThread(ThreadId::CpuPool);}   //!< assert that this is one of the CpuPool threads
/** @} */

    //! Gets the permission token which all code within DgnPlatform has to pass to non-SELECT ECSQL statements
    //! or other non-read EC CRUD operations.
    //! Otherwise the preparation of the ECSQL or the write operation will fail.
    //! @return EC CRUD write token. Is never nullptr but is returned as pointer as this is how you pass it to the ECSQL APIs. 
    //! @private
    BeSQLite::EC::ECCrudWriteToken const* GetECCrudWriteToken() const; //not inlined as it must not be called externally

    //! Gets the permission token to perform a ECSchema import/update
    //! @return ECSchemaImportToken. Is never nullptr but is returned as pointer as this is how you pass it to ECDbSchemaManager::ImportECSchemas. 
    //! @private
    BeSQLite::EC::ECSchemaImportToken const* GetECSchemaImportToken() const; //not inlined as it must not be called externally

    //! @private internal use only (v8 importer)
    DGNPLATFORM_EXPORT DgnDbStatus ImportSchemas(bvector<ECN::ECSchemaCP> schemas, bool doNotFailSchemaValidationForLegacyIssues);
};

END_BENTLEY_DGN_NAMESPACE


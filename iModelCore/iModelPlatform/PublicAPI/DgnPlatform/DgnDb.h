/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DgnDbTables.h"
#include "DgnModel.h"
#include "UpdatePlan.h"
#include <Bentley/BeFileName.h>
#include <BeSQLite/BeBriefcaseBasedIdSequence.h>
#include <unordered_map>
#include <ECDb/SchemaManager.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Interface adopted by an object that can indicate whether an operation should be
//! canceled. Typically provided to asynchronous, potentially long-running tasks.
//! NB: Derives from RefCountedBase instead of IRefCounted because C++ is garbage
//! (new/delete operators become ambiguous).
// @bsistruct
//=======================================================================================
struct ICancellable : RefCountedBase
{
    virtual bool IsCanceled() = 0;

    static bool IsCanceled(ICancellableP cancel) { return nullptr != cancel && cancel->IsCanceled(); }
};

//=======================================================================================
// @bsistruct
//=======================================================================================
template<typename T> struct ICancellableWrapper : ICancellable
{
protected:
    T const& m_impl;

    explicit ICancellableWrapper(T const& impl) : m_impl(impl) { }
public:
    bool IsCanceled() override { return m_impl.IsCanceled(); }

    static ICancellablePtr Create(T const& impl) { return new ICancellableWrapper(impl); }
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct AtomicCancellable : ICancellable
{
protected:
    BeAtomic<bool> m_canceled;
public:
    bool IsCanceled() override { return m_canceled.load(); }
    void Cancel() { m_canceled.store(true); }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
enum DgnDbProfileValues : int32_t
{
    DGNDB_CURRENT_VERSION_Major = 2,
    DGNDB_CURRENT_VERSION_Minor = 0,
    DGNDB_CURRENT_VERSION_Sub1  = 0,
    DGNDB_CURRENT_VERSION_Sub2  = 4,

    DGNDB_SUPPORTED_VERSION_Major = 2,  // oldest version of the profile supported by the current api
    DGNDB_SUPPORTED_VERSION_Minor = 0,
};

//=======================================================================================
//! A 4-digit number that specifies a serializable version of something in a DgnDb.
// @bsiclass
//=======================================================================================
struct DgnDbProfileVersion : BeSQLite::ProfileVersion
{
    DEFINE_T_SUPER(BeSQLite::ProfileVersion)
    friend struct DgnDb;

private:
    //! Map from legacy version range into current version range
    static DgnDbProfileVersion FromLegacy(Utf8CP versionJson);
    //! Former PropertySpec values of DgnProjectProperty::ProfileVersion.  Used for inspecting legacy .dgndb, .idgndb files.
    static BeSQLite::PropertySpec LegacyDbProfileVersionProperty() {return BeSQLite::PropertySpec("SchemaVersion", "dgn_Proj");}
    //! Former PropertySpec values of DgnEmbeddedProjectProperty::ProfileVersion.  Used for inspecting legacy .imodel files.
    static BeSQLite::PropertySpec LegacyEmbeddedDbProfileVersionProperty() {return BeSQLite::PropertySpec("SchemaVersion", "pkge_dgnProj");}

public:
    DgnDbProfileVersion() : T_Super(0, 0, 0, 0) {}
    DgnDbProfileVersion(uint16_t major, uint16_t minor) : T_Super(major, minor, 0, 0) {}
    DgnDbProfileVersion(uint16_t major, uint16_t minor, uint16_t sub1, uint16_t sub2) : ProfileVersion(major, minor, sub1, sub2) {}
    explicit DgnDbProfileVersion(Utf8CP json) : T_Super(json) {}

    bool IsValid() const {return !IsEmpty();}
    bool IsCurrent() const {return *this == GetCurrent();}
    bool IsPast() const {return *this < GetCurrent();}
    bool IsFuture() const {return *this > GetCurrent();}
    bool IsVersion_1_5() const {return *this == DgnDbProfileVersion::Version_1_5();}
    bool IsVersion_1_6() const {return *this == DgnDbProfileVersion::Version_1_6();}

    //! Extract the DgnDbProfileVersion from the specified file
    DGNPLATFORM_EXPORT static DgnDbProfileVersion Extract(BeFileNameCR fileName);
    static DgnDbProfileVersion GetCurrent() {return DgnDbProfileVersion(DGNDB_CURRENT_VERSION_Major, DGNDB_CURRENT_VERSION_Minor, DGNDB_CURRENT_VERSION_Sub1, DGNDB_CURRENT_VERSION_Sub2);}
    static DgnDbProfileVersion Version_1_0() {return DgnDbProfileVersion(1, 0);} // DgnV8
    static DgnDbProfileVersion Version_1_5() {return DgnDbProfileVersion(1, 5);} // Graphite05
    static DgnDbProfileVersion Version_1_6() {return DgnDbProfileVersion(1, 6);} // DgnDb0601
};

//=======================================================================================
//! Supplies the parameters necessary to create new DgnDbs.
// @bsiclass
//=======================================================================================
struct CreateDgnDbParams : BeSQLite::Db::CreateParams
{
public:
    bool m_overwriteExisting = false;
    Utf8String m_rootSubjectName;
    Utf8String m_rootSubjectDescription;
    Utf8String m_client;
    BeFileName m_seedDb;
    BeSQLite::BeGuid m_guid;
    DPoint3d m_globalOrigin = DPoint3d::FromZero();
    AxisAlignedBox3d m_projectExtents;

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

    //! Set the project extents for the new project.
    void SetProjectExtents(AxisAlignedBox3dCR extents) {m_projectExtents = extents;}

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
struct DgnDb : RefCounted<BeSQLite::EC::ECDb>, BeSQLite::EC::ECDb::IECDbCacheClearListener
{
    friend struct BisCoreDomain;
    DEFINE_T_SUPER(BeSQLite::EC::ECDb)

    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE OpenParams : BeSQLite::EC::ECDb::OpenParams
    {
        friend struct DgnDb;

    private:
        SchemaUpgradeOptions m_schemaUpgradeOptions;

    public:
        //! Constructor
        //! @param[in] openMode The mode for opening the database
        //! @param[in] startDefaultTxn Whether to start a default transaction on the database
        //! @param[in] schemaUpgradeOptions Options to upgrade the ECSchema-s in the database from registered domains, or revisions.
        explicit OpenParams(OpenMode openMode, BeSQLite::DefaultTxn startDefaultTxn = BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions schemaUpgradeOptions = SchemaUpgradeOptions()) :
            ECDb::OpenParams(openMode, startDefaultTxn), m_schemaUpgradeOptions(schemaUpgradeOptions)   {}

        SchemaUpgradeOptions& GetSchemaUpgradeOptionsR() { return m_schemaUpgradeOptions; }
        SchemaUpgradeOptions const& GetSchemaUpgradeOptions() const { return m_schemaUpgradeOptions; }

        virtual ~OpenParams() {}
    };

private:
    BeSQLite::BeBriefcaseBasedIdSequence m_elementIdSequence;
    int m_purgeOperation = 0;

    void Destroy();
    SchemaStatus PickSchemasToImport(bvector<ECN::ECSchemaCP>& importSchemas, bvector<ECN::ECSchemaCP> const& schemas, bool isImportingFromV8) const;
    void OnBisCoreSchemaImported(CreateDgnDbParams const& params);
    BeSQLite::DbResult InitializeElementIdSequence();
    void ClearECSqlCache() const { m_ecsqlCache.Empty(); }

    BeSQLite::DbResult InitializeSchemas(BeSQLite::Db::OpenParams const& params);
    BeSQLite::DbResult ProcessRevisions(BeSQLite::Db::OpenParams const& params);

protected:
    friend struct Txns;
    friend struct DgnElement;

    Utf8String m_fileName;
    DgnElements m_elements;
    DgnModels m_models;
    mutable DgnDbProfileVersion m_profileVersion;
    DgnDomains m_domains;
    DgnDbFonts m_fonts;
    DgnLineStylesPtr m_lineStyles;
    DgnGeoLocation m_geoLocation;
    DgnCodeSpecs m_codeSpecs;
    TxnManagerPtr m_txnManager;
    DgnSearchableText m_searchableText;
    mutable std::unique_ptr<RevisionManager> m_revisionManager;
    mutable BeSQLite::EC::ECSqlStatementCache m_ecsqlCache;
    mutable std::unordered_map<uint64_t, std::unique_ptr<BeSQLite::EC::ECInstanceInserter>> m_cacheECInstanceInserter;

    DGNPLATFORM_EXPORT BeSQLite::ProfileState _CheckProfileVersion() const override;
    DGNPLATFORM_EXPORT BeSQLite::DbResult _UpgradeProfile() override;
    DGNPLATFORM_EXPORT BeSQLite::DbResult _OnBeforeProfileUpgrade() override;
    DGNPLATFORM_EXPORT BeSQLite::DbResult _OnAfterProfileUpgrade() override;
    DGNPLATFORM_EXPORT void _OnDbClose() override;
    DGNPLATFORM_EXPORT BeSQLite::DbResult _OnDbOpening() override;
    DGNPLATFORM_EXPORT BeSQLite::DbResult _OnDbOpened(BeSQLite::Db::OpenParams const& params) override;

    DGNPLATFORM_EXPORT void _OnDbGuidChange(BeSQLite::BeGuid guid) override;
    DGNPLATFORM_EXPORT void _OnBeforeSetBriefcaseId(BeSQLite::BeBriefcaseId newId) override;
    DGNPLATFORM_EXPORT void _OnAfterSetBriefcaseId() override;

    DGNPLATFORM_EXPORT BeSQLite::DbResult _AfterSchemaChangeSetApplied() const override;
    DGNPLATFORM_EXPORT BeSQLite::DbResult _AfterDataChangeSetApplied() override;

    // *** WIP_SCHEMA_IMPORT - temporary work-around needed because ECClass objects are deleted when a schema is imported
    void _OnBeforeClearECDbCache() override;

    BeSQLite::DbResult CreateNewDgnDb(BeFileNameCR boundFileName, CreateDgnDbParams const& params); //!< @private
    BeSQLite::DbResult CreateDgnDbTables(CreateDgnDbParams const& params); //!< @private
    BeSQLite::DbResult CreateCodeSpecs(); //!< @private
    BeSQLite::DbResult CreateRepositoryModel(); //!< @private
    BeSQLite::DbResult CreateRootSubject(CreateDgnDbParams const& params); //!< @private
    BeSQLite::DbResult CreatePartitionElement(Utf8CP, DgnElementId, Utf8CP); //!< @private
    BeSQLite::DbResult CreateDictionaryModel(); //!< @private
    BeSQLite::DbResult CreateRealityDataSourcesModel(); //!< @private
    BeSQLite::DbResult InitializeDgnDb(CreateDgnDbParams const& params); //!< @private
    BeSQLite::DbResult SaveDgnDbProfileVersion(DgnDbProfileVersion version=DgnDbProfileVersion::GetCurrent()); //!< @private
    BeSQLite::DbResult DoOpenDgnDb(BeFileNameCR projectNameIn, OpenParams const&); //!< @private

public:
    Napi::ObjectReference m_private_iModelDbJs; // only public so it can be set from IModelJsNative::NativeDgnDb::SetIModelDb

    Napi::ObjectReference* GetJsIModelDb() { return (IsMainThread() && !m_private_iModelDbJs.IsEmpty()) ? &m_private_iModelDbJs : nullptr; }
    DGNPLATFORM_EXPORT Napi::Object GetJsTxns(); // get the "IModelDb.txns" JavaScript object of this DgnDb (or nullptr)
    Napi::String ToJsString(Utf8CP val, size_t len) { return Napi::String::New(m_private_iModelDbJs.Env(), val, len); }
    Napi::String ToJsString(Utf8CP val) { return ToJsString(val, std::strlen(val)); }
    Napi::String ToJsString(Utf8StringCR str) { return ToJsString(str.c_str(), str.length()); }
    Napi::String ToJsString(BeInt64Id id) { return ToJsString(id.ToHexStr()); }
    Napi::String GetJsClassName(DgnElementId id);
    DGNPLATFORM_EXPORT void CallJsHandlerMethod(DgnClassId classId, Utf8CP methodName, Napi::Object arg);
    static void CallJsFunction(Napi::Object obj, Utf8CP methodName, std::vector<napi_value> const& args);
    [[noreturn]] void ThrowException(Utf8CP message, int errNum);
    BeSQLite::DbResult DisqualifyTypeIndexForBisCoreExternalSourceAspect();
    DGNPLATFORM_EXPORT BeSQLite::EC::CachedECSqlStatementPtr GetGeometricModelUpdateStatement();
    DGNPLATFORM_EXPORT BeSQLite::CachedStatementPtr GetModelLastModUpdateStatement();

    DgnDb();
    virtual ~DgnDb();

    //! @private
    //! Get the BeFileName for this DgnDb.
    //! @note The superclass method BeSQLite::Db::GetDbFileName may also be used to get the same value, as a Utf8CP.
    BeFileName GetFileName() const {return BeFileName(m_fileName);}

    //! Get the profile version of an opened DgnDb.
    DGNPLATFORM_EXPORT DgnDbProfileVersion GetProfileVersion();

    //! Open an existing DgnDb file.
    //! @param[out] status BE_SQLITE_OK if the DgnDb file was successfully opened, error code otherwise. May be NULL.
    //! @param[in] filename The name of the BeSQLite::Db file from which the DgnDb is to be opened. Must be a valid filename on the local
    //! file system. It is not legal to open a DgnDb over a network share.
    //! @param[in] openParams Parameters for opening the database file
    //! @return a reference counted pointer to the opened DgnDb. Its IsValid() method will be false if the open failed for any reason.
    //! @remarks
    //! <ul>
    //! <li> If this method succeeds, it will return a valid DgnDbPtr. The project will be automatically closed when the last reference
    //! to it is released. There is no way to hold a pointer to a "closed project".
    //! <li> A DgnDb can have an expiration date. See Db::IsExpired
    //! <li> The ECSchemas supplied by registered DgnDomain-s are validated against the corresponding ones in the DgnDb, and
    //! an appropriate error status is returned in the case of a failure. See table below for the various ECSchema compatibility errors.
    //! If the error status is BE_SQLITE_ERROR_SchemaUpgradeRequired, it may be possible to
    //! upgrade (or import) the schemas in the DgnDb. This is done by opening the DgnDb with setting the option request upgrade of
    //! domain schemas (See @ref DgnDb::OpenParams). These domain schema validation errors can also be avoided by restricting the
    //! specific checks made to validate the domain schemas by by setting the appropriate @ref SchemaUpgradeOptions::DomainUpgradeOptions
    //! in @ref DgnDb::OpenParams.
    //! <pre>
    //! Sample schema compatibility validation results for an ECSchema in the BIM with Version 2.2.2 (Read.Write.Minor)
    //! -------------------------------------------------------------------------------------------------
    //! Application   |  Validation result              | Validation result
    //! Version       |  (Readonly)                     | (ReadWrite)
    //! -------------------------------------------------------------------------------------------------
    //! 2.2.2 (same)  | BE_SQLITE_OK                    | BE_SQLITE_OK
    //! -------------------------------------------------------------------------------------------------
    //! 1.2.2 (older) | BE_SQLITE_ERROR_SchemaTooNew    | BE_SQLITE_ERROR_SchemaTooNew
    //! 2.1.2 (older) | BE_SQLITE_OK                    | BE_SQLITE_ERROR_SchemaTooNew
    //! 2.2.1 (older) | BE_SQLITE_OK                    | BE_SQLITE_OK
    //! -------------------------------------------------------------------------------------------------
    //! 3.2.2 (newer) | BE_SQLITE_ERROR_SchemaTooOld    | BE_SQLITE_ERROR_SchemaTooOld
    //! 2.3.2 (newer) | BE_SQLITE_OK by default*        | BE_SQLITE_ERROR_SchemaUpgradeRequired
    //! 2.2.3 (newer) | BE_SQLITE_OK by default*        | BE_SQLITE_OK by default*
    //!
    //! * - BE_SQLITE_OK by default, or BE_SQLITE_ERROR_SchemaUpgradeRecommended if
    //! SchemaUpgradeOptions::DomainUpgradeOptions::CheckRecommendedUpgrades is passed in
    //! -------------------------------------------------------------------------------------------------
    //! </pre>
    //! <li> If the domain schemas are setup to be upgraded, a schema lock is first obtained before the upgrade.
    //! Note that any previously committed local changes that haven't been pushed up to the server
    //! will cause an error. These need to be flushed out by creating a revision. See @ref RevisionManager
    //! </ul>
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
    DgnDbFonts& Fonts() const {return const_cast<DgnDbFonts&>(m_fonts);}                    //!< The fonts for this DgnDb
    DgnDomains& Domains() const {return const_cast<DgnDomains&>(m_domains);}             //!< The DgnDomains associated with this DgnDb.
    DgnCodeSpecs& CodeSpecs() const {return const_cast<DgnCodeSpecs&>(m_codeSpecs);} //!< The codeSpecs associated with this DgnDb
    DgnSearchableText& SearchableText() const {return const_cast<DgnSearchableText&>(m_searchableText);} //!< The searchable text table for this DgnDb
    DGNPLATFORM_EXPORT TxnManagerR Txns();                 //!< The TxnManager for this DgnDb.
    DGNPLATFORM_EXPORT RevisionManagerR Revisions() const; //!< The RevisionManager for this DgnDb.

    //! Imports EC Schemas into the DgnDb
    //! @param[in] schemas Schemas to be imported.
    //! @remarks
    //! <ul>
    //! <li> ONLY to be used for cases where the schemas are NOT paired with a domain.
    //! <li> It's the caller's responsibility to start a new transaction before this call and commit it after a successful
    //! import. If an error happens during the import, the new transaction is abandoned within the call.
    //! <li> Errors out if there are local changes (uncommitted or committed). These need to be flushed by committing
    //! the changes if necessary, and then creating a revision. See @ref RevisionManager.
    //! <li> If the schemas already exist in the Database, they are upgraded if the schemas passed in have a newer, but
    //! compatible version number.
    //! </ul>
    DGNPLATFORM_EXPORT SchemaStatus ImportSchemas(bvector<ECN::ECSchemaCP> const& schemas);

    //! Drop a unreferenced schema with no instances
    //! @param[in] name schema that need to be dropped.
    DGNPLATFORM_EXPORT BeSQLite::EC::DropSchemaResult DropSchema(Utf8StringCR name, bool logIssue = true);

    DGNPLATFORM_EXPORT static BeSQLite::DbResult SchemaStatusToDbResult(SchemaStatus status, bool isUpgrade);

    //! Inserts a new link table ECRelationship.
    //! @note This function is only for ECRelationships that are stored in a link table. ECRelationships that are implemented as Navigation properties must be accessed using the element property API.
    //! @param[out] relKey key of the new ECRelationship
    //! @param[in] relClass ECRelationshipClass to create an instance of
    //! @param[in] sourceId SourceECInstanceId.
    //! @param[in] targetId TargetECInstanceId.
    //! @param[in] relInstanceProperties If @p relClass has ECProperties, pass its values via this parameter. Note: In this
    //! case @ref ECN::IECRelationshipInstance::GetSource "IECRelationshipInstance::GetSource" and @ref ECN::IECRelationshipInstance::GetTarget "IECRelationshipInstance::GetTarget"
    //! don't have to be set in @p relInstanceProperties
    //! @return BE_SQLITE_OK in case of success. Error codes otherwise
    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertLinkTableRelationship(BeSQLite::EC::ECInstanceKey& relKey, ECN::ECRelationshipClassCR relClass, BeSQLite::EC::ECInstanceId sourceId, BeSQLite::EC::ECInstanceId targetId, ECN::IECRelationshipInstanceCP relInstanceProperties = nullptr);

    //! Inserts a new link table ECRelationship between two elements.
    //! @note This function is only for ECRelationships that are stored in a link table. ECRelationships that are implemented as Navigation properties must be accessed using the element property API.
    //! @param[out] relKey key of the new ECRelationship
    //! @param[in] relClass ECRelationshipClass to create an instance of
    //! @param[in] sourceId The "source" element.
    //! @param[in] targetId The "target" element.
    //! @param[in] relInstanceProperties If @p relClass has ECProperties, pass its values via this parameter. Note: In this
    //! case @ref ECN::IECRelationshipInstance::GetSource "IECRelationshipInstance::GetSource" and @ref ECN::IECRelationshipInstance::GetTarget "IECRelationshipInstance::GetTarget"
    //! don't have to be set in @p relInstanceProperties
    //! @return BE_SQLITE_OK in case of success. Error codes otherwise
    BeSQLite::DbResult InsertLinkTableRelationship(BeSQLite::EC::ECInstanceKey& relKey, ECN::ECRelationshipClassCR relClass, DgnElementId sourceId, DgnElementId targetId, ECN::IECRelationshipInstanceCP relInstanceProperties = nullptr)
        {
        return InsertLinkTableRelationship(relKey, relClass, BeSQLite::EC::ECInstanceId(sourceId.GetValue()), BeSQLite::EC::ECInstanceId(targetId.GetValue()), relInstanceProperties);
        }

    //! Update one or more properties of an existing link table ECRelationship instance.
    //! Note that you cannot change the source or target.
    //! @note This function is only for ECRelationships that are stored in a link table.
    //! @param key Identifies the relationship instance.
    //! @param props Contains the properties to be written. Note that this functions updates props by setting its InstanceId.
    //! @return BE_SQLITE_OK in case of success. Error codes otherwise
    DGNPLATFORM_EXPORT BeSQLite::DbResult UpdateLinkTableRelationshipProperties(BeSQLite::EC::ECInstanceKeyCR key, ECN::IECInstanceR props);

    //! Deletes link table ECRelationships which match the specified @p sourceId and @p targetId.
    //! @note This function is only for ECRelationships that are stored in a link table. To "delete" an ECRelationship that is implemented as a Navigation property, you must set the appropriate element property to NULL, if that is allowed.
    //! @remarks @p sourceId and @p targetId are used to build the ECSQL where clause. So they are used to filter
    //! what to delete. If one of them is invalid, it will not be included in the filter. If both are invalid, it is an error.
    //! @param[in] relClassECSqlName ECRelationshipClass name in ECSQL format (schema name.class name)
    //! @param[in] sourceId SourceECInstanceId filter. If invalid, no SourceECInstanceId filter will be applied.
    //! @param[in] targetId TargetECInstanceId filter. If invalid, no TargetECInstanceId filter will be applied.
    //! @return BE_SQLITE_OK in case of success. Error codes otherwise
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteLinkTableRelationships(Utf8CP relClassECSqlName, BeSQLite::EC::ECInstanceId sourceId, BeSQLite::EC::ECInstanceId targetId);

    //! Deletes link table ECRelationships which match the specified @p sourceId and @p targetId.
    //! @note This function is only for ECRelationships that are stored in a link table. To "delete" an ECRelationship that is implemented as a Navigation property, you must set the appropriate element property to NULL, if that is allowed.
    //! @remarks @p sourceId and @p targetId are used to build the ECSQL where clause. So they are used to filter
    //! what to delete. If one of them is invalid, it will not be included in the filter. If both are invalid, it is an error.
    //! @param[in] relClassECSqlName ECRelationshipClass name in ECSQL format (schema name.class name)
    //! @param[in] sourceId SourceECInstanceId filter. If invalid, no SourceECInstanceId filter will be applied.
    //! @param[in] targetId TargetECInstanceId filter. If invalid, no TargetECInstanceId filter will be applied.
    //! @return BE_SQLITE_OK in case of success. Error codes otherwise
    BeSQLite::DbResult DeleteLinkTableRelationships(Utf8CP relClassECSqlName, DgnElementId sourceId, DgnElementId targetId)
        {
        return DeleteLinkTableRelationships(relClassECSqlName, BeSQLite::EC::ECInstanceId(sourceId.GetValueUnchecked()), BeSQLite::EC::ECInstanceId(targetId.GetValueUnchecked()));
        }

    //! Deletes a specific link table ECRelationship
    //! @note This function is only for ECRelationships that are stored in a link table. To "delete" an ECRelationship that is implemented as a Navigation property, you must set the appropriate element property to NULL, if that is allowed.
    //! @param key Identifies the ECRelationship instance
    //! @return BE_SQLITE_OK in case of success. Error codes otherwise
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteLinkTableRelationship(BeSQLite::EC::ECInstanceKeyCR key);

    //! Gets a cached and prepared ECSqlStatement that can be used only for select.
    DGNPLATFORM_EXPORT BeSQLite::EC::CachedECSqlStatementPtr GetPreparedECSqlStatement(Utf8CP ecsql, bool logPrepareErrors = true) const;
    //! Gets a cached and prepared ECSqlStatement that can be used to modify the Db. This should be used only for aspects.
    DGNPLATFORM_EXPORT BeSQLite::EC::CachedECSqlStatementPtr GetNonSelectPreparedECSqlStatement(Utf8CP ecsql, BeSQLite::EC::ECCrudWriteToken const*,bool logPrepareErrors = true) const;

    //! Perform a SQLite VACUUM on this DgnDb. This potentially makes the file smaller and more efficient to access.
    DGNPLATFORM_EXPORT DgnDbStatus CompactFile();

    // determine whether this Db has a parent changeset id (is attached to a timeline)
    DGNPLATFORM_EXPORT bool HasParentChangeset() const;
    // determine whether this StandaloneDb requires txns if it is modified
    DGNPLATFORM_EXPORT bool RequireStandaloneTxns() const;
    // determine whether this StandaloneDb permits write operations
    bool AllowStandaloneWrites() const {BeJsDocument val; QueryStandaloneEditFlags(val); return !val.isNull(); }
    // determine whether this DgnDb requires txns whenever it is modified
    bool AreTxnsRequired() const { return IsBriefcase() || (IsStandalone() && RequireStandaloneTxns()); }
    // determine whether this DgnDb requires txns and is opened writeable
    bool AreTxnsEnabled() const { return !IsReadonly() && AreTxnsRequired(); }
    // determine whether this DgnDb is a BriefcaseDb.
    bool IsBriefcase() const { return GetBriefcaseId().IsBriefcase(); }
    // determine whether this DgnDb is a StandaloneDb
    bool IsStandalone() const { return GetBriefcaseId().IsStandalone(); }
    // determine whether this DgnDb is a snapshot StandaloneDb.
    bool IsSnapshot() const { return IsStandalone() && !AllowStandaloneWrites(); }
    // determine whether this DgnDb is a checkpoint snapshot StandaloneDb
    bool IsCheckpointSnapshot() const { return IsSnapshot() && HasParentChangeset(); }
    // determine whether if this DgnDb is an editable StandaloneDb.
    bool IsWriteableStandalone() const { return IsStandalone() && AllowStandaloneWrites(); }

    DGNPLATFORM_EXPORT RepositoryModelPtr GetRepositoryModel(); //!< Return the RepositoryModel for this DgnDb.
    DGNPLATFORM_EXPORT DictionaryModelR GetDictionaryModel(); //!< Return the dictionary model for this DgnDb.
    DGNPLATFORM_EXPORT LinkModelPtr GetRealityDataSourcesModel(); //!< Return the LinkModel listing the reality data sources for this DgnDb.

/** @name DgnPlatform Threads */
/** @{ */
    //! Ids for DgnPlatform threads
    enum class ThreadId {Unknown=0, Client=100, IoPool=103, CpuPool=104};

    DGNPLATFORM_EXPORT static ThreadId GetThreadId();    //!< Get the ThreadId for the current thread
    DGNPLATFORM_EXPORT static WCharCP GetThreadIdName(); //!< For debugging purposes, get the current ThreadId as a string
    DGNPLATFORM_EXPORT static void SetThreadId(ThreadId);    //!< Set the ThreadId for the current thread
    static void VerifyThread(ThreadId id) {BeAssert(id==GetThreadId());}   //!< assert that this is a specific thread
    static void VerifyMainThread() {VerifyThread(ThreadId::Client);}     //!< assert that this is the Client thread
    static void VerifyClientThread() {VerifyThread(ThreadId::Client);}     //!< assert that this is the Client thread
    static bool IsMainThread() {return GetThreadId() == ThreadId::Client;}
/** @} */

    //! Gets the permission token which all code within DgnPlatform has to pass to non-SELECT ECSQL statements
    //! or other non-read EC CRUD operations.
    //! Otherwise the preparation of the ECSQL or the write operation will fail.
    //! @return EC CRUD write token. Is never nullptr but is returned as pointer as this is how you pass it to the ECSQL APIs.
    //! @private
    BeSQLite::EC::ECCrudWriteToken const* GetECCrudWriteToken() const; //not inlined as it must not be called externally

    //! Gets the permission token to perform a ECSchema import/update
    //! @return ECSchemaImportToken. Is never nullptr but is returned as pointer as this is how you pass it to ECDbSchemaManager::ImportSchemas.
    //! @private
    BeSQLite::EC::SchemaImportToken const* GetSchemaImportToken() const; //not inlined as it must not be called externally

    //! @private internal use only (V8 converter)
    //! Imports v8 EC Schemas into the DgnDb
    //! @param[in] schemas Schemas to be imported.
    //! @remarks
    //! <ul>
    //! <li> Only used by the V8 converter for first importing V8 legacy schemas. Upgrades of existing schemas are
    //! not allowed.
    //! <li> It's the caller's responsibility to start a new transaction before this call and commit it after a successful
    //! import. If an error happens during the import, the new transaction is abandoned within the call.
    //! <li> Errors out if there are local changes (uncommitted or committed). These need to be flushed by committing
    //! the changes if necessary, and then creating a revision. See @ref RevisionManager.
    //! </ul>
    DGNPLATFORM_EXPORT SchemaStatus ImportV8LegacySchemas(bvector<ECN::ECSchemaCP> const& schemas, size_t* numImported = nullptr);

    //! Utility method to get the next id in a sequence
    //! @private internal use only
    BeSQLite::BeBriefcaseBasedIdSequence const& GetElementIdSequence() const { return m_elementIdSequence; }

    //! Utility method to get the reset the element id sequence
    //! @private internal use only
    BeSQLite::DbResult ResetElementIdSequence(BeSQLite::BeBriefcaseId briefcaseId);

    //! Called to start a purge operation and obtain elevated delete permission.
    //! @note The overall operation should only delete DefinitionElements that are not used.
    void BeginPurgeOperation() { ++m_purgeOperation; }
    //! End a purge operation and release the elevated delete permission.
    void EndPurgeOperation() { if (m_purgeOperation > 0) { --m_purgeOperation; } }
    //! Return true if running in the context of a purge operation that will ensure a set of DefinitionElements are not used before deleting them.
    bool IsPurgeOperationActive() const { return m_purgeOperation > 0; }

    //=======================================================================================
    //! Calls BeginPurgeOperation in the constructor and EndPurgeOperation in the destructor.
    // @bsiclass
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE PurgeOperation : NonCopyableClass
    {
    private:
        DgnDbR m_db;
    public:
        PurgeOperation(DgnDbR db) : m_db(db) { m_db.BeginPurgeOperation(); }
        ~PurgeOperation() { m_db.EndPurgeOperation(); }
    };

    BeSQLite::DbResult CreateRebaseTable(); //!< @private
    DGNPLATFORM_EXPORT DRange3d ComputeGeometryExtentsWithoutOutliers(DRange3dP rangeWithOutliers = nullptr, bvector<BeInt64Id>* elementOutliers = nullptr, double maxDeviations = 5.0) const;
};

END_BENTLEY_DGN_NAMESPACE

/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/SatelliteChangeSets.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/BeFileName.h>
#include <Bentley/BeFile.h>
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnPlatform.h>

#define CHANGES_TABLE_DIRECT_ChangeSet  "changes_ChangeSet"
#define CHANGES_TABLE_DIRECT_Property   "be_Prop"

#define CHANGES_ATTACH_ALIAS        "CHANGES"
#define CHANGES_ATTACH_ALIAS_PREFIX CHANGES_ATTACH_ALIAS "."
#define CHANGES_TABLE_ChangeSet     CHANGES_ATTACH_ALIAS_PREFIX CHANGES_TABLE_DIRECT_ChangeSet
#define CHANGES_TABLE_Property      CHANGES_ATTACH_ALIAS_PREFIX CHANGES_TABLE_DIRECT_Property

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* Changeset-related properties that are stored in a project that keep track of the 
* latest changeset applied to the project and when it will be time to query for updates.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ProjectChangeSetProperties
    {
    UInt64   m_latestChangeSetId; //!< The SequenceNumber of the changeset most recently applied
    DateTime m_expirationDate; //!< When to query for new changes

    //! Updates the changeset-related properties in the specified db
    //! @param[in] project The project to be updated. Must already be open for read-write
    //! @return non-zero error status if the project's properties could not be updated or if \a info does not apply to this project.
    DGNPLATFORM_EXPORT BentleyStatus SetChangeSetProperties (BeSQLite::Db& project);

    //! Reads the changeset-related properties from the specified db
    //! @param[in] project The project to be updated. Must already be open for read-write
    //! @return non-zero error status if the project's properties could not be updated or if \a info does not apply to this project.
    DGNPLATFORM_EXPORT BentleyStatus GetChangeSetProperties (BeSQLite::Db& project);
    };

/*=================================================================================**//**
* Holds one or more a SQLite changeset or patchset in a stand-alone file that is
* associated with another SQLite Db, from which the changeset/patchset was computed.
* Can apply the changeset/patchset to a copy of the original Db.
* Holds a description and other annotations of the changesets.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SatelliteChangeSets
    {
    struct ChangesProperty
        {
        struct ChangesPropertySpec : BeSQLite::PropertySpec
            {
            ChangesPropertySpec (Utf8CP name, ProperyTxnMode setting) : PropertySpec (name, "Changes", setting, PropertySpec::COMPRESS_PROPERTY_No) {}
            };

        struct ChangesPropertyInstance : ChangesPropertySpec {ChangesPropertyInstance (Utf8CP name) : ChangesPropertySpec(name, PropertySpec::TXN_MODE_Normal){}};

        static ChangesPropertyInstance SchemaVersion()          {return ChangesPropertyInstance("SchemaVersion");}
        static ChangesPropertyInstance ProjectGUID()            {return ChangesPropertyInstance("ProjectGUID");}
        static ChangesPropertyInstance ProjectDbSchemaVersion() {return ChangesPropertyInstance("ProjectDbSchemaVersion");}
        static ChangesPropertyInstance ProjectSchemaVersion()   {return ChangesPropertyInstance("ProjectSchemaVersion");}
        static ChangesPropertyInstance FirstChangeSetSequenceNumber() {return ChangesPropertyInstance("FirstChangeSetSequenceNumber");}
        static ChangesPropertyInstance LastChangeSetSequenceNumber()  {return ChangesPropertyInstance("LastChangeSetSequenceNumber");}
        };

    // --------------------------------------------------------------------------------------
    //  Public Nested Types
    // --------------------------------------------------------------------------------------
public:
    enum Compressed {COMPRESSED_No=0, COMPRESSED_Lzma=1, COMPRESSED_Snappy=2};

    enum ChangeSetType {PatchSet=0, ChangeSet=1};

    //! Information about a changeset or patchset.
    struct ChangeSetInfo
        {
        UInt64 m_sequenceNumber;    //!< Where this changeset/patchset falls in the sequence of changes generated from the project. This value is assigned by changes file, not by SatelliteChangeSets! This value is also the "name" of the embedded changeset "file" in this db.
        ChangeSetType m_type;       //!< The type of changeset this is
        Utf8String m_description;   //!< The user's description of the change
        DateTime m_time;            //!< When the project's changes were committed and this changeset/patchset was recorded. In DateTime format, UTC.

        DGNPLATFORM_EXPORT ChangeSetInfo();

//__PUBLISH_SECTION_END__

        UInt8 GetTypeAsInt() const;
        BentleyStatus SetTypeFromInt (UInt8);

        static BeSQLite::DbResult CreateTable (BeSQLite::Db&);
        BeSQLite::DbResult Insert (BeSQLite::Db&) const;

        static BeSQLite::DbResult PrepareFindBySequenceNumber (BeSQLite::Statement&, SatelliteChangeSets&, UInt64 cid);
        BeSQLite::DbResult StepFindBySequenceNumber (BeSQLite::Statement&);

//__PUBLISH_SECTION_START__

        //! Prepare to step through ChangeSetInfos in a .changes file
        static BeSQLite::DbResult Prepare (BeSQLite::Statement&, SatelliteChangeSets&);
        BeSQLite::DbResult Step (BeSQLite::Statement&);

        //! Prepare a statement to find the changesets that come after \a cid, where order is determined by id numerical value.
        static BeSQLite::DbResult PrepareFindLater (BeSQLite::Statement&, SatelliteChangeSets&, UInt64 cid);

        //! Step to the next change set that come after a specified seed cid.
        BeSQLite::DbResult StepFindLater (BeSQLite::Statement&);
        };

    friend struct ChangeSetInfo;

    typedef bmap<UInt64, BeFileName> T_ChangesFileDictionary;

//__PUBLISH_SECTION_END__
    //! A changeset applier delegate
    struct IApplyChangeSet
        {
        //! Apply the changeset to the db
        virtual BeSQLite::DbResult ApplyChangeSet (BeSQLite::Db& db, BeSQLite::ChangeSet& changeset) = 0;
        };

    //! Implementation of IApplyChangeSet that simply applies the changeset.
    struct DefaultApplyChangeSet : IApplyChangeSet
        {
        virtual BeSQLite::DbResult ApplyChangeSet (BeSQLite::Db& db, BeSQLite::ChangeSet& changeset) {return changeset.ApplyChanges(db);}
        };
//__PUBLISH_SECTION_START__

    BentleyStatus PerformVersionChecks();

    // --------------------------------------------------------------------------------------
    //  Protected Member Variables
    // --------------------------------------------------------------------------------------
protected:
    BeSQLite::Db* m_project;
    BeSQLite::DbResult m_lastError;
    Utf8String m_lastErrorDescription;
    BeFileName m_dbFileName;
    bool m_isValid;

    // --------------------------------------------------------------------------------------
    //  Protected Member Functions
    // --------------------------------------------------------------------------------------
protected:
//__PUBLISH_SECTION_END__
    //! Create tables for a new .changes file
    DGNPLATFORM_EXPORT BeSQLite::DbResult CreateTables();

    void UpdateChangeSetSequenceNumberRange (UInt64);

    static BentleyStatus ApplyChangeSets0 (UInt32& nChangesApplied, BeSQLite::Db& project, bvector<BeFileName> const&, UInt64 lastKnownChangesetId, IApplyChangeSet&);

    DGNPLATFORM_EXPORT BeSQLite::DbResult SavePropertyString (BeSQLite::PropertySpec const& spec, Utf8StringCR stringData, UInt64 majorId=0, UInt64 subId=0);
    DGNPLATFORM_EXPORT BeSQLite::DbResult QueryProperty (Utf8StringR str, BeSQLite::PropertySpec const& spec, UInt64 id=0, UInt64 subId=0) const;

//__PUBLISH_SECTION_START__

    SatelliteChangeSets (SatelliteChangeSets const&); // no!
    SatelliteChangeSets& operator=(SatelliteChangeSets const&); // no!

    // --------------------------------------------------------------------------------------
    //  Public Member Functions
    // --------------------------------------------------------------------------------------
public:
    DGNPLATFORM_EXPORT SatelliteChangeSets();
    DGNPLATFORM_EXPORT ~SatelliteChangeSets();

    //! @name The Project - a SatelliteChangeSets object is always used in relation to a Db
    //! @{

    DGNPLATFORM_EXPORT BeSQLite::Db* GetProject();

    //! @}

//__PUBLISH_SECTION_END__

    //! @name Creating and Attaching a Changes File
    //! @{
    //! Create an empty file. Then attach it to the project.
    DGNPLATFORM_EXPORT BentleyStatus CreateEmptyFile (BeFileNameCR dbName, bool deleteIfExists);
    DGNPLATFORM_EXPORT BentleyStatus OnAttach (BeSQLite::Db&, BeSQLite::SchemaVersion const& targetSchemaVersion);
    DGNPLATFORM_EXPORT BentleyStatus AttachToProject (BeSQLite::Db& targetProject, BeFileNameCR dbName);
    BeFileName GetDbFileName() const {return m_dbFileName;}
    //! @}

    //! @name Working With a Changes File
    //! @{

    //! Save changes to the changes file.
    DGNPLATFORM_EXPORT BentleyStatus SaveChanges();

    //! Save changes to the changes file.
    DGNPLATFORM_EXPORT void AbandonChanges();

    //! Close the changes file.
    DGNPLATFORM_EXPORT void Close();
    
    //! Query if the changes file is valid (created or read)
    bool IsValid() const {return m_isValid;}

    //! Mark the changes file as valid or not.
    void SetValid (bool b) {m_isValid=b;}

    DGNPLATFORM_EXPORT void SetLastError (BeSQLite::DbResult);
    DGNPLATFORM_EXPORT void GetLastError (BeSQLite::DbResult&, Utf8String&);

    //! @}
//__PUBLISH_SECTION_START__

    //! @name Changes File Properties
    //! @{
    DGNPLATFORM_EXPORT UInt64 GetFirstChangeSetSequenceNumber();

    DGNPLATFORM_EXPORT UInt64 GetLastChangeSetSequenceNumber();

    DGNPLATFORM_EXPORT BeDbGuid GetTargetProjectGuid();

    DGNPLATFORM_EXPORT BentleyStatus SetExpirationDate (DateTime const& dt);
    DGNPLATFORM_EXPORT void GetExpirationDate (DateTime& expiration);
    //! @}

    //! @name Working with ChangeSets Within a .changes file
    //! @{

//__PUBLISH_SECTION_END__

    //! Save a changeset. Adds a record to this DB and also creates an external ".changes" file to hold the changeset data.
    //! @param[out] info The changeset record created 
    //! @param[in] csid The SequenceNumber of the changeset. Must be the value issued by SyncInfo for the target project.
    //! @param[in] data The data in the changeset
    //! @param[in] size The number of bytes of data in the changeset. Note that this is a signed 32-bit number. That is the type returned by BeSQLite::ChangeSet::GetSize(), and it is the type accepted by BeSQLite::Statement::BindBlob.
    //! @param[in] type The changeset type.
    //! @param[in] desc A description of the change
    //! @param[in] time The time of the change. Is converted to UTC if necessary.
    //! @param[in] compressOption How or if to compress the changeset data
    //! @return non-zero error status if the changeset could not be saved.
    DGNPLATFORM_EXPORT BentleyStatus InsertChangeSet (ChangeSetInfo& info, UInt64 csid, Compressed compressOption, void const* data, Int32 size, ChangeSetType type, Utf8StringCR desc, DateTime const& time);

    //! Updater calls then when trying to recover from an error.
    DGNPLATFORM_EXPORT BentleyStatus OnFatalError (UInt64);

    //! Compute the name of an external changeset file for the specified changeset id and the specified targetProject
    //! @param[in] targetProject the project for which changes are being collected
    //! @param[in] csid The SequenceNumber of the changeset that we want to store
    //! @param[in] useProjectGuid if true, the changes filename will be based on the project's GUID. If false, the name will be based on the project's name.
    //! @return the name of the changes file 
    DGNPLATFORM_EXPORT static BeFileName GetChangeSetFileName (BeSQLite::Db& targetProject, UInt64 csid, bool useProjectGuid);

    //! Get the changeset for the specified SequenceNumber
    //! @param[out] changeSet  the changeset
    //! @param[in] cid The changeset's SequenceNumber
    //! @return non-zero error status if the changeset could not be found.
    DGNPLATFORM_EXPORT BentleyStatus ExtractChangeSetBySequenceNumber (BeSQLite::ChangeSet& changeSet, UInt64 cid);

//__PUBLISH_SECTION_START__

    //! Query a changeset by its SequenceNumber
    //! @param[out] csinfo  the changeset information
    //! @param[in] cid The changeset's SequenceNumber
    //! @return non-zero error status if the changeset could not be found.
    DGNPLATFORM_EXPORT BentleyStatus FindChangeSetBySequenceNumber (ChangeSetInfo& csinfo, UInt64 cid);

    //! @}

    //! @name Detecting and Applying Changes
    //! @{

    struct ChangeSetRange
        {
        UInt64 m_earliest;
        UInt64 m_latest;
        };

//__PUBLISH_SECTION_END__

    //! Apply the applicable changesets contained in \a csfiles to \a project. To be applicable, a changes file must have been generated from 
    //! the project (or a copy of it), and its starting sequence number must be after the project's latest changeset property.
    //! Note that a changes file can contain multiple changesets.
    //! @param[out] nChangesApplied The number of changes that were actually applied is returned here.
    //! @param[in] project          The project to be updated. Must already be open for read-write
    //! @param[in] csfiles          The changeset files to use
    //! @param[in] applier          Delegate that actually applies the changeset.
    //! @return non-zero error status if the project is not open read-write or if the change sets could not be applied
    //! @see DetectChangeSets
    DGNPLATFORM_EXPORT static BentleyStatus ApplyChangeSets (UInt32& nChangesApplied, BeSQLite::Db& project, bvector<BeFileName> const& csfiles, IApplyChangeSet& applier);

    //! Apply the applicable changesets contained in \a csfiles to \a project. To be applicable, a changes file must have been generated from 
    //! the project (or a copy of it), and its starting sequence number must be after the project's latest changeset property.
    //! Note that a changes file can contain multiple changesets.
    //! @param[out] nChangesApplied The number of changes that were actually applied is returned here.
    //! @param[in] project          The project to be updated. Must already be open for read-write
    //! @param[in] csfiles          The changeset files to use
    //! @param[in] applier          Delegate that actually applies the changeset.
    //! @return non-zero error status if the project is not open read-write or if the change sets could not be applied
    //! @see DetectChangeSets
    DGNPLATFORM_EXPORT static BentleyStatus ApplyChangeSets (UInt32& nChangesApplied, BeSQLite::Db& project, T_ChangesFileDictionary const& csfiles, IApplyChangeSet& applier);

//__PUBLISH_SECTION_START__

    //! Detect the changes files in \a changeSetDir that are applicable to \a project. To be applicable, a changes file must have been generated from 
    //! the project (or a copy of it), and its starting sequence number must be after the project's latest changeset property.
    //! Note that a changes file can contain multiple changesets.
    //! @param[out] csfiles         A dictionary of changeset files found \a changeSetDir that are applicable to \a project. The key for each entry is the changeset ID.
    //! @param[out] range           The range of changeset SequenceNumbers found in the files.
    //! @param[in] project          The project that the changesets relate to. 
    //! @param[in] candidateFiles   The files to consider
    //! @return non-zero error status if the changesets found in \a candidateFiles are invalid
    //! @note This function does not apply changesets to the project. It just detects relevant changesets.
    //! @see ExtractChangeSets
    //! @see ApplyChangeSets
    DGNPLATFORM_EXPORT static BentleyStatus DetectChangeSets (T_ChangesFileDictionary& csfiles, ChangeSetRange& range, BeSQLite::Db& project, bvector<BeFileName> const& candidateFiles);
    
    //! @}
    
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
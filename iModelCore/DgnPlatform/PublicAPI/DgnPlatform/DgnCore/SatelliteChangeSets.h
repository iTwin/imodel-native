/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/SatelliteChangeSets.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/BeFileName.h>
#include <Bentley/BeFile.h>
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnPlatform.h>

#define CHANGESET_Table       "dgn_changeSet"
#define CHANGES_ATTACH_ALIAS  "changes"
#define CHANGESET_ATTACH(name) CHANGES_ATTACH_ALIAS "." name

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
* Changeset-related properties that are stored in a project that keep track of the 
* latest changeset applied to the project and when it will be time to query for updates.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ChangeSetProperties
{
    uint64_t m_latestChangeSetId; //!< The SequenceNumber of the changeset most recently applied
    DateTime m_expirationDate; //!< When to query for new changes

    //! Updates the changeset-related properties in the specified db
    //! @param[in] dgndb The DgnDb to be updated. Must already be open for read-write
    //! @return non-zero error status if the project's properties could not be updated or if \a info does not apply to this project.
    DGNPLATFORM_EXPORT BentleyStatus SaveToDb(BeSQLite::Db& dgndb);

    //! Reads the changeset-related properties from the specified db
    //! @param[in] dgndb The DgnDb to load from.
    DGNPLATFORM_EXPORT BentleyStatus LoadFromDb(BeSQLite::Db& dgndb);
};

/*=================================================================================**//**
* Holds one or more a SQLite changeset or patchset in a stand-alone file that is
* associated with another SQLite Db, from which the changeset/patchset was computed.
* Can apply the changeset/patchset to a copy of the original Db.
* Holds a description and other annotations of the changesets.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SatelliteChangeSets : NonCopyableClass
{
    struct Property
    {
        struct Spec : BeSQLite::PropertySpec
        {
            Spec(Utf8CP name) : PropertySpec(name, "Changes", PropertySpec::Mode::Normal, PropertySpec::Compress::No) {}
        };

        static Spec SchemaVersion()        {return Spec("SchemaVersion");}
        static Spec DgnDbGuid()            {return Spec("DgnDbGuid");}
        static Spec DbSchemaVersion()      {return Spec("DbSchemaVersion");}
        static Spec DgnDbSchemaVersion()   {return Spec("DgnDbSchemaVersion");}
        static Spec FirstSequenceNumber()  {return Spec("FirstSequenceNumber");}
        static Spec LastSequenceNumber()   {return Spec("LastSequenceNumber");}
    };

public:
    enum class Compressed {No=0, Lzma=1, Snappy=2};
    enum class ChangeSetType {Patch=0, Full=1};

    //! Information about a changeset or patchset.
    struct ChangeSetInfo
    {
        bool          m_isValid;
        uint64_t      m_sequenceNumber;  //!< Where this changeset/patchset falls in the sequence of changes generated from the project. This value is assigned by changes file, not by SatelliteChangeSets! This value is also the "name" of the embedded changeset "file" in this db.
        ChangeSetType m_type;            //!< The type of changeset this is
        Utf8String    m_description;     //!< The user's description of the change
        DateTime      m_time;            //!< When the project's changes were committed and this changeset/patchset was recorded. In DateTime format, UTC.

        ChangeSetInfo(uint64_t num, ChangeSetType type, Utf8CP descr, DateTime time) : m_sequenceNumber(num), m_type(type), m_description(descr), m_time(time) {m_isValid=true;}
        ChangeSetInfo(BeSQLite::Statement&);
        bool IsValid() const {return m_isValid;}
    };

    typedef bmap<uint64_t, BeFileName> T_ChangesFileDictionary;

protected:
    BeSQLite::Db*      m_dgndb;
    BeSQLite::DbResult m_lastError;
    Utf8String         m_lastErrorDescription;
    BeFileName         m_dbFileName;
    bool               m_isValid;

    //! Create tables for a new .changes file
    BeSQLite::DbResult CreateTables();

    BentleyStatus PerformVersionChecks();
    void UpdateSequenceNumberRange(uint64_t);

    BeSQLite::DbResult SavePropertyString(BeSQLite::PropertySpec const& spec, Utf8StringCR stringData, uint64_t majorId=0, uint64_t subId=0);
    BeSQLite::DbResult QueryProperty(Utf8StringR str, BeSQLite::PropertySpec const& spec, uint64_t id=0, uint64_t subId=0) const;

public:
    SatelliteChangeSets() : m_lastError(BeSQLite::BE_SQLITE_OK), m_isValid(false) {m_dgndb=nullptr;}
    DGNPLATFORM_EXPORT ~SatelliteChangeSets();

    //! @name The Project - a SatelliteChangeSets object is always used in relation to a Db
    //! @{

    BeSQLite::Db* GetDgnDb() {return m_dgndb;}

    //! @}

    //! @name Creating and Attaching a Changes File
    //! @{
    //! Create an empty file. Then attach it to the project.
    DGNPLATFORM_EXPORT BentleyStatus CreateEmptyFile(BeFileNameCR dbName, bool deleteIfExists);
    DGNPLATFORM_EXPORT BentleyStatus OnAttach(BeSQLite::Db&, BeSQLite::SchemaVersion const& targetSchemaVersion);
    DGNPLATFORM_EXPORT BentleyStatus AttachToDb(BeSQLite::Db& targetProject, BeFileNameCR dbName);
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
    void SetValid(bool b) {m_isValid=b;}

    DGNPLATFORM_EXPORT void SetLastError(BeSQLite::DbResult);
    DGNPLATFORM_EXPORT void GetLastError(BeSQLite::DbResult&, Utf8String&);

    //! @}

    //! @name Changes File Properties
    //! @{
    DGNPLATFORM_EXPORT uint64_t GetFirstSequenceNumber();
    DGNPLATFORM_EXPORT uint64_t GetLastSequenceNumber();
    DGNPLATFORM_EXPORT BeDbGuid GetTargetDbGuid();
    DGNPLATFORM_EXPORT BentleyStatus SetExpirationDate(DateTime const& dt);
    DGNPLATFORM_EXPORT void GetExpirationDate(DateTime& expiration);
    //! @}

    //! @name Working with ChangeSets Within a .changes file
    //! @{


    //! Save a changeset. Adds a record to this DB and also creates an external ".changes" file to hold the changeset data.
    //! @param[in] info The changeset record created 
    //! @param[in] data The data in the changeset
    //! @param[in] datasize The number of bytes of data in the changeset. Note that this is a signed 32-bit number. That is the type returned by BeSQLite::ChangeSet::GetSize(), and it is the type accepted by BeSQLite::Statement::BindBlob.
    //! @param[in] compressOption How or if to compress the changeset data
    //! @return non-zero error status if the changeset could not be saved.
    DGNPLATFORM_EXPORT BentleyStatus InsertChangeSet(ChangeSetInfo const& info, Compressed compressOption, void const* data, int32_t datasize);

    //! Updater calls then when trying to recover from an error.
    DGNPLATFORM_EXPORT BentleyStatus OnFatalError(uint64_t);

    //! Compute the name of an external changeset file for the specified changeset id and the specified targetProject
    //! @param[in] targetProject the project for which changes are being collected
    //! @param[in] csid The SequenceNumber of the changeset that we want to store
    //! @param[in] useProjectGuid if true, the changes filename will be based on the project's GUID. If false, the name will be based on the project's name.
    //! @return the name of the changes file 
    DGNPLATFORM_EXPORT static BeFileName GetChangeSetFileName(BeSQLite::Db& targetProject, uint64_t csid, bool useProjectGuid);

    //! Get the changeset for the specified SequenceNumber
    //! @param[out] changeSet  the changeset
    //! @param[in] cid The changeset's SequenceNumber
    //! @return non-zero error status if the changeset could not be found.
    DGNPLATFORM_EXPORT BentleyStatus ExtractChangeSetBySequenceNumber(BeSQLite::ChangeSet& changeSet, uint64_t cid);

    //! Query a changeset by its SequenceNumber
    //! @param[in] cid The changeset's SequenceNumber
    //! @return the changeset information or an ChangeSetInfo if not found
    DGNPLATFORM_EXPORT ChangeSetInfo FindChangeSetBySequenceNumber(uint64_t cid);

    //! @}

    //! @name Detecting and Applying Changes
    //! @{

    struct ChangeSetRange
        {
        uint64_t m_earliest;
        uint64_t m_latest;
        };

    //! Dump the contents of the specified changes file.
    //! @param[in] csfile           The changes file to dump
    //! @param[in] db               The DgnDb to which the changes file applies
    //! @param[in] detailLevel      How much detail to include in the dump
    DGNPLATFORM_EXPORT static BentleyStatus Dump(BeFileNameCR csfile, BeSQLite::Db& db, int detailLevel);

    //! Apply the applicable changesets contained in \a csfiles to \a DgnDb. To be applicable, a changes file must have been generated from 
    //! the DgnDb (or a copy of it), and its starting sequence number must be after the DgnDb's latest changeset property.
    //! Note that a changes file can contain multiple changesets.
    //! @param[out] nChangesApplied The number of changes that were actually applied is returned here.
    //! @param[in] db               The DgnDb to be updated. Must already be open for read-write
    //! @param[in] csfiles          The changeset files to use
    //! @return non-zero error status if the DgnDb is not open read-write or if the change sets could not be applied
    //! @see DetectChangeSets
    DGNPLATFORM_EXPORT static BentleyStatus ApplyChangeSets(uint32_t& nChangesApplied, BeSQLite::Db& db, bvector<BeFileName> const& csfiles);

    //! Apply the applicable changesets contained in \a csfiles to \a DgnDb. To be applicable, a changes file must have been generated from 
    //! the DgnDb (or a copy of it), and its starting sequence number must be after the DgnDb's latest changeset property.
    //! Note that a changes file can contain multiple changesets.
    //! @param[out] nChangesApplied The number of changes that were actually applied is returned here.
    //! @param[in] db               The DgnDb to be updated. Must already be open for read-write
    //! @param[in] csfiles          The changeset files to use
    //! @return non-zero error status if the DgnDb is not open read-write or if the change sets could not be applied
    //! @see DetectChangeSets
    DGNPLATFORM_EXPORT static BentleyStatus ApplyChangeSets(uint32_t& nChangesApplied, BeSQLite::Db& db, T_ChangesFileDictionary const& csfiles);

    //! Detect the changes files in \a changeSetDir that are applicable to \a DgnDb. To be applicable, a changes file must have been generated from 
    //! the DgnDb (or a copy of it), and its starting sequence number must be after the DgnDb's latest changeset property.
    //! Note that a changes file can contain multiple changesets.
    //! @param[out] csfiles         A dictionary of changeset files found \a changeSetDir that are applicable to \a DgnDb. The key for each entry is the changeset ID.
    //! @param[out] range           The range of changeset SequenceNumbers found in the files.
    //! @param[in] db               The DgnDb that the changesets relate to. 
    //! @param[in] candidateFiles   The files to consider
    //! @return non-zero error status if the changesets found in \a candidateFiles are invalid
    //! @note This function does not apply changesets to the DgnDb. It just detects relevant changesets.
    //! @see ExtractChangeSets
    //! @see ApplyChangeSets
    DGNPLATFORM_EXPORT static BentleyStatus DetectChangeSets(T_ChangesFileDictionary& csfiles, ChangeSetRange& range, BeSQLite::Db& db, bvector<BeFileName> const& candidateFiles);
    //! @}
    
};

END_BENTLEY_DGN_NAMESPACE